#include "jme/jme.h"
#include "context.h"
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#ifdef _WIN32
#include <windows.h>
#else
#include <sys/time.h>
#endif

#define JME_EVENTS_RAD_TO_DEG 57.295779513082320876798154814105170332405472466565
#define JME_EVENTS_DEG_TO_RAD 0.017453292519943295769236907684886127134428718885417
#define JME_EVENTS_PI 3.14159265358979323846264338327950288419716939937510
#define JME_EVENTS_EARTH_RADIUS_KM 6378.137
#define JME_EVENTS_MOON_RADIUS_KM 1737.4
#define JME_EVENTS_SUN_RADIUS_KM 695700.0

/*
 * Greenwich sidereal time (hours) consistent with the coordinates being
 * compared: apparent (equation of the equinoxes included) for apparent
 * true-equator-of-date positions, mean when JME_CALC_NO_NUTATION is set.
 */
static double events_gst_hours(double jd_ut, int flags)
{
    double hours = NAN;
    if (jme_sidereal_time_ex(jd_ut, (flags & JME_CALC_NO_NUTATION) == 0, &hours, 0) != JME_OK) {
        return NAN;
    }
    return hours;
}

static unsigned long long g_profile_rise_trans_calls = 0ULL;
static double g_profile_rise_trans_seconds = 0.0;
static unsigned long long g_profile_altitude_calls = 0ULL;
static double g_profile_altitude_seconds = 0.0;
static int g_profile_events_enabled = 0;
static int g_profile_events_init = 0;

static double jme_profile_events_now_seconds(void)
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

static void jme_profile_events_report(void)
{
    fprintf(stderr, "[jme-profile] rise_trans calls=%llu total_s=%.6f per_call_s=%.9f\n",
        g_profile_rise_trans_calls,
        g_profile_rise_trans_seconds,
        g_profile_rise_trans_calls > 0ULL ? (g_profile_rise_trans_seconds / (double)g_profile_rise_trans_calls) : 0.0);
    fprintf(stderr, "[jme-profile] altitude calls=%llu total_s=%.6f per_call_s=%.9f\n",
        g_profile_altitude_calls,
        g_profile_altitude_seconds,
        g_profile_altitude_calls > 0ULL ? (g_profile_altitude_seconds / (double)g_profile_altitude_calls) : 0.0);
}

static void jme_profile_events_maybe_init(void)
{
    if (g_profile_events_init) {
        return;
    }
    g_profile_events_init = 1;
    g_profile_events_enabled = getenv("JME_PROFILE") != 0;
    if (g_profile_events_enabled) {
        atexit(jme_profile_events_report);
    }
}

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

static int heliacal_body_from_data(const double *dat_hel, int *body, char *error)
{
    int candidate;

    if (dat_hel == 0 || body == 0) {
        jme_set_error(error, "Heliacal body selection requires input data");
        return JME_ERR;
    }

    if (!isfinite(dat_hel[0])) {
        jme_set_error(error, "Heliacal body selection requires a finite body id");
        return JME_ERR;
    }

    candidate = (int)dat_hel[0];
    if ((double)candidate != dat_hel[0]) {
        jme_set_error(error, "Heliacal body selection requires an integer body id");
        return JME_ERR;
    }

    if (candidate < JME_BODY_MOON || candidate > JME_BODY_PLUTO) {
        jme_set_error(error, "Heliacal body selection supports Moon and planetary bodies only");
        return JME_ERR;
    }

    *body = candidate;
    return JME_OK;
}

static double heliacal_required_arcus(double magnitude)
{
    if (!isfinite(magnitude)) {
        return NAN;
    }

    /*
     * Stellar and planetary first/last visibility threshold.
     * Independent visibility references commonly use the linear relation
     * arcus visionis = 10.50 + 1.40 * visual magnitude for heliacal events.
     */
    return 10.50 + 1.40 * magnitude;
}

static double heliacal_limiting_magnitude(double arcus)
{
    if (!isfinite(arcus)) {
        return NAN;
    }

    return (arcus - 10.50) / 1.40;
}

static int heliacal_visibility_flag(double body_alt, double sun_alt, double arcus, double magnitude, double limiting_mag, double required_arcus)
{
    return magnitude <= limiting_mag
        && body_alt >= 0.0
        && sun_alt < 0.0
        && arcus >= required_arcus;
}

#define JME_SOLAR_VISIBLE_UPPER_LIMB_ALTITUDE_DEG (-0.8333)

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

    if (jme_calc_ut(jd_ut, JME_BODY_SUN, 0, sun_state, error) != JME_OK
        || jme_calc_ut(jd_ut, JME_BODY_MOON, 0, moon_state, error) != JME_OK
        || jme_spherical_to_rectangular_state(sun_state, sun_state) != JME_OK
        || jme_spherical_to_rectangular_state(moon_state, moon_state) != JME_OK) {
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

        if (jme_calc_ut(t0, JME_BODY_SUN, 0, sun_state, error) != JME_OK
            || jme_calc_ut(t0, JME_BODY_MOON, 0, moon_state, error) != JME_OK) {
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

        if (jme_calc_ut(t1, JME_BODY_SUN, 0, sun_state, error) != JME_OK
            || jme_calc_ut(t1, JME_BODY_MOON, 0, moon_state, error) != JME_OK) {
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

                if (jme_calc_ut(m, JME_BODY_SUN, 0, sun_mid, error) != JME_OK
                    || jme_calc_ut(m, JME_BODY_MOON, 0, moon_mid, error) != JME_OK) {
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

        if (fabs(b - a) < 1e-9) {
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
    double altitude = get_altitude(jd_ut, JME_BODY_MOON, 0, 0, geopos, 0.0, 0.0, error);

    if (altitude_out != 0) {
        *altitude_out = altitude;
    }

    return altitude > 0.0;
}

static double moon_altitude_metric(double jd_ut, double *geopos, char *error)
{
    double altitude = get_altitude(jd_ut, JME_BODY_MOON, 0, 0, geopos, 0.0, 0.0, error);
    (void)error;
    return altitude;
}

static int refine_moon_horizon_crossing(double t0, double t1, double *geopos, double *tret, char *error)
{
    double f0 = moon_altitude_metric(t0, geopos, error);
    double f1 = moon_altitude_metric(t1, geopos, error);
    int i;

    if (tret == 0 || !isfinite(f0) || !isfinite(f1) || f0 * f1 > 0.0) {
        return JME_ERR;
    }

    for (i = 0; i < 48; i++) {
        double m = 0.5 * (t0 + t1);
        double fm = moon_altitude_metric(m, geopos, error);

        if (!isfinite(fm)) {
            return JME_ERR;
        }
        if (fabs(fm) < 1.0e-7 || fabs(t1 - t0) < 1.0e-8) {
            *tret = m;
            return JME_OK;
        }
        if (f0 * fm <= 0.0) {
            t1 = m;
            f1 = fm;
        } else {
            t0 = m;
            f0 = fm;
        }
    }

    (void)f1;
    *tret = 0.5 * (t0 + t1);
    return JME_OK;
}

static int find_visible_moon_interval(double start, double end, double *geopos, double *visible_start, double *visible_end, char *error)
{
    double step;
    double prev_t;
    double prev_alt;
    int prev_above;
    int in_visible;
    int found = 0;
    int i;

    if (visible_start == 0 || visible_end == 0 || geopos == 0 || !isfinite(start) || !isfinite(end) || end <= start) {
        jme_set_error(error, "Local lunar eclipse visibility interval is invalid");
        return JME_ERR;
    }

    step = (end - start) / 96.0;
    if (step < 1.0 / 1440.0) {
        step = 1.0 / 1440.0;
    }

    prev_t = start;
    prev_alt = moon_altitude_metric(prev_t, geopos, error);
    if (!isfinite(prev_alt)) {
        return JME_ERR;
    }
    prev_above = prev_alt > 0.0;
    in_visible = prev_above;
    if (in_visible) {
        *visible_start = start;
        found = 1;
    }

    for (i = 1; i <= 192; i++) {
        double t = start + step * i;
        double alt;
        int above;

        if (t > end) {
            t = end;
        }

        alt = moon_altitude_metric(t, geopos, error);
        if (!isfinite(alt)) {
            return JME_ERR;
        }
        above = alt > 0.0;

        if (!prev_above && above) {
            double cross = t;
            refine_moon_horizon_crossing(prev_t, t, geopos, &cross, error);
            if (!found) {
                *visible_start = cross;
            }
            found = 1;
            in_visible = 1;
        } else if (prev_above && !above) {
            double cross = t;
            refine_moon_horizon_crossing(prev_t, t, geopos, &cross, error);
            if (in_visible) {
                *visible_end = cross;
                return JME_OK;
            }
        }

        if (t >= end) {
            break;
        }
        prev_t = t;
        prev_alt = alt;
        prev_above = above;
    }

    if (found && in_visible) {
        *visible_end = end;
        return JME_OK;
    }

    jme_set_error(error, "No locally visible lunar eclipse interval found");
    return JME_ERR;
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

        if (jme_calc_ut(t0, JME_BODY_SUN, 0, sun_state, error) != JME_OK
            || jme_calc_ut(t0, JME_BODY_MOON, 0, moon_state, error) != JME_OK) {
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

        if (jme_calc_ut(t1, JME_BODY_SUN, 0, sun_state, error) != JME_OK
            || jme_calc_ut(t1, JME_BODY_MOON, 0, moon_state, error) != JME_OK) {
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

                if (jme_calc_ut(m, JME_BODY_SUN, 0, sun_mid, error) != JME_OK
                    || jme_calc_ut(m, JME_BODY_MOON, 0, moon_mid, error) != JME_OK) {
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


static int solar_eclipse_local_geometry(double jd_ut, double *geopos, double *separation_deg, double *sun_radius_deg, double *moon_radius_deg, double *sun_alt_deg, double *moon_alt_deg, char *error)
{
    double sun_eq[6];
    double moon_eq[6];
    double sun_dist_km;
    double moon_dist_km;

    if (geopos == 0) {
        jme_set_error(error, "Solar eclipse local geometry requires geopos");
        return JME_ERR;
    }

    internal_topo_override_set(geopos[0], geopos[1], geopos[2]);

    if (jme_calc_ut(jd_ut, JME_BODY_SUN, JME_CALC_EQUATORIAL | JME_CALC_TOPOCENTRIC, sun_eq, error) != JME_OK
        || jme_calc_ut(jd_ut, JME_BODY_MOON, JME_CALC_EQUATORIAL | JME_CALC_TOPOCENTRIC, moon_eq, error) != JME_OK) {
        return JME_ERR;
    }

    sun_dist_km = sun_eq[2] * JME_AU_KM;
    moon_dist_km = moon_eq[2] * JME_AU_KM;

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
        *sun_alt_deg = get_altitude(jd_ut, JME_BODY_SUN, 0, 0, geopos, 0.0, 0.0, error);
    }
    if (moon_alt_deg != 0) {
        *moon_alt_deg = get_altitude(jd_ut, JME_BODY_MOON, 0, 0, geopos, 0.0, 0.0, error);
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



/*
 * Greatest eclipse (global): the instant at which the axis of the lunar
 * shadow passes closest to the Earth's centre (minimum gamma), the
 * definition used by the NASA eclipse catalogues (Espenak & Meeus 2006,
 * NASA TP-2006-214141, sect. 1) and the Explanatory Supplement. Apparent
 * geocentric places of Sun and Moon are used. This differs from the minimum
 * of the geocentric Sun-Moon separation by up to a few seconds for eclipses
 * of large gamma.
 */
static double solar_eclipse_global_metric(double jd_ut, char *error)
{
    double sun[6], moon[6], axis[3], closest[3], d, s;
    int i;

    if (jme_calc_ut(jd_ut, JME_BODY_SUN, JME_CALC_XYZ, sun, error) != JME_OK
        || jme_calc_ut(jd_ut, JME_BODY_MOON, JME_CALC_XYZ, moon, error) != JME_OK) {
        return NAN;
    }
    for (i = 0; i < 3; i++) axis[i] = moon[i] - sun[i];
    d = vec_norm(axis);
    if (!isfinite(d) || d <= 0.0) return NAN;
    for (i = 0; i < 3; i++) axis[i] /= d;
    s = -vec_dot(moon, axis);
    for (i = 0; i < 3; i++) closest[i] = moon[i] + s * axis[i];
    return vec_norm(closest) * JME_AU_KM;
}

/* Local maximum: minimum topocentric apparent Sun-Moon separation
 * (independent of the Sun's altitude; visibility is judged separately). */
static double solar_eclipse_local_metric(double jd_ut, double *geopos, char *error)
{
    double sep = NAN;

    if (solar_eclipse_local_geometry(jd_ut, geopos, &sep, 0, 0, 0, 0, error) != JME_OK) {
        return NAN;
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

        if (fabs(b - a) < 1e-9) {
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

static double lunar_horizontal_parallax_deg(double moon_radius_deg)
{
    double moon_distance_km;

    if (!isfinite(moon_radius_deg) || moon_radius_deg <= 0.0) {
        return NAN;
    }

    moon_distance_km = JME_EVENTS_MOON_RADIUS_KM / sin(moon_radius_deg * JME_EVENTS_DEG_TO_RAD);
    return angular_radius_deg(JME_EVENTS_EARTH_RADIUS_KM, moon_distance_km);
}

static int moon_target_state(double jd_ut, int body, const char *starname, int flags, int local_mode, double *geopos, double *moon_eq, double *target_eq, double *moon_radius_deg, double *target_radius_deg, double *moon_alt_deg, double *target_alt_deg, char *error)
{
    double moon_eq_xyz[6];
    double target_eq_xyz[6];
    double topo_pos[3];
    double topo_state[6];
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

    if (jme_calc_ut(jd_ut, JME_BODY_MOON, JME_CALC_XYZ | JME_CALC_EQUATORIAL, moon_eq_xyz, error) != JME_OK) {
        return JME_ERR;
    }

    if (starname != 0 && starname[0] != '\0') {
        if (jme_fixstar_ut(starname, jd_ut, JME_CALC_XYZ | JME_CALC_EQUATORIAL, target_eq_xyz, error) != JME_OK) {
            return JME_ERR;
        }
    } else if (jme_calc_ut(jd_ut, body, JME_CALC_XYZ | JME_CALC_EQUATORIAL, target_eq_xyz, error) != JME_OK) {
        return JME_ERR;
    }

    for (i = 0; i < 6; i++) {
        moon_topo_xyz[i] = moon_eq_xyz[i];
        target_topo_xyz[i] = target_eq_xyz[i];
    }

    if (local_mode) {
        double jd_tt;
        internal_topo_override_set(geopos[0], geopos[1], geopos[2]);
        /* Observer in the true equator and equinox of date (same frame as the
         * apparent equatorial Moon/target vectors), at TT. */
        if (internal_ut_to_et(jd_ut, &jd_tt, error) != JME_OK
            || internal_topo_state_true_equator(jd_tt, topo_pos, 0, error) != JME_OK) {
            return JME_ERR;
        }
        topo_state[0] = topo_pos[0];
        topo_state[1] = topo_pos[1];
        topo_state[2] = topo_pos[2];
        topo_state[3] = 0.0;
        topo_state[4] = 0.0;
        topo_state[5] = 0.0;

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
        *moon_alt_deg = local_mode ? get_altitude(jd_ut, JME_BODY_MOON, 0, 0, geopos, 0.0, 0.0, error) : NAN;
    }
    if (target_alt_deg != 0) {
        *target_alt_deg = local_mode
            ? get_altitude(jd_ut, body, starname, 0, geopos, 0.0, 0.0, error)
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

    if (moon_target_state(jd_ut, body, starname, 0, local_mode, geopos, moon_eq, target_eq, &moon_r, &target_r, &moon_alt, &target_alt, error) != JME_OK) {
        return NAN;
    }

    sep = jme_spherical_angular_separation(moon_eq[0], moon_eq[1], target_eq[0], target_eq[1]);
    if (local_mode && (moon_alt <= 0.0 || target_alt <= 0.0)) {
        return sep + 180.0 + fabs(moon_alt < target_alt ? moon_alt : target_alt);
    }

    return sep;
}

static int find_moon_target_conjunction(double jd_start, int body, const char *starname, int backward, double *tret, char *error)
{
    double direction = backward ? -1.0 : 1.0;
    double t0 = jd_start;
    double prev_diff;
    int i;

    {
        double moon_state[6];
        double target_state[6];

        if (jme_calc_ut(t0, JME_BODY_MOON, 0, moon_state, error) != JME_OK) {
            return JME_ERR;
        }
        if (starname != 0 && starname[0] != '\0') {
            if (jme_fixstar_ut(starname, t0, 0, target_state, error) != JME_OK) {
                return JME_ERR;
            }
        } else if (jme_calc_ut(t0, body, 0, target_state, error) != JME_OK) {
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

        if (jme_calc_ut(t1, JME_BODY_MOON, 0, moon_state, error) != JME_OK) {
            return JME_ERR;
        }
        if (starname != 0 && starname[0] != '\0') {
            if (jme_fixstar_ut(starname, t1, 0, target_state, error) != JME_OK) {
                return JME_ERR;
            }
        } else if (jme_calc_ut(t1, body, 0, target_state, error) != JME_OK) {
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

                if (jme_calc_ut(m, JME_BODY_MOON, 0, moon_mid, error) != JME_OK) {
                    return JME_ERR;
                }
                if (starname != 0 && starname[0] != '\0') {
                    if (jme_fixstar_ut(starname, m, 0, target_mid, error) != JME_OK) {
                        return JME_ERR;
                    }
                } else if (jme_calc_ut(m, body, 0, target_mid, error) != JME_OK) {
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

static int refine_occultation_maximum(double guess, int body, const char *starname, double *geopos, int local_mode, double *tmax, char *error)
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

static int find_occultation_contact_time(double tmid, int body, const char *starname, double threshold_deg, double *geopos, int local_mode, int left_side, double *tret, char *error)
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

/*
 * Local contact: root of sep(t) - (r_sun(t) + r_moon(t)) (outer, inner = 0)
 * or sep(t) - |r_sun(t) - r_moon(t)| (inner = 1), all topocentric apparent
 * and re-evaluated at every trial instant.
 */
static double solar_local_contact_function(double t, double *geopos, int inner, char *error)
{
    double sep, sr, mr;
    if (solar_eclipse_local_geometry(t, geopos, &sep, &sr, &mr, 0, 0, error) != JME_OK
        || !isfinite(sep) || !isfinite(sr) || !isfinite(mr)) {
        return NAN;
    }
    return inner ? sep - fabs(sr - mr) : sep - (sr + mr);
}

static int find_solar_contact_time(double tmid, double *geopos, int inner, int left_side, double *tret, char *error)
{
    double step = 0.01;
    double t_prev = tmid;
    double f_prev = solar_local_contact_function(t_prev, geopos, inner, error);
    int i, j;

    if (!isfinite(f_prev)) return JME_ERR;
    if (f_prev > 0.0) return JME_ERR; /* no overlap of this kind at tmid */
    for (i = 1; i <= 40; i++) {
        double t = tmid + (left_side ? -1.0 : 1.0) * step * i;
        double f = solar_local_contact_function(t, geopos, inner, error);
        if (!isfinite(f)) return JME_ERR;
        if (f > 0.0) {
            double lo = t_prev, hi = t; /* f(lo) <= 0 < f(hi) */
            for (j = 0; j < 200 && fabs(hi - lo) > 1e-10; j++) {
                double m = 0.5 * (lo + hi);
                double fm = solar_local_contact_function(m, geopos, inner, error);
                if (!isfinite(fm)) return JME_ERR;
                if (fm > 0.0) hi = m; else lo = m;
            }
            *tret = 0.5 * (lo + hi);
            return JME_OK;
        }
        t_prev = t;
    }
    return JME_ERR;
}

/* Fraction of the solar disc area covered by the lunar disc (radii and
 * separation in the same angular unit); exact circle-overlap area. */
static double solar_obscuration(double sep, double rs, double rm)
{
    double area;
    if (!isfinite(sep) || !isfinite(rs) || !isfinite(rm) || rs <= 0.0 || rm <= 0.0) return NAN;
    if (sep >= rs + rm) return 0.0;
    if (sep <= fabs(rs - rm)) return rm >= rs ? 1.0 : (rm * rm) / (rs * rs);
    area = rm * rm * acos((sep * sep + rm * rm - rs * rs) / (2.0 * sep * rm))
        + rs * rs * acos((sep * sep + rs * rs - rm * rm) / (2.0 * sep * rs))
        - 0.5 * sqrt((-sep + rm + rs) * (sep + rm - rs) * (sep - rm + rs) * (sep + rm + rs));
    return area / (JME_EVENTS_PI * rs * rs);
}

static double get_altitude_internal(double jd_ut, int body, const char *starname, int flags, double *geopos, double atpress, double attemp, int set_topo, char *error)
{
    double t0 = 0.0;
    double results[6];
    double ra, dec, hour_angle, azimuth, altitude;
    double gst;
    int equatorial_flag = (int)JME_CALC_EQUATORIAL;
    int res;

    if (set_topo) {
        internal_topo_override_set(geopos[0], geopos[1], geopos[2]);
    }

    jme_profile_events_maybe_init();
    if (g_profile_events_enabled) {
        t0 = jme_profile_events_now_seconds();
    }

    if (starname != 0 && starname[0] != '\0') {
        flags = flags | (int)JME_CALC_TOPOCENTRIC | equatorial_flag;
        res = jme_fixstar_ut(starname, jd_ut, flags, results, error);
    } else {
        flags = flags | (int)JME_CALC_TOPOCENTRIC | equatorial_flag;
        res = jme_calc_ut(jd_ut, body, flags, results, error);
    }

    if (res != JME_OK) {
        if (g_profile_events_enabled) {
            g_profile_altitude_calls += 1ULL;
            g_profile_altitude_seconds += (jme_profile_events_now_seconds() - t0);
        }
        return -100.0;
    }

    ra = results[0];
    dec = results[1];

    gst = events_gst_hours(jd_ut, flags);
    hour_angle = jme_degree_normalize(gst * 15.0 + geopos[0] - ra);

    jme_equatorial_to_horizontal(hour_angle, dec, geopos[1], &azimuth, &altitude);

    if (atpress > 0.0) {
        altitude = jme_refract_extended(
            altitude,
            geopos != 0 ? geopos[2] : 0.0,
            atpress,
            attemp,
            jme_get_context()->lapse_rate,
            JME_COORD_TRUE_TO_APPARENT,
            0
        );
    }

    if (g_profile_events_enabled) {
        g_profile_altitude_calls += 1ULL;
        g_profile_altitude_seconds += (jme_profile_events_now_seconds() - t0);
    }
    return altitude;
}

static double get_altitude(double jd_ut, int body, const char *starname, int flags, double *geopos, double atpress, double attemp, char *error)
{
    return get_altitude_internal(jd_ut, body, starname, flags, geopos, atpress, attemp, 1, error);
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

    internal_topo_override_set(geopos[0], geopos[1], geopos[2]);
    a0 = get_altitude_internal(t0, body, starname, flags, geopos, atpress, attemp, 0, error) - h0;
    a1 = get_altitude_internal(t1, body, starname, flags, geopos, atpress, attemp, 0, error) - h0;

    if (a0 == -100.0 - h0 || a1 == -100.0 - h0 || a0 * a1 > 0.0) {
        return JME_ERR;
    }

    for (i = 0; i < 20; i++) {
        mid = (t0 + t1) / 2.0;
        amid = get_altitude_internal(mid, body, starname, flags, geopos, atpress, attemp, 0, error) - h0;

        if (fabs(amid) < 1e-6 || fabs(t1 - t0) < 1e-6) {
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
    double results[6];
    int i;

    if (starname != 0 && starname[0] != '\0') {
        if (jme_fixstar_ut(starname, t0, flags | JME_CALC_EQUATORIAL, results, error) != JME_OK) {
            return JME_ERR;
        }
    } else if (jme_calc_ut(t0, body, flags | JME_CALC_EQUATORIAL, results, error) != JME_OK) {
        return JME_ERR;
    }
    ha0 = jme_degrees_difference_signed(
        jme_degrees_difference_signed(events_gst_hours(t0, flags) * 15.0 + geopos[0], results[0]),
        target_hour_angle
    );

    if (starname != 0 && starname[0] != '\0') {
        if (jme_fixstar_ut(starname, t1, flags | JME_CALC_EQUATORIAL, results, error) != JME_OK) {
            return JME_ERR;
        }
    } else if (jme_calc_ut(t1, body, flags | JME_CALC_EQUATORIAL, results, error) != JME_OK) {
        return JME_ERR;
    }
    ha1 = jme_degrees_difference_signed(
        jme_degrees_difference_signed(events_gst_hours(t1, flags) * 15.0 + geopos[0], results[0]),
        target_hour_angle
    );

    if (ha0 * ha1 > 0.0) {
        return JME_ERR;
    }

    for (i = 0; i < 20; i++) {
        double tm = (t0 + t1) / 2.0;
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
        ham = jme_degrees_difference_signed(jme_degrees_difference_signed(events_gst_hours(tm, flags) * 15.0 + geopos[0], ra), target_hour_angle);

        if (fabs(ham) < 1e-4 || fabs(t1 - t0) < 1e-6) {
            *tret = tm;
            return JME_OK;
        }

        if (ha0 * ham <= 0.0) {
            t1 = tm;
            ha1 = ham;
        } else if (ha1 * ham <= 0.0) {
            t0 = tm;
            ha0 = ham;
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
    double search_t0 = 0.0;
    double t = jd_ut;
    double h0 = -0.5667; /* Standard for stars (refraction only) */
    double search_atpress = atpress;
    double alt, alt_next;
    int i;

    if (tret != 0) { *tret = 0.0; }
    jme_profile_events_maybe_init();
    if (g_profile_events_enabled) {
        search_t0 = jme_profile_events_now_seconds();
    }
    if (geopos == 0 || tret == 0) {
        jme_set_error(error, "Rise and transit output arguments are required");
        if (g_profile_events_enabled) {
            g_profile_rise_trans_calls += 1ULL;
            g_profile_rise_trans_seconds += (jme_profile_events_now_seconds() - search_t0);
        }
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

    internal_topo_override_set(geopos[0], geopos[1], geopos[2]);

    if (rsmi & (JME_RISE_MERIDIAN_TRANSIT | JME_RISE_ANTI_MERIDIAN_TRANSIT)) {
        double target_hour_angle = (rsmi & JME_RISE_ANTI_MERIDIAN_TRANSIT) ? 180.0 : 0.0;
        double dt = 0.01;
        /* Search for transit from the supplied Julian day across one day. */
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
        gst = events_gst_hours(t, flags);
        ha = jme_degrees_difference_signed(jme_degrees_difference_signed(gst * 15.0 + geopos[0], ra), target_hour_angle);

        for (i = 0; i < 240; i++) {
            t += dt; /* 14.4 mins */
            gst = events_gst_hours(t, flags);
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
                {
                    int rc = refine_meridian_transit(t - dt, t, body, starname, flags, geopos, target_hour_angle, tret, error);
                    if (g_profile_events_enabled) {
                        g_profile_rise_trans_calls += 1ULL;
                        g_profile_rise_trans_seconds += (jme_profile_events_now_seconds() - search_t0);
                    }
                    return rc;
                }
            }
            ha = ha_next;
        }
    }

    /* Rise/set search across one day. */
    alt = get_altitude_internal(t, body, starname, flags, geopos, search_atpress, attemp, 0, error);
    for (i = 0; i < 24; i++) {
        t += 1.0 / 24.0;
        alt_next = get_altitude_internal(t, body, starname, flags, geopos, search_atpress, attemp, 0, error);

        if (rsmi & JME_RISE_RISE) {
            if (alt < h0 && alt_next >= h0) {
                {
                    int rc = refine_altitude_crossing(t - 1.0 / 24.0, t, h0, body, starname, flags, geopos, search_atpress, attemp, tret, error);
                    if (g_profile_events_enabled) {
                        g_profile_rise_trans_calls += 1ULL;
                        g_profile_rise_trans_seconds += (jme_profile_events_now_seconds() - search_t0);
                    }
                    return rc;
                }
            }
        } else if (rsmi & JME_RISE_SET) {
            if (alt > h0 && alt_next <= h0) {
                {
                    int rc = refine_altitude_crossing(t - 1.0 / 24.0, t, h0, body, starname, flags, geopos, search_atpress, attemp, tret, error);
                    if (g_profile_events_enabled) {
                        g_profile_rise_trans_calls += 1ULL;
                        g_profile_rise_trans_seconds += (jme_profile_events_now_seconds() - search_t0);
                    }
                    return rc;
                }
            }
        }
        alt = alt_next;
    }

    jme_set_error(error, "Event not found in 24h search window");
    if (g_profile_events_enabled) {
        g_profile_rise_trans_calls += 1ULL;
        g_profile_rise_trans_seconds += (jme_profile_events_now_seconds() - search_t0);
    }
    return JME_ERR;
}

static int jme_rise_trans_impl(double jd_ut, int body, const char *starname, int flags, int rsmi, double *geopos, double atpress, double attemp, double *tret, char *error)
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

static int solar_bessel_classify(double jd_ut, int *rc_out, int *class_out, double *lon, double *lat, double *distance_km, char *error);

static int jme_solcross_impl(double x2cross, double jd_ut, int flags, double *tret, char *error)
{
    double t = jd_ut;
    double lon, lon_next;
    double results[6];
    int i;

    if (tret == 0) {
        jme_set_error(error, "Solar crossing output is required");
        return JME_ERR;
    }

    x2cross = jme_degree_normalize(x2cross);
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

    jme_set_error(error, "Solar crossing not found in search window");
    return JME_ERR;
}

static int jme_mooncross_impl(double x2cross, double jd_ut, int flags, double *tret, char *error)
{
    double t = jd_ut;
    double lon, lon_next;
    double results[6];
    int i;

    if (tret == 0) {
        jme_set_error(error, "Lunar crossing output is required");
        return JME_ERR;
    }

    x2cross = jme_degree_normalize(x2cross);
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

    jme_set_error(error, "Lunar crossing not found in search window");
    return JME_ERR;
}

/*
 * Next (or previous) solar eclipse visible from geopos (lon, lat, height m).
 * Successive new moons are examined: lunations without a global eclipse are
 * skipped, then the local maximum (minimum topocentric separation) and the
 * local contacts are computed. An eclipse counts as visible if the Sun's
 * upper limb is above the apparent horizon (-0.8333 deg, standard refraction
 * plus semi-diameter) at some instant between first and last contact.
 *   tret[0] local maximum; tret[1] 0; tret[2]/tret[3] first/last contact;
 *   tret[4]/tret[5] second/third contact (total or annular phase), else 0.
 *   attr[0] magnitude, attr[1] obscuration, attr[2] separation (deg),
 *   attr[3]/attr[4] solar/lunar apparent diameters (arcsec), attr[5]/attr[6]
 *   Sun/Moon true altitude at maximum (deg), attr[7] 1 if the central phase
 *   is seen, attr[8] JME_ECLIPSE_VISIBLE, attr[9] 1 if the maximum itself
 *   is visible (Sun above the horizon at maximum), else 0.
 */
static int solar_local_sun_visible(double t, double *geopos, char *error)
{
    double alt = get_altitude(t, JME_BODY_SUN, 0, 0, geopos, 0.0, 0.0, error);
    return isfinite(alt) && alt > JME_SOLAR_VISIBLE_UPPER_LIMB_ALTITUDE_DEG;
}

static int jme_sol_eclipse_when_loc_impl(double jd_start, int flags, double *geopos, double *tret, double *attr, int backward, char *error)
{
    double guess = 0.0;
    double search_jd = jd_start;
    double direction = backward ? -1.0 : 1.0;
    int attempt;
    (void)flags;

    if (geopos == 0 || tret == 0 || attr == 0) {
        jme_set_error(error, "Solar eclipse local search requires geopos, tret, and attr outputs");
        return JME_ERR;
    }
    if (!isfinite(jd_start) || !isfinite(geopos[0]) || !isfinite(geopos[1]) || !isfinite(geopos[2])
        || geopos[1] < -90.0 || geopos[1] > 90.0) {
        jme_set_error(error, "Solar eclipse local search inputs are invalid");
        return JME_ERR;
    }

    for (attempt = 0; attempt < 3000; attempt++) {
        double gmax = 0.0, tmax = 0.0, sep, sun_r, moon_r, sun_alt, moon_alt;
        double c1 = 0.0, c4 = 0.0;
        int grc = 0, rc, visible, i;

        if (find_new_moon_conjunction(search_jd, backward, &guess, error) != JME_OK) {
            return JME_ERR;
        }
        search_jd = guess + direction;
        if (refine_solar_maximum(guess, 0, 0, &gmax, error) != JME_OK
            || solar_bessel_classify(gmax, &grc, 0, 0, 0, 0, error) != JME_OK) {
            return JME_ERR;
        }
        if (grc == 0) continue;
        if (refine_solar_maximum(guess, geopos, 1, &tmax, error) != JME_OK
            || solar_eclipse_local_geometry(tmax, geopos, &sep, &sun_r, &moon_r, &sun_alt, &moon_alt, error) != JME_OK) {
            return JME_ERR;
        }
        rc = classify_solar_eclipse(sep, sun_r, moon_r);
        if (rc == 0) continue;
        if (find_solar_contact_time(tmax, geopos, 0, 1, &c1, error) != JME_OK
            || find_solar_contact_time(tmax, geopos, 0, 0, &c4, error) != JME_OK) {
            return JME_ERR;
        }
        visible = solar_local_sun_visible(tmax, geopos, error)
            || solar_local_sun_visible(c1, geopos, error)
            || solar_local_sun_visible(c4, geopos, error);
        if (!visible) {
            /* sunrise or sunset strictly inside the eclipse interval */
            double t;
            for (t = c1; t < c4 && !visible; t += 1.0 / 1440.0) {
                visible = solar_local_sun_visible(t, geopos, error);
            }
        }
        if (!visible) continue;

        memset(tret, 0, sizeof(double) * 10);
        for (i = 0; i < 20; i++) attr[i] = 0.0;
        tret[0] = tmax;
        tret[2] = c1;
        tret[3] = c4;
        if (rc == JME_ECLIPSE_SOLAR_TOTAL || rc == JME_ECLIPSE_SOLAR_ANNULAR) {
            if (find_solar_contact_time(tmax, geopos, 1, 1, &tret[4], error) != JME_OK
                || find_solar_contact_time(tmax, geopos, 1, 0, &tret[5], error) != JME_OK) {
                return JME_ERR;
            }
        }
        attr[0] = (sun_r + moon_r - sep) / (2.0 * sun_r);
        attr[1] = solar_obscuration(sep, sun_r, moon_r);
        attr[2] = sep;
        attr[3] = 2.0 * sun_r * 3600.0;
        attr[4] = 2.0 * moon_r * 3600.0;
        attr[5] = sun_alt;
        attr[6] = moon_alt;
        attr[7] = sep <= fabs(sun_r - moon_r) ? 1.0 : 0.0;
        attr[8] = JME_ECLIPSE_VISIBLE;
        attr[9] = solar_local_sun_visible(tmax, geopos, error) ? 1.0 : 0.0;
        if (error != 0) error[0] = '\0';
        return rc;
    }

    jme_set_error(error, "No local solar eclipse found in search window");
    return JME_ERR;
}

/* ------------------------------------------------------------------------
 * Global solar-eclipse geometry on the Besselian fundamental plane.
 *
 * Construction (Explanatory Supplement to the Astronomical Almanac, 1992,
 * ch. 8; Meeus, Elements of Solar Eclipses 1951-2200, 1989): the z axis is
 * parallel to the shadow axis (Moon -> Sun direction), the fundamental plane
 * passes through the Earth's centre, unit = Earth equatorial radius. Sun and
 * Moon are apparent geocentric places on the true equator and equinox of
 * date; Earth-fixed coordinates follow from Greenwich apparent sidereal time
 * (polar motion not applied). The Earth is the WGS84 ellipsoid, treated
 * exactly in 3-D (no first-order ellipticity approximation):
 *   - the shadow axis meets the surface where the line Moon + s(-k)
 *     intersects the ellipsoid (quadratic in s);
 *   - penumbral/umbral contacts with the Earth are tangencies of the shadow
 *     cones with the ellipsoid's silhouette (the curve where the surface
 *     normal is perpendicular to the axis), evaluated with the cone radius
 *     at each silhouette point's own height zeta above the plane.
 * Residual: tangency is taken on the silhouette for the axis direction, not
 * for each cone generator (tilted by the cone half-angle, about 0.27 deg);
 * the effect on P1/P4/U1/U4 is below 0.2 s.
 * ---------------------------------------------------------------------- */
#define JME_BESSEL_EARTH_A_KM 6378.137
#define JME_BESSEL_EARTH_F (1.0 / 298.257223563)

typedef struct solar_bessel {
    double moon[3];   /* Moon, geocentric true-equator-of-date, Earth radii */
    double k[3];      /* unit vector Moon -> Sun (shadow axis, towards Sun) */
    double xhat[3], yhat[3];
    double x, y, z;   /* Moon in the fundamental frame, Earth radii */
    double tanf1, tanf2;
    double l1, l2;    /* penumbral/umbral radius on the plane; l2 < 0 total */
    double gast;      /* Greenwich apparent sidereal time, rad */
} solar_bessel;

static int solar_bessel_at(double jd_ut, solar_bessel *b, char *error)
{
    double sun[6], moon[6], g[3], gn, sinf1, sinf2, kk, d, a;
    int i;

    if (jme_calc_ut(jd_ut, JME_BODY_SUN, JME_CALC_EQUATORIAL | JME_CALC_XYZ, sun, error) != JME_OK
        || jme_calc_ut(jd_ut, JME_BODY_MOON, JME_CALC_EQUATORIAL | JME_CALC_XYZ, moon, error) != JME_OK) {
        return JME_ERR;
    }
    for (i = 0; i < 3; i++) {
        sun[i] *= JME_AU_KM / JME_BESSEL_EARTH_A_KM;
        b->moon[i] = moon[i] * JME_AU_KM / JME_BESSEL_EARTH_A_KM;
        g[i] = sun[i] - b->moon[i];
    }
    gn = vec_norm(g);
    if (!isfinite(gn) || gn <= 0.0) {
        jme_set_error(error, "Solar eclipse geometry requires a valid Sun-Moon vector");
        return JME_ERR;
    }
    for (i = 0; i < 3; i++) b->k[i] = g[i] / gn;
    d = asin(b->k[2]);
    a = atan2(b->k[1], b->k[0]);
    b->xhat[0] = -sin(a); b->xhat[1] = cos(a); b->xhat[2] = 0.0;
    b->yhat[0] = -sin(d) * cos(a); b->yhat[1] = -sin(d) * sin(a); b->yhat[2] = cos(d);
    b->x = vec_dot(b->moon, b->xhat);
    b->y = vec_dot(b->moon, b->yhat);
    b->z = vec_dot(b->moon, b->k);
    sinf1 = (JME_EVENTS_SUN_RADIUS_KM + JME_EVENTS_MOON_RADIUS_KM) / (gn * JME_BESSEL_EARTH_A_KM);
    sinf2 = (JME_EVENTS_SUN_RADIUS_KM - JME_EVENTS_MOON_RADIUS_KM) / (gn * JME_BESSEL_EARTH_A_KM);
    b->tanf1 = sinf1 / sqrt(1.0 - sinf1 * sinf1);
    b->tanf2 = sinf2 / sqrt(1.0 - sinf2 * sinf2);
    kk = JME_EVENTS_MOON_RADIUS_KM / JME_BESSEL_EARTH_A_KM;
    b->l1 = (b->z + kk / sinf1) * b->tanf1;
    b->l2 = (b->z - kk / sinf2) * b->tanf2;
    b->gast = events_gst_hours(jd_ut, 0) * 15.0 * JME_EVENTS_DEG_TO_RAD;
    if (!isfinite(b->gast)) {
        jme_set_error(error, "Sidereal time unavailable for eclipse geometry");
        return JME_ERR;
    }
    return JME_OK;
}

/* Intersection of the shadow axis with the ellipsoid nearest the Moon.
 * Returns 1 and the point (Earth radii, true equator of date) if the axis
 * meets the Earth; 0 otherwise. *disc receives the scaled discriminant
 * (>= 0 iff the axis meets the ellipsoid). */
static int solar_bessel_axis_point(const solar_bessel *b, double *point, double *disc)
{
    double bb = 1.0 - JME_BESSEL_EARTH_F;
    double w = 1.0 / (bb * bb);
    double u[3] = {-b->k[0], -b->k[1], -b->k[2]};
    double qa = u[0] * u[0] + u[1] * u[1] + w * u[2] * u[2];
    double qb = 2.0 * (b->moon[0] * u[0] + b->moon[1] * u[1] + w * b->moon[2] * u[2]);
    double qc = b->moon[0] * b->moon[0] + b->moon[1] * b->moon[1] + w * b->moon[2] * b->moon[2] - 1.0;
    double dsc = qb * qb - 4.0 * qa * qc;
    double s;
    int i;

    if (disc != 0) *disc = dsc / (4.0 * qa);
    if (dsc < 0.0) return 0;
    /* nearer root, written to avoid cancellation */
    s = (2.0 * qc) / (-qb + sqrt(dsc));
    if (point != 0) for (i = 0; i < 3; i++) point[i] = b->moon[i] + s * u[i];
    return 1;
}

static void solar_bessel_geodetic(const solar_bessel *b, const double *p, double *lon_deg, double *lat_deg)
{
    double bb = 1.0 - JME_BESSEL_EARTH_F;
    double rho = sqrt(p[0] * p[0] + p[1] * p[1]);
    /* surface point: geodetic latitude from the normal (x, y, z / b^2) */
    *lat_deg = atan2(p[2], bb * bb * rho) * JME_EVENTS_RAD_TO_DEG;
    *lon_deg = normalize_lon_180((atan2(p[1], p[0]) - b->gast) * JME_EVENTS_RAD_TO_DEG);
}

/* Point of the silhouette at parameter t. */
static void solar_bessel_silhouette_point(const double *e1, const double *e2, double t, double *p)
{
    double bb = 1.0 - JME_BESSEL_EARTH_F;
    double c = cos(t), s = sin(t);
    p[0] = c * e1[0] + s * e2[0];
    p[1] = c * e1[1] + s * e2[1];
    p[2] = bb * (c * e1[2] + s * e2[2]);
}

/* kind 0: penumbra F = dist - L1'; kind 1: umbra F = dist - |L2'|;
 * kind 2: plain distance from the axis. */
static double solar_bessel_silhouette_f(const solar_bessel *b, const double *p, int kind)
{
    double xi = vec_dot(p, b->xhat) - b->x;
    double eta = vec_dot(p, b->yhat) - b->y;
    double zeta = vec_dot(p, b->k);
    double dist = sqrt(xi * xi + eta * eta);
    if (kind == 0) return dist - (b->l1 - zeta * b->tanf1);
    if (kind == 1) return dist - fabs(b->l2 - zeta * b->tanf2);
    return dist;
}

static double solar_bessel_silhouette_min(const solar_bessel *b, int kind, double *best_point)
{
    double bb = 1.0 - JME_BESSEL_EARTH_F;
    double kp[3] = {b->k[0], b->k[1], b->k[2] / bb};
    double e1[3], e2[3], n, p[3];
    double best_t = 0.0, best_f = INFINITY, lo, hi, gr = 0.6180339887498948482;
    double x1, x2, f1, f2;
    int i;
    const int samples = 720;

    n = vec_norm(kp);
    for (i = 0; i < 3; i++) kp[i] /= n;
    /* e1 = normalize(kp x z), or kp x x when kp is polar */
    if (fabs(kp[2]) < 0.9) {
        e1[0] = kp[1]; e1[1] = -kp[0]; e1[2] = 0.0;
    } else {
        e1[0] = 0.0; e1[1] = kp[2]; e1[2] = -kp[1];
    }
    n = vec_norm(e1);
    for (i = 0; i < 3; i++) e1[i] /= n;
    e2[0] = kp[1] * e1[2] - kp[2] * e1[1];
    e2[1] = kp[2] * e1[0] - kp[0] * e1[2];
    e2[2] = kp[0] * e1[1] - kp[1] * e1[0];

    for (i = 0; i < samples; i++) {
        double t = 2.0 * JME_EVENTS_PI * i / samples;
        double f;
        solar_bessel_silhouette_point(e1, e2, t, p);
        f = solar_bessel_silhouette_f(b, p, kind);
        if (f < best_f) { best_f = f; best_t = t; }
    }
    lo = best_t - 2.0 * JME_EVENTS_PI / samples;
    hi = best_t + 2.0 * JME_EVENTS_PI / samples;
    x1 = hi - gr * (hi - lo);
    x2 = lo + gr * (hi - lo);
    solar_bessel_silhouette_point(e1, e2, x1, p); f1 = solar_bessel_silhouette_f(b, p, kind);
    solar_bessel_silhouette_point(e1, e2, x2, p); f2 = solar_bessel_silhouette_f(b, p, kind);
    for (i = 0; i < 200 && (hi - lo) > 1e-14; i++) {
        if (f1 > f2) {
            lo = x1; x1 = x2; f1 = f2; x2 = lo + gr * (hi - lo);
            solar_bessel_silhouette_point(e1, e2, x2, p); f2 = solar_bessel_silhouette_f(b, p, kind);
        } else {
            hi = x2; x2 = x1; f2 = f1; x1 = hi - gr * (hi - lo);
            solar_bessel_silhouette_point(e1, e2, x1, p); f1 = solar_bessel_silhouette_f(b, p, kind);
        }
    }
    best_t = 0.5 * (lo + hi);
    solar_bessel_silhouette_point(e1, e2, best_t, p);
    if (best_point != 0) for (i = 0; i < 3; i++) best_point[i] = p[i];
    return solar_bessel_silhouette_f(b, p, kind);
}

/* Signed contact functions: negative while the condition holds.
 * kind 0: penumbra on Earth; 1: umbra/antumbra on Earth; 2: axis on Earth;
 * 3: L2' at the central point (sign changes = hybrid transitions). */
static double solar_bessel_condition(double jd_ut, int kind, char *error)
{
    solar_bessel b;
    double disc, p[3];
    int central;

    if (solar_bessel_at(jd_ut, &b, error) != JME_OK) return NAN;
    central = solar_bessel_axis_point(&b, p, &disc);
    if (kind == 2) return -disc;
    if (kind == 3) {
        if (!central) return NAN;
        return b.l2 - vec_dot(p, b.k) * b.tanf2;
    }
    if (central) return -1.0;
    return solar_bessel_silhouette_min(&b, kind, 0);
}

/* Find the instant, on one side of t0 (where condition(t0) < 0), at which
 * the condition becomes >= 0. Bisection to 1e-10 d (8.6 microseconds). */
static int solar_bessel_contact(double t0, int kind, int left_side, double max_span, double *tret, char *error)
{
    double step = 0.005, prev = t0, f;
    int i, j;

    f = solar_bessel_condition(t0, kind, error);
    if (!isfinite(f) || f >= 0.0) return JME_ERR;
    for (i = 1; i * step <= max_span; i++) {
        double t = t0 + (left_side ? -1.0 : 1.0) * step * i;
        f = solar_bessel_condition(t, kind, error);
        if (!isfinite(f)) return JME_ERR;
        if (f >= 0.0) {
            double lo = prev, hi = t;
            for (j = 0; j < 200 && fabs(hi - lo) > 1e-10; j++) {
                double m = 0.5 * (lo + hi);
                double fm = solar_bessel_condition(m, kind, error);
                if (!isfinite(fm)) return JME_ERR;
                if (fm >= 0.0) hi = m; else lo = m;
            }
            *tret = 0.5 * (lo + hi);
            return JME_OK;
        }
        prev = t;
    }
    return JME_ERR;
}

/* Root of L2'(central point) between a and b (sign change assumed). */
static int solar_bessel_hybrid_transition(double a, double b, double *tret, char *error)
{
    double fa = solar_bessel_condition(a, 3, error);
    int j;
    if (!isfinite(fa)) return JME_ERR;
    for (j = 0; j < 200 && fabs(b - a) > 1e-10; j++) {
        double m = 0.5 * (a + b);
        double fm = solar_bessel_condition(m, 3, error);
        if (!isfinite(fm)) return JME_ERR;
        if ((fm < 0.0) == (fa < 0.0)) { a = m; fa = fm; } else { b = m; }
    }
    *tret = 0.5 * (a + b);
    return JME_OK;
}

/* Instantaneous global classification and representative surface point.
 * rc: TOTAL/ANNULAR (central or not), PARTIAL or 0. class: CENTRAL,
 * NONCENTRAL or 0. distance_km: distance of the axis from the surface
 * silhouette (0 when central). */
static int solar_bessel_classify(double jd_ut, int *rc_out, int *class_out, double *lon, double *lat, double *distance_km, char *error)
{
    solar_bessel b;
    double p[3], disc, f;
    int rc = 0, cls = 0;

    if (solar_bessel_at(jd_ut, &b, error) != JME_OK) return JME_ERR;
    if (solar_bessel_axis_point(&b, p, &disc)) {
        double l2p = b.l2 - vec_dot(p, b.k) * b.tanf2;
        rc = l2p < 0.0 ? JME_ECLIPSE_SOLAR_TOTAL : JME_ECLIPSE_SOLAR_ANNULAR;
        cls = JME_ECLIPSE_SOLAR_CENTRAL;
        if (distance_km != 0) *distance_km = 0.0;
    } else {
        double q[3];
        double dmin = solar_bessel_silhouette_min(&b, 2, q);
        if (distance_km != 0) *distance_km = dmin * JME_BESSEL_EARTH_A_KM;
        f = solar_bessel_silhouette_min(&b, 1, p);
        if (f < 0.0) {
            double l2p = b.l2 - vec_dot(p, b.k) * b.tanf2;
            rc = l2p < 0.0 ? JME_ECLIPSE_SOLAR_TOTAL : JME_ECLIPSE_SOLAR_ANNULAR;
            cls = JME_ECLIPSE_SOLAR_NONCENTRAL;
        } else if (solar_bessel_silhouette_min(&b, 0, p) < 0.0) {
            rc = JME_ECLIPSE_SOLAR_PARTIAL;
        }
        /* representative point: limb point nearest the axis */
        p[0] = q[0]; p[1] = q[1]; p[2] = q[2];
    }
    if (rc != 0 && lon != 0 && lat != 0) solar_bessel_geodetic(&b, p, lon, lat);
    if (rc_out != 0) *rc_out = rc;
    if (class_out != 0) *class_out = cls;
    return JME_OK;
}

static int jme_sol_eclipse_where_impl(double jd_ut, int flags, double *geopos, double *attr, char *error)
{
    double sep = NAN, sun_r = NAN, moon_r = NAN;
    double sun_alt = NAN, moon_alt = NAN;
    double lon = 0.0, lat = 0.0, distance_km = NAN;
    int shadow_class = 0;
    int rc = 0;
    int i;
    (void)flags;

    if (geopos == 0 || attr == 0) {
        jme_set_error(error, "Solar eclipse geographic circumstance output is required");
        return JME_ERR;
    }
    if (!isfinite(jd_ut)) {
        jme_set_error(error, "Julian date must be finite");
        return JME_ERR;
    }
    if (solar_bessel_classify(jd_ut, &rc, &shadow_class, &lon, &lat, &distance_km, error) != JME_OK) {
        return JME_ERR;
    }
    if (rc == 0) {
        jme_set_error(error, "No solar eclipse is in progress at the supplied time");
        return JME_ERR;
    }

    /* Central point (axis on the surface) for central eclipses; otherwise
     * the point of the Earth's limb nearest the shadow axis. */
    geopos[0] = lon;
    geopos[1] = lat;
    geopos[2] = 0.0;

    if (solar_eclipse_local_geometry(jd_ut, geopos, &sep, &sun_r, &moon_r, &sun_alt, &moon_alt, error) != JME_OK) {
        return JME_ERR;
    }
    for (i = 0; i < 20; i++) attr[i] = 0.0;
    /* attr[0] magnitude: fraction of the solar diameter covered (Moon/Sun
     * diameter ratio for a central total/annular point);
     * attr[1] obscuration: fraction of the solar disc area covered. */
    attr[0] = (sun_r + moon_r - sep) / (2.0 * sun_r);
    if (attr[0] < 0.0) attr[0] = 0.0;
    attr[1] = solar_obscuration(sep, sun_r, moon_r);
    attr[2] = sep;
    attr[3] = 2.0 * sun_r * 3600.0;
    attr[4] = 2.0 * moon_r * 3600.0;
    attr[5] = sun_alt;
    attr[6] = moon_alt;
    attr[7] = isfinite(distance_km) ? distance_km : 0.0;
    attr[8] = (double)shadow_class;
    /* attr[9]: ratio of apparent lunar to solar diameter (the magnitude
     * quoted for central eclipses by the NASA catalogues). */
    attr[9] = moon_r / sun_r;
    return rc;
}

static int solar_type_filter_valid(int filter)
{
    return filter == 0 || filter == JME_ECLIPSE_SOLAR_TOTAL || filter == JME_ECLIPSE_SOLAR_ANNULAR
        || filter == JME_ECLIPSE_SOLAR_HYBRID || filter == JME_ECLIPSE_SOLAR_PARTIAL
        || filter == JME_ECLIPSE_SOLAR_CENTRAL || filter == JME_ECLIPSE_SOLAR_NONCENTRAL;
}

static int solar_type_filter_match(int filter, int rc, int cls)
{
    if (filter == 0) return 1;
    if (filter == JME_ECLIPSE_SOLAR_CENTRAL) return cls == JME_ECLIPSE_SOLAR_CENTRAL;
    if (filter == JME_ECLIPSE_SOLAR_NONCENTRAL) return cls == JME_ECLIPSE_SOLAR_NONCENTRAL;
    return rc == filter;
}

/*
 * Next (or previous) solar eclipse anywhere on Earth.
 *   epheflag: eclipse-type filter, 0 = any, or one of JME_ECLIPSE_SOLAR_TOTAL,
 *             _ANNULAR, _HYBRID, _PARTIAL, _CENTRAL, _NONCENTRAL; any other
 *             value is an error.
 *   tret[0] greatest eclipse (minimum distance of the shadow axis from the
 *           Earth's centre), UT;
 *   tret[1] same instant (kept for layout compatibility);
 *   tret[2] / tret[3] first / last contact of the penumbra with the Earth
 *           (P1 / P4);
 *   tret[4] / tret[5] first / last contact of the umbra or antumbra with
 *           the Earth (U1 / U4), 0 for partial eclipses;
 *   tret[6] / tret[7] central line begins / ends (axis meets the surface),
 *           0 for non-central eclipses;
 *   tret[8] / tret[9] for hybrid eclipses, the instants at which the
 *           central phase changes between annular and total (first and
 *           second change), 0 otherwise.
 * Return value: JME_ECLIPSE_SOLAR_TOTAL / _ANNULAR / _HYBRID / _PARTIAL.
 */
static int jme_sol_eclipse_when_glob_impl(double jd_start, int flags, int epheflag, double *tret, int backward, char *error)
{
    double guess = 0.0;
    double search_jd = jd_start;
    double direction = backward ? -1.0 : 1.0;
    double tmax = 0.0;
    int rc, cls;
    int attempt;
    (void)flags;

    if (tret == 0) {
        jme_set_error(error, "Solar eclipse time output is required");
        return JME_ERR;
    }
    if (!isfinite(jd_start)) {
        jme_set_error(error, "Julian date must be finite");
        return JME_ERR;
    }
    if (!solar_type_filter_valid(epheflag)) {
        jme_set_error(error, "Unknown solar eclipse type filter");
        return JME_ERR;
    }

    /* At most 12 eclipses of any type per 30 lunations; a filtered search
     * (e.g. hybrid) may need many years. */
    for (attempt = 0; attempt < 2000; attempt++) {
        if (find_new_moon_conjunction(search_jd, backward, &guess, error) != JME_OK) {
            return JME_ERR;
        }
        if (refine_solar_maximum(guess, 0, 0, &tmax, error) != JME_OK) {
            return JME_ERR;
        }
        if (solar_bessel_classify(tmax, &rc, &cls, 0, 0, 0, error) != JME_OK) {
            return JME_ERR;
        }
        if (rc != 0) {
            double c1 = 0.0, c2 = 0.0;
            memset(tret, 0, sizeof(double) * 10);
            tret[0] = tmax;
            tret[1] = tmax;
            if (cls == JME_ECLIPSE_SOLAR_CENTRAL) {
                double s0 = solar_bessel_condition(tmax, 3, error);
                if (solar_bessel_contact(tmax, 2, 1, 0.4, &c1, error) != JME_OK
                    || solar_bessel_contact(tmax, 2, 0, 0.4, &c2, error) != JME_OK) {
                    return JME_ERR;
                }
                tret[6] = c1;
                tret[7] = c2;
                {
                    /* the type at the ends of the central line: L2' just
                     * inside C1 / C2 */
                    double eps_t = 1e-7;
                    double sa = solar_bessel_condition(c1 + eps_t, 3, error);
                    double sb = solar_bessel_condition(c2 - eps_t, 3, error);
                    if (!isfinite(s0) || !isfinite(sa) || !isfinite(sb)) return JME_ERR;
                    if ((sa < 0.0) != (s0 < 0.0) || (sb < 0.0) != (s0 < 0.0)) {
                        rc = JME_ECLIPSE_SOLAR_HYBRID;
                        if ((sa < 0.0) != (s0 < 0.0)
                            && solar_bessel_hybrid_transition(c1 + eps_t, tmax, &tret[8], error) != JME_OK) {
                            return JME_ERR;
                        }
                        if ((sb < 0.0) != (s0 < 0.0)
                            && solar_bessel_hybrid_transition(tmax, c2 - eps_t, &tret[9], error) != JME_OK) {
                            return JME_ERR;
                        }
                        if (tret[8] == 0.0) { tret[8] = tret[9]; tret[9] = 0.0; }
                    }
                }
            }
            if (!solar_type_filter_match(epheflag, rc, cls)) {
                search_jd = guess + direction;
                continue;
            }
            if (solar_bessel_contact(tmax, 0, 1, 0.5, &tret[2], error) != JME_OK
                || solar_bessel_contact(tmax, 0, 0, 0.5, &tret[3], error) != JME_OK) {
                return JME_ERR;
            }
            if (rc != JME_ECLIPSE_SOLAR_PARTIAL) {
                if (solar_bessel_contact(tmax, 1, 1, 0.5, &tret[4], error) != JME_OK
                    || solar_bessel_contact(tmax, 1, 0, 0.5, &tret[5], error) != JME_OK) {
                    return JME_ERR;
                }
            }
            if (error != 0) error[0] = '\0';
            return rc;
        }
        search_jd = guess + direction;
    }

    jme_set_error(error, "No solar eclipse found in search window");
    return JME_ERR;
}

static int jme_lun_eclipse_when_impl(double jd_start, int flags, int iflag, double *tret, int backward, char *error)
{
    double guess = 0.0;
    double search_jd = jd_start;
    double direction = backward ? -1.0 : 1.0;
    double tmax = 0.0;
    double umbral_mag = 0.0;
    double penumbral_mag = 0.0;
    double umbra_km = 0.0;
    double penumbra_km = 0.0;
    double offset_km = 0.0;
    double axis_km = 0.0;
    int eclipse_type;
    int attempt;
    (void)flags;
    (void)iflag;

    if (tret == 0) {
        jme_set_error(error, "Lunar eclipse time output is required");
        return JME_ERR;
    }

    for (attempt = 0; attempt < 30; attempt++) {
        if (find_full_moon_opposition(search_jd, backward, &guess, error) != JME_OK) {
            return JME_ERR;
        }
        if (refine_lunar_eclipse_maximum(guess, &tmax, error) != JME_OK) {
            return JME_ERR;
        }
        if (lunar_eclipse_geometry(tmax, &umbral_mag, &penumbral_mag, &umbra_km, &penumbra_km, &offset_km, &axis_km, error) != JME_OK) {
            return JME_ERR;
        }

        eclipse_type = classify_lunar_eclipse(umbral_mag, penumbral_mag);
        if (eclipse_type != 0 && axis_km > 0.0) {
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

        search_jd = guess + direction;
    }

    jme_set_error(error, "No lunar eclipse found in search window");
    return JME_ERR;
}

static int jme_lun_occult_where_impl(double jd_ut, int body, const char *starname, int flags, double *geopos, double *attr, char *error)
{
    double moon_eq[6];
    double target_eq[6];
    double moon_r = 0.0;
    double target_r = 0.0;
    double sep = 0.0;
    double threshold = 0.0;
    double parallax = 0.0;
    double gst;
    int i;
    (void)flags;

    if (geopos == 0 || attr == 0) {
        jme_set_error(error, "Lunar occultation geographic circumstance output is required");
        return JME_ERR;
    }

    if (moon_target_state(jd_ut, body, starname, 0, 0, 0, moon_eq, target_eq, &moon_r, &target_r, 0, 0, error) != JME_OK) {
        return JME_ERR;
    }

    sep = jme_spherical_angular_separation(moon_eq[0], moon_eq[1], target_eq[0], target_eq[1]);
    parallax = lunar_horizontal_parallax_deg(moon_r);
    threshold = moon_r + target_r + parallax;
    if (sep > threshold) {
        jme_set_error(error, "No lunar occultation is in progress at the supplied time");
        return JME_ERR;
    }

    gst = events_gst_hours(jd_ut, 0) * 15.0;
    geopos[0] = normalize_lon_180(target_eq[0] - gst);
    geopos[1] = target_eq[1];
    geopos[2] = 0.0;

    for (i = 0; i < 20; i++) {
        attr[i] = 0.0;
    }

    attr[0] = (threshold - sep) / (2.0 * moon_r);
    attr[1] = target_r > 0.0 ? (target_r * target_r) / (moon_r * moon_r) : 0.0;
    attr[2] = sep;
    attr[3] = 2.0 * moon_r * 3600.0;
    attr[4] = 2.0 * target_r * 3600.0;
    attr[7] = sep <= fabs(moon_r - target_r) ? 1.0 : 0.0;
    attr[9] = parallax;

    return JME_OK;
}

static int jme_lun_occult_when_loc_impl(double jd_start, int body, const char *starname, int flags, double *geopos, double *tret, double *attr, int backward, char *error)
{
    double guess = 0.0;
    double tmax = 0.0;
    double moon_eq[6];
    double target_eq[6];
    double moon_r = 0.0;
    double target_r = 0.0;
    double moon_alt = -90.0;
    double target_alt = -90.0;
    double sep = 0.0;
    double threshold = 0.0;
    int i;
    (void)flags;

    if (geopos == 0 || tret == 0 || attr == 0) {
        jme_set_error(error, "Lunar occultation local search requires geopos, tret, and attr outputs");
        return JME_ERR;
    }

    if (find_moon_target_conjunction(jd_start, body, starname, backward, &guess, error) != JME_OK
        || refine_occultation_maximum(guess, body, starname, geopos, 1, &tmax, error) != JME_OK
        || moon_target_state(tmax, body, starname, 0, 1, geopos, moon_eq, target_eq, &moon_r, &target_r, &moon_alt, &target_alt, error) != JME_OK) {
        return JME_ERR;
    }

    sep = jme_spherical_angular_separation(moon_eq[0], moon_eq[1], target_eq[0], target_eq[1]);
    threshold = moon_r + target_r;
    if (sep > threshold || moon_alt <= 0.0 || target_alt <= 0.0) {
        jme_set_error(error, "No local lunar occultation found near the supplied date");
        return JME_ERR;
    }

    memset(tret, 0, sizeof(double) * 10);
    for (i = 0; i < 20; i++) {
        attr[i] = 0.0;
    }

    tret[0] = tmax;
    tret[1] = tmax;
    find_occultation_contact_time(tmax, body, starname, threshold, geopos, 1, 1, &tret[2], error);
    find_occultation_contact_time(tmax, body, starname, threshold, geopos, 1, 0, &tret[3], error);
    if (sep <= fabs(moon_r - target_r)) {
        double inner = fabs(moon_r - target_r);
        find_occultation_contact_time(tmax, body, starname, inner, geopos, 1, 1, &tret[4], error);
        find_occultation_contact_time(tmax, body, starname, inner, geopos, 1, 0, &tret[5], error);
    }

    attr[0] = (threshold - sep) / (2.0 * moon_r);
    attr[1] = target_r > 0.0 ? (target_r * target_r) / (moon_r * moon_r) : 0.0;
    attr[2] = sep;
    attr[3] = 2.0 * moon_r * 3600.0;
    attr[4] = 2.0 * target_r * 3600.0;
    attr[5] = moon_alt;
    attr[6] = target_alt;
    attr[7] = sep <= fabs(moon_r - target_r) ? 1.0 : 0.0;
    attr[8] = JME_ECLIPSE_VISIBLE;

    return JME_OK;
}

static int jme_lun_occult_when_glob_impl(double jd_start, int body, const char *starname, int flags, int iflag, double *tret, int backward, char *error)
{
    double guess = 0.0;
    double tmax = 0.0;
    double moon_eq[6];
    double target_eq[6];
    double moon_r = 0.0;
    double target_r = 0.0;
    double sep = 0.0;
    double threshold = 0.0;
    double parallax = 0.0;
    (void)flags;
    (void)iflag;

    if (tret == 0) {
        jme_set_error(error, "Lunar occultation time output is required");
        return JME_ERR;
    }

    if (find_moon_target_conjunction(jd_start, body, starname, backward, &guess, error) != JME_OK
        || refine_occultation_maximum(guess, body, starname, 0, 0, &tmax, error) != JME_OK
        || moon_target_state(tmax, body, starname, 0, 0, 0, moon_eq, target_eq, &moon_r, &target_r, 0, 0, error) != JME_OK) {
        return JME_ERR;
    }

    sep = jme_spherical_angular_separation(moon_eq[0], moon_eq[1], target_eq[0], target_eq[1]);
    parallax = lunar_horizontal_parallax_deg(moon_r);
    threshold = moon_r + target_r + parallax;
    if (sep > threshold) {
        jme_set_error(error, "No lunar occultation found near the supplied date");
        return JME_ERR;
    }

    memset(tret, 0, sizeof(double) * 10);
    tret[0] = tmax;
    tret[1] = tmax;
    find_occultation_contact_time(tmax, body, starname, threshold, 0, 0, 1, &tret[2], error);
    find_occultation_contact_time(tmax, body, starname, threshold, 0, 0, 0, &tret[3], error);
    if (sep <= fabs(moon_r - target_r)) {
        double inner = fabs(moon_r - target_r);
        find_occultation_contact_time(tmax, body, starname, inner, 0, 0, 1, &tret[4], error);
        find_occultation_contact_time(tmax, body, starname, inner, 0, 0, 0, &tret[5], error);
    }

    return JME_OK;
}

static int jme_rise_trans_true_hor_impl(double jd_ut, int body, const char *starname, int flags, int rsmi, double *geopos, double atpress, double attemp, double horhgt, double *tret, char *error)
{
    return rise_trans_search(jd_ut, body, starname, flags, rsmi, geopos, atpress, attemp, 1, horhgt, tret, error);
}

static int jme_solcross_ut_impl(double x2cross, double jd_ut, int flags, double *tret, char *error)
{
    return jme_solcross(x2cross, jd_ut, flags, tret, error);
}

static int jme_mooncross_ut_impl(double x2cross, double jd_ut, int flags, double *tret, char *error)
{
    return jme_mooncross(x2cross, jd_ut, flags, tret, error);
}

static int jme_mooncross_node_impl(double jd_ut, int flags, double *tret, char *error)
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

static int jme_mooncross_node_ut_impl(double jd_ut, int flags, double *tret, char *error)
{
    return jme_mooncross_node(jd_ut, flags, tret, error);
}

static int jme_helio_cross_impl(int body, double x2cross, double jd_ut, int flags, double *tret, char *error)
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

static int jme_helio_cross_ut_impl(int body, double x2cross, double jd_ut, int flags, double *tret, char *error)
{
    return jme_helio_cross(body, x2cross, jd_ut, flags, tret, error);
}

static int jme_nod_aps_ut_impl(double jd_ut, int body, int flags, int method, double *tret, char *error)
{
    double jd_et;
    if (tret == 0) {
        jme_set_error(error, "Nodal/apsidal event output is required");
        return JME_ERR;
    }
    *tret = NAN;
    if (internal_ut_to_et(jd_ut, &jd_et, error) != JME_OK) return JME_ERR;
    return jme_nod_aps(jd_et, body, flags, method, tret, error);
}

static int jme_heliacal_pheno_ut_impl(double jd_ut, double *geopos, double *dat_hel, char *error)
{
    int body;
    double sun_alt;
    double body_alt;
    double sun_eq[6];
    double body_eq[6];
    double pheno[20];
    double arcus;
    double elongation;
    double limiting_mag;
    double required_arcus;
    int i;

    if (geopos == 0 || dat_hel == 0) {
        jme_set_error(error, "Heliacal phenomena require geopos and output data");
        return JME_ERR;
    }

    if (heliacal_body_from_data(dat_hel, &body, error) != JME_OK) {
        return JME_ERR;
    }

    sun_alt = get_altitude(jd_ut, JME_BODY_SUN, 0, 0, geopos, 0.0, 0.0, error);
    body_alt = get_altitude(jd_ut, body, 0, 0, geopos, 0.0, 0.0, error);
    if (sun_alt <= -99.0 || body_alt <= -99.0) {
        return JME_ERR;
    }

    if (jme_calc_ut(jd_ut, JME_BODY_SUN, JME_CALC_EQUATORIAL, sun_eq, error) != JME_OK
        || jme_calc_ut(jd_ut, body, JME_CALC_EQUATORIAL, body_eq, error) != JME_OK
        || jme_pheno_ut(jd_ut, body, 0, pheno, error) != JME_OK) {
        return JME_ERR;
    }

    arcus = body_alt - sun_alt;
    elongation = jme_spherical_angular_separation(sun_eq[0], sun_eq[1], body_eq[0], body_eq[1]);
    limiting_mag = heliacal_limiting_magnitude(arcus);
    required_arcus = heliacal_required_arcus(pheno[4]);
    if (!isfinite(limiting_mag) || !isfinite(required_arcus)) {
        jme_set_error(error, "Heliacal visibility model produced non-finite thresholds");
        return JME_ERR;
    }

    for (i = 0; i < 20; i++) {
        dat_hel[i] = 0.0;
    }
    dat_hel[0] = jd_ut;
    dat_hel[1] = (double)body;
    dat_hel[2] = sun_alt;
    dat_hel[3] = body_alt;
    dat_hel[4] = arcus;
    dat_hel[5] = limiting_mag;
    dat_hel[6] = pheno[4];
    dat_hel[7] = elongation;
    dat_hel[8] = pheno[3];
    dat_hel[9] = heliacal_visibility_flag(body_alt, sun_alt, arcus, pheno[4], limiting_mag, required_arcus) ? 1.0 : 0.0;
    dat_hel[10] = required_arcus;

    return JME_OK;
}

static double jme_heliacal_angle_impl(double jd_ut, double *geopos, double *dat_hel, char *error)
{
    double local[20] = {0.0};
    double *out = dat_hel != 0 ? dat_hel : local;

    if (dat_hel == 0) {
        local[0] = JME_BODY_VENUS;
    }

    if (jme_heliacal_pheno_ut(jd_ut, geopos, out, error) != JME_OK) {
        return NAN;
    }

    return out[7];
}

static double jme_topo_arcus_visionis_impl(double jd_ut, double *geopos, double *dat_hel, char *error)
{
    double local[20] = {0.0};
    double *out = dat_hel != 0 ? dat_hel : local;

    if (dat_hel == 0) {
        local[0] = JME_BODY_VENUS;
    }

    if (jme_heliacal_pheno_ut(jd_ut, geopos, out, error) != JME_OK) {
        return NAN;
    }

    return out[4];
}

static int jme_lun_eclipse_when_loc_impl(double jd_start, int flags, double *geopos, double *tret, double *attr, int backward, char *error)
{
    int rc;
    double altitude = -90.0;
    double global_tret[10];
    double local_tmax;
    double eclipse_start = 0.0;
    double eclipse_end = 0.0;
    double visible_start = 0.0;
    double visible_end = 0.0;
    int i;

    if (geopos == 0 || tret == 0 || attr == 0) {
        jme_set_error(error, "Lunar eclipse local search requires geopos, tret, and attr outputs");
        return JME_ERR;
    }

    rc = jme_lun_eclipse_when(jd_start, flags, 0, global_tret, backward, error);
    if (rc == JME_ERR) {
        return JME_ERR;
    }

    for (i = 2; i < 8; i += 2) {
        if (global_tret[i] != 0.0 && (eclipse_start == 0.0 || global_tret[i] < eclipse_start)) {
            eclipse_start = global_tret[i];
        }
    }
    for (i = 3; i < 8; i += 2) {
        if (global_tret[i] != 0.0 && global_tret[i] > eclipse_end) {
            eclipse_end = global_tret[i];
        }
    }

    if (eclipse_start == 0.0 || eclipse_end == 0.0 || eclipse_end <= eclipse_start) {
        eclipse_start = global_tret[0] - 0.25;
        eclipse_end = global_tret[0] + 0.25;
    }

    if (find_visible_moon_interval(eclipse_start, eclipse_end, geopos, &visible_start, &visible_end, error) != JME_OK) {
        return JME_ERR;
    }

    local_tmax = global_tret[0];
    if (local_tmax < visible_start) {
        local_tmax = visible_start;
    }
    if (local_tmax > visible_end) {
        local_tmax = visible_end;
    }

    if (jme_lun_eclipse_how(local_tmax, flags, geopos, attr, error) == JME_ERR) {
        return JME_ERR;
    }

    for (i = 0; i < 10; i++) {
        tret[i] = global_tret[i];
    }

    tret[0] = local_tmax;
    tret[1] = global_tret[0];
    for (i = 2; i < 8; i += 2) {
        if (tret[i] != 0.0 && tret[i] < visible_start) {
            tret[i] = visible_start;
        }
    }
    for (i = 3; i < 8; i += 2) {
        if (tret[i] != 0.0 && tret[i] > visible_end) {
            tret[i] = visible_end;
        }
    }
    tret[8] = visible_start;
    tret[9] = visible_end;

    attr[8] = moon_is_above_horizon(local_tmax, geopos, error, &altitude) ? JME_ECLIPSE_VISIBLE : 0.0;
    attr[7] = altitude;
    attr[9] = (visible_end - visible_start) * 24.0;

    return rc;
}

static int jme_sol_eclipse_how_impl(double jd_ut, int flags, double *geopos, double *attr, char *error)
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
    if (rc == 0 || sun_alt <= JME_SOLAR_VISIBLE_UPPER_LIMB_ALTITUDE_DEG) {
        jme_set_error(error, "No local solar eclipse is in progress at the supplied time");
        return JME_ERR;
    }

    for (i = 0; i < 20; i++) {
        attr[i] = 0.0;
    }

    attr[0] = (sun_r + moon_r - sep) / (2.0 * sun_r);
    attr[1] = solar_obscuration(sep, sun_r, moon_r);
    attr[2] = sep;
    attr[3] = 2.0 * sun_r * 3600.0;
    attr[4] = 2.0 * moon_r * 3600.0;
    attr[5] = sun_alt;
    attr[6] = moon_alt;
    attr[7] = sep <= fabs(sun_r - moon_r) ? 1.0 : 0.0;
    attr[8] = JME_ECLIPSE_VISIBLE;

    return rc;
}

static int jme_lun_eclipse_how_impl(double jd_ut, int flags, double *geopos, double *attr, char *error)
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

static int jme_nod_aps_impl(double jd_et, int body, int flags, int method, double *tret, char *error)
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
    if (!isfinite(jd_et)) {
        jme_set_error(error, "Terrestrial-time Julian date must be finite");
        return JME_ERR;
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

static int jme_get_orbital_elements_impl(double jd_et, int body, int flags, double *elem, char *error)
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

static int jme_heliacal_ut_impl(double jd_ut, double *geopos, double *dat_hel, char *error)
{
    int body;
    double best_t = 0.0;
    double best_data[20] = {0.0};
    int i;

    if (geopos == 0 || dat_hel == 0) {
        jme_set_error(error, "Heliacal visibility requires geopos and output data");
        return JME_ERR;
    }

    if (heliacal_body_from_data(dat_hel, &body, error) != JME_OK) {
        return JME_ERR;
    }

    for (i = 0; i <= 370; i++) {
        double day = jd_ut + (double)i;
        double candidates[2] = {0.0, 0.0};
        double pheno[20];
        double required_arcus;
        int n = 0;
        double t;

        if (jme_pheno_ut(day + 0.5, body, 0, pheno, error) != JME_OK) {
            return JME_ERR;
        }
        required_arcus = heliacal_required_arcus(pheno[4]);
        if (!isfinite(required_arcus)) {
            jme_set_error(error, "Heliacal visibility search requires finite body magnitude");
            return JME_ERR;
        }

        if (jme_rise_trans_true_hor(day, JME_BODY_SUN, 0, 0, JME_RISE_SET, geopos, 0.0, 0.0, -required_arcus, &t, error) == JME_OK) {
            candidates[n++] = t;
        }
        if (jme_rise_trans_true_hor(day, JME_BODY_SUN, 0, 0, JME_RISE_RISE, geopos, 0.0, 0.0, -required_arcus, &t, error) == JME_OK) {
            candidates[n++] = t;
        }

        while (n-- > 0) {
            double tmp[20] = {0.0};
            tmp[0] = (double)body;
            if (candidates[n] < jd_ut) {
                continue;
            }
            if (jme_heliacal_pheno_ut(candidates[n], geopos, tmp, error) != JME_OK) {
                return JME_ERR;
            }
            if (tmp[9] > 0.0 && (best_t == 0.0 || candidates[n] < best_t)) {
                best_t = candidates[n];
                memcpy(best_data, tmp, sizeof(best_data));
            }
        }

        if (best_t != 0.0) {
            memcpy(dat_hel, best_data, sizeof(best_data));
            return JME_OK;
        }
    }

    jme_set_error(error, "No heliacal visibility event found in search window");
    return JME_ERR;
}

static int jme_vis_limit_mag_impl(double jd_ut, double *geopos, double *dat_hel, char *error)
{
    if (dat_hel == 0) {
        jme_set_error(error, "Visual limiting magnitude requires output data");
        return JME_ERR;
    }

    if (jme_heliacal_pheno_ut(jd_ut, geopos, dat_hel, error) != JME_OK) {
        return JME_ERR;
    }

    return JME_OK;
}

/*
 * Public entry points: each call runs with a scoped, thread-local observer
 * (geopos) and restores the caller's state, so these routines never modify
 * the user's jme_set_topo() configuration.
 */
int jme_rise_trans(double jd_ut, int body, const char *starname, int flags, int rsmi, double *geopos, double atpress, double attemp, double *tret, char *error)
{
    double saved_topo[4];
    int result;
    internal_topo_override_save(saved_topo);
    result = jme_rise_trans_impl(jd_ut, body, starname, flags, rsmi, geopos, atpress, attemp, tret, error);
    internal_topo_override_restore(saved_topo);
    return result;
}

int jme_solcross(double x2cross, double jd_ut, int flags, double *tret, char *error)
{
    double saved_topo[4];
    int result;
    internal_topo_override_save(saved_topo);
    result = jme_solcross_impl(x2cross, jd_ut, flags, tret, error);
    internal_topo_override_restore(saved_topo);
    return result;
}

int jme_mooncross(double x2cross, double jd_ut, int flags, double *tret, char *error)
{
    double saved_topo[4];
    int result;
    internal_topo_override_save(saved_topo);
    result = jme_mooncross_impl(x2cross, jd_ut, flags, tret, error);
    internal_topo_override_restore(saved_topo);
    return result;
}

int jme_sol_eclipse_when_loc(double jd_start, int flags, double *geopos, double *tret, double *attr, int backward, char *error)
{
    double saved_topo[4];
    int result;
    internal_topo_override_save(saved_topo);
    result = jme_sol_eclipse_when_loc_impl(jd_start, flags, geopos, tret, attr, backward, error);
    internal_topo_override_restore(saved_topo);
    return result;
}

int jme_sol_eclipse_where(double jd_ut, int flags, double *geopos, double *attr, char *error)
{
    double saved_topo[4];
    int result;
    internal_topo_override_save(saved_topo);
    result = jme_sol_eclipse_where_impl(jd_ut, flags, geopos, attr, error);
    internal_topo_override_restore(saved_topo);
    return result;
}

int jme_sol_eclipse_when_glob(double jd_start, int flags, int epheflag, double *tret, int backward, char *error)
{
    double saved_topo[4];
    int result;
    internal_topo_override_save(saved_topo);
    result = jme_sol_eclipse_when_glob_impl(jd_start, flags, epheflag, tret, backward, error);
    internal_topo_override_restore(saved_topo);
    return result;
}

int jme_lun_eclipse_when(double jd_start, int flags, int iflag, double *tret, int backward, char *error)
{
    double saved_topo[4];
    int result;
    internal_topo_override_save(saved_topo);
    result = jme_lun_eclipse_when_impl(jd_start, flags, iflag, tret, backward, error);
    internal_topo_override_restore(saved_topo);
    return result;
}

int jme_lun_occult_where(double jd_ut, int body, const char *starname, int flags, double *geopos, double *attr, char *error)
{
    double saved_topo[4];
    int result;
    internal_topo_override_save(saved_topo);
    result = jme_lun_occult_where_impl(jd_ut, body, starname, flags, geopos, attr, error);
    internal_topo_override_restore(saved_topo);
    return result;
}

int jme_lun_occult_when_loc(double jd_start, int body, const char *starname, int flags, double *geopos, double *tret, double *attr, int backward, char *error)
{
    double saved_topo[4];
    int result;
    internal_topo_override_save(saved_topo);
    result = jme_lun_occult_when_loc_impl(jd_start, body, starname, flags, geopos, tret, attr, backward, error);
    internal_topo_override_restore(saved_topo);
    return result;
}

int jme_lun_occult_when_glob(double jd_start, int body, const char *starname, int flags, int iflag, double *tret, int backward, char *error)
{
    double saved_topo[4];
    int result;
    internal_topo_override_save(saved_topo);
    result = jme_lun_occult_when_glob_impl(jd_start, body, starname, flags, iflag, tret, backward, error);
    internal_topo_override_restore(saved_topo);
    return result;
}

int jme_rise_trans_true_hor(double jd_ut, int body, const char *starname, int flags, int rsmi, double *geopos, double atpress, double attemp, double horhgt, double *tret, char *error)
{
    double saved_topo[4];
    int result;
    internal_topo_override_save(saved_topo);
    result = jme_rise_trans_true_hor_impl(jd_ut, body, starname, flags, rsmi, geopos, atpress, attemp, horhgt, tret, error);
    internal_topo_override_restore(saved_topo);
    return result;
}

int jme_solcross_ut(double x2cross, double jd_ut, int flags, double *tret, char *error)
{
    double saved_topo[4];
    int result;
    internal_topo_override_save(saved_topo);
    result = jme_solcross_ut_impl(x2cross, jd_ut, flags, tret, error);
    internal_topo_override_restore(saved_topo);
    return result;
}

int jme_mooncross_ut(double x2cross, double jd_ut, int flags, double *tret, char *error)
{
    double saved_topo[4];
    int result;
    internal_topo_override_save(saved_topo);
    result = jme_mooncross_ut_impl(x2cross, jd_ut, flags, tret, error);
    internal_topo_override_restore(saved_topo);
    return result;
}

int jme_mooncross_node(double jd_ut, int flags, double *tret, char *error)
{
    double saved_topo[4];
    int result;
    internal_topo_override_save(saved_topo);
    result = jme_mooncross_node_impl(jd_ut, flags, tret, error);
    internal_topo_override_restore(saved_topo);
    return result;
}

int jme_mooncross_node_ut(double jd_ut, int flags, double *tret, char *error)
{
    double saved_topo[4];
    int result;
    internal_topo_override_save(saved_topo);
    result = jme_mooncross_node_ut_impl(jd_ut, flags, tret, error);
    internal_topo_override_restore(saved_topo);
    return result;
}

int jme_helio_cross(int body, double x2cross, double jd_ut, int flags, double *tret, char *error)
{
    double saved_topo[4];
    int result;
    internal_topo_override_save(saved_topo);
    result = jme_helio_cross_impl(body, x2cross, jd_ut, flags, tret, error);
    internal_topo_override_restore(saved_topo);
    return result;
}

int jme_helio_cross_ut(int body, double x2cross, double jd_ut, int flags, double *tret, char *error)
{
    double saved_topo[4];
    int result;
    internal_topo_override_save(saved_topo);
    result = jme_helio_cross_ut_impl(body, x2cross, jd_ut, flags, tret, error);
    internal_topo_override_restore(saved_topo);
    return result;
}

int jme_nod_aps_ut(double jd_ut, int body, int flags, int method, double *tret, char *error)
{
    double saved_topo[4];
    int result;
    internal_topo_override_save(saved_topo);
    result = jme_nod_aps_ut_impl(jd_ut, body, flags, method, tret, error);
    internal_topo_override_restore(saved_topo);
    return result;
}

int jme_heliacal_pheno_ut(double jd_ut, double *geopos, double *dat_hel, char *error)
{
    double saved_topo[4];
    int result;
    internal_topo_override_save(saved_topo);
    result = jme_heliacal_pheno_ut_impl(jd_ut, geopos, dat_hel, error);
    internal_topo_override_restore(saved_topo);
    return result;
}

double jme_heliacal_angle(double jd_ut, double *geopos, double *dat_hel, char *error)
{
    double saved_topo[4];
    double result;
    internal_topo_override_save(saved_topo);
    result = jme_heliacal_angle_impl(jd_ut, geopos, dat_hel, error);
    internal_topo_override_restore(saved_topo);
    return result;
}

double jme_topo_arcus_visionis(double jd_ut, double *geopos, double *dat_hel, char *error)
{
    double saved_topo[4];
    double result;
    internal_topo_override_save(saved_topo);
    result = jme_topo_arcus_visionis_impl(jd_ut, geopos, dat_hel, error);
    internal_topo_override_restore(saved_topo);
    return result;
}

int jme_lun_eclipse_when_loc(double jd_start, int flags, double *geopos, double *tret, double *attr, int backward, char *error)
{
    double saved_topo[4];
    int result;
    internal_topo_override_save(saved_topo);
    result = jme_lun_eclipse_when_loc_impl(jd_start, flags, geopos, tret, attr, backward, error);
    internal_topo_override_restore(saved_topo);
    return result;
}

int jme_sol_eclipse_how(double jd_ut, int flags, double *geopos, double *attr, char *error)
{
    double saved_topo[4];
    int result;
    internal_topo_override_save(saved_topo);
    result = jme_sol_eclipse_how_impl(jd_ut, flags, geopos, attr, error);
    internal_topo_override_restore(saved_topo);
    return result;
}

int jme_lun_eclipse_how(double jd_ut, int flags, double *geopos, double *attr, char *error)
{
    double saved_topo[4];
    int result;
    internal_topo_override_save(saved_topo);
    result = jme_lun_eclipse_how_impl(jd_ut, flags, geopos, attr, error);
    internal_topo_override_restore(saved_topo);
    return result;
}

int jme_nod_aps(double jd_et, int body, int flags, int method, double *tret, char *error)
{
    double saved_topo[4];
    int result;
    internal_topo_override_save(saved_topo);
    result = jme_nod_aps_impl(jd_et, body, flags, method, tret, error);
    internal_topo_override_restore(saved_topo);
    return result;
}

int jme_get_orbital_elements(double jd_et, int body, int flags, double *elem, char *error)
{
    double saved_topo[4];
    int result;
    internal_topo_override_save(saved_topo);
    result = jme_get_orbital_elements_impl(jd_et, body, flags, elem, error);
    internal_topo_override_restore(saved_topo);
    return result;
}

int jme_heliacal_ut(double jd_ut, double *geopos, double *dat_hel, char *error)
{
    double saved_topo[4];
    int result;
    internal_topo_override_save(saved_topo);
    result = jme_heliacal_ut_impl(jd_ut, geopos, dat_hel, error);
    internal_topo_override_restore(saved_topo);
    return result;
}

int jme_vis_limit_mag(double jd_ut, double *geopos, double *dat_hel, char *error)
{
    double saved_topo[4];
    int result;
    internal_topo_override_save(saved_topo);
    result = jme_vis_limit_mag_impl(jd_ut, geopos, dat_hel, error);
    internal_topo_override_restore(saved_topo);
    return result;
}

