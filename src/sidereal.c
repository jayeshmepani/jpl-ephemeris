#include "jme/jme.h"
#include "context.h"

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

double jme_get_ayanamsa(double jd_et)
{
    double ayan = 0.0;
    jme_context *ctx = jme_get_context();
    jme_get_ayanamsa_ex(jd_et, ctx->sidereal_mode, &ayan, 0);
    return ayan;
}

double jme_get_ayanamsa_ut(double jd_ut)
{
    double dt = jme_delta_t(jd_ut);
    double jd_et = jd_ut + dt / 86400.0;
    return jme_get_ayanamsa(jd_et);
}

int jme_get_ayanamsa_ex(double jd_et, int model, double *ayan, char *error)
{
    double t = (jd_et - 2451545.0) / 36525.0;
    double offset = 0.0;
    double prec = 0.0;

    (void)error;

    if (ayan != 0) { *ayan = 0.0; }

    switch (model) {
    case JME_SIDEREAL_FAGAN_BRADLEY:
        offset = 24.733333333; /* Approx */
        break;
    case JME_SIDEREAL_LAHIRI:
        offset = 23.8570; /* Approx */
        break;
    default:
        /* Default to Lahiri if unknown */
        offset = 23.8570;
        break;
    }

    /* Precession in longitude (simplified Lieske for ayanamsa) */
    /* p_a = 5029.0966t + 1.11113t^2 ... arcsec */
    prec = (5029.0966 * t + 1.11113 * t * t) / 3600.0;

    if (ayan != 0) {
        *ayan = offset + prec;
    }

    return JME_OK;
}

int jme_get_ayanamsa_ex_ut(double jd_ut, int model, double *ayan, char *error)
{
    double dt = jme_delta_t(jd_ut);
    double jd_et = jd_ut + dt / 86400.0;
    return jme_get_ayanamsa_ex(jd_et, model, ayan, error);
}

const char *jme_get_ayanamsa_name(int model)
{
    switch (model) {
    case JME_SIDEREAL_FAGAN_BRADLEY: return "Fagan-Bradley";
    case JME_SIDEREAL_LAHIRI: return "Lahiri";
    default: return "Sidereal Mode";
    }
}
