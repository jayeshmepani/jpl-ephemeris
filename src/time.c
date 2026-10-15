#include "jme/jme.h"
#include "context.h"
#include "jme_erfa_vendor.h"

#include <math.h>

static void split_hour(double hour_value, int *hour, int *minute, double *second)
{
    int h = (int)floor(hour_value);
    double minute_value = (hour_value - (double)h) * 60.0;
    int m = (int)floor(minute_value);
    double s = (minute_value - (double)m) * 60.0;

    if (hour != 0) {
        *hour = h;
    }
    if (minute != 0) {
        *minute = m;
    }
    if (second != 0) {
        *second = s;
    }
}

int jme_utc_to_jd(
    int year,
    int month,
    int day,
    int hour,
    int minute,
    double second,
    int calendar,
    double *jd_utc
)
{
    double hour_value;

    if (jd_utc == 0) {
        return JME_ERR;
    }

    if (!jme_date_is_valid(year, month, day, calendar)) {
        return JME_ERR;
    }

    if (hour < 0 || hour > 23 || minute < 0 || minute > 59 || second < 0.0 || second >= 61.0) {
        return JME_ERR;
    }

    hour_value = jme_decimal_hour(hour, minute, second);
    *jd_utc = jme_julian_day(year, month, day, hour_value, calendar);
    return JME_OK;
}

void jme_jd_to_utc(
    double jd,
    int calendar,
    int *year,
    int *month,
    int *day,
    int *hour,
    int *minute,
    double *second
)
{
    double hour_value = 0.0;

    jme_reverse_julian_day(jd, calendar, year, month, day, &hour_value);
    split_hour(hour_value, hour, minute, second);
}

double jme_delta_t(double jd_ut)
{
    return jme_delta_t_ex(jd_ut, jme_context_deltat_model(), 0);
}

#include "deltat_data.inc"
#include "deltat_ms2004_data.inc"

/* Julian-epoch year evaluated from a two-part JD without first summing the parts. */
static double jme_julian_epoch_year_split(double jd1, double jd2)
{
    return 2000.0 + ((jd1 - 2451545.0) + jd2) / 365.25;
}

/*
 * Calendar decimal year used by Espenak & Meeus (NASA TP-2006-214141):
 * year + fraction of that calendar year, with the Julian calendar before
 * 1582-10-15 and the Gregorian calendar from then on (astronomical year
 * numbering). Keep the date in two parts through calendar decomposition and
 * compute the year fraction after subtracting the nearby January-1 epoch.
 */
static double jme_calendar_decimal_year_split(double jd1, double jd2)
{
    int calendar = ((jd1 - 2299160.5) + jd2) < 0.0
        ? JME_CALENDAR_JULIAN : JME_CALENDAR_GREGORIAN;
    int year, month, day;
    double fd, start, next, elapsed;

    /*
     * Take the year in the same calendar as the year bounds below (Julian
     * before 1582 Oct 15), so elapsed/(next-start) stays within [0, 1).
     */
    if (calendar == JME_CALENDAR_GREGORIAN) {
        if (jme_eraJd2cal(jd1, jd2, &year, &month, &day, &fd) != 0) {
            return NAN;
        }
    } else {
        if (!isfinite(jd1) || !isfinite(jd2)) return NAN;
        jme_reverse_julian_day(jd1 + jd2, JME_CALENDAR_JULIAN, &year, &month, &day, &fd);
    }
    start = jme_julian_day(year, 1, 1, 0.0, calendar);
    next = jme_julian_day(year + 1, 1, 1, 0.0, calendar);
    elapsed = (jd1 - start) + jd2;
    return (double)year + elapsed / (next - start);
}

/*
 * Espenak & Meeus (2006) piecewise polynomials, NASA TP-2006-214141, as published
 * at https://eclipse.gsfc.nasa.gov/SEhelp/deltatpoly2004.html (all digits kept).
 * No secular-acceleration correction is applied (the polynomials assume
 * n-dot = -26 arcsec/cy^2, see docs/TIME_SCALES.md).
 */
static double jme_delta_t_espenak_meeus_y(double y)
{
    double t, u;

    if (y < -500.0) {
        u = (y - 1820.0) / 100.0;
        return -20.0 + 32.0 * u * u;
    } else if (y < 500.0) {
        u = y / 100.0;
        return 10583.6 + u * (-1014.41 + u * (33.78311 + u * (-5.952053
            + u * (-0.1798452 + u * (0.022174192 + u * 0.0090316521)))));
    } else if (y < 1600.0) {
        u = (y - 1000.0) / 100.0;
        return 1574.2 + u * (-556.01 + u * (71.23472 + u * (0.319781
            + u * (-0.8503463 + u * (-0.005050998 + u * 0.0083572073)))));
    } else if (y < 1700.0) {
        t = y - 1600.0;
        return 120.0 - 0.9808 * t - 0.01532 * t * t + t * t * t / 7129.0;
    } else if (y < 1800.0) {
        t = y - 1700.0;
        return 8.83 + 0.1603 * t - 0.0059285 * t * t + 0.00013336 * t * t * t - t * t * t * t / 1174000.0;
    } else if (y < 1860.0) {
        t = y - 1800.0;
        return 13.72 + t * (-0.332447 + t * (0.0068612 + t * (0.0041116 + t * (-0.00037436
            + t * (0.0000121272 + t * (-0.0000001699 + t * 0.000000000875))))));
    } else if (y < 1900.0) {
        t = y - 1860.0;
        return 7.62 + t * (0.5737 + t * (-0.251754 + t * (0.01680668 + t * (-0.0004473624))))
            + t * t * t * t * t / 233174.0;
    } else if (y < 1920.0) {
        t = y - 1900.0;
        return -2.79 + t * (1.494119 + t * (-0.0598939 + t * (0.0061966 + t * (-0.000197))));
    } else if (y < 1941.0) {
        t = y - 1920.0;
        return 21.20 + 0.84493 * t - 0.076100 * t * t + 0.0020936 * t * t * t;
    } else if (y < 1961.0) {
        t = y - 1950.0;
        return 29.07 + 0.407 * t - t * t / 233.0 + t * t * t / 2547.0;
    } else if (y < 1986.0) {
        t = y - 1975.0;
        return 45.45 + 1.067 * t - t * t / 260.0 - t * t * t / 718.0;
    } else if (y < 2005.0) {
        t = y - 2000.0;
        return 63.86 + t * (0.3345 + t * (-0.060374 + t * (0.0017275 + t * (0.000651814 + t * 0.00002373599))));
    } else if (y < 2050.0) {
        t = y - 2000.0;
        return 62.92 + 0.32217 * t + 0.005589 * t * t;
    } else if (y < 2150.0) {
        u = (y - 1820.0) / 100.0;
        return -20.0 + 32.0 * u * u - 0.5628 * (2150.0 - y);
    }
    u = (y - 1820.0) / 100.0;
    return -20.0 + 32.0 * u * u;
}

static double jme_delta_t_espenak_meeus_split(double jd1, double jd2)
{
    jme_context *ctx = jme_get_context();
    double y = jme_calendar_decimal_year_split(jd1, jd2);
    double dt = jme_delta_t_espenak_meeus_y(y);

    /*
     * Morrison-Stephenson historical Delta-T values use a lunar secular
     * acceleration of -26 arcsec/century^2. NASA's Five Millennium Canon
     * gives the generalized correction for an alternate acceleration a:
     *
     *   c = -0.91072 * (a + 26.0) * ((y - 1955.0) / 100.0)^2 seconds.
     *
     * Apply it only before 1955. Observational values from 1955 onward are
     * referred to atomic time and are not adjusted for a lunar ephemeris.
     * The published IERS and SMH-2016/2021 paths are intentionally separate.
     */
    if (ctx->tidal_acceleration_unresolved && y < 1955.0) return NAN;
    if (ctx->tidal_acceleration_explicit && y < 1955.0) {
        double centuries = (y - 1955.0) / 100.0;
        dt += -0.91072 * (ctx->tidal_acceleration_effective + 26.0)
            * centuries * centuries;
    }

    return dt;
}

/*
 * Stephenson, Morrison & Hohenkerk (2016), Proc. R. Soc. A 472: 20160404.
 *   JME_MODEL_DELTAT_STEPHENSON_ETC_2016: the published Table S15 (54 rows,
 *   -720.0 .. 2016.0) with the supplement's rule S5.1: the row with
 *   K_i <= Y < K_i+1, equality at the upper end only for Y = 2016.0.
 *   JME_MODEL_DELTAT_STEPHENSON_ETC_2021: the 2020 revision of Table S15
 *   (Morrison, Stephenson, Hohenkerk & Zawilski 2021, Proc. R. Soc. A 477:
 *   20200776; 58 rows, -720 .. 2019), K_i <= Y < K_i+1.
 * Outside the spline range both use the published parabola (eq. 4.1 of the
 * 2016 paper) -320 + 32.5 ((y - 1825)/100)^2, which does not join the spline
 * (published model; see docs/TIME_SCALES.md).
 */
static int jme_delta_t_spline(const double (*tab)[6], int n, int upper_inclusive, double y, double *dt)
{
    int lo = 0, hi = n - 1;
    const double *c;
    double t;

    if (!(y >= tab[0][0] && (y < tab[n - 1][1] || (upper_inclusive && y == tab[n - 1][1])))) return 0;
    while (lo < hi) {
        int mid = (lo + hi + 1) / 2;
        if (tab[mid][0] <= y) lo = mid; else hi = mid - 1;
    }
    c = tab[lo];
    t = (y - c[0]) / (c[1] - c[0]);
    *dt = c[2] + t * (c[3] + t * (c[4] + t * c[5]));
    return 1;
}

/*
 * The SMH spline values were reduced with the JPL DE430 lunar ephemeris and
 * correspond to a lunar tidal acceleration of -25.82 arcsec/century^2
 * (Stephenson et al. 2016, sect. 2(a): "the value implicit in the JPL
 * ephemerides DE430 ... is -25.82"). The 2020 revision keeps that data
 * reduction. An explicitly selected acceleration corrects the pre-1955
 * domain relative to -25.82.
 */
#define JME_SMH_REFERENCE_NDOT (-25.82)

static double jme_delta_t_smh_split(double jd1, double jd2, int revision_2021)
{
    jme_context *ctx = jme_get_context();
    double y = jme_julian_epoch_year_split(jd1, jd2), dt, u;
    int ok;
    if (!isfinite(y)) return NAN;
    if (revision_2021) {
        ok = jme_delta_t_spline(jme_deltat_s15, (int)(sizeof(jme_deltat_s15) / sizeof(jme_deltat_s15[0])), 0, y, &dt);
    } else {
        ok = jme_delta_t_spline(jme_deltat_s15_2016, (int)(sizeof(jme_deltat_s15_2016) / sizeof(jme_deltat_s15_2016[0])), 1, y, &dt);
    }
    if (!ok) {
        u = (y - 1825.0) / 100.0;
        dt = -320.0 + 32.5 * u * u;
    }

    if (ctx->tidal_acceleration_unresolved && y < 1955.0) return NAN;
    if (ctx->tidal_acceleration_explicit && y < 1955.0) {
        double centuries = (y - 1955.0) / 100.0;
        dt += -0.91072 * (ctx->tidal_acceleration_effective - JME_SMH_REFERENCE_NDOT)
            * centuries * centuries;
    }

    return dt;
}

static double jme_delta_t_smh2016_split(double jd1, double jd2)
{
    return jme_delta_t_smh_split(jd1, jd2, 0);
}

static double jme_delta_t_smh2021_split(double jd1, double jd2)
{
    return jme_delta_t_smh_split(jd1, jd2, 1);
}

/*
 * JME_MODEL_DELTAT_MS2004_TABLE1_LINEAR: a reconstruction from Morrison &
 * Stephenson (2004), J. Hist. Astron. 35, 327, Table 1 (values read from
 * their cubic-spline curve, -1000 .. +2000, rounded to whole seconds). The
 * authors state (p. 331) that "intermediate figures for Delta-T can be found
 * with adequate precision by simple interpolation"; JME uses linear
 * interpolation in the calendar decimal year between adjacent rows. Outside
 * the table it uses the paper's long-term parabola -20 + 32 t^2,
 * t = (year - 1820)/100 (p. 332). This is not the authors' spline: that
 * curve is not published in tabular coefficient form, so
 * JME_MODEL_DELTAT_STEPHENSON_MORRISON_2004 remains refused.
 * Reference lunar acceleration: -26.0 arcsec/century^2 (p. 332).
 */
static double jme_delta_t_ms2004_table1_split(double jd1, double jd2)
{
    jme_context *ctx = jme_get_context();
    const int n = (int)(sizeof(jme_deltat_ms2004_table1) / sizeof(jme_deltat_ms2004_table1[0]));
    double y = jme_calendar_decimal_year_split(jd1, jd2), dt;

    if (!isfinite(y)) return NAN;
    if (y >= jme_deltat_ms2004_table1[0][0] && y <= jme_deltat_ms2004_table1[n - 1][0]) {
        int lo = 0, hi = n - 1;
        while (hi - lo > 1) {
            int mid = (lo + hi) / 2;
            if (jme_deltat_ms2004_table1[mid][0] <= y) lo = mid; else hi = mid;
        }
        dt = jme_deltat_ms2004_table1[lo][1]
            + (y - jme_deltat_ms2004_table1[lo][0])
            * (jme_deltat_ms2004_table1[hi][1] - jme_deltat_ms2004_table1[lo][1])
            / (jme_deltat_ms2004_table1[hi][0] - jme_deltat_ms2004_table1[lo][0]);
    } else {
        double t = (y - 1820.0) / 100.0;
        dt = -20.0 + 32.0 * t * t;
    }
    if (ctx->tidal_acceleration_unresolved && y < 1955.0) return NAN;
    if (ctx->tidal_acceleration_explicit && y < 1955.0) {
        double centuries = (y - 1955.0) / 100.0;
        dt += -0.91072 * (ctx->tidal_acceleration_effective + 26.0) * centuries * centuries;
    }
    return dt;
}

/*
 * IERS observed (and, after the last observed day, predicted) Delta-T from the
 * embedded finals2000A table: Delta-T = 32.184 s - (UT1 - TAI), with UT1 - TAI
 * interpolated by 4-point Lagrange interpolation in MJD, the scheme of the IERS
 * EOP interpolation routine (Bizouard/IERS interp.f) without its sub-daily
 * ocean-tide/libration terms (< 0.1 ms). The table argument is UTC; jd_ut is
 * used for it (|UT1-UTC| < 0.9 s changes Delta-T by < 2e-8 s).
 * Returns 1 inside the table, with *predicted = 1 for IERS predictions.
 */
static int jme_delta_t_iers_split(double jd1, double jd2, double *dt, int *predicted)
{
    /* Subtract the MJD epoch before adding the small part, preserving sub-ULP offsets. */
    double x = ((jd1 - 2400000.5) - (double)JME_DELTAT_IERS_FIRST_MJD) + jd2;
    int i0, k, m;
    double sum = 0.0;

    if (!(x >= 0.0 && x <= (double)(JME_DELTAT_IERS_COUNT - 1))) return 0;
    i0 = (int)floor(x) - 1;
    if (i0 < 0) i0 = 0;
    if (i0 > JME_DELTAT_IERS_COUNT - 4) i0 = JME_DELTAT_IERS_COUNT - 4;
    for (k = 0; k < 4; k++) {
        double w = 1.0;
        for (m = 0; m < 4; m++) {
            if (m != k) w *= (x - (double)(i0 + m)) / (double)(k - m);
        }
        sum += w * jme_deltat_iers_ut1_tai[i0 + k];
    }
    *dt = 32.184 - sum;
    if (predicted != 0) *predicted = x > (double)JME_DELTAT_IERS_LAST_OBSERVED_INDEX;
    return 1;
}

/*
 * JME_TIME_DELTAT_AUTOMATIC: IERS observed/predicted values where the embedded
 * table covers the date (1973-01-02 .. end of the IERS predictions), otherwise
 * Stephenson-Morrison-Hohenkerk 2021 revision (splines -720..2019, parabola before
 * -720), and Espenak-Meeus 2006 after the end of the IERS predictions.
 */
static double jme_delta_t_automatic_split(double jd1, double jd2)
{
    double dt;
    double from_iers_start = ((jd1 - 2400000.5) - (double)JME_DELTAT_IERS_FIRST_MJD) + jd2;
    if (jme_delta_t_iers_split(jd1, jd2, &dt, 0)) return dt;
    if (from_iers_start < 0.0) return jme_delta_t_smh2021_split(jd1, jd2);
    return jme_delta_t_espenak_meeus_split(jd1, jd2);
}

int jme_delta_t_iers_table_info(double *jd_first, double *jd_last_observed, double *jd_last, char *error)
{
    if (jd_first != 0) *jd_first = 2400000.5 + (double)JME_DELTAT_IERS_FIRST_MJD;
    if (jd_last_observed != 0) *jd_last_observed = 2400000.5 + (double)(JME_DELTAT_IERS_FIRST_MJD + JME_DELTAT_IERS_LAST_OBSERVED_INDEX);
    if (jd_last != 0) *jd_last = 2400000.5 + (double)(JME_DELTAT_IERS_FIRST_MJD + JME_DELTAT_IERS_COUNT - 1);
    jme_set_error(error, "");
    return JME_OK;
}

double internal_delta_t_ex_split(double jd1, double jd2, int model, char *error)
{
    jme_context *ctx = jme_get_context();
    double dt;

    if (error != 0) error[0] = '\0';
    if (!isfinite(jd1) || !isfinite(jd2)) {
        jme_set_error(error, "Two-part Julian date for Delta-T must be finite");
        return NAN;
    }

    if (ctx->delta_t_userdef_enabled) return ctx->delta_t_userdef;

    switch (model) {
    case JME_TIME_DELTAT_AUTOMATIC:
        dt = jme_delta_t_automatic_split(jd1, jd2);
        break;

    case JME_MODEL_DELTAT_ESPENAK_MEEUS_2006:
        dt = jme_delta_t_espenak_meeus_split(jd1, jd2);
        break;

    case JME_MODEL_DELTAT_STEPHENSON_ETC_2016:
        dt = jme_delta_t_smh2016_split(jd1, jd2);
        break;

    case JME_MODEL_DELTAT_STEPHENSON_ETC_2021:
        dt = jme_delta_t_smh2021_split(jd1, jd2);
        break;

    case JME_MODEL_DELTAT_MS2004_TABLE1_LINEAR:
        dt = jme_delta_t_ms2004_table1_split(jd1, jd2);
        break;

    case JME_MODEL_DELTAT_STEPHENSON_MORRISON_1984:
    case JME_MODEL_DELTAT_STEPHENSON_1997:
    case JME_MODEL_DELTAT_STEPHENSON_MORRISON_2004:
        /*
         * The defining tables and interpolation rules of these models have not
         * been transcribed from their primary publications and verified, so they
         * are refused rather than substituted by another model.
         */
        jme_set_error(error, "Delta-T model unavailable (primary tables not yet transcribed and verified); use AUTOMATIC, ESPENAK_MEEUS_2006, STEPHENSON_ETC_2016 or STEPHENSON_ETC_2021");
        return NAN;

    default:
        jme_set_error(error, "Unsupported Delta-T model");
        return NAN;
    }

    if (!isfinite(dt)) {
        if (ctx->tidal_acceleration_unresolved) {
            jme_set_error(error, "Historical Delta-T needs a lunar tidal acceleration: JPL has published no value for DE441 (Park et al. 2021); set a numeric value with jme_set_tid_acc");
            return NAN;
        }
        if (error == 0 || error[0] == '\0') jme_set_error(error, "Delta-T date conversion failed");
        return NAN;
    }
    return dt;
}

double jme_delta_t_ex(double jd_ut, int model, char *error)
{
    if (error != 0) error[0] = '\0';
    if (!isfinite(jd_ut)) {
        jme_set_error(error, "Julian date for Delta-T must be finite");
        return NAN;
    }
    {
        double jd1 = floor(jd_ut);
        double jd2 = jd_ut - jd1;
        return internal_delta_t_ex_split(jd1, jd2, model, error);
    }
}

/*
 * UTC -> TAI data and drift expressions follow the published ERFA/SOFA
 * eraDat model, adapted to JME's status/error contract. ERFA is BSD-3-Clause;
 * see THIRD_PARTY_NOTICES.md. The embedded offset table is confirmed through
 * 2026-12-31 and must be updated after any new IERS announcement.
 */
typedef struct jme_utc_offset_entry {
    int year;
    int month;
    double tai_minus_utc_at_effective_month;
} jme_utc_offset_entry;

static const jme_utc_offset_entry jme_utc_offsets[] = {
    {1960, 1, 1.4178180}, {1961, 1, 1.4228180}, {1961, 8, 1.3728180},
    {1962, 1, 1.8458580}, {1963,11, 1.9458580}, {1964, 1, 3.2401300},
    {1964, 4, 3.3401300}, {1964, 9, 3.4401300}, {1965, 1, 3.5401300},
    {1965, 3, 3.6401300}, {1965, 7, 3.7401300}, {1965, 9, 3.8401300},
    {1966, 1, 4.3131700}, {1968, 2, 4.2131700}, {1972, 1,10.0},
    {1972, 7,11.0}, {1973, 1,12.0}, {1974, 1,13.0}, {1975, 1,14.0},
    {1976, 1,15.0}, {1977, 1,16.0}, {1978, 1,17.0}, {1979, 1,18.0},
    {1980, 1,19.0}, {1981, 7,20.0}, {1982, 7,21.0}, {1983, 7,22.0},
    {1985, 7,23.0}, {1988, 1,24.0}, {1990, 1,25.0}, {1991, 1,26.0},
    {1992, 7,27.0}, {1993, 7,28.0}, {1994, 7,29.0}, {1996, 1,30.0},
    {1997, 7,31.0}, {1999, 1,32.0}, {2006, 1,33.0}, {2009, 1,34.0},
    {2012, 7,35.0}, {2015, 7,36.0}, {2017, 1,37.0}
};

typedef struct jme_utc_drift_entry {
    double reference_mjd;
    double rate_seconds_per_day;
} jme_utc_drift_entry;

static const jme_utc_drift_entry jme_utc_drifts[] = {
    {37300.0, 0.0012960}, {37300.0, 0.0012960}, {37300.0, 0.0012960},
    {37665.0, 0.0011232}, {37665.0, 0.0011232},
    {38761.0, 0.0012960}, {38761.0, 0.0012960}, {38761.0, 0.0012960},
    {38761.0, 0.0012960}, {38761.0, 0.0012960}, {38761.0, 0.0012960},
    {38761.0, 0.0012960}, {39126.0, 0.0025920}, {39126.0, 0.0025920}
};

#define JME_UTC_PRE_LEAP_DRIFT_COUNT (sizeof(jme_utc_drifts) / sizeof(jme_utc_drifts[0]))
/*
 * Validity of the embedded TAI-UTC table. IERS Bulletin C 72 (Paris, 6 July
 * 2026) announced that NO leap second will be introduced at the end of
 * December 2026, so TAI-UTC = 37 s is known through 2027-06-30 23:59:59 UTC.
 * Whether a leap second occurs at the end of June 2027 is not yet known.
 * UPDATE PROCEDURE: see docs/TIME_SCALES.md ("Leap-second table update").
 */
#define JME_UTC_VALID_UNTIL_YEAR 2027
#define JME_UTC_VALID_UNTIL_MONTH 6

int internal_tai_minus_utc(
    int gregorian_year,
    int gregorian_month,
    int gregorian_day,
    double fraction_of_utc_day,
    double *tai_minus_utc,
    char *error
)
{
    size_t index;
    int month_key;
    double mjd;

    if (tai_minus_utc == 0) {
        jme_set_error(error, "TAI-UTC output is required");
        return JME_ERR;
    }
    *tai_minus_utc = NAN;

    if (gregorian_year < 1960) {
        jme_set_error(error, "UTC-to-TT conversion before 1960-01-01 is outside the realized UTC time scale");
        return JME_ERR;
    }
    if (12 * gregorian_year + gregorian_month > 12 * JME_UTC_VALID_UNTIL_YEAR + JME_UTC_VALID_UNTIL_MONTH) {
        jme_set_error(error, "UTC after 2027-06-30 is beyond the embedded leap-second table (IERS Bulletin C 72); future leap seconds are not predictable: use TT/TDB or update the table");
        return JME_ERR;
    }
    if (gregorian_month < 1 || gregorian_month > 12
        || gregorian_day < 1
        || gregorian_day > jme_days_in_month(gregorian_year, gregorian_month, JME_CALENDAR_GREGORIAN)
        || !isfinite(fraction_of_utc_day)
        || fraction_of_utc_day < 0.0 || fraction_of_utc_day > 1.0) {
        jme_set_error(error, "UTC calendar date or day fraction is invalid");
        return JME_ERR;
    }

    month_key = 12 * gregorian_year + gregorian_month;
    index = sizeof(jme_utc_offsets) / sizeof(jme_utc_offsets[0]);
    while (index > 0) {
        const jme_utc_offset_entry *entry = &jme_utc_offsets[index - 1];
        if (month_key >= 12 * entry->year + entry->month) break;
        index--;
    }
    if (index == 0) {
        jme_set_error(error, "No TAI-UTC definition is available for this UTC date");
        return JME_ERR;
    }
    index--;

    *tai_minus_utc = jme_utc_offsets[index].tai_minus_utc_at_effective_month;
    if (index < JME_UTC_PRE_LEAP_DRIFT_COUNT) {
        double jd_midnight = jme_julian_day(
            gregorian_year, gregorian_month, gregorian_day, 0.0, JME_CALENDAR_GREGORIAN
        );
        mjd = jd_midnight - 2400000.5;
        *tai_minus_utc +=
            (mjd + fraction_of_utc_day - jme_utc_drifts[index].reference_mjd)
            * jme_utc_drifts[index].rate_seconds_per_day;
    }

    return JME_OK;
}

static int jme_is_positive_leap_second_day(double jd_midnight)
{
    size_t i;
    const size_t count = sizeof(jme_utc_offsets) / sizeof(jme_utc_offsets[0]);

    /* 1972-01-01 redefined UTC; it was not an inserted leap second. */
    for (i = JME_UTC_PRE_LEAP_DRIFT_COUNT + 1; i < count; i++) {
        const jme_utc_offset_entry *prior = &jme_utc_offsets[i - 1];
        const jme_utc_offset_entry *next = &jme_utc_offsets[i];
        double effective_midnight;

        if (fabs((next->tai_minus_utc_at_effective_month
                  - prior->tai_minus_utc_at_effective_month) - 1.0) > 1.0e-12) {
            continue;
        }
        effective_midnight = jme_julian_day(
            next->year, next->month, 1, 0.0, JME_CALENDAR_GREGORIAN
        );
        if (jd_midnight + 1.0 == effective_midnight) return 1;
    }
    return 0;
}

int jme_leap_second_table_info(double *jd_utc_valid_until, double *last_tai_minus_utc, char *error)
{
    const size_t count = sizeof(jme_utc_offsets) / sizeof(jme_utc_offsets[0]);
    if (error != 0) error[0] = '\0';
    if (jd_utc_valid_until == 0 && last_tai_minus_utc == 0) {
        jme_set_error(error, "At least one output is required");
        return JME_ERR;
    }
    if (jd_utc_valid_until != 0) {
        int y = JME_UTC_VALID_UNTIL_YEAR, m = JME_UTC_VALID_UNTIL_MONTH + 1;
        if (m > 12) { m = 1; y++; }
        /* First instant (UTC JD) NOT covered by the table. */
        *jd_utc_valid_until = jme_julian_day(y, m, 1, 0.0, JME_CALENDAR_GREGORIAN);
    }
    if (last_tai_minus_utc != 0) *last_tai_minus_utc = jme_utc_offsets[count - 1].tai_minus_utc_at_effective_month;
    return JME_OK;
}

int internal_utc_to_tt_split(
    int year,
    int month,
    int day,
    int hour,
    int minute,
    double second,
    int calendar,
    double *jd0_tt,
    double *fraction_tt,
    char *error
)
{
    double jd_midnight;
    double utc_seconds;
    double fraction_for_offset;
    double tai_minus_utc;
    double jd0;
    double fraction;
    int gy, gm, gd;

    if (error != 0) error[0] = '\0';
    if (jd0_tt == 0 || fraction_tt == 0) {
        jme_set_error(error, "TT two-part Julian date outputs are required");
        return JME_ERR;
    }
    *jd0_tt = NAN;
    *fraction_tt = NAN;

    if (calendar != JME_CALENDAR_GREGORIAN && calendar != JME_CALENDAR_JULIAN) {
        jme_set_error(error, "UTC calendar type is invalid");
        return JME_ERR;
    }
    if (!jme_date_is_valid(year, month, day, calendar)
        || hour < 0 || hour > 23 || minute < 0 || minute > 59
        || !isfinite(second) || second < 0.0 || second >= 61.0) {
        jme_set_error(error, "UTC calendar date or clock time is invalid");
        return JME_ERR;
    }

    jd_midnight = jme_julian_day(year, month, day, 0.0, calendar);
    if (!isfinite(jd_midnight)) {
        jme_set_error(error, "UTC calendar conversion produced a non-finite Julian date");
        return JME_ERR;
    }

    if (second >= 60.0) {
        if (hour != 23 || minute != 59 || !jme_is_positive_leap_second_day(jd_midnight)) {
            jme_set_error(error, "Second 60 is valid only at an announced positive UTC leap second");
            return JME_ERR;
        }
        utc_seconds = 86400.0 + (second - 60.0);
        fraction_for_offset = 1.0;
    } else {
        utc_seconds = (double)hour * 3600.0 + (double)minute * 60.0 + second;
        fraction_for_offset = utc_seconds / JME_SECONDS_PER_DAY;
    }

    jme_reverse_julian_day(
        jd_midnight, JME_CALENDAR_GREGORIAN, &gy, &gm, &gd, 0
    );
    if (internal_tai_minus_utc(
            gy, gm, gd, fraction_for_offset, &tai_minus_utc, error) != JME_OK) {
        return JME_ERR;
    }

    /*
     * UTC -> TAI adds Delta(AT); TAI -> TT adds exactly 32.184 seconds.
     * Keep the date as a two-part value so small second offsets are not added
     * directly to one large Julian-date double.
     */
    jd0 = floor(jd_midnight);
    fraction = (jd_midnight - jd0)
        + (utc_seconds + tai_minus_utc + 32.184) / JME_SECONDS_PER_DAY;
    {
        double carry = floor(fraction);
        jd0 += carry;
        fraction -= carry;
    }

    if (!isfinite(jd0) || !isfinite(fraction) || fraction < 0.0 || fraction >= 1.0) {
        jme_set_error(error, "UTC-to-TT conversion failed to produce a normalized two-part date");
        return JME_ERR;
    }
    *jd0_tt = jd0;
    *fraction_tt = fraction;
    return JME_OK;
}

int internal_ut_to_et(double jd_ut, double *jd_et, char *error)
{
    double dt;

    if (error != 0) error[0] = '\0';
    if (jd_et == 0) {
        jme_set_error(error, "Terrestrial-time output is required");
        return JME_ERR;
    }
    *jd_et = NAN;
    if (!isfinite(jd_ut)) {
        jme_set_error(error, "UT Julian date must be finite");
        return JME_ERR;
    }

    dt = jme_delta_t_ex(jd_ut, jme_context_deltat_model(), error);
    if (!isfinite(dt)) {
        if (error == 0 || error[0] == '\0') jme_set_error(error, "Delta-T calculation failed");
        return JME_ERR;
    }

    *jd_et = jd_ut + dt / JME_SECONDS_PER_DAY;
    if (!isfinite(*jd_et)) {
        jme_set_error(error, "UT-to-TT conversion produced a non-finite Julian date");
        *jd_et = NAN;
        return JME_ERR;
    }
    return JME_OK;
}

/*
 * Invert UT1 -> TT for the selected Delta-T model: find jd_ut with
 * jd_ut + DeltaT(jd_ut)/86400 = jd_tt. Delta-T varies by far less than one
 * second per day, so fixed-point iteration contracts quickly; iterate until
 * the residual is below 1e-9 s or fail.
 */
int internal_tt_to_ut(double jd_tt, double *jd_ut, char *error)
{
    double ut;
    int i;

    if (error != 0) error[0] = '\0';
    if (jd_ut == 0) {
        jme_set_error(error, "UT output is required");
        return JME_ERR;
    }
    *jd_ut = NAN;
    if (!isfinite(jd_tt)) {
        jme_set_error(error, "TT Julian date must be finite");
        return JME_ERR;
    }
    ut = jd_tt;
    for (i = 0; i < 50; i++) {
        double dt = jme_delta_t_ex(ut, jme_context_deltat_model(), error);
        double next;
        if (!isfinite(dt)) {
            if (error == 0 || error[0] == '\0') jme_set_error(error, "Delta-T calculation failed");
            return JME_ERR;
        }
        next = jd_tt - dt / JME_SECONDS_PER_DAY;
        if (fabs(next - ut) * JME_SECONDS_PER_DAY < 1.0e-9) {
            *jd_ut = next;
            return JME_OK;
        }
        ut = next;
    }
    jme_set_error(error, "TT-to-UT inversion of the Delta-T model did not converge");
    return JME_ERR;
}

int jme_time_equ(double jd_ut, double *e, char *error)
{
    double jd_et;
    double t;
    double l0;
    double sun_equ[6];
    double dpsi = 0.0;
    double deps = 0.0;
    double eps = 0.0;
    double eot_deg;

    if (e == 0) {
        jme_set_error(error, "Equation-of-time output is required");
        return JME_ERR;
    }
    *e = NAN;
    if (internal_ut_to_et(jd_ut, &jd_et, error) != JME_OK) return JME_ERR;

    t = (jd_et - 2451545.0) / 36525.0;
    l0 = 280.46646 + 36000.76983 * t + 0.0003032 * t * t;

    if (internal_calc_ut_positions(jd_ut, JME_BODY_SUN, JME_CALC_EQUATORIAL, sun_equ, error) != JME_OK) return JME_ERR;
    if (jme_get_nutation(jd_et, jme_context_nutation_model(), &dpsi, &deps, error) != JME_OK
        || jme_get_obliquity(jd_et, jme_context_obliquity_model(), &eps, error) != JME_OK) {
        return JME_ERR;
    }

    l0 = jme_degree_normalize(l0);
    eot_deg = jme_degrees_difference_signed(
        l0 - 0.0057183 - sun_equ[0] + dpsi * cos(eps * (acos(-1.0) / 180.0)),
        0.0
    );
    *e = eot_deg / 360.0;
    return JME_OK;
}

int jme_lmt_to_lat(double jd_lmt, double geo_lon, double *jd_lat, char *error)
{
    double jd_ut = jd_lmt - geo_lon / 360.0;
    double e;
    if (jd_lat == 0) {
        jme_set_error(error, "Local apparent time output is required");
        return JME_ERR;
    }
    if (jme_time_equ(jd_ut, &e, error) != JME_OK) { return JME_ERR; }
    *jd_lat = jd_lmt + e;
    return JME_OK;
}

int jme_lat_to_lmt(double jd_lat, double geo_lon, double *jd_lmt, char *error)
{
    double lmt = jd_lat;
    double e;
    int i;

    if (jd_lmt == 0) {
        jme_set_error(error, "Local mean time output is required");
        return JME_ERR;
    }

    for (i = 0; i < 8; i++) {
        double jd_ut = lmt - geo_lon / 360.0;
        double next_lmt;
        if (jme_time_equ(jd_ut, &e, error) != JME_OK) { return JME_ERR; }
        next_lmt = jd_lat - e;
        if (fabs(next_lmt - lmt) < 1.0e-12) {
            lmt = next_lmt;
            break;
        }
        lmt = next_lmt;
    }

    *jd_lmt = lmt;
    return JME_OK;
}

double jme_jd_add_seconds(double jd, double seconds)
{
    return jd + (seconds / JME_SECONDS_PER_DAY);
}

double jme_jd_difference_seconds(double jd_end, double jd_start)
{
    return (jd_end - jd_start) * JME_SECONDS_PER_DAY;
}

void jme_utc_time_zone(
    int year,
    int month,
    int day,
    int hour,
    int minute,
    double second,
    double timezone,
    int *out_year,
    int *out_month,
    int *out_day,
    int *out_hour,
    int *out_minute,
    double *out_second
)
{
    int y = year;
    int m = month;
    int d = day;
    double total_seconds;

    if (!jme_date_is_valid(year, month, day, JME_CALENDAR_GREGORIAN)
        || hour < 0 || hour > 23
        || minute < 0 || minute > 59
        || second < 0.0 || second >= 61.0) {
        if (out_year != 0) {
            *out_year = 0;
        }
        if (out_month != 0) {
            *out_month = 0;
        }
        if (out_day != 0) {
            *out_day = 0;
        }
        if (out_hour != 0) {
            *out_hour = 0;
        }
        if (out_minute != 0) {
            *out_minute = 0;
        }
        if (out_second != 0) {
            *out_second = 0.0;
        }
        return;
    }

    total_seconds = (double)(hour * 3600 + minute * 60) + second - timezone * 3600.0;

    while (total_seconds < 0.0) {
        double jd = jme_julian_day(y, m, d, 12.0, JME_CALENDAR_GREGORIAN) - 1.0;
        double ignored_hour = 0.0;
        jme_reverse_julian_day(jd, JME_CALENDAR_GREGORIAN, &y, &m, &d, &ignored_hour);
        total_seconds += 86400.0;
    }

    while (total_seconds >= 86400.0) {
        double jd = jme_julian_day(y, m, d, 12.0, JME_CALENDAR_GREGORIAN) + 1.0;
        double ignored_hour = 0.0;
        jme_reverse_julian_day(jd, JME_CALENDAR_GREGORIAN, &y, &m, &d, &ignored_hour);
        total_seconds -= 86400.0;
    }

    if (out_year != 0) {
        *out_year = y;
    }
    if (out_month != 0) {
        *out_month = m;
    }
    if (out_day != 0) {
        *out_day = d;
    }
    if (out_hour != 0) {
        *out_hour = (int)floor(total_seconds / 3600.0);
    }
    total_seconds -= floor(total_seconds / 3600.0) * 3600.0;
    if (out_minute != 0) {
        *out_minute = (int)floor(total_seconds / 60.0);
    }
    total_seconds -= floor(total_seconds / 60.0) * 60.0;
    if (out_second != 0) {
        *out_second = total_seconds;
    }
}
