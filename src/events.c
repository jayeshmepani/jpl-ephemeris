#include "jme/jme.h"
#include "context.h"
#include <math.h>
#include <string.h>

#define JME_EVENTS_RAD_TO_DEG 57.295779513082320876798154814105170332405472466565
#define JME_EVENTS_DEG_TO_RAD 0.017453292519943295769236907684886127134428718885417
#define JME_EVENTS_PI 3.14159265358979323846264338327950288419716939937510
#define JME_EVENTS_EARTH_RADIUS_KM 6378.137
#define JME_EVENTS_MOON_RADIUS_KM 1737.4
#define JME_EVENTS_SUN_RADIUS_KM 695700.0

static double vec_dot(const double *a, const double *b)
{
    return a[0] * b[0] + a[1] * b[1] + a[2] * b[2];
}

static void vec_cross(const double *a, const double *b, double *out)
{
    out[0] = a[1] * b[2] - a[2] * b[1];
    out[1] = a[2] * b[0] - a[0] * b[2];
    out[2] = a[0] * b[1] - a[1] * b[0];
}

static double vec_norm(const double *a)
{
    return sqrt(vec_dot(a, a));
}

static double orbital_radius_from_elements(double a, double e, double true_anomaly_deg)
{
    double denom = 1.0 + e * cos(true_anomaly_deg * JME_EVENTS_DEG_TO_RAD);

    if (!isfinite(a) || !isfinite(e) || !isfinite(true_anomaly_deg) || a <= 0.0 || denom == 0.0) {
        return NAN;
    }

    return a * (1.0 - e * e) / denom;
}

static double get_altitude(double jd_ut, int body, const char *starname, int flags, double *geopos, double atpress, double attemp, char *error);

static int lunar_eclipse_geometry(double jd_ut, double *umbral_mag, double *penumbral_mag, double *umbral_radius_km, double *penumbral_radius_km, double *offset_km, double *axis_distance_km, char *error)
{
    double sun_state[6];
    double moon_state[6];
    double sun_vec[3];
    double moon_vec[3];
    double axis[3];
    double along;
    double offset_vec[3];
    double sun_distance_km;
    double earth_radius_km;
    double moon_distance_km;
    double umbra_km;
    double penumbra_km;
    double offset;
    int i;

    if (jme_calc_ut(jd_ut, JME_BODY_SUN, JME_CALC_TRUE_POSITION | JME_CALC_XYZ, sun_state, error) != JME_OK
        || jme_calc_ut(jd_ut, JME_BODY_MOON, JME_CALC_TRUE_POSITION | JME_CALC_XYZ, moon_state, error) != JME_OK) {
        return JME_ERR;
    }

    for (i = 0; i < 3; i++) {
        sun_vec[i] = sun_state[i] * JME_AU_KM;
        moon_vec[i] = moon_state[i] * JME_AU_KM;
    }

    sun_distance_km = vec_norm(sun_vec);
    if (!isfinite(sun_distance_km) || sun_distance_km <= 0.0) {
        jme_set_error(error, "Lunar eclipse geometry requires a valid Sun-Earth distance");
        return JME_ERR;
    }

    for (i = 0; i < 3; i++) {
        axis[i] = -sun_vec[i] / sun_distance_km;
    }

    along = vec_dot(moon_vec, axis);
    for (i = 0; i < 3; i++) {
        offset_vec[i] = moon_vec[i] - along * axis[i];
    }

    offset = vec_norm(offset_vec);
    earth_radius_km = JME_EVENTS_EARTH_RADIUS_KM * (1.0 + 1.0 / 85.0);
    moon_distance_km = vec_norm(moon_vec);

    umbra_km = earth_radius_km - along * (JME_EVENTS_SUN_RADIUS_KM - earth_radius_km) / sun_distance_km;
    penumbra_km = earth_radius_km + along * (JME_EVENTS_SUN_RADIUS_KM + earth_radius_km) / sun_distance_km;

    if (!isfinite(along) || !isfinite(offset) || !isfinite(moon_distance_km)
        || !isfinite(umbra_km) || !isfinite(penumbra_km)
        || moon_distance_km <= 0.0 || penumbra_km <= 0.0) {
        jme_set_error(error, "Lunar eclipse geometry produced non-finite shadow values");
        return JME_ERR;
    }

    if (umbral_mag != 0) {
        *umbral_mag = (umbra_km + JME_EVENTS_MOON_RADIUS_KM - offset) / (2.0 * JME_EVENTS_MOON_RADIUS_KM);
    }
    if (penumbral_mag != 0) {
        *penumbral_mag = (penumbra_km + JME_EVENTS_MOON_RADIUS_KM - offset) / (2.0 * JME_EVENTS_MOON_RADIUS_KM);
    }
    if (umbral_radius_km != 0) {
        *umbral_radius_km = umbra_km;
    }
    if (penumbral_radius_km != 0) {
        *penumbral_radius_km = penumbra_km;
    }
    if (offset_km != 0) {
        *offset_km = offset;
    }
    if (axis_distance_km != 0) {
        *axis_distance_km = along;
    }

    return JME_OK;
}

static double lunar_eclipse_offset_metric(double jd_ut, char *error)
{
    double offset = 0.0;
    double axis_distance = 0.0;

    if (lunar_eclipse_geometry(jd_ut, 0, 0, 0, 0, &offset, &axis_distance, error) != JME_OK) {
        return NAN;
    }

    if (axis_distance <= 0.0) {
        return fabs(axis_distance) + 1.0e12;
    }

    return offset;
}

static int find_full_moon_opposition(double jd_start, int backward, double *tret, char *error)
{
    double direction = backward ? -1.0 : 1.0;
    double t0 = jd_start;
    double prev_diff;
    int i;

    {
        double sun_state[6];
        double moon_state[6];

        if (jme_calc_ut(t0, JME_BODY_SUN, JME_CALC_TRUE_POSITION, sun_state, error) != JME_OK
            || jme_calc_ut(t0, JME_BODY_MOON, JME_CALC_TRUE_POSITION, moon_state, error) != JME_OK) {
            return JME_ERR;
        }
        prev_diff = jme_degrees_difference_signed(moon_state[0], sun_state[0] + 180.0);
    }

    for (i = 1; i <= 120; i++) {
        double t1 = jd_start + direction * (0.5 * i);
        double sun_state[6];
        double moon_state[6];
        double diff;
        int j;

        if (jme_calc_ut(t1, JME_BODY_SUN, JME_CALC_TRUE_POSITION, sun_state, error) != JME_OK
            || jme_calc_ut(t1, JME_BODY_MOON, JME_CALC_TRUE_POSITION, moon_state, error) != JME_OK) {
            return JME_ERR;
        }
        diff = jme_degrees_difference_signed(moon_state[0], sun_state[0] + 180.0);

        if ((prev_diff <= 0.0 && diff >= 0.0) || (prev_diff >= 0.0 && diff <= 0.0)) {
            double a = t0;
            double b = t1;
            double fa = prev_diff;

            for (j = 0; j < 48; j++) {
                double m = 0.5 * (a + b);
                double sun_mid[6];
                double moon_mid[6];
                double fm;

                if (jme_calc_ut(m, JME_BODY_SUN, JME_CALC_TRUE_POSITION, sun_mid, error) != JME_OK
                    || jme_calc_ut(m, JME_BODY_MOON, JME_CALC_TRUE_POSITION, moon_mid, error) != JME_OK) {
                    return JME_ERR;
                }
                fm = jme_degrees_difference_signed(moon_mid[0], sun_mid[0] + 180.0);

                if (fabs(fm) < 1e-8 || fabs(b - a) < 1e-8) {
                    *tret = m;
                    return JME_OK;
                }

                if ((fa <= 0.0 && fm >= 0.0) || (fa >= 0.0 && fm <= 0.0)) {
                    b = m;
                } else {
                    a = m;
                    fa = fm;
                }
            }

            *tret = 0.5 * (a + b);
            return JME_OK;
        }

        t0 = t1;
        prev_diff = diff;
    }

    jme_set_error(error, "Full-moon opposition not found in search window");
    return JME_ERR;
}

static int refine_lunar_eclipse_maximum(double guess, double *tmax, char *error)
{
    double a = guess - 0.75;
    double b = guess + 0.75;
    double gr = 0.6180339887498948482;
    double x1 = b - gr * (b - a);
    double x2 = a + gr * (b - a);
    double f1 = lunar_eclipse_offset_metric(x1, error);
    double f2 = lunar_eclipse_offset_metric(x2, error);
    int i;

    if (!isfinite(f1) || !isfinite(f2)) {
        return JME_ERR;
    }

    for (i = 0; i < 64; i++) {
        if (f1 > f2) {
            a = x1;
            x1 = x2;
            f1 = f2;
            x2 = a + gr * (b - a);
            f2 = lunar_eclipse_offset_metric(x2, error);
        } else {
            b = x2;
            x2 = x1;
            f2 = f1;
            x1 = b - gr * (b - a);
            f1 = lunar_eclipse_offset_metric(x1, error);
        }

        if (!isfinite(f1) || !isfinite(f2)) {
            return JME_ERR;
        }

        if (fabs(b - a) < 1e-7) {
            break;
        }
    }

    *tmax = 0.5 * (a + b);
    return JME_OK;
}

static int find_contact_time(double tmid, double threshold_km, int left_side, double *tret, char *error)
{
    double step = 0.02;
    double t_prev = tmid;
    double f_prev;
    int i;

    f_prev = lunar_eclipse_offset_metric(t_prev, error) - threshold_km;
    if (!isfinite(f_prev)) {
        return JME_ERR;
    }

    for (i = 1; i <= 100; i++) {
        double t = tmid + (left_side ? -1.0 : 1.0) * step * i;
        double f = lunar_eclipse_offset_metric(t, error) - threshold_km;
        int j;

        if (!isfinite(f)) {
            return JME_ERR;
        }

        if ((f_prev <= 0.0 && f >= 0.0) || (f_prev >= 0.0 && f <= 0.0)) {
            double a = left_side ? t : t_prev;
            double b = left_side ? t_prev : t;
            double fa = left_side ? f : f_prev;

            for (j = 0; j < 48; j++) {
                double m = 0.5 * (a + b);
                double fm = lunar_eclipse_offset_metric(m, error) - threshold_km;

                if (!isfinite(fm)) {
                    return JME_ERR;
                }

                if (fabs(fm) < 1e-6 || fabs(b - a) < 1e-8) {
                    *tret = m;
                    return JME_OK;
                }

                if ((fa <= 0.0 && fm >= 0.0) || (fa >= 0.0 && fm <= 0.0)) {
                    b = m;
                } else {
                    a = m;
                    fa = fm;
                }
            }

            *tret = 0.5 * (a + b);
            return JME_OK;
        }

        t_prev = t;
        f_prev = f;
    }

    return JME_ERR;
}

static int classify_lunar_eclipse(double umbral_mag, double penumbral_mag)
{
    if (umbral_mag >= 1.0) {
        return JME_ECLIPSE_LUNAR_TOTAL;
    }
    if (umbral_mag > 0.0) {
        return JME_ECLIPSE_LUNAR_PARTIAL;
    }
    if (penumbral_mag > 0.0) {
        return JME_ECLIPSE_LUNAR_PENUMBRAL;
    }
    return 0;
}

static int moon_is_above_horizon(double jd_ut, double *geopos, char *error, double *altitude_out)
{
    double altitude = get_altitude(jd_ut, JME_BODY_MOON, 0, JME_CALC_TRUE_POSITION, geopos, 0.0, 0.0, error);

    if (altitude_out != 0) {
        *altitude_out = altitude;
    }

    return altitude > 0.0;
}

static int find_new_moon_conjunction(double jd_start, int backward, double *tret, char *error)
{
    double direction = backward ? -1.0 : 1.0;
    double t0 = jd_start;
    double prev_diff;
    int i;

    {
        double sun_state[6];
        double moon_state[6];

        if (jme_calc_ut(t0, JME_BODY_SUN, JME_CALC_TRUE_POSITION, sun_state, error) != JME_OK
            || jme_calc_ut(t0, JME_BODY_MOON, JME_CALC_TRUE_POSITION, moon_state, error) != JME_OK) {
            return JME_ERR;
        }
        prev_diff = jme_degrees_difference_signed(moon_state[0], sun_state[0]);
    }

    for (i = 1; i <= 120; i++) {
        double t1 = jd_start + direction * (0.5 * i);
        double sun_state[6];
        double moon_state[6];
        double diff;
        int j;

        if (jme_calc_ut(t1, JME_BODY_SUN, JME_CALC_TRUE_POSITION, sun_state, error) != JME_OK
            || jme_calc_ut(t1, JME_BODY_MOON, JME_CALC_TRUE_POSITION, moon_state, error) != JME_OK) {
            return JME_ERR;
        }
        diff = jme_degrees_difference_signed(moon_state[0], sun_state[0]);

        if ((prev_diff <= 0.0 && diff >= 0.0) || (prev_diff >= 0.0 && diff <= 0.0)) {
            double a = t0;
            double b = t1;
            double fa = prev_diff;

            for (j = 0; j < 48; j++) {
                double m = 0.5 * (a + b);
                double sun_mid[6];
                double moon_mid[6];
                double fm;

                if (jme_calc_ut(m, JME_BODY_SUN, JME_CALC_TRUE_POSITION, sun_mid, error) != JME_OK
                    || jme_calc_ut(m, JME_BODY_MOON, JME_CALC_TRUE_POSITION, moon_mid, error) != JME_OK) {
                    return JME_ERR;
                }
                fm = jme_degrees_difference_signed(moon_mid[0], sun_mid[0]);

                if (fabs(fm) < 1e-8 || fabs(b - a) < 1e-8) {
                    *tret = m;
                    return JME_OK;
                }

                if ((fa <= 0.0 && fm >= 0.0) || (fa >= 0.0 && fm <= 0.0)) {
                    b = m;
                } else {
                    a = m;
                    fa = fm;
                }
            }

            *tret = 0.5 * (a + b);
            return JME_OK;
        }

        t0 = t1;
        prev_diff = diff;
    }

    jme_set_error(error, "New-moon conjunction not found in search window");
    return JME_ERR;
}

static double angular_radius_deg(double radius_km, double distance_km)
{
    if (!isfinite(radius_km) || !isfinite(distance_km) || radius_km <= 0.0 || distance_km <= radius_km) {
        return NAN;
    }

    return asin(radius_km / distance_km) * JME_EVENTS_RAD_TO_DEG;
}

static double normalize_lon_180(double lon)
{
    lon = jme_degree_normalize(lon);
    if (lon > 180.0) {
        lon -= 360.0;
    }
    return lon;
}

static int solar_eclipse_global_geometry(double jd_ut, double *separation_deg, double *sun_radius_deg, double *moon_radius_deg, double *sub_lon, double *sub_lat, char *error)
{
    double sun_eq[6];
    double moon_eq[6];
    double sun_xyz[6];
    double moon_xyz[6];
    double sun_dist_km;
    double moon_dist_km;
    double gst;

    if (jme_calc_ut(jd_ut, JME_BODY_SUN, JME_CALC_TRUE_POSITION | JME_CALC_EQUATORIAL, sun_eq, error) != JME_OK
        || jme_calc_ut(jd_ut, JME_BODY_MOON, JME_CALC_TRUE_POSITION | JME_CALC_EQUATORIAL, moon_eq, error) != JME_OK
        || jme_calc_ut(jd_ut, JME_BODY_SUN, JME_CALC_TRUE_POSITION | JME_CALC_XYZ, sun_xyz, error) != JME_OK
        || jme_calc_ut(jd_ut, JME_BODY_MOON, JME_CALC_TRUE_POSITION | JME_CALC_XYZ, moon_xyz, error) != JME_OK) {
        return JME_ERR;
    }

    sun_dist_km = jme_state_distance(sun_xyz) * JME_AU_KM;
    moon_dist_km = jme_state_distance(moon_xyz) * JME_AU_KM;

    if (separation_deg != 0) {
        *separation_deg = jme_spherical_angular_separation(sun_eq[0], sun_eq[1], moon_eq[0], moon_eq[1]);
    }
    if (sun_radius_deg != 0) {
        *sun_radius_deg = angular_radius_deg(JME_EVENTS_SUN_RADIUS_KM, sun_dist_km);
    }
    if (moon_radius_deg != 0) {
        *moon_radius_deg = angular_radius_deg(JME_EVENTS_MOON_RADIUS_KM, moon_dist_km);
    }
    if (sub_lon != 0 || sub_lat != 0) {
        gst = jme_sidereal_time(jd_ut) * 15.0;
        if (sub_lon != 0) {
            *sub_lon = normalize_lon_180(moon_eq[0] - gst);
        }
        if (sub_lat != 0) {
            *sub_lat = moon_eq[1];
        }
    }

    return JME_OK;
}

static int solar_eclipse_local_geometry(double jd_ut, double *geopos, double *separation_deg, double *sun_radius_deg, double *moon_radius_deg, double *sun_alt_deg, double *moon_alt_deg, char *error)
{
    double sun_eq_xyz[6];
    double moon_eq_xyz[6];
    double topo_pos[3];
    double topo_state[6];
    double prec_mat[9];
    double nut_mat[9];
    double eps;
    double dpsi;
    double deps;
    double sun_topo_xyz[6];
    double moon_topo_xyz[6];
    double sun_eq[6];
    double moon_eq[6];
    double sun_dist_km;
    double moon_dist_km;
    int i;

    if (geopos == 0) {
        jme_set_error(error, "Solar eclipse local geometry requires geopos");
        return JME_ERR;
    }

    jme_set_topo(geopos[0], geopos[1], geopos[2]);

    if (jme_calc_ut(jd_ut, JME_BODY_SUN, JME_CALC_TRUE_POSITION | JME_CALC_XYZ | JME_CALC_EQUATORIAL, sun_eq_xyz, error) != JME_OK
        || jme_calc_ut(jd_ut, JME_BODY_MOON, JME_CALC_TRUE_POSITION | JME_CALC_XYZ | JME_CALC_EQUATORIAL, moon_eq_xyz, error) != JME_OK
        || jme_get_topo_pos(jd_ut, topo_pos, error) != JME_OK
        || jme_get_obliquity(jd_ut, JME_MODEL_OBL_IAU_1980, &eps, error) != JME_OK
        || jme_get_nutation(jd_ut, JME_MODEL_NUT_IAU_1980, &dpsi, &deps, error) != JME_OK) {
        return JME_ERR;
    }

    topo_state[0] = topo_pos[0];
    topo_state[1] = topo_pos[1];
    topo_state[2] = topo_pos[2];
    topo_state[3] = 0.0;
    topo_state[4] = 0.0;
    topo_state[5] = 0.0;
    jme_get_precession_matrix(2451545.0, jd_ut, JME_MODEL_PREC_IAU_1976, prec_mat);
    jme_matrix_transform_state(prec_mat, topo_state, topo_state);
    jme_get_nutation_matrix(dpsi * JME_EVENTS_DEG_TO_RAD, deps * JME_EVENTS_DEG_TO_RAD, eps * JME_EVENTS_DEG_TO_RAD, nut_mat);
    jme_matrix_transform_state(nut_mat, topo_state, topo_state);

    for (i = 0; i < 6; i++) {
        sun_topo_xyz[i] = sun_eq_xyz[i];
        moon_topo_xyz[i] = moon_eq_xyz[i];
    }
    for (i = 0; i < 3; i++) {
        sun_topo_xyz[i] -= topo_state[i];
        moon_topo_xyz[i] -= topo_state[i];
    }

    if (jme_rectangular_to_spherical_state(sun_topo_xyz, sun_eq) != JME_OK
        || jme_rectangular_to_spherical_state(moon_topo_xyz, moon_eq) != JME_OK) {
        jme_set_error(error, "Solar eclipse local geometry rectangular-to-spherical conversion failed");
        return JME_ERR;
    }

    sun_dist_km = jme_state_distance(sun_topo_xyz) * JME_AU_KM;
    moon_dist_km = jme_state_distance(moon_topo_xyz) * JME_AU_KM;

    if (separation_deg != 0) {
        *separation_deg = jme_spherical_angular_separation(sun_eq[0], sun_eq[1], moon_eq[0], moon_eq[1]);
    }
    if (sun_radius_deg != 0) {
        *sun_radius_deg = angular_radius_deg(JME_EVENTS_SUN_RADIUS_KM, sun_dist_km);
    }
    if (moon_radius_deg != 0) {
        *moon_radius_deg = angular_radius_deg(JME_EVENTS_MOON_RADIUS_KM, moon_dist_km);
    }
    if (sun_alt_deg != 0) {
        *sun_alt_deg = get_altitude(jd_ut, JME_BODY_SUN, 0, JME_CALC_TRUE_POSITION, geopos, 0.0, 0.0, error);
    }
    if (moon_alt_deg != 0) {
        *moon_alt_deg = get_altitude(jd_ut, JME_BODY_MOON, 0, JME_CALC_TRUE_POSITION, geopos, 0.0, 0.0, error);
    }

    return JME_OK;
}

static int classify_solar_eclipse(double sep_deg, double sun_r_deg, double moon_r_deg)
{
    if (!isfinite(sep_deg) || !isfinite(sun_r_deg) || !isfinite(moon_r_deg)) {
        return 0;
    }
    if (sep_deg > sun_r_deg + moon_r_deg) {
        return 0;
    }
    if (moon_r_deg >= sun_r_deg && sep_deg <= moon_r_deg - sun_r_deg) {
        return JME_ECLIPSE_SOLAR_TOTAL;
    }
    if (sun_r_deg > moon_r_deg && sep_deg <= sun_r_deg - moon_r_deg) {
        return JME_ECLIPSE_SOLAR_ANNULAR;
    }
    return JME_ECLIPSE_SOLAR_PARTIAL;
}

static double shadow_radius_km_at(double distance_from_moon_km, double source_radius_km, double occulter_radius_km, double source_to_occulter_distance_km, int exterior)
{
    if (!isfinite(distance_from_moon_km) || !isfinite(source_radius_km) || !isfinite(occulter_radius_km)
        || !isfinite(source_to_occulter_distance_km) || source_to_occulter_distance_km <= 0.0) {
        return NAN;
    }

    if (exterior) {
        return occulter_radius_km - distance_from_moon_km * (source_radius_km - occulter_radius_km) / source_to_occulter_distance_km;
    }

    return occulter_radius_km + distance_from_moon_km * (source_radius_km + occulter_radius_km) / source_to_occulter_distance_km;
}

static int solar_shadow_global_geometry(double jd_ut, int *rc_out, int *class_out, double *sub_lon, double *sub_lat, double *shadow_distance_km, char *error)
{
    double sun_xyz[6];
    double moon_xyz[6];
    double sun_vec[3];
    double moon_vec[3];
    double axis[3];
    double source_to_moon[3];
    double closest[3];
    double surface_point[3];
    double sun_moon_dist_km;
    double gamma_km;
    double s_closest_km;
    double umbra_closest_km;
    double penumbra_closest_km;
    double surface_offset_km;
    double entry_radius_km = NAN;
    double exit_radius_km = NAN;
    double half_chord_km = 0.0;
    double s_entry_km = 0.0;
    double point_norm;
    int rc = 0;
    int shadow_class = 0;
    int i;

    if (jme_calc_ut(jd_ut, JME_BODY_SUN, JME_CALC_TRUE_POSITION | JME_CALC_XYZ, sun_xyz, error) != JME_OK
        || jme_calc_ut(jd_ut, JME_BODY_MOON, JME_CALC_TRUE_POSITION | JME_CALC_XYZ, moon_xyz, error) != JME_OK) {
        return JME_ERR;
    }

    for (i = 0; i < 3; i++) {
        sun_vec[i] = sun_xyz[i] * JME_AU_KM;
        moon_vec[i] = moon_xyz[i] * JME_AU_KM;
        source_to_moon[i] = moon_vec[i] - sun_vec[i];
    }

    sun_moon_dist_km = vec_norm(source_to_moon);
    if (!isfinite(sun_moon_dist_km) || sun_moon_dist_km <= 0.0) {
        jme_set_error(error, "Solar shadow geometry requires valid Sun-Moon separation");
        return JME_ERR;
    }

    for (i = 0; i < 3; i++) {
        axis[i] = source_to_moon[i] / sun_moon_dist_km;
    }

    s_closest_km = -vec_dot(moon_vec, axis);
    for (i = 0; i < 3; i++) {
        closest[i] = moon_vec[i] + s_closest_km * axis[i];
    }

    gamma_km = vec_norm(closest);
    if (!isfinite(gamma_km)) {
        jme_set_error(error, "Solar shadow geometry produced invalid axis distance");
        return JME_ERR;
    }

    umbra_closest_km = shadow_radius_km_at(s_closest_km, JME_EVENTS_SUN_RADIUS_KM, JME_EVENTS_MOON_RADIUS_KM, sun_moon_dist_km, 1);
    penumbra_closest_km = shadow_radius_km_at(s_closest_km, JME_EVENTS_SUN_RADIUS_KM, JME_EVENTS_MOON_RADIUS_KM, sun_moon_dist_km, 0);
    surface_offset_km = gamma_km > JME_EVENTS_EARTH_RADIUS_KM ? (gamma_km - JME_EVENTS_EARTH_RADIUS_KM) : 0.0;

    if (gamma_km <= JME_EVENTS_EARTH_RADIUS_KM) {
        half_chord_km = sqrt(JME_EVENTS_EARTH_RADIUS_KM * JME_EVENTS_EARTH_RADIUS_KM - gamma_km * gamma_km);
        s_entry_km = s_closest_km - half_chord_km;
        entry_radius_km = shadow_radius_km_at(s_entry_km, JME_EVENTS_SUN_RADIUS_KM, JME_EVENTS_MOON_RADIUS_KM, sun_moon_dist_km, 1);
        exit_radius_km = shadow_radius_km_at(s_closest_km + half_chord_km, JME_EVENTS_SUN_RADIUS_KM, JME_EVENTS_MOON_RADIUS_KM, sun_moon_dist_km, 1);

        if ((entry_radius_km >= 0.0 && exit_radius_km < 0.0) || (entry_radius_km < 0.0 && exit_radius_km >= 0.0)) {
            rc = JME_ECLIPSE_SOLAR_HYBRID;
            shadow_class = JME_ECLIPSE_SOLAR_CENTRAL;
        } else if (entry_radius_km >= 0.0 && exit_radius_km >= 0.0) {
            rc = JME_ECLIPSE_SOLAR_TOTAL;
            shadow_class = JME_ECLIPSE_SOLAR_CENTRAL;
        } else if (entry_radius_km < 0.0 && exit_radius_km < 0.0) {
            rc = JME_ECLIPSE_SOLAR_ANNULAR;
            shadow_class = JME_ECLIPSE_SOLAR_CENTRAL;
        }

        for (i = 0; i < 3; i++) {
            surface_point[i] = moon_vec[i] + s_entry_km * axis[i];
        }
    } else if (isfinite(umbra_closest_km) && fabs(umbra_closest_km) >= surface_offset_km) {
        rc = umbra_closest_km >= 0.0 ? JME_ECLIPSE_SOLAR_TOTAL : JME_ECLIPSE_SOLAR_ANNULAR;
        shadow_class = JME_ECLIPSE_SOLAR_NONCENTRAL;
        for (i = 0; i < 3; i++) {
            surface_point[i] = closest[i] * (JME_EVENTS_EARTH_RADIUS_KM / gamma_km);
        }
    } else if (isfinite(penumbra_closest_km) && penumbra_closest_km >= surface_offset_km) {
        rc = JME_ECLIPSE_SOLAR_PARTIAL;
        for (i = 0; i < 3; i++) {
            surface_point[i] = closest[i] * (JME_EVENTS_EARTH_RADIUS_KM / gamma_km);
        }
    } else {
        rc = 0;
    }

    if (rc == 0) {
        if (rc_out != 0) {
            *rc_out = 0;
        }
        if (class_out != 0) {
            *class_out = 0;
        }
        return JME_OK;
    }

    point_norm = vec_norm(surface_point);
    if (!isfinite(point_norm) || point_norm <= 0.0) {
        jme_set_error(error, "Solar shadow surface point is invalid");
        return JME_ERR;
    }

    if (sub_lon != 0) {
        *sub_lon = normalize_lon_180(atan2(surface_point[1], surface_point[0]) * JME_EVENTS_RAD_TO_DEG - jme_sidereal_time(jd_ut) * 15.0);
    }
    if (sub_lat != 0) {
        *sub_lat = asin(surface_point[2] / point_norm) * JME_EVENTS_RAD_TO_DEG;
    }
    if (shadow_distance_km != 0) {
        *shadow_distance_km = surface_offset_km;
    }
    if (rc_out != 0) {
        *rc_out = rc;
    }
    if (class_out != 0) {
        *class_out = shadow_class;
    }

    return JME_OK;
}

static double solar_eclipse_global_metric(double jd_ut, char *error)
{
    double sep = 0.0;

    if (solar_eclipse_global_geometry(jd_ut, &sep, 0, 0, 0, 0, error) != JME_OK) {
        return NAN;
    }
    return sep;
}

static double solar_eclipse_local_metric(double jd_ut, double *geopos, char *error)
{
    double sep = 0.0;
    double sun_alt = -90.0;

    if (solar_eclipse_local_geometry(jd_ut, geopos, &sep, 0, 0, &sun_alt, 0, error) != JME_OK) {
        return NAN;
    }

    if (sun_alt <= 0.0) {
        return sep + 180.0 + fabs(sun_alt);
    }

    return sep;
}

static int refine_solar_maximum(double guess, double *geopos, int local_mode, double *tmax, char *error)
{
    double a = guess - 0.75;
    double b = guess + 0.75;
    double gr = 0.6180339887498948482;
    double x1 = b - gr * (b - a);
    double x2 = a + gr * (b - a);
    double f1 = local_mode ? solar_eclipse_local_metric(x1, geopos, error) : solar_eclipse_global_metric(x1, error);
    double f2 = local_mode ? solar_eclipse_local_metric(x2, geopos, error) : solar_eclipse_global_metric(x2, error);
    int i;

    if (!isfinite(f1) || !isfinite(f2)) {
        return JME_ERR;
    }

    for (i = 0; i < 64; i++) {
        if (f1 > f2) {
            a = x1;
            x1 = x2;
            f1 = f2;
            x2 = a + gr * (b - a);
            f2 = local_mode ? solar_eclipse_local_metric(x2, geopos, error) : solar_eclipse_global_metric(x2, error);
        } else {
            b = x2;
            x2 = x1;
            f2 = f1;
            x1 = b - gr * (b - a);
            f1 = local_mode ? solar_eclipse_local_metric(x1, geopos, error) : solar_eclipse_global_metric(x1, error);
        }

        if (!isfinite(f1) || !isfinite(f2)) {
            return JME_ERR;
        }

        if (fabs(b - a) < 1e-7) {
            break;
        }
    }

    *tmax = 0.5 * (a + b);
    return JME_OK;
}

static double event_body_radius_km(int body)
{
    switch (body) {
    case JME_BODY_SUN: return JME_EVENTS_SUN_RADIUS_KM;
    case JME_BODY_MOON: return JME_EVENTS_MOON_RADIUS_KM;
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

static int moon_target_state(double jd_ut, int body, const char *starname, int flags, int local_mode, double *geopos, double *moon_eq, double *target_eq, double *moon_radius_deg, double *target_radius_deg, double *moon_alt_deg, double *target_alt_deg, char *error)
{
    double moon_eq_xyz[6];
    double target_eq_xyz[6];
    double topo_pos[3];
    double topo_state[6];
    double prec_mat[9];
    double nut_mat[9];
    double eps;
    double dpsi;
    double deps;
    double moon_topo_xyz[6];
    double target_topo_xyz[6];
    double moon_dist_km;
    double target_dist_km;
    int i;

    if (local_mode && geopos == 0) {
        jme_set_error(error, "Occultation local geometry requires geopos");
        return JME_ERR;
    }
    (void)flags;

    if (jme_calc_ut(jd_ut, JME_BODY_MOON, JME_CALC_TRUE_POSITION | JME_CALC_XYZ | JME_CALC_EQUATORIAL, moon_eq_xyz, error) != JME_OK) {
        return JME_ERR;
    }

    if (starname != 0 && starname[0] != '\0') {
        if (jme_fixstar_ut(starname, jd_ut, JME_CALC_TRUE_POSITION | JME_CALC_XYZ | JME_CALC_EQUATORIAL, target_eq_xyz, error) != JME_OK) {
            return JME_ERR;
        }
    } else if (jme_calc_ut(jd_ut, body, JME_CALC_TRUE_POSITION | JME_CALC_XYZ | JME_CALC_EQUATORIAL, target_eq_xyz, error) != JME_OK) {
        return JME_ERR;
    }

    for (i = 0; i < 6; i++) {
        moon_topo_xyz[i] = moon_eq_xyz[i];
        target_topo_xyz[i] = target_eq_xyz[i];
    }

    if (local_mode) {
        jme_set_topo(geopos[0], geopos[1], geopos[2]);
        if (jme_get_topo_pos(jd_ut, topo_pos, error) != JME_OK
            || jme_get_obliquity(jd_ut, JME_MODEL_OBL_IAU_1980, &eps, error) != JME_OK
            || jme_get_nutation(jd_ut, JME_MODEL_NUT_IAU_1980, &dpsi, &deps, error) != JME_OK) {
            return JME_ERR;
        }

        topo_state[0] = topo_pos[0];
        topo_state[1] = topo_pos[1];
        topo_state[2] = topo_pos[2];
        topo_state[3] = 0.0;
        topo_state[4] = 0.0;
        topo_state[5] = 0.0;
        jme_get_precession_matrix(2451545.0, jd_ut, JME_MODEL_PREC_IAU_1976, prec_mat);
        jme_matrix_transform_state(prec_mat, topo_state, topo_state);
        jme_get_nutation_matrix(dpsi * JME_EVENTS_DEG_TO_RAD, deps * JME_EVENTS_DEG_TO_RAD, eps * JME_EVENTS_DEG_TO_RAD, nut_mat);
        jme_matrix_transform_state(nut_mat, topo_state, topo_state);

        for (i = 0; i < 3; i++) {
            moon_topo_xyz[i] -= topo_state[i];
            if (!(starname != 0 && starname[0] != '\0')) {
                target_topo_xyz[i] -= topo_state[i];
            }
        }
    }

    if (jme_rectangular_to_spherical_state(moon_topo_xyz, moon_eq) != JME_OK
        || jme_rectangular_to_spherical_state(target_topo_xyz, target_eq) != JME_OK) {
        jme_set_error(error, "Occultation geometry rectangular-to-spherical conversion failed");
        return JME_ERR;
    }

    moon_dist_km = jme_state_distance(moon_topo_xyz) * JME_AU_KM;
    if (moon_radius_deg != 0) {
        *moon_radius_deg = angular_radius_deg(JME_EVENTS_MOON_RADIUS_KM, moon_dist_km);
    }

    if (target_radius_deg != 0) {
        if (starname != 0 && starname[0] != '\0') {
            *target_radius_deg = 0.0;
        } else {
            target_dist_km = jme_state_distance(target_topo_xyz) * JME_AU_KM;
            *target_radius_deg = angular_radius_deg(event_body_radius_km(body), target_dist_km);
        }
    }

    if (moon_alt_deg != 0) {
        *moon_alt_deg = local_mode ? get_altitude(jd_ut, JME_BODY_MOON, 0, JME_CALC_TRUE_POSITION, geopos, 0.0, 0.0, error) : NAN;
    }
    if (target_alt_deg != 0) {
        *target_alt_deg = local_mode
            ? get_altitude(jd_ut, body, starname, JME_CALC_TRUE_POSITION, geopos, 0.0, 0.0, error)
            : NAN;
    }

    return JME_OK;
}

static double occultation_metric(double jd_ut, int body, const char *starname, double *geopos, int local_mode, char *error)
{
    double moon_eq[6];
    double target_eq[6];
    double moon_r = 0.0;
    double target_r = 0.0;
    double moon_alt = -90.0;
    double target_alt = -90.0;
    double sep;

    if (moon_target_state(jd_ut, body, starname, JME_CALC_TRUE_POSITION, local_mode, geopos, moon_eq, target_eq, &moon_r, &target_r, &moon_alt, &target_alt, error) != JME_OK) {
        return NAN;
    }

    sep = jme_spherical_angular_separation(moon_eq[0], moon_eq[1], target_eq[0], target_eq[1]);
    if (local_mode && (moon_alt <= 0.0 || target_alt <= 0.0)) {
        return sep + 180.0 + fabs(moon_alt < target_alt ? moon_alt : target_alt);
    }

    return sep;
}

static int __attribute__((unused)) find_moon_target_conjunction(double jd_start, int body, const char *starname, int backward, double *tret, char *error)
{
    double direction = backward ? -1.0 : 1.0;
    double t0 = jd_start;
    double prev_diff;
    int i;

    {
        double moon_state[6];
        double target_state[6];

        if (jme_calc_ut(t0, JME_BODY_MOON, JME_CALC_TRUE_POSITION, moon_state, error) != JME_OK) {
            return JME_ERR;
        }
        if (starname != 0 && starname[0] != '\0') {
            if (jme_fixstar_ut(starname, t0, JME_CALC_TRUE_POSITION, target_state, error) != JME_OK) {
                return JME_ERR;
            }
        } else if (jme_calc_ut(t0, body, JME_CALC_TRUE_POSITION, target_state, error) != JME_OK) {
            return JME_ERR;
        }
        prev_diff = jme_degrees_difference_signed(moon_state[0], target_state[0]);
    }

    for (i = 1; i <= 240; i++) {
        double t1 = jd_start + direction * (0.25 * i);
        double moon_state[6];
        double target_state[6];
        double diff;
        int j;

        if (jme_calc_ut(t1, JME_BODY_MOON, JME_CALC_TRUE_POSITION, moon_state, error) != JME_OK) {
            return JME_ERR;
        }
        if (starname != 0 && starname[0] != '\0') {
            if (jme_fixstar_ut(starname, t1, JME_CALC_TRUE_POSITION, target_state, error) != JME_OK) {
                return JME_ERR;
            }
        } else if (jme_calc_ut(t1, body, JME_CALC_TRUE_POSITION, target_state, error) != JME_OK) {
            return JME_ERR;
        }
        diff = jme_degrees_difference_signed(moon_state[0], target_state[0]);

        if ((prev_diff <= 0.0 && diff >= 0.0) || (prev_diff >= 0.0 && diff <= 0.0)) {
            double a = t0;
            double b = t1;
            double fa = prev_diff;

            for (j = 0; j < 48; j++) {
                double m = 0.5 * (a + b);
                double moon_mid[6];
                double target_mid[6];
                double fm;

                if (jme_calc_ut(m, JME_BODY_MOON, JME_CALC_TRUE_POSITION, moon_mid, error) != JME_OK) {
                    return JME_ERR;
                }
                if (starname != 0 && starname[0] != '\0') {
                    if (jme_fixstar_ut(starname, m, JME_CALC_TRUE_POSITION, target_mid, error) != JME_OK) {
                        return JME_ERR;
                    }
                } else if (jme_calc_ut(m, body, JME_CALC_TRUE_POSITION, target_mid, error) != JME_OK) {
                    return JME_ERR;
                }
                fm = jme_degrees_difference_signed(moon_mid[0], target_mid[0]);

                if (fabs(fm) < 1e-8 || fabs(b - a) < 1e-8) {
                    *tret = m;
                    return JME_OK;
                }

                if ((fa <= 0.0 && fm >= 0.0) || (fa >= 0.0 && fm <= 0.0)) {
                    b = m;
                } else {
                    a = m;
                    fa = fm;
                }
            }

            *tret = 0.5 * (a + b);
            return JME_OK;
        }

        t0 = t1;
        prev_diff = diff;
    }

    jme_set_error(error, "Moon-target conjunction not found in search window");
    return JME_ERR;
}

static int __attribute__((unused)) refine_occultation_maximum(double guess, int body, const char *starname, double *geopos, int local_mode, double *tmax, char *error)
{
    double a = guess - 0.75;
    double b = guess + 0.75;
    double gr = 0.6180339887498948482;
    double x1 = b - gr * (b - a);
    double x2 = a + gr * (b - a);
    double f1 = occultation_metric(x1, body, starname, geopos, local_mode, error);
    double f2 = occultation_metric(x2, body, starname, geopos, local_mode, error);
    int i;

    if (!isfinite(f1) || !isfinite(f2)) {
        return JME_ERR;
    }

    for (i = 0; i < 64; i++) {
        if (f1 > f2) {
            a = x1;
            x1 = x2;
            f1 = f2;
            x2 = a + gr * (b - a);
            f2 = occultation_metric(x2, body, starname, geopos, local_mode, error);
        } else {
            b = x2;
            x2 = x1;
            f2 = f1;
            x1 = b - gr * (b - a);
            f1 = occultation_metric(x1, body, starname, geopos, local_mode, error);
        }
        if (!isfinite(f1) || !isfinite(f2)) {
            return JME_ERR;
        }
    }

    *tmax = 0.5 * (a + b);
    return JME_OK;
}

static int __attribute__((unused)) find_occultation_contact_time(double tmid, int body, const char *starname, double threshold_deg, double *geopos, int local_mode, int left_side, double *tret, char *error)
{
    double t_prev = tmid;
    double f_prev = occultation_metric(tmid, body, starname, geopos, local_mode, error) - threshold_deg;
    double direction = left_side ? -1.0 : 1.0;
    int i;

    if (!isfinite(f_prev)) {
        return JME_ERR;
    }
    if (f_prev > 0.0) {
        return JME_ERR;
    }

    for (i = 1; i <= 96; i++) {
        double t = tmid + direction * (i / 96.0);
        double f = occultation_metric(t, body, starname, geopos, local_mode, error) - threshold_deg;
        int j;

        if (!isfinite(f)) {
            return JME_ERR;
        }

        if (f >= 0.0) {
            double a = left_side ? t : t_prev;
            double b = left_side ? t_prev : t;

            for (j = 0; j < 48; j++) {
                double m = 0.5 * (a + b);
                double fm = occultation_metric(m, body, starname, geopos, local_mode, error) - threshold_deg;

                if (!isfinite(fm)) {
                    return JME_ERR;
                }
                if (fabs(fm) < 1e-8 || fabs(b - a) < 1e-8) {
                    *tret = m;
                    return JME_OK;
                }

                if ((occultation_metric(a, body, starname, geopos, local_mode, error) - threshold_deg) * fm <= 0.0) {
                    b = m;
                } else {
                    a = m;
                }
            }

            *tret = 0.5 * (a + b);
            return JME_OK;
        }

        t_prev = t;
        f_prev = f;
    }

    return JME_ERR;
}

static int find_solar_contact_time(double tmid, double threshold_deg, double *geopos, int local_mode, int left_side, double *tret, char *error)
{
    double step = 0.02;
    double t_prev = tmid;
    double f_prev;
    int i;

    f_prev = (local_mode ? solar_eclipse_local_metric(t_prev, geopos, error) : solar_eclipse_global_metric(t_prev, error)) - threshold_deg;
    if (!isfinite(f_prev)) {
        return JME_ERR;
    }

    for (i = 1; i <= 100; i++) {
        double t = tmid + (left_side ? -1.0 : 1.0) * step * i;
        double f = (local_mode ? solar_eclipse_local_metric(t, geopos, error) : solar_eclipse_global_metric(t, error)) - threshold_deg;
        int j;

        if (!isfinite(f)) {
            return JME_ERR;
        }

        if ((f_prev <= 0.0 && f >= 0.0) || (f_prev >= 0.0 && f <= 0.0)) {
            double a = left_side ? t : t_prev;
            double b = left_side ? t_prev : t;
            double fa = left_side ? f : f_prev;

            for (j = 0; j < 48; j++) {
                double m = 0.5 * (a + b);
                double fm = (local_mode ? solar_eclipse_local_metric(m, geopos, error) : solar_eclipse_global_metric(m, error)) - threshold_deg;

                if (!isfinite(fm)) {
                    return JME_ERR;
                }
                if (fabs(fm) < 1e-6 || fabs(b - a) < 1e-8) {
                    *tret = m;
                    return JME_OK;
                }
                if ((fa <= 0.0 && fm >= 0.0) || (fa >= 0.0 && fm <= 0.0)) {
                    b = m;
                } else {
                    a = m;
                    fa = fm;
                }
            }

            *tret = 0.5 * (a + b);
            return JME_OK;
        }

        t_prev = t;
        f_prev = f;
    }

    return JME_ERR;
}

static double get_altitude(double jd_ut, int body, const char *starname, int flags, double *geopos, double atpress, double attemp, char *error)
{
    double results[6];
    double ra, dec, hour_angle, azimuth, altitude;
    double eps, gst;
    int equatorial_flag = (int)JME_CALC_EQUATORIAL;
    int res;

    jme_set_topo(geopos[0], geopos[1], geopos[2]);

    if (starname != 0 && starname[0] != '\0') {
        flags = flags | (int)JME_CALC_TOPOCENTRIC | equatorial_flag;
        res = jme_fixstar_ut(starname, jd_ut, flags, results, error);
    } else {
        double body_eq_xyz[6];
        double topo_pos[3];
        double topo_state[6];
        double prec_mat[9];
        double nut_mat[9];
        double dpsi;
        double deps;
        int i;

        flags = flags | equatorial_flag | (int)JME_CALC_XYZ;
        res = jme_calc_ut(jd_ut, body, flags, body_eq_xyz, error);
        if (res == JME_OK) {
            res = jme_get_topo_pos(jd_ut, topo_pos, error);
        }
        if (res != JME_OK) { return -100.0; }

        topo_state[0] = topo_pos[0];
        topo_state[1] = topo_pos[1];
        topo_state[2] = topo_pos[2];
        topo_state[3] = 0.0;
        topo_state[4] = 0.0;
        topo_state[5] = 0.0;

        if (jme_get_obliquity(jd_ut, JME_MODEL_OBL_IAU_1980, &eps, error) != JME_OK
            || jme_get_nutation(jd_ut, JME_MODEL_NUT_IAU_1980, &dpsi, &deps, error) != JME_OK) {
            return -100.0;
        }

        jme_get_precession_matrix(2451545.0, jd_ut, JME_MODEL_PREC_IAU_1976, prec_mat);
        jme_matrix_transform_state(prec_mat, topo_state, topo_state);
        jme_get_nutation_matrix(dpsi * JME_EVENTS_DEG_TO_RAD, deps * JME_EVENTS_DEG_TO_RAD, eps * JME_EVENTS_DEG_TO_RAD, nut_mat);
        jme_matrix_transform_state(nut_mat, topo_state, topo_state);

        for (i = 0; i < 6; i++) {
            results[i] = body_eq_xyz[i];
        }
        for (i = 0; i < 3; i++) {
            results[i] -= topo_state[i];
        }
        if (jme_rectangular_to_spherical_state(results, results) != JME_OK) {
            jme_set_error(error, "Altitude conversion failed");
            return -100.0;
        }
    }

    if (res != JME_OK) { return -100.0; }

    ra = results[0];
    dec = results[1];

    jme_get_obliquity(jd_ut, JME_MODEL_OBL_IAU_1980, &eps, 0);
    gst = jme_sidereal_time(jd_ut);
    hour_angle = jme_degree_normalize(gst * 15.0 + geopos[0] - ra);

    jme_equatorial_to_horizontal(hour_angle, dec, geopos[1], &azimuth, &altitude);

    if (atpress > 0.0) {
        altitude = jme_refract(altitude, atpress, attemp, JME_COORD_TRUE_TO_APPARENT);
    }

    return altitude;
}

static int refine_altitude_crossing(
    double t0,
    double t1,
    double h0,
    int body,
    const char *starname,
    int flags,
    double *geopos,
    double atpress,
    double attemp,
    double *tret,
    char *error
)
{
    double a0;
    double a1;
    double mid;
    double amid;
    int i;

    a0 = get_altitude(t0, body, starname, flags, geopos, atpress, attemp, error) - h0;
    a1 = get_altitude(t1, body, starname, flags, geopos, atpress, attemp, error) - h0;

    if (a0 == -100.0 - h0 || a1 == -100.0 - h0 || a0 * a1 > 0.0) {
        return JME_ERR;
    }

    for (i = 0; i < 32; i++) {
        mid = (t0 + t1) / 2.0;
        amid = get_altitude(mid, body, starname, flags, geopos, atpress, attemp, error) - h0;

        if (fabs(amid) < 1e-7 || fabs(t1 - t0) < 1e-8) {
            *tret = mid;
            return JME_OK;
        }

        if (a0 * amid <= 0.0) {
            t1 = mid;
            a1 = amid;
        } else {
            t0 = mid;
            a0 = amid;
        }
    }

    (void)a1;
    *tret = (t0 + t1) / 2.0;
    return JME_OK;
}

static int refine_meridian_transit(
    double t0,
    double t1,
    int body,
    const char *starname,
    int flags,
    double *geopos,
    double target_hour_angle,
    double *tret,
    char *error
)
{
    double ha0;
    double ha1;
    int i;

    for (i = 0; i < 32; i++) {
        double tm = (t0 + t1) / 2.0;
        double results[6];
        double ra;
        double ham;

        if (starname != 0 && starname[0] != '\0') {
            if (jme_fixstar_ut(starname, tm, flags | JME_CALC_EQUATORIAL, results, error) != JME_OK) {
                return JME_ERR;
            }
        } else if (jme_calc_ut(tm, body, flags | JME_CALC_EQUATORIAL, results, error) != JME_OK) {
            return JME_ERR;
        }

        ra = results[0];
        ham = jme_degrees_difference_signed(jme_degrees_difference_signed(jme_sidereal_time(tm) * 15.0 + geopos[0], ra), target_hour_angle);

        if (fabs(ham) < 1e-5 || fabs(t1 - t0) < 1e-8) {
            *tret = tm;
            return JME_OK;
        }

        if (starname != 0 && starname[0] != '\0') {
            if (jme_fixstar_ut(starname, t0, flags | JME_CALC_EQUATORIAL, results, error) != JME_OK) {
                return JME_ERR;
            }
        } else if (jme_calc_ut(t0, body, flags | JME_CALC_EQUATORIAL, results, error) != JME_OK) {
            return JME_ERR;
        }
        ha0 = jme_degrees_difference_signed(jme_degrees_difference_signed(jme_sidereal_time(t0) * 15.0 + geopos[0], results[0]), target_hour_angle);

        if (starname != 0 && starname[0] != '\0') {
            if (jme_fixstar_ut(starname, t1, flags | JME_CALC_EQUATORIAL, results, error) != JME_OK) {
                return JME_ERR;
            }
        } else if (jme_calc_ut(t1, body, flags | JME_CALC_EQUATORIAL, results, error) != JME_OK) {
            return JME_ERR;
        }
        ha1 = jme_degrees_difference_signed(jme_degrees_difference_signed(jme_sidereal_time(t1) * 15.0 + geopos[0], results[0]), target_hour_angle);

        if (ha0 * ham <= 0.0) {
            t1 = tm;
        } else if (ha1 * ham <= 0.0) {
            t0 = tm;
        } else {
            *tret = tm;
            return JME_OK;
        }
    }

    *tret = (t0 + t1) / 2.0;
    return JME_OK;
}

static int rise_trans_search(double jd_ut, int body, const char *starname, int flags, int rsmi, double *geopos, double atpress, double attemp, int use_true_horizon, double true_horizon, double *tret, char *error)
{
    double t = jd_ut;
    double h0 = -0.5667; /* Standard for stars (refraction only) */
    double search_atpress = atpress;
    double alt, alt_next;
    int i;

    if (tret != 0) { *tret = 0.0; }
    if (geopos == 0 || tret == 0) {
        jme_set_error(error, "Rise and transit output arguments are required");
        return JME_ERR;
    }

    if (body == JME_BODY_SUN || body == JME_BODY_MOON) {
        h0 = -0.8333; /* Includes semi-diameter */
    }
    if (rsmi & JME_RISE_NO_REFRACTION) {
        search_atpress = 0.0;
        h0 = 0.0;
    }
    if (rsmi & JME_RISE_DISC_CENTER) {
        h0 = (rsmi & JME_RISE_NO_REFRACTION) ? 0.0 : -0.5667;
    }
    if (rsmi & JME_RISE_CIVIL_TWILIGHT) {
        h0 = -6.0;
    } else if (rsmi & JME_RISE_NAUTICAL_TWILIGHT) {
        h0 = -12.0;
    } else if (rsmi & JME_RISE_ASTRONOMICAL_TWILIGHT) {
        h0 = -18.0;
    }
    if (use_true_horizon) {
        h0 = true_horizon;
    }

    if (rsmi & (JME_RISE_MERIDIAN_TRANSIT | JME_RISE_ANTI_MERIDIAN_TRANSIT)) {
        double target_hour_angle = (rsmi & JME_RISE_ANTI_MERIDIAN_TRANSIT) ? 180.0 : 0.0;
        /* Search for transit from the supplied Julian day across one day. */
        for (i = 0; i < 240; i++) {
            double results[6];
            double ra, gst, ha, ha_next;
            if (starname != 0 && starname[0] != '\0') {
                if (jme_fixstar_ut(starname, t, flags | JME_CALC_EQUATORIAL, results, error) != JME_OK) {
                    return JME_ERR;
                }
            } else {
                if (jme_calc_ut(t, body, flags | JME_CALC_EQUATORIAL, results, error) != JME_OK) {
                    return JME_ERR;
                }
            }
            ra = results[0];
            gst = jme_sidereal_time(t);
            ha = jme_degrees_difference_signed(jme_degrees_difference_signed(gst * 15.0 + geopos[0], ra), target_hour_angle);
            
            t += 0.01; /* 14.4 mins */
            gst = jme_sidereal_time(t);
            if (starname != 0 && starname[0] != '\0') {
                if (jme_fixstar_ut(starname, t, flags | JME_CALC_EQUATORIAL, results, error) != JME_OK) {
                    return JME_ERR;
                }
            } else {
                if (jme_calc_ut(t, body, flags | JME_CALC_EQUATORIAL, results, error) != JME_OK) {
                    return JME_ERR;
                }
            }
            ra = results[0];
            ha_next = jme_degrees_difference_signed(jme_degrees_difference_signed(gst * 15.0 + geopos[0], ra), target_hour_angle);

            if (ha < 0 && ha_next >= 0) {
                return refine_meridian_transit(t - 0.01, t, body, starname, flags, geopos, target_hour_angle, tret, error);
            }
        }
    }

    /* Rise/set search across one day. */
    for (i = 0; i < 24; i++) {
        alt = get_altitude(t, body, starname, flags, geopos, search_atpress, attemp, error);
        t += 1.0 / 24.0;
        alt_next = get_altitude(t, body, starname, flags, geopos, search_atpress, attemp, error);

        if (rsmi & JME_RISE_RISE) {
            if (alt < h0 && alt_next >= h0) {
                return refine_altitude_crossing(t - 1.0 / 24.0, t, h0, body, starname, flags, geopos, search_atpress, attemp, tret, error);
            }
        } else if (rsmi & JME_RISE_SET) {
            if (alt > h0 && alt_next <= h0) {
                return refine_altitude_crossing(t - 1.0 / 24.0, t, h0, body, starname, flags, geopos, search_atpress, attemp, tret, error);
            }
        }
    }

    jme_set_error(error, "Event not found in 24h search window");
    return JME_ERR;
}

int jme_rise_trans(double jd_ut, int body, const char *starname, int flags, int rsmi, double *geopos, double atpress, double attemp, double *tret, char *error)
{
    return rise_trans_search(jd_ut, body, starname, flags, rsmi, geopos, atpress, attemp, 0, 0.0, tret, error);
}

static int longitude_difference_at(double jd_ut, int body, int flags, int heliocentric, double x2cross, double *value, char *error)
{
    double results[6];

    if (value == 0) {
        jme_set_error(error, "Longitude crossing work value is required");
        return JME_ERR;
    }

    if (jme_calc_ut(jd_ut, body, flags | (heliocentric ? JME_CALC_HELIOCENTRIC : 0), results, error) != JME_OK) {
        return JME_ERR;
    }

    *value = jme_degrees_difference_signed(results[0], x2cross);
    return JME_OK;
}

static int refine_longitude_crossing(double t0, double t1, int body, int flags, int heliocentric, double x2cross, double *tret, char *error)
{
    double v0;
    double v1;
    int i;

    if (longitude_difference_at(t0, body, flags, heliocentric, x2cross, &v0, error) != JME_OK
        || longitude_difference_at(t1, body, flags, heliocentric, x2cross, &v1, error) != JME_OK
        || v0 * v1 > 0.0) {
        return JME_ERR;
    }

    for (i = 0; i < 48; i++) {
        double tm = (t0 + t1) / 2.0;
        double vm;

        if (longitude_difference_at(tm, body, flags, heliocentric, x2cross, &vm, error) != JME_OK) {
            return JME_ERR;
        }
        if (fabs(vm) < 1e-10 || fabs(t1 - t0) < 1e-10) {
            *tret = tm;
            return JME_OK;
        }
        if (v0 * vm <= 0.0) {
            t1 = tm;
            v1 = vm;
        } else {
            t0 = tm;
            v0 = vm;
        }
    }

    (void)v1;
    *tret = (t0 + t1) / 2.0;
    return JME_OK;
}

int jme_solcross(double x2cross, double jd_ut, int flags, double *tret, char *error)
{
    double t = jd_ut;
    double lon, lon_next;
    double results[6];
    int i;

    if (tret == 0) {
        jme_set_error(error, "Solar crossing output is required");
        return JME_ERR;
    }

    for (i = 0; i < 365 * 2; i++) {
        if (jme_calc_ut(t, JME_BODY_SUN, flags, results, error) != JME_OK) {
            return JME_ERR;
        }
        lon = results[0];
        t += 0.5;
        if (jme_calc_ut(t, JME_BODY_SUN, flags, results, error) != JME_OK) {
            return JME_ERR;
        }
        lon_next = results[0];

        if (jme_degrees_difference_signed(lon_next, x2cross) >= 0 && jme_degrees_difference_signed(lon, x2cross) < 0) {
            return refine_longitude_crossing(t - 0.5, t, JME_BODY_SUN, flags, 0, x2cross, tret, error);
        }
    }

    return JME_ERR;
}

int jme_mooncross(double x2cross, double jd_ut, int flags, double *tret, char *error)
{
    double t = jd_ut;
    double lon, lon_next;
    double results[6];
    int i;

    if (tret == 0) {
        jme_set_error(error, "Lunar crossing output is required");
        return JME_ERR;
    }

    for (i = 0; i < 30 * 2; i++) {
        if (jme_calc_ut(t, JME_BODY_MOON, flags, results, error) != JME_OK) {
            return JME_ERR;
        }
        lon = results[0];
        t += 0.5;
        if (jme_calc_ut(t, JME_BODY_MOON, flags, results, error) != JME_OK) {
            return JME_ERR;
        }
        lon_next = results[0];

        if (jme_degrees_difference_signed(lon_next, x2cross) >= 0 && jme_degrees_difference_signed(lon, x2cross) < 0) {
            return refine_longitude_crossing(t - 0.5, t, JME_BODY_MOON, flags, 0, x2cross, tret, error);
        }
    }

    return JME_ERR;
}

int jme_sol_eclipse_when_loc(double jd_start, int flags, double *geopos, double *tret, double *attr, int backward, char *error)
{
    double guess = 0.0;
    double tmax = 0.0;
    double sep = 0.0;
    double sun_r = 0.0;
    double moon_r = 0.0;
    double sun_alt = -90.0;
    double moon_alt = -90.0;
    int rc;
    int i;
    (void)flags;

    if (geopos == 0 || tret == 0 || attr == 0) {
        jme_set_error(error, "Solar eclipse local search requires geopos, tret, and attr outputs");
        return JME_ERR;
    }

    if (find_new_moon_conjunction(jd_start, backward, &guess, error) != JME_OK) {
        return JME_ERR;
    }
    if (refine_solar_maximum(guess, geopos, 1, &tmax, error) != JME_OK) {
        return JME_ERR;
    }
    if (solar_eclipse_local_geometry(tmax, geopos, &sep, &sun_r, &moon_r, &sun_alt, &moon_alt, error) != JME_OK) {
        return JME_ERR;
    }

    rc = classify_solar_eclipse(sep, sun_r, moon_r);
    if (rc == 0 || sun_alt <= 0.0) {
        jme_set_error(error, "No local solar eclipse found near the supplied date");
        return JME_ERR;
    }

    memset(tret, 0, sizeof(double) * 10);
    for (i = 0; i < 20; i++) {
        attr[i] = 0.0;
    }

    tret[0] = tmax;
    tret[1] = tmax;
    find_solar_contact_time(tmax, sun_r + moon_r, geopos, 1, 1, &tret[2], error);
    find_solar_contact_time(tmax, sun_r + moon_r, geopos, 1, 0, &tret[3], error);
    if (rc == JME_ECLIPSE_SOLAR_TOTAL || rc == JME_ECLIPSE_SOLAR_ANNULAR || rc == JME_ECLIPSE_SOLAR_HYBRID) {
        double inner = fabs(sun_r - moon_r);
        find_solar_contact_time(tmax, inner, geopos, 1, 1, &tret[4], error);
        find_solar_contact_time(tmax, inner, geopos, 1, 0, &tret[5], error);
    }

    attr[0] = (sun_r + moon_r - sep) / (2.0 * sun_r);
    attr[1] = rc == JME_ECLIPSE_SOLAR_TOTAL ? 1.0 : ((rc == JME_ECLIPSE_SOLAR_ANNULAR || rc == JME_ECLIPSE_SOLAR_HYBRID) ? (moon_r * moon_r) / (sun_r * sun_r) : attr[0]);
    attr[2] = sep;
    attr[3] = 2.0 * sun_r * 3600.0;
    attr[4] = 2.0 * moon_r * 3600.0;
    attr[5] = sun_alt;
    attr[6] = moon_alt;
    attr[7] = sep <= fabs(sun_r - moon_r) ? 1.0 : 0.0;
    attr[8] = JME_ECLIPSE_VISIBLE;

    return rc;
}

int jme_sol_eclipse_where(double jd_ut, int flags, double *geopos, double *attr, char *error)
{
    double sep = 0.0;
    double sun_r = 0.0;
    double moon_r = 0.0;
    double sub_lon = 0.0;
    double sub_lat = 0.0;
    double shadow_distance_km = NAN;
    int shadow_class = 0;
    int rc;
    int i;
    (void)flags;

    if (geopos == 0 || attr == 0) {
        jme_set_error(error, "Solar eclipse geographic circumstance output is required");
        return JME_ERR;
    }

    if (solar_eclipse_global_geometry(jd_ut, &sep, &sun_r, &moon_r, 0, 0, error) != JME_OK
        || solar_shadow_global_geometry(jd_ut, &rc, &shadow_class, &sub_lon, &sub_lat, &shadow_distance_km, error) != JME_OK) {
        return JME_ERR;
    }

    if (rc == 0) {
        jme_set_error(error, "No solar eclipse is in progress at the supplied time");
        return JME_ERR;
    }

    geopos[0] = sub_lon;
    geopos[1] = sub_lat;
    geopos[2] = 0.0;

    for (i = 0; i < 20; i++) {
        attr[i] = 0.0;
    }
    attr[0] = (sun_r + moon_r - sep) / (2.0 * sun_r);
    attr[1] = rc == JME_ECLIPSE_SOLAR_TOTAL ? 1.0 : ((rc == JME_ECLIPSE_SOLAR_ANNULAR || rc == JME_ECLIPSE_SOLAR_HYBRID) ? (moon_r * moon_r) / (sun_r * sun_r) : attr[0]);
    attr[2] = sep;
    attr[3] = 2.0 * sun_r * 3600.0;
    attr[4] = 2.0 * moon_r * 3600.0;
    attr[7] = isfinite(shadow_distance_km) ? shadow_distance_km : 0.0;
    attr[8] = (double)shadow_class;

    return rc;
}

int jme_sol_eclipse_when_glob(double jd_start, int flags, int epheflag, double *tret, int backward, char *error)
{
    double guess = 0.0;
    double tmax = 0.0;
    double sep = 0.0;
    double sun_r = 0.0;
    double moon_r = 0.0;
    double shadow_distance_km = NAN;
    int rc;
    (void)epheflag;
    (void)flags;

    if (tret == 0) {
        jme_set_error(error, "Solar eclipse time output is required");
        return JME_ERR;
    }

    if (find_new_moon_conjunction(jd_start, backward, &guess, error) != JME_OK) {
        return JME_ERR;
    }
    if (refine_solar_maximum(guess, 0, 0, &tmax, error) != JME_OK) {
        return JME_ERR;
    }
    if (solar_eclipse_global_geometry(tmax, &sep, &sun_r, &moon_r, 0, 0, error) != JME_OK
        || solar_shadow_global_geometry(tmax, &rc, 0, 0, 0, &shadow_distance_km, error) != JME_OK) {
        return JME_ERR;
    }

    if (rc == 0) {
        jme_set_error(error, "No solar eclipse found near the supplied date");
        return JME_ERR;
    }

    memset(tret, 0, sizeof(double) * 10);
    tret[0] = tmax;
    tret[1] = tmax;
    find_solar_contact_time(tmax, sun_r + moon_r, 0, 0, 1, &tret[2], error);
    find_solar_contact_time(tmax, sun_r + moon_r, 0, 0, 0, &tret[3], error);
    if (rc == JME_ECLIPSE_SOLAR_TOTAL || rc == JME_ECLIPSE_SOLAR_ANNULAR || rc == JME_ECLIPSE_SOLAR_HYBRID) {
        double inner = fabs(sun_r - moon_r);
        find_solar_contact_time(tmax, inner, 0, 0, 1, &tret[4], error);
        find_solar_contact_time(tmax, inner, 0, 0, 0, &tret[5], error);
    }

    return rc;
}

int jme_lun_eclipse_when(double jd_start, int flags, int iflag, double *tret, int backward, char *error)
{
    double guess = 0.0;
    double tmax = 0.0;
    double umbral_mag = 0.0;
    double penumbral_mag = 0.0;
    double umbra_km = 0.0;
    double penumbra_km = 0.0;
    double offset_km = 0.0;
    double axis_km = 0.0;
    int eclipse_type;
    (void)flags;
    (void)iflag;

    if (tret == 0) {
        jme_set_error(error, "Lunar eclipse time output is required");
        return JME_ERR;
    }

    if (find_full_moon_opposition(jd_start, backward, &guess, error) != JME_OK) {
        return JME_ERR;
    }
    if (refine_lunar_eclipse_maximum(guess, &tmax, error) != JME_OK) {
        return JME_ERR;
    }
    if (lunar_eclipse_geometry(tmax, &umbral_mag, &penumbral_mag, &umbra_km, &penumbra_km, &offset_km, &axis_km, error) != JME_OK) {
        return JME_ERR;
    }

    eclipse_type = classify_lunar_eclipse(umbral_mag, penumbral_mag);
    if (eclipse_type == 0 || axis_km <= 0.0) {
        jme_set_error(error, "No lunar eclipse found near the supplied date");
        return JME_ERR;
    }

    memset(tret, 0, sizeof(double) * 10);
    tret[0] = tmax;
    tret[1] = tmax;

    if (penumbral_mag > 0.0) {
        find_contact_time(tmax, penumbra_km + JME_EVENTS_MOON_RADIUS_KM, 1, &tret[2], error);
        find_contact_time(tmax, penumbra_km + JME_EVENTS_MOON_RADIUS_KM, 0, &tret[3], error);
    }
    if (umbral_mag > 0.0) {
        find_contact_time(tmax, umbra_km + JME_EVENTS_MOON_RADIUS_KM, 1, &tret[4], error);
        find_contact_time(tmax, umbra_km + JME_EVENTS_MOON_RADIUS_KM, 0, &tret[5], error);
    }
    if (umbral_mag >= 1.0) {
        find_contact_time(tmax, umbra_km - JME_EVENTS_MOON_RADIUS_KM, 1, &tret[6], error);
        find_contact_time(tmax, umbra_km - JME_EVENTS_MOON_RADIUS_KM, 0, &tret[7], error);
    }

    return eclipse_type;
}

int jme_lun_occult_where(double jd_ut, int body, const char *starname, int flags, double *geopos, double *attr, char *error)
{
    (void)jd_ut; (void)body; (void)starname; (void)flags; (void)geopos; (void)attr;
    jme_set_error(error, "Exact lunar occultation geographic circumstances require a validated algorithm");
    return JME_ERR;
}

int jme_lun_occult_when_loc(double jd_start, int body, const char *starname, int flags, double *geopos, double *tret, double *attr, int backward, char *error)
{
    (void)jd_start; (void)body; (void)starname; (void)flags; (void)geopos; (void)tret; (void)attr; (void)backward;
    jme_set_error(error, "Exact lunar occultation local circumstances require a validated algorithm");
    return JME_ERR;
}

int jme_lun_occult_when_glob(double jd_start, int body, const char *starname, int flags, int iflag, double *tret, int backward, char *error)
{
    (void)jd_start; (void)body; (void)starname; (void)flags; (void)iflag; (void)tret; (void)backward;
    jme_set_error(error, "Exact lunar occultation global search requires a validated algorithm");
    return JME_ERR;
}

int jme_rise_trans_true_hor(double jd_ut, int body, const char *starname, int flags, int rsmi, double *geopos, double atpress, double attemp, double horhgt, double *tret, char *error)
{
    return rise_trans_search(jd_ut, body, starname, flags, rsmi, geopos, atpress, attemp, 1, horhgt, tret, error);
}

int jme_solcross_ut(double x2cross, double jd_ut, int flags, double *tret, char *error)
{
    return jme_solcross(x2cross, jd_ut, flags, tret, error);
}

int jme_mooncross_ut(double x2cross, double jd_ut, int flags, double *tret, char *error)
{
    return jme_mooncross(x2cross, jd_ut, flags, tret, error);
}

int jme_mooncross_node(double jd_ut, int flags, double *tret, char *error)
{
    double t = jd_ut;
    double pos[6];
    double lat;
    double next_lat;
    int i;

    if (tret == 0) {
        jme_set_error(error, "Lunar node crossing output is required");
        return JME_ERR;
    }

    if (jme_calc_ut(t, JME_BODY_MOON, flags, pos, error) != JME_OK) {
        return JME_ERR;
    }
    lat = pos[1];

    for (i = 0; i < 60; i++) {
        t += 0.25;
        if (jme_calc_ut(t, JME_BODY_MOON, flags, pos, error) != JME_OK) {
            return JME_ERR;
        }
        next_lat = pos[1];
        if ((lat <= 0.0 && next_lat >= 0.0) || (lat >= 0.0 && next_lat <= 0.0)) {
            double t0 = t - 0.25;
            double t1 = t;
            double l0 = lat;
            int j;

            for (j = 0; j < 48; j++) {
                double tm = (t0 + t1) / 2.0;
                double lm;
                if (jme_calc_ut(tm, JME_BODY_MOON, flags, pos, error) != JME_OK) {
                    return JME_ERR;
                }
                lm = pos[1];
                if (fabs(lm) < 1e-10 || fabs(t1 - t0) < 1e-10) {
                    *tret = tm;
                    return JME_OK;
                }
                if ((l0 <= 0.0 && lm >= 0.0) || (l0 >= 0.0 && lm <= 0.0)) {
                    t1 = tm;
                } else {
                    t0 = tm;
                    l0 = lm;
                }
            }
            *tret = (t0 + t1) / 2.0;
            return JME_OK;
        }
        lat = next_lat;
    }

    jme_set_error(error, "Lunar node crossing not found in search window");
    return JME_ERR;
}

int jme_mooncross_node_ut(double jd_ut, int flags, double *tret, char *error)
{
    return jme_mooncross_node(jd_ut, flags, tret, error);
}

int jme_helio_cross(int body, double x2cross, double jd_ut, int flags, double *tret, char *error)
{
    double t = jd_ut;
    double pos[6];
    double lon;
    double lon_next;
    int i;

    if (tret == 0) {
        jme_set_error(error, "Heliocentric crossing output is required");
        return JME_ERR;
    }

    x2cross = jme_degree_normalize(x2cross);
    for (i = 0; i < 365 * 20; i++) {
        if (jme_calc_ut(t, body, flags | JME_CALC_HELIOCENTRIC, pos, error) != JME_OK) {
            return JME_ERR;
        }
        lon = pos[0];
        t += 0.5;
        if (jme_calc_ut(t, body, flags | JME_CALC_HELIOCENTRIC, pos, error) != JME_OK) {
            return JME_ERR;
        }
        lon_next = pos[0];
        if (jme_degrees_difference_signed(lon_next, x2cross) >= 0.0 && jme_degrees_difference_signed(lon, x2cross) < 0.0) {
            return refine_longitude_crossing(t - 0.5, t, body, flags, 1, x2cross, tret, error);
        }
    }

    jme_set_error(error, "Heliocentric crossing not found in search window");
    return JME_ERR;
}

int jme_helio_cross_ut(int body, double x2cross, double jd_ut, int flags, double *tret, char *error)
{
    return jme_helio_cross(body, x2cross, jd_ut, flags, tret, error);
}

int jme_nod_aps_ut(double jd_ut, int body, int flags, int method, double *tret, char *error)
{
    double dt = jme_delta_t(jd_ut);
    double jd_et = jd_ut + dt / 86400.0;
    return jme_nod_aps(jd_et, body, flags, method, tret, error);
}

int jme_heliacal_pheno_ut(double jd_ut, double *geopos, double *dat_hel, char *error)
{
    (void)jd_ut; (void)geopos; (void)dat_hel;
    jme_set_error(error, "Heliacal phenomena require a validated visibility model");
    return JME_ERR;
}

double jme_heliacal_angle(double jd_ut, double *geopos, double *dat_hel, char *error)
{
    (void)jd_ut; (void)geopos; (void)dat_hel;
    jme_set_error(error, "Heliacal angle requires a validated visibility model");
    return NAN;
}

double jme_topo_arcus_visionis(double jd_ut, double *geopos, double *dat_hel, char *error)
{
    (void)jd_ut; (void)geopos; (void)dat_hel;
    jme_set_error(error, "Topocentric arcus visionis requires a validated visibility model");
    return NAN;
}

int jme_lun_eclipse_when_loc(double jd_start, int flags, double *geopos, double *tret, double *attr, int backward, char *error)
{
    int rc;
    double altitude = -90.0;

    if (geopos == 0 || tret == 0 || attr == 0) {
        jme_set_error(error, "Lunar eclipse local search requires geopos, tret, and attr outputs");
        return JME_ERR;
    }

    rc = jme_lun_eclipse_when(jd_start, flags, 0, tret, backward, error);
    if (rc == JME_ERR) {
        return JME_ERR;
    }

    if (jme_lun_eclipse_how(tret[0], flags, geopos, attr, error) != JME_ERR) {
        attr[8] = moon_is_above_horizon(tret[0], geopos, error, &altitude) ? JME_ECLIPSE_VISIBLE : 0.0;
        attr[7] = altitude;
        return rc;
    }

    return rc;
}

int jme_sol_eclipse_how(double jd_ut, int flags, double *geopos, double *attr, char *error)
{
    double sep = 0.0;
    double sun_r = 0.0;
    double moon_r = 0.0;
    double sun_alt = -90.0;
    double moon_alt = -90.0;
    int rc;
    int i;
    (void)flags;

    if (geopos == 0 || attr == 0) {
        jme_set_error(error, "Solar eclipse circumstance output requires geopos and attr");
        return JME_ERR;
    }

    if (solar_eclipse_local_geometry(jd_ut, geopos, &sep, &sun_r, &moon_r, &sun_alt, &moon_alt, error) != JME_OK) {
        return JME_ERR;
    }

    rc = classify_solar_eclipse(sep, sun_r, moon_r);
    if (rc == 0 || sun_alt <= 0.0) {
        jme_set_error(error, "No local solar eclipse is in progress at the supplied time");
        return JME_ERR;
    }

    for (i = 0; i < 20; i++) {
        attr[i] = 0.0;
    }

    attr[0] = (sun_r + moon_r - sep) / (2.0 * sun_r);
    attr[1] = rc == JME_ECLIPSE_SOLAR_TOTAL ? 1.0 : ((rc == JME_ECLIPSE_SOLAR_ANNULAR || rc == JME_ECLIPSE_SOLAR_HYBRID) ? (moon_r * moon_r) / (sun_r * sun_r) : attr[0]);
    attr[2] = sep;
    attr[3] = 2.0 * sun_r * 3600.0;
    attr[4] = 2.0 * moon_r * 3600.0;
    attr[5] = sun_alt;
    attr[6] = moon_alt;
    attr[7] = sep <= fabs(sun_r - moon_r) ? 1.0 : 0.0;
    attr[8] = JME_ECLIPSE_VISIBLE;

    return rc;
}

int jme_lun_eclipse_how(double jd_ut, int flags, double *geopos, double *attr, char *error)
{
    double umbral_mag = 0.0;
    double penumbral_mag = 0.0;
    double umbra_km = 0.0;
    double penumbra_km = 0.0;
    double offset_km = 0.0;
    double axis_km = 0.0;
    double altitude = -90.0;
    int rc;
    int i;
    (void)flags;

    if (attr == 0) {
        jme_set_error(error, "Lunar eclipse circumstance output is required");
        return JME_ERR;
    }

    for (i = 0; i < 20; i++) {
        attr[i] = 0.0;
    }

    if (lunar_eclipse_geometry(jd_ut, &umbral_mag, &penumbral_mag, &umbra_km, &penumbra_km, &offset_km, &axis_km, error) != JME_OK) {
        return JME_ERR;
    }

    rc = classify_lunar_eclipse(umbral_mag, penumbral_mag);
    if (rc == 0 || axis_km <= 0.0) {
        jme_set_error(error, "No lunar eclipse is in progress at the supplied time");
        return JME_ERR;
    }

    attr[0] = umbral_mag;
    attr[1] = penumbral_mag;
    attr[2] = offset_km;
    attr[3] = umbra_km;
    attr[4] = penumbra_km;
    attr[5] = axis_km;

    attr[8] = (geopos != 0 && moon_is_above_horizon(jd_ut, geopos, error, &altitude)) ? JME_ECLIPSE_VISIBLE : 0.0;
    attr[7] = altitude;

    return rc;
}

int jme_nod_aps(double jd_et, int body, int flags, int method, double *tret, char *error)
{
    double t;
    double mean_node;
    double true_node[6];
    double mean_apogee;
    double elem[20];
    double a;
    double e;
    double omega;
    double peri_lon;
    double node_radius;
    double apsis_radius;
    double true_anomaly_deg;
    double lon;
    int i;

    if (tret == 0) {
        jme_set_error(error, "Node/apside output is required");
        return JME_ERR;
    }

    for (i = 0; i < 4; i++) {
        tret[i] = 0.0;
    }

    if (body == JME_BODY_MOON) {
        t = (jd_et - 2451545.0) / 36525.0;
        mean_node = 125.0445479
            - 1934.1362891 * t
            + 0.0020754 * t * t
            + (t * t * t) / 467441.0
            - (t * t * t * t) / 60616000.0;
        mean_apogee = 83.3532465
            + 4069.0137287 * t
            - 0.0103200 * t * t
            - (t * t * t) / 80053.0
            + (t * t * t * t) / 18999000.0;

        if (method & 1) {
            lon = mean_node;
        } else if (method & 2) {
            if (jme_calc(jd_et, JME_BODY_TRUE_NODE, flags | JME_CALC_TRUE_POSITION, true_node, error) != JME_OK) {
                return JME_ERR;
            }
            lon = true_node[0];
        } else if (method & 4) {
            lon = mean_apogee;
        } else if (method & 256) {
            lon = jme_degree_normalize(mean_apogee + 180.0);
        } else {
            lon = mean_node;
        }

        tret[0] = jme_degree_normalize(lon);
        tret[1] = 0.0;
        tret[2] = 1.0;
        tret[3] = jd_et;
        return JME_OK;
    }

    if (body == JME_BODY_SUN || body == JME_BODY_EARTH) {
        jme_set_error(error, "Node/apside calculation requires a non-central orbiting body");
        return JME_ERR;
    }

    if (jme_get_orbital_elements(jd_et, body, flags | JME_CALC_HELIOCENTRIC | JME_CALC_TRUE_POSITION, elem, error) != JME_OK) {
        return JME_ERR;
    }

    a = elem[0];
    e = elem[1];
    omega = elem[4];
    peri_lon = elem[17];

    if (!isfinite(a) || !isfinite(e) || !isfinite(omega) || !isfinite(peri_lon) || a <= 0.0 || e < 0.0 || e >= 1.0) {
        jme_set_error(error, "Node/apside calculation requires a bounded elliptical osculating orbit");
        return JME_ERR;
    }

    if (method & 4) {
        lon = jme_degree_normalize(peri_lon + 180.0);
        apsis_radius = a * (1.0 + e);
    } else if (method & 256) {
        lon = jme_degree_normalize(peri_lon);
        apsis_radius = a * (1.0 - e);
    } else {
        lon = elem[3];
        if (method & 2) {
            lon = jme_degree_normalize(lon);
        }
        true_anomaly_deg = jme_degree_normalize(-omega);
        node_radius = orbital_radius_from_elements(a, e, true_anomaly_deg);
        apsis_radius = isfinite(node_radius) ? node_radius : 0.0;
    }

    tret[0] = jme_degree_normalize(lon);
    tret[1] = apsis_radius;
    tret[2] = 0.0;
    tret[3] = jd_et;

    return JME_OK;
}

int jme_get_orbital_elements(double jd_et, int body, int flags, double *elem, char *error)
{
    static const double mu = 0.0002959122082855911025; /* Gaussian gravitational constant squared, AU^3/day^2 */
    double state[6];
    double r[3], v[3], h[3], n[3], evec[3];
    double z[3] = {0.0, 0.0, 1.0};
    double rmag, vmag, hmag, nmag, e, energy;
    double inc, node, argp, nu, a;
    double ecc_anomaly = 0.0;
    double mean_anomaly = 0.0;
    double mean_motion = 0.0;
    double period_days = 0.0;
    double perihelion_distance = 0.0;
    double aphelion_distance = 0.0;
    int i;

    if (elem == 0) {
        jme_set_error(error, "Orbital element output is required");
        return JME_ERR;
    }

    for (i = 0; i < 20; i++) {
        elem[i] = 0.0;
    }

    if (body == JME_BODY_SUN || body == JME_BODY_EARTH) {
        jme_set_error(error, "Heliocentric osculating elements require a non-central body");
        return JME_ERR;
    }

    if (jme_calc(jd_et, body, flags | JME_CALC_HELIOCENTRIC | JME_CALC_XYZ | JME_CALC_SPEED | JME_CALC_TRUE_POSITION, state, error) != JME_OK) {
        return JME_ERR;
    }

    for (i = 0; i < 3; i++) {
        r[i] = state[i];
        v[i] = state[i + 3];
    }

    rmag = vec_norm(r);
    vmag = vec_norm(v);
    vec_cross(r, v, h);
    hmag = vec_norm(h);
    vec_cross(z, h, n);
    nmag = vec_norm(n);

    if (!isfinite(rmag) || !isfinite(vmag) || !isfinite(hmag) || rmag <= 0.0 || vmag <= 0.0 || hmag <= 0.0) {
        jme_set_error(error, "Orbital element state lacks a valid position/velocity vector");
        return JME_ERR;
    }

    for (i = 0; i < 3; i++) {
        evec[i] = ((vmag * vmag - mu / rmag) * r[i] - vec_dot(r, v) * v[i]) / mu;
    }
    e = vec_norm(evec);
    energy = 0.5 * vmag * vmag - mu / rmag;
    a = fabs(energy) > 0.0 ? -mu / (2.0 * energy) : 0.0;

    inc = acos(h[2] / hmag) * JME_EVENTS_RAD_TO_DEG;
    node = nmag > 0.0 ? jme_degree_normalize(atan2(n[1], n[0]) * JME_EVENTS_RAD_TO_DEG) : 0.0;

    if (nmag > 0.0 && e > 0.0) {
        double c = vec_dot(n, evec) / (nmag * e);
        if (c > 1.0) { c = 1.0; }
        if (c < -1.0) { c = -1.0; }
        argp = acos(c) * JME_EVENTS_RAD_TO_DEG;
        if (evec[2] < 0.0) { argp = 360.0 - argp; }
    } else {
        argp = 0.0;
    }

    if (e > 0.0) {
        double c = vec_dot(evec, r) / (e * rmag);
        if (c > 1.0) { c = 1.0; }
        if (c < -1.0) { c = -1.0; }
        nu = acos(c) * JME_EVENTS_RAD_TO_DEG;
        if (vec_dot(r, v) < 0.0) { nu = 360.0 - nu; }
    } else {
        nu = 0.0;
    }

    if (e < 1.0 && a > 0.0) {
        ecc_anomaly = 2.0 * atan2(
            sqrt(1.0 - e) * sin(nu * JME_EVENTS_DEG_TO_RAD / 2.0),
            sqrt(1.0 + e) * cos(nu * JME_EVENTS_DEG_TO_RAD / 2.0)
        );
        if (ecc_anomaly < 0.0) {
            ecc_anomaly += 2.0 * JME_EVENTS_PI;
        }
        mean_anomaly = ecc_anomaly - e * sin(ecc_anomaly);
        mean_motion = sqrt(mu / (a * a * a));
        period_days = 2.0 * JME_EVENTS_PI / mean_motion;
        mean_motion *= JME_EVENTS_RAD_TO_DEG;
        mean_anomaly *= JME_EVENTS_RAD_TO_DEG;
        perihelion_distance = a * (1.0 - e);
        aphelion_distance = a * (1.0 + e);
    }

    elem[0] = a;                                        /* semi-major axis, AU */
    elem[1] = e;                                        /* eccentricity */
    elem[2] = inc;                                      /* inclination, deg */
    elem[3] = node;                                     /* ascending node, deg */
    elem[4] = argp;                                     /* argument of perihelion, deg */
    elem[5] = nu;                                       /* true anomaly, deg */
    elem[6] = hmag;                                     /* specific angular momentum, AU^2/day */
    elem[7] = rmag;                                     /* current radius, AU */
    elem[8] = vmag;                                     /* current speed, AU/day */
    elem[9] = ecc_anomaly * JME_EVENTS_RAD_TO_DEG;      /* eccentric anomaly, deg */
    elem[10] = mean_anomaly;                            /* mean anomaly, deg */
    elem[11] = mean_motion;                             /* mean motion, deg/day */
    elem[12] = period_days;                             /* sidereal period, days */
    elem[13] = perihelion_distance;                     /* perihelion distance, AU */
    elem[14] = aphelion_distance;                       /* aphelion distance, AU */
    elem[15] = jme_degree_normalize(argp + nu);         /* argument of latitude, deg */
    elem[16] = jme_degree_normalize(node + argp + nu);  /* true longitude, deg */
    elem[17] = jme_degree_normalize(node + argp);       /* longitude of perihelion, deg */

    return JME_OK;
}

int jme_heliacal_ut(double jd_ut, double *geopos, double *dat_hel, char *error)
{
    (void)jd_ut; (void)geopos; (void)dat_hel;
    jme_set_error(error, "Heliacal visibility requires a validated visibility model");
    return JME_ERR;
}

int jme_vis_limit_mag(double jd_ut, double *geopos, double *dat_hel, char *error)
{
    (void)jd_ut; (void)geopos; (void)dat_hel;
    jme_set_error(error, "Limiting visual magnitude requires a validated visibility model");
    return JME_ERR;
}
