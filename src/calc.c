#include "jme/jme.h"
#include "context.h"
#include "jme_erfa_vendor.h"
#include <math.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#ifdef _WIN32
#include <windows.h>
#else
#include <sys/time.h>
#endif

#define JME_DEG_TO_RAD 0.017453292519943295769236907684886127134428718885417
#define JME_PI 3.14159265358979323846264338327950288419716939937510

extern int jme_internal_elp2000_moon_position(double jd_et, double *results);
extern int jme_internal_moshier_moon_position(double jd_et, double *results);
extern int jme_internal_moshier_moon_state(double jd_et, double *results);
extern int jme_internal_moshier_planet_position(double jd_et, int body, double *results);

static unsigned long long g_profile_calc_ut_calls = 0ULL;
static double g_profile_calc_ut_seconds = 0.0;
static int g_profile_calc_ut_enabled = 0;
static int g_profile_calc_ut_init = 0;

static double jme_profile_now_seconds_local(void)
{
#ifdef _WIN32
    LARGE_INTEGER frequency;
    LARGE_INTEGER counter;
    QueryPerformanceFrequency(&frequency);
    QueryPerformanceCounter(&counter);
    return (double)counter.QuadPart / (double)frequency.QuadPart;
#else
    struct timeval tv;
    gettimeofday(&tv, 0);
    return (double)tv.tv_sec + ((double)tv.tv_usec / 1000000.0);
#endif
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

/*
 * Heliocentric planetary state (AU, AU/day, J2000 ecliptic) from exactly one
 * published theory per engine; there is no silent fallback to another model:
 *   MOSHIER and AUTO (analytical case): Moshier plan404 (DE404 fit) for
 *     Mercury..Pluto and Earth;
 *   VSOP_ELP_MEEUS: VSOP87A (Bretagnon & Francou 1988) for Mercury..Neptune
 *     and Earth; VSOP87 has no Pluto, so Pluto uses Moshier plan404.
 * (The approximate Keplerian elements behind jme_meeus_planet_state are no
 * longer used by jme_calc; they remain available as a separate API.)
 */
static int analytic_body_state(double jd_et, int body, double *state, int engine_policy, int with_velocity)
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
    if (body < JME_BODY_MERCURY || body > JME_BODY_EARTH || body == JME_BODY_MOON) {
        return JME_ERR;
    }
    if (engine_policy == JME_ENGINE_VSOP_ELP_MEEUS && body != JME_BODY_PLUTO) {
        rc = jme_vsop87_planet_state(jd_et, body, pos);
    } else if (with_velocity) {
        rc = jme_moshier_planet_state(jd_et, body, pos);
    } else {
        rc = jme_internal_moshier_planet_position(jd_et, body, pos);
    }
    if (rc != JME_OK) {
        return JME_ERR;
    }
    for (i = 0; i < 6; i++) {
        if (!isfinite(pos[i])) return JME_ERR;
    }
    if (state != 0) {
        for (i = 0; i < 6; i++) state[i] = pos[i];
    }
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
        double ayan, rate;
        if ((flags & JME_CALC_EQUATORIAL) || (flags & JME_CALC_J2000) || (flags & JME_CALC_ICRS)) {
            jme_set_error(error, "JME_CALC_SIDEREAL is defined only for ecliptic-of-date output");
            return JME_ERR;
        }
        /* Node longitudes are referred to the mean equinox of date: subtract the mean ayanamsa. */
        if (internal_context_ayanamsa(jd_et, &ayan, &rate, error) != JME_OK) {
            return JME_ERR;
        }
        spherical[0] = jme_degree_normalize(spherical[0] - ayan);
        spherical[3] -= rate;
    }

    if (flags & JME_CALC_XYZ) {
        if (jme_spherical_to_rectangular_state(spherical, rectangular) != JME_OK) {
            jme_set_error(error, "Lunar node rectangular conversion failed");
            return JME_ERR;
        }

        if (flags & JME_CALC_EQUATORIAL) {
            if (jme_get_obliquity((flags & JME_CALC_J2000) ? 2451545.0 : jd_et, jme_context_obliquity_model(), &eps, error) != JME_OK) {
                return JME_ERR;
            }
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
            if (jme_get_obliquity((flags & JME_CALC_J2000) ? 2451545.0 : jd_et, jme_context_obliquity_model(), &eps, error) != JME_OK) {
                return JME_ERR;
            }
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
    double jd_et;
    int i;
    if (attr == 0) {
        jme_set_error(error, "Physical phenomena output is required");
        return JME_ERR;
    }
    for (i=0;i<20;i++) attr[i]=0.0;
    if (internal_ut_to_et(jd_ut, &jd_et, error) != JME_OK) return JME_ERR;
    return jme_pheno(jd_et, body, flags, attr, error);
}

int jme_calc_ut(double jd_ut, int body, int flags, double *results, char *error)
{
    double t0, jd_et;
    int rc, i;
    if (results == 0) {
        jme_set_error(error, "Output buffer is required");
        return JME_ERR;
    }
    for (i=0;i<6;i++) results[i]=0.0;
    if (internal_ut_to_et(jd_ut, &jd_et, error) != JME_OK) return JME_ERR;
    jme_profile_calc_ut_maybe_init();
    t0 = g_profile_calc_ut_enabled ? jme_profile_now_seconds_local() : 0.0;
    rc = jme_calc(jd_et, body, flags, results, error);
    if (g_profile_calc_ut_enabled) {
        g_profile_calc_ut_calls += 1ULL;
        g_profile_calc_ut_seconds += jme_profile_now_seconds_local() - t0;
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

    for (i = 0; i < 6; i++) results[i] = 0.0;
    if (body == center) {
        jme_set_error(error, "Target and centre are the same body");
        return JME_ERR;
    }
    /*
     * Geometric planet-centric vector (no light time, deflection or
     * aberration): difference of heliocentric states in the requested frame.
     * JME_CALC_SIDEREAL is applied by jme_calc as a rotation of the vectors.
     */
    if (body == JME_BODY_SUN) {
        for (i = 0; i < 6; i++) body_state[i] = 0.0;
    } else if (jme_calc(jd_et, body, flags | JME_CALC_HELIOCENTRIC | JME_CALC_XYZ | JME_CALC_TRUE_POSITION, body_state, error) != JME_OK) {
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
            for (i = 0; i < 6; i++) results[i] = 0.0;
            jme_set_error(error, "Rectangular-to-spherical conversion failed");
            return JME_ERR;
        }
        for (i = 0; i < 6; i++) {
            results[i] = spherical[i];
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
    if (!isfinite(jd_et)) {
        jme_set_error(error, "Terrestrial-time Julian date must be finite");
        return JME_ERR;
    }

    if (internal_calc_positions(jd_et, body, flags | JME_CALC_XYZ | JME_CALC_TRUE_POSITION, planet_geo, error) != JME_OK) { return JME_ERR; }
    if (internal_calc_positions(jd_et, JME_BODY_SUN, flags | JME_CALC_XYZ | JME_CALC_TRUE_POSITION, sun_geo, error) != JME_OK) { return JME_ERR; }
    if (internal_calc_positions(jd_et, body, flags | JME_CALC_EQUATORIAL | JME_CALC_TRUE_POSITION, planet_equ, error) != JME_OK) { return JME_ERR; }
    if (internal_calc_positions(jd_et, JME_BODY_SUN, flags | JME_CALC_EQUATORIAL | JME_CALC_TRUE_POSITION, sun_equ, error) != JME_OK) { return JME_ERR; }

    delta = jme_state_distance(planet_geo);
    if (body == JME_BODY_SUN) {
        r = 1.0;
        phase_angle = 0.0;
        phase = 1.0;
        elongation = 0.0;
    } else {
        if (internal_calc_positions(jd_et, body, flags | JME_CALC_XYZ | JME_CALC_HELIOCENTRIC | JME_CALC_TRUE_POSITION, planet_helio, error) != JME_OK) { return JME_ERR; }
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

/*
 * ---------------------------------------------------------------------------
 * Geometric state providers.
 *
 * All internal vectors use J2000-mean-equator axes:
 *   JPL backend:        ICRF axes, origin = solar-system barycentre (SSB).
 *   Analytical engines: dynamical J2000 mean equator (VSOP87/ELP2000/Moshier
 *                       J2000 ecliptic rotated by the J2000 mean obliquity of
 *                       the context obliquity model), origin = Sun centre.
 * One backend is chosen per jme_calc() call so that target and observer are
 * never mixed between engines.
 * ---------------------------------------------------------------------------
 */
#define JME_BACKEND_JPL 1
#define JME_BACKEND_ANALYTIC 2

static int ecliptic_j2000_to_equatorial_j2000(double *state, char *error)
{
    double eps = 0.0;
    double equatorial[6];
    int i;

    if (jme_get_obliquity(2451545.0, jme_context_obliquity_model(), &eps, error) != JME_OK
        || jme_ecliptic_to_equatorial_rectangular_state(state, eps, equatorial) != JME_OK) {
        if (error != 0 && error[0] == '\0') jme_set_error(error, "Analytical ecliptic-to-equatorial frame conversion failed");
        return JME_ERR;
    }
    for (i = 0; i < 6; i++) state[i] = equatorial[i];
    return JME_OK;
}

/* Geocentric Moon from the analytical engines, J2000 mean equatorial, AU, AU/day. */
static int analytic_moon_geocentric_j2000_uncached(double jd_et, int engine_policy, int with_velocity, double *state, char *error)
{
    double moon[6];
    int i;

    if (engine_policy == JME_ENGINE_MOSHIER || engine_policy == JME_ENGINE_AUTO) {
        /*
         * Moshier's lunar theory returns geocentric coordinates referred to
         * the mean ecliptic and equinox of date. Rotate to the mean equator of
         * date with the mean obliquity of date, then precess back to J2000.
         * This is part of the theory's own reference frame, so it uses a fixed
         * model (IAU 2006 / P03 precession and obliquity, the modern
         * equivalent of the precession Moshier fitted against) and does NOT
         * follow the user's precession/obliquity selection: the geometric Moon
         * must not change when only the output-frame models change.
         */
        double eps = 0.0, eq_date[6], prec[9], inv[9];
        int r, c;
        if ((with_velocity ? jme_internal_moshier_moon_state(jd_et, moon)
                           : jme_internal_moshier_moon_position(jd_et, moon)) != JME_OK) {
            jme_set_error(error, "Moshier engine cannot compute Moon state");
            return JME_ERR;
        }
        if (jme_get_obliquity(jd_et, JME_MODEL_OBL_IAU_2006, &eps, error) != JME_OK
            || jme_ecliptic_to_equatorial_rectangular_state(moon, eps, eq_date) != JME_OK) {
            return JME_ERR;
        }
        if (jme_get_precession_matrix(2451545.0, jd_et, JME_MODEL_PREC_IAU_2006, prec) != JME_OK) {
            jme_set_error(error, "Unsupported precession model");
            return JME_ERR;
        }
        for (r = 0; r < 3; r++) for (c = 0; c < 3; c++) inv[r * 3 + c] = prec[c * 3 + r];
        jme_matrix_transform_state(inv, eq_date, state);
        return JME_OK;
    }

    if ((with_velocity ? jme_elp2000_moon_state(jd_et, moon)
                       : jme_internal_elp2000_moon_position(jd_et, moon)) != JME_OK) {
        jme_set_error(error, "ELP2000-82B cannot compute Moon state");
        return JME_ERR;
    }
    for (i = 0; i < 6; i++) state[i] = moon[i];
    return ecliptic_j2000_to_equatorial_j2000(state, error);
}

/* Heliocentric J2000 mean-equatorial state from the analytical engines. */
/*
 * Exact-input memoisation of analytical evaluations (thread-local, lossless:
 * a hit requires bit-identical date, body, engine, frame models, and the
 * velocity flag). A velocity entry may satisfy a position-only request:
 * its position components are the position-only result, and the returned
 * rates are cleared. A position-only entry must not satisfy a velocity
 * request: aberration and the solar-system barycentre read those components.
 * One apparent position evaluates the same bodies at the same instants several
 * times (observer, Sun barycentre, deflection, rate stencil).
 */
#define JME_ANALYTIC_CACHE_SIZE 64
typedef struct analytic_cache_entry {
    double jd;
    int body;
    int engine;
    int obl;
    int prec;
    int vel;
    int valid;
    double state[6];
} analytic_cache_entry;
static JME_THREAD_LOCAL analytic_cache_entry g_analytic_cache[JME_ANALYTIC_CACHE_SIZE];
static JME_THREAD_LOCAL int g_analytic_cache_next;

static int analytic_cache_get(double jd, int body, int engine, int with_velocity, double *state)
{
    int i, k, obl = jme_context_obliquity_model(), prec = jme_context_precession_model();
    for (i = 0; i < JME_ANALYTIC_CACHE_SIZE; i++) {
        const analytic_cache_entry *e = &g_analytic_cache[i];
        if (e->valid && e->body == body && e->engine == engine
            && e->obl == obl && e->prec == prec
            && memcmp(&e->jd, &jd, sizeof(double)) == 0
            && (e->vel == with_velocity || (e->vel && !with_velocity))) {
            if (with_velocity || !e->vel) {
                for (k = 0; k < 6; k++) state[k] = e->state[k];
            } else {
                for (k = 0; k < 3; k++) state[k] = e->state[k];
                state[3] = state[4] = state[5] = 0.0;
            }
            return 1;
        }
    }
    return 0;
}

static void analytic_cache_put(double jd, int body, int engine, int with_velocity, const double *state)
{
    analytic_cache_entry *e = &g_analytic_cache[g_analytic_cache_next];
    int k;
    g_analytic_cache_next = (g_analytic_cache_next + 1) % JME_ANALYTIC_CACHE_SIZE;
    e->jd = jd;
    e->body = body;
    e->engine = engine;
    e->obl = jme_context_obliquity_model();
    e->prec = jme_context_precession_model();
    e->vel = with_velocity;
    for (k = 0; k < 6; k++) e->state[k] = state[k];
    e->valid = 1;
}

#define JME_CACHE_MOON_GEO (-1000)

static int analytic_moon_geocentric_j2000(double jd_et, int engine_policy, int with_velocity, double *state, char *error)
{
    if (analytic_cache_get(jd_et, JME_CACHE_MOON_GEO, engine_policy, with_velocity, state)) return JME_OK;
    if (analytic_moon_geocentric_j2000_uncached(jd_et, engine_policy, with_velocity, state, error) != JME_OK) return JME_ERR;
    analytic_cache_put(jd_et, JME_CACHE_MOON_GEO, engine_policy, with_velocity, state);
    return JME_OK;
}

static int analytic_heliocentric_state_uncached(double jd_et, int body, int engine_policy, int with_velocity, double *state, char *error);

static int analytic_heliocentric_state(double jd_et, int body, int engine_policy, int with_velocity, double *state, char *error)
{
    if (analytic_cache_get(jd_et, body, engine_policy, with_velocity, state)) return JME_OK;
    if (analytic_heliocentric_state_uncached(jd_et, body, engine_policy, with_velocity, state, error) != JME_OK) return JME_ERR;
    analytic_cache_put(jd_et, body, engine_policy, with_velocity, state);
    return JME_OK;
}

static int analytic_heliocentric_state_uncached(double jd_et, int body, int engine_policy, int with_velocity, double *state, char *error)
{
    int i;

    if (body == JME_BODY_SUN) {
        for (i = 0; i < 6; i++) state[i] = 0.0;
        return JME_OK;
    }
    if (body == JME_BODY_MOON) {
        double earth[6], moon[6];
        if (analytic_heliocentric_state(jd_et, JME_BODY_EARTH, engine_policy, with_velocity, earth, error) != JME_OK
            || analytic_moon_geocentric_j2000(jd_et, engine_policy, with_velocity, moon, error) != JME_OK) {
            return JME_ERR;
        }
        for (i = 0; i < 6; i++) state[i] = earth[i] + moon[i];
        return JME_OK;
    }
    if (body < JME_BODY_MERCURY || body > JME_BODY_EARTH) {
        jme_set_error(error, "Selected analytical engine cannot compute requested body");
        return JME_ERR;
    }
    if (analytic_body_state(jd_et, body, state, engine_policy, with_velocity) != JME_OK) {
        jme_set_error(error, "Selected analytical engine cannot compute requested body");
        return JME_ERR;
    }
    return ecliptic_j2000_to_equatorial_j2000(state, error);
}

/* Barycentric (JPL) or heliocentric (analytical) state of body. */
static int backend_state(int backend, double jd_et, int body, int engine_policy, int with_velocity, double *state, char *error)
{
    if (backend == JME_BACKEND_JPL) {
        return internal_jpl_body_state_tt(jd_et, body, JME_BODY_SOLAR_SYSTEM_BARYCENTER,
            JME_VECTOR_AU_PER_DAY, state, error);
    }
    if (body == JME_BODY_SOLAR_SYSTEM_BARYCENTER) {
        jme_set_error(error, "Analytical engines do not provide the solar-system barycentre");
        return JME_ERR;
    }
    if (body > JME_BODY_EARTH && body != JME_BODY_SOLAR_SYSTEM_BARYCENTER) {
        jme_set_error(error, "Selected analytical engine does not support requested body or centre");
        return JME_ERR;
    }
    {
        /* The analytical theories (VSOP87, Moshier/DE404 fits, ELP 2000-82B) use TDB. */
        double jd_tdb;
        if (internal_tt_to_tdb_jd(jd_et, &jd_tdb, error) != JME_OK) return JME_ERR;
        return analytic_heliocentric_state(jd_tdb, body, engine_policy, with_velocity, state, error);
    }
}

/*
 * State at TT = jd_et + offset_days (offset = minus a light time). The JPL
 * path keeps the offset in the day fraction (no rounding to the binary64
 * spacing of a full Julian date). The analytical theories take one TDB date,
 * so for them jd_et + offset is rounded to that spacing (<= 20 microseconds,
 * < 1 m for any body: far below the theories' own accuracy).
 */
static int backend_state_offset(int backend, double jd_et, double offset_days, int body, int engine_policy, int with_velocity, double *state, char *error)
{
    if (backend == JME_BACKEND_JPL) {
        return internal_jpl_body_state_tt_offset(jd_et, offset_days, body, JME_BODY_SOLAR_SYSTEM_BARYCENTER,
            JME_VECTOR_AU_PER_DAY, state, error);
    }
    return backend_state(backend, jd_et + offset_days, body, engine_policy, with_velocity, state, error);
}

/* Choose the backend for one calculation; AUTO uses JPL only if it can serve the target. */
static int choose_backend(double jd_et, int body, int engine_policy, int *backend, char *error)
{
    if (engine_policy == JME_ENGINE_JPL) {
        *backend = JME_BACKEND_JPL;
        return JME_OK;
    }
    if (engine_policy == JME_ENGINE_AUTO) {
        double probe[6];
        char ignored[256];
        if (internal_jpl_body_state_tt(jd_et, body, JME_BODY_SOLAR_SYSTEM_BARYCENTER,
                JME_VECTOR_AU_PER_DAY, probe, ignored) == JME_OK
            && internal_jpl_body_state_tt(jd_et, JME_BODY_EARTH, JME_BODY_SOLAR_SYSTEM_BARYCENTER,
                JME_VECTOR_AU_PER_DAY, probe, ignored) == JME_OK) {
            *backend = JME_BACKEND_JPL;
            return JME_OK;
        }
    }
    (void)error;
    *backend = JME_BACKEND_ANALYTIC;
    return JME_OK;
}

int jme_calc_resolve_engine(double jd_et, int body, int *engine_id, int *jpl_center_substituted, char *error)
{
    int policy = jme_context_engine_policy();
    int backend;
    double probe[6];

    if (engine_id != 0) *engine_id = 0;
    if (jpl_center_substituted != 0) *jpl_center_substituted = 0;
    if (engine_id == 0) {
        jme_set_error(error, "engine_id output is required");
        return JME_ERR;
    }
    if (!isfinite(jd_et)) {
        jme_set_error(error, "Julian date must be finite");
        return JME_ERR;
    }
    if (choose_backend(jd_et, body, policy, &backend, error) != JME_OK) return JME_ERR;
    if (backend == JME_BACKEND_JPL) {
        if (internal_jpl_body_state_tt(jd_et, body, JME_BODY_SOLAR_SYSTEM_BARYCENTER,
                JME_VECTOR_AU_PER_DAY, probe, error) != JME_OK) {
            return JME_ERR;
        }
        if (jpl_center_substituted != 0) *jpl_center_substituted = internal_jpl_last_center_substituted();
        *engine_id = JME_ENGINE_ID_JPL;
    } else {
        *engine_id = policy == JME_ENGINE_VSOP_ELP_MEEUS ? JME_ENGINE_ID_VSOP87_ELP82B : JME_ENGINE_ID_MOSHIER;
    }
    jme_set_error(error, "");
    return JME_OK;
}

/*
 * Barycentric state of the Sun for the analytical engines, from the
 * heliocentric states of the planets of the same engine:
 *   r_sun,bary = -sum(mu_i r_i) / (1 + sum(mu_i)),  mu_i = m_i / M_sun
 * Mass ratios: IAU 2009 System of Astronomical Constants (Luzum et al. 2011,
 * Celest. Mech. Dyn. Astr. 110, 293), Table 1 (Sun/planet-system mass ratios;
 * Moon/Earth 0.0123000371 for the Earth-Moon barycentre). Pluto is included
 * whenever the selected engine provides it at that date (its contribution to
 * the observer velocity is < 1 microarcsecond of aberration).
 * jd_tdb is TDB. Output: AU, AU/day, analytical J2000 equatorial axes.
 */
static int analytic_sun_barycentric(double jd_tdb, int engine_policy, double *sun, char *error)
{
    static const int bodies[9] = {
        JME_BODY_MERCURY, JME_BODY_VENUS, JME_BODY_EARTH, JME_BODY_MARS, JME_BODY_JUPITER,
        JME_BODY_SATURN, JME_BODY_URANUS, JME_BODY_NEPTUNE, JME_BODY_PLUTO
    };
    static const double inv_mass[9] = {
        6023597.400017, 408523.718655, 328900.559708, 3098703.590267, 1047.348644,
        3497.901768, 22902.981613, 19412.237346, 135836683.767599
    };
    const double moon_earth = 0.0123000371;
    double acc[6] = {0, 0, 0, 0, 0, 0};
    double musum = 0.0;
    int b, i;

    for (b = 0; b < 9; b++) {
        double st[6];
        double mu = 1.0 / inv_mass[b];
        if (analytic_heliocentric_state(jd_tdb, bodies[b], engine_policy, 1, st, error) != JME_OK) {
            if (bodies[b] == JME_BODY_PLUTO) {
                if (error != 0) error[0] = '\0';
                continue;
            }
            return JME_ERR;
        }
        if (bodies[b] == JME_BODY_EARTH) {
            double moon[6];
            if (analytic_moon_geocentric_j2000(jd_tdb, engine_policy, 1, moon, error) != JME_OK) return JME_ERR;
            for (i = 0; i < 6; i++) st[i] += moon[i] * moon_earth / (1.0 + moon_earth);
        }
        for (i = 0; i < 6; i++) acc[i] += mu * st[i];
        musum += mu;
    }
    for (i = 0; i < 6; i++) sun[i] = -acc[i] / (1.0 + musum);
    return JME_OK;
}

/*
 * Observer (Earth centre + optional topocentric offset) state for aberration,
 * in the backend frame. vel_bary receives the observer velocity used for
 * aberration (AU/day).
 */
static int observer_state(int backend, double jd_et, int topocentric, int engine_policy,
    double *obs, double *vel_bary, char *error)
{
    int i;

    if (backend_state(backend, jd_et, JME_BODY_EARTH, engine_policy, vel_bary != 0, obs, error) != JME_OK) {
        return JME_ERR;
    }
    if (vel_bary != 0) {
        /*
         * JPL: barycentric velocity directly. Analytical engines: heliocentric
         * Earth velocity plus the Sun's barycentric velocity from the same
         * engine's planets (analytic_sun_barycentric).
         */
        for (i = 0; i < 3; i++) vel_bary[i] = obs[i + 3];
        if (backend == JME_BACKEND_ANALYTIC) {
            double sun[6], jd_tdb;
            if (internal_tt_to_tdb_jd(jd_et, &jd_tdb, error) != JME_OK
                || analytic_sun_barycentric(jd_tdb, engine_policy, sun, error) != JME_OK) {
                return JME_ERR;
            }
            for (i = 0; i < 3; i++) vel_bary[i] += sun[i + 3];
        }
    }
    if (topocentric) {
        double topo[6];
        if (internal_topo_state_j2000(jd_et, topo, error) != JME_OK) {
            return JME_ERR;
        }
        for (i = 0; i < 3; i++) {
            obs[i] += topo[i];
            obs[i + 3] += topo[i + 3];
            if (vel_bary != 0) vel_bary[i] += topo[i + 3];
        }
    }
    return JME_OK;
}

int internal_earth_aberration_state(double jd_et, double *state, char *error)
{
    int engine_policy = jme_context_engine_policy();
    int backend;
    double vel[3];
    int i;

    if (state == 0) {
        jme_set_error(error, "Earth observer state output is required");
        return JME_ERR;
    }
    if (error != 0) error[0] = '\0';
    if (choose_backend(jd_et, JME_BODY_EARTH, engine_policy, &backend, error) != JME_OK
        || observer_state(backend, jd_et, 0, engine_policy, state, vel, error) != JME_OK) {
        if (error != 0 && error[0] == '\0') {
            jme_set_error(error, "Selected engine cannot compute Earth observer velocity for aberration");
        }
        return JME_ERR;
    }
    for (i = 0; i < 3; i++) state[i + 3] = vel[i];
    return JME_OK;
}

/*
 * Relativistic stellar aberration (ERFA eraAb, Klioner 2003 / Explanatory
 * Supplement 3rd ed. eq. 7.40). p: unit direction (in/out), v: observer
 * barycentric velocity in AU/day, sun_dist: observer-Sun distance in AU.
 */
static int apply_aberration(double *p, const double *v_au_per_day, double sun_dist, char *error)
{
    double v[3], pnat[3], ppr[3];
    double v2 = 0.0, bm1;
    int i;

    for (i = 0; i < 3; i++) {
        v[i] = v_au_per_day[i] / JME_C_AU_PER_DAY;
        v2 += v[i] * v[i];
        pnat[i] = p[i];
    }
    if (!(v2 < 1.0) || !isfinite(sun_dist) || sun_dist <= 0.0) {
        jme_set_error(error, "Observer velocity or Sun distance is invalid for aberration");
        return JME_ERR;
    }
    bm1 = sqrt(1.0 - v2);
    jme_eraAb(pnat, v, sun_dist, bm1, ppr);
    for (i = 0; i < 3; i++) p[i] = ppr[i];
    return JME_OK;
}

static double vec_norm3(const double *a)
{
    return sqrt(a[0] * a[0] + a[1] * a[1] + a[2] * a[2]);
}

/*
 * Gravitational light deflection by the Sun, the Jupiter system and the
 * Saturn system (and, for a topocentric observer, the Earth): the deflectors
 * of the IERS Conventions 2010 ch. 7 / Explanatory Supplement (3rd ed., 2013)
 * sect. 7.2.4 model, each with eraLd (Klioner 2003) and the deflector placed
 * at the time of closest approach of the light ray (as in ERFA eraLdn), from
 * the backend ephemeris without linearisation.
 * Mass ratios GM/GM_Sun: DE440/DE441 values (Park et al. 2021, AJ 161:105,
 * Table 2). Deflection limiters dl = phi^2/2 (phi = angular radius scale at
 * which the deflection is tapered): Sun 1e-6 (as before), Jupiter 3e-9 and
 * Saturn 3e-10 (SOFA/ERFA eraLdn documentation).
 *   p    : unit observer->source direction (in/out)
 *   obs  : observer position (backend frame, AU)
 *   src  : source position at emission (backend frame) or 0 for infinity
 *   tau  : light time observer<-source (day); ignored when src == 0
 *   geo  : geocentre position (topocentric observers) or 0
 */
#define JME_GM_SUN_KM3S2 132712440041.279419
static int apply_light_deflection(int backend, int engine_policy, double jd_et, int body,
    const double *obs, const double *src, double tau, const double *geo, double *p, char *error)
{
    static const double gm[3] = { JME_GM_SUN_KM3S2, 126712764.100000, 37940584.841800 };
    static const double dl[3] = { 1.0e-6, 3.0e-9, 3.0e-10 };
    int k, i;

    for (k = 0; k < 3; k++) {
        int dbody;
        double d0[6], d[6], ev[3], qv[3], em, qn, dt, p1[3], pp[3];
        if (k == 0) {
            dbody = JME_BODY_SUN;
            if (body == JME_BODY_SUN) continue;
        } else if (k == 1) {
            dbody = backend == JME_BACKEND_JPL ? JME_BODY_JUPITER_BARYCENTER : JME_BODY_JUPITER;
            if (body == JME_BODY_JUPITER || body == JME_BODY_JUPITER_BARYCENTER) continue;
        } else {
            dbody = backend == JME_BACKEND_JPL ? JME_BODY_SATURN_BARYCENTER : JME_BODY_SATURN;
            if (body == JME_BODY_SATURN || body == JME_BODY_SATURN_BARYCENTER) continue;
        }
        if (backend_state(backend, jd_et, dbody, engine_policy, 0, d0, error) != JME_OK) return JME_ERR;
        /* Light from the closest-approach point reaches the observer after
         * dt = p.(obs - D)/c, bounded by [0, tau] for a source at finite distance. */
        dt = 0.0;
        for (i = 0; i < 3; i++) dt += p[i] * (obs[i] - d0[i]);
        dt /= JME_C_AU_PER_DAY;
        if (dt < 0.0) dt = 0.0;
        if (src != 0 && dt > tau) dt = tau;
        if (dt > 0.0) {
            if (backend_state_offset(backend, jd_et, -dt, dbody, engine_policy, 0, d, error) != JME_OK) return JME_ERR;
        } else {
            for (i = 0; i < 6; i++) d[i] = d0[i];
        }
        for (i = 0; i < 3; i++) {
            ev[i] = obs[i] - d[i];
            qv[i] = src != 0 ? src[i] - d[i] : p[i];
        }
        em = vec_norm3(ev);
        qn = vec_norm3(qv);
        if (!(em > 0.0) || !(qn > 0.0)) {
            jme_set_error(error, "Degenerate deflector-observer-source geometry");
            return JME_ERR;
        }
        for (i = 0; i < 3; i++) { ev[i] /= em; qv[i] /= qn; pp[i] = p[i]; }
        jme_eraLd(gm[k] / JME_GM_SUN_KM3S2, pp, qv, ev, em, dl[k], p1);
        for (i = 0; i < 3; i++) p[i] = p1[i];
    }

    if (geo != 0) {
        /* Earth deflection for an observer on the Earth (GM_Earth from DE440).
         * eraLd divides by max(q.(q+e), dlim); dlim = 1 leaves every source
         * above the observer's geocentric horizon (q.(q+e) >= 1) exact and
         * keeps the term continuous across the horizon and bounded towards
         * the nadir, where no light reaches the observer. */
        double ev[3], qv[3], em, qn, p1[3], pp[3];
        for (i = 0; i < 3; i++) {
            ev[i] = obs[i] - geo[i];
            qv[i] = src != 0 ? src[i] - geo[i] : p[i];
        }
        em = vec_norm3(ev);
        qn = vec_norm3(qv);
        if (em > 0.0 && qn > 0.0) {
            for (i = 0; i < 3; i++) { ev[i] /= em; qv[i] /= qn; pp[i] = p[i]; }
            jme_eraLd(398600.435507 / JME_GM_SUN_KM3S2, pp, qv, ev, em, 1.0, p1);
            for (i = 0; i < 3; i++) p[i] = p1[i];
        }
    }
    return JME_OK;
}

/*
 * Compute the observer-relative state of body at jd_et in J2000-mean axes.
 *   apparent = 0: geometric, both at jd_et.
 *   apparent = 1: light-time iterated (target at t - tau, observer at t);
 *                 optional solar deflection and aberration (geocentric /
 *                 topocentric observers only).
 * On return *frame_bias_applicable tells whether the vector is in ICRF axes
 * (JPL) so that frame bias applies when transforming to mean-of-date.
 */
static int observer_relative_state(
    double jd_et, int body, int center, int flags, int engine_policy,
    double *out, int *backend_out, char *error)
{
    int backend;
    int geocentric = (center == JME_BODY_EARTH);
    int topocentric = geocentric && (flags & JME_CALC_TOPOCENTRIC);
    int apparent = !(flags & JME_CALC_TRUE_POSITION);
    double obs[6], vobs[3], tgt[6], rel[6];
    double tau = 0.0, dist;
    int i, iter;

    if (choose_backend(jd_et, body, engine_policy, &backend, error) != JME_OK) {
        return JME_ERR;
    }
    if (error != 0) error[0] = '\0';
    if (backend_out != 0) *backend_out = backend;

    if (geocentric) {
        int need_velocity = apparent && !(flags & JME_CALC_NO_ABERRATION);
        if (observer_state(backend, jd_et, topocentric, engine_policy, obs, need_velocity ? vobs : 0, error) != JME_OK) {
            return JME_ERR;
        }
    } else if (center == JME_BODY_SUN) {
        if (backend_state(backend, jd_et, JME_BODY_SUN, engine_policy, 0, obs, error) != JME_OK) {
            return JME_ERR;
        }
    } else if (center == JME_BODY_SOLAR_SYSTEM_BARYCENTER) {
        if (backend != JME_BACKEND_JPL) {
            jme_set_error(error, "Barycentric positions require the JPL engine");
            return JME_ERR;
        }
        for (i = 0; i < 6; i++) obs[i] = 0.0;
    } else {
        jme_set_error(error, "Unsupported centre");
        return JME_ERR;
    }

    if (body == center && !topocentric) {
        /* A body relative to itself is the zero vector (e.g. heliocentric Sun). */
        for (i = 0; i < 6; i++) out[i] = 0.0;
        return JME_OK;
    }

    if (backend_state(backend, jd_et, body, engine_policy, 0, tgt, error) != JME_OK) {
        return JME_ERR;
    }
    for (i = 0; i < 6; i++) rel[i] = tgt[i] - obs[i];

    if (apparent) {
        /* Light-time iteration; converges to < 1e-12 day (< 0.1 microsecond) quickly. */
        for (iter = 0; iter < 20; iter++) {
            double new_tau;
            dist = vec_norm3(rel);
            if (!isfinite(dist) || dist <= 0.0) {
                jme_set_error(error, "Calculation produced invalid distance");
                return JME_ERR;
            }
            new_tau = dist / JME_C_AU_PER_DAY;
            if (iter > 0 && fabs(new_tau - tau) < 1.0e-12) {
                tau = new_tau;
                break;
            }
            tau = new_tau;
            if (backend_state_offset(backend, jd_et, -tau, body, engine_policy, 0, tgt, error) != JME_OK) {
                return JME_ERR;
            }
            for (i = 0; i < 6; i++) rel[i] = tgt[i] - obs[i];
        }
        if (iter >= 20) {
            jme_set_error(error, "Light-time iteration did not converge");
            return JME_ERR;
        }

        if (geocentric && body != JME_BODY_SUN
            && (!(flags & JME_CALC_NO_LIGHT_DEFLECTION) || !(flags & JME_CALC_NO_ABERRATION))) {
            double sun_t[6], geo[6];
            double p[3], e[3], em;

            if (backend_state(backend, jd_et, JME_BODY_SUN, engine_policy, 0, sun_t, error) != JME_OK) {
                return JME_ERR;
            }
            dist = vec_norm3(rel);
            for (i = 0; i < 3; i++) {
                p[i] = rel[i] / dist;
                e[i] = obs[i] - sun_t[i];
            }
            em = vec_norm3(e);
            if (!(em > 0.0)) {
                jme_set_error(error, "Degenerate Sun-observer geometry");
                return JME_ERR;
            }

            if (!(flags & JME_CALC_NO_LIGHT_DEFLECTION)) {
                if (topocentric
                    && backend_state(backend, jd_et, JME_BODY_EARTH, engine_policy, 0, geo, error) != JME_OK) {
                    return JME_ERR;
                }
                if (apply_light_deflection(backend, engine_policy, jd_et, body, obs, tgt, tau,
                        topocentric ? geo : 0, p, error) != JME_OK) {
                    return JME_ERR;
                }
            }
            if (!(flags & JME_CALC_NO_ABERRATION)) {
                if (apply_aberration(p, vobs, em, error) != JME_OK) return JME_ERR;
            }
            for (i = 0; i < 3; i++) rel[i] = p[i] * dist;
        } else if (geocentric && body == JME_BODY_SUN && !(flags & JME_CALC_NO_ABERRATION)) {
            double p[3];
            dist = vec_norm3(rel);
            for (i = 0; i < 3; i++) p[i] = rel[i] / dist;
            if (apply_aberration(p, vobs, dist, error) != JME_OK) return JME_ERR;
            for (i = 0; i < 3; i++) rel[i] = p[i] * dist;
        }
    }

    for (i = 0; i < 6; i++) out[i] = rel[i];
    if (!state_is_finite(out, 6)) {
        jme_set_error(error, "Calculation produced non-finite state");
        return JME_ERR;
    }
    return JME_OK;
}

/*
 * Rotate a position vector from backend axes to the requested output frame
 * and coordinate system.
 *   icrs_axes: 1 when the input vector is in ICRS/ICRF axes (JPL, star
 *              catalogue), 0 for the analytical engines' mean J2000 axes.
 *   JME_CALC_ICRS : input axes unchanged (equatorial), ecliptic via J2000 obliquity.
 *   JME_CALC_J2000: mean equator & equinox of J2000 (frame bias applied to ICRS vectors).
 *   default       : true equator & equinox of date (mean of date with
 *                   JME_CALC_NO_NUTATION), context bias/precession/nutation.
 *   Ecliptic output uses the obliquity matching the frame (true obliquity
 *   when nutation is applied). JME_CALC_SIDEREAL rotates the ecliptic of date
 *   by the true ayanamsa (mean ayanamsa without nutation).
 */
int internal_rotate_to_output(double jd_et, int flags, int icrs_axes, const double *in, double *out, char *error)
{
    double m[9], v[6], tmp[6];
    double eps = 0.0;
    double dpsi_applied = 0.0;
    int i;

    for (i = 0; i < 3; i++) { v[i] = in[i]; v[i + 3] = 0.0; }

    if ((flags & JME_CALC_SIDEREAL)
        && ((flags & JME_CALC_EQUATORIAL) || (flags & JME_CALC_J2000) || (flags & JME_CALC_ICRS))) {
        jme_set_error(error, "JME_CALC_SIDEREAL is defined only for ecliptic-of-date output");
        return JME_ERR;
    }

    if (flags & JME_CALC_ICRS) {
        if (jme_get_obliquity(2451545.0, jme_context_obliquity_model(), &eps, error) != JME_OK) return JME_ERR;
    } else if (flags & JME_CALC_J2000) {
        if (icrs_axes) {
            if (jme_get_frame_bias_matrix(jme_context_bias_model(), m) != JME_OK) {
                jme_set_error(error, "Unsupported frame-bias model");
                return JME_ERR;
            }
            jme_matrix_transform_state(m, v, v);
        }
        if (jme_get_obliquity(2451545.0, jme_context_obliquity_model(), &eps, error) != JME_OK) return JME_ERR;
    } else {
        if (icrs_axes) {
            if (internal_bias_precession_matrix(jd_et, m, error) != JME_OK) return JME_ERR;
        } else if (jme_get_precession_matrix(2451545.0, jd_et, jme_context_precession_model(), m) != JME_OK) {
            jme_set_error(error, "Unsupported precession model");
            return JME_ERR;
        }
        jme_matrix_transform_state(m, v, v);
        if (flags & JME_CALC_NO_NUTATION) {
            if (jme_get_obliquity(jd_et, jme_context_obliquity_model(), &eps, error) != JME_OK) return JME_ERR;
        } else {
            double epsm, deps;
            if (internal_nutation_matrix(jd_et, m, &dpsi_applied, &epsm, &deps, error) != JME_OK) return JME_ERR;
            jme_matrix_transform_state(m, v, v);
            eps = epsm + deps;
        }
    }

    if (!(flags & JME_CALC_EQUATORIAL)) {
        if (jme_equatorial_to_ecliptic_rectangular_state(v, eps, tmp) != JME_OK) {
            jme_set_error(error, "Equatorial-to-ecliptic conversion failed");
            return JME_ERR;
        }
        for (i = 0; i < 6; i++) v[i] = tmp[i];
    }

    if (flags & JME_CALC_SIDEREAL) {
        double ayan_mean, rot, c, sn, x, y;
        if (internal_context_ayanamsa(jd_et, &ayan_mean, 0, error) != JME_OK) return JME_ERR;
        /* lambda_sid = lambda_true - (mean ayanamsa + dpsi) = lambda_mean - mean ayanamsa. */
        rot = (ayan_mean + dpsi_applied) * JME_DEG_TO_RAD;
        c = cos(rot); sn = sin(rot);
        x = v[0]; y = v[1];
        v[0] = c * x + sn * y;
        v[1] = -sn * x + c * y;
    }

    for (i = 0; i < 3; i++) {
        out[i] = v[i];
        if (!isfinite(out[i])) {
            jme_set_error(error, "Frame transformation produced non-finite values");
            return JME_ERR;
        }
    }
    return JME_OK;
}

/*
 * Observer-relative position in the requested output frame (no velocity).
 */
static int calc_output_position(double jd_et, int body, int center, int flags, int engine_policy,
    double *pos, char *error)
{
    double state[6];
    int backend = 0;
    if (observer_relative_state(jd_et, body, center, flags, engine_policy, state, &backend, error) != JME_OK) {
        return JME_ERR;
    }
    return internal_rotate_to_output(jd_et, flags, backend == JME_BACKEND_JPL, state, pos, error);
}

/*
 * Rates are obtained by differentiating the complete reduction (light time,
 * deflection, aberration, precession, nutation, sidereal rotation), so
 * speeds are consistent with the reported positions in the rotating frame of
 * date. Five-point central stencil, step 0.01 day: truncation error ~1e-11
 * relative for the Moon, below the ~1e-8 relative floor set by binary64
 * evaluation noise of the series (see docs/ACCURACY.md).
 */
#define JME_CALC_RATE_STEP_DAYS 1.0e-2

int internal_finish_output(const double *p0, const double *const *stencil, double h, int flags,
    double *results, char *error)
{
    double state[6], spherical[6];
    int i;

    /* stencil = {p(t-2h), p(t-h), p(t+h), p(t+2h)} */
    for (i = 0; i < 3; i++) {
        state[i] = p0[i];
        state[i + 3] = (stencil[0][i] - 8.0 * stencil[1][i] + 8.0 * stencil[2][i] - stencil[3][i]) / (12.0 * h);
    }

    if (flags & JME_CALC_XYZ) {
        for (i = 0; i < 6; i++) results[i] = state[i];
    } else {
        if (jme_rectangular_to_spherical_state(state, spherical) != JME_OK) {
            jme_set_error(error, "Rectangular-to-spherical conversion failed");
            return JME_ERR;
        }
        for (i = 0; i < 6; i++) results[i] = spherical[i];
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
        for (i = 3; i < 6; i++) results[i] /= JME_SECONDS_PER_DAY;
    }
    if (!state_is_finite(results, 6)) {
        for (i = 0; i < 6; i++) { results[i] = 0.0; }
        jme_set_error(error, "Calculation produced non-finite output");
        return JME_ERR;
    }
    return JME_OK;
}

/*
 * Position-only evaluation for internal event searches: the same complete
 * reduction as jme_calc (identical position bits), rectangular, au, in the
 * frame selected by flags, without the rate stencil (positions only are
 * needed by root finders; this is not an approximation of jme_calc).
 */
static int calc_position_et(double jd_et, int body, int flags, double *pos, char *error)
{
    int center = JME_BODY_EARTH;
    int i;

    if (pos == 0) {
        jme_set_error(error, "Output buffer is required");
        return JME_ERR;
    }
    for (i = 0; i < 3; i++) pos[i] = 0.0;
    if (error != 0) error[0] = '\0';
    if (body == JME_BODY_MEAN_NODE || body == JME_BODY_TRUE_NODE) {
        double r[6];
        if (jme_calc(jd_et, body, (flags | JME_CALC_XYZ) & ~(JME_CALC_DISTANCE_KM | JME_CALC_VELOCITY_PER_SECOND), r, error) != JME_OK) return JME_ERR;
        for (i = 0; i < 3; i++) pos[i] = r[i];
        return JME_OK;
    }
    if (flags & JME_CALC_BARYCENTRIC) {
        center = JME_BODY_SOLAR_SYSTEM_BARYCENTER;
    } else if (flags & JME_CALC_HELIOCENTRIC) {
        center = JME_BODY_SUN;
    }
    if (calc_output_position(jd_et, body, center, flags, jme_context_engine_policy(), pos, error) != JME_OK) {
        return JME_ERR;
    }
    if (!state_is_finite(pos, 3)) {
        jme_set_error(error, "Calculation produced non-finite output");
        return JME_ERR;
    }
    return JME_OK;
}

int internal_calc_position_ut(double jd_ut, int body, int flags, double *pos, char *error)
{
    double jd_et;
    int i;

    if (pos == 0) {
        jme_set_error(error, "Output buffer is required");
        return JME_ERR;
    }
    for (i = 0; i < 3; i++) pos[i] = 0.0;
    if (internal_ut_to_et(jd_ut, &jd_et, error) != JME_OK) return JME_ERR;
    return calc_position_et(jd_et, body, flags, pos, error);
}

/*
 * jme_calc_ut output layout for positions only: results[0..2] are bit-identical
 * to jme_calc_ut (same reduction and conversions); results[3..5] (rates) are
 * NaN because they are not computed. Used by internal event searches.
 */
int internal_pack_position_results(const double *rect, int flags, double *results, char *error)
{
    double st[6], sph[6];
    int i;

    if (results == 0 || rect == 0) {
        jme_set_error(error, "Output buffer is required");
        return JME_ERR;
    }
    for (i = 0; i < 6; i++) results[i] = 0.0;
    st[0] = rect[0];
    st[1] = rect[1];
    st[2] = rect[2];
    st[3] = st[4] = st[5] = 0.0;
    if (flags & JME_CALC_XYZ) {
        for (i = 0; i < 3; i++) results[i] = st[i];
        if (flags & JME_CALC_DISTANCE_KM) for (i = 0; i < 3; i++) results[i] *= JME_AU_KM;
    } else {
        if (jme_rectangular_to_spherical_state(st, sph) != JME_OK) {
            jme_set_error(error, "Rectangular-to-spherical conversion failed");
            return JME_ERR;
        }
        results[0] = angle_to_output_unit(sph[0], flags);
        results[1] = angle_to_output_unit(sph[1], flags);
        results[2] = (flags & JME_CALC_DISTANCE_KM) ? sph[2] * JME_AU_KM : sph[2];
    }
    results[3] = results[4] = results[5] = NAN;
    return JME_OK;
}

int internal_calc_positions(double jd_et, int body, int flags, double *results, char *error)
{
    double st[3];

    if (results == 0) {
        jme_set_error(error, "Output buffer is required");
        return JME_ERR;
    }
    if (!isfinite(jd_et)) {
        jme_set_error(error, "Terrestrial-time Julian date must be finite");
        return JME_ERR;
    }
    if (calc_position_et(jd_et, body, flags & ~(JME_CALC_DISTANCE_KM | JME_CALC_VELOCITY_PER_SECOND), st, error) != JME_OK) {
        return JME_ERR;
    }
    return internal_pack_position_results(st, flags, results, error);
}

int internal_calc_ut_positions(double jd_ut, int body, int flags, double *results, char *error)
{
    double jd_et;
    int i;

    if (results == 0) {
        jme_set_error(error, "Output buffer is required");
        return JME_ERR;
    }
    for (i = 0; i < 6; i++) results[i] = 0.0;
    if (internal_ut_to_et(jd_ut, &jd_et, error) != JME_OK) return JME_ERR;
    return internal_calc_positions(jd_et, body, flags, results, error);
}

int jme_calc(double jd_et, int body, int flags, double *results, char *error)
{
    double p0[3], q[4][3];
    const double *stencil[4];
    const double h = JME_CALC_RATE_STEP_DAYS;
    static const double offs[4] = {-2.0, -1.0, 1.0, 2.0};
    int center = JME_BODY_EARTH;
    int engine_policy = jme_context_engine_policy();
    int i;

    if (results == 0) {
        jme_set_error(error, "Output buffer is required");
        return JME_ERR;
    }
    for (i = 0; i < 6; i++) { results[i] = 0.0; }
    if (error != 0) error[0] = '\0';
    if (!isfinite(jd_et)) {
        jme_set_error(error, "Terrestrial-time Julian date must be finite");
        return JME_ERR;
    }

    if (body == JME_BODY_MEAN_NODE || body == JME_BODY_TRUE_NODE) {
        if (calc_lunar_node(jd_et, body, flags, results, error) != JME_OK
            || !state_is_finite(results, 6)) {
            for (i = 0; i < 6; i++) results[i] = 0.0;
            if (error != 0 && error[0] == '\0') jme_set_error(error, "Calculation produced non-finite output");
            return JME_ERR;
        }
        if (error != 0) error[0] = '\0';
        return JME_OK;
    }

    if (flags & JME_CALC_BARYCENTRIC) {
        center = JME_BODY_SOLAR_SYSTEM_BARYCENTER;
    } else if (flags & JME_CALC_HELIOCENTRIC) {
        center = JME_BODY_SUN;
    }

    if (calc_output_position(jd_et, body, center, flags, engine_policy, p0, error) != JME_OK) {
        return JME_ERR;
    }
    for (i = 0; i < 4; i++) {
        if (calc_output_position(jd_et + offs[i] * h, body, center, flags, engine_policy, q[i], error) != JME_OK) {
            return JME_ERR;
        }
        stencil[i] = q[i];
    }
    if (internal_finish_output(p0, stencil, h, flags, results, error) != JME_OK) {
        return JME_ERR;
    }
    if (error != 0) error[0] = '\0';
    return JME_OK;
}

/*
 * Observer-relative vector of a star whose barycentric direction is u
 * (ICRS unit vector) at distance dist_au (0 = unknown, star at infinity).
 * Applies annual (and diurnal, topocentric) parallax when the distance is
 * known, then gravitational deflection by the Sun and aberration unless
 * disabled or JME_CALC_TRUE_POSITION. Output in ICRS axes; *icrs_axes
 * reports whether observer vectors were in ICRS axes (JPL) or analytical
 * J2000 axes (the 23 mas frame bias is negligible for the observer offset).
 */
int internal_star_observer_vector(double jd_et, int flags, const double *u, double dist_au, double *out, char *error)
{
    int engine_policy = jme_context_engine_policy();
    int backend;
    int geocentric = !(flags & (JME_CALC_HELIOCENTRIC | JME_CALC_BARYCENTRIC));
    int topocentric = geocentric && (flags & JME_CALC_TOPOCENTRIC);
    double obs[6] = {0, 0, 0, 0, 0, 0}, vobs[3] = {0, 0, 0};
    double v[3], dist;
    double sunoff[3] = {0.0, 0.0, 0.0};
    int i;

    if (flags & JME_CALC_BARYCENTRIC) {
        for (i = 0; i < 3; i++) out[i] = (dist_au > 0.0) ? u[i] * dist_au : u[i];
        return JME_OK;
    }
    if (choose_backend(jd_et, JME_BODY_EARTH, engine_policy, &backend, error) != JME_OK) return JME_ERR;
    if (flags & JME_CALC_HELIOCENTRIC) {
        if (backend != JME_BACKEND_JPL) {
            jme_set_error(error, "Heliocentric star positions require the JPL engine (analytical engines have no barycentre)");
            return JME_ERR;
        }
        if (backend_state(backend, jd_et, JME_BODY_SUN, engine_policy, 0, obs, error) != JME_OK) return JME_ERR;
    } else if (observer_state(backend, jd_et, topocentric, engine_policy, obs,
                   (!(flags & JME_CALC_TRUE_POSITION) && !(flags & JME_CALC_NO_ABERRATION)) ? vobs : 0, error) != JME_OK) {
        return JME_ERR;
    }

    if (backend == JME_BACKEND_ANALYTIC && dist_au > 0.0) {
        /* Parallax needs the barycentric observer position. */
        double sun[6], jd_tdb;
        if (internal_tt_to_tdb_jd(jd_et, &jd_tdb, error) != JME_OK
            || analytic_sun_barycentric(jd_tdb, engine_policy, sun, error) != JME_OK) {
            return JME_ERR;
        }
        for (i = 0; i < 3; i++) { obs[i] += sun[i]; sunoff[i] = sun[i]; }
    }

    if (dist_au > 0.0) {
        for (i = 0; i < 3; i++) v[i] = u[i] * dist_au - obs[i];
    } else {
        for (i = 0; i < 3; i++) v[i] = u[i];
    }
    dist = vec_norm3(v);
    if (!(dist > 0.0) || !isfinite(dist)) {
        jme_set_error(error, "Invalid star vector");
        return JME_ERR;
    }

    if (geocentric && !(flags & JME_CALC_TRUE_POSITION)) {
        double p[3], e[3], em, sun[6];
        for (i = 0; i < 3; i++) p[i] = v[i] / dist;
        if (backend_state(backend, jd_et, JME_BODY_SUN, engine_policy, 0, sun, error) != JME_OK) return JME_ERR;
        for (i = 0; i < 3; i++) e[i] = obs[i] - (sun[i] + sunoff[i]);
        em = vec_norm3(e);
        if (!(em > 0.0)) {
            jme_set_error(error, "Degenerate Sun-observer geometry");
            return JME_ERR;
        }
        for (i = 0; i < 3; i++) e[i] /= em;
        if (!(flags & JME_CALC_NO_LIGHT_DEFLECTION)) {
            /* Analytical backends are heliocentric: express the observer in
             * the same origin as the deflector states. */
            double obs_b[3], geo_b[6];
            for (i = 0; i < 3; i++) obs_b[i] = obs[i] - sunoff[i];
            if (topocentric) {
                if (backend_state(backend, jd_et, JME_BODY_EARTH, engine_policy, 0, geo_b, error) != JME_OK) return JME_ERR;
            }
            if (apply_light_deflection(backend, engine_policy, jd_et, -1, obs_b, 0, 0.0,
                    topocentric ? geo_b : 0, p, error) != JME_OK) {
                return JME_ERR;
            }
        }
        if (!(flags & JME_CALC_NO_ABERRATION)) {
            if (apply_aberration(p, vobs, em, error) != JME_OK) return JME_ERR;
        }
        for (i = 0; i < 3; i++) v[i] = p[i] * dist;
    }
    for (i = 0; i < 3; i++) out[i] = (dist_au > 0.0) ? v[i] : v[i] / dist;
    return JME_OK;
}
