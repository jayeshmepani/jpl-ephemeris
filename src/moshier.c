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

int jme_moshier_planet_state(double jd_et, int body, double *results)
{
    struct plantbl *table = 0;
    double pobj[3];
    int res;
    int i;

    if (results != 0) {
        for (i = 0; i < 6; i++) { results[i] = 0.0; }
    }

    switch (body) {
    case JME_BODY_MERCURY: table = &mer404; break;
    case JME_BODY_VENUS: table = &ven404; break;
    case JME_BODY_EARTH: table = &ear404; break;
    case JME_BODY_MARS: table = &mar404; break;
    case JME_BODY_JUPITER: table = &jup404; break;
    case JME_BODY_SATURN: table = &sat404; break;
    case JME_BODY_URANUS: table = &ura404; break;
    case JME_BODY_NEPTUNE: table = &nep404; break;
    case JME_BODY_PLUTO: table = &plu404; break;
    default:
        return JME_ERR;
    }

    res = jme_moshier_gplan(jd_et, table, pobj);
    if (res != 0) {
        return JME_ERR;
    }

    if (results != 0) {
        /* results[0]=long, results[1]=lat, results[2]=radius */
        /* Moshier results are in radians (long/lat) and AU (radius) */
        double spherical[6] = {0,0,0,0,0,0};
        spherical[0] = pobj[0] * (180.0 / JME_M_PI);
        spherical[1] = pobj[1] * (180.0 / JME_M_PI);
        spherical[2] = pobj[2];
        jme_spherical_to_rectangular_state(spherical, results);
    }

    return JME_OK;
}
