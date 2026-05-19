#include "jme/jme.h"
#include "context.h"
#include <math.h>

#define JME_ARCSEC_TO_RAD 4.848136811095359935899141023221522846734e-6
#define JME_DEG_TO_RAD 0.017453292519943295769236907684886127134428718885417

typedef struct jme_nutation_term {
    signed char l;
    signed char lp;
    signed char f;
    signed char d;
    signed char om;
    int psi_a;
    int psi_b;
    int eps_a;
    int eps_b;
} jme_nutation_term;

/* IAU 1980 Nutation coefficients. Units: 0.0001 arcsec.
   Source: Seidelmann, 1982 (IAU 1980 Theory of Nutation).
   Fundamental arguments: l, l', F, D, Omega. */
static const jme_nutation_term nut80[] = {
    { 0,  0,  0,  0,  1, -171996, -1742,  92025,   89},
    { 0,  0,  0,  0,  2,    2062,     2,   -895,    5},
    {-2,  0,  2,  0,  1,      46,     0,    -24,    0},
    { 2,  0, -2,  0,  0,      11,     0,      0,    0},
    {-2,  0,  2,  0,  2,      -3,     0,      1,    0},
    { 1, -1,  0, -1,  0,      -3,     0,      0,    0},
    { 0, -2,  2, -2,  1,      -2,     0,      1,    0},
    { 2,  0, -2,  0,  1,       1,     0,      0,    0},
    { 0,  0,  2, -2,  2,  -13187,   -16,   5736,  -31},
    { 0,  1,  0,  0,  0,    1426,   -34,     54,   -1},
    { 0,  1,  2, -2,  2,    -517,    12,    224,   -6},
    { 0, -1,  2, -2,  2,     217,    -5,    -95,    3},
    { 0,  0,  2, -2,  1,     129,     1,    -70,    0},
    { 2,  0,  0, -2,  0,      48,     0,      1,    0},
    { 0,  0,  2, -2,  0,     -22,     0,      0,    0},
    { 0,  2,  0,  0,  0,      17,    -1,      0,    0},
    { 0,  1,  0,  0,  1,     -15,     0,      9,    0},
    { 0,  2,  2, -2,  2,     -16,     1,      7,    0},
    { 0, -1,  0,  0,  1,     -12,     0,      6,    0},
    {-2,  0,  0,  2,  1,      -6,     0,      3,    0},
    { 0, -1,  2, -2,  1,      -5,     0,      3,    0},
    { 2,  0,  0, -2,  1,       4,     0,     -2,    0},
    { 0,  1,  2, -2,  1,       4,     0,     -2,    0},
    { 1,  0,  0, -1,  0,      -4,     0,      0,    0},
    { 2,  1,  0, -2,  0,       1,     0,      0,    0},
    { 0,  0, -2,  2,  1,       1,     0,      0,    0},
    { 0,  1, -2,  2,  0,      -1,     0,      0,    0},
    { 0,  1,  0,  0,  2,       1,     0,      0,    0},
    {-1,  0,  0,  1,  1,       1,     0,      0,    0},
    { 0,  1,  2, -2,  0,      -1,     0,      0,    0},
    { 0,  0,  2,  0,  2,   -2274,    -2,    977,   -5},
    { 1,  0,  0,  0,  0,     712,     1,     -7,    0},
    { 0,  0,  2,  0,  1,    -386,    -4,    200,    0},
    { 1,  0,  2,  0,  2,    -301,     0,    129,   -1},
    { 1,  0,  0, -2,  0,    -158,     0,     -1,    0},
    {-1,  0,  2,  0,  2,     123,     0,    -53,    0},
    { 0,  0,  0,  2,  0,      63,     0,     -2,    0},
    { 1,  0,  0,  0,  1,      63,     1,    -33,    0},
    {-1,  0,  0,  0,  1,     -58,    -1,     32,    0},
    {-1,  0,  2,  2,  2,     -59,     0,     26,    0},
    { 1,  0,  2,  0,  1,     -51,     0,     27,    0},
    { 0,  0,  2,  2,  2,     -38,     0,     16,    0},
    { 2,  0,  0,  0,  0,      29,     0,     -1,    0},
    { 1,  0,  2, -2,  2,      29,     0,    -12,    0},
    { 2,  0,  2,  0,  2,     -31,     0,     13,    0},
    { 0,  0,  2,  0,  0,      26,     0,     -1,    0},
    {-1,  0,  2,  0,  1,      21,     0,    -10,    0},
    {-1,  0,  0,  2,  1,      16,     0,     -8,    0},
    { 1,  0,  0, -2,  1,     -13,     0,      7,    0},
    {-1,  0,  2,  2,  1,     -10,     0,      5,    0},
    { 1,  1,  0,  0,  0,      -7,     0,      0,    0},
    { 0,  1,  2,  0,  2,       7,     0,     -3,    0},
    { 0, -1,  2,  0,  2,      -7,     0,      3,    0},
    { 1,  0,  2,  2,  2,      -8,     0,      3,    0},
    { 1,  0,  0,  2,  0,       6,     0,      0,    0},
    { 2,  0,  2, -2,  2,       6,     0,     -3,    0},
    { 0,  0,  0,  2,  1,      -6,     0,      3,    0},
    { 0,  0,  2,  2,  1,      -6,     0,      3,    0},
    { 1,  0,  2,  0,  0,       5,     0,      0,    0},
    {-1,  0,  0,  0,  2,      -5,     0,      3,    0},
    { 1,  0,  0, -4,  0,      -5,     0,      0,    0},
    {-2,  0,  2,  2,  2,       5,     0,     -2,    0},
    {-1,  0,  2,  4,  2,      -4,     0,      2,    0},
    { 2,  0,  0, -4,  0,       4,     0,      0,    0},
    { 1,  1,  2,  0,  2,      -4,     0,      2,    0},
    { 1,  0,  2, -2,  1,      -4,     0,      2,    0},
    {-2,  0,  2,  4,  2,      -3,     0,      1,    0},
    {-1,  0,  4,  0,  2,      -3,     0,      1,    0},
    { 1, -1,  0,  0,  0,      -3,     0,      0,    0},
    { 1,  0,  2,  2,  1,      -3,     0,      1,    0},
    {-2,  0,  2,  4,  1,      -2,     0,      1,    0},
    {-1,  0,  4,  2,  2,      -2,     0,      1,    0},
    { 1, -1,  2,  0,  2,      -2,     0,      1,    0},
    { 1,  0,  0,  2,  1,      -2,     0,      1,    0},
    { 2,  0,  2,  0,  1,      -2,     0,      1,    0},
    { 0,  0,  4, -2,  2,       2,     0,     -1,    0},
    { 3,  0,  0,  0,  0,       2,     0,      0,    0},
    { 1,  1,  2, -2,  2,      -1,     0,      1,    0},
    { 0,  0,  2,  0, -2,       1,     0,      0,    0},
    {-1, -1,  2,  2,  2,       1,     0,     -1,    0},
    {-1,  0,  2, -2,  1,       1,     0,     -1,    0},
    {-2,  0,  0,  2,  0,       1,     0,      0,    0},
    { 3,  0,  2,  0,  2,      -1,     0,      1,    0},
    { 0,  1,  2,  2,  2,      -1,     0,      1,    0},
    { 1,  1,  0, -2,  0,      -1,     0,      0,    0},
    { 0,  1,  0,  2,  0,       1,     0,      0,    0},
    { 1, -1,  2,  2,  2,      -1,     0,      1,    0},
    {-1,  0,  0,  0,  3,       1,     0,     -1,    0},
    { 2, -1,  0, -2,  0,       1,     0,      0,    0},
    { 1,  0,  0, -2,  2,       1,     0,      0,    0},
    {-1,  0,  2,  2,  0,       1,     0,      0,    0},
    { 0,  0,  2,  4,  2,      -1,     0,      0,    0},
    { 0,  1,  0, -2,  0,       1,     0,      0,    0},
    { 1,  0, -2,  0,  0,       1,     0,      0,    0},
    { 2,  0,  2, -2,  1,       1,     0,     -1,    0},
    { 1,  0,  2,  2,  0,      -1,     0,      0,    0},
    { 1,  0,  0,  2,  2,      -1,     0,      0,    0},
    {-1,  0,  2,  4,  1,      -1,     0,      0,    0},
    { 0,  2,  2, -2,  1,      -1,     0,      0,    0},
    { 1,  0,  2, -2,  0,      -1,     0,      0,    0},
    {-2,  0,  2,  4,  0,       1,     0,      0,    0},
    {-1,  0,  4,  0,  1,       1,     0,      0,    0},
    { 1,  1,  2,  0,  1,      -1,     0,      0,    0},
    { 1,  0,  4,  0,  2,       1,     0,      0,    0},
    {-2,  1,  0,  2,  0,       1,     0,      0,    0},
    { 1, -1,  0, -2,  0,       1,     0,      0,    0}
};

static void jme_nutation_args(double t, double *args)
{
    /* Fundamental arguments (IAU 1980). Units: arcsec.
       T is Julian centuries from J2000.0. */
    double l, lp, f, d, om;

    l = 485866.733 + (1325.0 * 1296000.0 + 715922.633) * t + 31.310 * t * t + 0.064 * t * t * t;
    lp = 1287099.804 + (99.0 * 1296000.0 + 1292581.224) * t - 0.577 * t * t - 0.012 * t * t * t;
    f = 335277.910 + (1342.0 * 1296000.0 + 295263.137) * t - 13.257 * t * t + 0.011 * t * t * t;
    d = 1072261.307 + (1236.0 * 1296000.0 + 1105601.328) * t - 6.891 * t * t + 0.019 * t * t * t;
    om = 450160.280 - (5.0 * 1296000.0 + 482890.539) * t + 7.455 * t * t + 0.008 * t * t * t;

    args[0] = fmod(l, 1296000.0) * JME_ARCSEC_TO_RAD;
    args[1] = fmod(lp, 1296000.0) * JME_ARCSEC_TO_RAD;
    args[2] = fmod(f, 1296000.0) * JME_ARCSEC_TO_RAD;
    args[3] = fmod(d, 1296000.0) * JME_ARCSEC_TO_RAD;
    args[4] = fmod(om, 1296000.0) * JME_ARCSEC_TO_RAD;
}

int jme_get_nutation(double jd_et, int model, double *dpsi, double *deps, char *error)
{
    double t = (jd_et - 2451545.0) / 36525.0;
    double args[5];
    double arg;
    double dp = 0.0;
    double de = 0.0;
    size_t i;

    if (dpsi != 0) { *dpsi = 0.0; }
    if (deps != 0) { *deps = 0.0; }

    if (model != JME_MODEL_NUT_IAU_1980 && model != JME_MODEL_NUT_IAU_2000B) {
        /* Default to 1980 for now if other models not yet implemented */
        model = JME_MODEL_NUT_IAU_1980;
    }

    if (model == JME_MODEL_NUT_IAU_1980) {
        jme_nutation_args(t, args);
        for (i = 0; i < sizeof(nut80) / sizeof(nut80[0]); i++) {
            arg = (double)nut80[i].l * args[0] + (double)nut80[i].lp * args[1] + (double)nut80[i].f * args[2] + (double)nut80[i].d * args[3] + (double)nut80[i].om * args[4];
            dp += ((double)nut80[i].psi_a + (double)nut80[i].psi_b * 0.1 * t) * sin(arg);
            de += ((double)nut80[i].eps_a + (double)nut80[i].eps_b * 0.1 * t) * cos(arg);
        }

        if (dpsi != 0) { *dpsi = dp * 0.0001 / 3600.0; } /* degrees */
        if (deps != 0) { *deps = de * 0.0001 / 3600.0; } /* degrees */
        return JME_OK;
    }

    jme_set_error(error, "Unsupported nutation model");
    return JME_ERR;
}

int jme_get_obliquity(double jd_et, int model, double *eps, char *error)
{
    double t = (jd_et - 2451545.0) / 36525.0;
    double eps0;

    (void)error;

    if (eps != 0) { *eps = 0.0; }

    switch (model) {
    case JME_MODEL_OBL_IAU_1980:
        /* IAU 1980 Mean Obliquity */
        eps0 = 84381.448 - 46.8150 * t - 0.00059 * t * t + 0.001813 * t * t * t;
        if (eps != 0) { *eps = eps0 / 3600.0; }
        return JME_OK;
    case JME_MODEL_OBL_IAU_2000:
    case JME_MODEL_OBL_IAU_2006:
        /* IAU 2006 (P03) Mean Obliquity */
        eps0 = 84381.406 - 46.836769 * t - 0.01831 * t * t + 0.0020034 * t * t * t
               - 0.000000576 * t * t * t * t - 0.0000000434 * t * t * t * t * t;
        if (eps != 0) { *eps = eps0 / 3600.0; }
        return JME_OK;
    default:
        return jme_get_obliquity(jd_et, JME_MODEL_OBL_IAU_1980, eps, error);
    }
}

int jme_get_precession_matrix(double jd_start, double jd_end, int model, double *m)
{
    double t = (jd_start - 2451545.0) / 36525.0;
    double dt = (jd_end - jd_start) / 36525.0;
    double zeta, z, theta;

    jme_matrix_identity(m);

    if (model != JME_MODEL_PREC_IAU_1976) {
        /* Default to 1976 for now */
        model = JME_MODEL_PREC_IAU_1976;
    }

    if (model == JME_MODEL_PREC_IAU_1976) {
        /* IAU 1976 Precession (Lieske et al. 1977) */
        zeta = (2306.2181 + 1.39656 * t - 0.000139 * t * t) * dt
               + (0.30188 - 0.000344 * t) * dt * dt + 0.017998 * dt * dt * dt;
        z = zeta + (0.79280 + 0.000411 * t) * dt * dt + 0.000205 * dt * dt * dt;
        theta = (2004.3109 - 0.85330 * t - 0.000217 * t * t) * dt
                - (0.42665 + 0.000217 * t) * dt * dt - 0.041833 * dt * dt * dt;

        zeta *= JME_ARCSEC_TO_RAD;
        z *= JME_ARCSEC_TO_RAD;
        theta *= JME_ARCSEC_TO_RAD;

        jme_matrix_rotate_z(-zeta, m);
        jme_matrix_rotate_y(theta, m);
        jme_matrix_rotate_z(-z, m);

        return JME_OK;
    }

    return JME_ERR;
}

void jme_get_nutation_matrix(double dpsi_rad, double deps_rad, double eps_rad, double *m)
{
    jme_matrix_identity(m);
    jme_matrix_rotate_x(eps_rad, m);
    jme_matrix_rotate_z(-dpsi_rad, m);
    jme_matrix_rotate_x(-(eps_rad + deps_rad), m);
}

int jme_get_topo_pos(double jd_et, double *pos_au, char *error)
{
    jme_context *ctx = jme_get_context();
    double eps, dpsi, deps;
    double gst, lst_rad;
    double cos_phi, sin_phi;
    double a = 6378.137; /* WGS84 semi-major axis in km */
    double f = 1.0 / 298.257223563; /* WGS84 flattening */
    double c, s;
    double x_km, y_km, z_km;

    if (jme_get_obliquity(jd_et, JME_MODEL_OBL_IAU_1980, &eps, error) != JME_OK) { return JME_ERR; }
    if (jme_get_nutation(jd_et, JME_MODEL_NUT_IAU_1980, &dpsi, &deps, error) != JME_OK) { return JME_ERR; }

    /* Greenwhich Sidereal Time (True) */
    gst = jme_sidereal_time0(jd_et, eps, dpsi); /* Note: jme_sidereal_time0 takes jd_ut, but jd_et is close enough for basic topo */
    /* Local Sidereal Time */
    lst_rad = (gst * 15.0 + ctx->topo_lon) * JME_DEG_TO_RAD;

    cos_phi = cos(ctx->topo_lat * JME_DEG_TO_RAD);
    sin_phi = sin(ctx->topo_lat * JME_DEG_TO_RAD);

    c = 1.0 / sqrt(cos_phi * cos_phi + (1.0 - f) * (1.0 - f) * sin_phi * sin_phi);
    s = (1.0 - f) * (1.0 - f) * c;

    x_km = (a * c + ctx->topo_alt * 0.001) * cos_phi * cos(lst_rad);
    y_km = (a * c + ctx->topo_alt * 0.001) * cos_phi * sin(lst_rad);
    z_km = (a * s + ctx->topo_alt * 0.001) * sin_phi;

    /* x_km, y_km, z_km are in True Equatorial system of date.
       Need to convert to Mean Equatorial J2000 for relative vector arithmetic. */
    {
        double prec_mat[9];
        double nut_mat[9];
        double state[6];

        state[0] = x_km;
        state[1] = y_km;
        state[2] = z_km;
        state[3] = 0.0;
        state[4] = 0.0;
        state[5] = 0.0;

        /* Invert Nutation and Precession to get back to J2000 */
        jme_get_nutation_matrix(dpsi * JME_DEG_TO_RAD, deps * JME_DEG_TO_RAD, eps * JME_DEG_TO_RAD, nut_mat);
        /* Simple matrix inversion for rotation matrices is transpose */
        {
            double inv_nut[9];
            int i, j;
            for (i = 0; i < 3; i++) for (j = 0; j < 3; j++) inv_nut[i * 3 + j] = nut_mat[j * 3 + i];
            jme_matrix_transform_state(inv_nut, state, state);
        }

        jme_get_precession_matrix(2451545.0, jd_et, JME_MODEL_PREC_IAU_1976, prec_mat);
        {
            double inv_prec[9];
            int i, j;
            for (i = 0; i < 3; i++) for (j = 0; j < 3; j++) inv_prec[i * 3 + j] = prec_mat[j * 3 + i];
            jme_matrix_transform_state(inv_prec, state, state);
        }

        pos_au[0] = state[0] / JME_AU_KM;
        pos_au[1] = state[1] / JME_AU_KM;
        pos_au[2] = state[2] / JME_AU_KM;
    }

    return JME_OK;
}
