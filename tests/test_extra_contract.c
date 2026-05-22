#include "jme/jme.h"

#include <math.h>
#include <stdio.h>
#include <string.h>

static int close_enough(const char *label, double got, double expected, double tolerance)
{
    if (!isfinite(got) || fabs(got - expected) > tolerance) {
        fprintf(stderr, "%s: got %.17g expected %.17g tolerance %.17g\n", label, got, expected, tolerance);
        return 0;
    }
    return 1;
}

static int finite_values(const double *values, int count)
{
    int i;

    for (i = 0; i < count; i++) {
        if (!isfinite(values[i])) {
            return 0;
        }
    }

    return 1;
}

static int matrix_is_orthonormal(const double *m, double tolerance)
{
    int r;
    int c;

    for (r = 0; r < 3; r++) {
        for (c = 0; c < 3; c++) {
            double dot = 0.0;
            int k;
            for (k = 0; k < 3; k++) {
                dot += m[r * 3 + k] * m[c * 3 + k];
            }
            if (!close_enough("matrix orthonormal", dot, r == c ? 1.0 : 0.0, tolerance)) {
                return 0;
            }
        }
    }

    return 1;
}

static int test_metadata_calendar_time(void)
{
    if (jme_body_id_from_name("Mars") != JME_BODY_MARS
        || jme_body_id_from_name("Solar System Barycenter") != JME_BODY_SOLAR_SYSTEM_BARYCENTER
        || jme_body_id_from_name("mars") != JME_ERR
        || jme_body_id_from_name(0) != JME_ERR
        || jme_body_naif_id(JME_BODY_MARS) != 499
        || jme_body_naif_id(JME_BODY_SOLAR_SYSTEM_BARYCENTER) != 0
        || jme_body_naif_id(999999) != -1) {
        fprintf(stderr, "body metadata contract failed\n");
        return 1;
    }

    if (jme_calendar_is_leap_year(2000, JME_CALENDAR_GREGORIAN) != 1
        || jme_calendar_is_leap_year(1900, JME_CALENDAR_GREGORIAN) != 0
        || jme_calendar_is_leap_year(1900, JME_CALENDAR_JULIAN) != 1
        || jme_days_in_month(2000, 2, JME_CALENDAR_GREGORIAN) != 29
        || jme_days_in_month(1900, 2, JME_CALENDAR_GREGORIAN) != 28
        || jme_days_in_month(2024, 13, JME_CALENDAR_GREGORIAN) != 0
        || jme_day_of_year(2024, 12, 31, JME_CALENDAR_GREGORIAN) != 366
        || jme_day_of_year(2023, 12, 31, JME_CALENDAR_GREGORIAN) != 365
        || jme_day_of_year(2023, 2, 29, JME_CALENDAR_GREGORIAN) != 0) {
        fprintf(stderr, "calendar utility contract failed\n");
        return 1;
    }

    if (!close_enough("decimal hour", jme_decimal_hour(23, 59, 59.5), 23.999861111111112, 1e-14)
        || !close_enough("degrees to hours", jme_degrees_to_hours(360.0), 24.0, 1e-15)
        || !close_enough("hours to degrees", jme_hours_to_degrees(1.5), 22.5, 1e-15)
        || !close_enough("degrees to radians", jme_degrees_to_radians(180.0), acos(-1.0), 1e-15)
        || !close_enough("radians to degrees", jme_radians_to_degrees(acos(-1.0)), 180.0, 1e-12)
        || !close_enough("hours normalize", jme_hours_normalize(-1.25), 22.75, 1e-15)
        || !close_enough("jd add seconds", jme_jd_add_seconds(2451545.0, 43200.0), 2451545.5, 1e-15)
        || !close_enough("jd difference seconds", jme_jd_difference_seconds(2451545.5, 2451545.0), 43200.0, 1e-6)) {
        return 1;
    }

    return 0;
}

static int test_state_and_matrix_math(void)
{
    double spherical[6] = {30.0, -20.0, 2.0, 0.1, -0.2, 0.03};
    double rectangular[6] = {0.0};
    double round_trip[6] = {0.0};
    double invalid_spherical[6] = {0.0, 0.0, -1.0, 0.0, 0.0, 0.0};
    double invalid_rectangular[6] = {0.0};
    double state_a[6] = {1.0, 2.0, 2.0, 0.1, -0.2, 0.2};
    double state_b[6] = {-0.5, 4.0, 1.0, -0.4, 0.3, 0.0};
    double out[6] = {0.0};
    double km_state[6] = {0.0};
    double m[9];
    double r[9];
    double transformed[6] = {0.0};

    if (jme_spherical_to_rectangular_state(spherical, rectangular) != JME_OK
        || jme_rectangular_to_spherical_state(rectangular, round_trip) != JME_OK
        || jme_spherical_to_rectangular_state(invalid_spherical, invalid_rectangular) != JME_ERR
        || jme_spherical_to_rectangular_state(0, rectangular) != JME_ERR
        || jme_rectangular_to_spherical_state(0, spherical) != JME_ERR) {
        fprintf(stderr, "spherical/rectangular conversion error-path contract failed\n");
        return 1;
    }

    if (!close_enough("round-trip lon", jme_degrees_difference_signed(round_trip[0], spherical[0]), 0.0, 1e-12)
        || !close_enough("round-trip lat", round_trip[1], spherical[1], 1e-12)
        || !close_enough("round-trip radius", round_trip[2], spherical[2], 1e-12)
        || !close_enough("angular separation", jme_spherical_angular_separation(0.0, 0.0, 90.0, 0.0), 90.0, 1e-14)
        || !close_enough("position angle east", jme_spherical_position_angle(0.0, 0.0, 90.0, 0.0), 90.0, 1e-14)) {
        return 1;
    }

    if (!close_enough("state distance", jme_state_distance(state_a), 3.0, 1e-15)
        || !close_enough("state speed", jme_state_speed(state_a), 0.3, 1e-15)
        || !close_enough("state dot", jme_state_position_velocity_dot(state_a), 0.1, 1e-15)
        || jme_state_add(state_a, state_b, out) != JME_OK
        || !close_enough("state add", out[1], 6.0, 1e-15)
        || jme_state_subtract(state_a, state_b, out) != JME_OK
        || !close_enough("state subtract", out[0], 1.5, 1e-15)
        || jme_state_scale(state_a, -2.0, out) != JME_OK
        || !close_enough("state scale", out[2], -4.0, 1e-15)
        || jme_state_add(0, state_b, out) != JME_ERR
        || jme_state_subtract(state_a, 0, out) != JME_ERR
        || jme_state_scale(0, 1.0, out) != JME_ERR) {
        fprintf(stderr, "state arithmetic contract failed\n");
        return 1;
    }

    if (jme_state_convert_units(state_a, JME_VECTOR_AU_PER_DAY, JME_VECTOR_KM_PER_SECOND, km_state) != JME_OK
        || !close_enough("state km position", km_state[0], JME_AU_KM, 1e-6)
        || !close_enough("state km/s velocity", km_state[3], 0.1 * JME_AU_KM / JME_SECONDS_PER_DAY, 1e-12)
        || jme_state_convert_units(km_state, JME_VECTOR_KM_PER_SECOND, JME_VECTOR_AU_PER_DAY, out) != JME_OK
        || !close_enough("state unit round-trip", out[3], state_a[3], 1e-15)
        || jme_state_convert_units(state_a, -1, JME_VECTOR_AU_PER_DAY, out) != JME_ERR
        || !close_enough("light time", jme_state_light_time_days(state_a, JME_VECTOR_AU_PER_DAY), 3.0 * JME_AU_KM / JME_SPEED_OF_LIGHT_KM_PER_SEC / JME_SECONDS_PER_DAY, 1e-15)
        || jme_state_distance(0) != 0.0
        || jme_state_speed(0) != 0.0
        || jme_state_position_velocity_dot(0) != 0.0
        || jme_state_light_time_days(0, JME_VECTOR_AU_PER_DAY) != 0.0) {
        fprintf(stderr, "state unit/light-time contract failed\n");
        return 1;
    }

    jme_matrix_identity(m);
    if (!matrix_is_orthonormal(m, 1e-15)) {
        return 1;
    }

    jme_matrix_identity(m);
    jme_matrix_rotate_z(acos(-1.0) / 2.0, m);
    transformed[0] = 1.0;
    transformed[4] = 1.0;
    jme_matrix_transform_state(m, transformed, out);
    if (!close_enough("rotate z x", out[0], 0.0, 1e-15)
        || !close_enough("rotate z y", out[1], -1.0, 1e-15)
        || !close_enough("rotate z vy", out[4], 0.0, 1e-15)
        || !matrix_is_orthonormal(m, 1e-15)) {
        return 1;
    }

    jme_matrix_identity(r);
    jme_matrix_rotate_x(0.25, r);
    jme_matrix_rotate_y(-0.5, r);
    jme_matrix_multiply(m, r, m);
    if (!matrix_is_orthonormal(m, 1e-14)) {
        return 1;
    }

    return 0;
}

static int test_model_helpers(void)
{
    char error[256] = "";
    double value = 0.0;
    double value2 = 0.0;
    double m[9];
    double topo[3];

    if (jme_get_obliquity(2451545.0, JME_MODEL_OBL_IAU_1980, &value, error) != JME_OK
        || !close_enough("IAU 1980 obliquity", value, 84381.448 / 3600.0, 1e-15)
        || jme_get_obliquity(2451545.0, JME_MODEL_OBL_IAU_2006, &value, error) != JME_OK
        || !close_enough("IAU 2006 obliquity", value, 84381.406 / 3600.0, 1e-15)
        || jme_get_obliquity(2451545.0, 999999, &value, error) != JME_ERR
        || jme_get_obliquity(2451545.0, JME_MODEL_OBL_IAU_1980, 0, error) != JME_ERR) {
        fprintf(stderr, "obliquity helper contract failed\n");
        return 1;
    }

    if (jme_get_nutation(2451545.0, JME_MODEL_NUT_IAU_1980, &value, &value2, error) != JME_OK
        || !close_enough("IAU 1980 nut dpsi", value, -0.0038698621712391834, 1e-15)
        || !close_enough("IAU 1980 nut deps", value2, -0.0016041131523510195, 1e-15)
        || jme_get_nutation(2451545.0, JME_MODEL_NUT_IAU_2000B, &value, &value2, error) != JME_OK
        || !close_enough("IAU 2000B nut dpsi", value, -0.0038699066358249398, 1e-15)
        || !close_enough("IAU 2000B nut deps", value2, -0.0016026158548035686, 1e-15)
        || jme_get_nutation(2451545.0, JME_MODEL_NUT_IAU_2000A, &value, &value2, error) != JME_ERR
        || jme_get_nutation(2451545.0, 999999, &value, &value2, error) != JME_ERR
        || jme_get_nutation(2451545.0, JME_MODEL_NUT_IAU_1980, 0, &value2, error) != JME_ERR) {
        fprintf(stderr, "nutation helper contract failed\n");
        return 1;
    }

    if (jme_get_precession_matrix(2451545.0, 2451545.0, JME_MODEL_PREC_IAU_1976, m) != JME_OK
        || !matrix_is_orthonormal(m, 1e-15)
        || jme_get_precession_matrix(2451545.0, 2488070.0, JME_MODEL_PREC_IAU_2006, m) != JME_OK
        || !matrix_is_orthonormal(m, 1e-12)
        || jme_get_precession_matrix(2451545.0, 2488070.0, 999999, m) != JME_ERR
        || jme_get_precession_matrix(2451545.0, 2488070.0, JME_MODEL_PREC_IAU_2006, 0) != JME_ERR) {
        fprintf(stderr, "precession matrix contract failed\n");
        return 1;
    }

    if (jme_get_frame_bias_matrix(JME_MODEL_BIAS_NONE, m) != JME_OK
        || !matrix_is_orthonormal(m, 1e-15)
        || jme_get_frame_bias_matrix(JME_MODEL_BIAS_IAU2006, m) != JME_OK
        || !matrix_is_orthonormal(m, 1e-15)
        || jme_get_frame_bias_matrix(999999, m) != JME_ERR
        || jme_get_frame_bias_matrix(JME_MODEL_BIAS_NONE, 0) != JME_ERR) {
        fprintf(stderr, "frame bias matrix contract failed\n");
        return 1;
    }

    jme_get_nutation_matrix(0.0, 0.0, 84381.406 / 3600.0 * acos(-1.0) / 180.0, m);
    if (!matrix_is_orthonormal(m, 1e-15)) {
        return 1;
    }

    jme_set_topo(77.5946, 12.9716, 920.0);
    if (jme_get_topo_pos(2451545.0, topo, error) != JME_OK
        || !finite_values(topo, 3)
        || jme_state_distance(topo) < 0.000041
        || jme_state_distance(topo) > 0.000044
        || jme_get_topo_pos(2451545.0, 0, error) != JME_ERR) {
        fprintf(stderr, "topocentric position contract failed\n");
        return 1;
    }

    return 0;
}

static int test_analytical_engines(void)
{
    static const int vsop_bodies[] = {
        JME_BODY_MERCURY, JME_BODY_VENUS, JME_BODY_EARTH, JME_BODY_MARS,
        JME_BODY_JUPITER, JME_BODY_SATURN, JME_BODY_URANUS, JME_BODY_NEPTUNE
    };
    static const int moshier_bodies[] = {
        JME_BODY_MERCURY, JME_BODY_VENUS, JME_BODY_EARTH, JME_BODY_MARS,
        JME_BODY_JUPITER, JME_BODY_SATURN, JME_BODY_URANUS, JME_BODY_NEPTUNE, JME_BODY_PLUTO
    };
    double dates[] = {2451545.0, 2440587.5, 2469807.5};
    double state[6];
    int i;
    int j;

    for (i = 0; i < (int)(sizeof(vsop_bodies) / sizeof(vsop_bodies[0])); i++) {
        for (j = 0; j < (int)(sizeof(dates) / sizeof(dates[0])); j++) {
            if (jme_vsop87_planet_state(dates[j], vsop_bodies[i], state) != JME_OK
                || !finite_values(state, 6)
                || jme_state_distance(state) <= 0.0
                || jme_state_speed(state) <= 0.0) {
                fprintf(stderr, "VSOP87 engine contract failed body=%d date=%.1f\n", vsop_bodies[i], dates[j]);
                return 1;
            }
        }
    }

    if (jme_vsop87_planet_state(2451545.0, JME_BODY_SUN, state) != JME_ERR
        || jme_vsop87_planet_state(2451545.0, JME_BODY_MARS, 0) != JME_ERR) {
        fprintf(stderr, "VSOP87 invalid-input contract failed\n");
        return 1;
    }

    for (i = 0; i < (int)(sizeof(moshier_bodies) / sizeof(moshier_bodies[0])); i++) {
        if (jme_moshier_planet_state(2451545.0, moshier_bodies[i], state) != JME_OK
            || !finite_values(state, 6)
            || jme_state_distance(state) <= 0.0
            || jme_state_speed(state) <= 0.0) {
            fprintf(stderr, "Moshier engine contract failed body=%d\n", moshier_bodies[i]);
            return 1;
        }
    }

    if (jme_moshier_planet_state(2451545.0, JME_BODY_SUN, state) != JME_ERR
        || jme_moshier_planet_state(2451545.0, JME_BODY_MARS, 0) != JME_ERR) {
        fprintf(stderr, "Moshier invalid-input contract failed\n");
        return 1;
    }

    if (jme_elp2000_moon_state(2451545.0, state) != JME_OK
        || !finite_values(state, 6)
        || jme_state_distance(state) < 0.0023
        || jme_state_distance(state) > 0.0028
        || jme_state_speed(state) <= 0.0
        || jme_elp2000_moon_state(2451545.0, 0) != JME_ERR) {
        fprintf(stderr, "ELP2000 engine contract failed\n");
        return 1;
    }

    if (jme_meeus_sun_state(2451545.0, state) != JME_OK
        || !finite_values(state, 6)
        || jme_state_distance(state) < 0.98
        || jme_state_distance(state) > 1.02
        || jme_meeus_sun_state(2451545.0, 0) != JME_ERR
        || jme_meeus_moon_state(2451545.0, state) != JME_OK
        || !finite_values(state, 6)
        || jme_state_distance(state) < 0.0023
        || jme_state_distance(state) > 0.0028
        || jme_meeus_moon_state(2451545.0, 0) != JME_ERR) {
        fprintf(stderr, "Meeus Sun/Moon contract failed\n");
        return 1;
    }

    for (i = JME_BODY_MERCURY; i <= JME_BODY_PLUTO; i++) {
        if (jme_meeus_planet_state(2451545.0, i, state) != JME_OK
            || !finite_values(state, 6)
            || jme_state_distance(state) <= 0.0) {
            fprintf(stderr, "Meeus planet contract failed body=%d\n", i);
            return 1;
        }
    }

    if (jme_meeus_planet_state(2451545.0, JME_BODY_SUN, state) != JME_ERR
        || jme_meeus_planet_state(2451545.0, JME_BODY_MARS, 0) != JME_ERR) {
        fprintf(stderr, "Meeus planet invalid-input contract failed\n");
        return 1;
    }

    return 0;
}

int main(void)
{
    if (test_metadata_calendar_time() != 0
        || test_state_and_matrix_math() != 0
        || test_model_helpers() != 0
        || test_analytical_engines() != 0) {
        return 1;
    }

    return 0;
}
