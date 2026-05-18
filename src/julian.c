#include "jme/jme.h"

#include <math.h>

int jme_calendar_is_leap_year(int year, int calendar)
{
    if (calendar == JME_CALENDAR_GREGORIAN) {
        if ((year % 4) != 0) {
            return 0;
        }
        if ((year % 100) != 0) {
            return 1;
        }
        return (year % 400) == 0;
    }

    return (year % 4) == 0;
}

int jme_days_in_month(int year, int month, int calendar)
{
    static const int month_days[12] = {
        31, 28, 31, 30, 31, 30,
        31, 31, 30, 31, 30, 31
    };

    if (calendar != JME_CALENDAR_GREGORIAN && calendar != JME_CALENDAR_JULIAN) {
        return 0;
    }

    if (month < 1 || month > 12) {
        return 0;
    }

    if (month == 2 && jme_calendar_is_leap_year(year, calendar)) {
        return 29;
    }

    return month_days[month - 1];
}

int jme_date_is_valid(int year, int month, int day, int calendar)
{
    int max_day = jme_days_in_month(year, month, calendar);

    return day >= 1 && day <= max_day;
}

int jme_day_of_year(int year, int month, int day, int calendar)
{
    int current_month;
    int total = 0;

    if (!jme_date_is_valid(year, month, day, calendar)) {
        return 0;
    }

    for (current_month = 1; current_month < month; current_month++) {
        total += jme_days_in_month(year, current_month, calendar);
    }

    return total + day;
}

double jme_decimal_hour(int hour, int minute, double second)
{
    return (double)hour + ((double)minute / 60.0) + (second / 3600.0);
}

double jme_julian_day(int year, int month, int day, double hour, int calendar)
{
    int y = year;
    int m = month;
    int a;
    int b = 0;

    if (m <= 2) {
        y -= 1;
        m += 12;
    }

    if (calendar == JME_CALENDAR_GREGORIAN) {
        a = (int)floor((double)y / 100.0);
        b = 2 - a + (int)floor((double)a / 4.0);
    }

    return floor(365.25 * (double)(y + 4716))
        + floor(30.6001 * (double)(m + 1))
        + (double)day + (double)b - 1524.5 + hour / 24.0;
}

int jme_day_of_week(double jd)
{
    int dow = (int)fmod(floor(jd + 1.5), 7.0);
    if (dow < 0) {
        dow += 7;
    }
    return dow;
}

void jme_reverse_julian_day(
    double jd,
    int calendar,
    int *year,
    int *month,
    int *day,
    double *hour
)
{
    double z;
    double f;
    double a;
    double alpha;
    double b;
    double c;
    double d;
    double e;
    double exact_day;
    int mon;
    int yr;

    jd += 0.5;
    z = floor(jd);
    f = jd - z;

    a = z;
    if (calendar == JME_CALENDAR_GREGORIAN) {
        alpha = floor((z - 1867216.25) / 36524.25);
        a = z + 1.0 + alpha - floor(alpha / 4.0);
    }

    b = a + 1524.0;
    c = floor((b - 122.1) / 365.25);
    d = floor(365.25 * c);
    e = floor((b - d) / 30.6001);
    exact_day = b - d - floor(30.6001 * e) + f;

    mon = (e < 14.0) ? (int)(e - 1.0) : (int)(e - 13.0);
    yr = (mon > 2) ? (int)(c - 4716.0) : (int)(c - 4715.0);

    if (year != 0) {
        *year = yr;
    }
    if (month != 0) {
        *month = mon;
    }
    if (day != 0) {
        *day = (int)floor(exact_day);
    }
    if (hour != 0) {
        *hour = (exact_day - floor(exact_day)) * 24.0;
    }
}
