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

static double ayanamsa_precession_degrees(double jd_et, double t0)
{
    double t = (jd_et - t0) / 36525.0;

    return (5029.0966 * t + 1.11113 * t * t) / 3600.0;
}

static double krishnamurti_newcomb_ayanamsa_degrees(double jd_et)
{
    const double b1900_jd = 2415020.31352;
    const double besselian_year_days = 365.242198781;
    double besselian_year = 1900.0 + (jd_et - b1900_jd) / besselian_year_days;
    double t = besselian_year - 1900.0;
    double seconds = 80564.38104 + (50.2564 * t) + (0.000111 * t * t);

    return jme_degree_normalize(seconds / 3600.0);
}

static int gregorian_year_from_jd(double jd)
{
    int year = 0;
    jme_reverse_julian_day(jd, JME_CALENDAR_GREGORIAN, &year, 0, 0, 0);
    return year;
}

static int ayanamsa_from_fixed_star(double jd_et, const char *star, double sidereal_lon, double *ayan, char *error)
{
    double equatorial[6];
    double eps = 0.0;
    double dpsi = 0.0;
    double deps = 0.0;
    double lon = 0.0;
    double lat = 0.0;

    if (jme_fixstar(star, jd_et, JME_CALC_TRUE_POSITION | JME_CALC_EQUATORIAL, equatorial, error) != JME_OK
        || jme_get_obliquity(jd_et, jme_context_obliquity_model(), &eps, error) != JME_OK) {
        return JME_ERR;
    }
    if (jme_get_nutation(jd_et, jme_context_nutation_model(), &dpsi, &deps, error) == JME_OK) {
        eps += deps;
    }

    jme_equatorial_to_ecliptic(equatorial[0], equatorial[1], eps, &lon, &lat);
    (void)lat;

    *ayan = jme_degree_normalize(lon - sidereal_lon);
    return JME_OK;
}

static int ayanamsa_from_j2000_equatorial(double jd_et, double ra, double dec, double sidereal_lon, double *ayan, char *error)
{
    double spherical[6];
    double pos[6];
    double eps = 0.0;
    double lon = 0.0;
    double lat = 0.0;

    spherical[0] = ra;
    spherical[1] = dec;
    spherical[2] = 1.0;
    spherical[3] = 0.0;
    spherical[4] = 0.0;
    spherical[5] = 0.0;
    jme_spherical_to_rectangular_state(spherical, pos);

    if (jd_et != 2451545.0) {
        double bias_mat[9];
        double prec_mat[9];

        if (jme_get_frame_bias_matrix(jme_context_bias_model(), bias_mat) == JME_OK) {
            jme_matrix_transform_state(bias_mat, pos, pos);
        }
        jme_get_precession_matrix(2451545.0, jd_et, jme_context_precession_model(), prec_mat);
        jme_matrix_transform_state(prec_mat, pos, pos);
        if (jme_rectangular_to_spherical_state(pos, spherical) != JME_OK) {
            jme_set_error(error, "Invalid ayanamsa anchor vector");
            return JME_ERR;
        }
        ra = spherical[0];
        dec = spherical[1];
    }

    if (jme_get_obliquity(jd_et, jme_context_obliquity_model(), &eps, error) != JME_OK) {
        return JME_ERR;
    }
    jme_equatorial_to_ecliptic(ra, dec, eps, &lon, &lat);
    (void)lat;

    *ayan = jme_degree_normalize(lon - sidereal_lon);
    return JME_OK;
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
    double offset = 0.0;
    double t0 = 2451545.0;
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
    case JME_SIDEREAL_KRISHNAMURTI:
        *ayan = krishnamurti_newcomb_ayanamsa_degrees(jd_et);
        return JME_OK;
    case JME_SIDEREAL_RAMAN:
        *ayan = jme_degree_normalize(((double)(gregorian_year_from_jd(jd_et) - 397) * (151.0 / 3.0)) / 3600.0);
        return JME_OK;
    case JME_SIDEREAL_YUKTESHWAR:
        *ayan = jme_degree_normalize(((double)(gregorian_year_from_jd(jd_et) - 499) * 54.0) / 3600.0);
        return JME_OK;
    case JME_SIDEREAL_ARYABHATA:
        t0 = 1903396.789532;
        offset = 0.0;
        break;
    case JME_SIDEREAL_BABYL_ETPSC:
        t0 = jme_julian_day(-129, 1, 1, 0.0, JME_CALENDAR_JULIAN);
        offset = -(5.0 + 4.0 / 60.0 + 46.0 / 3600.0);
        break;
    case JME_SIDEREAL_BABYL_HUBER:
        t0 = jme_julian_day(-100, 1, 1, 0.0, JME_CALENDAR_JULIAN);
        offset = -(4.0 + 28.0 / 60.0);
        break;
    case JME_SIDEREAL_BABYL_KUGLER1:
        t0 = jme_julian_day(-100, 1, 1, 0.0, JME_CALENDAR_JULIAN);
        offset = -(5.0 + 40.0 / 60.0);
        break;
    case JME_SIDEREAL_BABYL_KUGLER2:
        t0 = jme_julian_day(-100, 1, 1, 0.0, JME_CALENDAR_JULIAN);
        offset = -(4.0 + 16.0 / 60.0);
        break;
    case JME_SIDEREAL_BABYL_KUGLER3:
        t0 = jme_julian_day(-100, 1, 1, 0.0, JME_CALENDAR_JULIAN);
        offset = -(3.0 + 25.0 / 60.0);
        break;
    case JME_SIDEREAL_DELUCE:
        t0 = 2415020.5;
        offset = 26.0 + 24.0 / 60.0 + 47.0 / 3600.0;
        break;
    case JME_SIDEREAL_HIPPARCHOS:
        t0 = 1674484.0;
        offset = -(9.0 + 20.0 / 60.0);
        break;
    case JME_SIDEREAL_JN_BHASIN:
        t0 = 1854239.3;
        offset = 0.0;
        break;
    case JME_SIDEREAL_SASSANIAN:
        t0 = 1927135.8747793;
        offset = 0.0;
        break;
    case JME_SIDEREAL_J2000:
        offset = 0.0;
        break;
    case JME_SIDEREAL_J1900:
        offset = 0.0;
        t0 = 2415020.0;
        break;
    case JME_SIDEREAL_B1950:
        offset = 0.0;
        t0 = 2433282.42345905;
        break;
    case JME_SIDEREAL_ALDEBARAN_15TAU:
        return ayanamsa_from_fixed_star(jd_et, "Aldebaran", 45.0, ayan, error);
    case JME_SIDEREAL_TRUE_CITRA:
    case JME_SIDEREAL_SS_CITRA:
        return ayanamsa_from_fixed_star(jd_et, "Spica", 180.0, ayan, error);
    case JME_SIDEREAL_TRUE_REVATI:
    case JME_SIDEREAL_SS_REVATI:
        return ayanamsa_from_fixed_star(jd_et, "Revati", 0.0, ayan, error);
    case JME_SIDEREAL_SURYASIDDHANTA:
        return ayanamsa_from_fixed_star(jd_et, "Revati", 359.8333333333333, ayan, error);
    case JME_SIDEREAL_USHASHASHI:
        return ayanamsa_from_fixed_star(jd_et, "Revati", 359.8333333333333, ayan, error);
    case JME_SIDEREAL_GALCENT_0SAG:
        return ayanamsa_from_j2000_equatorial(jd_et, 266.4168085291667, -29.0078377805556, 240.0, ayan, error);
    case JME_SIDEREAL_TRUE_MULA:
        return ayanamsa_from_fixed_star(jd_et, "Shaula", 240.0, ayan, error);
    case JME_SIDEREAL_TRUE_PUSHYA:
        return ayanamsa_from_fixed_star(jd_et, "Delta Cancri", 106.0, ayan, error);
    case JME_SIDEREAL_USER:
        if (ctx->sidereal_t0 == 0.0) {
            *ayan = ctx->sidereal_ayan_t0;
            return JME_OK;
        }
        t0 = ctx->sidereal_t0;
        offset = ctx->sidereal_ayan_t0;
        break;
    default:
        jme_set_error(error, "Unsupported ayanamsa model");
        return JME_ERR;
    }

    if (ayan != 0) {
        *ayan = jme_degree_normalize(offset + ayanamsa_precession_degrees(jd_et, t0));
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
