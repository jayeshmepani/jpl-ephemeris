#include "jme/jme.h"
#include "context.h"
#include <ctype.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>

#define JME_FIXSTAR_DEG_TO_RAD 0.017453292519943295769236907684886127134428718885417

typedef struct jme_star_entry {
    int hr;
    int hd;
    int sao;
    const char *name;
    double ra; /* J2000 degrees */
    double dec; /* J2000 degrees */
    double pm_ra; /* arcsec/year */
    double pm_dec; /* arcsec/year */
    double mag;
} jme_star_entry;

typedef struct jme_star_alias {
    const char *name;
    int hr;
} jme_star_alias;

/* Common-name aliases point into the generated Bright Star catalog. */
static const jme_star_alias star_aliases[] = {
    {"Achernar", 472},
    {"Aldebaran", 1457},
    {"Altair", 7557},
    {"Antares", 6134},
    {"Arcturus", 5340},
    {"Betelgeuse", 2061},
    {"Canopus", 2326},
    {"Capella", 1708},
    {"Deneb", 7924},
    {"Delta Cancri", 3461},
    {"Fomalhaut", 8728},
    {"Pollux", 2990},
    {"Procyon", 2943},
    {"Regulus", 3982},
    {"Revati", 361},
    {"Rigel", 1713},
    {"Shaula", 6527},
    {"Sirius", 2491},
    {"Spica", 5056},
    {"Vega", 7001},
    {"Asellus Australis", 3461},
    {"Lambda Scorpii", 6527},
    {"Eta Psc", 437},
    {"Zeta Psc", 361}
};

#include "fixstar_catalog.inc"

static int ascii_case_equal(const char *a, const char *b)
{
    while (*a != '\0' && *b != '\0') {
        if (tolower((unsigned char)*a) != tolower((unsigned char)*b)) {
            return 0;
        }
        a++;
        b++;
    }

    return *a == '\0' && *b == '\0';
}

static int parse_prefixed_number(const char *text, const char *prefix, int *value)
{
    const char *p = text;
    char *end = 0;
    long parsed;

    while (isspace((unsigned char)*p)) {
        p++;
    }

    while (*prefix != '\0') {
        if (tolower((unsigned char)*p) != tolower((unsigned char)*prefix)) {
            return 0;
        }
        p++;
        prefix++;
    }

    while (isspace((unsigned char)*p) || *p == '-') {
        p++;
    }

    if (!isdigit((unsigned char)*p)) {
        return 0;
    }

    parsed = strtol(p, &end, 10);
    while (end != 0 && isspace((unsigned char)*end)) {
        end++;
    }

    if (end == 0 || *end != '\0' || parsed <= 0 || parsed > 999999999L) {
        return 0;
    }

    *value = (int)parsed;
    return 1;
}

static const jme_star_entry *find_star_by_hr(int hr)
{
    size_t i;

    for (i = 0; i < sizeof(star_catalog) / sizeof(star_catalog[0]); i++) {
        if (star_catalog[i].hr == hr) {
            return &star_catalog[i];
        }
    }

    return 0;
}

static const jme_star_entry *find_star(const char *name)
{
    size_t i;
    int value = 0;

    if (name == 0 || name[0] == '\0') {
        return 0;
    }

    if (parse_prefixed_number(name, "HR", &value)) {
        return find_star_by_hr(value);
    }

    if (parse_prefixed_number(name, "HD", &value)) {
        for (i = 0; i < sizeof(star_catalog) / sizeof(star_catalog[0]); i++) {
            if (star_catalog[i].hd == value) {
                return &star_catalog[i];
            }
        }
        return 0;
    }

    if (parse_prefixed_number(name, "SAO", &value)) {
        for (i = 0; i < sizeof(star_catalog) / sizeof(star_catalog[0]); i++) {
            if (star_catalog[i].sao == value) {
                return &star_catalog[i];
            }
        }
        return 0;
    }

    for (i = 0; i < sizeof(star_aliases) / sizeof(star_aliases[0]); i++) {
        if (ascii_case_equal(star_aliases[i].name, name)) {
            return find_star_by_hr(star_aliases[i].hr);
        }
    }

    for (i = 0; i < sizeof(star_catalog) / sizeof(star_catalog[0]); i++) {
        if (star_catalog[i].name[0] != '\0' && ascii_case_equal(star_catalog[i].name, name)) {
            return &star_catalog[i];
        }
    }
    return 0;
}

static double fixstar_angle_to_output_unit(double degrees, int flags)
{
    if (flags & JME_CALC_RADIANS) {
        return degrees * JME_FIXSTAR_DEG_TO_RAD;
    }

    return degrees;
}

static double fixstar_ayanamsa_rate(double jd_et)
{
    double before = jme_get_ayanamsa(jd_et - 0.5);
    double after = jme_get_ayanamsa(jd_et + 0.5);

    if (!isfinite(before) || !isfinite(after)) {
        return 0.0;
    }

    return jme_degrees_difference_signed(after, before);
}

int jme_fixstar_ut(const char *star, double jd_ut, int flags, double *results, char *error)
{
    double dt = jme_delta_t(jd_ut);
    double jd_et = jd_ut + dt / 86400.0;
    return jme_fixstar(star, jd_et, flags, results, error);
}

int jme_fixstar(const char *star, double jd_et, int flags, double *results, char *error)
{
    const jme_star_entry *entry = find_star(star);
    double t = (jd_et - 2451545.0) / 365.25; /* years from J2000 */
    double pos[6];
    double spherical[6];
    double ra, dec;
    int i;

    if (results == 0) {
        jme_set_error(error, "Fixed-star output is required");
        return JME_ERR;
    }

    for (i = 0; i < 6; i++) { results[i] = 0.0; }

    if (entry == 0) {
        jme_set_error(error, "Star not found in catalog");
        return JME_ERR;
    }

    /* 1. Apply Proper Motion */
    ra = entry->ra + (entry->pm_ra / 3600.0) * t;
    dec = entry->dec + (entry->pm_dec / 3600.0) * t;

    /* 2. Convert to rectangular state */
    spherical[0] = ra;
    spherical[1] = dec;
    spherical[2] = 1.0; /* Distant star unit distance */
    spherical[3] = (entry->pm_ra / 3600.0) / 365.25;
    spherical[4] = (entry->pm_dec / 3600.0) / 365.25;
    spherical[5] = 0.0;
    jme_spherical_to_rectangular_state(spherical, pos);

    /* 3. Apply Reduction Pipeline (Aberration, Precession, Nutation) */
    /* Reuse jme_calc reduction logic but for a point source at infinity */
    /* Note: simplified geocentric geocentric without light-time from ephemeris */
    
    if (!(flags & JME_CALC_TRUE_POSITION)) {
        /* Aberration */
        if (!(flags & JME_CALC_NO_ABERRATION)) {
            double earth_state[6];
            double v_obs[3];
            double u[3], u_prime[3];
            double v_dot_u, beta;
            double v2 = 0.0;

            if (jme_jpl_body_state(jd_et, JME_BODY_EARTH, JME_BODY_SOLAR_SYSTEM_BARYCENTER, JME_VECTOR_AU_PER_DAY, earth_state, error) == JME_OK) {
                for (i = 0; i < 3; i++) {
                    v_obs[i] = earth_state[i + 3] / 173.1446; /* c in AU/day */
                    v2 += v_obs[i] * v_obs[i];
                }

                for (i = 0; i < 3; i++) { u[i] = pos[i]; } /* pos is already unit vector */

                v_dot_u = 0.0;
                for (i = 0; i < 3; i++) { v_dot_u += v_obs[i] * u[i]; }

                beta = sqrt(1.0 - v2);
                for (i = 0; i < 3; i++) {
                    u_prime[i] = (u[i] + v_obs[i] + (v_dot_u * v_obs[i] / (1.0 + beta))) / (1.0 + v_dot_u);
                }
                for (i = 0; i < 3; i++) { pos[i] = u_prime[i]; }
            }
        }
    }

    /* Reference Frame */
    if (!(flags & JME_CALC_J2000)) {
        double bias_mat[9];
        double prec_mat[9];

        if (jme_get_frame_bias_matrix(jme_context_bias_model(), bias_mat) == JME_OK) {
            jme_matrix_transform_state(bias_mat, pos, pos);
        }

        jme_get_precession_matrix(2451545.0, jd_et, jme_context_precession_model(), prec_mat);
        jme_matrix_transform_state(prec_mat, pos, pos);

        if (!(flags & JME_CALC_NO_NUTATION)) {
            double dpsi, deps, eps;
            double nut_mat[9];
            jme_get_nutation(jd_et, jme_context_nutation_model(), &dpsi, &deps, error);
            jme_get_obliquity(jd_et, jme_context_obliquity_model(), &eps, error);
            jme_get_nutation_matrix(
                dpsi * JME_FIXSTAR_DEG_TO_RAD,
                deps * JME_FIXSTAR_DEG_TO_RAD,
                eps * JME_FIXSTAR_DEG_TO_RAD,
                nut_mat
            );
            jme_matrix_transform_state(nut_mat, pos, pos);
        }
    }

    /* Coordinate System */
    if (flags & JME_CALC_XYZ) {
        for (i = 0; i < 3; i++) { results[i] = pos[i]; }
    } else {
        if (flags & JME_CALC_EQUATORIAL) {
            jme_rectangular_to_spherical_state(pos, spherical);
        } else {
            double ecliptic_rect[6];
            double eps;
            double jd_for_obl = (flags & JME_CALC_J2000) ? 2451545.0 : jd_et;
            jme_get_obliquity(jd_for_obl, jme_context_obliquity_model(), &eps, error);
            if (!(flags & JME_CALC_J2000) && !(flags & JME_CALC_NO_NUTATION)) {
                double dpsi, deps;
                jme_get_nutation(jd_et, jme_context_nutation_model(), &dpsi, &deps, error);
                eps += deps;
            }
            jme_equatorial_to_ecliptic_rectangular_state(pos, eps, ecliptic_rect);
            jme_rectangular_to_spherical_state(ecliptic_rect, spherical);
        }
        for (i = 0; i < 6; i++) { results[i] = spherical[i]; }
        if (flags & JME_CALC_SIDEREAL) {
            results[0] = jme_degree_normalize(results[0] - jme_get_ayanamsa(jd_et));
            results[3] -= fixstar_ayanamsa_rate(jd_et);
        }
        results[0] = fixstar_angle_to_output_unit(results[0], flags);
        results[1] = fixstar_angle_to_output_unit(results[1], flags);
        results[3] = fixstar_angle_to_output_unit(results[3], flags);
        results[4] = fixstar_angle_to_output_unit(results[4], flags);
    }

    return JME_OK;
}

int jme_fixstar_mag(const char *star, double *mag, char *error)
{
    const jme_star_entry *entry = find_star(star);
    if (mag == 0) {
        jme_set_error(error, "Fixed-star magnitude output is required");
        return JME_ERR;
    }
    if (entry == 0) {
        jme_set_error(error, "Star not found");
        return JME_ERR;
    }
    *mag = entry->mag;
    return JME_OK;
}

int jme_fixstar2(const char *star, double jd_et, int flags, double *results, char *error)
{
    return jme_fixstar(star, jd_et, flags, results, error);
}

int jme_fixstar2_ut(const char *star, double jd_ut, int flags, double *results, char *error)
{
    return jme_fixstar_ut(star, jd_ut, flags, results, error);
}

int jme_fixstar2_mag(const char *star, double *mag, char *error)
{
    return jme_fixstar_mag(star, mag, error);
}
