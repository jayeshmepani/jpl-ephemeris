#include "jme/jme.h"

#include <math.h>
#include <stdio.h>
#include <stdlib.h>

#define JME_DEG360 360.0
#define JME_RAD2PI 6.2831853071795864769252867665590057683943387987502
#define JME_DEG_TO_RAD 0.017453292519943295769236907684886127134428718885417
#define JME_RAD_TO_DEG 57.295779513082320876798154814105170332405472466565
#define JME_CSEC_FULL_CIRCLE 129600000
#define JME_CSEC_HALF_CIRCLE 64800000

double jme_degree_normalize(double x)
{
    double r = fmod(x, JME_DEG360);
    if (r < 0.0) {
        r += JME_DEG360;
    }
    return r;
}

double jme_radian_normalize(double x)
{
    double r = fmod(x, JME_RAD2PI);
    if (r < 0.0) {
        r += JME_RAD2PI;
    }
    return r;
}

double jme_degrees_to_radians(double degrees)
{
    return degrees * JME_DEG_TO_RAD;
}

double jme_radians_to_degrees(double radians)
{
    return radians * JME_RAD_TO_DEG;
}

double jme_hours_normalize(double hours)
{
    double r = fmod(hours, 24.0);
    if (r < 0.0) {
        r += 24.0;
    }
    return r;
}

double jme_degrees_to_hours(double degrees)
{
    return degrees / 15.0;
}

double jme_hours_to_degrees(double hours)
{
    return hours * 15.0;
}

double jme_degrees_difference(double p1, double p2)
{
    return jme_degree_normalize(p1 - p2);
}

double jme_degrees_difference_signed(double p1, double p2)
{
    double d = jme_degrees_difference(p1, p2);
    if (d >= 180.0) {
        d -= 360.0;
    }
    return d;
}

double jme_radians_difference_signed(double p1, double p2)
{
    double d = jme_radian_normalize(p1 - p2);
    if (d >= (JME_RAD2PI / 2.0)) {
        d -= JME_RAD2PI;
    }
    return d;
}

double jme_degree_midpoint(double x1, double x0)
{
    return jme_degree_normalize(x0 + jme_degrees_difference_signed(x1, x0) / 2.0);
}

double jme_radian_midpoint(double x1, double x0)
{
    return jme_radian_normalize(x0 + jme_radians_difference_signed(x1, x0) / 2.0);
}

int jme_centiseconds_normalize(int p)
{
    int r = p % JME_CSEC_FULL_CIRCLE;
    if (r < 0) {
        r += JME_CSEC_FULL_CIRCLE;
    }
    return r;
}

int jme_centiseconds_difference(int p1, int p2)
{
    return jme_centiseconds_normalize(p1 - p2);
}

int jme_centiseconds_difference_signed(int p1, int p2)
{
    int d = jme_centiseconds_difference(p1, p2);
    if (d >= JME_CSEC_HALF_CIRCLE) {
        d -= JME_CSEC_FULL_CIRCLE;
    }
    return d;
}

int jme_centiseconds_round_second(int x)
{
    if (x >= 0) {
        return ((x + 50) / 100) * 100;
    }
    return ((x - 50) / 100) * 100;
}

int jme_double_to_long(double x)
{
    if (x >= 0.0) {
        return (int)floor(x + 0.5);
    }
    return (int)ceil(x - 0.5);
}

void jme_split_degree(
    double ddeg,
    int roundflag,
    int *ideg,
    int *imin,
    int *isec,
    double *dsecfr,
    int *isgn
)
{
    double absdeg = fabs(ddeg);
    double minutes_total;
    double seconds_total;
    int deg;
    int min;
    int sec;

    if ((roundflag & JME_ANGLE_FORMAT_ROUND_DEG) != 0) {
        absdeg = floor(absdeg + 0.5);
    } else if ((roundflag & JME_ANGLE_FORMAT_ROUND_MIN) != 0) {
        absdeg = floor(absdeg * 60.0 + 0.5) / 60.0;
    } else if ((roundflag & JME_ANGLE_FORMAT_ROUND_SEC) != 0) {
        absdeg = floor(absdeg * 3600.0 + 0.5) / 3600.0;
    }

    deg = (int)floor(absdeg);
    minutes_total = (absdeg - (double)deg) * 60.0;
    min = (int)floor(minutes_total);
    seconds_total = (minutes_total - (double)min) * 60.0;
    sec = (int)floor(seconds_total);

    if (ideg != 0) {
        *ideg = deg;
    }
    if (imin != 0) {
        *imin = min;
    }
    if (isec != 0) {
        *isec = sec;
    }
    if (dsecfr != 0) {
        *dsecfr = seconds_total - (double)sec;
    }
    if (isgn != 0) {
        *isgn = ddeg < 0.0 ? -1 : 1;
    }
}

char *jme_centiseconds_to_time_string(int cs, char *buffer)
{
    int h, m, s, c;
    h = cs / 360000;
    m = (cs % 360000) / 6000;
    s = (cs % 6000) / 100;
    c = cs % 100;
    sprintf(buffer, "%02d:%02d:%02d.%02d", h, m, s, c);
    return buffer;
}

char *jme_centiseconds_to_lonlat_string(int cs, char *buffer)
{
    int d, m, s, c;
    char sign = (cs >= 0) ? '+' : '-';
    int acs = abs(cs);
    d = acs / 360000;
    m = (acs % 360000) / 6000;
    s = (acs % 6000) / 100;
    c = acs % 100;
    sprintf(buffer, "%c%03d°%02d'%02d.%02d\"", sign, d, m, s, c);
    return buffer;
}

char *jme_centiseconds_to_degree_string(int cs, char *buffer)
{
    return jme_centiseconds_to_lonlat_string(cs, buffer);
}
