#include "jme/jme.h"
#include "context.h"
#include <math.h>

#define JME_C_AU_PER_DAY 173.1446326846693
#define JME_DEG_TO_RAD 0.017453292519943295769236907684886127134428718885417

static int analytic_body_state(double jd_et, int body, double *state)
{
    if (body == JME_BODY_SUN) {
        jme_meeus_sun_state(jd_et, state);
        return JME_OK;
    }

    if (body == JME_BODY_MOON) {
        jme_meeus_moon_state(jd_et, state);
        return JME_OK;
    }

    if (jme_vsop87_planet_state(jd_et, body, state) == JME_OK) {
        return JME_OK;
    }

    if (jme_moshier_planet_state(jd_et, body, state) == JME_OK) {
        return JME_OK;
    }

    return jme_meeus_planet_state(jd_et, body, state);
}

int jme_pheno_ut(double jd_ut, int body, int flags, double *attr, char *error)
{
    double dt = jme_delta_t(jd_ut);
    double jd_et = jd_ut + dt / 86400.0;
    return jme_pheno(jd_et, body, flags, attr, error);
}

int jme_calc_ut(double jd_ut, int body, int flags, double *results, char *error)
{
    double dt = jme_delta_t(jd_ut);
    double jd_et = jd_ut + dt / 86400.0;
    return jme_calc(jd_et, body, flags, results, error);
}

int jme_calc_pctr(double jd_et, int body, int center, int flags, double *results, char *error)
{
    double body_state[6];
    double center_state[6];
    int i;

    if (results == 0) {
        jme_set_error(error, "Output buffer is required");
        return JME_ERR;
    }

    if (jme_calc(jd_et, body, flags | JME_CALC_HELIOCENTRIC | JME_CALC_XYZ | JME_CALC_TRUE_POSITION, body_state, error) != JME_OK) {
        return JME_ERR;
    }

    if (center == JME_BODY_SUN) {
        for (i = 0; i < 6; i++) {
            center_state[i] = 0.0;
        }
    } else if (jme_calc(jd_et, center, flags | JME_CALC_HELIOCENTRIC | JME_CALC_XYZ | JME_CALC_TRUE_POSITION, center_state, error) != JME_OK) {
        return JME_ERR;
    }

    for (i = 0; i < 6; i++) {
        results[i] = body_state[i] - center_state[i];
    }

    if ((flags & JME_CALC_XYZ) == 0) {
        double spherical[6];
        if (jme_rectangular_to_spherical_state(results, spherical) != JME_OK) {
            return JME_ERR;
        }
        for (i = 0; i < 6; i++) {
            results[i] = spherical[i];
        }
    }

    return JME_OK;
}

int jme_orbit_max_min_true_distance(double jd_et, int body, int flags, double *tmax, double *tmin, double *dmax, double *dmin, char *error)
{
    (void)jd_et; (void)body; (void)flags; (void)tmax; (void)tmin; (void)dmax; (void)dmin; (void)error;
    return JME_ERR;
}

int jme_pheno(double jd_et, int body, int flags, double *attr, char *error)
{
    double planet_geo[6], earth_helio[6], planet_helio[6];
    double r, delta, R, cos_i, phase;
    int i;

    if (attr != 0) {
        for (i = 0; i < 20; i++) { attr[i] = 0.0; }
    }

    if (jme_calc(jd_et, body, flags | JME_CALC_TRUE_POSITION, planet_geo, error) != JME_OK) { return JME_ERR; }
    if (jme_calc(jd_et, JME_BODY_EARTH, flags | JME_CALC_HELIOCENTRIC | JME_CALC_TRUE_POSITION, earth_helio, error) != JME_OK) { return JME_ERR; }
    if (jme_calc(jd_et, body, flags | JME_CALC_HELIOCENTRIC | JME_CALC_TRUE_POSITION, planet_helio, error) != JME_OK) { return JME_ERR; }

    delta = jme_state_distance(planet_geo);
    R = jme_state_distance(earth_helio);
    r = jme_state_distance(planet_helio);

    cos_i = (r * r + delta * delta - R * R) / (2.0 * r * delta);
    if (cos_i > 1.0) cos_i = 1.0;
    if (cos_i < -1.0) cos_i = -1.0;

    phase = (1.0 + cos_i) / 2.0;

    if (attr != 0) {
        attr[0] = phase;
        attr[1] = acos(cos_i) * 180.0 / 3.14159265358979323846;
        /* attr[2] and following indices reserved for extended attributes */
    }

    return JME_OK;
}

int jme_calc(double jd_et, int body, int flags, double *results, char *error)
{
    double state[6];
    double target_pos[6];
    double observer_pos_au[3] = {0,0,0};
    double dist, light_time;
    int center = JME_BODY_EARTH;
    int i;

    if (results != 0) {
        for (i = 0; i < 6; i++) { results[i] = 0.0; }
    }

    if (flags & JME_CALC_BARYCENTRIC) {
        center = JME_BODY_SOLAR_SYSTEM_BARYCENTER;
    } else if (flags & JME_CALC_HELIOCENTRIC) {
        center = JME_BODY_SUN;
    } else if (flags & JME_CALC_TOPOCENTRIC) {
        if (jme_get_topo_pos(jd_et, observer_pos_au, error) != JME_OK) {
            return JME_ERR;
        }
    }

    /* 1. Geometric position (Target - Center) at t */
    if (jme_jpl_body_state(jd_et, body, center, JME_VECTOR_AU_PER_DAY, target_pos, error) != JME_OK) {
        if (analytic_body_state(jd_et, body, target_pos) != JME_OK) {
            return JME_ERR;
        }

        if (center == JME_BODY_EARTH) {
            double earth_helio[6];
            if (analytic_body_state(jd_et, JME_BODY_EARTH, earth_helio) != JME_OK) {
                return JME_ERR;
            }
            for (i = 0; i < 3; i++) { target_pos[i] -= earth_helio[i]; }
        }
    }

    /* Apply topocentric correction to geometric position if needed */
    if (flags & JME_CALC_TOPOCENTRIC) {
        for (i = 0; i < 3; i++) { target_pos[i] -= observer_pos_au[i]; }
    }

    /* 2. Light-time correction */
    if (!(flags & JME_CALC_TRUE_POSITION)) {
        dist = jme_state_distance(target_pos);
        light_time = dist / JME_C_AU_PER_DAY;

        if (jme_jpl_body_state(jd_et - light_time, body, center, JME_VECTOR_AU_PER_DAY, target_pos, error) != JME_OK) {
            if (analytic_body_state(jd_et - light_time, body, target_pos) != JME_OK) {
                return JME_ERR;
            }
            if (center == JME_BODY_EARTH) {
                double earth_helio[6];
                if (analytic_body_state(jd_et - light_time, JME_BODY_EARTH, earth_helio) != JME_OK) {
                    return JME_ERR;
                }
                for (i = 0; i < 3; i++) { target_pos[i] -= earth_helio[i]; }
            }
        }

        /* Second iteration */
        dist = jme_state_distance(target_pos);
        light_time = dist / JME_C_AU_PER_DAY;
        if (jme_jpl_body_state(jd_et - light_time, body, center, JME_VECTOR_AU_PER_DAY, target_pos, error) != JME_OK) {
            if (analytic_body_state(jd_et - light_time, body, target_pos) != JME_OK) {
                return JME_ERR;
            }
            if (center == JME_BODY_EARTH) {
                double earth_helio[6];
                if (analytic_body_state(jd_et - light_time, JME_BODY_EARTH, earth_helio) != JME_OK) {
                    return JME_ERR;
                }
                for (i = 0; i < 3; i++) { target_pos[i] -= earth_helio[i]; }
            }
        }

        /* 2.1 Aberration */
        if (!(flags & JME_CALC_NO_ABERRATION)) {
            double earth_state[6];
            double v_obs[3];
            double u[3], u_prime[3];
            double v_dot_u, beta;
            double v2 = 0.0;

            /* Need Earth's barycentric velocity for stellar aberration */
            if (jme_jpl_body_state(jd_et, JME_BODY_EARTH, JME_BODY_SOLAR_SYSTEM_BARYCENTER, JME_VECTOR_AU_PER_DAY, earth_state, error) != JME_OK) {
                if (analytic_body_state(jd_et, JME_BODY_EARTH, earth_state) != JME_OK) {
                    return JME_ERR;
                }
            }

            for (i = 0; i < 3; i++) {
                v_obs[i] = earth_state[i + 3] / JME_C_AU_PER_DAY;
                v2 += v_obs[i] * v_obs[i];
            }

            dist = jme_state_distance(target_pos);
            for (i = 0; i < 3; i++) { u[i] = target_pos[i] / dist; }

            v_dot_u = 0.0;
            for (i = 0; i < 3; i++) { v_dot_u += v_obs[i] * u[i]; }

            beta = sqrt(1.0 - v2);

            for (i = 0; i < 3; i++) {
                u_prime[i] = (u[i] + v_obs[i] + (v_dot_u * v_obs[i] / (1.0 + beta))) / (1.0 + v_dot_u);
            }

            /* Update target_pos with aberrated position (maintain same distance for simplicity of angular result) */
            for (i = 0; i < 3; i++) { target_pos[i] = u_prime[i] * dist; }
        }

        /* 2.2 Gravitational Deflection (mostly Sun) */
        if (!(flags & JME_CALC_NO_LIGHT_DEFLECTION) && (body != JME_BODY_SUN)) {
            double sun_state[6];
            double e[3], p[3], u[3], u_prime[3];
            double sun_dist, e_dot_u, factor;
            /* Using Schwarzschild radius 2GM/c^2 in AU */
            const double r_s_sun = 2.0 * 1.32712440041e20 / (JME_SPEED_OF_LIGHT_KM_PER_SEC * 1000.0 * JME_SPEED_OF_LIGHT_KM_PER_SEC * 1000.0) / (JME_AU_KM * 1000.0);

            if (jme_jpl_body_state(jd_et, JME_BODY_SUN, center, JME_VECTOR_AU_PER_DAY, sun_state, error) == JME_OK) {
                sun_dist = jme_state_distance(sun_state);
                for (i = 0; i < 3; i++) { e[i] = sun_state[i] / sun_dist; }

                dist = jme_state_distance(target_pos);
                for (i = 0; i < 3; i++) { u[i] = target_pos[i] / dist; }

                e_dot_u = 0.0;
                for (i = 0; i < 3; i++) { e_dot_u += e[i] * u[i]; }

                factor = 2.0 * r_s_sun / (sun_dist * (1.0 + e_dot_u));

                for (i = 0; i < 3; i++) {
                    p[i] = e[i] - e_dot_u * u[i];
                    u_prime[i] = u[i] + factor * p[i];
                }

                /* Normalize u_prime */
                dist = sqrt(u_prime[0] * u_prime[0] + u_prime[1] * u_prime[1] + u_prime[2] * u_prime[2]);
                dist = jme_state_distance(target_pos) / dist;
                for (i = 0; i < 3; i++) { target_pos[i] = u_prime[i] * dist; }
            }
        }
    }

    /* 3. Reference Frame / Transformation */
    if (!(flags & JME_CALC_J2000)) {
        /* Precession to date */
        double prec_mat[9];
        jme_get_precession_matrix(2451545.0, jd_et, JME_MODEL_PREC_IAU_1976, prec_mat);
        jme_matrix_transform_state(prec_mat, target_pos, target_pos);

        if (!(flags & JME_CALC_NO_NUTATION)) {
            /* Nutation to date */
            double dpsi, deps, eps;
            double nut_mat[9];
            jme_get_nutation(jd_et, JME_MODEL_NUT_IAU_1980, &dpsi, &deps, error);
            jme_get_obliquity(jd_et, JME_MODEL_OBL_IAU_1980, &eps, error);
            jme_get_nutation_matrix(dpsi * JME_DEG_TO_RAD, deps * JME_DEG_TO_RAD, eps * JME_DEG_TO_RAD, nut_mat);
            jme_matrix_transform_state(nut_mat, target_pos, target_pos);
        }
    }

    if (flags & JME_CALC_XYZ) {
        /* Return rectangular */
        for (i = 0; i < 6; i++) { results[i] = target_pos[i]; }
    } else {
        /* Convert to spherical (default is Ecliptic if not JME_CALC_EQUATORIAL) */
        if (flags & JME_CALC_EQUATORIAL) {
            jme_rectangular_to_spherical_state(target_pos, state);
        } else {
            /* Convert to ecliptic then spherical */
            double ecliptic_rect[6];
            double eps;
            double jd_for_obl = (flags & JME_CALC_J2000) ? 2451545.0 : jd_et;
            jme_get_obliquity(jd_for_obl, JME_MODEL_OBL_IAU_1980, &eps, error);
            
            /* If not J2000, we need the True Obliquity for True Ecliptic */
            if (!(flags & JME_CALC_J2000) && !(flags & JME_CALC_NO_NUTATION)) {
                double dpsi, deps;
                jme_get_nutation(jd_et, JME_MODEL_NUT_IAU_1980, &dpsi, &deps, error);
                eps += deps;
            }

            jme_equatorial_to_ecliptic_rectangular_state(target_pos, eps, ecliptic_rect);
            jme_rectangular_to_spherical_state(ecliptic_rect, state);
        }

        for (i = 0; i < 6; i++) { results[i] = state[i]; }
    }

    return JME_OK;
}
