#include "jme/jme.h"
#include "context.h"
#include "jme_erfa_vendor.h"

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
    {JME_SIDEREAL_LAHIRI_ICRC, "Lahiri (ICRC 1955)"},
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

/*
 * AYANAMSA MODELS
 * ===============
 * Full definitions, sources, epochs and verification status of every model
 * are documented in docs/AYANAMSA_MODELS.md. Conventions used here:
 *
 *  - jme_get_ayanamsa_ex() returns the MEAN ayanamsa: the longitude of the
 *    sidereal origin measured from the MEAN equinox of date on the mean
 *    ecliptic of date. The TRUE ayanamsa (JME_AYANAMSA_TRUE) adds the
 *    nutation in longitude of the context nutation model, i.e. it is measured
 *    from the true equinox of date.
 *  - Epoch-defined models: mean(t) = mean(t0) + P(t0 -> t), where P is the
 *    IAU 1976 general precession in longitude between two arbitrary epochs
 *    (Lieske et al. 1977, A&A 58, 1), in the two-epoch form published in the
 *    Indian Astronomical Ephemeris 1989, p. 525:
 *      P = (5029.0966 + 2.22226 T - 0.000042 T^2) t
 *          + (1.11161 - 0.000127 T) t^2 - 0.000113 t^3   [arcsec]
 *      T = (t0 - J2000)/36525, t = (t - t0)/36525 (TT Julian centuries).
 *  - Star-anchored ("true star") models: mean ayanamsa = geocentric TRUE
 *    position (proper motion, parallax and radial velocity propagated; no
 *    aberration or light deflection) of the anchor star, referred to the mean
 *    ecliptic and equinox of date, minus the anchor longitude.
 *  - Historical epochs stated in UT are converted to TT with the context
 *    Delta-T model; dates before 1582-10-15 are Julian-calendar dates.
 */

#define JME_J2000 2451545.0

static double iae_general_precession_arcsec(double jd_t0, double jd_t)
{
    double big_t = (jd_t0 - JME_J2000) / 36525.0;
    double t = (jd_t - jd_t0) / 36525.0;
    return (5029.0966 + 2.22226 * big_t - 0.000042 * big_t * big_t) * t
        + (1.11161 - 0.000127 * big_t) * t * t
        - 0.000113 * t * t * t;
}

static double iae_general_precession_rate_arcsec_per_day(double jd_t0, double jd_t)
{
    double big_t = (jd_t0 - JME_J2000) / 36525.0;
    double t = (jd_t - jd_t0) / 36525.0;
    return ((5029.0966 + 2.22226 * big_t - 0.000042 * big_t * big_t)
        + 2.0 * (1.11161 - 0.000127 * big_t) * t
        - 3.0 * 0.000113 * t * t) / 36525.0;
}

static int calendar_for_historical_date(int year, int month, int day)
{
    if (year < 1582 || (year == 1582 && (month < 10 || (month == 10 && day < 15)))) {
        return JME_CALENDAR_JULIAN;
    }
    return JME_CALENDAR_GREGORIAN;
}

static int epoch_ut_to_tt(int year, int month, int day, double hour_ut, double *jd_tt, char *error)
{
    double jd_ut = jme_julian_day(year, month, day, hour_ut, calendar_for_historical_date(year, month, day));
    if (!isfinite(jd_ut)) {
        jme_set_error(error, "Sidereal reference epoch could not be converted to Julian date");
        return JME_ERR;
    }
    return internal_ut_to_et(jd_ut, jd_tt, error);
}

static int epoch_model(double jd_et, double jd_t0_tt, double mean_at_t0_deg, double *ayan, double *rate)
{
    *ayan = jme_degree_normalize(mean_at_t0_deg + iae_general_precession_arcsec(jd_t0_tt, jd_et) / 3600.0);
    if (rate != 0) *rate = iae_general_precession_rate_arcsec_per_day(jd_t0_tt, jd_et) / 3600.0;
    return JME_OK;
}

static int epoch_model_ut(double jd_et, int y, int m, int d, double hour_ut, double mean_at_t0_deg,
    double *ayan, double *rate, char *error)
{
    double jd_t0;
    if (epoch_ut_to_tt(y, m, d, hour_ut, &jd_t0, error) != JME_OK) return JME_ERR;
    return epoch_model(jd_et, jd_t0, mean_at_t0_deg, ayan, rate);
}

/*
 * Lahiri: the Indian Astronomical Ephemeris (from 1985) uses a TRUE ayanamsa
 * of 23 deg 15' 00.658" on 1956 March 21, 0h ET(=TT) (IAE 1989, p. 556
 * footnote), IAU 1976 precession and IAU 1980 nutation. The ICRC value
 * (Report of the Calendar Reform Committee, 1955; pre-1985 IAE/IENA) is
 * 23 deg 15' 00" for the same instant. Mean value at t0 = true value minus
 * IAU 1980 nutation in longitude at t0.
 */
static int lahiri_model(double jd_et, double true_at_t0_arcsec, double *ayan, double *rate)
{
    const double t0 = 2435553.5; /* 1956-03-21 0h TT */
    double dpsi, deps;
    jme_eraNut80(t0, 0.0, &dpsi, &deps);
    return epoch_model(jd_et, t0, (true_at_t0_arcsec - dpsi * 206264.80624709635515647) / 3600.0, ayan, rate);
}

/* Krishnamurti (KP) as historically implemented in JME; see AYANAMSA_MODELS.md (provenance flagged). */
static double krishnamurti_newcomb_ayanamsa_degrees(double jd_et)
{
    const double b1900_jd = 2415020.31352;
    const double besselian_year_days = 365.242198781;
    double t = (jd_et - b1900_jd) / besselian_year_days;
    double seconds = 80564.38104 + (50.2564 * t) + (0.000111 * t * t);
    return jme_degree_normalize(seconds / 3600.0);
}

static int gregorian_year_from_jd(double jd)
{
    int year = 0;
    jme_reverse_julian_day(jd, JME_CALENDAR_GREGORIAN, &year, 0, 0, 0);
    return year;
}

/* Mean ecliptic-of-date longitude of a catalogue star (true position, no aberration). */
static int ayanamsa_from_fixed_star(double jd_et, const char *star, double sidereal_lon, double *ayan, char *error)
{
    double ecl[6];
    if (internal_fixstar_mean_ecliptic_of_date(star, jd_et, ecl, error) != JME_OK) {
        return JME_ERR;
    }
    *ayan = jme_degree_normalize(ecl[0] - sidereal_lon);
    return JME_OK;
}

static int ayanamsa_from_j2000_equatorial(double jd_et, double ra, double dec, double sidereal_lon, double *ayan, char *error)
{
    double spherical[6];
    double pos[6];
    double prec_mat[9];
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
    /* ICRS -> mean equator/equinox of date (bias, precession). */
    if (internal_bias_precession_matrix(jd_et, prec_mat, error) != JME_OK) {
        return JME_ERR;
    }
    jme_matrix_transform_state(prec_mat, pos, pos);
    if (jme_rectangular_to_spherical_state(pos, spherical) != JME_OK) {
        jme_set_error(error, "Invalid ayanamsa anchor vector");
        return JME_ERR;
    }
    if (jme_get_obliquity(jd_et, jme_context_obliquity_model(), &eps, error) != JME_OK) {
        return JME_ERR;
    }
    jme_equatorial_to_ecliptic(spherical[0], spherical[1], eps, &lon, &lat);
    (void)lat;
    *ayan = jme_degree_normalize(lon - sidereal_lon);
    return JME_OK;
}

static int ayanamsa_mean(double jd_et, int model, double *ayan, double *rate, char *error)
{
    jme_context *ctx = jme_get_context();
    const double ss_hour = 7.0 + 30.0 / 60.0 + 31.57 / 3600.0;

    if (rate != 0) *rate = NAN;
    switch (model) {
    case JME_SIDEREAL_FAGAN_BRADLEY:
        /* 24 deg 02' 31.36" at 1950.0 (taken as B1950.0, JD 2433282.42345905 TT). */
        return epoch_model(jd_et, 2433282.42345905, 24.0 + 2.0 / 60.0 + 31.36 / 3600.0, ayan, rate);
    case JME_SIDEREAL_LAHIRI:
        return lahiri_model(jd_et, 23.0 * 3600.0 + 15.0 * 60.0 + 0.658, ayan, rate);
    case JME_SIDEREAL_LAHIRI_ICRC:
        return lahiri_model(jd_et, 23.0 * 3600.0 + 15.0 * 60.0, ayan, rate);
    case JME_SIDEREAL_KRISHNAMURTI:
        *ayan = krishnamurti_newcomb_ayanamsa_degrees(jd_et);
        if (rate != 0) *rate = (50.2564 + 2.0 * 0.000111 * ((jd_et - 2415020.31352) / 365.242198781)) / 365.242198781 / 3600.0;
        return JME_OK;
    case JME_SIDEREAL_RAMAN:
        /* B. V. Raman's whole-year rule: (Gregorian year - 397) x 50 1/3". */
        *ayan = jme_degree_normalize(((double)(gregorian_year_from_jd(jd_et) - 397) * (151.0 / 3.0)) / 3600.0);
        if (rate != 0) *rate = 0.0;
        return JME_OK;
    case JME_SIDEREAL_YUKTESHWAR:
        /* Sri Yukteswar whole-year rule: (Gregorian year - 499) x 54". */
        *ayan = jme_degree_normalize(((double)(gregorian_year_from_jd(jd_et) - 499) * 54.0) / 3600.0);
        if (rate != 0) *rate = 0.0;
        return JME_OK;
    case JME_SIDEREAL_ARYABHATA:
        return epoch_model_ut(jd_et, 499, 3, 21, 6.0 + 56.0 / 60.0 + 55.57 / 3600.0, 0.0, ayan, rate, error);
    case JME_SIDEREAL_SURYASIDDHANTA:
        return epoch_model_ut(jd_et, 499, 3, 21, ss_hour, 0.0, ayan, rate, error);
    case JME_SIDEREAL_SS_REVATI:
        return epoch_model_ut(jd_et, 499, 3, 21, ss_hour, -0.79167046, ayan, rate, error);
    case JME_SIDEREAL_SS_CITRA:
        return epoch_model_ut(jd_et, 499, 3, 21, ss_hour, 2.11070444, ayan, rate, error);
    case JME_SIDEREAL_USHASHASHI:
        /* 18 deg 39' 39.46" on 1900 January 1 (taken as 0h UT, Gregorian). */
        return epoch_model_ut(jd_et, 1900, 1, 1, 0.0, 18.0 + 39.0 / 60.0 + 39.46 / 3600.0, ayan, rate, error);
    case JME_SIDEREAL_BABYL_ETPSC:
        return epoch_model(jd_et, jme_julian_day(-129, 1, 1, 0.0, JME_CALENDAR_JULIAN), -(5.0 + 4.0 / 60.0 + 46.0 / 3600.0), ayan, rate);
    case JME_SIDEREAL_BABYL_HUBER:
        return epoch_model(jd_et, jme_julian_day(-100, 1, 1, 0.0, JME_CALENDAR_JULIAN), -(4.0 + 28.0 / 60.0), ayan, rate);
    case JME_SIDEREAL_BABYL_KUGLER1:
        return epoch_model(jd_et, jme_julian_day(-100, 1, 1, 0.0, JME_CALENDAR_JULIAN), -(5.0 + 40.0 / 60.0), ayan, rate);
    case JME_SIDEREAL_BABYL_KUGLER2:
        return epoch_model(jd_et, jme_julian_day(-100, 1, 1, 0.0, JME_CALENDAR_JULIAN), -(4.0 + 16.0 / 60.0), ayan, rate);
    case JME_SIDEREAL_BABYL_KUGLER3:
        return epoch_model(jd_et, jme_julian_day(-100, 1, 1, 0.0, JME_CALENDAR_JULIAN), -(3.0 + 25.0 / 60.0), ayan, rate);
    case JME_SIDEREAL_DELUCE:
        return epoch_model(jd_et, 2415020.5, 26.0 + 24.0 / 60.0 + 47.0 / 3600.0, ayan, rate);
    case JME_SIDEREAL_HIPPARCHOS:
        return epoch_model(jd_et, 1674484.0, -(9.0 + 20.0 / 60.0), ayan, rate);
    case JME_SIDEREAL_JN_BHASIN:
        return epoch_model(jd_et, 1854239.3, 0.0, ayan, rate);
    case JME_SIDEREAL_SASSANIAN:
        return epoch_model(jd_et, 1927135.8747793, 0.0, ayan, rate);
    case JME_SIDEREAL_J2000:
        return epoch_model(jd_et, 2451545.0, 0.0, ayan, rate);
    case JME_SIDEREAL_J1900:
        return epoch_model(jd_et, 2415020.0, 0.0, ayan, rate);
    case JME_SIDEREAL_B1950:
        return epoch_model(jd_et, 2433282.42345905, 0.0, ayan, rate);
    case JME_SIDEREAL_ALDEBARAN_15TAU:
        return ayanamsa_from_fixed_star(jd_et, "Aldebaran", 45.0, ayan, error);
    case JME_SIDEREAL_TRUE_CITRA:
        return ayanamsa_from_fixed_star(jd_et, "Spica", 180.0, ayan, error);
    case JME_SIDEREAL_TRUE_REVATI:
        return ayanamsa_from_fixed_star(jd_et, "Revati", 359.0 + 50.0 / 60.0, ayan, error);
    case JME_SIDEREAL_TRUE_MULA:
        return ayanamsa_from_fixed_star(jd_et, "Shaula", 240.0, ayan, error);
    case JME_SIDEREAL_TRUE_PUSHYA:
        return ayanamsa_from_fixed_star(jd_et, "Delta Cancri", 106.0, ayan, error);
    case JME_SIDEREAL_GALCENT_0SAG:
        /* Sgr A* ICRS position (Reid & Brunthaler 2004, ApJ 616, 872): RA 17h45m40.0409s, Dec -29d00'28.118". */
        return ayanamsa_from_j2000_equatorial(jd_et, 266.4168370833333, -29.007810555555556, 240.0, ayan, error);
    case JME_SIDEREAL_USER:
        if (ctx->sidereal_t0 == 0.0) {
            /* Documented legacy behaviour: t0 == 0 means a constant ayanamsa. */
            *ayan = ctx->sidereal_ayan_t0;
            if (rate != 0) *rate = 0.0;
            return JME_OK;
        }
        return epoch_model(jd_et, ctx->sidereal_t0, ctx->sidereal_ayan_t0, ayan, rate);
    default:
        jme_set_error(error, "Unsupported ayanamsa model");
        return JME_ERR;
    }
}

int internal_ayanamsa(double jd_et, int model, int flags, double *ayan, double *rate, char *error)
{
    double value, r = NAN;

    if (ayan == 0) {
        jme_set_error(error, "Ayanamsa output is required");
        return JME_ERR;
    }
    *ayan = NAN;
    if (rate != 0) *rate = NAN;
    if (!isfinite(jd_et)) {
        jme_set_error(error, "Ayanamsa Julian date must be finite");
        return JME_ERR;
    }
    if ((flags & ~JME_AYANAMSA_TRUE) != 0) {
        jme_set_error(error, "Unsupported ayanamsa flags");
        return JME_ERR;
    }
    if (ayanamsa_mean(jd_et, model, &value, &r, error) != JME_OK) {
        return JME_ERR;
    }
    if (rate != 0 && !isfinite(r)) {
        /* Star-anchored models: numerical derivative over +/- 0.5 day. */
        double before, after;
        if (ayanamsa_mean(jd_et - 0.5, model, &before, 0, error) != JME_OK
            || ayanamsa_mean(jd_et + 0.5, model, &after, 0, error) != JME_OK) {
            return JME_ERR;
        }
        r = jme_degrees_difference_signed(after, before);
    }
    if (flags & JME_AYANAMSA_TRUE) {
        double dpsi, deps;
        if (jme_get_nutation(jd_et, jme_context_nutation_model(), &dpsi, &deps, error) != JME_OK) {
            return JME_ERR;
        }
        value = jme_degree_normalize(value + dpsi);
    }
    if (!isfinite(value)) {
        jme_set_error(error, "Ayanamsa calculation produced a non-finite value");
        return JME_ERR;
    }
    *ayan = value;
    if (rate != 0) *rate = r;
    if (error != 0) error[0] = '\0';
    return JME_OK;
}

int internal_context_ayanamsa(double jd_et, double *mean, double *rate, char *error)
{
    return internal_ayanamsa(jd_et, jme_get_context()->sidereal_mode, 0, mean, rate, error);
}

double jme_get_ayanamsa(double jd_et)
{
    double ayan = NAN;
    if (internal_context_ayanamsa(jd_et, &ayan, 0, 0) != JME_OK) {
        return NAN;
    }
    return ayan;
}

double jme_get_ayanamsa_ut(double jd_ut)
{
    double jd_et;
    if (internal_ut_to_et(jd_ut, &jd_et, 0) != JME_OK) return NAN;
    return jme_get_ayanamsa(jd_et);
}

int jme_get_ayanamsa_ex(double jd_et, int model, double *ayan, char *error)
{
    return internal_ayanamsa(jd_et, model, JME_AYANAMSA_MEAN, ayan, 0, error);
}

int jme_get_ayanamsa_ex_ut(double jd_ut, int model, double *ayan, char *error)
{
    double jd_et;
    if (ayan == 0) {
        jme_set_error(error, "Ayanamsa output is required");
        return JME_ERR;
    }
    *ayan = NAN;
    if (internal_ut_to_et(jd_ut, &jd_et, error) != JME_OK) return JME_ERR;
    return jme_get_ayanamsa_ex(jd_et, model, ayan, error);
}

int jme_get_ayanamsa_ex2(double jd_et, int model, int flags, double *ayan, double *rate, char *error)
{
    return internal_ayanamsa(jd_et, model, flags, ayan, rate, error);
}

int jme_get_ayanamsa_ex2_ut(double jd_ut, int model, int flags, double *ayan, double *rate, char *error)
{
    double jd_et;
    if (ayan == 0) {
        jme_set_error(error, "Ayanamsa output is required");
        return JME_ERR;
    }
    *ayan = NAN;
    if (internal_ut_to_et(jd_ut, &jd_et, error) != JME_OK) return JME_ERR;
    return internal_ayanamsa(jd_et, model, flags, ayan, rate, error);
}

/*
 * Sidereal time. Both models are evaluated with the vendored ERFA routines
 * (see src/astrometry.c, internal_sidereal_time_rad()).
 */
double jme_sidereal_time0(double jd_ut, double eps, double nut)
{
    /* GMST, IAU 1982 (Aoki et al. 1982; ERFA eraGmst82) plus the caller's
     * nutation in longitude times cos(obliquity), both in degrees. */
    double u1, u2, theta;
    if (!isfinite(jd_ut) || !isfinite(eps) || !isfinite(nut)) return NAN;
    u1 = floor(jd_ut);
    u2 = jd_ut - u1;
    theta = jme_eraGmst82(u1, u2) / JME_DEG_TO_RAD + nut * cos(eps * JME_DEG_TO_RAD);
    return jme_hours_normalize(jme_degrees_to_hours(jme_degree_normalize(theta)));
}

int jme_sidereal_time_ex(double jd_ut, int apparent, double *hours, char *error)
{
    double jd_tt, gst;
    if (hours == 0) {
        jme_set_error(error, "Sidereal-time output is required");
        return JME_ERR;
    }
    *hours = NAN;
    if (internal_ut_to_et(jd_ut, &jd_tt, error) != JME_OK) return JME_ERR;
    if (internal_sidereal_time_rad(jd_ut, jd_tt, apparent != 0, &gst, error) != JME_OK) return JME_ERR;
    *hours = jme_hours_normalize(gst / JME_DEG_TO_RAD / 15.0);
    return JME_OK;
}

double jme_sidereal_time(double jd_ut)
{
    /* Greenwich MEAN sidereal time (hours) for the context sidereal-time
     * model; see jme_sidereal_time_ex() for apparent sidereal time. */
    double hours;
    if (jme_sidereal_time_ex(jd_ut, 0, &hours, 0) != JME_OK) return NAN;
    return hours;
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
