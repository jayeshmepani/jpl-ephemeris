/*
 * Explicit time-scale conversions on two-part Julian dates.
 *
 * Scales: UTC, UT1, TAI, TT, TDB, TCG, TCB (see docs/TIME_SCALES.md).
 *   UTC <-> TAI : embedded TAI-UTC table (1960-01-01 .. table expiry), with
 *                 the pre-1972 drift; UTC two-part JDs use the ERFA/SOFA
 *                 "quasi-JD" convention (a leap-second day has 86401 s and its
 *                 fraction is scaled accordingly). The UTC<->TAI algorithm is
 *                 adapted from ERFA eraUtctai/eraTaiutc (BSD-3-Clause, see
 *                 THIRD_PARTY_NOTICES.md).
 *   TAI <-> TT  : TT = TAI + 32.184 s exactly.
 *   TT  <-> TDB : ERFA eraDtdb (Fairhead & Bretagnon 1990, geocentre),
 *                 inverse by fixed-point iteration to binary64 convergence.
 *   TDB <-> TCB : IAU 2006 Resolution B3 (ERFA eraTdbtcb/eraTcbtdb).
 *   TT  <-> TCG : IAU 2000 Resolution B1.9 (ERFA eraTttcg/eraTcgtt).
 *   TT  <-> UT1 : Delta-T of the context model (jme_set_delta_t_model /
 *                 jme_set_delta_t_userdef); AUTOMATIC uses the embedded
 *                 IERS finals2000A UT1 table where it covers the date.
 * Every failure (date outside a model or table, non-finite input) returns
 * JME_ERR with a message; nothing is clamped or extrapolated silently.
 */
#include "jme/jme.h"
#include "jme/jme_extended.h"
#include "context.h"
#include "jme_erfa.h"
#include "jme_erfa_vendor.h"
#include <math.h>
#include "jme_tls.h"

/*
 * TDB-TT (ERFA eraDtdb, Fairhead & Bretagnon 1990, ~800 terms) with a small
 * per-thread memo keyed on the exact two-part TT date: a hit returns the
 * bit-identical value of the series for the same arguments, so results are
 * unchanged; event searches evaluate the same epochs many times (light-time
 * iterations, deflector states, rate stencils).
 */
#define JME_DTDB_MEMO 16
typedef struct { double d1, d2, v; int used; } jme_dtdb_memo_entry;
static JME_THREAD_LOCAL jme_dtdb_memo_entry g_dtdb_memo[JME_DTDB_MEMO];
static JME_THREAD_LOCAL int g_dtdb_memo_next;

double internal_dtdb(double d1, double d2)
{
    int i;
    double v;
    for (i = 0; i < JME_DTDB_MEMO; i++) {
        if (g_dtdb_memo[i].used && g_dtdb_memo[i].d1 == d1 && g_dtdb_memo[i].d2 == d2) return g_dtdb_memo[i].v;
    }
    v = jme_erfa_dtdb(d1, d2, 0.0, 0.0, 0.0, 0.0);
    g_dtdb_memo[g_dtdb_memo_next].d1 = d1;
    g_dtdb_memo[g_dtdb_memo_next].d2 = d2;
    g_dtdb_memo[g_dtdb_memo_next].v = v;
    g_dtdb_memo[g_dtdb_memo_next].used = 1;
    g_dtdb_memo_next = (g_dtdb_memo_next + 1) % JME_DTDB_MEMO;
    return v;
}

#define TS_DAYSEC 86400.0

static int ts_utc_to_tai(double u1, double u2, double *t1, double *t2, char *error)
{
    int big1 = fabs(u1) >= fabs(u2);
    int iy, im, id, iyt, imt, idt;
    double fd, w, dat0, dat12, dat24, dlod, dleap, z1, z2, a2;
    double v1 = big1 ? u1 : u2;
    double v2 = big1 ? u2 : u1;

    if (jme_eraJd2cal(v1, v2, &iy, &im, &id, &fd) != 0) {
        jme_set_error(error, "UTC date is outside the supported calendar range");
        return JME_ERR;
    }
    if (internal_tai_minus_utc(iy, im, id, 0.0, &dat0, error) != JME_OK
        || internal_tai_minus_utc(iy, im, id, 0.5, &dat12, error) != JME_OK) {
        return JME_ERR;
    }
    if (jme_eraJd2cal(v1 + 1.5, v2 - fd, &iyt, &imt, &idt, &w) != 0) {
        jme_set_error(error, "UTC date is outside the supported calendar range");
        return JME_ERR;
    }
    if (internal_tai_minus_utc(iyt, imt, idt, 0.0, &dat24, error) != JME_OK) {
        /* Last day covered by the table: a leap second at its end is not yet known. */
        jme_set_error(error, "UTC day length is unknown: TAI-UTC for the following day is beyond the leap-second table");
        return JME_ERR;
    }
    dlod = 2.0 * (dat12 - dat0);
    dleap = dat24 - (dat0 + dlod);
    fd *= (TS_DAYSEC + dleap) / TS_DAYSEC;
    fd *= (TS_DAYSEC + dlod) / TS_DAYSEC;
    if (jme_eraCal2jd(iy, im, id, &z1, &z2) != 0) {
        jme_set_error(error, "UTC calendar conversion failed");
        return JME_ERR;
    }
    a2 = z1 - v1;
    a2 += z2;
    a2 += fd + dat0 / TS_DAYSEC;
    if (big1) { *t1 = v1; *t2 = a2; } else { *t1 = a2; *t2 = v1; }
    return JME_OK;
}

static int ts_tai_to_utc(double a1, double a2, double *u1o, double *u2o, char *error)
{
    int big1 = fabs(a1) >= fabs(a2);
    double b1 = big1 ? a1 : a2, b2 = big1 ? a2 : a1;
    double u1 = b1, u2 = b2, g1, g2;
    int i;
    for (i = 0; i < 3; i++) {
        if (ts_utc_to_tai(u1, u2, &g1, &g2, error) != JME_OK) return JME_ERR;
        u2 += b1 - g1;
        u2 += b2 - g2;
    }
    if (big1) { *u1o = u1; *u2o = u2; } else { *u1o = u2; *u2o = u1; }
    return JME_OK;
}

static int ts_tt_to_tdb(double t1, double t2, double *o1, double *o2, char *error)
{
    double sum = t1 + t2;
    double day = floor(sum);
    double frac = (t1 - day) + t2;
    double d0, f0;
    /* Normalise the split without losing the fraction's precision. */
    while (frac < 0.0) { frac += 1.0; day -= 1.0; }
    while (frac >= 1.0) { frac -= 1.0; day += 1.0; }
    if (internal_tt_to_tdb_split(day, frac, &d0, &f0, error) != JME_OK) return JME_ERR;
    *o1 = d0;
    *o2 = f0;
    return JME_OK;
}

static int ts_tdb_to_tt(double b1, double b2, double *o1, double *o2, char *error)
{
    /* Solve TT + dtdb(TT)/86400 = TDB by fixed-point iteration. */
    double tt1 = b1, tt2 = b2, dtr = 0.0, prev = NAN;
    int i;
    for (i = 0; i < 16; i++) {
        dtr = internal_dtdb(tt1, tt2);
        if (!isfinite(dtr)) {
            jme_set_error(error, "ERFA TDB-TT offset is not finite");
            return JME_ERR;
        }
        if (dtr == prev) break;
        prev = dtr;
        tt2 = b2 - dtr / TS_DAYSEC;
    }
    if (i == 16) {
        jme_set_error(error, "TDB-to-TT iteration did not converge");
        return JME_ERR;
    }
    *o1 = tt1;
    *o2 = tt2;
    return JME_OK;
}

static int ts_tt_to_ut1(double t1, double t2, double *o1, double *o2, char *error)
{
    double dts, next_dts;
    int i, converged = 0;

    /* Solve TT = UT1 + Delta-T(UT1) on the original two-part date. */
    dts = internal_delta_t_ex_split(t1, t2, jme_context_deltat_model(), error);
    if (!isfinite(dts)) return JME_ERR;

    for (i = 0; i < 50; i++) {
        double ut2 = t2 - dts / TS_DAYSEC;
        next_dts = internal_delta_t_ex_split(t1, ut2, jme_context_deltat_model(), error);
        if (!isfinite(next_dts)) return JME_ERR;
        if (fabs(next_dts - dts) < 1.0e-9) {
            dts = next_dts;
            converged = 1;
            break;
        }
        dts = next_dts;
    }
    if (!converged) {
        jme_set_error(error, "Two-part TT-to-UT1 inversion of the Delta-T model did not converge");
        return JME_ERR;
    }

    *o1 = t1;
    *o2 = t2 - dts / TS_DAYSEC;
    return JME_OK;
}

static int ts_ut1_to_tt(double u1, double u2, double *o1, double *o2, char *error)
{
    double dts = internal_delta_t_ex_split(u1, u2, jme_context_deltat_model(), error);
    if (!isfinite(dts)) {
        if (error != 0 && error[0] == '\0') jme_set_error(error, "Delta-T calculation failed");
        return JME_ERR;
    }
    *o1 = u1;
    *o2 = u2 + dts / TS_DAYSEC;
    return JME_OK;
}

/* Convert any scale to TT. */
static int ts_to_tt(int from, double a, double b, double *o1, double *o2, char *error)
{
    double x1, x2;
    switch (from) {
    case JME_TIME_SCALE_TT: *o1 = a; *o2 = b; return JME_OK;
    case JME_TIME_SCALE_TAI: *o1 = a; *o2 = b + 32.184 / TS_DAYSEC; return JME_OK;
    case JME_TIME_SCALE_UTC:
        if (ts_utc_to_tai(a, b, &x1, &x2, error) != JME_OK) return JME_ERR;
        *o1 = x1; *o2 = x2 + 32.184 / TS_DAYSEC; return JME_OK;
    case JME_TIME_SCALE_UT1: return ts_ut1_to_tt(a, b, o1, o2, error);
    case JME_TIME_SCALE_TDB: return ts_tdb_to_tt(a, b, o1, o2, error);
    case JME_TIME_SCALE_TCB:
        if (jme_eraTcbtdb(a, b, &x1, &x2) != 0) { jme_set_error(error, "TCB-to-TDB conversion failed"); return JME_ERR; }
        return ts_tdb_to_tt(x1, x2, o1, o2, error);
    case JME_TIME_SCALE_TCG:
        if (jme_eraTcgtt(a, b, o1, o2) != 0) { jme_set_error(error, "TCG-to-TT conversion failed"); return JME_ERR; }
        return JME_OK;
    default:
        jme_set_error(error, "Unsupported source time scale");
        return JME_ERR;
    }
}

static int ts_from_tt(int to, double a, double b, double *o1, double *o2, char *error)
{
    double x1, x2;
    switch (to) {
    case JME_TIME_SCALE_TT: *o1 = a; *o2 = b; return JME_OK;
    case JME_TIME_SCALE_TAI: *o1 = a; *o2 = b - 32.184 / TS_DAYSEC; return JME_OK;
    case JME_TIME_SCALE_UTC: return ts_tai_to_utc(a, b - 32.184 / TS_DAYSEC, o1, o2, error);
    case JME_TIME_SCALE_UT1: return ts_tt_to_ut1(a, b, o1, o2, error);
    case JME_TIME_SCALE_TDB: return ts_tt_to_tdb(a, b, o1, o2, error);
    case JME_TIME_SCALE_TCB:
        if (ts_tt_to_tdb(a, b, &x1, &x2, error) != JME_OK) return JME_ERR;
        if (jme_erfa_tdbtcb(x1, x2, o1, o2) != 0) { jme_set_error(error, "TDB-to-TCB conversion failed"); return JME_ERR; }
        return JME_OK;
    case JME_TIME_SCALE_TCG:
        if (jme_eraTttcg(a, b, o1, o2) != 0) { jme_set_error(error, "TT-to-TCG conversion failed"); return JME_ERR; }
        return JME_OK;
    default:
        jme_set_error(error, "Unsupported target time scale");
        return JME_ERR;
    }
}

int jme_time_scale_convert(double jd1, double jd2, int from_scale, int to_scale,
    double *out1, double *out2, char *error)
{
    double t1, t2, r1, r2;
    if (error != 0) error[0] = '\0';
    if (out1 == 0 || out2 == 0) {
        jme_set_error(error, "Two-part Julian date outputs are required");
        return JME_ERR;
    }
    *out1 = NAN;
    *out2 = NAN;
    if (!isfinite(jd1) || !isfinite(jd2)) {
        jme_set_error(error, "Julian date parts must be finite");
        return JME_ERR;
    }
    if (from_scale == to_scale) {
        if (from_scale < JME_TIME_SCALE_UTC || from_scale > JME_TIME_SCALE_TCB) {
            jme_set_error(error, "Unsupported time scale");
            return JME_ERR;
        }
        *out1 = jd1;
        *out2 = jd2;
        return JME_OK;
    }
    if (ts_to_tt(from_scale, jd1, jd2, &t1, &t2, error) != JME_OK) return JME_ERR;
    if (ts_from_tt(to_scale, t1, t2, &r1, &r2, error) != JME_OK) return JME_ERR;
    if (!isfinite(r1) || !isfinite(r2)) {
        jme_set_error(error, "Time-scale conversion produced a non-finite result");
        return JME_ERR;
    }
    *out1 = r1;
    *out2 = r2;
    if (error != 0) error[0] = '\0';
    return JME_OK;
}

const char *jme_time_scale_name(int scale)
{
    switch (scale) {
    case JME_TIME_SCALE_UTC: return "UTC";
    case JME_TIME_SCALE_UT1: return "UT1";
    case JME_TIME_SCALE_TAI: return "TAI";
    case JME_TIME_SCALE_TT: return "TT";
    case JME_TIME_SCALE_TDB: return "TDB";
    case JME_TIME_SCALE_TCG: return "TCG";
    case JME_TIME_SCALE_TCB: return "TCB";
    default: return "Unknown time scale";
    }
}
