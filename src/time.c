#include "jme/jme.h"
#include "context.h"

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

static double jme_delta_t_espenak_meeus(double jd_ut)
{
    int year, month, day, hour, minute;
    double second;
    double y, t, u;
    double dt = 0.0;

    jme_jd_to_utc(jd_ut, JME_CALENDAR_GREGORIAN, &year, &month, &day, &hour, &minute, &second);

    /* NASA polynomial fit uses y = year + (month - 0.5) / 12 */
    y = (double)year + ((double)month - 0.5) / 12.0;

    if (year < -500) {
        u = (y - 1820.0) / 100.0;
        dt = -20.0 + 32.0 * u * u;
    } else if (year < 500) {
        u = y / 100.0;
        dt = 10583.6 - 1014.41 * u + 33.78311 * u * u - 5.952053 * u * u * u
             - 0.1798452 * u * u * u * u + 0.022174192 * u * u * u * u * u
             + 0.0090316521 * u * u * u * u * u * u;
    } else if (year < 1600) {
        u = (y - 1000.0) / 100.0;
        dt = 1574.2 - 556.01 * u + 71.23472 * u * u + 0.319781 * u * u * u
             - 0.8503463 * u * u * u * u - 0.005050998 * u * u * u * u * u
             + 0.0083572073 * u * u * u * u * u * u;
    } else if (year < 1700) {
        t = y - 1600.0;
        dt = 120.0 - 0.9808 * t - 0.01532 * t * t + t * t * t / 7129.0;
    } else if (year < 1800) {
        t = y - 1700.0;
        dt = 8.83 + 0.1603 * t - 0.0059285 * t * t + 0.00013336 * t * t * t - t * t * t * t / 1174000.0;
    } else if (year < 1860) {
        t = y - 1800.0;
        dt = 13.72 - 0.332447 * t + 0.0068612 * t * t + 0.0041116 * t * t * t - 0.00037436 * t * t * t * t
             + 0.0000121272 * t * t * t * t * t - 0.0000001699 * t * t * t * t * t * t
             + 0.000000000875 * t * t * t * t * t * t * t;
    } else if (year < 1900) {
        t = y - 1860.0;
        dt = 7.62 + 0.5737 * t - 0.251754 * t * t + 0.0168066 * t * t * t - 0.000447362 * t * t * t * t
             + t * t * t * t * t / 233174.0;
    } else if (year < 1920) {
        t = y - 1900.0;
        dt = -2.73 + 0.1273 * t - 0.0055 * t * t;
    } else if (year < 1941) {
        t = y - 1920.0;
        dt = 21.20 + 0.84493 * t - 0.076100 * t * t + 0.0020936 * t * t * t;
    } else if (year < 1961) {
        t = y - 1950.0;
        dt = 29.07 + 0.407 * t - t * t / 233.0 + t * t * t / 2547.0;
    } else if (year < 1986) {
        t = y - 1975.0;
        dt = 45.45 + 1.067 * t - t * t / 260.0 - t * t * t / 718.0;
    } else if (year < 2005) {
        t = y - 2000.0;
        dt = 63.86 + 0.3345 * t - 0.060374 * t * t + 0.0017275 * t * t * t + 0.000651814 * t * t * t * t
             + 0.00002373599 * t * t * t * t * t;
    } else if (year < 2050) {
        t = y - 2000.0;
        dt = 62.92 + 0.32217 * t + 0.005589 * t * t;
    } else if (year < 2150) {
        u = (y - 1820.0) / 100.0;
        dt = -20.0 + 32.0 * u * u - 0.5628 * (2150.0 - y);
    } else {
        u = (y - 1820.0) / 100.0;
        dt = -20.0 + 32.0 * u * u;
    }

    return dt;
}

double jme_delta_t_ex(double jd_ut, int model, char *error)
{
    jme_context *ctx = jme_get_context();

    if (error != 0) {
        error[0] = '\0';
    }

    if (!isfinite(jd_ut)) {
        jme_set_error(error, "Julian date for Delta-T must be finite");
        return NAN;
    }

    if (ctx->delta_t_userdef_enabled) {
        return ctx->delta_t_userdef;
    }

    switch (model) {
    case JME_TIME_DELTAT_AUTOMATIC:
    case JME_MODEL_DELTAT_ESPENAK_MEEUS_2006:
        return jme_delta_t_espenak_meeus(jd_ut);

    case JME_MODEL_DELTAT_STEPHENSON_MORRISON_1984:
    case JME_MODEL_DELTAT_STEPHENSON_1997:
    case JME_MODEL_DELTAT_STEPHENSON_MORRISON_2004:
    case JME_MODEL_DELTAT_STEPHENSON_ETC_2016:
        /*
         * These model identifiers are public, but their model-specific
         * historical tables and transition rules have no validated implementation.
         * Returning the Espenak-Meeus polynomial under a different model
         * name would silently return the wrong requested model. Fail closed
         * until each model can be implemented and validated independently.
         */
        jme_set_error(error, "no validated implementation exists for the requested Delta-T model; refusing to substitute Espenak-Meeus 2006");
        return NAN;

    default:
        jme_set_error(error, "Unsupported Delta-T model");
        return NAN;
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

    if (jme_calc_ut(jd_ut, JME_BODY_SUN, JME_CALC_EQUATORIAL, sun_equ, error) != JME_OK) return JME_ERR;
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
