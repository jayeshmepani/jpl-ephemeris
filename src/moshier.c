#include "jme/jme.h"
#include <math.h>

#define JME_M_PI 3.14159265358979323846

/* Forward declarations for Moshier tables and driver in moshier_full.c */
#ifndef JME_MOSHIER_PLANTBL_DEFINED
struct plantbl {
  char max_harmonic[9];
  char max_power_of_t;
  char *arg_tbl;
  double *lon_tbl;
  double *lat_tbl;
  double *rad_tbl;
  double distance;
};
#define JME_MOSHIER_PLANTBL_DEFINED
#endif

extern struct plantbl mer404, ven404, ear404, mar404, jup404, sat404, ura404, nep404, plu404;
extern int jme_moshier_gplan(double J, struct plantbl *plan, double pobj[]);

static struct plantbl *moshier_table_for_body(int body)
{
    switch (body) {
    case JME_BODY_MERCURY: return &mer404;
    case JME_BODY_VENUS: return &ven404;
    case JME_BODY_EARTH: return &ear404;
    case JME_BODY_MARS: return &mar404;
    case JME_BODY_JUPITER: return &jup404;
    case JME_BODY_SATURN: return &sat404;
    case JME_BODY_URANUS: return &ura404;
    case JME_BODY_NEPTUNE: return &nep404;
    case JME_BODY_PLUTO: return &plu404;
    default: return 0;
    }
}

static int moshier_position(double jd_et, int body, double *position)
{
    struct plantbl *table = moshier_table_for_body(body);
    double pobj[3];
    double spherical[6] = {0,0,0,0,0,0};

    if (position == 0 || table == 0 || jme_moshier_gplan(jd_et, table, pobj) != 0) {
        return JME_ERR;
    }

    spherical[0] = pobj[0] * (180.0 / JME_M_PI);
    spherical[1] = pobj[1] * (180.0 / JME_M_PI);
    spherical[2] = pobj[2];

    return jme_spherical_to_rectangular_state(spherical, position);
}

int jme_moshier_planet_state(double jd_et, int body, double *results)
{
    double prev[6];
    double next[6];
    double step = 1.0e-3;
    int i;

    if (results == 0) {
        return JME_ERR;
    }

    for (i = 0; i < 6; i++) { results[i] = 0.0; }

    if (moshier_position(jd_et, body, results) != JME_OK) {
        return JME_ERR;
    }

    if (moshier_position(jd_et - step, body, prev) == JME_OK
        && moshier_position(jd_et + step, body, next) == JME_OK) {
        for (i = 0; i < 3; i++) {
            results[i + 3] = (next[i] - prev[i]) / (2.0 * step);
        }
    }

    return JME_OK;
}
