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
    case JME_HOUSE_MERIDIAN: return 'X';
    case JME_HOUSE_MORINUS: return 'M';
    case JME_HOUSE_AZIMUTHAL: return 'A';
    case JME_HOUSE_HORIZONTAL: return 'A';
    default: return house_system;
    }
}

static double clamp_sector(double sector)
{
    if (sector < 1.0) {
        return 1.0;
    }
    if (sector >= 37.0) {
        return 36.999999999;
    }
    return sector;
}

static int gauquelin_sector_from_equatorial(double gst_deg, double ra, double dec, double geo_lon, double geo_lat, double *dgsect, char *error)
{
    double lst;
    double h;
    double arg;
    double h0;
    double nocturnal_arc;
    double fraction;

    if (dgsect == 0 || !isfinite(ra) || !isfinite(dec) || !isfinite(geo_lon) || !isfinite(geo_lat)) {
        jme_set_error(error, "Gauquelin semiarc inputs are invalid");
        return JME_ERR;
    }

    lst = jme_degree_normalize(gst_deg + geo_lon);
    h = jme_degrees_difference_signed(lst, ra);
    arg = -tan(geo_lat * JME_DEG_TO_RAD) * tan(dec * JME_DEG_TO_RAD);

    if (arg > -1.0 && arg < 1.0) {
        h0 = acos(arg) * JME_RAD_TO_DEG;
        nocturnal_arc = 180.0 - h0;

        if (h < 0.0 && h >= -h0) {
            fraction = (h + h0) / h0;
            *dgsect = clamp_sector(1.0 + fraction * 9.0);
            return JME_OK;
        }
        if (h >= 0.0 && h <= h0) {
            fraction = h / h0;
            *dgsect = clamp_sector(10.0 + fraction * 9.0);
            return JME_OK;
        }
        if (h > h0) {
            fraction = nocturnal_arc > 0.0 ? (h - h0) / nocturnal_arc : 0.0;
            *dgsect = clamp_sector(19.0 + fraction * 9.0);
            return JME_OK;
        }

        fraction = nocturnal_arc > 0.0 ? (h + 180.0) / nocturnal_arc : 0.0;
        *dgsect = clamp_sector(28.0 + fraction * 9.0);
        return JME_OK;
    }

    if (arg <= -1.0) {
        if (h < 0.0) {
            *dgsect = clamp_sector(1.0 + ((h + 180.0) / 180.0) * 9.0);
        } else {
            *dgsect = clamp_sector(10.0 + (h / 180.0) * 9.0);
        }
        return JME_OK;
    }

    if (h >= 0.0) {
        *dgsect = clamp_sector(19.0 + (h / 180.0) * 9.0);
    } else {
        *dgsect = clamp_sector(28.0 + ((h + 180.0) / 180.0) * 9.0);
    }
    return JME_OK;
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

/*
 * Placidus cusp: the ecliptic point whose meridian distance (in RA) is the
 * given fraction of its diurnal semi-arc, RA = RAMC + fraction * DSA(RA),
 * with tan(dec) = tan(eps) sin(RA) for ecliptic points. g(x) = x - RAMC -
 * fraction * DSA(x) changes sign between x = RAMC and x = RAMC + 180 fraction
 * (either sign of fraction), so the root is always bracketed; bisection to
 * the binary64 limit. Negative fractions give western points: cusp 8 has
 * hour angle +2/3 of its own semi-arc and cusp 9 +1/3, so cusps 2 and 3 are
 * the antipodes of the fraction -2/3 and -1/3 solutions.
 * Errors where the semi-arc is undefined (circumpolar ecliptic points).
 */
static int placidus_dsa(double ra, double tan_lat, double tan_eps, double *dsa)
{
    double arg = -sin(ra * JME_DEG_TO_RAD) * tan_lat * tan_eps;
    if (!(arg >= -1.0 && arg <= 1.0)) return JME_ERR;
    *dsa = acos(arg) * JME_RAD_TO_DEG;
    return JME_OK;
}

static int placidus_cusp(double ramc, double lat, double eps, double fraction, double *longitude)
{
    double tan_lat = tan(lat * JME_DEG_TO_RAD);
    double tan_eps = tan(eps * JME_DEG_TO_RAD);
    double lo = fraction < 0.0 ? 180.0 * fraction : 0.0;
    double hi = fraction < 0.0 ? 0.0 : 180.0 * fraction;
    double dsa;
    int i;

    if (longitude == 0 || fraction == 0.0 || !(fraction > -1.0 && fraction < 1.0)) {
        return JME_ERR;
    }
    for (i = 0; i < 200 && hi - lo > 0.0; i++) {
        double mid = 0.5 * (lo + hi);
        if (mid == lo || mid == hi) break;
        if (placidus_dsa(ramc + mid, tan_lat, tan_eps, &dsa) != JME_OK) return JME_ERR;
        if (mid - fraction * dsa < 0.0) lo = mid; else hi = mid;
    }
    if (placidus_dsa(ramc + lo, tan_lat, tan_eps, &dsa) != JME_OK
        || placidus_dsa(ramc + hi, tan_lat, tan_eps, &dsa) != JME_OK) {
        return JME_ERR;
    }
    *longitude = ecliptic_longitude_from_ra(jme_degree_normalize(ramc + 0.5 * (lo + hi)), eps);
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

/*
 * Polich-Page (topocentric) cusp: Polich & Page (1964) define the cusp as the
 * ascendant of the house circle whose oblique ascension is RAMC + offset,
 * taken at the pole height atan(fraction * tan(lat)). This is the ascendant
 * formula evaluated for RAMC' = RAMC + offset - 90 at that pole. (The
 * Regiomontanus formula must not be reused here: it applies its own
 * sin(offset) pole reduction on top of the Polich-Page pole.)
 */
static double ascendant_from_ramc(double ramc, double lat, double eps);

static double topocentric_cusp(double ramc, double lat, double eps, double house_offset, double pole_fraction)
{
    double pole = atan(tan(lat * JME_DEG_TO_RAD) * pole_fraction) * JME_RAD_TO_DEG;
    return ascendant_from_ramc(jme_degree_normalize(ramc + house_offset - 90.0), pole, eps);
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

/*
 * Campanus cusp: intersection of the ecliptic with the great circle through
 * the north and south points of the horizon and the point of the prime
 * vertical at elevation 90 - house_offset (house_offset 30 = cusp 11, 60 =
 * cusp 12, 120 = cusp 2, 150 = cusp 3). Of the two intersections the one on
 * the side of that prime-vertical point is the cusp.
 */
static int campanus_cusp(double ramc, double lat, double eps, double house_offset, double expected, double *longitude)
{
    double elev = (90.0 - house_offset) * JME_DEG_TO_RAD;
    double r = ramc * JME_DEG_TO_RAD;
    double p = lat * JME_DEG_TO_RAD;
    double e = eps * JME_DEG_TO_RAD;
    double east[3] = {-sin(r), cos(r), 0.0};
    double zenith[3] = {cos(p) * cos(r), cos(p) * sin(r), sin(p)};
    double north[3] = {
        zenith[1] * east[2] - zenith[2] * east[1],
        zenith[2] * east[0] - zenith[0] * east[2],
        zenith[0] * east[1] - zenith[1] * east[0]
    };
    double pv[3], n[3], a, b, l, v[3];
    int i;

    (void)expected;
    if (longitude == 0) return JME_ERR;
    for (i = 0; i < 3; i++) pv[i] = east[i] * cos(elev) + zenith[i] * sin(elev);
    n[0] = north[1] * pv[2] - north[2] * pv[1];
    n[1] = north[2] * pv[0] - north[0] * pv[2];
    n[2] = north[0] * pv[1] - north[1] * pv[0];
    /* ecliptic point (cos l, sin l cos e, sin l sin e) . n = 0 */
    a = n[0];
    b = n[1] * cos(e) + n[2] * sin(e);
    if (!(fabs(a) + fabs(b) > 0.0)) return JME_ERR;
    l = atan2(-a, b);
    v[0] = cos(l); v[1] = sin(l) * cos(e); v[2] = sin(l) * sin(e);
    if (v[0] * pv[0] + v[1] * pv[1] + v[2] * pv[2] < 0.0) l += 3.14159265358979323846264338327950288;
    *longitude = jme_degree_normalize(l * JME_RAD_TO_DEG);
    return JME_OK;
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

/* Ecliptic point on the prime vertical on the eastern side (positive
 * component along the east point of the horizon). */
static int horizontal_east_cusp(double ramc, double lat, double eps, double *longitude)
{
    double r = ramc * JME_DEG_TO_RAD;
    double p = lat * JME_DEG_TO_RAD;
    double e = eps * JME_DEG_TO_RAD;
    double east[3] = {-sin(r), cos(r), 0.0};
    double zenith[3] = {cos(p) * cos(r), cos(p) * sin(r), sin(p)};
    double pv_normal[3];
    double ecl_normal[3] = {0.0, -sin(e), cos(e)};
    double inter[3];
    double norm;
    int i;

    /* Prime vertical = great circle through zenith and east point. */
    pv_normal[0] = zenith[1] * east[2] - zenith[2] * east[1];
    pv_normal[1] = zenith[2] * east[0] - zenith[0] * east[2];
    pv_normal[2] = zenith[0] * east[1] - zenith[1] * east[0];
    inter[0] = pv_normal[1] * ecl_normal[2] - pv_normal[2] * ecl_normal[1];
    inter[1] = pv_normal[2] * ecl_normal[0] - pv_normal[0] * ecl_normal[2];
    inter[2] = pv_normal[0] * ecl_normal[1] - pv_normal[1] * ecl_normal[0];
    norm = sqrt(inter[0] * inter[0] + inter[1] * inter[1] + inter[2] * inter[2]);
    if (!isfinite(norm) || norm <= 0.0 || longitude == 0) {
        return JME_ERR;
    }
    if (inter[0] * east[0] + inter[1] * east[1] + inter[2] * east[2] < 0.0) norm = -norm;
    for (i = 0; i < 3; i++) inter[i] /= norm;
    *longitude = longitude_from_equatorial_vector(inter, eps);
    return JME_OK;
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
    case 'X':
        /*
         * Meridian (axial rotation) system: the equator is divided into 12
         * equal parts from the ARMC and each cusp is the ecliptic point
         * having that right ascension (projection along hour circles), so
         * cusp 10 is the MC and cusp 1 generally differs from the Ascendant.
         * Definition: R. Holden, The Elements of House Division (1977).
         */
        for (i = 1; i <= 12; i++) {
            cusps[i] = ecliptic_longitude_from_ra(ramc + (double)((i + 2) % 12) * 30.0, eps);
        }
        cusps[10] = mc;
        return JME_OK;
    case 'M':
        /*
         * Morinus system: the equator is divided into 12 equal parts from
         * the ARMC and those equator points (declination 0) are transformed
         * to ecliptic coordinates (projection along circles of ecliptic
         * longitude): tan(lambda) = sin(alpha) cos(eps) / cos(alpha).
         * Cusp 1 differs from the Ascendant and cusp 10 from the MC.
         * Definition: R. Holden, The Elements of House Division (1977).
         */
        for (i = 1; i <= 12; i++) {
            double a = (ramc + (double)((i + 2) % 12) * 30.0) * JME_DEG_TO_RAD;
            double e = eps * JME_DEG_TO_RAD;
            cusps[i] = jme_degree_normalize(atan2d(sin(a) * cos(e), cos(a)));
        }
        return JME_OK;
    case 'P':
        {
            double c11, c12, c2, c3;

            if (placidus_cusp(ramc, lat, eps, 1.0 / 3.0, &c11) != JME_OK
                || placidus_cusp(ramc, lat, eps, 2.0 / 3.0, &c12) != JME_OK
                || placidus_cusp(ramc, lat, eps, -2.0 / 3.0, &c2) != JME_OK
                || placidus_cusp(ramc, lat, eps, -1.0 / 3.0, &c3) != JME_OK) {
                return JME_ERR;
            }

            /* The fraction -2/3 and -1/3 solutions are cusps 8 and 9;
             * cusps 2 and 3 are their antipodes. */
            c2 = jme_degree_normalize(c2 + 180.0);
            c3 = jme_degree_normalize(c3 + 180.0);
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
        /*
         * Horizontal (azimuthal) system: the horizon is divided into 12 equal
         * azimuth arcs from the east point; each cusp is the intersection of
         * the ecliptic with the vertical circle (through the zenith) at that
         * azimuth. Cusp 10 lies on the meridian (the MC); cusp 1 lies on the
         * prime vertical due east and generally differs from the Ascendant.
         * Definition: R. Holden, The Elements of House Division (1977).
         */
        if (horizontal_east_cusp(ramc, lat, eps, &cusps[1]) != JME_OK) {
            return JME_ERR;
        }
        cusps[7] = jme_degree_normalize(cusps[1] + 180.0);
        if (horizontal_cusp(ramc, lat, eps, 120.0, jme_degree_normalize(cusps[1] + jme_degrees_difference(mc + 180.0, cusps[1]) / 3.0), &cusps[2]) != JME_OK
            || horizontal_cusp(ramc, lat, eps, 150.0, jme_degree_normalize(cusps[1] + 2.0 * jme_degrees_difference(mc + 180.0, cusps[1]) / 3.0), &cusps[3]) != JME_OK
            || horizontal_cusp(ramc, lat, eps, 30.0, jme_degree_normalize(mc + jme_degrees_difference(cusps[1], mc) / 3.0), &cusps[11]) != JME_OK
            || horizontal_cusp(ramc, lat, eps, 60.0, jme_degree_normalize(mc + 2.0 * jme_degrees_difference(cusps[1], mc) / 3.0), &cusps[12]) != JME_OK) {
            return JME_ERR;
        }
        cusps[4] = jme_degree_normalize(mc + 180.0);
        cusps[5] = jme_degree_normalize(cusps[11] + 180.0);
        cusps[6] = jme_degree_normalize(cusps[12] + 180.0);
        cusps[8] = jme_degree_normalize(cusps[2] + 180.0);
        cusps[9] = jme_degree_normalize(cusps[3] + 180.0);
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

/*
 * Houses at a UT instant. The sidereal angle is the context's Greenwich
 * apparent sidereal time (consistent with the selected precession and
 * nutation models; ERFA eraGst06/eraGst00a/eraGst94 family), evaluated with
 * UT1 for the Earth rotation part and TT for precession/nutation. The
 * obliquity is the true obliquity of date at TT. With JME_CALC_NO_NUTATION
 * the mean sidereal time and mean obliquity are used. With
 * JME_CALC_SIDEREAL the ecliptic results (cusps, Asc, MC, ARMC excluded,
 * Vertex, equatorial Ascendant, co-Ascendants, polar Ascendant) are
 * reduced by the context ayanamsa referred to the same equinox
 * (mean ayanamsa plus nutation in longitude unless NO_NUTATION).
 */
/*
 * Local frame of a house computation at a UT instant: ARMC (deg), obliquity
 * of date (true unless JME_CALC_NO_NUTATION) and, with JME_CALC_SIDEREAL,
 * the ayanamsa referred to the same equinox (else 0).
 */
static int houses_frame(double jd_ut, int flags, double geo_lat, double geo_lon,
    double *ramc, double *eps, double *aya, char *error)
{
    double jd_tt, gst, eps_mean, dpsi, deps;
    double nm[9];
    int apparent = !(flags & JME_CALC_NO_NUTATION);

    if (!isfinite(jd_ut) || !isfinite(geo_lat) || !isfinite(geo_lon)) {
        jme_set_error(error, "House inputs must be finite");
        return JME_ERR;
    }
    if (geo_lat < -90.0 || geo_lat > 90.0) {
        jme_set_error(error, "Geographic latitude must be within [-90, 90] degrees");
        return JME_ERR;
    }
    if (internal_ut_to_et(jd_ut, &jd_tt, error) != JME_OK) return JME_ERR;
    if (internal_nutation_matrix(jd_tt, nm, &dpsi, &eps_mean, &deps, error) != JME_OK) return JME_ERR;
    *eps = apparent ? eps_mean + deps : eps_mean;
    if (internal_sidereal_time_rad(jd_ut, jd_tt, apparent, &gst, error) != JME_OK) return JME_ERR;
    *ramc = jme_degree_normalize(gst / JME_DEG_TO_RAD + geo_lon);
    *aya = 0.0;
    if (flags & JME_CALC_SIDEREAL) {
        double rate;
        if (internal_context_ayanamsa(jd_tt, aya, &rate, error) != JME_OK) return JME_ERR;
        if (apparent) *aya += dpsi;
    }
    return JME_OK;
}

static int houses_compute(double jd_ut, int flags, double geo_lat, double geo_lon, int house_system, double *cusps, double *ascmc, char *error)
{
    double ramc, eps, aya = 0.0;
    double local_ascmc[10];
    int system = normalize_house_system(house_system);
    int i;

    if (cusps == 0) {
        jme_set_error(error, "House cusp output is required");
        return JME_ERR;
    }
    if (houses_frame(jd_ut, flags, geo_lat, geo_lon, &ramc, &eps, &aya, error) != JME_OK) return JME_ERR;

    calc_ascmc(ramc, geo_lat, eps, local_ascmc);
    if (flags & JME_CALC_SIDEREAL) {
        /* ascmc[0..5] are ecliptic longitudes; [6] ARMC and [7] obliquity are not. */
        for (i = 0; i <= 5; i++) {
            if (isfinite(local_ascmc[i])) local_ascmc[i] = jme_degree_normalize(local_ascmc[i] - aya);
        }
    }
    if (ascmc != 0) {
        for (i = 0; i < 10; i++) ascmc[i] = local_ascmc[i];
    }

    if (system == 'I') {
        double sun_equ[6];
        /* Apparent declination of the Sun (true equator of date). */
        if (jme_calc_ut(jd_ut, JME_BODY_SUN, JME_CALC_EQUATORIAL | (flags & JME_CALC_NO_NUTATION), sun_equ, error) != JME_OK) {
            return JME_ERR;
        }
        if (ascmc != 0) ascmc[9] = sun_equ[1];
        /* Tropical angles are needed by the cusp construction. */
        calc_ascmc(ramc, geo_lat, eps, local_ascmc);
        if (sunshine_cusps(local_ascmc[0], local_ascmc[1], ramc, geo_lat, eps, sun_equ[1], cusps) != JME_OK) {
            jme_set_error(error, "Sunshine house cusps undefined for these inputs");
            return JME_ERR;
        }
    } else {
        calc_ascmc(ramc, geo_lat, eps, local_ascmc);
        if (fill_house_cusps_from_angles(local_ascmc[0], local_ascmc[1], ramc, geo_lat, eps, house_system, cusps) != JME_OK) {
            jme_set_error(error, "House cusps undefined for this system at this latitude");
            return JME_ERR;
        }
    }
    if (flags & JME_CALC_SIDEREAL) {
        for (i = 1; i <= 12; i++) cusps[i] = jme_degree_normalize(cusps[i] - aya);
    }
    return JME_OK;
}

int jme_houses_ex(double jd_ut, int flags, double geo_lat, double geo_lon, int house_system, double *cusps, double *ascmc)
{
    return houses_compute(jd_ut, flags, geo_lat, geo_lon, house_system, cusps, ascmc, 0);
}

double jme_house_pos(double armc, double geo_lat, double eps, int house_system, double *xpin, char *error)
{
    double ascmc[10];
    double cusps[13];
    double lon;
    int i;

    if (xpin == 0) {
        jme_set_error(error, "House position input is required");
        return NAN;
    }

    if (jme_houses_armc(armc, geo_lat, eps, house_system, cusps, ascmc) != JME_OK) {
        jme_set_error(error, "House system is not available");
        return NAN;
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
    /* Rates in degrees per day by the five-point central stencil
     * f' = [f(-2h) - 8 f(-h) + 8 f(h) - f(2h)] / (12 h), truncation O(h^4),
     * h = 1e-3 d. Angle differences are unwrapped relative to the centre. */
    static const double step = 1.0e-3;
    static const double offs[4] = { -2.0, -1.0, 1.0, 2.0 };
    static const double wts[4] = { 1.0, -8.0, 8.0, -1.0 };
    double sc[4][13];
    double sa[4][10];
    double centre_cusps[13];
    double centre_ascmc[10];
    int i, k;
    int rc = jme_houses_ex(jd_ut, flags, geo_lat, geo_lon, house_system, cusps, ascmc);

    if (rc != JME_OK) {
        return rc;
    }
    if (cusps_speed == 0 && ascmc_speed == 0) return JME_OK;
    for (i = 0; i < 13; i++) centre_cusps[i] = cusps[i];
    if (ascmc != 0) {
        for (i = 0; i < 10; i++) centre_ascmc[i] = ascmc[i];
    } else if (jme_houses_ex(jd_ut, flags, geo_lat, geo_lon, house_system, centre_cusps, centre_ascmc) != JME_OK) {
        return JME_ERR;
    }
    for (k = 0; k < 4; k++) {
        if (jme_houses_ex(jd_ut + offs[k] * step, flags, geo_lat, geo_lon, house_system, sc[k], sa[k]) != JME_OK) {
            /* A neighbouring instant is undefined (e.g. a polar transition):
             * the rate is not defined; report NaN rather than a false zero. */
            if (cusps_speed != 0) for (i = 0; i < 13; i++) cusps_speed[i] = NAN;
            if (ascmc_speed != 0) for (i = 0; i < 10; i++) ascmc_speed[i] = NAN;
            return JME_OK;
        }
    }
    if (cusps_speed != 0) {
        cusps_speed[0] = 0.0;
        for (i = 1; i <= 12; i++) {
            double acc = 0.0;
            for (k = 0; k < 4; k++) acc += wts[k] * jme_degrees_difference_signed(sc[k][i], centre_cusps[i]);
            cusps_speed[i] = acc / (12.0 * step);
        }
    }
    if (ascmc_speed != 0) {
        for (i = 0; i < 10; i++) {
            double acc = 0.0;
            for (k = 0; k < 4; k++) {
                if (i == 7 || i == 9) acc += wts[k] * (sa[k][i] - centre_ascmc[i]);
                else acc += wts[k] * jme_degrees_difference_signed(sa[k][i], centre_ascmc[i]);
            }
            ascmc_speed[i] = acc / (12.0 * step);
        }
    }
    return JME_OK;
}

int jme_houses_armc(double armc, double geo_lat, double eps, int house_system, double *cusps, double *ascmc)
{
    double local_ascmc[10];
    double input_sun_decl = ascmc != 0 ? ascmc[9] : NAN;
    int system = normalize_house_system(house_system);
    int i;

    if (cusps == 0) {
        return JME_ERR;
    }

    calc_ascmc(armc, geo_lat, eps, local_ascmc);
    if (ascmc != 0) {
        for (i = 0; i < 10; i++) {
            ascmc[i] = local_ascmc[i];
        }
        if (system == 'I' && isfinite(input_sun_decl)) {
            ascmc[9] = input_sun_decl;
        }
    }

    if (system == 'I') {
        if (!isfinite(input_sun_decl)) {
            return JME_ERR;
        }
        return sunshine_cusps(local_ascmc[0], local_ascmc[1], armc, geo_lat, eps, input_sun_decl, cusps);
    }

    return fill_house_cusps_from_angles(local_ascmc[0], local_ascmc[1], armc, geo_lat, eps, house_system, cusps);
}

int jme_houses_armc_ex2(double armc, double geo_lat, double eps, int house_system, double *cusps, double *ascmc, double *cusps_speed, double *ascmc_speed)
{
    /* Derivatives with respect to ARMC (degrees per degree of ARMC) at fixed
     * latitude and obliquity, by the five-point central stencil with
     * h = 1e-3 degree (truncation O(h^4)). For the Sunshine system the
     * caller's solar declination in ascmc[9] is held fixed. */
    static const double step = 1.0e-3;
    static const double offs[4] = { -2.0, -1.0, 1.0, 2.0 };
    static const double wts[4] = { 1.0, -8.0, 8.0, -1.0 };
    double sc[4][13];
    double sa[4][10];
    double centre_ascmc[10];
    double sun_decl = ascmc != 0 ? ascmc[9] : NAN;
    int i, k;
    int rc;

    for (i = 0; i < 10; i++) centre_ascmc[i] = 0.0;
    centre_ascmc[9] = sun_decl;
    rc = jme_houses_armc(armc, geo_lat, eps, house_system, cusps, centre_ascmc);
    if (ascmc != 0) for (i = 0; i < 10; i++) ascmc[i] = centre_ascmc[i];
    if (rc != JME_OK) {
        return rc;
    }
    if (cusps_speed == 0 && ascmc_speed == 0) return JME_OK;
    for (k = 0; k < 4; k++) {
        for (i = 0; i < 10; i++) sa[k][i] = 0.0;
        sa[k][9] = sun_decl;
        if (jme_houses_armc(armc + offs[k] * step, geo_lat, eps, house_system, sc[k], sa[k]) != JME_OK) {
            if (cusps_speed != 0) for (i = 0; i < 13; i++) cusps_speed[i] = NAN;
            if (ascmc_speed != 0) for (i = 0; i < 10; i++) ascmc_speed[i] = NAN;
            return JME_OK;
        }
    }
    if (cusps_speed != 0) {
        cusps_speed[0] = 0.0;
        for (i = 1; i <= 12; i++) {
            double acc = 0.0;
            for (k = 0; k < 4; k++) acc += wts[k] * jme_degrees_difference_signed(sc[k][i], cusps[i]);
            cusps_speed[i] = acc / (12.0 * step);
        }
    }
    if (ascmc_speed != 0) {
        for (i = 0; i < 10; i++) {
            double acc = 0.0;
            if (i == 7 || i == 8 || i == 9) { ascmc_speed[i] = 0.0; continue; }
            for (k = 0; k < 4; k++) acc += wts[k] * jme_degrees_difference_signed(sa[k][i], centre_ascmc[i]);
            ascmc_speed[i] = acc / (12.0 * step);
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
    int rsmi_extra = 0;

    if (dgsect != 0) { *dgsect = 0.0; }
    if (dgsect == 0 || geopos == 0) {
        jme_set_error(error, "Gauquelin sector output arguments are required");
        return JME_ERR;
    }

    if (imeth < 0 || imeth > 3) {
        jme_set_error(error, "Gauquelin method must be 0, 1, 2, or 3");
        return JME_ERR;
    }

    if (imeth == 0 || imeth == 1) {
        /* Methods 0/1: semi-arc position from the body's place on the true
         * equator and equinox of date (apparent unless the caller passes
         * JME_CALC_TRUE_POSITION), against apparent sidereal time (mean
         * with JME_CALC_NO_NUTATION). Method 1 drops the ecliptic latitude
         * before converting with the true obliquity of date at TT. */
        double pos[6];
        double ra, dec, eps, eps_mean, dpsi, deps, jd_tt, gst;
        double nm[9];
        int apparent = !(flags & JME_CALC_NO_NUTATION);
        int pflags = flags & (JME_CALC_TRUE_POSITION | JME_CALC_NO_NUTATION);

        if (!isfinite(jd_ut) || !isfinite(geopos[0]) || !isfinite(geopos[1]) || geopos[1] < -90.0 || geopos[1] > 90.0) {
            jme_set_error(error, "Gauquelin sector inputs are invalid");
            return JME_ERR;
        }
        if (starname != 0 && starname[0] != '\0') {
            if (jme_fixstar_ut(starname, jd_ut, pflags, pos, error) != JME_OK) {
                return JME_ERR;
            }
        } else if (jme_calc_ut(jd_ut, body, pflags, pos, error) != JME_OK) {
            return JME_ERR;
        }
        if (internal_ut_to_et(jd_ut, &jd_tt, error) != JME_OK
            || internal_nutation_matrix(jd_tt, nm, &dpsi, &eps_mean, &deps, error) != JME_OK
            || internal_sidereal_time_rad(jd_ut, jd_tt, apparent, &gst, error) != JME_OK) {
            return JME_ERR;
        }
        eps = apparent ? eps_mean + deps : eps_mean;
        jme_ecliptic_to_equatorial(pos[0], imeth == 0 ? pos[1] : 0.0, eps, &ra, &dec);
        return gauquelin_sector_from_equatorial(gst * JME_RAD_TO_DEG, ra, dec, geopos[0], geopos[1], dgsect, error);
    }

    if (imeth == 2) {
        atpress = 0.0;
    } else if (atpress == 0.0) {
        atpress = 1013.25;
    }

    rsmi_extra = JME_RISE_DISC_CENTER;
    if (imeth == 2) {
        rsmi_extra |= JME_RISE_NO_REFRACTION;
    }

    rc_rise_prev = jme_rise_trans(day_start - 1.0, body, starname, flags, JME_RISE_RISE | rsmi_extra, geopos, atpress, attemp, &prev_rise, error);
    rc_rise_next = jme_rise_trans(day_start, body, starname, flags, JME_RISE_RISE | rsmi_extra, geopos, atpress, attemp, &next_rise, error);
    rc_set_prev = jme_rise_trans(day_start - 1.0, body, starname, flags, JME_RISE_SET | rsmi_extra, geopos, atpress, attemp, &prev_set, error);
    rc_set_next = jme_rise_trans(day_start, body, starname, flags, JME_RISE_SET | rsmi_extra, geopos, atpress, attemp, &next_set, error);
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

    *dgsect = clamp_sector(1.0 + sector_offset + fraction * 9.0);

    return JME_OK;
}

/*
 * GAUQUELIN 36 SECTORS
 * ====================
 * M. Gauquelin's sectors divide the diurnal and the nocturnal semi-arc of
 * every point into 18 equal parts of time each (a Placidus-type division,
 * nine sectors per quadrant), numbered in the direction of the diurnal
 * motion: sector 1 begins at the rising point (Ascendant), sector 10 at the
 * upper culmination (MC), 19 at setting (Descendant), 28 at the lower
 * culmination (IC). This is the numbering of jme_gauquelin_sector().
 *
 * Cusp k (1..36) is the ecliptic point at the start of sector k. For the
 * diurnal cusps 2..18 the hour angle H of the point is H = ((k - 10)/9) DSA,
 * i.e. its meridian distance in right ascension is the Placidus fraction
 * (10 - k)/9 of its own diurnal semi-arc; it is solved with the same
 * bracketed Placidus solver as the 12-house system (binary64 limit).
 * Nocturnal cusps are the antipodes: cusp k + 18 = cusp k + 180 deg (an
 * ecliptic point and its antipode have complementary semi-arcs).
 * cusps[0] is set to 0; cusps[1] = Ascendant, cusps[10] = MC.
 * Errors where a semi-arc is undefined (circumpolar ecliptic points).
 * Source of the sector definition: M. Gauquelin, L'influence des astres
 * (1955); identical to the sector numbering of jme_gauquelin_sector().
 */
static int gauquelin_cusps(double ramc, double lat, double eps, double *cusps)
{
    double angles[10];
    int k;

    calc_ascmc(ramc, lat, eps, angles);
    if (!isfinite(angles[0]) || !isfinite(angles[1])) return JME_ERR;
    cusps[0] = 0.0;
    cusps[1] = angles[0];
    cusps[10] = angles[1];
    for (k = 2; k <= 18; k++) {
        if (k == 10) continue;
        if (placidus_cusp(ramc, lat, eps, (double)(10 - k) / 9.0, &cusps[k]) != JME_OK) return JME_ERR;
    }
    for (k = 1; k <= 18; k++) cusps[k + 18] = jme_degree_normalize(cusps[k] + 180.0);
    return JME_OK;
}

int jme_houses_gauquelin_armc(double armc, double geo_lat, double eps, double *cusps, double *ascmc, char *error)
{
    double local_ascmc[10];
    int i;

    if (cusps == 0) {
        jme_set_error(error, "Gauquelin cusp output (37 doubles) is required");
        return JME_ERR;
    }
    if (!isfinite(armc) || !isfinite(geo_lat) || !isfinite(eps) || geo_lat < -90.0 || geo_lat > 90.0) {
        jme_set_error(error, "Gauquelin sector inputs are invalid");
        return JME_ERR;
    }
    armc = jme_degree_normalize(armc);
    if (gauquelin_cusps(armc, geo_lat, eps, cusps) != JME_OK) {
        jme_set_error(error, "Gauquelin sectors undefined at this latitude (circumpolar ecliptic points)");
        return JME_ERR;
    }
    if (ascmc != 0) {
        calc_ascmc(armc, geo_lat, eps, local_ascmc);
        for (i = 0; i < 10; i++) ascmc[i] = local_ascmc[i];
    }
    return JME_OK;
}

int jme_houses_gauquelin(double jd_ut, int flags, double geo_lat, double geo_lon, double *cusps, double *ascmc, char *error)
{
    double ramc, eps, aya;
    int i;

    if (cusps == 0) {
        jme_set_error(error, "Gauquelin cusp output (37 doubles) is required");
        return JME_ERR;
    }
    if (houses_frame(jd_ut, flags, geo_lat, geo_lon, &ramc, &eps, &aya, error) != JME_OK) return JME_ERR;
    if (jme_houses_gauquelin_armc(ramc, geo_lat, eps, cusps, ascmc, error) != JME_OK) return JME_ERR;
    if (flags & JME_CALC_SIDEREAL) {
        for (i = 1; i <= 36; i++) cusps[i] = jme_degree_normalize(cusps[i] - aya);
        if (ascmc != 0) {
            for (i = 0; i <= 5; i++) {
                if (isfinite(ascmc[i])) ascmc[i] = jme_degree_normalize(ascmc[i] - aya);
            }
        }
    }
    return JME_OK;
}
