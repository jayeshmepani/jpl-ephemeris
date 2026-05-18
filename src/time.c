#include "jme/jme.h"

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
