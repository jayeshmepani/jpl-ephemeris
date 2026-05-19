#include "jme/jme.h"

#include <math.h>

#define JME_RAD_TO_DEG 57.295779513082320876798154814105170332405472466565

void vsop87d_full_getEarth(double t, double temp[]);
void vsop87d_full_getJupiter(double t, double temp[]);
void vsop87d_full_getMars(double t, double temp[]);
void vsop87d_full_getMercury(double t, double temp[]);
void vsop87d_full_getNeptune(double t, double temp[]);
void vsop87d_full_getSaturn(double t, double temp[]);
void vsop87d_full_getUranus(double t, double temp[]);
void vsop87d_full_getVenus(double t, double temp[]);

static int jme_vsop87_spherical(double jd_et, int body, double *spherical)
{
    double t = (jd_et - 2451545.0) / 365250.0;
    double raw[3] = {0.0, 0.0, 0.0};

    if (spherical == 0) {
        return JME_ERR;
    }

    switch (body) {
    case JME_BODY_MERCURY:
        vsop87d_full_getMercury(t, raw);
        break;
    case JME_BODY_VENUS:
        vsop87d_full_getVenus(t, raw);
        break;
    case JME_BODY_EARTH:
        vsop87d_full_getEarth(t, raw);
        break;
    case JME_BODY_MARS:
        vsop87d_full_getMars(t, raw);
        break;
    case JME_BODY_JUPITER:
        vsop87d_full_getJupiter(t, raw);
        break;
    case JME_BODY_SATURN:
        vsop87d_full_getSaturn(t, raw);
        break;
    case JME_BODY_URANUS:
        vsop87d_full_getUranus(t, raw);
        break;
    case JME_BODY_NEPTUNE:
        vsop87d_full_getNeptune(t, raw);
        break;
    default:
        return JME_ERR;
    }

    spherical[0] = jme_degree_normalize(raw[0] * JME_RAD_TO_DEG);
    spherical[1] = raw[1] * JME_RAD_TO_DEG;
    spherical[2] = raw[2];
    spherical[3] = 0.0;
    spherical[4] = 0.0;
    spherical[5] = 0.0;

    return isfinite(spherical[0]) && isfinite(spherical[1]) && isfinite(spherical[2]) ? JME_OK : JME_ERR;
}

int jme_vsop87_planet_state(double jd_et, int body, double *results)
{
    double spherical[6];
    int i;

    if (results != 0) {
        for (i = 0; i < 6; i++) {
            results[i] = 0.0;
        }
    }

    if (results == 0) {
        return JME_ERR;
    }

    if (jme_vsop87_spherical(jd_et, body, spherical) != JME_OK) {
        return JME_ERR;
    }

    return jme_spherical_to_rectangular_state(spherical, results);
}
