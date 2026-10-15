#include "jme/jme.h"
#include "context.h"
#include <math.h>

/* jd is TDB (callers convert from TT). */
static int elp2000_position(double jd_et, double *out)
{
    double km[3];

    if (out == 0 || jme_elp82b_geocentric_km(jd_et, km) != JME_OK) {
        return JME_ERR;
    }
    out[0] = km[0] / 149597870.7;
    out[1] = km[1] / 149597870.7;
    out[2] = km[2] / 149597870.7;

    return isfinite(out[0]) && isfinite(out[1]) && isfinite(out[2]) ? JME_OK : JME_ERR;
}

#if defined(__GNUC__)
__attribute__((visibility("hidden")))
#endif
int jme_internal_elp2000_moon_position(double jd_et, double *results)
{
    int i;

    if (results != 0) {
        for (i = 0; i < 6; i++) {
            results[i] = 0.0;
        }
    }

    if (results == 0) {
        return JME_ERR;
    }

    return elp2000_position(jd_et, results);
}

/*
 * Geocentric Moon from the complete ELP 2000-82B series: position (AU) and
 * the exact analytical time derivative (AU/day). jd is TDB. J2000 ecliptic.
 */
int jme_elp2000_moon_state(double jd_et, double *results)
{
    double km[3], kmd[3];
    int i;

    if (results == 0) {
        return JME_ERR;
    }
    for (i = 0; i < 6; i++) {
        results[i] = 0.0;
    }
    if (jme_elp82b_geocentric_state_km(jd_et, km, kmd) != JME_OK) {
        return JME_ERR;
    }
    for (i = 0; i < 3; i++) {
        results[i] = km[i] / 149597870.7;
        results[i + 3] = kmd[i] / 149597870.7;
    }
    return JME_OK;
}
