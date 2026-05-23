#include "jme/jme.h"

#include <math.h>
#include <stdio.h>

#ifndef M_PI
#define M_PI 3.141592653589793238462643383279502884
#endif

static int check_close(const char *label, double got, double expected, double tolerance)
{
    if (!isfinite(got) || fabs(got - expected) > tolerance) {
        fprintf(stderr, "%s mismatch: got %.17g expected %.17g tolerance %.17g\n", label, got, expected, tolerance);
        return 1;
    }
    return 0;
}

static int convert_j2000_ecliptic_to_equatorial_of_date(double jd_et, const double *input, double *output, char *error)
{
    double eps = 0.0;
    double bias[9];
    double prec[9];
    double dpsi = 0.0;
    double deps = 0.0;
    double nut[9];
    double temp[6];
    int i;

    if (jme_get_obliquity(2451545.0, JME_MODEL_OBL_IAU_1980, &eps, error) != JME_OK
        || jme_ecliptic_to_equatorial_rectangular_state(input, eps, temp) != JME_OK) {
        return JME_ERR;
    }
    for (i = 0; i < 6; i++) {
        output[i] = temp[i];
    }
    if (jme_get_frame_bias_matrix(JME_MODEL_BIAS_NONE, bias) == JME_OK) {
        jme_matrix_transform_state(bias, output, output);
    }
    if (jme_get_precession_matrix(2451545.0, jd_et, JME_MODEL_PREC_IAU_1976, prec) != JME_OK) {
        return JME_ERR;
    }
    jme_matrix_transform_state(prec, output, output);
    if (jme_get_nutation(jd_et, JME_MODEL_NUT_IAU_1980, &dpsi, &deps, error) != JME_OK
        || jme_get_obliquity(jd_et, JME_MODEL_OBL_IAU_1980, &eps, error) != JME_OK) {
        return JME_ERR;
    }
    jme_get_nutation_matrix(dpsi * (M_PI / 180.0), deps * (M_PI / 180.0), eps * (M_PI / 180.0), nut);
    jme_matrix_transform_state(nut, output, output);
    for (i = 0; i < 6; i++) {
        if (!isfinite(output[i])) {
            return JME_ERR;
        }
    }
    return JME_OK;
}

static double angular_distance_deg(double a, double b)
{
    return fabs(jme_degrees_difference_signed(a, b));
}

static void vec_cross(const double *a, const double *b, double *out)
{
    out[0] = a[1] * b[2] - a[2] * b[1];
    out[1] = a[2] * b[0] - a[0] * b[2];
    out[2] = a[0] * b[1] - a[1] * b[0];
}

static double vec_dot(const double *a, const double *b)
{
    return a[0] * b[0] + a[1] * b[1] + a[2] * b[2];
}

static double vec_norm(const double *a)
{
    return sqrt(vec_dot(a, a));
}

static int derive_elements(const double *state, double *elem)
{
    const double mu = 0.0002959122082855911025;
    const double rad_to_deg = 57.2957795130823208768;
    double r[3] = {state[0], state[1], state[2]};
    double v[3] = {state[3], state[4], state[5]};
    double h[3];
    double n[3];
    double z[3] = {0.0, 0.0, 1.0};
    double evec[3];
    double rmag = vec_norm(r);
    double vmag = vec_norm(v);
    double hmag;
    double nmag;
    double e;
    double c;
    double inc;
    double node;
    double argp = 0.0;
    double nu = 0.0;
    double energy;
    double a;
    int i;

    vec_cross(r, v, h);
    hmag = vec_norm(h);
    vec_cross(z, h, n);
    nmag = vec_norm(n);

    if (!isfinite(rmag) || !isfinite(vmag) || !isfinite(hmag) || rmag <= 0.0 || hmag <= 0.0) {
        return 1;
    }

    for (i = 0; i < 3; i++) {
        evec[i] = ((vmag * vmag - mu / rmag) * r[i] - vec_dot(r, v) * v[i]) / mu;
    }
    e = vec_norm(evec);
    energy = 0.5 * vmag * vmag - mu / rmag;
    a = fabs(energy) > 0.0 ? -mu / (2.0 * energy) : 0.0;
    inc = acos(h[2] / hmag) * rad_to_deg;
    node = nmag > 0.0 ? jme_degree_normalize(atan2(n[1], n[0]) * rad_to_deg) : 0.0;

    if (nmag > 0.0 && e > 0.0) {
        c = vec_dot(n, evec) / (nmag * e);
        if (c > 1.0) { c = 1.0; }
        if (c < -1.0) { c = -1.0; }
        argp = acos(c) * rad_to_deg;
        if (evec[2] < 0.0) {
            argp = 360.0 - argp;
        }
    }

    if (e > 0.0) {
        c = vec_dot(evec, r) / (e * rmag);
        if (c > 1.0) { c = 1.0; }
        if (c < -1.0) { c = -1.0; }
        nu = acos(c) * rad_to_deg;
        if (vec_dot(r, v) < 0.0) {
            nu = 360.0 - nu;
        }
    }

    elem[0] = a;
    elem[1] = e;
    elem[2] = inc;
    elem[3] = node;
    elem[4] = jme_degree_normalize(node + argp);
    elem[5] = nu;
    return 0;
}

static int build_engine_state(
    int (*engine_fn)(double jd_et, int body, double *results),
    double jd_et,
    int body,
    double *state
)
{
    double prev[6];
    double next[6];
    const double step = 1.0e-3;
    int i;

    if (engine_fn(jd_et, body, state) != JME_OK) {
        return 1;
    }

    if (engine_fn(jd_et - step, body, prev) != JME_OK || engine_fn(jd_et + step, body, next) != JME_OK) {
        return 1;
    }

    for (i = 0; i < 3; i++) {
        state[i + 3] = (next[i] - prev[i]) / (2.0 * step);
    }

    return 0;
}

static int check_vsop87a_j2000(void)
{
    struct case_row {
        int body;
        const char *name;
        double xyz[3];
    } cases[] = {
        {JME_BODY_MERCURY, "Mercury", {-0.1300934115, -0.4472876716, -0.0245983802}},
        {JME_BODY_VENUS, "Venus", {-0.7183022797, -0.0326546017, 0.0410142975}},
        {JME_BODY_EARTH, "Earth", {-0.1771354586, 0.9672416237, -0.0000039000}},
        {JME_BODY_MARS, "Mars", {1.3907159264, -0.0134157043, -0.0344677967}},
        {JME_BODY_JUPITER, "Jupiter", {4.0011740268, 2.9385810077, -0.1017837501}},
        {JME_BODY_SATURN, "Saturn", {6.4064068573, 6.5699929449, -0.3690768029}},
        {JME_BODY_URANUS, "Uranus", {14.4318934159, -13.7343162527, -0.2381421963}},
        {JME_BODY_NEPTUNE, "Neptune", {16.8121116576, -24.9916630908, 0.1272190171}}
    };
    int i;

    for (i = 0; i < (int)(sizeof(cases) / sizeof(cases[0])); i++) {
        double state[6] = {0.0, 0.0, 0.0, 0.0, 0.0, 0.0};
        char label[128];
        int j;

        if (jme_vsop87_planet_state(2451545.0, cases[i].body, state) != JME_OK) {
            fprintf(stderr, "VSOP87A %s failed\n", cases[i].name);
            return 1;
        }

        for (j = 0; j < 3; j++) {
            snprintf(label, sizeof(label), "VSOP87A %s[%d]", cases[i].name, j);
            if (check_close(label, state[j], cases[i].xyz[j], 5e-10) != 0) {
                return 1;
            }
        }

        if (!isfinite(state[3]) || !isfinite(state[4]) || !isfinite(state[5])
            || fabs(state[3]) + fabs(state[4]) + fabs(state[5]) <= 0.0) {
            fprintf(stderr, "VSOP87A %s velocity is not finite/non-zero\n", cases[i].name);
            return 1;
        }
    }

    return 0;
}

static int check_elp2000_reference(void)
{
    struct elp_case {
        double jd;
        double xyz_km[3];
    } cases[] = {
        {2451555.5, {382979.7604730463, -68204.20174530084, -25987.71602589964}},
        {2450535.0858589401, {-275315.85366705677, -278571.10866498074, 26325.720663196265}},
        {2452219.5869187401, {-81955.583671778193, 368196.84963875631, 8622.8434201957880}},
        {2457671.9752448499, {206861.16245819122, -326463.48893797904, 22436.002934452619}},
        {2468059.7751763500, {371212.90122084937, 117491.28319247597, 25803.144969337736}}
    };
    int i;

    for (i = 0; i < (int)(sizeof(cases) / sizeof(cases[0])); i++) {
        double state[6] = {0.0, 0.0, 0.0, 0.0, 0.0, 0.0};
        int j;

        if (jme_elp2000_moon_state(cases[i].jd, state) != JME_OK) {
            fprintf(stderr, "ELP2000 state failed at JD %.10f\n", cases[i].jd);
            return 1;
        }

        for (j = 0; j < 3; j++) {
            char label[96];
            snprintf(label, sizeof(label), "ELP2000 Moon JD %.1f km[%d]", cases[i].jd, j);
            if (check_close(label, state[j] * JME_AU_KM, cases[i].xyz_km[j], 50.0) != 0) {
                return 1;
            }
        }

        if (!isfinite(state[3]) || !isfinite(state[4]) || !isfinite(state[5])
            || fabs(state[3]) + fabs(state[4]) + fabs(state[5]) <= 0.0) {
            fprintf(stderr, "ELP2000 velocity is not finite/non-zero\n");
            return 1;
        }
    }

    return 0;
}

static int check_engine_elements_against_jpl_table(void)
{
    struct element_case {
        int body;
        const char *name;
        double a;
        double e;
        double inc;
        double long_peri;
        double node;
    } cases[] = {
        {JME_BODY_MERCURY, "Mercury", 0.38709927, 0.20563593, 7.00497902, 77.45779628, 48.33076593},
        {JME_BODY_VENUS, "Venus", 0.72333566, 0.00677672, 3.39467605, 131.60246718, 76.67984255},
        {JME_BODY_MARS, "Mars", 1.52371034, 0.09339410, 1.84969142, 336.05637041, 49.55953891}
    };
    int i;

    for (i = 0; i < (int)(sizeof(cases) / sizeof(cases[0])); i++) {
        double state[6];
        double elem[6];
        char label[128];

        if (build_engine_state(jme_vsop87_planet_state, 2451545.0, cases[i].body, state) != 0 || derive_elements(state, elem) != 0) {
            fprintf(stderr, "VSOP87 element derivation failed for %s\n", cases[i].name);
            return 1;
        }

        snprintf(label, sizeof(label), "VSOP87 %s a", cases[i].name);
        if (check_close(label, elem[0], cases[i].a, 5e-4) != 0) { return 1; }
        snprintf(label, sizeof(label), "VSOP87 %s e", cases[i].name);
        if (check_close(label, elem[1], cases[i].e, 5e-4) != 0) { return 1; }
        snprintf(label, sizeof(label), "VSOP87 %s i", cases[i].name);
        if (check_close(label, elem[2], cases[i].inc, 0.05) != 0) { return 1; }
        snprintf(label, sizeof(label), "VSOP87 %s peri", cases[i].name);
        if (check_close(label, angular_distance_deg(elem[4], cases[i].long_peri), 0.0, 0.5) != 0) { return 1; }
        snprintf(label, sizeof(label), "VSOP87 %s node", cases[i].name);
        if (check_close(label, angular_distance_deg(elem[3], cases[i].node), 0.0, 0.5) != 0) { return 1; }

        if (build_engine_state(jme_moshier_planet_state, 2451545.0, cases[i].body, state) != 0 || derive_elements(state, elem) != 0) {
            fprintf(stderr, "Moshier element derivation failed for %s\n", cases[i].name);
            return 1;
        }

        snprintf(label, sizeof(label), "Moshier %s a", cases[i].name);
        if (check_close(label, elem[0], cases[i].a, 2e-3) != 0) { return 1; }
        snprintf(label, sizeof(label), "Moshier %s e", cases[i].name);
        if (check_close(label, elem[1], cases[i].e, 2e-3) != 0) { return 1; }
        snprintf(label, sizeof(label), "Moshier %s i", cases[i].name);
        if (check_close(label, elem[2], cases[i].inc, 0.2) != 0) { return 1; }
        snprintf(label, sizeof(label), "Moshier %s peri", cases[i].name);
        if (check_close(label, angular_distance_deg(elem[4], cases[i].long_peri), 0.0, 2.0) != 0) { return 1; }
        snprintf(label, sizeof(label), "Moshier %s node", cases[i].name);
        if (check_close(label, angular_distance_deg(elem[3], cases[i].node), 0.0, 2.0) != 0) { return 1; }

        if (build_engine_state(jme_meeus_planet_state, 2451545.0, cases[i].body, state) != 0 || derive_elements(state, elem) != 0) {
            fprintf(stderr, "Meeus element derivation failed for %s\n", cases[i].name);
            return 1;
        }

        snprintf(label, sizeof(label), "Meeus %s a", cases[i].name);
        if (check_close(label, elem[0], cases[i].a, 1e-1) != 0) { return 1; }
        snprintf(label, sizeof(label), "Meeus %s e", cases[i].name);
        if (check_close(label, elem[1], cases[i].e, 1e-1) != 0) { return 1; }
        snprintf(label, sizeof(label), "Meeus %s i", cases[i].name);
        if (check_close(label, elem[2], cases[i].inc, 2.0) != 0) { return 1; }
        snprintf(label, sizeof(label), "Meeus %s peri", cases[i].name);
        if (check_close(label, angular_distance_deg(elem[4], cases[i].long_peri), 0.0, 10.0) != 0) { return 1; }
        snprintf(label, sizeof(label), "Meeus %s node", cases[i].name);
        if (check_close(label, angular_distance_deg(elem[3], cases[i].node), 0.0, 10.0) != 0) { return 1; }
    }

    return 0;
}

static int check_engine_public_state_contracts(void)
{
    double state[6];
    int planet_bodies[] = {
        JME_BODY_MERCURY,
        JME_BODY_VENUS,
        JME_BODY_EARTH,
        JME_BODY_MARS,
        JME_BODY_JUPITER,
        JME_BODY_SATURN,
        JME_BODY_URANUS,
        JME_BODY_NEPTUNE,
        JME_BODY_PLUTO
    };
    int vsop_bodies[] = {
        JME_BODY_MERCURY,
        JME_BODY_VENUS,
        JME_BODY_EARTH,
        JME_BODY_MARS,
        JME_BODY_JUPITER,
        JME_BODY_SATURN,
        JME_BODY_URANUS,
        JME_BODY_NEPTUNE
    };
    int i;

    for (i = 0; i < (int)(sizeof(vsop_bodies) / sizeof(vsop_bodies[0])); i++) {
        if (jme_vsop87_planet_state(2451545.0, vsop_bodies[i], state) != JME_OK
            || !isfinite(state[0]) || !isfinite(state[1]) || !isfinite(state[2])
            || !isfinite(state[3]) || !isfinite(state[4]) || !isfinite(state[5])
            || jme_state_distance(state) <= 0.0
            || jme_state_speed(state) <= 0.0) {
            fprintf(stderr, "VSOP87 public state contract failed for body %d\n", vsop_bodies[i]);
            return 1;
        }
    }

    if (jme_vsop87_planet_state(2451545.0, JME_BODY_PLUTO, state) != JME_ERR) {
        fprintf(stderr, "VSOP87 unexpectedly accepted unsupported Pluto fallback\n");
        return 1;
    }

    if (jme_vsop87_planet_state(2451545.0, JME_BODY_SUN, state) != JME_ERR
        || jme_vsop87_planet_state(2451545.0, JME_BODY_MARS, 0) != JME_ERR) {
        fprintf(stderr, "VSOP87 invalid-input contract failed\n");
        return 1;
    }

    for (i = 0; i < (int)(sizeof(planet_bodies) / sizeof(planet_bodies[0])); i++) {
        if (jme_moshier_planet_state(2451545.0, planet_bodies[i], state) != JME_OK
            || !isfinite(state[0]) || !isfinite(state[1]) || !isfinite(state[2])
            || !isfinite(state[3]) || !isfinite(state[4]) || !isfinite(state[5])
            || jme_state_distance(state) <= 0.0
            || jme_state_speed(state) <= 0.0) {
            fprintf(stderr, "Moshier public state contract failed for body %d\n", planet_bodies[i]);
            return 1;
        }

        if (jme_meeus_planet_state(2451545.0, planet_bodies[i], state) != JME_OK
            || !isfinite(state[0]) || !isfinite(state[1]) || !isfinite(state[2])
            || !isfinite(state[3]) || !isfinite(state[4]) || !isfinite(state[5])
            || jme_state_distance(state) <= 0.0
            || jme_state_speed(state) <= 0.0) {
            fprintf(stderr, "Meeus public state contract failed for body %d\n", planet_bodies[i]);
            return 1;
        }
    }

    if (jme_moshier_planet_state(2451545.0, JME_BODY_MOON, state) != JME_ERR
        || jme_moshier_planet_state(2451545.0, JME_BODY_MERCURY, 0) != JME_ERR
        || jme_meeus_planet_state(2451545.0, JME_BODY_MOON, state) != JME_ERR
        || jme_meeus_planet_state(2451545.0, JME_BODY_MERCURY, 0) != JME_ERR) {
        fprintf(stderr, "Moshier/Meeus invalid-input contract failed\n");
        return 1;
    }

    if (jme_elp2000_moon_state(2451545.0, state) != JME_OK
        || !isfinite(state[0]) || !isfinite(state[1]) || !isfinite(state[2])
        || !isfinite(state[3]) || !isfinite(state[4]) || !isfinite(state[5])
        || jme_state_distance(state) <= 0.0
        || jme_state_speed(state) <= 0.0
        || jme_elp2000_moon_state(2451545.0, 0) != JME_ERR) {
        fprintf(stderr, "ELP2000 public state contract failed\n");
        return 1;
    }

    if (jme_meeus_sun_state(2451545.0, state) != JME_OK
        || jme_state_distance(state) <= 0.0
        || jme_state_speed(state) <= 0.0
        || jme_meeus_sun_state(2451545.0, 0) != JME_ERR) {
        fprintf(stderr, "Meeus Sun state contract failed\n");
        return 1;
    }

    if (jme_meeus_moon_state(2451545.0, state) != JME_OK
        || jme_state_distance(state) <= 0.0
        || jme_state_speed(state) <= 0.0
        || jme_meeus_moon_state(2451545.0, 0) != JME_ERR) {
        fprintf(stderr, "Meeus Moon state contract failed\n");
        return 1;
    }

    return 0;
}

static int check_high_level_analytical_fallback_order(void)
{
    struct planet_case {
        int body;
        const char *name;
    } planet_cases[] = {
        {JME_BODY_MERCURY, "Mercury"},
        {JME_BODY_EARTH, "Earth"},
        {JME_BODY_PLUTO, "Pluto"}
    };
    double jd_cases[] = {
        2451545.0,
        2415020.5,
        2488070.5
    };
    char error[256] = "";
    int i;
    int j;

    jme_jpl_close();
    jme_set_astro_models("ENGINE=MOSHIER", 0);

    for (i = 0; i < (int)(sizeof(planet_cases) / sizeof(planet_cases[0])); i++) {
        for (j = 0; j < (int)(sizeof(jd_cases) / sizeof(jd_cases[0])); j++) {
            double calc_state[6];
            double moshier_state[6];
            int k;

            if (jme_calc(jd_cases[j], planet_cases[i].body, JME_CALC_TRUE_POSITION | JME_CALC_HELIOCENTRIC | JME_CALC_XYZ, calc_state, error) != JME_OK) {
                fprintf(stderr, "jme_calc analytical fallback failed for %s at JD %.1f: %s\n", planet_cases[i].name, jd_cases[j], error);
                return 1;
            }

            if (jme_moshier_planet_state(jd_cases[j], planet_cases[i].body, moshier_state) != JME_OK
                || convert_j2000_ecliptic_to_equatorial_of_date(jd_cases[j], moshier_state, moshier_state, error) != JME_OK) {
                fprintf(stderr, "direct Moshier state failed for %s at JD %.1f\n", planet_cases[i].name, jd_cases[j]);
                return 1;
            }

            for (k = 0; k < 6; k++) {
                char label[128];
                snprintf(label, sizeof(label), "jme_calc Moshier-first %s JD %.1f[%d]", planet_cases[i].name, jd_cases[j], k);
                if (check_close(label, calc_state[k], moshier_state[k], 1.0e-14) != 0) {
                    return 1;
                }
            }
        }
    }

    for (j = 0; j < (int)(sizeof(jd_cases) / sizeof(jd_cases[0])); j++) {
        double calc_state[6];
        int k;

        if (jme_calc(jd_cases[j], JME_BODY_MOON, JME_CALC_TRUE_POSITION | JME_CALC_HELIOCENTRIC | JME_CALC_XYZ, calc_state, error) != JME_OK) {
            fprintf(stderr, "jme_calc Moshier Moon failed at JD %.1f: %s\n", jd_cases[j], error);
            return 1;
        }

        for (k = 0; k < 6; k++) {
            if (!isfinite(calc_state[k])) {
                fprintf(stderr, "jme_calc Moshier Moon produced non-finite state at JD %.1f[%d]\n", jd_cases[j], k);
                return 1;
            }
        }
    }

    jme_set_astro_models("ENGINE=VSOP_ELP_MEEUS", 0);

    for (j = 0; j < (int)(sizeof(jd_cases) / sizeof(jd_cases[0])); j++) {
        double calc_state[6];
        double vsop_state[6];
        double meeus_state[6];
        int k;

        if (jme_calc(jd_cases[j], JME_BODY_MERCURY, JME_CALC_TRUE_POSITION | JME_CALC_HELIOCENTRIC | JME_CALC_XYZ, calc_state, error) != JME_OK
            || jme_vsop87_planet_state(jd_cases[j], JME_BODY_MERCURY, vsop_state) != JME_OK
            || convert_j2000_ecliptic_to_equatorial_of_date(jd_cases[j], vsop_state, vsop_state, error) != JME_OK) {
            fprintf(stderr, "VSOP+ELP+Meeus Mercury fallback failed at JD %.1f\n", jd_cases[j]);
            return 1;
        }
        for (k = 0; k < 6; k++) {
            char label[128];
            snprintf(label, sizeof(label), "jme_calc VSOP-selected Mercury JD %.1f[%d]", jd_cases[j], k);
            if (check_close(label, calc_state[k], vsop_state[k], 1.0e-14) != 0) {
                return 1;
            }
        }

        if (jme_calc(jd_cases[j], JME_BODY_PLUTO, JME_CALC_TRUE_POSITION | JME_CALC_HELIOCENTRIC | JME_CALC_XYZ, calc_state, error) != JME_OK
            || jme_meeus_planet_state(jd_cases[j], JME_BODY_PLUTO, meeus_state) != JME_OK
            || convert_j2000_ecliptic_to_equatorial_of_date(jd_cases[j], meeus_state, meeus_state, error) != JME_OK) {
            fprintf(stderr, "VSOP+ELP+Meeus Pluto fallback failed at JD %.1f\n", jd_cases[j]);
            return 1;
        }
        for (k = 0; k < 6; k++) {
            char label[128];
            snprintf(label, sizeof(label), "jme_calc Meeus-selected Pluto JD %.1f[%d]", jd_cases[j], k);
            if (check_close(label, calc_state[k], meeus_state[k], 1.0e-14) != 0) {
                return 1;
            }
        }
    }

    jme_set_astro_models(0, 0);

    return 0;
}

static int check_horizons_observer_ecliptic_regression(void)
{
    const double jd_ut = 2461182.5; /* 2026-05-22 00:00 UTC */
    const struct {
        int body;
        const char *name;
        double lon;
        double lat;
        double lon_tolerance;
        double lat_tolerance;
    } cases[] = {
        {JME_BODY_SUN, "Sun", 60.9377788, 0.0000444, 0.02, 0.01},
        {JME_BODY_MOON, "Moon", 132.4438470, 1.9106796, 0.02, 0.02}
    };
    char error[256];
    int i;

    jme_set_astro_models("ENGINE=MOSHIER", 0);
    for (i = 0; i < (int)(sizeof(cases) / sizeof(cases[0])); i++) {
        double result[6];
        double lon_delta;
        double lat_delta;

        if (jme_calc_ut(jd_ut, cases[i].body, 0, result, error) != JME_OK) {
            fprintf(stderr, "Horizons regression %s calculation failed: %s\n", cases[i].name, error);
            return 1;
        }

        lon_delta = fabs(jme_degrees_difference_signed(result[0], cases[i].lon));
        lat_delta = fabs(result[1] - cases[i].lat);
        if (lon_delta > cases[i].lon_tolerance || lat_delta > cases[i].lat_tolerance) {
            fprintf(stderr,
                "Horizons regression %s mismatch: got lon %.10f lat %.10f, expected lon %.10f lat %.10f, delta lon %.10f lat %.10f\n",
                cases[i].name, result[0], result[1], cases[i].lon, cases[i].lat, lon_delta, lat_delta);
            return 1;
        }
    }
    jme_set_astro_models(0, 0);

    return 0;
}

static int check_horizons_major_body_matrix(void)
{
    struct horizons_case {
        double jd_ut;
        const char *date_name;
        int body;
        const char *body_name;
        double lon;
        double lat;
    } cases[] = {
        {2451544.5, "2000-01-01T00:00:00Z", JME_BODY_SUN, "Sun", 279.8592049, 0.0002403},
        {2451544.5, "2000-01-01T00:00:00Z", JME_BODY_MOON, "Moon", 217.2933209, 5.2312994},
        {2451544.5, "2000-01-01T00:00:00Z", JME_BODY_MERCURY, "Mercury", 271.1117994, -0.9456805},
        {2451544.5, "2000-01-01T00:00:00Z", JME_BODY_VENUS, "Venus", 240.9614017, 2.0802345},
        {2451544.5, "2000-01-01T00:00:00Z", JME_BODY_MARS, "Mars", 327.5754592, -1.0740062},
        {2451544.5, "2000-01-01T00:00:00Z", JME_BODY_JUPITER, "Jupiter", 25.2331086, -1.2647766},
        {2451544.5, "2000-01-01T00:00:00Z", JME_BODY_SATURN, "Saturn", 40.4058374, -2.4472226},
        {2451544.5, "2000-01-01T00:00:00Z", JME_BODY_URANUS, "Uranus", 314.7840519, -0.6584479},
        {2451544.5, "2000-01-01T00:00:00Z", JME_BODY_NEPTUNE, "Neptune", 303.1752428, 0.2351174},
        {2451544.5, "2000-01-01T00:00:00Z", JME_BODY_PLUTO, "Pluto", 251.4371500, 10.8545444},

        {2461182.5, "2026-05-22T00:00:00Z", JME_BODY_SUN, "Sun", 60.9377788, 0.0000444},
        {2461182.5, "2026-05-22T00:00:00Z", JME_BODY_MOON, "Moon", 132.4438470, 1.9106796},
        {2461182.5, "2026-05-22T00:00:00Z", JME_BODY_MERCURY, "Mercury", 69.8371739, 1.3481608},
        {2461182.5, "2026-05-22T00:00:00Z", JME_BODY_VENUS, "Venus", 93.5275757, 1.6891227},
        {2461182.5, "2026-05-22T00:00:00Z", JME_BODY_MARS, "Mars", 32.3037968, -0.7195196},
        {2461182.5, "2026-05-22T00:00:00Z", JME_BODY_JUPITER, "Jupiter", 112.2434940, 0.4045434},
        {2461182.5, "2026-05-22T00:00:00Z", JME_BODY_SATURN, "Saturn", 11.3448285, -2.2275012},
        {2461182.5, "2026-05-22T00:00:00Z", JME_BODY_URANUS, "Uranus", 61.4815444, -0.1607086},
        {2461182.5, "2026-05-22T00:00:00Z", JME_BODY_NEPTUNE, "Neptune", 3.8455532, -1.3329376},
        {2461182.5, "2026-05-22T00:00:00Z", JME_BODY_PLUTO, "Pluto", 305.4552740, -4.1118264},

        {2469807.5, "2050-01-01T00:00:00Z", JME_BODY_SUN, "Sun", 280.7483803, 0.0001127},
        {2469807.5, "2050-01-01T00:00:00Z", JME_BODY_MOON, "Moon", 18.6755950, 3.3912133},
        {2469807.5, "2050-01-01T00:00:00Z", JME_BODY_MERCURY, "Mercury", 270.0594864, 3.1357631},
        {2469807.5, "2050-01-01T00:00:00Z", JME_BODY_VENUS, "Venus", 281.2480360, -0.6060626},
        {2469807.5, "2050-01-01T00:00:00Z", JME_BODY_MARS, "Mars", 227.7146917, 0.7726782},
        {2469807.5, "2050-01-01T00:00:00Z", JME_BODY_JUPITER, "Jupiter", 121.6915463, 0.4580707},
        {2469807.5, "2050-01-01T00:00:00Z", JME_BODY_SATURN, "Saturn", 297.5742782, -0.2035839},
        {2469807.5, "2050-01-01T00:00:00Z", JME_BODY_URANUS, "Uranus", 170.7326082, 0.7867256},
        {2469807.5, "2050-01-01T00:00:00Z", JME_BODY_NEPTUNE, "Neptune", 53.6034294, -1.7627912},
        {2469807.5, "2050-01-01T00:00:00Z", JME_BODY_PLUTO, "Pluto", 337.5330656, -12.6913738}
    };
    struct engine_case {
        const char *models;
        const char *name;
        double lon_tolerance;
        double lat_tolerance;
    } engines[] = {
        {"ENGINE=MOSHIER", "MOSHIER", 0.02, 0.02},
        {"ENGINE=VSOP_ELP_MEEUS", "VSOP_ELP_MEEUS", 0.03, 0.02}
    };
    char error[256];
    int i;
    int j;

    for (j = 0; j < (int)(sizeof(engines) / sizeof(engines[0])); j++) {
        jme_set_astro_models(engines[j].models, 0);
        for (i = 0; i < (int)(sizeof(cases) / sizeof(cases[0])); i++) {
            double result[6];
            double lon_delta;
            double lat_delta;

            if (jme_calc_ut(cases[i].jd_ut, cases[i].body, 0, result, error) != JME_OK) {
                fprintf(stderr, "Horizons major-body matrix %s %s failed: %s\n",
                    engines[j].name, cases[i].body_name, error);
                return 1;
            }

            lon_delta = fabs(jme_degrees_difference_signed(result[0], cases[i].lon));
            lat_delta = fabs(result[1] - cases[i].lat);
            if (lon_delta > engines[j].lon_tolerance || lat_delta > engines[j].lat_tolerance) {
                fprintf(stderr,
                    "Horizons major-body matrix %s %s %s mismatch: got lon %.10f lat %.10f, expected lon %.10f lat %.10f, delta lon %.10f lat %.10f\n",
                    engines[j].name, cases[i].date_name, cases[i].body_name,
                    result[0], result[1], cases[i].lon, cases[i].lat, lon_delta, lat_delta);
                return 1;
            }
        }
    }

    jme_set_astro_models(0, 0);
    return 0;
}

int main(void)
{
    if (check_vsop87a_j2000() != 0) {
        return 1;
    }

    if (check_elp2000_reference() != 0) {
        return 1;
    }

    if (check_engine_elements_against_jpl_table() != 0) {
        return 1;
    }

    if (check_engine_public_state_contracts() != 0) {
        return 1;
    }

    if (check_high_level_analytical_fallback_order() != 0) {
        return 1;
    }

    if (check_horizons_observer_ecliptic_regression() != 0) {
        return 1;
    }

    if (check_horizons_major_body_matrix() != 0) {
        return 1;
    }

    return 0;
}
