/*
 * Astrometric model functions for JME.
 *
 * Nutation (IAU 1980, IAU 2000A, IAU 2000B), obliquity (IAU 1980, IAU 2000,
 * IAU 2006), precession (IAU 1976, IAU 2000, IAU 2006, Vondrak et al. 2011),
 * frame bias, sidereal time and equation of the equinoxes are evaluated with
 * the ERFA routines vendored (unmodified apart from a jme_ symbol prefix) in
 * src/erfa/vendor/. ERFA is BSD-3-Clause, Copyright (C) 2013-2023 NumFOCUS
 * Foundation, derived with permission from IAU SOFA; the full licence text is
 * in src/erfa/vendor/LICENSE.ERFA and THIRD_PARTY_NOTICES.md. The rest of this
 * file is JME project code under the MIT License.
 */

#include "jme/jme.h"
#include "context.h"
#include "jme_erfa_vendor.h"
#include "erfam.h"
#include <math.h>
#include <string.h>

#define JME_ARCSEC_TO_RAD 4.848136811095359935899141023221522846734e-6
#define JME_DEG_TO_RAD 0.017453292519943295769236907684886127134428718885417
#define JME_RAD_TO_DEG 57.295779513082320876798154814105170332405472466564

/* IAU 2000 precession-rate corrections (Mathews, Herring & Buffett 2002;
 * IERS Conventions 2003, eq. 5.?; as used by ERFA eraPr00), arcsec/century. */
#define JME_PR00_DEPSPR (-0.02524)

static void matrix_from_erfa(double r[3][3], double *m)
{
    memcpy(m, &r[0][0], 9 * sizeof(double));
}

static void matrix_transpose(const double *a, double *out)
{
    double t[9];
    int i, j;
    for (i = 0; i < 3; i++) for (j = 0; j < 3; j++) t[i * 3 + j] = a[j * 3 + i];
    memcpy(out, t, sizeof(t));
}

static int nutation_raw(double jd_et, int model, double *dpsi_rad, double *deps_rad, char *error)
{
    double d1 = floor(jd_et);
    double d2 = jd_et - d1;

    switch (model) {
    case JME_MODEL_NUT_IAU_1980:
        jme_eraNut80(d1, d2, dpsi_rad, deps_rad);
        return JME_OK;
    case JME_MODEL_NUT_IAU_2000A:
        jme_eraNut00a(d1, d2, dpsi_rad, deps_rad);
        return JME_OK;
    case JME_MODEL_NUT_IAU_2000B:
        jme_eraNut00b(d1, d2, dpsi_rad, deps_rad);
        return JME_OK;
    default:
        jme_set_error(error, "Unsupported nutation model");
        return JME_ERR;
    }
}

int internal_get_nutation_rad(double jd_et, int model, int interpolate, double *dpsi, double *deps, char *error)
{
    if (dpsi == 0 || deps == 0) {
        jme_set_error(error, "Nutation output pointers are required");
        return JME_ERR;
    }
    *dpsi = NAN;
    *deps = NAN;
    if (!isfinite(jd_et)) {
        jme_set_error(error, "Nutation Julian date must be finite");
        return JME_ERR;
    }

    if (interpolate) {
        /*
         * Optional speed/consistency mode: linear interpolation between
         * nutation values at consecutive integer Julian dates (12h TT). This is a documented
         * approximation (error below ~1 mas) that callers opt into explicitly
         * with jme_set_interpolate_nut(1).
         */
        double lower_jd = floor(jd_et);
        double weight = jd_et - lower_jd;
        double dp0, de0, dp1, de1;
        if (nutation_raw(lower_jd, model, &dp0, &de0, error) != JME_OK
            || nutation_raw(lower_jd + 1.0, model, &dp1, &de1, error) != JME_OK) {
            return JME_ERR;
        }
        *dpsi = dp0 + (dp1 - dp0) * weight;
        *deps = de0 + (de1 - de0) * weight;
        return JME_OK;
    }

    return nutation_raw(jd_et, model, dpsi, deps, error);
}

int jme_get_nutation(double jd_et, int model, double *dpsi, double *deps, char *error)
{
    double dp, de;

    if (dpsi != 0) { *dpsi = NAN; }
    if (deps != 0) { *deps = NAN; }
    if (dpsi == 0 || deps == 0) {
        jme_set_error(error, "Nutation output pointers are required");
        return JME_ERR;
    }
    if (internal_get_nutation_rad(jd_et, model, jme_context_interpolate_nut(), &dp, &de, error) != JME_OK) {
        return JME_ERR;
    }
    *dpsi = dp * JME_RAD_TO_DEG;
    *deps = de * JME_RAD_TO_DEG;
    return JME_OK;
}

int jme_get_obliquity(double jd_et, int model, double *eps, char *error)
{
    double d1, d2, t;

    if (eps != 0) { *eps = NAN; }
    if (eps == 0) {
        jme_set_error(error, "Obliquity output is required");
        return JME_ERR;
    }
    if (!isfinite(jd_et)) {
        jme_set_error(error, "Obliquity Julian date must be finite");
        return JME_ERR;
    }
    d1 = floor(jd_et);
    d2 = jd_et - d1;
    t = ((d1 - 2451545.0) + d2) / 36525.0;

    switch (model) {
    case JME_MODEL_OBL_IAU_1980:
        /* Lieske et al. (1977) / IAU 1980 mean obliquity. */
        *eps = jme_eraObl80(d1, d2) * JME_RAD_TO_DEG;
        return JME_OK;
    case JME_MODEL_OBL_IAU_2000:
        /* IAU 2000: IAU 1980 mean obliquity plus the IAU 2000 precession-rate
         * correction in obliquity (-0.02524"/century, Mathews et al. 2002). */
        *eps = (jme_eraObl80(d1, d2) + JME_PR00_DEPSPR * t * JME_ARCSEC_TO_RAD) * JME_RAD_TO_DEG;
        return JME_OK;
    case JME_MODEL_OBL_IAU_2006:
        /* Hilton et al. (2006) / IAU 2006 P03 mean obliquity. */
        *eps = jme_eraObl06(d1, d2) * JME_RAD_TO_DEG;
        return JME_OK;
    default:
        jme_set_error(error, "Unsupported obliquity model");
        return JME_ERR;
    }
}

/*
 * Precession matrix from the mean equator and equinox of J2000.0 to the
 * mean equator and equinox of jd (TT), without frame bias.
 */
static int precession_from_j2000(double jd, int model, double *m)
{
    double d1 = floor(jd);
    double d2 = jd - d1;
    double rb[3][3], rp[3][3], rbp[3][3];

    switch (model) {
    case JME_MODEL_PREC_IAU_1976:
        jme_eraPmat76(d1, d2, rp);
        break;
    case JME_MODEL_PREC_IAU_2000:
        jme_eraBp00(d1, d2, rb, rp, rbp);
        break;
    case JME_MODEL_PREC_IAU_2006:
        jme_eraBp06(d1, d2, rb, rp, rbp);
        break;
    case JME_MODEL_PREC_VONDRAK_2011:
        /* Vondrak, Capitaine & Wallace (2011, 2012), A&A 534, A22 and
         * A&A 541, C1; valid for +/-200 000 years around J2000. */
        jme_eraLtp(2000.0 + ((d1 - 2451545.0) + d2) / 365.25, rp);
        break;
    default:
        return JME_ERR;
    }
    matrix_from_erfa(rp, m);
    return JME_OK;
}

int jme_get_precession_matrix(double jd_start, double jd_end, int model, double *m)
{
    double p_start[9];
    double p_end[9];

    if (m == 0) {
        return JME_ERR;
    }
    jme_matrix_identity(m);
    if (!isfinite(jd_start) || !isfinite(jd_end)) {
        return JME_ERR;
    }

    if (model == JME_MODEL_PREC_IAU_1976) {
        /* Lieske et al. (1977) angles are defined for an arbitrary start
         * epoch, so evaluate them directly between the two epochs. */
        double s1 = floor(jd_start), s2 = jd_start - s1;
        double e1 = floor(jd_end), e2 = jd_end - e1;
        double zeta, z, theta;
        double r[3][3];
        jme_eraPrec76(s1, s2, e1, e2, &zeta, &z, &theta);
        jme_eraIr(r);
        jme_eraRz(-zeta, r);
        jme_eraRy(theta, r);
        jme_eraRz(-z, r);
        matrix_from_erfa(r, m);
        return JME_OK;
    }

    if (precession_from_j2000(jd_start, model, p_start) != JME_OK
        || precession_from_j2000(jd_end, model, p_end) != JME_OK) {
        jme_matrix_identity(m);
        return JME_ERR;
    }
    matrix_transpose(p_start, p_start);
    jme_matrix_multiply(p_end, p_start, m);
    return JME_OK;
}

int jme_get_frame_bias_matrix(int model, double *m)
{
    double rb[3][3], rp[3][3], rbp[3][3];

    if (m == 0) {
        return JME_ERR;
    }
    jme_matrix_identity(m);

    switch (model) {
    case JME_MODEL_BIAS_NONE:
        return JME_OK;
    case JME_MODEL_BIAS_IAU2000:
        /* IAU 2000 frame bias (Chapront et al. 2002 / IERS 2003): ERFA eraBi00. */
        jme_eraBp00(2451545.0, 0.0, rb, rp, rbp);
        matrix_from_erfa(rb, m);
        return JME_OK;
    case JME_MODEL_BIAS_IAU2006:
        /* IAU 2006 Fukushima-Williams frame bias at J2000 (ERFA eraBp06). */
        jme_eraBp06(2451545.0, 0.0, rb, rp, rbp);
        matrix_from_erfa(rb, m);
        return JME_OK;
    default:
        return JME_ERR;
    }
}

void jme_get_nutation_matrix(double dpsi_rad, double deps_rad, double eps_rad, double *m)
{
    jme_matrix_identity(m);
    jme_matrix_rotate_x(eps_rad, m);
    jme_matrix_rotate_z(-dpsi_rad, m);
    jme_matrix_rotate_x(-(eps_rad + deps_rad), m);
}

/*
 * Mean-of-date <- J2000 mean (+ optional bias) <- input frame, and the true
 * equator/equinox of date nutation matrix, using the context models. Every
 * model call is checked; an unsupported model yields JME_ERR.
 */
int internal_bias_precession_matrix(double jd_et, double *bp, char *error)
{
    double bias[9];
    double prec[9];

    if (jme_get_frame_bias_matrix(jme_context_bias_model(), bias) != JME_OK) {
        jme_set_error(error, "Unsupported frame-bias model");
        return JME_ERR;
    }
    if (jme_get_precession_matrix(2451545.0, jd_et, jme_context_precession_model(), prec) != JME_OK) {
        jme_set_error(error, "Unsupported precession model (Laskar 1986 precession is not implemented)");
        return JME_ERR;
    }
    jme_matrix_multiply(prec, bias, bp);
    return JME_OK;
}

int internal_nutation_matrix(double jd_et, double *nm, double *dpsi_deg, double *eps_mean_deg, double *deps_deg, char *error)
{
    double dpsi, deps, eps;

    if (jme_get_obliquity(jd_et, jme_context_obliquity_model(), &eps, error) != JME_OK
        || jme_get_nutation(jd_et, jme_context_nutation_model(), &dpsi, &deps, error) != JME_OK) {
        return JME_ERR;
    }
    if (nm != 0) {
        jme_get_nutation_matrix(dpsi * JME_DEG_TO_RAD, deps * JME_DEG_TO_RAD, eps * JME_DEG_TO_RAD, nm);
    }
    if (dpsi_deg != 0) *dpsi_deg = dpsi;
    if (deps_deg != 0) *deps_deg = deps;
    if (eps_mean_deg != 0) *eps_mean_deg = eps;
    return JME_OK;
}

/*
 * Greenwich sidereal time in radians.
 *   JME_MODEL_SIDT_IAU_1976: GMST = IAU 1982 (Aoki et al. 1982), ERFA eraGmst82,
 *                            function of UT1 only.
 *   JME_MODEL_SIDT_IAU_2006: GMST = ERA(UT1) + IAU 2006 polynomial(TT),
 *                            ERFA eraGmst06.
 * Apparent sidereal time adds the equation of the equinoxes:
 *   dpsi(context nutation) * cos(mean obliquity(context)) + complementary terms
 *   (IAU 1994 terms, eraEqeq94 convention, for SIDT_IAU_1976; IERS 2003
 *   eraEect00 terms for SIDT_IAU_2006).
 */
int internal_sidereal_time_rad(double jd_ut, double jd_tt, int apparent, double *gst, char *error)
{
    double u1, u2, t1, t2, value;

    if (gst == 0) {
        jme_set_error(error, "Sidereal-time output is required");
        return JME_ERR;
    }
    *gst = NAN;
    if (!isfinite(jd_ut) || !isfinite(jd_tt)) {
        jme_set_error(error, "Sidereal time requires finite UT1 and TT dates");
        return JME_ERR;
    }
    u1 = floor(jd_ut); u2 = jd_ut - u1;
    t1 = floor(jd_tt); t2 = jd_tt - t1;

    switch (jme_context_sidereal_time_model()) {
    case JME_MODEL_SIDT_IAU_1976:
        value = jme_eraGmst82(u1, u2);
        break;
    case JME_MODEL_SIDT_IAU_2006:
        value = jme_eraGmst06(u1, u2, t1, t2);
        break;
    default:
        jme_set_error(error, "Unsupported sidereal-time model");
        return JME_ERR;
    }

    if (apparent) {
        double dpsi, deps, eps, om, ct;
        if (jme_get_obliquity(jd_tt, jme_context_obliquity_model(), &eps, error) != JME_OK
            || jme_get_nutation(jd_tt, jme_context_nutation_model(), &dpsi, &deps, error) != JME_OK) {
            return JME_ERR;
        }
        if (jme_context_sidereal_time_model() == JME_MODEL_SIDT_IAU_1976) {
            double t = ((t1 - 2451545.0) + t2) / 36525.0;
            om = jme_eraAnpm((450160.280 + (-482890.539 + (7.455 + 0.008 * t) * t) * t) * JME_ARCSEC_TO_RAD
                + fmod(-5.0 * t, 1.0) * ERFA_D2PI);
            ct = (0.00264 * sin(om) + 0.000063 * sin(om + om)) * JME_ARCSEC_TO_RAD;
        } else {
            ct = jme_eraEect00(t1, t2);
        }
        value += dpsi * JME_DEG_TO_RAD * cos(eps * JME_DEG_TO_RAD) + ct;
    }
    *gst = jme_eraAnp(value);
    return JME_OK;
}

/*
 * Observer geocentric position/velocity on the WGS84 ellipsoid, rotated by
 * Greenwich apparent sidereal time (computed from UT1 obtained by inverting
 * the selected Delta-T model) into the true equator and equinox of date.
 * Polar motion is not applied (documented; < 15 m).
 */
int internal_topo_state_true_equator(double jd_et, double *pos_au, double *vel_au_per_day, char *error)
{
    jme_context *ctx = jme_get_context();
    double jd_ut, gast, lst_rad;
    double cos_phi, sin_phi;
    const double a = 6378.137; /* WGS84 semi-major axis in km */
    const double f = 1.0 / 298.257223563; /* WGS84 flattening */
    /* Earth rotation rate relative to the true equinox, rad per UT1 day. */
    const double omega = ERFA_D2PI * 1.00273781191135448;
    double c, s, rxy, z_km;

    if (pos_au == 0) {
        jme_set_error(error, "Topocentric position output is required");
        return JME_ERR;
    }
    if (!isfinite(ctx->topo_lon) || !isfinite(ctx->topo_lat) || !isfinite(ctx->topo_alt)) {
        jme_set_error(error, "Topocentric observer location is invalid");
        return JME_ERR;
    }
    if (internal_tt_to_ut(jd_et, &jd_ut, error) != JME_OK) {
        return JME_ERR;
    }
    if (internal_sidereal_time_rad(jd_ut, jd_et, 1, &gast, error) != JME_OK) {
        return JME_ERR;
    }
    lst_rad = gast + ctx->topo_lon * JME_DEG_TO_RAD;

    cos_phi = cos(ctx->topo_lat * JME_DEG_TO_RAD);
    sin_phi = sin(ctx->topo_lat * JME_DEG_TO_RAD);
    c = 1.0 / sqrt(cos_phi * cos_phi + (1.0 - f) * (1.0 - f) * sin_phi * sin_phi);
    s = (1.0 - f) * (1.0 - f) * c;

    rxy = (a * c + ctx->topo_alt * 0.001) * cos_phi;
    z_km = (a * s + ctx->topo_alt * 0.001) * sin_phi;

    pos_au[0] = rxy * cos(lst_rad) / JME_AU_KM;
    pos_au[1] = rxy * sin(lst_rad) / JME_AU_KM;
    pos_au[2] = z_km / JME_AU_KM;
    if (vel_au_per_day != 0) {
        vel_au_per_day[0] = -omega * pos_au[1];
        vel_au_per_day[1] = omega * pos_au[0];
        vel_au_per_day[2] = 0.0;
    }
    return JME_OK;
}

int jme_get_topo_pos_true_equator(double jd_et, double *pos_au, char *error)
{
    return internal_topo_state_true_equator(jd_et, pos_au, 0, error);
}

/* Observer offset from the geocentre in the J2000 mean frame (bias removed when a bias model is selected). */
int internal_topo_state_j2000(double jd_et, double *state_au, char *error)
{
    double pos[3], vel[3];
    double bp[9], nm[9], full[9], inv[9];
    double st[6];
    int i;

    if (state_au == 0) {
        jme_set_error(error, "Topocentric state output is required");
        return JME_ERR;
    }
    for (i = 0; i < 6; i++) state_au[i] = NAN;
    if (internal_topo_state_true_equator(jd_et, pos, vel, error) != JME_OK
        || internal_bias_precession_matrix(jd_et, bp, error) != JME_OK
        || internal_nutation_matrix(jd_et, nm, 0, 0, 0, error) != JME_OK) {
        return JME_ERR;
    }
    jme_matrix_multiply(nm, bp, full);
    matrix_transpose(full, inv);
    for (i = 0; i < 3; i++) { st[i] = pos[i]; st[i + 3] = vel[i]; }
    jme_matrix_transform_state(inv, st, state_au);
    return JME_OK;
}

int jme_get_topo_pos(double jd_et, double *pos_au, char *error)
{
    double st[6];

    if (pos_au == 0) {
        jme_set_error(error, "Topocentric position output is required");
        return JME_ERR;
    }
    if (internal_topo_state_j2000(jd_et, st, error) != JME_OK) {
        return JME_ERR;
    }
    pos_au[0] = st[0];
    pos_au[1] = st[1];
    pos_au[2] = st[2];
    return JME_OK;
}
