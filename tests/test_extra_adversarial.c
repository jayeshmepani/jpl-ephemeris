#include "jme/jme.h"

#include <math.h>
#include <stdio.h>
#include <string.h>

#if defined(__unix__) || defined(__APPLE__)
#include <pthread.h>
#define JME_HAS_PTHREAD 1
#endif

#ifndef M_PI
#define M_PI 3.141592653589793238462643383279502884
#endif

#define ASSERT_TRUE(expr) \
    do { \
        if (!(expr)) { \
            fprintf(stderr, "Assertion failed: %s at %s:%d\n", #expr, __FILE__, __LINE__); \
            return 1; \
        } \
    } while (0)

#define ASSERT_OK(expr) ASSERT_TRUE((expr) == JME_OK)
#define ASSERT_ERR(expr) ASSERT_TRUE((expr) == JME_ERR)

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

static unsigned int lcg_next(unsigned int *state)
{
    *state = (*state * 1103515245u) + 12345u;
    return *state;
}

static double random_range(unsigned int *state, double min_value, double max_value)
{
    double unit = (double)(lcg_next(state) & 0x00ffffffu) / (double)0x01000000u;
    return min_value + unit * (max_value - min_value);
}

static int test_metadata_and_calendar_fuzz(void)
{
    unsigned int seed = 0x1234abcd;
    int i;

    ASSERT_TRUE(jme_body_id_from_name("Mars") == JME_BODY_MARS);
    ASSERT_TRUE(jme_body_id_from_name("mars") == JME_ERR);
    ASSERT_TRUE(jme_body_id_from_name(0) == JME_ERR);
    ASSERT_TRUE(jme_body_naif_id(JME_BODY_EARTH) == 399);
    ASSERT_TRUE(jme_body_naif_id(999999) == -1);

    for (i = 0; i < 10000; i++) {
        int year = (int)random_range(&seed, -4000.0, 5000.0);
        int month = (int)random_range(&seed, -5.0, 18.0);
        int day = (int)random_range(&seed, -5.0, 40.0);
        int cal = (lcg_next(&seed) & 1u) ? JME_CALENDAR_GREGORIAN : JME_CALENDAR_JULIAN;
        int valid = jme_date_is_valid(year, month, day, cal);

        if (month >= 1 && month <= 12) {
            int dim = jme_days_in_month(year, month, cal);
            ASSERT_TRUE(dim >= 28 && dim <= 31);
            if (valid) {
                ASSERT_TRUE(day >= 1 && day <= dim);
                ASSERT_TRUE(jme_day_of_year(year, month, day, cal) >= 1);
            }
        } else {
            ASSERT_TRUE(jme_days_in_month(year, month, cal) == 0);
            ASSERT_TRUE(valid == 0);
        }
    }

    ASSERT_TRUE(jme_calendar_is_leap_year(2000, JME_CALENDAR_GREGORIAN) == 1);
    ASSERT_TRUE(jme_calendar_is_leap_year(1900, JME_CALENDAR_GREGORIAN) == 0);
    ASSERT_TRUE(jme_decimal_hour(23, 59, 59.0) > 23.9997);
    return 0;
}

static int test_matrix_state_roundtrip_and_stress(void)
{
    unsigned int seed = 0x5eed5eed;
    int i;

    for (i = 0; i < 8000; i++) {
        double spherical[6] = {
            random_range(&seed, 0.0, 360.0),
            random_range(&seed, -89.5, 89.5),
            random_range(&seed, 0.001, 50.0),
            random_range(&seed, -0.2, 0.2),
            random_range(&seed, -0.2, 0.2),
            random_range(&seed, -0.2, 0.2)
        };
        double rect[6] = {0.0};
        double roundtrip[6] = {0.0};
        double m1[9];
        double m2[9];
        double m3[9];
        double state_out[6] = {0.0};
        double state2[6] = {0.0};

        ASSERT_OK(jme_spherical_to_rectangular_state(spherical, rect));
        ASSERT_OK(jme_rectangular_to_spherical_state(rect, roundtrip));
        ASSERT_TRUE(finite_values(rect, 6));
        ASSERT_TRUE(finite_values(roundtrip, 6));
        ASSERT_TRUE(fabs(jme_degrees_difference_signed(roundtrip[0], spherical[0])) < 1.0e-7);
        ASSERT_TRUE(fabs(roundtrip[1] - spherical[1]) < 1.0e-7);
        ASSERT_TRUE(fabs(roundtrip[2] - spherical[2]) < 1.0e-9);

        jme_matrix_identity(m1);
        jme_matrix_rotate_x(random_range(&seed, -M_PI, M_PI), m1);
        jme_matrix_rotate_y(random_range(&seed, -M_PI, M_PI), m2);
        jme_matrix_rotate_z(random_range(&seed, -M_PI, M_PI), m3);
        jme_matrix_multiply(m1, m2, m1);
        jme_matrix_multiply(m1, m3, m1);
        jme_matrix_transform_state(m1, rect, state_out);
        ASSERT_TRUE(finite_values(state_out, 6));

        ASSERT_OK(jme_state_add(rect, state_out, state2));
        ASSERT_OK(jme_state_subtract(state2, state_out, state2));
        ASSERT_OK(jme_state_scale(state2, 1.0, state2));
        ASSERT_TRUE(finite_values(state2, 6));
        ASSERT_TRUE(jme_state_distance(state2) > 0.0);
        ASSERT_TRUE(jme_state_speed(state2) >= 0.0);
        ASSERT_TRUE(isfinite(jme_state_light_time_days(state2, JME_VECTOR_AU_PER_DAY)));
        ASSERT_TRUE(isfinite(jme_state_position_velocity_dot(state2)));

        ASSERT_OK(jme_state_convert_units(state2, JME_VECTOR_AU_PER_DAY, JME_VECTOR_KM_PER_SECOND, state_out));
        ASSERT_OK(jme_state_convert_units(state_out, JME_VECTOR_KM_PER_SECOND, JME_VECTOR_AU_PER_DAY, rect));
        ASSERT_TRUE(finite_values(rect, 6));
    }

    ASSERT_ERR(jme_spherical_to_rectangular_state(0, 0));
    ASSERT_ERR(jme_rectangular_to_spherical_state(0, 0));
    ASSERT_ERR(jme_state_add(0, 0, 0));
    ASSERT_ERR(jme_state_subtract(0, 0, 0));
    ASSERT_ERR(jme_state_scale(0, 2.0, 0));
    ASSERT_ERR(jme_state_convert_units(0, JME_VECTOR_AU_PER_DAY, JME_VECTOR_KM_PER_SECOND, 0));
    return 0;
}

static int test_astrometry_models_and_topo(void)
{
    const double dates[] = {1674484.0, 2415020.5, 2451545.0, 2460310.5, 2816787.5};
    int i;
    char error[256];

    for (i = 0; i < (int)(sizeof(dates) / sizeof(dates[0])); i++) {
        double eps = 0.0;
        double dpsi = 0.0;
        double deps = 0.0;
        double m[9];
        double topo[3];

        ASSERT_OK(jme_get_obliquity(dates[i], JME_MODEL_OBL_IAU_1980, &eps, error));
        ASSERT_TRUE(isfinite(eps) && eps > 20.0 && eps < 25.0);
        ASSERT_OK(jme_get_obliquity(dates[i], JME_MODEL_OBL_IAU_2006, &eps, error));
        ASSERT_TRUE(isfinite(eps) && eps > 20.0 && eps < 25.0);

        ASSERT_OK(jme_get_nutation(dates[i], JME_MODEL_NUT_IAU_1980, &dpsi, &deps, error));
        ASSERT_TRUE(isfinite(dpsi) && isfinite(deps));
        ASSERT_OK(jme_get_nutation(dates[i], JME_MODEL_NUT_IAU_2000B, &dpsi, &deps, error));
        ASSERT_TRUE(isfinite(dpsi) && isfinite(deps));

        ASSERT_OK(jme_get_frame_bias_matrix(JME_MODEL_BIAS_NONE, m));
        ASSERT_TRUE(finite_values(m, 9));
        ASSERT_OK(jme_get_frame_bias_matrix(JME_MODEL_BIAS_IAU2006, m));
        ASSERT_TRUE(finite_values(m, 9));

        ASSERT_OK(jme_get_precession_matrix(2451545.0, dates[i], JME_MODEL_PREC_IAU_1976, m));
        ASSERT_TRUE(finite_values(m, 9));
        ASSERT_OK(jme_get_precession_matrix(2451545.0, dates[i], JME_MODEL_PREC_IAU_2006, m));
        ASSERT_TRUE(finite_values(m, 9));

        jme_get_nutation_matrix(0.0, 0.0, 23.4 * M_PI / 180.0, m);
        ASSERT_TRUE(finite_values(m, 9));

        jme_set_topo(77.5946, 12.9716, 920.0);
        ASSERT_OK(jme_get_topo_pos(dates[i], topo, error));
        ASSERT_TRUE(finite_values(topo, 3));
    }

    ASSERT_ERR(jme_get_obliquity(2451545.0, 999999, 0, error));
    ASSERT_ERR(jme_get_nutation(2451545.0, 999999, 0, 0, error));
    ASSERT_ERR(jme_get_frame_bias_matrix(999999, 0));
    ASSERT_ERR(jme_get_precession_matrix(2451545.0, 2451545.0, 999999, 0));
    ASSERT_ERR(jme_get_topo_pos(2451545.0, 0, error));
    return 0;
}

static int test_analytical_state_providers(void)
{
    const double dates[] = {2440587.5, 2451545.0, 2460310.5};
    const int vsop_bodies[] = {JME_BODY_MERCURY, JME_BODY_VENUS, JME_BODY_EARTH, JME_BODY_MARS, JME_BODY_JUPITER, JME_BODY_SATURN, JME_BODY_URANUS, JME_BODY_NEPTUNE};
    const int moshier_bodies[] = {JME_BODY_MERCURY, JME_BODY_VENUS, JME_BODY_EARTH, JME_BODY_MARS, JME_BODY_JUPITER, JME_BODY_SATURN, JME_BODY_URANUS, JME_BODY_NEPTUNE, JME_BODY_PLUTO};
    const int meeus_bodies[] = {JME_BODY_MERCURY, JME_BODY_VENUS, JME_BODY_MARS, JME_BODY_JUPITER, JME_BODY_SATURN, JME_BODY_URANUS, JME_BODY_NEPTUNE, JME_BODY_PLUTO};
    int i;
    int j;

    for (i = 0; i < (int)(sizeof(dates) / sizeof(dates[0])); i++) {
        double moon_state[6];
        double sun_state[6];
        ASSERT_OK(jme_elp2000_moon_state(dates[i], moon_state));
        ASSERT_TRUE(finite_values(moon_state, 6));
        ASSERT_OK(jme_meeus_sun_state(dates[i], sun_state));
        ASSERT_TRUE(finite_values(sun_state, 6));
        ASSERT_OK(jme_meeus_moon_state(dates[i], moon_state));
        ASSERT_TRUE(finite_values(moon_state, 6));

        for (j = 0; j < (int)(sizeof(vsop_bodies) / sizeof(vsop_bodies[0])); j++) {
            double state_a[6];
            double state_b[6];
            ASSERT_OK(jme_vsop87_planet_state(dates[i], vsop_bodies[j], state_a));
            ASSERT_OK(jme_vsop87_planet_state(dates[i], vsop_bodies[j], state_b));
            ASSERT_TRUE(finite_values(state_a, 6));
            ASSERT_TRUE(memcmp(state_a, state_b, sizeof(state_a)) == 0);
        }
        for (j = 0; j < (int)(sizeof(moshier_bodies) / sizeof(moshier_bodies[0])); j++) {
            double state[6];
            ASSERT_OK(jme_moshier_planet_state(dates[i], moshier_bodies[j], state));
            ASSERT_TRUE(finite_values(state, 6));
        }
        for (j = 0; j < (int)(sizeof(meeus_bodies) / sizeof(meeus_bodies[0])); j++) {
            double state[6];
            ASSERT_OK(jme_meeus_planet_state(dates[i], meeus_bodies[j], state));
            ASSERT_TRUE(finite_values(state, 6));
        }
    }

    {
        double state[6];
        ASSERT_ERR(jme_vsop87_planet_state(2451545.0, JME_BODY_PLUTO, state));
        ASSERT_ERR(jme_moshier_planet_state(2451545.0, 999999, state));
        ASSERT_ERR(jme_meeus_planet_state(2451545.0, JME_BODY_SUN, state));
        ASSERT_ERR(jme_elp2000_moon_state(2451545.0, 0));
        ASSERT_ERR(jme_meeus_sun_state(2451545.0, 0));
        ASSERT_ERR(jme_meeus_moon_state(2451545.0, 0));
    }
    return 0;
}

static int test_raw_jpl_surface_without_kernel(void)
{
    char error[256];
    double state[12];
    double t1 = -1.0;
    double t2 = -1.0;
    double value = -1.0;
    int continuous = -1;
    int id = -1;
    int target = -1;
    int center = -1;
    int frame = -1;
    int segment = -1;
    char name[128];
    const char *paths[] = {"/definitely/not/a/kernel1.bsp", "/definitely/not/a/kernel2.bsp"};

    ASSERT_ERR(jme_jpl_open("", error));
    ASSERT_ERR(jme_jpl_open("/definitely/not/a/kernel.bsp", error));
    ASSERT_ERR(jme_jpl_open_array(2, paths, error));
    ASSERT_TRUE(jme_jpl_is_open() == 0);

    ASSERT_ERR(jme_jpl_body_state(2451545.0, JME_BODY_SUN, JME_BODY_EARTH, JME_VECTOR_AU_PER_DAY, state, error));
    ASSERT_ERR(jme_jpl_body_state_split(2451545.0, 0.25, JME_BODY_SUN, JME_BODY_EARTH, JME_VECTOR_AU_PER_DAY, state, error));
    ASSERT_ERR(jme_jpl_body_state_naif(2451545.0, 10, 399, JME_VECTOR_AU_PER_DAY, state, error));
    ASSERT_ERR(jme_jpl_body_state_native(2451545.0, JME_BODY_SUN, JME_BODY_EARTH, state, error));
    ASSERT_ERR(jme_jpl_body_state_native_naif(2451545.0, 10, 399, state, error));
    ASSERT_ERR(jme_jpl_body_state_native_split(2451545.0, 0.25, JME_BODY_SUN, JME_BODY_EARTH, state, error));
    ASSERT_ERR(jme_jpl_body_state_native_split_naif(2451545.0, 0.25, 10, 399, state, error));
    ASSERT_ERR(jme_jpl_body_state_order(2451545.0, 0.25, JME_BODY_SUN, JME_BODY_EARTH, JME_VECTOR_AU_PER_DAY, 1, state, error));
    ASSERT_ERR(jme_jpl_body_state_order_naif(2451545.0, 0.25, 10, 399, JME_VECTOR_AU_PER_DAY, 1, state, error));
    ASSERT_ERR(jme_jpl_body_state_utc(2000, 1, 1, 12, 0, 0.0, JME_CALENDAR_GREGORIAN, JME_BODY_SUN, JME_BODY_EARTH, JME_VECTOR_AU_PER_DAY, state, error));
    ASSERT_ERR(jme_jpl_body_state_utc_naif(2000, 1, 1, 12, 0, 0.0, JME_CALENDAR_GREGORIAN, 10, 399, JME_VECTOR_AU_PER_DAY, state, error));
    ASSERT_ERR(jme_jpl_ecliptic_state(2451545.0, JME_BODY_SUN, JME_BODY_EARTH, JME_VECTOR_AU_PER_DAY, state, error));
    ASSERT_ERR(jme_jpl_ecliptic_state_naif(2451545.0, 10, 399, JME_VECTOR_AU_PER_DAY, state, error));
    ASSERT_ERR(jme_jpl_ecliptic_state_split(2451545.0, 0.25, JME_BODY_SUN, JME_BODY_EARTH, JME_VECTOR_AU_PER_DAY, state, error));
    ASSERT_ERR(jme_jpl_ecliptic_state_split_naif(2451545.0, 0.25, 10, 399, JME_VECTOR_AU_PER_DAY, state, error));
    ASSERT_ERR(jme_jpl_ecliptic_state_utc(2000, 1, 1, 12, 0, 0.0, JME_CALENDAR_GREGORIAN, JME_BODY_SUN, JME_BODY_EARTH, JME_VECTOR_AU_PER_DAY, state, error));
    ASSERT_ERR(jme_jpl_ecliptic_state_utc_naif(2000, 1, 1, 12, 0, 0.0, JME_CALENDAR_GREGORIAN, 10, 399, JME_VECTOR_AU_PER_DAY, state, error));
    ASSERT_ERR(jme_jpl_orientation_state_naif(2451545.0, 399, JME_ORIENTATION_RAD_PER_DAY, state, error));
    ASSERT_ERR(jme_jpl_orientation_state_split_naif(2451545.0, 0.25, 399, JME_ORIENTATION_RAD_PER_DAY, state, error));
    ASSERT_ERR(jme_jpl_orientation_state_order_naif(2451545.0, 0.25, 399, JME_ORIENTATION_RAD_PER_DAY, 1, state, error));
    ASSERT_ERR(jme_jpl_orientation_state_utc_naif(2000, 1, 1, 12, 0, 0.0, JME_CALENDAR_GREGORIAN, 399, JME_ORIENTATION_RAD_PER_DAY, state, error));
    ASSERT_ERR(jme_jpl_rotational_angular_momentum_state_naif(2451545.0, 399, JME_ORIENTATION_RAD_PER_DAY, state, error));
    ASSERT_ERR(jme_jpl_rotational_angular_momentum_state_split_naif(2451545.0, 0.25, 399, JME_ORIENTATION_RAD_PER_DAY, state, error));
    ASSERT_ERR(jme_jpl_rotational_angular_momentum_state_order_naif(2451545.0, 0.25, 399, JME_ORIENTATION_RAD_PER_DAY, 1, state, error));
    ASSERT_ERR(jme_jpl_rotational_angular_momentum_state_utc_naif(2000, 1, 1, 12, 0, 0.0, JME_CALENDAR_GREGORIAN, 399, JME_ORIENTATION_RAD_PER_DAY, state, error));

    ASSERT_ERR(jme_jpl_coverage(&t1, &t2, &continuous, error));
    ASSERT_ERR(jme_jpl_constant("AU", &value, error));
    ASSERT_ERR(jme_jpl_constant_count(error));
    ASSERT_ERR(jme_jpl_constant_index(0, name, sizeof(name), &value, error));
    ASSERT_ERR(jme_jpl_constant_vector("AU", state, 3, error));
    ASSERT_ERR(jme_jpl_constant_string("NAME", name, sizeof(name), error));
    ASSERT_ERR(jme_jpl_constant_string_vector("NAME", name, sizeof(name), 1, error));
    ASSERT_ERR(jme_jpl_file_version(name, sizeof(name), error));
    ASSERT_ERR(jme_jpl_current_file_data(name, sizeof(name), &t1, &t2, &continuous, error));
    ASSERT_TRUE(jme_jpl_engine_version(name, sizeof(name)) != 0);
    ASSERT_TRUE(jme_jpl_is_available() == 0 || jme_jpl_is_available() == 1);
    ASSERT_ERR(jme_jpl_prefetch(error));
    ASSERT_ERR(jme_jpl_is_thread_safe(error));
    ASSERT_ERR(jme_jpl_id_by_name("EARTH", &id, error));
    ASSERT_ERR(jme_jpl_name_by_id(399, name, sizeof(name), error));
    ASSERT_TRUE(jme_jpl_max_supported_order(1) == JME_ERR || jme_jpl_max_supported_order(1) >= 0);
    ASSERT_ERR(jme_jpl_position_record_count(error));
    ASSERT_ERR(jme_jpl_position_record_index(0, &target, &center, &t1, &t2, &frame, &segment, error));
    ASSERT_ERR(jme_jpl_orientation_record_count(error));
    ASSERT_ERR(jme_jpl_orientation_record_index(0, &target, &t1, &t2, &frame, &segment, error));
    ASSERT_TRUE(jme_jpl_timescale() >= 0);
    jme_jpl_close();
    return 0;
}

#if defined(JME_HAS_PTHREAD)
static void *extra_thread_worker(void *arg)
{
    int idx = *(int *)arg;
    int i;

    for (i = 0; i < 400; i++) {
        double jd = 2451545.0 + idx * 1000.0 + i * 0.25;
        double state[6];
        double eps = 0.0;
        char error[256];

        if (jme_meeus_planet_state(jd, JME_BODY_MARS, state) != JME_OK || !finite_values(state, 6)) {
            return (void *)1;
        }
        if (jme_moshier_planet_state(jd, JME_BODY_JUPITER, state) != JME_OK || !finite_values(state, 6)) {
            return (void *)1;
        }
        if (jme_vsop87_planet_state(jd, JME_BODY_VENUS, state) != JME_OK || !finite_values(state, 6)) {
            return (void *)1;
        }
        if (jme_get_obliquity(jd, JME_MODEL_OBL_IAU_2006, &eps, error) != JME_OK || !isfinite(eps)) {
            return (void *)1;
        }
    }
    return 0;
}

static int test_extra_concurrency(void)
{
    pthread_t threads[4];
    int idx[4] = {0, 1, 2, 3};
    int i;

    for (i = 0; i < 4; i++) {
        ASSERT_TRUE(pthread_create(&threads[i], 0, extra_thread_worker, &idx[i]) == 0);
    }
    for (i = 0; i < 4; i++) {
        void *status = 0;
        ASSERT_TRUE(pthread_join(threads[i], &status) == 0);
        ASSERT_TRUE(status == 0);
    }
    return 0;
}
#else
static int test_extra_concurrency(void)
{
    return 0;
}
#endif

int main(void)
{
    if (test_metadata_and_calendar_fuzz() != 0) return 1;
    if (test_matrix_state_roundtrip_and_stress() != 0) return 1;
    if (test_astrometry_models_and_topo() != 0) return 1;
    if (test_analytical_state_providers() != 0) return 1;
    if (test_raw_jpl_surface_without_kernel() != 0) return 1;
    if (test_extra_concurrency() != 0) return 1;
    printf("Extra adversarial validation passed\n");
    return 0;
}
