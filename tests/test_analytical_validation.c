#include "jme/jme.h"

#include <math.h>
#include <stdio.h>

static int check_close(const char *label, double got, double expected, double tolerance)
{
    if (!isfinite(got) || fabs(got - expected) > tolerance) {
        fprintf(stderr, "%s mismatch: got %.17g expected %.17g tolerance %.17g\n", label, got, expected, tolerance);
        return 1;
    }
    return 0;
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
    double state[6] = {0.0, 0.0, 0.0, 0.0, 0.0, 0.0};
    double expected_km[3] = {382979.7604730463, -68204.20174530084, -25987.71602589964};
    int i;

    if (jme_elp2000_moon_state(2451555.5, state) != JME_OK) {
        fprintf(stderr, "ELP2000 state failed\n");
        return 1;
    }

    for (i = 0; i < 3; i++) {
        char label[64];
        snprintf(label, sizeof(label), "ELP2000 Moon km[%d]", i);
        if (check_close(label, state[i] * JME_AU_KM, expected_km[i], 10.0) != 0) {
            return 1;
        }
    }

    if (!isfinite(state[3]) || !isfinite(state[4]) || !isfinite(state[5])
        || fabs(state[3]) + fabs(state[4]) + fabs(state[5]) <= 0.0) {
        fprintf(stderr, "ELP2000 velocity is not finite/non-zero\n");
        return 1;
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

    return 0;
}
