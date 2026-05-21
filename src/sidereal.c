#include "jme/jme.h"
#include "context.h"

#include <math.h>

#define JME_DEG_TO_RAD 0.017453292519943295769236907684886127134428718885417
#define JME_ARCSEC_TO_RAD 4.848136811095359935899141023221522846734e-6
#define JME_TWO_PI 6.2831853071795864769252867665590057683943387987502

typedef struct jme_sidereal_name_entry {
    int model;
    const char *name;
} jme_sidereal_name_entry;

static const jme_sidereal_name_entry sidereal_names[] = {
    {JME_SIDEREAL_FAGAN_BRADLEY, "Fagan-Bradley"},
    {JME_SIDEREAL_LAHIRI, "Lahiri"},
    {JME_SIDEREAL_ALDEBARAN_15TAU, "Aldebaran 15 Tau"},
    {JME_SIDEREAL_ARYABHATA, "Aryabhata"},
    {JME_SIDEREAL_B1950, "B1950"},
    {JME_SIDEREAL_BABYL_ETPSC, "Babylonian ETPSC"},
    {JME_SIDEREAL_BABYL_HUBER, "Babylonian Huber"},
    {JME_SIDEREAL_BABYL_KUGLER1, "Babylonian Kugler 1"},
    {JME_SIDEREAL_BABYL_KUGLER2, "Babylonian Kugler 2"},
    {JME_SIDEREAL_BABYL_KUGLER3, "Babylonian Kugler 3"},
    {JME_SIDEREAL_DELUCE, "De Luce"},
    {JME_SIDEREAL_GALCENT_0SAG, "Galactic Center 0 Sagittarius"},
    {JME_SIDEREAL_HIPPARCHOS, "Hipparchos"},
    {JME_SIDEREAL_J1900, "J1900"},
    {JME_SIDEREAL_J2000, "J2000"},
    {JME_SIDEREAL_JN_BHASIN, "J.N. Bhasin"},
    {JME_SIDEREAL_KRISHNAMURTI, "Krishnamurti"},
    {JME_SIDEREAL_RAMAN, "Raman"},
    {JME_SIDEREAL_SASSANIAN, "Sassanian"},
    {JME_SIDEREAL_SS_CITRA, "Surya Siddhanta Citra"},
    {JME_SIDEREAL_SS_REVATI, "Surya Siddhanta Revati"},
    {JME_SIDEREAL_SURYASIDDHANTA, "Surya Siddhanta"},
    {JME_SIDEREAL_TRUE_CITRA, "True Citra"},
    {JME_SIDEREAL_TRUE_MULA, "True Mula"},
    {JME_SIDEREAL_TRUE_PUSHYA, "True Pushya"},
    {JME_SIDEREAL_TRUE_REVATI, "True Revati"},
    {JME_SIDEREAL_USER, "User"},
    {JME_SIDEREAL_USHASHASHI, "Ushashashi"},
    {JME_SIDEREAL_YUKTESHWAR, "Yukteshwar"}
};

static double normalize_radians(double angle)
{
    double value = fmod(angle, JME_TWO_PI);
    if (value < 0.0) {
        value += JME_TWO_PI;
    }
    return value;
}

static double earth_rotation_angle_iau2000(double jd_ut)
{
    double d1 = floor(jd_ut);
    double d2 = jd_ut - d1;
    double t = d1 + (d2 - 2451545.0);
    double f = fmod(d1, 1.0) + fmod(d2, 1.0);

    return normalize_radians(JME_TWO_PI * (f + 0.7790572732640 + 0.00273781191135448 * t));
}

static double sidereal_time_iau2006(double jd_ut)
{
    double jd_tt = jd_ut + jme_delta_t(jd_ut) / 86400.0;
    double t = (jd_tt - 2451545.0) / 36525.0;
    double gmst = earth_rotation_angle_iau2000(jd_ut)
        + (0.014506
            + (4612.156534
            + (1.3915817
            + (-0.00000044
            + (-0.000029956
            + (-0.0000000368) * t) * t) * t) * t) * t) * JME_ARCSEC_TO_RAD;

    return jme_hours_normalize(jme_degrees_to_hours(normalize_radians(gmst) / JME_DEG_TO_RAD));
}

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
    if (jme_context_sidereal_time_model() == JME_MODEL_SIDT_IAU_2006) {
        return sidereal_time_iau2006(jd_ut);
    }

    return jme_sidereal_time0(jd_ut, 0.0, 0.0);
}

double jme_get_ayanamsa(double jd_et)
{
    double ayan = 0.0;
    jme_context *ctx = jme_get_context();
    if (jme_get_ayanamsa_ex(jd_et, ctx->sidereal_mode, &ayan, 0) != JME_OK) {
        return NAN;
    }
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
    jme_context *ctx = jme_get_context();

    if (ayan != 0) { *ayan = 0.0; }
    if (ayan == 0) {
        jme_set_error(error, "Ayanamsa output is required");
        return JME_ERR;
    }

    switch (model) {
    case JME_SIDEREAL_FAGAN_BRADLEY:
        offset = 24.733333333;
        break;
    case JME_SIDEREAL_LAHIRI:
        offset = 23.8570;
        break;
    case JME_SIDEREAL_USER:
        if (ctx->sidereal_t0 == 0.0) {
            *ayan = ctx->sidereal_ayan_t0;
            return JME_OK;
        }
        t = (jd_et - ctx->sidereal_t0) / 36525.0;
        offset = ctx->sidereal_ayan_t0;
        break;
    default:
        jme_set_error(error, "Unsupported ayanamsa model");
        return JME_ERR;
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
    size_t i;

    for (i = 0; i < sizeof(sidereal_names) / sizeof(sidereal_names[0]); i++) {
        if (sidereal_names[i].model == model) {
            return sidereal_names[i].name;
        }
    }

    return "Unknown sidereal mode";
}
