#include "jme/jme.h"
#include "context.h"
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/time.h>

#define JME_C_AU_PER_DAY 173.1446326846693
#define JME_DEG_TO_RAD 0.017453292519943295769236907684886127134428718885417
#define JME_PI 3.14159265358979323846264338327950288419716939937510

static unsigned long long g_profile_calc_ut_calls = 0ULL;
static double g_profile_calc_ut_seconds = 0.0;
static int g_profile_calc_ut_enabled = 0;
static int g_profile_calc_ut_init = 0;

static double jme_profile_now_seconds_local(void)
{
    struct timeval tv;
    gettimeofday(&tv, 0);
    return (double)tv.tv_sec + ((double)tv.tv_usec / 1000000.0);
}

static void jme_profile_calc_ut_report(void)
{
    fprintf(stderr, "[jme-profile] calc_ut calls=%llu total_s=%.6f per_call_s=%.9f\n",
        g_profile_calc_ut_calls,
        g_profile_calc_ut_seconds,
        g_profile_calc_ut_calls > 0ULL ? (g_profile_calc_ut_seconds / (double)g_profile_calc_ut_calls) : 0.0);
}

static void jme_profile_calc_ut_maybe_init(void)
{
    if (g_profile_calc_ut_init) {
        return;
    }
    g_profile_calc_ut_init = 1;
    g_profile_calc_ut_enabled = getenv("JME_PROFILE") != 0;
    if (g_profile_calc_ut_enabled) {
        atexit(jme_profile_calc_ut_report);
    }
}

static double angle_to_output_unit(double degrees, int flags)
{
    if (flags & JME_CALC_RADIANS) {
        return degrees * JME_DEG_TO_RAD;
    }

    return degrees;
}

static int state_is_finite(const double *state, int count)
{
    int i;

    if (state == 0) {
        return 0;
    }

    for (i = 0; i < count; i++) {
        if (!isfinite(state[i])) {
            return 0;
        }
    }

    return 1;
}

static int state_distance_is_usable(const double *state, double *distance, char *error)
{
    *distance = jme_state_distance(state);
    if (!isfinite(*distance) || *distance <= 0.0) {
        jme_set_error(error, "Calculation produced invalid distance");
        return JME_ERR;
    }

    return JME_OK;
}

static double vector_angle_degrees(const double *a, const double *b)
{
    double adotb = a[0] * b[0] + a[1] * b[1] + a[2] * b[2];
    double amag = sqrt(a[0] * a[0] + a[1] * a[1] + a[2] * a[2]);
    double bmag = sqrt(b[0] * b[0] + b[1] * b[1] + b[2] * b[2]);
    double c;

    if (!isfinite(amag) || !isfinite(bmag) || amag <= 0.0 || bmag <= 0.0) {
        return NAN;
    }

    c = adotb / (amag * bmag);
    if (c > 1.0) { c = 1.0; }
    if (c < -1.0) { c = -1.0; }
    return acos(c) * 180.0 / JME_PI;
}

static int apply_reference_frame_transform(double jd_et, int flags, int target_is_ecliptic, double *target_pos, char *error)
{
    if (!target_is_ecliptic && !(flags & JME_CALC_J2000)) {
        double bias_mat[9];
        double prec_mat[9];

        if (jme_get_frame_bias_matrix(jme_context_bias_model(), bias_mat) == JME_OK) {
            jme_matrix_transform_state(bias_mat, target_pos, target_pos);
        }

        jme_get_precession_matrix(2451545.0, jd_et, jme_context_precession_model(), prec_mat);
        jme_matrix_transform_state(prec_mat, target_pos, target_pos);

        if (!(flags & JME_CALC_NO_NUTATION)) {
            double dpsi, deps, eps;
            double nut_mat[9];
            jme_get_nutation(jd_et, jme_context_nutation_model(), &dpsi, &deps, error);
            jme_get_obliquity(jd_et, jme_context_obliquity_model(), &eps, error);
            jme_get_nutation_matrix(dpsi * JME_DEG_TO_RAD, deps * JME_DEG_TO_RAD, eps * JME_DEG_TO_RAD, nut_mat);
            jme_matrix_transform_state(nut_mat, target_pos, target_pos);
        }
    }

    return JME_OK;
}

static double body_equatorial_radius_km(int body)
{
    switch (body) {
    case JME_BODY_SUN: return 695700.0;
    case JME_BODY_MOON: return 1737.4;
    case JME_BODY_MERCURY: return 2439.7;
    case JME_BODY_VENUS: return 6051.8;
    case JME_BODY_MARS: return 3396.2;
    case JME_BODY_JUPITER: return 71492.0;
    case JME_BODY_SATURN: return 60268.0;
    case JME_BODY_URANUS: return 25559.0;
    case JME_BODY_NEPTUNE: return 24764.0;
    case JME_BODY_PLUTO: return 1188.3;
    default: return 0.0;
    }
}

static double apparent_magnitude_estimate(int body, double r, double delta, double phase_angle)
{
    double rd;

    if (!isfinite(r) || !isfinite(delta) || r <= 0.0 || delta <= 0.0 || !isfinite(phase_angle)) {
        return NAN;
    }

    rd = 5.0 * log10(r * delta);
    switch (body) {
    case JME_BODY_SUN:
        return -26.74;
    case JME_BODY_MOON:
        return -12.73 + 0.026 * phase_angle + 4.0e-9 * phase_angle * phase_angle * phase_angle * phase_angle;
    case JME_BODY_MERCURY:
        return -0.42 + rd + 0.0380 * phase_angle - 0.000273 * phase_angle * phase_angle + 0.000002 * phase_angle * phase_angle * phase_angle;
    case JME_BODY_VENUS:
        return -4.40 + rd + 0.0009 * phase_angle + 0.000239 * phase_angle * phase_angle - 0.00000065 * phase_angle * phase_angle * phase_angle;
    case JME_BODY_MARS:
        return -1.52 + rd + 0.016 * phase_angle;
    case JME_BODY_JUPITER:
        return -9.40 + rd + 0.005 * phase_angle;
    case JME_BODY_SATURN:
        return -8.88 + rd + 0.044 * phase_angle;
    case JME_BODY_URANUS:
        return -7.19 + rd;
    case JME_BODY_NEPTUNE:
        return -6.87 + rd;
    case JME_BODY_PLUTO:
        return -1.0 + rd;
    default:
        return NAN;
    }
}

static int lunar_node_spherical_state(double jd_et, int body, double *state)
{
    double t = (jd_et - 2451545.0) / 36525.0;
    double omega;
    double d;
    double m;
    double mp;
    double f;
    double speed;

    if (body != JME_BODY_MEAN_NODE && body != JME_BODY_TRUE_NODE) {
        return JME_ERR;
    }

    omega = 125.0445479
        - 1934.1362891 * t
        + 0.0020754 * t * t
        + (t * t * t) / 467441.0
        - (t * t * t * t) / 60616000.0;

    if (body == JME_BODY_TRUE_NODE) {
        d = 297.8501921 + 445267.1114034 * t - 0.0018819 * t * t + t * t * t / 545868.0 - t * t * t * t / 113065000.0;
        m = 357.5291092 + 35999.0502909 * t - 0.0001536 * t * t + t * t * t / 24490000.0;
        mp = 134.9633964 + 477198.8675055 * t + 0.0087414 * t * t + t * t * t / 69699.0 - t * t * t * t / 14712000.0;
        f = 93.2720950 + 483202.0175233 * t - 0.0036539 * t * t - t * t * t / 3526000.0 + t * t * t * t / 863310000.0;

        omega += -1.4979 * sin((2.0 * (d - f)) * JME_DEG_TO_RAD)
            - 0.1500 * sin(m * JME_DEG_TO_RAD)
            - 0.1226 * sin((2.0 * d) * JME_DEG_TO_RAD)
            + 0.1176 * sin((2.0 * f) * JME_DEG_TO_RAD)
            - 0.0801 * sin((2.0 * (mp - f)) * JME_DEG_TO_RAD);
    }

    speed = -1934.1362891 / 36525.0;

    if (state != 0) {
        state[0] = jme_degree_normalize(omega);
        state[1] = 0.0;
        state[2] = 1.0;
        state[3] = speed;
        state[4] = 0.0;
        state[5] = 0.0;
    }

    return JME_OK;
}

static int analytic_body_state(double jd_et, int body, double *state, int engine_policy)
{
    double pos[6];
    int rc;
    int i;

    if (body == JME_BODY_SUN) {
        if (state != 0) {
            for (i = 0; i < 6; i++) { state[i] = 0.0; }
        }
        return JME_OK;
    }

    if (body == JME_BODY_MOON) {
        double moon_geo[6];
        double earth_helio[6];
        if (jme_elp2000_moon_state(jd_et, moon_geo) != JME_OK) {
            if (jme_meeus_moon_state(jd_et, moon_geo) != JME_OK) {
                return JME_ERR;
            }
        }
        if (engine_policy == JME_ENGINE_VSOP_ELP_MEEUS) {
            if (jme_vsop87_planet_state(jd_et, JME_BODY_EARTH, earth_helio) != JME_OK
                && jme_meeus_planet_state(jd_et, JME_BODY_EARTH, earth_helio) != JME_OK) {
                return JME_ERR;
            }
        } else if (jme_moshier_planet_state(jd_et, JME_BODY_EARTH, earth_helio) != JME_OK
            && (engine_policy == JME_ENGINE_MOSHIER
                || (jme_vsop87_planet_state(jd_et, JME_BODY_EARTH, earth_helio) != JME_OK
                    && jme_meeus_planet_state(jd_et, JME_BODY_EARTH, earth_helio) != JME_OK))) {
            return JME_ERR;
        }
        if (state != 0) {
            for (i = 0; i < 6; i++) { state[i] = moon_geo[i] + earth_helio[i]; }
        }
        return JME_OK;
    }

    if (engine_policy == JME_ENGINE_VSOP_ELP_MEEUS) {
        rc = jme_vsop87_planet_state(jd_et, body, pos);
        if (rc != JME_OK) {
            rc = jme_meeus_planet_state(jd_et, body, pos);
        }
    } else {
        rc = jme_moshier_planet_state(jd_et, body, pos);
        if (rc != JME_OK && engine_policy != JME_ENGINE_MOSHIER) {
            rc = jme_vsop87_planet_state(jd_et, body, pos);
        }
        if (rc != JME_OK && engine_policy != JME_ENGINE_MOSHIER) {
            rc = jme_meeus_planet_state(jd_et, body, pos);
        }
    }
    if (rc != JME_OK) {
        return JME_ERR;
    }

    if (state != 0) {
        for (i = 0; i < 3; i++) {
            state[i] = pos[i];
            state[i + 3] = pos[i + 3];
        }
    }

    return JME_OK;
}

static int selected_body_state(
    double jd_et,
    int body,
    int center,
    int engine_policy,
    double *state,
    char *error,
    int *is_ecliptic
)
{
    int i;

    if (engine_policy == JME_ENGINE_AUTO || engine_policy == JME_ENGINE_JPL) {
        if (jme_jpl_body_state(jd_et, body, center, JME_VECTOR_AU_PER_DAY, state, error) == JME_OK) {
            if (is_ecliptic != 0) { *is_ecliptic = 0; }
            return JME_OK;
        }
        if (engine_policy == JME_ENGINE_JPL) {
            return JME_ERR;
        }
    }

    if (center != JME_BODY_SUN && center != JME_BODY_EARTH) {
        jme_set_error(error, "Selected analytical engine does not support requested center");
        return JME_ERR;
    }

    if (analytic_body_state(jd_et, body, state, engine_policy) != JME_OK) {
        jme_set_error(error, "Selected analytical engine cannot compute requested body");
        return JME_ERR;
    }

    if (center == JME_BODY_EARTH) {
        double earth_helio[6];
        if (analytic_body_state(jd_et, JME_BODY_EARTH, earth_helio, engine_policy) != JME_OK) {
            jme_set_error(error, "Selected analytical engine cannot compute Earth state");
            return JME_ERR;
        }
        for (i = 0; i < 6; i++) { state[i] -= earth_helio[i]; }
    }

    {
        double eps = 0.0;
        double equatorial[6];

        if (jme_get_obliquity(2451545.0, jme_context_obliquity_model(), &eps, error) != JME_OK
            || jme_ecliptic_to_equatorial_rectangular_state(state, eps, equatorial) != JME_OK) {
            jme_set_error(error, "Analytical ecliptic-to-equatorial frame conversion failed");
            return JME_ERR;
        }
        for (i = 0; i < 6; i++) {
            state[i] = equatorial[i];
        }
    }

    if (is_ecliptic != 0) { *is_ecliptic = 0; }
    return JME_OK;
}

static int calc_lunar_node(double jd_et, int body, int flags, double *results, char *error)
{
    double spherical[6];
    double rectangular[6];
    double eps;
    double equatorial[6];
    int i;

    if (lunar_node_spherical_state(jd_et, body, spherical) != JME_OK) {
        return JME_ERR;
    }

    if (flags & JME_CALC_SIDEREAL) {
        spherical[0] = jme_degree_normalize(spherical[0] - jme_get_ayanamsa(jd_et));
    }

    if (flags & JME_CALC_XYZ) {
        if (jme_spherical_to_rectangular_state(spherical, rectangular) != JME_OK) {
            jme_set_error(error, "Lunar node rectangular conversion failed");
            return JME_ERR;
        }

        if (flags & JME_CALC_EQUATORIAL) {
            jme_get_obliquity((flags & JME_CALC_J2000) ? 2451545.0 : jd_et, jme_context_obliquity_model(), &eps, error);
            if (jme_ecliptic_to_equatorial_rectangular_state(rectangular, eps, equatorial) != JME_OK) {
                jme_set_error(error, "Lunar node equatorial conversion failed");
                return JME_ERR;
            }
            for (i = 0; i < 6; i++) { results[i] = equatorial[i]; }
        } else {
            for (i = 0; i < 6; i++) { results[i] = rectangular[i]; }
        }
    } else {
        if (flags & JME_CALC_EQUATORIAL) {
            if (jme_spherical_to_rectangular_state(spherical, rectangular) != JME_OK) {
                jme_set_error(error, "Lunar node rectangular conversion failed");
                return JME_ERR;
            }
            jme_get_obliquity((flags & JME_CALC_J2000) ? 2451545.0 : jd_et, jme_context_obliquity_model(), &eps, error);
            if (jme_ecliptic_to_equatorial_rectangular_state(rectangular, eps, equatorial) != JME_OK
                || jme_rectangular_to_spherical_state(equatorial, spherical) != JME_OK) {
                jme_set_error(error, "Lunar node equatorial conversion failed");
                return JME_ERR;
            }
        }
        for (i = 0; i < 6; i++) { results[i] = spherical[i]; }
        results[0] = angle_to_output_unit(results[0], flags);
        results[1] = angle_to_output_unit(results[1], flags);
        results[3] = angle_to_output_unit(results[3], flags);
        results[4] = angle_to_output_unit(results[4], flags);
    }

    return JME_OK;
}

int jme_pheno_ut(double jd_ut, int body, int flags, double *attr, char *error)
{
    double dt = jme_delta_t(jd_ut);
    double jd_et = jd_ut + dt / 86400.0;
    return jme_pheno(jd_et, body, flags, attr, error);
}

int jme_calc_ut(double jd_ut, int body, int flags, double *results, char *error)
{
    double t0;
    double dt = jme_delta_t(jd_ut);
    double jd_et = jd_ut + dt / 86400.0;
    jme_profile_calc_ut_maybe_init();
    t0 = g_profile_calc_ut_enabled ? jme_profile_now_seconds_local() : 0.0;
    int rc = jme_calc(jd_et, body, flags, results, error);
    if (g_profile_calc_ut_enabled) {
        g_profile_calc_ut_calls += 1ULL;
        g_profile_calc_ut_seconds += (jme_profile_now_seconds_local() - t0);
    }
    return rc;
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
        if (flags & JME_CALC_SIDEREAL) {
            results[0] = jme_degree_normalize(results[0] - jme_get_ayanamsa(jd_et));
        }
        results[0] = angle_to_output_unit(results[0], flags);
        results[1] = angle_to_output_unit(results[1], flags);
        results[3] = angle_to_output_unit(results[3], flags);
        results[4] = angle_to_output_unit(results[4], flags);
    }

    return JME_OK;
}

int jme_orbit_max_min_true_distance(double jd_et, int body, int flags, double *tmax, double *tmin, double *dmax, double *dmin, char *error)
{
    static const double mu = 0.0002959122082855911025;
    double elem[20];
    double a, e, nu, E, M, n, t_peri, period;

    if (tmax == 0 || tmin == 0 || dmax == 0 || dmin == 0) {
        jme_set_error(error, "Orbit distance extrema outputs are required");
        return JME_ERR;
    }

    if (jme_get_orbital_elements(jd_et, body, flags, elem, error) != JME_OK) {
        return JME_ERR;
    }

    a = elem[0];
    e = elem[1];
    nu = elem[5] * JME_DEG_TO_RAD;

    if (!isfinite(a) || !isfinite(e) || a <= 0.0 || e < 0.0 || e >= 1.0) {
        jme_set_error(error, "Orbit distance extrema require an elliptical osculating orbit");
        return JME_ERR;
    }

    E = 2.0 * atan2(sqrt(1.0 - e) * sin(nu / 2.0), sqrt(1.0 + e) * cos(nu / 2.0));
    M = E - e * sin(E);
    n = sqrt(mu / (a * a * a));
    period = 2.0 * JME_PI / n;
    t_peri = jd_et - M / n;

    while (t_peri > jd_et + period / 2.0) { t_peri -= period; }
    while (t_peri < jd_et - period / 2.0) { t_peri += period; }

    *tmin = t_peri;
    *tmax = t_peri + period / 2.0;
    if (*tmax > jd_et + period / 2.0) {
        *tmax -= period;
    }
    *dmin = a * (1.0 - e);
    *dmax = a * (1.0 + e);

    return JME_OK;
}

int jme_pheno(double jd_et, int body, int flags, double *attr, char *error)
{
    double planet_geo[6], sun_geo[6], planet_helio[6];
    double planet_equ[6], sun_equ[6];
    double earth_from_body[3], sun_from_body[3];
    double r, delta, phase_angle, phase, elongation, radius_km, diameter_arcsec, magnitude;
    double light_time_days;
    double apparent_radius_arcsec;
    double phase_defect_arcsec;
    double bright_limb_pa;
    int i;

    if (attr == 0) {
        jme_set_error(error, "Physical phenomena output is required");
        return JME_ERR;
    }

    for (i = 0; i < 20; i++) { attr[i] = 0.0; }

    if (jme_calc(jd_et, body, flags | JME_CALC_XYZ | JME_CALC_TRUE_POSITION, planet_geo, error) != JME_OK) { return JME_ERR; }
    if (jme_calc(jd_et, JME_BODY_SUN, flags | JME_CALC_XYZ | JME_CALC_TRUE_POSITION, sun_geo, error) != JME_OK) { return JME_ERR; }
    if (jme_calc(jd_et, body, flags | JME_CALC_EQUATORIAL | JME_CALC_TRUE_POSITION, planet_equ, error) != JME_OK) { return JME_ERR; }
    if (jme_calc(jd_et, JME_BODY_SUN, flags | JME_CALC_EQUATORIAL | JME_CALC_TRUE_POSITION, sun_equ, error) != JME_OK) { return JME_ERR; }

    delta = jme_state_distance(planet_geo);
    if (body == JME_BODY_SUN) {
        r = 1.0;
        phase_angle = 0.0;
        phase = 1.0;
        elongation = 0.0;
    } else {
        if (jme_calc(jd_et, body, flags | JME_CALC_XYZ | JME_CALC_HELIOCENTRIC | JME_CALC_TRUE_POSITION, planet_helio, error) != JME_OK) { return JME_ERR; }
        r = jme_state_distance(planet_helio);

        for (i = 0; i < 3; i++) {
            earth_from_body[i] = -planet_geo[i];
            sun_from_body[i] = -planet_helio[i];
        }
        phase_angle = vector_angle_degrees(earth_from_body, sun_from_body);
        phase = (1.0 + cos(phase_angle * JME_DEG_TO_RAD)) / 2.0;
        elongation = vector_angle_degrees(planet_geo, sun_geo);
    }

    if (!isfinite(delta) || delta <= 0.0 || !isfinite(r) || r <= 0.0 || !isfinite(phase_angle) || !isfinite(phase) || !isfinite(elongation)) {
        jme_set_error(error, "Physical phenomena geometry is invalid");
        return JME_ERR;
    }

    radius_km = body_equatorial_radius_km(body);
    diameter_arcsec = radius_km > 0.0 ? 2.0 * asin(radius_km / (delta * JME_AU_KM)) * (180.0 / JME_PI) * 3600.0 : NAN;
    magnitude = apparent_magnitude_estimate(body, r, delta, phase_angle);
    light_time_days = delta / JME_C_AU_PER_DAY;
    apparent_radius_arcsec = diameter_arcsec / 2.0;
    phase_defect_arcsec = isfinite(diameter_arcsec) ? diameter_arcsec * (1.0 - phase) / 2.0 : NAN;

    if (body == JME_BODY_SUN) {
        bright_limb_pa = 0.0;
    } else {
        double ra_obj = planet_equ[0] * JME_DEG_TO_RAD;
        double dec_obj = planet_equ[1] * JME_DEG_TO_RAD;
        double ra_sun = sun_equ[0] * JME_DEG_TO_RAD;
        double dec_sun = sun_equ[1] * JME_DEG_TO_RAD;
        double dra = ra_sun - ra_obj;
        bright_limb_pa = atan2(
            cos(dec_sun) * sin(dra),
            sin(dec_sun) * cos(dec_obj) - cos(dec_sun) * sin(dec_obj) * cos(dra)
        ) * 180.0 / JME_PI;
        bright_limb_pa = jme_degree_normalize(bright_limb_pa);
    }

    attr[0] = phase_angle;
    attr[1] = phase;
    attr[2] = elongation;
    attr[3] = diameter_arcsec;
    attr[4] = magnitude;
    attr[5] = delta;
    attr[6] = r;
    attr[7] = light_time_days;
    attr[8] = apparent_radius_arcsec;
    attr[9] = phase_defect_arcsec;
    attr[10] = bright_limb_pa;

    return JME_OK;
}

int jme_calc(double jd_et, int body, int flags, double *results, char *error)
{
    double state[6];
    double target_pos[6];
    double observer_pos_au[3] = {0,0,0};
    double dist, light_time;
    int center = JME_BODY_EARTH;
    int target_is_ecliptic = 0;
    int engine_policy = jme_context_engine_policy();
    int frame_already_applied = 0;
    int observer_pos_ready = 0;
    int i;

    if (results == 0) {
        jme_set_error(error, "Output buffer is required");
        return JME_ERR;
    }

    for (i = 0; i < 6; i++) { results[i] = 0.0; }

    if (body == JME_BODY_MEAN_NODE || body == JME_BODY_TRUE_NODE) {
        if (calc_lunar_node(jd_et, body, flags, results, error) != JME_OK) {
            return JME_ERR;
        }
        return state_is_finite(results, 6) ? JME_OK : JME_ERR;
    }

    if (flags & JME_CALC_BARYCENTRIC) {
        center = JME_BODY_SOLAR_SYSTEM_BARYCENTER;
    } else if (flags & JME_CALC_HELIOCENTRIC) {
        center = JME_BODY_SUN;
    } else if ((flags & JME_CALC_TOPOCENTRIC)
        && !(flags & JME_CALC_TRUE_POSITION)
        && !(flags & JME_CALC_J2000)) {
        if (jme_get_topo_pos(jd_et, observer_pos_au, error) != JME_OK) {
            return JME_ERR;
        }
        observer_pos_ready = 1;
    }

    /* 1. Geometric position (Target - Center) at t */
    if (selected_body_state(jd_et, body, center, engine_policy, target_pos, error, &target_is_ecliptic) != JME_OK) {
        return JME_ERR;
    }

    /* Apply topocentric correction to geometric position if needed */
    if ((flags & JME_CALC_TOPOCENTRIC) && observer_pos_ready) {
        for (i = 0; i < 3; i++) { target_pos[i] -= observer_pos_au[i]; }
    }

    /* 2. Light-time correction */
    if (!(flags & JME_CALC_TRUE_POSITION)) {
        if (state_distance_is_usable(target_pos, &dist, error) != JME_OK) {
            return JME_ERR;
        }
        light_time = dist / JME_C_AU_PER_DAY;

        if (selected_body_state(jd_et - light_time, body, center, engine_policy, target_pos, error, &target_is_ecliptic) != JME_OK) {
            return JME_ERR;
        }

        /* Second iteration */
        if (state_distance_is_usable(target_pos, &dist, error) != JME_OK) {
            return JME_ERR;
        }
        light_time = dist / JME_C_AU_PER_DAY;
        if (selected_body_state(jd_et - light_time, body, center, engine_policy, target_pos, error, &target_is_ecliptic) != JME_OK) {
            return JME_ERR;
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
                if (engine_policy == JME_ENGINE_JPL || analytic_body_state(jd_et, JME_BODY_EARTH, earth_state, engine_policy) != JME_OK) {
                    return JME_ERR;
                }
            }

            for (i = 0; i < 3; i++) {
                v_obs[i] = earth_state[i + 3] / JME_C_AU_PER_DAY;
                v2 += v_obs[i] * v_obs[i];
            }

            if (state_distance_is_usable(target_pos, &dist, error) != JME_OK) {
                return JME_ERR;
            }
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
                if (state_distance_is_usable(sun_state, &sun_dist, error) != JME_OK) {
                    return JME_ERR;
                }
                for (i = 0; i < 3; i++) { e[i] = sun_state[i] / sun_dist; }

                if (state_distance_is_usable(target_pos, &dist, error) != JME_OK) {
                    return JME_ERR;
                }
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
                if (!isfinite(dist) || dist <= 0.0) {
                    jme_set_error(error, "Calculation produced invalid light-deflection vector");
                    return JME_ERR;
                }
                dist = jme_state_distance(target_pos) / dist;
                for (i = 0; i < 3; i++) { target_pos[i] = u_prime[i] * dist; }
            }
        }
    }

    if ((flags & JME_CALC_TOPOCENTRIC)
        && (flags & JME_CALC_TRUE_POSITION)
        && !(flags & JME_CALC_J2000)
        && !target_is_ecliptic) {
        if (apply_reference_frame_transform(jd_et, flags, target_is_ecliptic, target_pos, error) != JME_OK) {
            return JME_ERR;
        }
        if (jme_get_topo_pos_true_equator(jd_et, observer_pos_au, error) != JME_OK) {
            return JME_ERR;
        }
        for (i = 0; i < 3; i++) {
            target_pos[i] -= observer_pos_au[i];
        }
        frame_already_applied = 1;
        observer_pos_ready = 1;
    }

    /* 3. Reference Frame / Transformation */
    if (!frame_already_applied) {
        if (apply_reference_frame_transform(jd_et, flags, target_is_ecliptic, target_pos, error) != JME_OK) {
            return JME_ERR;
        }
    }

    if (flags & JME_CALC_XYZ) {
        /* Return rectangular */
        for (i = 0; i < 6; i++) { results[i] = target_pos[i]; }
    } else {
        /* Convert to spherical (default is Ecliptic if not JME_CALC_EQUATORIAL) */
        if (flags & JME_CALC_EQUATORIAL) {
            if (target_is_ecliptic) {
                double equatorial_rect[6];
                double eps;
                double jd_for_obl = (flags & JME_CALC_J2000) ? 2451545.0 : jd_et;
                jme_get_obliquity(jd_for_obl, jme_context_obliquity_model(), &eps, error);
                jme_ecliptic_to_equatorial_rectangular_state(target_pos, eps, equatorial_rect);
                jme_rectangular_to_spherical_state(equatorial_rect, state);
            } else {
                jme_rectangular_to_spherical_state(target_pos, state);
            }
        } else {
            if (target_is_ecliptic) {
                jme_rectangular_to_spherical_state(target_pos, state);
            } else {
                /* Convert to ecliptic then spherical */
                double ecliptic_rect[6];
                double eps;
                double jd_for_obl = (flags & JME_CALC_J2000) ? 2451545.0 : jd_et;
                jme_get_obliquity(jd_for_obl, jme_context_obliquity_model(), &eps, error);
                
                /* If not J2000, we need the True Obliquity for True Ecliptic */
                if (!(flags & JME_CALC_J2000) && !(flags & JME_CALC_NO_NUTATION)) {
                    double dpsi, deps;
                    jme_get_nutation(jd_et, jme_context_nutation_model(), &dpsi, &deps, error);
                    eps += deps;
                }

                jme_equatorial_to_ecliptic_rectangular_state(target_pos, eps, ecliptic_rect);
                jme_rectangular_to_spherical_state(ecliptic_rect, state);
            }
        }

        for (i = 0; i < 6; i++) { results[i] = state[i]; }

        if (flags & JME_CALC_SIDEREAL) {
            results[0] = jme_degree_normalize(results[0] - jme_get_ayanamsa(jd_et));
        }

        results[0] = angle_to_output_unit(results[0], flags);
        results[1] = angle_to_output_unit(results[1], flags);
        results[3] = angle_to_output_unit(results[3], flags);
        results[4] = angle_to_output_unit(results[4], flags);
    }

    if (flags & JME_CALC_DISTANCE_KM) {
        if (flags & JME_CALC_XYZ) {
            for (i = 0; i < 6; i++) { results[i] *= JME_AU_KM; }
        } else {
            results[2] *= JME_AU_KM;
            results[5] *= JME_AU_KM;
        }
    }

    if (flags & JME_CALC_VELOCITY_PER_SECOND) {
        if (flags & JME_CALC_XYZ) {
            for (i = 3; i < 6; i++) { results[i] /= JME_SECONDS_PER_DAY; }
        } else {
            results[3] /= JME_SECONDS_PER_DAY;
            results[4] /= JME_SECONDS_PER_DAY;
            results[5] /= JME_SECONDS_PER_DAY;
        }
    }

    if (!state_is_finite(results, 6)) {
        for (i = 0; i < 6; i++) { results[i] = 0.0; }
        jme_set_error(error, "Calculation produced non-finite output");
        return JME_ERR;
    }

    return JME_OK;
}
