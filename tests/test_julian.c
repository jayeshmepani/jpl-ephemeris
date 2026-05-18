#include "jme/jme.h"

#include <math.h>
#include <stdio.h>

static int near_equal(double a, double b)
{
    return fabs(a - b) < 1e-12;
}

int main(void)
{
    int y = 0;
    int m = 0;
    int d = 0;
    double h = 0.0;
    int hh = 0;
    int mm = 0;
    double ss = 0.0;
    double jd = jme_julian_day(2000, 1, 1, 12.0, JME_CALENDAR_GREGORIAN);

    if (!jme_date_is_valid(2000, 2, 29, JME_CALENDAR_GREGORIAN)) {
        fprintf(stderr, "Gregorian leap day rejected\n");
        return 1;
    }

    if (jme_date_is_valid(1900, 2, 29, JME_CALENDAR_GREGORIAN)) {
        fprintf(stderr, "Gregorian non-leap day accepted\n");
        return 1;
    }

    if (!jme_date_is_valid(1900, 2, 29, JME_CALENDAR_JULIAN)) {
        fprintf(stderr, "Julian leap day rejected\n");
        return 1;
    }

    if (!near_equal(jd, 2451545.0)) {
        fprintf(stderr, "J2000 JD mismatch: %.17g\n", jd);
        return 1;
    }

    if (jme_day_of_week(jd) != 6) {
        fprintf(stderr, "J2000 day-of-week mismatch: %d\n", jme_day_of_week(jd));
        return 1;
    }

    jme_reverse_julian_day(jd, JME_CALENDAR_GREGORIAN, &y, &m, &d, &h);

    if (y != 2000 || m != 1 || d != 1 || !near_equal(h, 12.0)) {
        fprintf(stderr, "reverse JD mismatch: %d-%d-%d %.17g\n", y, m, d, h);
        return 1;
    }

    if (jme_utc_to_jd(2000, 1, 1, 12, 0, 0.0, JME_CALENDAR_GREGORIAN, &jd) != JME_OK) {
        fprintf(stderr, "UTC to JD rejected valid input\n");
        return 1;
    }

    if (!near_equal(jd, 2451545.0)) {
        fprintf(stderr, "UTC to JD mismatch: %.17g\n", jd);
        return 1;
    }

    jme_jd_to_utc(jd, JME_CALENDAR_GREGORIAN, &y, &m, &d, &hh, &mm, &ss);
    if (y != 2000 || m != 1 || d != 1 || hh != 12 || mm != 0 || !near_equal(ss, 0.0)) {
        fprintf(stderr, "JD to UTC mismatch: %d-%d-%d %d:%d %.17g\n", y, m, d, hh, mm, ss);
        return 1;
    }

    return 0;
}
