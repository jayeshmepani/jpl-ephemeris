#include "jme/jme.h"
#include "elp2000/elp2000-82b.h"
#include <math.h>

static int elp2000_position(double jd_et, double *out)
{
    double t = (jd_et - 2451545.0) / 36525.0; /* Julian Centuries from J2000 */
    cartesian_3d_point pos;

    if (out == 0) {
        return JME_ERR;
    }

    pos = geocentric_moon_position_cartesian_of_J2000(t);
    out[0] = pos.x / 149597870.7;
    out[1] = pos.y / 149597870.7;
    out[2] = pos.z / 149597870.7;

    return isfinite(out[0]) && isfinite(out[1]) && isfinite(out[2]) ? JME_OK : JME_ERR;
}

int jme_elp2000_moon_state(double jd_et, double *results)
{
    double prev[3];
    double next[3];
    double step = 1.0e-3;
    int i;

    if (results != 0) {
        for (i = 0; i < 6; i++) { results[i] = 0.0; }
    }

    if (results == 0) {
        return JME_ERR;
    }

    /* ELP2000-82B returns J2000 geocentric ecliptic rectangular in km; convert to AU. */
    if (elp2000_position(jd_et, results) != JME_OK) {
        return JME_ERR;
    }

    if (elp2000_position(jd_et - step, prev) == JME_OK
        && elp2000_position(jd_et + step, next) == JME_OK) {
        for (i = 0; i < 3; i++) {
            results[i + 3] = (next[i] - prev[i]) / (2.0 * step);
        }
    }

    return JME_OK;
}
