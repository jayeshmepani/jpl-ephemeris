#include "jme/jme.h"
#include "context.h"
#include <string.h>
#include <math.h>

typedef struct jme_star_entry {
    const char *name;
    double ra; /* J2000 degrees */
    double dec; /* J2000 degrees */
    double pm_ra; /* arcsec/year */
    double pm_dec; /* arcsec/year */
    double mag;
} jme_star_entry;

/* Small reference catalog for initial implementation.
   Data source: SIMBAD / FK5. */
static const jme_star_entry star_catalog[] = {
    {"Spica", 201.298218, -11.161319, -0.042, -0.032, 0.98},
    {"Regulus", 152.092962, 11.967209, -0.249, 0.001, 1.35},
    {"Aldebaran", 68.980162, 16.509302, 0.063, -0.189, 0.85},
    {"Antares", 247.351915, -26.432002, -0.010, -0.020, 1.06},
    {"Revati", 14.1611, 2.5000, 0.0, 0.0, 5.0} /* Temporary entry for yogatara Revati */
};

static const jme_star_entry *find_star(const char *name)
{
    size_t i;
    for (i = 0; i < sizeof(star_catalog) / sizeof(star_catalog[0]); i++) {
        if (strcmp(star_catalog[i].name, name) == 0) {
            return &star_catalog[i];
        }
    }
    return 0;
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

    if (results != 0) {
        for (i = 0; i < 6; i++) { results[i] = 0.0; }
    }

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
    spherical[3] = 0.0;
    spherical[4] = 0.0;
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
        double prec_mat[9];
        jme_get_precession_matrix(2451545.0, jd_et, JME_MODEL_PREC_IAU_1976, prec_mat);
        jme_matrix_transform_state(prec_mat, pos, pos);

        if (!(flags & JME_CALC_NO_NUTATION)) {
            double dpsi, deps, eps;
            double nut_mat[9];
            jme_get_nutation(jd_et, JME_MODEL_NUT_IAU_1980, &dpsi, &deps, error);
            jme_get_obliquity(jd_et, JME_MODEL_OBL_IAU_1980, &eps, error);
            jme_get_nutation_matrix(dpsi * 0.017453, deps * 0.017453, eps * 0.017453, nut_mat);
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
            jme_get_obliquity(jd_for_obl, JME_MODEL_OBL_IAU_1980, &eps, error);
            if (!(flags & JME_CALC_J2000) && !(flags & JME_CALC_NO_NUTATION)) {
                double dpsi, deps;
                jme_get_nutation(jd_et, JME_MODEL_NUT_IAU_1980, &dpsi, &deps, error);
                eps += deps;
            }
            jme_equatorial_to_ecliptic_rectangular_state(pos, eps, ecliptic_rect);
            jme_rectangular_to_spherical_state(ecliptic_rect, spherical);
        }
        for (i = 0; i < 6; i++) { results[i] = spherical[i]; }
    }

    return JME_OK;
}

int jme_fixstar_mag(const char *star, double *mag, char *error)
{
    const jme_star_entry *entry = find_star(star);
    if (entry == 0) {
        jme_set_error(error, "Star not found");
        return JME_ERR;
    }
    if (mag != 0) { *mag = entry->mag; }
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
