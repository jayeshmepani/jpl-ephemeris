#include "jme/jme.h"
#include "context.h"
#include <math.h>

#define JME_DEG_TO_RAD 0.017453292519943295769236907684886127134428718885417
#define JME_RAD_TO_DEG 57.295779513082320876798154814105170332405472466565
#define JME_PI 3.1415926535897932384626433832795028841971693993751

static double atan2d(double y, double x)
{
    return atan2(y, x) * JME_RAD_TO_DEG;
}

static void calc_ascmc(double ramc, double lat, double eps, double *ascmc);
static int select_cusp_intersection(const double *v, double expected, double eps, double *longitude);

static int normalize_house_system(int house_system)
{
    switch (house_system) {
    case JME_HOUSE_EQUAL: return 'E';
    case JME_HOUSE_VEHLOW_EQUAL: return 'V';
    case JME_HOUSE_WHOLE_SIGN: return 'W';
    case JME_HOUSE_PORPHYRIUS: return 'O';
    case JME_HOUSE_PLACIDUS: return 'P';
    case JME_HOUSE_KOCH: return 'K';
    case JME_HOUSE_KRUSINSKI: return 'U';
    case JME_HOUSE_APC: return 'Y';
    case JME_HOUSE_SUNSHINE: return 'I';
    case JME_HOUSE_REGIOMONTANUS: return 'R';
    case JME_HOUSE_CAMPANUS: return 'C';
    case JME_HOUSE_POLICH_PAGE: return 'T';
    case JME_HOUSE_ALCABITIUS: return 'B';
    case JME_HOUSE_MERIDIAN: return 'M';
    case JME_HOUSE_MORINUS: return 'M';
    case JME_HOUSE_AZIMUTHAL: return 'A';
    case JME_HOUSE_HORIZONTAL: return 'A';
    default: return house_system;
    }
}

static double ecliptic_longitude_from_ra(double ra, double eps)
{
    double r = ra * JME_DEG_TO_RAD;
    double e = eps * JME_DEG_TO_RAD;
    return jme_degree_normalize(atan2d(sin(r), cos(r) * cos(e)));
}

static double right_ascension_from_ecliptic_longitude(double lon, double eps)
{
    double l = lon * JME_DEG_TO_RAD;
    double e = eps * JME_DEG_TO_RAD;
    return jme_degree_normalize(atan2d(sin(l) * cos(e), cos(l)));
}

static int placidus_cusp(double ramc, double lat, double eps, double fraction, double *longitude)
{
    double ra = jme_degree_normalize(ramc + 90.0 * fraction);
    double tan_lat = tan(lat * JME_DEG_TO_RAD);
    double tan_eps = tan(eps * JME_DEG_TO_RAD);
    int i;

    if (longitude == 0 || fraction <= 0.0 || fraction >= 1.0) {
        return JME_ERR;
    }

    for (i = 0; i < 40; i++) {
        double arg = -sin(ra * JME_DEG_TO_RAD) * tan_lat * tan_eps;
        double dsa;
        double next_ra;
        double delta;

        if (arg < -1.0 || arg > 1.0) {
            return JME_ERR;
        }

        dsa = acos(arg) * JME_RAD_TO_DEG;
        next_ra = jme_degree_normalize(ramc + dsa * fraction);
        delta = fabs(jme_degrees_difference_signed(next_ra, ra));
        ra = next_ra;

        if (delta < 1e-12) {
            *longitude = ecliptic_longitude_from_ra(ra, eps);
            return JME_OK;
        }
    }

    *longitude = ecliptic_longitude_from_ra(ra, eps);
    return JME_OK;
}

static double regiomontanus_cusp(double ramc, double lat, double eps, double house_offset)
{
    double ram = ramc * JME_DEG_TO_RAD;
    double ram_p = (ramc + house_offset) * JME_DEG_TO_RAD;
    double e = eps * JME_DEG_TO_RAD;
    double p = lat * JME_DEG_TO_RAD;
    double numerator = sin(ram_p) / cos(e);
    double denominator = cos(ram_p) - tan(e) * tan(p) * (cos(ram) * sin(ram_p) - sin(ram) * cos(ram_p));

    return jme_degree_normalize(atan2d(numerator, denominator));
}

static double topocentric_cusp(double ramc, double lat, double eps, double house_offset, double pole_fraction)
{
    double pole = atan(tan(lat * JME_DEG_TO_RAD) * pole_fraction) * JME_RAD_TO_DEG;
    return regiomontanus_cusp(ramc, pole, eps, house_offset);
}

static double longitude_from_equatorial_vector(const double *v, double eps)
{
    double y_ecl = v[1] * cos(eps * JME_DEG_TO_RAD) + v[2] * sin(eps * JME_DEG_TO_RAD);
    return jme_degree_normalize(atan2d(y_ecl, v[0]));
}

static void ecliptic_longitude_to_equatorial_vector(double lon, double eps, double *v)
{
    double l = lon * JME_DEG_TO_RAD;
    double e = eps * JME_DEG_TO_RAD;
    double ecl[3] = {cos(l), sin(l), 0.0};

    v[0] = ecl[0];
    v[1] = ecl[1] * cos(e);
    v[2] = ecl[1] * sin(e);
}

static void normalize_vector(double *v)
{
    double n = sqrt(v[0] * v[0] + v[1] * v[1] + v[2] * v[2]);
    if (isfinite(n) && n > 0.0) {
        v[0] /= n;
        v[1] /= n;
        v[2] /= n;
    }
}

static void slerp_unit_vector(const double *a, const double *b, double fraction, double *out)
{
    double dot = a[0] * b[0] + a[1] * b[1] + a[2] * b[2];
    double omega;
    double so;
    double ca;
    double cb;
    int i;

    if (dot > 1.0) { dot = 1.0; }
    if (dot < -1.0) { dot = -1.0; }

    omega = acos(dot);
    so = sin(omega);
    if (fabs(so) < 1e-14) {
        for (i = 0; i < 3; i++) {
            out[i] = a[i] + (b[i] - a[i]) * fraction;
        }
        normalize_vector(out);
        return;
    }

    ca = sin((1.0 - fraction) * omega) / so;
    cb = sin(fraction * omega) / so;
    for (i = 0; i < 3; i++) {
        out[i] = ca * a[i] + cb * b[i];
    }
    normalize_vector(out);
}

static int meridian_projection_to_ecliptic(const double *point, double expected, double eps, double *longitude)
{
    double ra;
    double lon1;
    double lon2;

    if (longitude == 0 || point == 0 || (!isfinite(point[0])) || (!isfinite(point[1]))) {
        return JME_ERR;
    }

    if (fabs(point[0]) + fabs(point[1]) <= 1e-14) {
        return JME_ERR;
    }

    ra = atan2d(point[1], point[0]);
    lon1 = ecliptic_longitude_from_ra(ra, eps);
    lon2 = jme_degree_normalize(lon1 + 180.0);
    *longitude = fabs(jme_degrees_difference_signed(lon1, expected)) <= fabs(jme_degrees_difference_signed(lon2, expected)) ? lon1 : lon2;
    return JME_OK;
}

static int select_cusp_intersection(const double *v, double expected, double eps, double *longitude)
{
    double lon1;
    double lon2;

    if (longitude == 0) {
        return JME_ERR;
    }

    lon1 = longitude_from_equatorial_vector(v, eps);
    lon2 = jme_degree_normalize(lon1 + 180.0);
    *longitude = fabs(jme_degrees_difference_signed(lon1, expected)) <= fabs(jme_degrees_difference_signed(lon2, expected)) ? lon1 : lon2;
    return JME_OK;
}

static int campanus_cusp(double ramc, double lat, double eps, double house_offset, double expected, double *longitude)
{
    double theta = (house_offset - 90.0) * JME_DEG_TO_RAD;
    double r = ramc * JME_DEG_TO_RAD;
    double p = lat * JME_DEG_TO_RAD;
    double east[3] = {-sin(r), cos(r), 0.0};
    double zenith[3] = {cos(p) * cos(r), cos(p) * sin(r), sin(p)};
    double north[3] = {
        zenith[1] * east[2] - zenith[2] * east[1],
        zenith[2] * east[0] - zenith[0] * east[2],
        zenith[0] * east[1] - zenith[1] * east[0]
    };
    double pv[3];
    double house_normal[3];
    double ecl_normal[3] = {0.0, -sin(eps * JME_DEG_TO_RAD), cos(eps * JME_DEG_TO_RAD)};
    double inter[3];
    double norm;
    int i;

    for (i = 0; i < 3; i++) {
        pv[i] = east[i] * cos(theta) + zenith[i] * sin(theta);
    }

    house_normal[0] = north[1] * pv[2] - north[2] * pv[1];
    house_normal[1] = north[2] * pv[0] - north[0] * pv[2];
    house_normal[2] = north[0] * pv[1] - north[1] * pv[0];

    inter[0] = house_normal[1] * ecl_normal[2] - house_normal[2] * ecl_normal[1];
    inter[1] = house_normal[2] * ecl_normal[0] - house_normal[0] * ecl_normal[2];
    inter[2] = house_normal[0] * ecl_normal[1] - house_normal[1] * ecl_normal[0];
    norm = sqrt(inter[0] * inter[0] + inter[1] * inter[1] + inter[2] * inter[2]);

    if (!isfinite(norm) || norm <= 0.0) {
        return JME_ERR;
    }

    for (i = 0; i < 3; i++) {
        inter[i] /= norm;
    }

    return select_cusp_intersection(inter, expected, eps, longitude);
}

static int horizontal_cusp(double ramc, double lat, double eps, double house_offset, double expected, double *longitude)
{
    double theta = (house_offset - 90.0) * JME_DEG_TO_RAD;
    double r = ramc * JME_DEG_TO_RAD;
    double p = lat * JME_DEG_TO_RAD;
    double east[3] = {-sin(r), cos(r), 0.0};
    double zenith[3] = {cos(p) * cos(r), cos(p) * sin(r), sin(p)};
    double north[3] = {
        zenith[1] * east[2] - zenith[2] * east[1],
        zenith[2] * east[0] - zenith[0] * east[2],
        zenith[0] * east[1] - zenith[1] * east[0]
    };
    double horizon_point[3];
    double vertical_normal[3];
    double ecl_normal[3] = {0.0, -sin(eps * JME_DEG_TO_RAD), cos(eps * JME_DEG_TO_RAD)};
    double inter[3];
    double norm;
    int i;

    for (i = 0; i < 3; i++) {
        horizon_point[i] = east[i] * cos(theta) + north[i] * sin(theta);
    }

    vertical_normal[0] = zenith[1] * horizon_point[2] - zenith[2] * horizon_point[1];
    vertical_normal[1] = zenith[2] * horizon_point[0] - zenith[0] * horizon_point[2];
    vertical_normal[2] = zenith[0] * horizon_point[1] - zenith[1] * horizon_point[0];

    inter[0] = vertical_normal[1] * ecl_normal[2] - vertical_normal[2] * ecl_normal[1];
    inter[1] = vertical_normal[2] * ecl_normal[0] - vertical_normal[0] * ecl_normal[2];
    inter[2] = vertical_normal[0] * ecl_normal[1] - vertical_normal[1] * ecl_normal[0];
    norm = sqrt(inter[0] * inter[0] + inter[1] * inter[1] + inter[2] * inter[2]);

    if (!isfinite(norm) || norm <= 0.0) {
        return JME_ERR;
    }

    for (i = 0; i < 3; i++) {
        inter[i] /= norm;
    }

    return select_cusp_intersection(inter, expected, eps, longitude);
}

static double ascendant_from_ramc(double ramc, double lat, double eps)
{
    double local_ascmc[10];
    calc_ascmc(ramc, lat, eps, local_ascmc);
    return local_ascmc[0];
}

static int previous_rising_ramc_for_longitude(double ramc, double lat, double eps, double longitude, double *rise_ramc)
{
    double end = ramc;
    double step = 1.0;
    double t0 = end;
    double f0 = jme_degrees_difference_signed(ascendant_from_ramc(t0, lat, eps), longitude);
    int i;

    if (rise_ramc == 0) {
        return JME_ERR;
    }

    for (i = 1; i <= 360; i++) {
        double t1 = end - step * i;
        double f1 = jme_degrees_difference_signed(ascendant_from_ramc(t1, lat, eps), longitude);

        if (f0 == 0.0 || f0 * f1 <= 0.0) {
            int j;
            double lo = t1;
            double hi = t0;
            double flo = f1;

            for (j = 0; j < 60; j++) {
                double mid = (lo + hi) / 2.0;
                double fm = jme_degrees_difference_signed(ascendant_from_ramc(mid, lat, eps), longitude);
                if (fabs(fm) < 1e-12 || fabs(hi - lo) < 1e-12) {
                    *rise_ramc = jme_degree_normalize(mid);
                    return JME_OK;
                }
                if (flo * fm <= 0.0) {
                    hi = mid;
                } else {
                    lo = mid;
                    flo = fm;
                }
            }

            *rise_ramc = jme_degree_normalize((lo + hi) / 2.0);
            return JME_OK;
        }

        t0 = t1;
        f0 = f1;
    }

    return JME_ERR;
}

static int koch_cusps(double asc, double mc, double ramc, double lat, double eps, double *cusps)
{
    double mc_rise;
    double ic_rise;
    double mc_elapsed;
    double ic_elapsed;
    double r12;
    double r11;
    double r3;
    double r2;

    if (previous_rising_ramc_for_longitude(ramc, lat, eps, mc, &mc_rise) != JME_OK
        || previous_rising_ramc_for_longitude(ramc, lat, eps, jme_degree_normalize(mc + 180.0), &ic_rise) != JME_OK) {
        return JME_ERR;
    }

    mc_elapsed = jme_degrees_difference(ramc, mc_rise);
    ic_elapsed = jme_degrees_difference(ramc, ic_rise);
    if (!isfinite(mc_elapsed) || !isfinite(ic_elapsed) || mc_elapsed <= 0.0 || ic_elapsed <= 0.0) {
        return JME_ERR;
    }

    r12 = mc_rise + mc_elapsed / 3.0;
    r11 = mc_rise + 2.0 * mc_elapsed / 3.0;
    r3 = ic_rise + ic_elapsed / 3.0;
    r2 = ic_rise + 2.0 * ic_elapsed / 3.0;

    cusps[1] = asc;
    cusps[2] = ascendant_from_ramc(r2, lat, eps);
    cusps[3] = ascendant_from_ramc(r3, lat, eps);
    cusps[4] = jme_degree_normalize(mc + 180.0);
    cusps[7] = jme_degree_normalize(asc + 180.0);
    cusps[8] = jme_degree_normalize(cusps[2] + 180.0);
    cusps[9] = jme_degree_normalize(cusps[3] + 180.0);
    cusps[10] = mc;
    cusps[11] = ascendant_from_ramc(r11, lat, eps);
    cusps[12] = ascendant_from_ramc(r12, lat, eps);
    cusps[5] = jme_degree_normalize(cusps[11] + 180.0);
    cusps[6] = jme_degree_normalize(cusps[12] + 180.0);

    return JME_OK;
}

static int krusinski_cusps(double asc, double mc, double ramc, double lat, double eps, double *cusps)
{
    double r = ramc * JME_DEG_TO_RAD;
    double p = lat * JME_DEG_TO_RAD;
    double zenith[3] = {cos(p) * cos(r), cos(p) * sin(r), sin(p)};
    double nadir[3] = {-zenith[0], -zenith[1], -zenith[2]};
    double asc_vec[3];
    double p12[3], p11[3], p2[3], p3[3];

    ecliptic_longitude_to_equatorial_vector(asc, eps, asc_vec);
    normalize_vector(zenith);
    normalize_vector(nadir);
    normalize_vector(asc_vec);

    slerp_unit_vector(asc_vec, zenith, 1.0 / 3.0, p12);
    slerp_unit_vector(asc_vec, zenith, 2.0 / 3.0, p11);
    slerp_unit_vector(asc_vec, nadir, 1.0 / 3.0, p2);
    slerp_unit_vector(asc_vec, nadir, 2.0 / 3.0, p3);

    cusps[1] = asc;
    if (meridian_projection_to_ecliptic(p2, jme_degree_normalize(asc + 30.0), eps, &cusps[2]) != JME_OK
        || meridian_projection_to_ecliptic(p3, jme_degree_normalize(asc + 60.0), eps, &cusps[3]) != JME_OK
        || meridian_projection_to_ecliptic(p11, jme_degree_normalize(mc + 30.0), eps, &cusps[11]) != JME_OK
        || meridian_projection_to_ecliptic(p12, jme_degree_normalize(mc + 60.0), eps, &cusps[12]) != JME_OK) {
        return JME_ERR;
    }

    cusps[4] = jme_degree_normalize(mc + 180.0);
    cusps[5] = jme_degree_normalize(cusps[11] + 180.0);
    cusps[6] = jme_degree_normalize(cusps[12] + 180.0);
    cusps[7] = jme_degree_normalize(asc + 180.0);
    cusps[8] = jme_degree_normalize(cusps[2] + 180.0);
    cusps[9] = jme_degree_normalize(cusps[3] + 180.0);
    cusps[10] = mc;

    return JME_OK;
}

static int apc_cusp(double ramc, double lat, double eps, int cusp, double *longitude)
{
    double ph = lat * JME_DEG_TO_RAD;
    double e = eps * JME_DEG_TO_RAD;
    double az = ramc * JME_DEG_TO_RAD;
    double tan_ph = tan(ph);
    double tan_e = tan(e);
    double kv_den;
    double kv;
    double dasc;
    double a;
    double x;
    double y;

    if (longitude == 0 || cusp < 1 || cusp > 12 || fabs(tan_ph) < 1e-14) {
        return JME_ERR;
    }

    kv_den = 1.0 + tan_ph * tan_e * sin(az);
    if (fabs(kv_den) < 1e-14) {
        return JME_ERR;
    }

    /*
     * Ascendant Parallel Circle construction: divide the APC above and below
     * the horizon, then project the oblique-ascension planes to the ecliptic.
     */
    kv = atan((tan_ph * tan_e * cos(az)) / kv_den);
    dasc = atan(sin(kv) / tan_ph);

    if (cusp < 8) {
        a = kv + az + JME_PI / 2.0 + (double)(cusp - 1) * (JME_PI / 2.0 - kv) / 3.0;
    } else {
        a = kv + az + JME_PI / 2.0 + (double)(cusp - 13) * (JME_PI / 2.0 + kv) / 3.0;
    }

    y = tan(dasc) * tan_ph * sin(az) + sin(a);
    x = cos(e) * (tan(dasc) * tan_ph * cos(az) + cos(a)) + sin(e) * tan_ph * sin(az - a);
    if (!isfinite(x) || !isfinite(y)) {
        return JME_ERR;
    }

    *longitude = jme_degree_normalize(atan2d(y, x));
    return JME_OK;
}

static int apc_cusps(double asc, double mc, double ramc, double lat, double eps, double *cusps)
{
    int i;

    for (i = 1; i <= 12; i++) {
        if (apc_cusp(ramc, lat, eps, i, &cusps[i]) != JME_OK) {
            return JME_ERR;
        }
    }

    cusps[1] = asc;
    cusps[4] = jme_degree_normalize(mc + 180.0);
    cusps[7] = jme_degree_normalize(asc + 180.0);
    cusps[10] = mc;
    return JME_OK;
}

static int north_point_projection_cusp(double ramc, double lat, double eps, const double *point, double expected, double *longitude)
{
    double r = ramc * JME_DEG_TO_RAD;
    double p = lat * JME_DEG_TO_RAD;
    double north[3] = {-sin(p) * cos(r), -sin(p) * sin(r), cos(p)};
    double circle_normal[3];
    double ecl_normal[3] = {0.0, -sin(eps * JME_DEG_TO_RAD), cos(eps * JME_DEG_TO_RAD)};
    double inter[3];
    double norm;
    int i;

    if (point == 0 || longitude == 0) {
        return JME_ERR;
    }

    circle_normal[0] = north[1] * point[2] - north[2] * point[1];
    circle_normal[1] = north[2] * point[0] - north[0] * point[2];
    circle_normal[2] = north[0] * point[1] - north[1] * point[0];

    inter[0] = circle_normal[1] * ecl_normal[2] - circle_normal[2] * ecl_normal[1];
    inter[1] = circle_normal[2] * ecl_normal[0] - circle_normal[0] * ecl_normal[2];
    inter[2] = circle_normal[0] * ecl_normal[1] - circle_normal[1] * ecl_normal[0];
    norm = sqrt(inter[0] * inter[0] + inter[1] * inter[1] + inter[2] * inter[2]);

    if (!isfinite(norm) || norm <= 0.0) {
        return JME_ERR;
    }

    for (i = 0; i < 3; i++) {
        inter[i] /= norm;
    }

    return select_cusp_intersection(inter, expected, eps, longitude);
}

static void equatorial_point_from_hour_angle(double ramc, double decl, double hour_angle, double *point)
{
    double ra = (ramc - hour_angle) * JME_DEG_TO_RAD;
    double d = decl * JME_DEG_TO_RAD;

    point[0] = cos(d) * cos(ra);
    point[1] = cos(d) * sin(ra);
    point[2] = sin(d);
}

static int sunshine_cusps(double asc, double mc, double ramc, double lat, double eps, double sun_decl, double *cusps)
{
    double tan_lat = tan(lat * JME_DEG_TO_RAD);
    double tan_decl = tan(sun_decl * JME_DEG_TO_RAD);
    double cos_h0 = -tan_lat * tan_decl;
    double h0;
    double noct_step;
    double h[13];
    int i;

    if (cusps == 0 || !isfinite(sun_decl) || cos_h0 < -1.0 || cos_h0 > 1.0) {
        return JME_ERR;
    }

    h0 = acos(cos_h0) * JME_RAD_TO_DEG;
    noct_step = (360.0 - 2.0 * h0) / 6.0;

    h[1] = -h0;
    h[12] = -2.0 * h0 / 3.0;
    h[11] = -h0 / 3.0;
    h[10] = 0.0;
    h[9] = h0 / 3.0;
    h[8] = 2.0 * h0 / 3.0;
    h[7] = h0;
    h[6] = h0 + noct_step;
    h[5] = h0 + 2.0 * noct_step;
    h[4] = 180.0;
    h[3] = h0 + 4.0 * noct_step;
    h[2] = h0 + 5.0 * noct_step;

    for (i = 1; i <= 12; i++) {
        double point[3];
        double expected = jme_degree_normalize(asc + (double)(i - 1) * 30.0);
        equatorial_point_from_hour_angle(ramc, sun_decl, h[i], point);
        if (north_point_projection_cusp(ramc, lat, eps, point, expected, &cusps[i]) != JME_OK) {
            return JME_ERR;
        }
    }

    cusps[1] = asc;
    cusps[4] = jme_degree_normalize(mc + 180.0);
    cusps[7] = jme_degree_normalize(asc + 180.0);
    cusps[10] = mc;
    return JME_OK;
}

static void calc_ascmc(double ramc, double lat, double eps, double *ascmc)
{
    double r = ramc * JME_DEG_TO_RAD;
    double l = lat * JME_DEG_TO_RAD;
    double e = eps * JME_DEG_TO_RAD;
    double mc, asc, vertex, ep;

    /* MC */
    mc = atan2d(sin(r), cos(r) * cos(e));
    ascmc[1] = jme_degree_normalize(mc);

    /* Ascendant */
    asc = atan2d(cos(r), -sin(r) * cos(e) - tan(l) * sin(e));
    ascmc[0] = jme_degree_normalize(asc);

    /* IC */
    ascmc[2] = jme_degree_normalize(mc + 180.0);

    /* Descendant */
    ascmc[3] = jme_degree_normalize(asc + 180.0);

    /* Vertex */
    vertex = atan2d(-cos(r + 90.0 * JME_DEG_TO_RAD), sin(r + 90.0 * JME_DEG_TO_RAD) * cos(e) - tan(l) * sin(e));
    ascmc[4] = jme_degree_normalize(vertex);

    /* Equatorial Ascendant */
    ep = atan2d(cos(r), -sin(r) * cos(e));
    ascmc[5] = jme_degree_normalize(ep);

    /* ARMC and true obliquity, useful for compatibility callers. */
    ascmc[6] = jme_degree_normalize(ramc);
    ascmc[7] = eps;
    ascmc[8] = 0.0;
    ascmc[9] = 0.0;
}

static int fill_house_cusps_from_angles(double asc, double mc, double ramc, double lat, double eps, int house_system, double *cusps)
{
    int i;
    int system = normalize_house_system(house_system);

    if (cusps == 0) {
        return JME_OK;
    }

    for (i = 0; i <= 12; i++) {
        cusps[i] = 0.0;
    }

    switch (system) {
    case 'E':
        for (i = 1; i <= 12; i++) {
            cusps[i] = jme_degree_normalize(asc + (i - 1) * 30.0);
        }
        return JME_OK;
    case 'V':
        for (i = 1; i <= 12; i++) {
            cusps[i] = jme_degree_normalize(asc - 15.0 + (i - 1) * 30.0);
        }
        return JME_OK;
    case 'W':
        {
            double sign_start = floor(asc / 30.0) * 30.0;
            for (i = 1; i <= 12; i++) {
                cusps[i] = jme_degree_normalize(sign_start + (i - 1) * 30.0);
            }
        }
        return JME_OK;
    case 'O':
        {
            double ic = jme_degree_normalize(mc + 180.0);
            double desc = jme_degree_normalize(asc + 180.0);
            double q1 = jme_degrees_difference(ic, asc) / 3.0;
            double q2 = jme_degrees_difference(desc, ic) / 3.0;
            double q3 = jme_degrees_difference(mc, desc) / 3.0;
            double q4 = jme_degrees_difference(asc, mc) / 3.0;

            cusps[1] = asc;
            cusps[2] = jme_degree_normalize(asc + q1);
            cusps[3] = jme_degree_normalize(asc + 2.0 * q1);
            cusps[4] = ic;
            cusps[5] = jme_degree_normalize(ic + q2);
            cusps[6] = jme_degree_normalize(ic + 2.0 * q2);
            cusps[7] = desc;
            cusps[8] = jme_degree_normalize(desc + q3);
            cusps[9] = jme_degree_normalize(desc + 2.0 * q3);
            cusps[10] = mc;
            cusps[11] = jme_degree_normalize(mc + q4);
            cusps[12] = jme_degree_normalize(mc + 2.0 * q4);
        }
        return JME_OK;
    case 'M':
        /*
         * Morinus divides the celestial equator into equal 30-degree right
         * ascension arcs and projects those great circles to the ecliptic.
         */
        cusps[1] = ecliptic_longitude_from_ra(ramc + 90.0, eps);
        cusps[2] = ecliptic_longitude_from_ra(ramc + 120.0, eps);
        cusps[3] = ecliptic_longitude_from_ra(ramc + 150.0, eps);
        cusps[4] = ecliptic_longitude_from_ra(ramc + 180.0, eps);
        cusps[5] = ecliptic_longitude_from_ra(ramc + 210.0, eps);
        cusps[6] = ecliptic_longitude_from_ra(ramc + 240.0, eps);
        cusps[7] = ecliptic_longitude_from_ra(ramc + 270.0, eps);
        cusps[8] = ecliptic_longitude_from_ra(ramc + 300.0, eps);
        cusps[9] = ecliptic_longitude_from_ra(ramc + 330.0, eps);
        cusps[10] = mc;
        cusps[11] = ecliptic_longitude_from_ra(ramc + 30.0, eps);
        cusps[12] = ecliptic_longitude_from_ra(ramc + 60.0, eps);
        return JME_OK;
    case 'P':
        {
            double c11, c12, c2, c3;

            if (placidus_cusp(ramc, lat, eps, 1.0 / 3.0, &c11) != JME_OK
                || placidus_cusp(ramc, lat, eps, 2.0 / 3.0, &c12) != JME_OK
                || placidus_cusp(ramc + 180.0, -lat, eps, 2.0 / 3.0, &c2) != JME_OK
                || placidus_cusp(ramc + 180.0, -lat, eps, 1.0 / 3.0, &c3) != JME_OK) {
                return JME_ERR;
            }

            cusps[1] = asc;
            cusps[2] = c2;
            cusps[3] = c3;
            cusps[4] = jme_degree_normalize(mc + 180.0);
            cusps[5] = jme_degree_normalize(c11 + 180.0);
            cusps[6] = jme_degree_normalize(c12 + 180.0);
            cusps[7] = jme_degree_normalize(asc + 180.0);
            cusps[8] = jme_degree_normalize(c2 + 180.0);
            cusps[9] = jme_degree_normalize(c3 + 180.0);
            cusps[10] = mc;
            cusps[11] = c11;
            cusps[12] = c12;
        }
        return JME_OK;
    case 'R':
        cusps[1] = asc;
        cusps[2] = regiomontanus_cusp(ramc, lat, eps, 120.0);
        cusps[3] = regiomontanus_cusp(ramc, lat, eps, 150.0);
        cusps[4] = jme_degree_normalize(mc + 180.0);
        cusps[5] = regiomontanus_cusp(ramc, lat, eps, 210.0);
        cusps[6] = regiomontanus_cusp(ramc, lat, eps, 240.0);
        cusps[7] = jme_degree_normalize(asc + 180.0);
        cusps[8] = regiomontanus_cusp(ramc, lat, eps, 300.0);
        cusps[9] = regiomontanus_cusp(ramc, lat, eps, 330.0);
        cusps[10] = mc;
        cusps[11] = regiomontanus_cusp(ramc, lat, eps, 30.0);
        cusps[12] = regiomontanus_cusp(ramc, lat, eps, 60.0);
        return JME_OK;
    case 'C':
        cusps[1] = asc;
        if (campanus_cusp(ramc, lat, eps, 120.0, jme_degree_normalize(asc + 30.0), &cusps[2]) != JME_OK
            || campanus_cusp(ramc, lat, eps, 150.0, jme_degree_normalize(asc + 60.0), &cusps[3]) != JME_OK
            || campanus_cusp(ramc, lat, eps, 210.0, jme_degree_normalize(mc + 210.0), &cusps[5]) != JME_OK
            || campanus_cusp(ramc, lat, eps, 240.0, jme_degree_normalize(mc + 240.0), &cusps[6]) != JME_OK
            || campanus_cusp(ramc, lat, eps, 300.0, jme_degree_normalize(asc + 210.0), &cusps[8]) != JME_OK
            || campanus_cusp(ramc, lat, eps, 330.0, jme_degree_normalize(asc + 240.0), &cusps[9]) != JME_OK
            || campanus_cusp(ramc, lat, eps, 30.0, jme_degree_normalize(mc + 30.0), &cusps[11]) != JME_OK
            || campanus_cusp(ramc, lat, eps, 60.0, jme_degree_normalize(mc + 60.0), &cusps[12]) != JME_OK) {
            return JME_ERR;
        }
        cusps[4] = jme_degree_normalize(mc + 180.0);
        cusps[7] = jme_degree_normalize(asc + 180.0);
        cusps[10] = mc;
        return JME_OK;
    case 'B':
        {
            double asc_ra = right_ascension_from_ecliptic_longitude(asc, eps);
            double desc_ra = jme_degree_normalize(asc_ra + 180.0);
            double ic_ra = jme_degree_normalize(ramc + 180.0);
            double q4 = jme_degrees_difference(asc_ra, ramc) / 3.0;
            double q1 = jme_degrees_difference(ic_ra, asc_ra) / 3.0;
            double q2 = jme_degrees_difference(desc_ra, ic_ra) / 3.0;
            double q3 = jme_degrees_difference(ramc, desc_ra) / 3.0;

            cusps[1] = asc;
            cusps[2] = ecliptic_longitude_from_ra(asc_ra + q1, eps);
            cusps[3] = ecliptic_longitude_from_ra(asc_ra + 2.0 * q1, eps);
            cusps[4] = jme_degree_normalize(mc + 180.0);
            cusps[5] = ecliptic_longitude_from_ra(ic_ra + q2, eps);
            cusps[6] = ecliptic_longitude_from_ra(ic_ra + 2.0 * q2, eps);
            cusps[7] = jme_degree_normalize(asc + 180.0);
            cusps[8] = ecliptic_longitude_from_ra(desc_ra + q3, eps);
            cusps[9] = ecliptic_longitude_from_ra(desc_ra + 2.0 * q3, eps);
            cusps[10] = mc;
            cusps[11] = ecliptic_longitude_from_ra(ramc + q4, eps);
            cusps[12] = ecliptic_longitude_from_ra(ramc + 2.0 * q4, eps);
        }
        return JME_OK;
    case 'T':
        cusps[1] = asc;
        cusps[2] = topocentric_cusp(ramc, lat, eps, 120.0, 2.0 / 3.0);
        cusps[3] = topocentric_cusp(ramc, lat, eps, 150.0, 1.0 / 3.0);
        cusps[4] = jme_degree_normalize(mc + 180.0);
        cusps[5] = jme_degree_normalize(topocentric_cusp(ramc, lat, eps, 30.0, 1.0 / 3.0) + 180.0);
        cusps[6] = jme_degree_normalize(topocentric_cusp(ramc, lat, eps, 60.0, 2.0 / 3.0) + 180.0);
        cusps[7] = jme_degree_normalize(asc + 180.0);
        cusps[8] = jme_degree_normalize(cusps[2] + 180.0);
        cusps[9] = jme_degree_normalize(cusps[3] + 180.0);
        cusps[10] = mc;
        cusps[11] = topocentric_cusp(ramc, lat, eps, 30.0, 1.0 / 3.0);
        cusps[12] = topocentric_cusp(ramc, lat, eps, 60.0, 2.0 / 3.0);
        return JME_OK;
    case 'A':
        cusps[1] = asc;
        if (horizontal_cusp(ramc, lat, eps, 120.0, jme_degree_normalize(asc + 30.0), &cusps[2]) != JME_OK
            || horizontal_cusp(ramc, lat, eps, 150.0, jme_degree_normalize(asc + 60.0), &cusps[3]) != JME_OK
            || horizontal_cusp(ramc, lat, eps, 210.0, jme_degree_normalize(mc + 210.0), &cusps[5]) != JME_OK
            || horizontal_cusp(ramc, lat, eps, 240.0, jme_degree_normalize(mc + 240.0), &cusps[6]) != JME_OK
            || horizontal_cusp(ramc, lat, eps, 300.0, jme_degree_normalize(asc + 210.0), &cusps[8]) != JME_OK
            || horizontal_cusp(ramc, lat, eps, 330.0, jme_degree_normalize(asc + 240.0), &cusps[9]) != JME_OK
            || horizontal_cusp(ramc, lat, eps, 30.0, jme_degree_normalize(mc + 30.0), &cusps[11]) != JME_OK
            || horizontal_cusp(ramc, lat, eps, 60.0, jme_degree_normalize(mc + 60.0), &cusps[12]) != JME_OK) {
            return JME_ERR;
        }
        cusps[4] = jme_degree_normalize(mc + 180.0);
        cusps[7] = jme_degree_normalize(asc + 180.0);
        cusps[10] = mc;
        return JME_OK;
    case 'K':
        return koch_cusps(asc, mc, ramc, lat, eps, cusps);
    case 'U':
        return krusinski_cusps(asc, mc, ramc, lat, eps, cusps);
    case 'Y':
        return apc_cusps(asc, mc, ramc, lat, eps, cusps);
    default:
        return JME_ERR;
    }
}

int jme_houses(double jd_ut, double geo_lat, double geo_lon, int house_system, double *cusps, double *ascmc)
{
    return jme_houses_ex(jd_ut, 0, geo_lat, geo_lon, house_system, cusps, ascmc);
}

int jme_houses_ex(double jd_ut, int flags, double geo_lat, double geo_lon, int house_system, double *cusps, double *ascmc)
{
    double eps, dpsi, deps;
    double gst, ramc;
    double local_ascmc[10];
    int system = normalize_house_system(house_system);
    int i;

    (void)flags;

    /* 1. Need Obliquity and Nutation for RAMC and coordinate transform */
    jme_get_obliquity(jd_ut, JME_MODEL_OBL_IAU_1980, &eps, 0);
    jme_get_nutation(jd_ut, JME_MODEL_NUT_IAU_1980, &dpsi, &deps, 0);
    eps += deps; /* True obliquity */

    gst = jme_sidereal_time0(jd_ut, eps - deps, dpsi);
    ramc = jme_degree_normalize(gst * 15.0 + geo_lon);

    calc_ascmc(ramc, geo_lat, eps, local_ascmc);
    if (ascmc != 0) {
        for (i = 0; i < 10; i++) { ascmc[i] = local_ascmc[i]; }
    }

    if (system == 'I') {
        double sun_equ[6];
        if (jme_calc_ut(jd_ut, JME_BODY_SUN, JME_CALC_EQUATORIAL | JME_CALC_TRUE_POSITION, sun_equ, 0) != JME_OK) {
            return JME_ERR;
        }
        if (sunshine_cusps(local_ascmc[0], local_ascmc[1], ramc, geo_lat, eps, sun_equ[1], cusps) != JME_OK) {
            return JME_ERR;
        }
        return JME_OK;
    }

    if (fill_house_cusps_from_angles(local_ascmc[0], local_ascmc[1], ramc, geo_lat, eps, house_system, cusps) != JME_OK) {
        return JME_ERR;
    }

    return JME_OK;
}

double jme_house_pos(double armc, double geo_lat, double eps, int house_system, double *xpin, char *error)
{
    double ascmc[10];
    double cusps[13];
    double lon;
    int i;

    if (xpin == 0) {
        jme_set_error(error, "House position input is required");
        return 0.0;
    }

    if (jme_houses_armc(armc, geo_lat, eps, house_system, cusps, ascmc) != JME_OK) {
        jme_set_error(error, "House system is not available");
        return 0.0;
    }

    lon = jme_degree_normalize(xpin[0]);
    for (i = 1; i <= 12; i++) {
        int next = i == 12 ? 1 : i + 1;
        double width = jme_degrees_difference(cusps[next], cusps[i]);
        double delta = jme_degrees_difference(lon, cusps[i]);
        if (delta < width || (i == 12 && delta <= width)) {
            return (double)i + (width > 0.0 ? delta / width : 0.0);
        }
    }

    return 0.0;
}

int jme_houses_ex2(double jd_ut, int flags, double geo_lat, double geo_lon, int house_system, double *cusps, double *ascmc, double *cusps_speed, double *ascmc_speed)
{
    double step = 1.0e-3;
    double prev_cusps[13];
    double next_cusps[13];
    double prev_ascmc[10];
    double next_ascmc[10];
    int i;
    int rc = jme_houses_ex(jd_ut, flags, geo_lat, geo_lon, house_system, cusps, ascmc);

    if (rc != JME_OK) {
        return rc;
    }

    if (cusps_speed != 0) {
        for (i = 0; i < 13; i++) { cusps_speed[i] = 0.0; }
        if (jme_houses_ex(jd_ut - step, flags, geo_lat, geo_lon, house_system, prev_cusps, prev_ascmc) == JME_OK
            && jme_houses_ex(jd_ut + step, flags, geo_lat, geo_lon, house_system, next_cusps, next_ascmc) == JME_OK) {
            for (i = 1; i <= 12; i++) {
                cusps_speed[i] = jme_degrees_difference_signed(next_cusps[i], prev_cusps[i]) / (2.0 * step);
            }
        }
    }

    if (ascmc_speed != 0) {
        for (i = 0; i < 10; i++) { ascmc_speed[i] = 0.0; }
        if (jme_houses_ex(jd_ut - step, flags, geo_lat, geo_lon, house_system, prev_cusps, prev_ascmc) == JME_OK
            && jme_houses_ex(jd_ut + step, flags, geo_lat, geo_lon, house_system, next_cusps, next_ascmc) == JME_OK) {
            for (i = 0; i < 10; i++) {
                if (i == 7) {
                    ascmc_speed[i] = (next_ascmc[i] - prev_ascmc[i]) / (2.0 * step);
                } else {
                    ascmc_speed[i] = jme_degrees_difference_signed(next_ascmc[i], prev_ascmc[i]) / (2.0 * step);
                }
            }
        }
    }

    return JME_OK;
}

int jme_houses_armc(double armc, double geo_lat, double eps, int house_system, double *cusps, double *ascmc)
{
    double local_ascmc[10];
    int i;

    calc_ascmc(armc, geo_lat, eps, local_ascmc);
    if (ascmc != 0) {
        for (i = 0; i < 10; i++) {
            ascmc[i] = local_ascmc[i];
        }
    }

    return fill_house_cusps_from_angles(local_ascmc[0], local_ascmc[1], armc, geo_lat, eps, house_system, cusps);
}

int jme_houses_armc_ex2(double armc, double geo_lat, double eps, int house_system, double *cusps, double *ascmc, double *cusps_speed, double *ascmc_speed)
{
    int i;
    double step = 1.0e-3;
    double prev_cusps[13];
    double next_cusps[13];
    double prev_ascmc[10];
    double next_ascmc[10];
    int rc = jme_houses_armc(armc, geo_lat, eps, house_system, cusps, ascmc);

    if (rc != JME_OK) {
        return rc;
    }

    if (cusps_speed != 0) {
        for (i = 0; i < 13; i++) { cusps_speed[i] = 0.0; }
        if (jme_houses_armc(armc - step, geo_lat, eps, house_system, prev_cusps, prev_ascmc) == JME_OK
            && jme_houses_armc(armc + step, geo_lat, eps, house_system, next_cusps, next_ascmc) == JME_OK) {
            for (i = 1; i <= 12; i++) {
                cusps_speed[i] = jme_degrees_difference_signed(next_cusps[i], prev_cusps[i]) / (2.0 * step);
            }
        }
    }
    if (ascmc_speed != 0) {
        for (i = 0; i < 10; i++) { ascmc_speed[i] = 0.0; }
        if (jme_houses_armc(armc - step, geo_lat, eps, house_system, prev_cusps, prev_ascmc) == JME_OK
            && jme_houses_armc(armc + step, geo_lat, eps, house_system, next_cusps, next_ascmc) == JME_OK) {
            for (i = 0; i < 10; i++) {
                if (i == 7) {
                    ascmc_speed[i] = 0.0;
                } else {
                    ascmc_speed[i] = jme_degrees_difference_signed(next_ascmc[i], prev_ascmc[i]) / (2.0 * step);
                }
            }
        }
    }
    return JME_OK;
}

int jme_gauquelin_sector(double jd_ut, int body, const char *starname, int flags, int imeth, double *geopos, double atpress, double attemp, double *dgsect, char *error)
{
    double prev_rise = 0.0;
    double next_rise = 0.0;
    double prev_set = 0.0;
    double next_set = 0.0;
    double prev_mc = 0.0;
    double next_mc = 0.0;
    double prev_ic = 0.0;
    double next_ic = 0.0;
    double day_start = floor(jd_ut - 0.5) + 0.5;
    double span_start = 0.0;
    double span_end = 0.0;
    double fraction;
    int rc_rise_prev;
    int rc_rise_next;
    int rc_set_prev;
    int rc_set_next;
    int rc_mc_prev;
    int rc_mc_next;
    int rc_ic_prev;
    int rc_ic_next;
    int sector_offset = 0;

    if (dgsect != 0) { *dgsect = 0.0; }
    if (dgsect == 0 || geopos == 0) {
        jme_set_error(error, "Gauquelin sector output arguments are required");
        return JME_ERR;
    }

    rc_rise_prev = jme_rise_trans(day_start - 1.0, body, starname, flags, JME_RISE_RISE, geopos, atpress, attemp, &prev_rise, error);
    rc_rise_next = jme_rise_trans(day_start, body, starname, flags, JME_RISE_RISE, geopos, atpress, attemp, &next_rise, error);
    rc_set_prev = jme_rise_trans(day_start - 1.0, body, starname, flags, JME_RISE_SET, geopos, atpress, attemp, &prev_set, error);
    rc_set_next = jme_rise_trans(day_start, body, starname, flags, JME_RISE_SET, geopos, atpress, attemp, &next_set, error);
    rc_mc_prev = jme_rise_trans(day_start - 1.0, body, starname, flags, JME_RISE_MERIDIAN_TRANSIT, geopos, atpress, attemp, &prev_mc, error);
    rc_mc_next = jme_rise_trans(day_start, body, starname, flags, JME_RISE_MERIDIAN_TRANSIT, geopos, atpress, attemp, &next_mc, error);
    rc_ic_prev = jme_rise_trans(day_start - 1.0, body, starname, flags, JME_RISE_ANTI_MERIDIAN_TRANSIT, geopos, atpress, attemp, &prev_ic, error);
    rc_ic_next = jme_rise_trans(day_start, body, starname, flags, JME_RISE_ANTI_MERIDIAN_TRANSIT, geopos, atpress, attemp, &next_ic, error);

    if (rc_rise_prev != JME_OK || rc_rise_next != JME_OK
        || rc_set_prev != JME_OK || rc_set_next != JME_OK
        || rc_mc_prev != JME_OK || rc_mc_next != JME_OK
        || rc_ic_prev != JME_OK || rc_ic_next != JME_OK) {
        jme_set_error(error, "Gauquelin sector requires rise, set, upper transit, and lower transit events around the supplied time");
        return JME_ERR;
    }

    if (prev_rise <= jd_ut && jd_ut < prev_mc) {
        span_start = prev_rise;
        span_end = prev_mc;
        sector_offset = 0;
    } else if (prev_mc <= jd_ut && jd_ut < prev_set) {
        span_start = prev_mc;
        span_end = prev_set;
        sector_offset = 9;
    } else if (prev_set <= jd_ut && jd_ut < prev_ic) {
        span_start = prev_set;
        span_end = prev_ic;
        sector_offset = 18;
    } else if (prev_ic <= jd_ut && jd_ut < next_rise) {
        span_start = prev_ic;
        span_end = next_rise;
        sector_offset = 27;
    } else if (next_rise <= jd_ut && jd_ut < next_mc) {
        span_start = next_rise;
        span_end = next_mc;
        sector_offset = 0;
    } else if (next_mc <= jd_ut && jd_ut < next_set) {
        span_start = next_mc;
        span_end = next_set;
        sector_offset = 9;
    } else if (next_set <= jd_ut && jd_ut < next_ic) {
        span_start = next_set;
        span_end = next_ic;
        sector_offset = 18;
    } else {
        span_start = next_ic;
        span_end = next_rise + 1.0;
        sector_offset = 27;
    }

    if (!isfinite(span_start) || !isfinite(span_end) || span_end <= span_start) {
        jme_set_error(error, "Gauquelin sector span is invalid");
        return JME_ERR;
    }

    fraction = (jd_ut - span_start) / (span_end - span_start);
    if (fraction < 0.0) { fraction = 0.0; }
    if (fraction >= 1.0) { fraction = 0.999999999; }

    (void)imeth;
    *dgsect = floor(fraction * 9.0) + 1.0 + sector_offset;

    return JME_OK;
}
