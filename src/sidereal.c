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

/*
 * Ayanamsa parameter sources (comments only; values below are unchanged).
 *
 * Where a value is marked "source to be verified", the original publication
 * defining that ayanamsa has not yet been checked first-hand by this project;
 * the value is a commonly published astrological convention and must be
 * traced to a primary source before it is described as such.
 *
 * Epoch-based models use: ayanamsa(t) = offset(t0) + general precession in
 * longitude accumulated from t0 (see ayanamsa_precession_degrees()).
 */

/*
 * General precession in longitude, p_A = 5029.0966" T + 1.11113" T^2
 * (T in Julian centuries from t0): IAU 1976 precession, J. H. Lieske,
 * T. Lederle, W. Fricke, B. Morando, "Expressions for the precession
 * quantities based upon the IAU (1976) system of astronomical constants",
 * Astron. Astrophys. 58, 1-16 (1977). The cubic term (-0.000006" T^3) is
 * omitted here.
 */
static double ayanamsa_precession_degrees(double jd_et, double t0)
{
    double t = (jd_et - t0) / 36525.0;

    return (5029.0966 * t + 1.11113 * t * t) / 3600.0;
}

/*
 * Krishnamurti (KP) ayanamsa: K. S. Krishnamurti's Krishnamurti Paddhati,
 * stated as a Newcomb-precession formula in Besselian years from B1900:
 * 80564.38104" (22 deg 22' 44.38") at B1900.0 + 50.2564" t + 0.000111" t^2.
 * The exact KP publication and edition for these coefficients: source to be
 * verified. B1900.0 = JD 2415020.31352 and the tropical (Besselian) year of
 * 365.242198781 days follow the standard Besselian epoch definitions
 * (Lieske 1979, Astron. Astrophys. 73, 282).
 */
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

/*
 * Earth Rotation Angle, IAU 2000 Resolution B1.8: N. Capitaine, B. Guinot,
 * D. D. McCarthy, Astron. Astrophys. 355, 398 (2000); IERS Conventions (2010),
 * eq. (5.15).
 */
static double earth_rotation_angle_iau2000(double jd_ut)
{
    double d1 = floor(jd_ut);
    double d2 = jd_ut - d1;
    double t = d1 + (d2 - 2451545.0);
    double f = fmod(d1, 1.0) + fmod(d2, 1.0);

    return normalize_radians(JME_TWO_PI * (f + 0.7790572732640 + 0.00273781191135448 * t));
}

/*
 * GMST consistent with IAU 2006 precession: N. Capitaine, P. T. Wallace,
 * J. Chapront, Astron. Astrophys. 412, 567 (2003); IERS Conventions (2010),
 * eq. (5.32).
 */
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

/*
 * GMST, IAU 1982 expression (S. Aoki et al., Astron. Astrophys. 105, 359,
 * 1982) in the degree form given by J. Meeus, Astronomical Algorithms,
 * 2nd ed., eq. (12.4); the nutation term gives apparent sidereal time.
 */
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
        /* Fagan-Bradley "Synetic Vernal Point" (C. Fagan, D. Bradley, Western
         * sidereal astrology, 1950s). Offset is the value at J2000 (24 deg 44');
         * original epoch definition and value: source to be verified. */
        offset = 24.733333333;
        break;
    case JME_SIDEREAL_LAHIRI:
        /* Lahiri (Chitrapaksha) ayanamsa adopted by the Indian Calendar Reform
         * Committee, Report of the Calendar Reform Committee, Government of
         * India (CSIR), New Delhi, 1955 (chair M. N. Saha; N. C. Lahiri),
         * defined as 23 deg 15' 00" on 1956 March 21, 0h ET. The J2000 offset
         * used here (23.8570 deg) is that definition carried to J2000; the exact
         * reduction behind this rounded value: source to be verified. */
        offset = 23.8570;
        break;
    case JME_SIDEREAL_KRISHNAMURTI:
        /* See krishnamurti_newcomb_ayanamsa_degrees(). */
        *ayan = krishnamurti_newcomb_ayanamsa_degrees(jd_et);
        return JME_OK;
    case JME_SIDEREAL_RAMAN:
        /* B. V. Raman's ayanamsa: zero in 397 CE, 50.333" per year
         * (B. V. Raman, Hindu Predictive Astrology / A Manual of Hindu
         * Astrology; edition and page: source to be verified). */
        *ayan = jme_degree_normalize(((double)(gregorian_year_from_jd(jd_et) - 397) * (151.0 / 3.0)) / 3600.0);
        return JME_OK;
    case JME_SIDEREAL_YUKTESHWAR:
        /* Sri Yukteswar Giri, The Holy Science (1894): equinox at the start of
         * sidereal Aries in 499 CE and precession taken as 54" per year.
         * Edition and page reference: source to be verified. */
        *ayan = jme_degree_normalize(((double)(gregorian_year_from_jd(jd_et) - 499) * 54.0) / 3600.0);
        return JME_OK;
    case JME_SIDEREAL_ARYABHATA:
        /* Zero ayanamsa in 499 CE (era of Aryabhata's Aryabhatiya);
         * t0 = JD 1903396.789532. Exact epoch convention: source to be verified. */
        t0 = 1903396.789532;
        offset = 0.0;
        break;
    case JME_SIDEREAL_BABYL_ETPSC:
        /* Babylonian sidereal zodiac, "ETPSC" variant; epoch -129 and offset
         * -5 deg 04' 46" at that epoch: source to be verified. */
        t0 = jme_julian_day(-129, 1, 1, 0.0, JME_CALENDAR_JULIAN);
        offset = -(5.0 + 4.0 / 60.0 + 46.0 / 3600.0);
        break;
    case JME_SIDEREAL_BABYL_HUBER:
        /* Babylonian zodiac after P. J. Huber; offset -4 deg 28' at -100:
         * source to be verified. */
        t0 = jme_julian_day(-100, 1, 1, 0.0, JME_CALENDAR_JULIAN);
        offset = -(4.0 + 28.0 / 60.0);
        break;
    case JME_SIDEREAL_BABYL_KUGLER1:
        /* Babylonian zodiac after F. X. Kugler, Sternkunde und Sterndienst in
         * Babel; Kugler 1/2/3 offsets at -100: source to be verified. */
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
        /* Robert DeLuce's ayanamsa; offset 26 deg 24' 47" at JD 2415020.5
         * (1900 Jan 1): source to be verified. */
        t0 = 2415020.5;
        offset = 26.0 + 24.0 / 60.0 + 47.0 / 3600.0;
        break;
    case JME_SIDEREAL_HIPPARCHOS:
        /* "Hipparchos" ayanamsa (zodiac referred to Hipparchus' era);
         * t0 = JD 1674484.0 and offset -9 deg 20': source to be verified. */
        t0 = 1674484.0;
        offset = -(9.0 + 20.0 / 60.0);
        break;
    case JME_SIDEREAL_JN_BHASIN:
        /* J. N. Bhasin's ayanamsa; zero at JD 1854239.3: source to be verified. */
        t0 = 1854239.3;
        offset = 0.0;
        break;
    case JME_SIDEREAL_SASSANIAN:
        /* Sassanian (Zij-i Shah) zodiac; zero at JD 1927135.8747793: source to be verified. */
        t0 = 1927135.8747793;
        offset = 0.0;
        break;
    case JME_SIDEREAL_J2000:
        /* Ayanamsa zero at J2000.0 = JD 2451545.0 TT (IAU 1976 System). */
        offset = 0.0;
        break;
    case JME_SIDEREAL_J1900:
        /* Ayanamsa zero at J1900.0 = JD 2415020.0. */
        offset = 0.0;
        t0 = 2415020.0;
        break;
    case JME_SIDEREAL_B1950:
        /* Ayanamsa zero at Besselian epoch B1950.0 = JD 2433282.42345905
         * (J. H. Lieske, Astron. Astrophys. 73, 282, 1979). */
        offset = 0.0;
        t0 = 2433282.42345905;
        break;
    case JME_SIDEREAL_ALDEBARAN_15TAU:
        /* Star-anchored models below are definitions, not fitted constants:
         * the named star is placed at the stated sidereal longitude (e.g.
         * Aldebaran at 15 Taurus = 45 deg; Spica at 180 deg for Citra; Revati
         * star at 0 deg; Shaula at 240 deg for Mula; Delta Cancri at 106 deg
         * for Pushya). Star positions come from the fixed-star catalogue
         * (Yale Bright Star Catalogue, see THIRD_PARTY_NOTICES.md). Who first
         * defined each individual anchor: source to be verified. */
        return ayanamsa_from_fixed_star(jd_et, "Aldebaran", 45.0, ayan, error);
    case JME_SIDEREAL_TRUE_CITRA:
    case JME_SIDEREAL_SS_CITRA:
        return ayanamsa_from_fixed_star(jd_et, "Spica", 180.0, ayan, error);
    case JME_SIDEREAL_TRUE_REVATI:
    case JME_SIDEREAL_SS_REVATI:
        return ayanamsa_from_fixed_star(jd_et, "Revati", 0.0, ayan, error);
    case JME_SIDEREAL_SURYASIDDHANTA:
        /* Surya Siddhanta and Ushashashi variants: Revati star at 359 deg 50'
         * (10' before the initial point); anchor convention: source to be verified. */
        return ayanamsa_from_fixed_star(jd_et, "Revati", 359.8333333333333, ayan, error);
    case JME_SIDEREAL_USHASHASHI:
        return ayanamsa_from_fixed_star(jd_et, "Revati", 359.8333333333333, ayan, error);
    case JME_SIDEREAL_GALCENT_0SAG:
        /* Galactic Center at 0 Sagittarius (240 deg). J2000 position
         * RA 266.4168085 deg, Dec -29.0078378 deg corresponds to Sgr A*
         * (cf. M. J. Reid, A. Brunthaler, ApJ 616, 872, 2004); exact catalogue
         * origin of these digits: source to be verified. */
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
