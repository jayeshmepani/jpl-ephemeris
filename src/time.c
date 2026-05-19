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
    return jme_delta_t_ex(jd_ut, JME_TIME_DELTAT_AUTOMATIC, 0);
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
    (void)error;

    if (ctx->delta_t_userdef_enabled) {
        return ctx->delta_t_userdef;
    }

    switch (model) {
    case JME_TIME_DELTAT_AUTOMATIC:
    case JME_MODEL_DELTAT_ESPENAK_MEEUS_2006:
        return jme_delta_t_espenak_meeus(jd_ut);
    default:
        /* Fallback to the default model for unsupported model identifiers. */
        return jme_delta_t_espenak_meeus(jd_ut);
    }
}

int jme_time_equ(double jd_ut, double *e, char *error)
{
    double t = (jd_ut - 2451545.0) / 36525.0;
    double l0 = 280.46646 + 36000.76983 * t + 0.0003032 * t * t;
    double state[6];
    double spherical[6];
    double ra, eot_deg;

    if (e != 0) {
        *e = 0.0;
    }

    if (jme_jpl_body_state(jd_ut, JME_BODY_SUN, JME_BODY_EARTH, JME_VECTOR_AU_PER_DAY, state, error) != JME_OK) {
        return JME_ERR;
    }

    if (jme_rectangular_to_spherical_state(state, spherical) != JME_OK) {
        jme_set_error(error, "Sun state conversion failed");
        return JME_ERR;
    }

    /* spherical[0] is RA in degrees if state was equatorial */
    ra = spherical[0];
    l0 = jme_degree_normalize(l0);

    /* E = L - RA (simplified, ignoring nutation/aberration for basic EOT) */
    /* Higher precision requires apparent RA which includes nutation and aberration */
    eot_deg = jme_degrees_difference_signed(l0, ra);

    if (e != 0) {
        *e = eot_deg / 360.0; /* as fraction of a day */
    }

    return JME_OK;
}

int jme_lmt_to_lat(double jd_lmt, double geo_lon, double *jd_lat, char *error)
{
    double jd_ut = jd_lmt - geo_lon / 360.0;
    double e;
    if (jme_time_equ(jd_ut, &e, error) != JME_OK) { return JME_ERR; }
    if (jd_lat != 0) { *jd_lat = jd_lmt + e; }
    return JME_OK;
}

int jme_lat_to_lmt(double jd_lat, double geo_lon, double *jd_lmt, char *error)
{
    double jd_ut = jd_lat - geo_lon / 360.0; /* Approx UT */
    double e;
    if (jme_time_equ(jd_ut, &e, error) != JME_OK) { return JME_ERR; }
    if (jd_lmt != 0) { *jd_lmt = jd_lat - e; }
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
