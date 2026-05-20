#include "jme/jme.h"
#include <math.h>

/* Forward declarations for VSOP87A functions in vsop87a_full.c */
extern void vsop87a_full_getMercury(double t, double temp[]);
extern void vsop87a_full_getVenus(double t, double temp[]);
extern void vsop87a_full_getEarth(double t, double temp[]);
extern void vsop87a_full_getMars(double t, double temp[]);
extern void vsop87a_full_getJupiter(double t, double temp[]);
extern void vsop87a_full_getSaturn(double t, double temp[]);
extern void vsop87a_full_getUranus(double t, double temp[]);
extern void vsop87a_full_getNeptune(double t, double temp[]);
extern void vsop87a_full_getEmb(double t, double temp[]);
extern void vsop87a_full_getMoon(double earth[], double emb[], double temp[]);

static int vsop87_position(double jd_et, int body, double *pos)
{
    double t = (jd_et - 2451545.0) / 365250.0; /* Julian Millennia from J2000 */
    double temp[3] = {0.0, 0.0, 0.0};
    int i;

    if (pos == 0) {
        return JME_ERR;
    }

    switch (body) {
    case JME_BODY_MERCURY: vsop87a_full_getMercury(t, temp); break;
    case JME_BODY_VENUS: vsop87a_full_getVenus(t, temp); break;
    case JME_BODY_EARTH: vsop87a_full_getEarth(t, temp); break;
    case JME_BODY_MARS: vsop87a_full_getMars(t, temp); break;
    case JME_BODY_JUPITER: vsop87a_full_getJupiter(t, temp); break;
    case JME_BODY_SATURN: vsop87a_full_getSaturn(t, temp); break;
    case JME_BODY_URANUS: vsop87a_full_getUranus(t, temp); break;
    case JME_BODY_NEPTUNE: vsop87a_full_getNeptune(t, temp); break;
    case JME_BODY_EARTH_MOON_BARYCENTER: vsop87a_full_getEmb(t, temp); break;
    case JME_BODY_MOON:
        {
            double earth[6], emb[6];
            vsop87a_full_getEarth(t, earth);
            vsop87a_full_getEmb(t, emb);
            vsop87a_full_getMoon(earth, emb, temp);
        }
        break;
    default:
        return JME_ERR;
    }

    for (i = 0; i < 3; i++) {
        pos[i] = temp[i];
        if (!isfinite(pos[i])) {
            return JME_ERR;
        }
    }

    return JME_OK;
}

int jme_vsop87_planet_state(double jd_et, int body, double *results)
{
    double prev[3];
    double next[3];
    double step = 1.0e-3;
    int i;

    if (results != 0) {
        for (i = 0; i < 6; i++) { results[i] = 0.0; }
    }

    if (results == 0) {
        return vsop87_position(jd_et, body, prev);
    }

    if (vsop87_position(jd_et, body, results) != JME_OK) {
        return jme_meeus_planet_state(jd_et, body, results);
    }

    if (vsop87_position(jd_et - step, body, prev) == JME_OK
        && vsop87_position(jd_et + step, body, next) == JME_OK) {
        for (i = 0; i < 3; i++) {
            results[i + 3] = (next[i] - prev[i]) / (2.0 * step);
        }
    }

    return JME_OK;
}
