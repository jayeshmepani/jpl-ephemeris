#include "jme/jme.h"

#include <math.h>

#define JME_DEG_TO_RAD 0.017453292519943295769236907684886127134428718885417

double jme_sidereal_time0(double jd_ut, double eps, double nut)
{
    double t = (jd_ut - 2451545.0) / 36525.0;
    double theta = 280.46061837
        + 360.98564736629 * (jd_ut - 2451545.0)
        + 0.000387933 * t * t
        - (t * t * t) / 38710000.0;

    theta += nut * cos(eps * JME_DEG_TO_RAD);
    return jme_hours_normalize(jme_degrees_to_hours(jme_degree_normalize(theta)));
}

double jme_sidereal_time(double jd_ut)
{
    return jme_sidereal_time0(jd_ut, 0.0, 0.0);
}
