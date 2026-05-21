#include "jme/jme.h"

#include <math.h>
#include <stdio.h>
#include <stdlib.h>

static int finite_state(const double *state)
{
    int i;

    for (i = 0; i < 6; i++) {
        if (!isfinite(state[i])) {
            return 0;
        }
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

static int test_constant_kernel(const char *path)
{
    char error[256] = "";
    char name[128] = "";
    char value[128] = "";
    char values[256] = "";
    double number = 0.0;
    double vector[3] = {0.0, 0.0, 0.0};
    int count;

    if (path == 0 || path[0] == '\0') {
        return 0;
    }

    jme_jpl_close();
    if (jme_jpl_open(path, error) != JME_OK) {
        fprintf(stderr, "constant kernel open failed: %s\n", error);
        return 1;
    }

    count = jme_jpl_constant_count(error);
    if (count < 4) {
        fprintf(stderr, "constant kernel count too small: %d %s\n", count, error);
        return 1;
    }

    if (jme_jpl_constant_index(3, name, sizeof(name), &number, error) != JME_OK
        || name[0] == '\0'
        || !isfinite(number)) {
        fprintf(stderr, "constant index success path failed: %s\n", error);
        return 1;
    }

    if (jme_jpl_constant("JME_TEST_NUMBER", &number, error) != JME_OK
        || fabs(number - 42.0) > 1.0e-15) {
        fprintf(stderr, "numeric constant mismatch: %s %.17g\n", error, number);
        return 1;
    }

    if (jme_jpl_constant_vector("JME_TEST_VECTOR", vector, 3, error) != JME_OK
        || fabs(vector[0] - 1.0) > 1.0e-15
        || fabs(vector[1] - 2.0) > 1.0e-15
        || fabs(vector[2] - 3.0) > 1.0e-15) {
        fprintf(stderr, "numeric vector constant mismatch: %s\n", error);
        return 1;
    }

    if (jme_jpl_constant_string("JME_TEST_STRING", value, sizeof(value), error) != JME_OK
        || value[0] != 'J'
        || value[1] != 'M'
        || value[2] != 'E'
        || value[3] != '\0') {
        fprintf(stderr, "string constant mismatch: %s %s\n", error, value);
        return 1;
    }

    if (jme_jpl_constant_string_vector("JME_TEST_STRING_VECTOR", values, 64, 2, error) != JME_OK
        || values[0] != 'J'
        || values[1] != 'M'
        || values[2] != 'E'
        || values[64] != 'C') {
        fprintf(stderr, "string vector constant mismatch: %s\n", error);
        return 1;
    }

    jme_jpl_close();
    return 0;
}

static int test_orientation_kernel(const char *path)
{
    char error[256] = "";
    double first_time = 0.0;
    double last_time = 0.0;
    double state[12] = {0.0};
    int target = 0;
    int frame = 0;
    int segment_type = 0;
    int count;

    if (path == 0 || path[0] == '\0') {
        return 0;
    }

    jme_jpl_close();
    if (jme_jpl_open(path, error) != JME_OK) {
        fprintf(stderr, "orientation kernel open failed: %s\n", error);
        return 1;
    }

    count = jme_jpl_orientation_record_count(error);
    if (count <= 0) {
        fprintf(stderr, "orientation record count failed: %s\n", error);
        return 1;
    }

    if (jme_jpl_orientation_record_index(1, &target, &first_time, &last_time, &frame, &segment_type, error) != JME_OK
        || target == 0
        || !(first_time < last_time)) {
        fprintf(stderr, "orientation record metadata failed: %s\n", error);
        return 1;
    }

    if (jme_jpl_orientation_state_naif(0.5 * (first_time + last_time), target, JME_ORIENTATION_RAD_PER_DAY, state, error) != JME_OK
        || !finite_state(state)) {
        fprintf(stderr, "orientation state failed: %s\n", error);
        return 1;
    }

    if (jme_jpl_orientation_state_split_naif(0.5 * (first_time + last_time), 0.0, target, JME_ORIENTATION_RAD_PER_DAY, state, error) != JME_OK
        || !finite_state(state)) {
        fprintf(stderr, "orientation split state failed: %s\n", error);
        return 1;
    }

    if (jme_jpl_orientation_state_order_naif(0.5 * (first_time + last_time), 0.0, target, JME_ORIENTATION_RAD_PER_DAY, 1, state, error) != JME_OK
        || !finite_values(state, 6)) {
        fprintf(stderr, "orientation order state failed: %s\n", error);
        return 1;
    }

    if (jme_jpl_orientation_state_utc_naif(2000, 1, 1, 12, 0, 0.0, JME_CALENDAR_GREGORIAN, target, JME_ORIENTATION_RAD_PER_DAY, state, error) == JME_OK
        && !finite_state(state)) {
        fprintf(stderr, "orientation UTC state is non-finite\n");
        return 1;
    }

    if (jme_jpl_rotational_angular_momentum_state_naif(0.5 * (first_time + last_time), target, JME_ORIENTATION_RAD_PER_DAY, state, error) == JME_OK
        && !finite_state(state)) {
        fprintf(stderr, "rotational angular momentum state is non-finite\n");
        return 1;
    }

    if (jme_jpl_rotational_angular_momentum_state_split_naif(0.5 * (first_time + last_time), 0.0, target, JME_ORIENTATION_RAD_PER_DAY, state, error) == JME_OK
        && !finite_state(state)) {
        fprintf(stderr, "rotational angular momentum split state is non-finite\n");
        return 1;
    }

    if (jme_jpl_rotational_angular_momentum_state_order_naif(0.5 * (first_time + last_time), 0.0, target, JME_ORIENTATION_RAD_PER_DAY, 1, state, error) == JME_OK
        && !finite_values(state, 6)) {
        fprintf(stderr, "rotational angular momentum order state is non-finite\n");
        return 1;
    }

    if (jme_jpl_rotational_angular_momentum_state_utc_naif(2000, 1, 1, 12, 0, 0.0, JME_CALENDAR_GREGORIAN, target, JME_ORIENTATION_RAD_PER_DAY, state, error) == JME_OK
        && !finite_state(state)) {
        fprintf(stderr, "rotational angular momentum UTC state is non-finite\n");
        return 1;
    }

    jme_jpl_close();
    return 0;
}

static int test_version_kernel(const char *path)
{
    char error[256] = "";
    char version[1024] = "";

    if (path == 0 || path[0] == '\0') {
        return 0;
    }

    jme_jpl_close();
    if (jme_jpl_open(path, error) != JME_OK) {
        fprintf(stderr, "version kernel open failed: %s\n", error);
        return 1;
    }

    if (jme_jpl_file_version(version, sizeof(version), error) != JME_OK || version[0] == '\0') {
        fprintf(stderr, "file version success path failed: %s\n", error);
        return 1;
    }

    jme_jpl_close();
    return 0;
}

static int test_rotangmom_kernel(const char *path)
{
    char error[256] = "";
    double state[12] = {0.0};

    if (path == 0 || path[0] == '\0') {
        return 0;
    }

    jme_jpl_close();
    if (jme_jpl_open(path, error) != JME_OK) {
        fprintf(stderr, "rotational angular momentum kernel open failed: %s\n", error);
        return 1;
    }

    if (jme_jpl_rotational_angular_momentum_state_naif(2451419.5, 399, JME_ORIENTATION_RAD_PER_SECOND, state, error) != JME_OK
        || !finite_state(state)) {
        fprintf(stderr, "rotational angular momentum state failed: %s\n", error);
        return 1;
    }

    if (jme_jpl_rotational_angular_momentum_state_split_naif(2451419.0, 0.5, 399, JME_ORIENTATION_RAD_PER_SECOND, state, error) != JME_OK
        || !finite_state(state)) {
        fprintf(stderr, "rotational angular momentum split state failed: %s\n", error);
        return 1;
    }

    if (jme_jpl_rotational_angular_momentum_state_order_naif(2451419.0, 0.5, 399, JME_ORIENTATION_RAD_PER_SECOND, 1, state, error) != JME_OK
        || !finite_values(state, 6)) {
        fprintf(stderr, "rotational angular momentum order state failed: %s\n", error);
        return 1;
    }

    if (jme_jpl_rotational_angular_momentum_state_utc_naif(1999, 8, 29, 0, 0, 0.0, JME_CALENDAR_GREGORIAN, 399, JME_ORIENTATION_RAD_PER_SECOND, state, error) != JME_OK
        || !finite_state(state)) {
        fprintf(stderr, "rotational angular momentum UTC state failed: %s\n", error);
        return 1;
    }

    jme_jpl_close();
    return 0;
}

int main(void)
{
    const char *kernel = getenv("JME_TEST_JPL_KERNEL");
    const char *constant_kernel = getenv("JME_TEST_JPL_CONSTANT_KERNEL");
    const char *orientation_kernel = getenv("JME_TEST_JPL_ORIENTATION_KERNEL");
    const char *version_kernel = getenv("JME_TEST_JPL_VERSION_KERNEL");
    const char *rotangmom_kernel = getenv("JME_TEST_JPL_ROTANGMOM_KERNEL");
    char error[256] = "";
    double first_time = 0.0;
    double last_time = 0.0;
    int continuous = 0;
    double rectangular[6] = {0.0, 0.0, 0.0, 0.0, 0.0, 0.0};
    double ecliptic[6] = {0.0, 0.0, 0.0, 0.0, 0.0, 0.0};
    double calc_results[6] = {0.0, 0.0, 0.0, 0.0, 0.0, 0.0};
    double order_state[12] = {
        0.0, 0.0, 0.0, 0.0,
        0.0, 0.0, 0.0, 0.0,
        0.0, 0.0, 0.0, 0.0
    };
    double native_state[6] = {0.0, 0.0, 0.0, 0.0, 0.0, 0.0};
    char body_name[1024] = "";
    char file_version[1024] = "";
    char file_path[1024] = "";
    char engine_version[1024] = "";
    char constant_name[64] = "";
    char string_values[2048] = "";
    double constant_value = 0.0;
    double indexed_value = 0.0;
    int named_id = 0;
    int target = 0;
    int center = 0;
    int frame = 0;
    int segment_type = 0;
    int constant_count = 0;
    int orientation_count = 0;

    if ((kernel == 0 || kernel[0] == '\0')
        && (constant_kernel == 0 || constant_kernel[0] == '\0')
        && (orientation_kernel == 0 || orientation_kernel[0] == '\0')
        && (version_kernel == 0 || version_kernel[0] == '\0')
        && (rotangmom_kernel == 0 || rotangmom_kernel[0] == '\0')) {
        return 0;
    }

    if (!jme_jpl_is_available()) {
        fprintf(stderr, "JPL runtime env is set but CALCEPH support is disabled\n");
        return 1;
    }

    if (test_constant_kernel(constant_kernel) != 0) {
        return 1;
    }

    if (test_orientation_kernel(orientation_kernel) != 0) {
        return 1;
    }

    if (test_version_kernel(version_kernel) != 0) {
        return 1;
    }

    if (test_rotangmom_kernel(rotangmom_kernel) != 0) {
        return 1;
    }

    if (kernel == 0 || kernel[0] == '\0') {
        return 0;
    }

    if (jme_jpl_open(kernel, error) != JME_OK) {
        fprintf(stderr, "kernel open failed: %s\n", error);
        return 1;
    }

    {
        const char *paths[1] = {kernel};

        if (jme_jpl_open_array(1, paths, error) != JME_OK) {
            fprintf(stderr, "kernel array open failed: %s\n", error);
            return 1;
        }
    }

    if (!jme_jpl_is_open()) {
        fprintf(stderr, "kernel open state mismatch\n");
        return 1;
    }

    if (jme_jpl_engine_version(engine_version, sizeof(engine_version)) == 0 || engine_version[0] == '\0') {
        fprintf(stderr, "engine version query returned empty string\n");
        return 1;
    }

    if (jme_jpl_timescale() != JME_JPL_TIMESCALE_TDB && jme_jpl_timescale() != JME_JPL_TIMESCALE_TCB) {
        fprintf(stderr, "kernel timescale query returned unexpected value\n");
        return 1;
    }

    if (jme_jpl_coverage(&first_time, &last_time, &continuous, error) != JME_OK) {
        fprintf(stderr, "coverage query failed: %s\n", error);
        return 1;
    }

    if (jme_jpl_current_file_data(
            file_path,
            sizeof(file_path),
            &first_time,
            &last_time,
            &continuous,
            error
        ) != JME_OK) {
        fprintf(stderr, "current file data query failed: %s\n", error);
        return 1;
    }

    if (file_path[0] == '\0' || !(first_time < last_time)) {
        fprintf(stderr, "current file data returned invalid metadata\n");
        return 1;
    }

    if (jme_jpl_current_file_data(file_path, sizeof(file_path), 0, &last_time, &continuous, error) != JME_ERR
        || jme_jpl_current_file_data(file_path, sizeof(file_path), &first_time, 0, &continuous, error) != JME_ERR
        || jme_jpl_current_file_data(file_path, sizeof(file_path), &first_time, &last_time, 0, error) != JME_ERR) {
        fprintf(stderr, "current file data accepted null metadata outputs\n");
        return 1;
    }

    if (!(first_time < 2451545.0 && last_time > 2451545.0)) {
        jme_jpl_close();
        return 0;
    }

    if (jme_jpl_prefetch(error) != JME_OK) {
        fprintf(stderr, "prefetch failed: %s\n", error);
        return 1;
    }

    if (jme_jpl_is_thread_safe(error) < 0) {
        fprintf(stderr, "thread query failed: %s\n", error);
        return 1;
    }

    if (jme_jpl_id_by_name("EARTH", &named_id, error) != JME_OK || named_id != 399) {
        fprintf(stderr, "EARTH name lookup failed or returned unexpected id %d: %s\n", named_id, error);
        return 1;
    }

    if (jme_jpl_name_by_id(399, body_name, sizeof(body_name), error) == JME_OK && body_name[0] == '\0') {
        fprintf(stderr, "Earth id lookup returned empty name\n");
        return 1;
    }

    (void)jme_jpl_file_version(file_version, sizeof(file_version), error);

    constant_count = jme_jpl_constant_count(error);

    if (constant_count > 0) {
        if (jme_jpl_constant_index(1, constant_name, sizeof(constant_name), &constant_value, error) != JME_OK) {
            fprintf(stderr, "constant index failed: %s\n", error);
            return 1;
        }

        if (constant_name[0] == '\0' || !isfinite(constant_value)) {
            fprintf(stderr, "constant index returned invalid data\n");
            return 1;
        }

        if (jme_jpl_constant(constant_name, &indexed_value, error) != JME_OK) {
            fprintf(stderr, "indexed constant lookup failed: %s\n", error);
            return 1;
        }

        if (fabs(indexed_value - constant_value) > 1.0e-12 * fmax(1.0, fabs(constant_value))) {
            fprintf(stderr, "indexed constant value mismatch %.17g %.17g\n", indexed_value, constant_value);
            return 1;
        }

        (void)jme_jpl_constant_vector(constant_name, &constant_value, 1, error);
        (void)jme_jpl_constant_string(constant_name, body_name, sizeof(body_name), error);
    }

    if (jme_jpl_constant_string_vector("GROUP", string_values, 64, 1, error) == JME_OK
        && string_values[0] == '\0') {
        fprintf(stderr, "string vector query returned empty success\n");
        return 1;
    }

    if (jme_jpl_position_record_count(error) <= 0) {
        fprintf(stderr, "position record count failed: %s\n", error);
        return 1;
    }

    if (jme_jpl_position_record_index(
            1,
            &target,
            &center,
            &first_time,
            &last_time,
            &frame,
            &segment_type,
            error
        ) != JME_OK) {
        fprintf(stderr, "position record index failed: %s\n", error);
        return 1;
    }

    if (target == center || !(first_time < last_time)) {
        fprintf(stderr, "position record returned invalid metadata\n");
        return 1;
    }

    if (jme_jpl_max_supported_order(segment_type) < 0) {
        fprintf(stderr, "max supported order failed\n");
        return 1;
    }

    orientation_count = jme_jpl_orientation_record_count(error);
    if (orientation_count < 0) {
        fprintf(stderr, "orientation record count failed: %s\n", error);
        return 1;
    }

    if (orientation_count > 0) {
        if (jme_jpl_orientation_record_index(
                1,
                &target,
                &first_time,
                &last_time,
                &frame,
                &segment_type,
                error
            ) != JME_OK) {
            fprintf(stderr, "orientation record index failed: %s\n", error);
            return 1;
        }

        if (!(first_time < last_time)) {
            fprintf(stderr, "orientation record returned invalid metadata\n");
            return 1;
        }

        if (jme_jpl_orientation_state_naif(
                0.5 * (first_time + last_time),
                target,
                JME_ORIENTATION_RAD_PER_DAY,
                rectangular,
                error
            ) != JME_OK) {
            fprintf(stderr, "orientation state failed: %s\n", error);
            return 1;
        }

        if (!finite_state(rectangular)) {
            fprintf(stderr, "orientation state is non-finite\n");
            return 1;
        }

        if (jme_jpl_orientation_state_split_naif(
                0.5 * (first_time + last_time),
                0.0,
                target,
                JME_ORIENTATION_RAD_PER_DAY,
                rectangular,
                error
            ) != JME_OK) {
            fprintf(stderr, "orientation split state failed: %s\n", error);
            return 1;
        }

        if (!finite_state(rectangular)) {
            fprintf(stderr, "orientation split state is non-finite\n");
            return 1;
        }

        if (jme_jpl_orientation_state_utc_naif(
                2000,
                1,
                1,
                12,
                0,
                0.0,
                JME_CALENDAR_GREGORIAN,
                target,
                JME_ORIENTATION_RAD_PER_DAY,
                rectangular,
                error
            ) == JME_OK && !finite_state(rectangular)) {
            fprintf(stderr, "orientation UTC state is non-finite\n");
            return 1;
        }

        if (jme_jpl_orientation_state_order_naif(
                0.5 * (first_time + last_time),
                0.0,
                target,
                JME_ORIENTATION_RAD_PER_DAY,
                1,
                order_state,
                error
            ) != JME_OK) {
            fprintf(stderr, "orientation order state failed: %s\n", error);
            return 1;
        }

        if (!finite_values(order_state, 6)) {
            fprintf(stderr, "orientation order state is non-finite\n");
            return 1;
        }

        if (jme_jpl_rotational_angular_momentum_state_naif(
                0.5 * (first_time + last_time),
                target,
                JME_ORIENTATION_RAD_PER_DAY,
                rectangular,
                error
            ) == JME_OK && !finite_state(rectangular)) {
            fprintf(stderr, "rotational angular momentum state is non-finite\n");
            return 1;
        }

        if (jme_jpl_rotational_angular_momentum_state_split_naif(
                0.5 * (first_time + last_time),
                0.0,
                target,
                JME_ORIENTATION_RAD_PER_DAY,
                rectangular,
                error
            ) == JME_OK && !finite_state(rectangular)) {
            fprintf(stderr, "rotational angular momentum split state is non-finite\n");
            return 1;
        }

        if (jme_jpl_rotational_angular_momentum_state_utc_naif(
                2000,
                1,
                1,
                12,
                0,
                0.0,
                JME_CALENDAR_GREGORIAN,
                target,
                JME_ORIENTATION_RAD_PER_DAY,
                rectangular,
                error
            ) == JME_OK && !finite_state(rectangular)) {
            fprintf(stderr, "rotational angular momentum UTC state is non-finite\n");
            return 1;
        }

        if (jme_jpl_rotational_angular_momentum_state_order_naif(
                0.5 * (first_time + last_time),
                0.0,
                target,
                JME_ORIENTATION_RAD_PER_DAY,
                1,
                order_state,
                error
            ) == JME_OK && !finite_values(order_state, 6)) {
            fprintf(stderr, "rotational angular momentum order state is non-finite\n");
            return 1;
        }
    }

    if (jme_jpl_body_state(
            2451545.0,
            JME_BODY_MOON,
            JME_BODY_EARTH,
            JME_VECTOR_AU_PER_DAY,
            rectangular,
            error
        ) != JME_OK) {
        fprintf(stderr, "rectangular state failed: %s\n", error);
        return 1;
    }

    if (jme_jpl_body_state_naif(
            2451545.0,
            301,
            399,
            JME_VECTOR_AU_PER_DAY,
            native_state,
            error
        ) != JME_OK) {
        fprintf(stderr, "NAIF rectangular state failed: %s\n", error);
        return 1;
    }

    if (!finite_state(native_state)) {
        fprintf(stderr, "NAIF rectangular state is non-finite\n");
        return 1;
    }

    if (jme_jpl_body_state_split(
            2451545.0,
            0.0,
            JME_BODY_MOON,
            JME_BODY_EARTH,
            JME_VECTOR_AU_PER_DAY,
            rectangular,
            error
        ) != JME_OK) {
        fprintf(stderr, "split rectangular state failed: %s\n", error);
        return 1;
    }

    if (jme_jpl_body_state_split_naif(
            2451545.0,
            0.0,
            301,
            399,
            JME_VECTOR_AU_PER_DAY,
            native_state,
            error
        ) != JME_OK) {
        fprintf(stderr, "NAIF split rectangular state failed: %s\n", error);
        return 1;
    }

    if (!finite_state(native_state)) {
        fprintf(stderr, "NAIF split rectangular state is non-finite\n");
        return 1;
    }

    if (!finite_state(rectangular)) {
        fprintf(stderr, "rectangular state is non-finite\n");
        return 1;
    }

    if (jme_jpl_body_state_native_naif(
            2451545.0,
            301,
            399,
            native_state,
            error
        ) != JME_OK) {
        fprintf(stderr, "native rectangular state failed: %s\n", error);
        return 1;
    }

    if (!finite_state(native_state)) {
        fprintf(stderr, "native rectangular state is non-finite\n");
        return 1;
    }

    if (jme_jpl_body_state_native_split_naif(
            2451545.0,
            0.0,
            301,
            399,
            native_state,
            error
        ) != JME_OK) {
        fprintf(stderr, "native split rectangular state failed: %s\n", error);
        return 1;
    }

    if (!finite_state(native_state)) {
        fprintf(stderr, "native split rectangular state is non-finite\n");
        return 1;
    }

    if (jme_jpl_body_state_native_split(
            2451545.0,
            0.0,
            JME_BODY_MOON,
            JME_BODY_EARTH,
            native_state,
            error
        ) != JME_OK) {
        fprintf(stderr, "native enum split rectangular state failed: %s\n", error);
        return 1;
    }

    if (!finite_state(native_state)) {
        fprintf(stderr, "native enum split rectangular state is non-finite\n");
        return 1;
    }

    if (jme_jpl_body_state_native(
            2451545.0,
            JME_BODY_MOON,
            JME_BODY_EARTH,
            native_state,
            error
        ) != JME_OK) {
        fprintf(stderr, "native enum rectangular state failed: %s\n", error);
        return 1;
    }

    if (!finite_state(native_state)) {
        fprintf(stderr, "native enum rectangular state is non-finite\n");
        return 1;
    }

    if (jme_jpl_body_state_utc_naif(
            2000,
            1,
            1,
            12,
            0,
            0.0,
            JME_CALENDAR_GREGORIAN,
            301,
            399,
            JME_VECTOR_AU_PER_DAY,
            native_state,
            error
        ) != JME_OK) {
        fprintf(stderr, "UTC rectangular state failed: %s\n", error);
        return 1;
    }

    if (!finite_state(native_state)) {
        fprintf(stderr, "UTC rectangular state is non-finite\n");
        return 1;
    }

    if (jme_jpl_body_state_utc(
            2000,
            1,
            1,
            12,
            0,
            0.0,
            JME_CALENDAR_GREGORIAN,
            JME_BODY_MOON,
            JME_BODY_EARTH,
            JME_VECTOR_AU_PER_DAY,
            native_state,
            error
        ) != JME_OK) {
        fprintf(stderr, "UTC enum rectangular state failed: %s\n", error);
        return 1;
    }

    if (!finite_state(native_state)) {
        fprintf(stderr, "UTC enum rectangular state is non-finite\n");
        return 1;
    }

    if (jme_jpl_body_state_order_naif(
            2451545.0,
            0.0,
            301,
            399,
            JME_VECTOR_AU_PER_DAY,
            1,
            order_state,
            error
        ) != JME_OK) {
        fprintf(stderr, "body order state failed: %s\n", error);
        return 1;
    }

    if (!finite_values(order_state, 6)) {
        fprintf(stderr, "body order state is non-finite\n");
        return 1;
    }

    if (jme_jpl_body_state_order_naif(
            2451545.0,
            0.0,
            301,
            399,
            JME_VECTOR_AU_PER_DAY,
            3,
            order_state,
            error
        ) != JME_OK) {
        fprintf(stderr, "body third-order state failed: %s\n", error);
        return 1;
    }

    if (!finite_values(order_state, 12)) {
        fprintf(stderr, "body third-order state is non-finite\n");
        return 1;
    }

    if (jme_jpl_body_state_order(
            2451545.0,
            0.0,
            JME_BODY_MOON,
            JME_BODY_EARTH,
            JME_VECTOR_AU_PER_DAY,
            1,
            order_state,
            error
        ) != JME_OK) {
        fprintf(stderr, "body enum order state failed: %s\n", error);
        return 1;
    }

    if (!finite_values(order_state, 6)) {
        fprintf(stderr, "body enum order state is non-finite\n");
        return 1;
    }

    if (jme_jpl_ecliptic_state_naif(
            2451545.0,
            301,
            399,
            JME_VECTOR_AU_PER_DAY,
            ecliptic,
            error
        ) != JME_OK) {
        fprintf(stderr, "NAIF ecliptic state failed: %s\n", error);
        return 1;
    }

    if (!finite_state(ecliptic)) {
        fprintf(stderr, "NAIF ecliptic state is non-finite\n");
        return 1;
    }

    if (jme_jpl_ecliptic_state(
            2451545.0,
            JME_BODY_MOON,
            JME_BODY_EARTH,
            JME_VECTOR_AU_PER_DAY,
            ecliptic,
            error
        ) != JME_OK) {
        fprintf(stderr, "ecliptic state failed: %s\n", error);
        return 1;
    }

    if (jme_jpl_ecliptic_state_split(
            2451545.0,
            0.0,
            JME_BODY_MOON,
            JME_BODY_EARTH,
            JME_VECTOR_AU_PER_DAY,
            ecliptic,
            error
        ) != JME_OK) {
        fprintf(stderr, "split ecliptic state failed: %s\n", error);
        return 1;
    }

    if (jme_jpl_ecliptic_state_split_naif(
            2451545.0,
            0.0,
            301,
            399,
            JME_VECTOR_AU_PER_DAY,
            ecliptic,
            error
        ) != JME_OK) {
        fprintf(stderr, "NAIF split ecliptic state failed: %s\n", error);
        return 1;
    }

    if (!finite_state(ecliptic)) {
        fprintf(stderr, "NAIF split ecliptic state is non-finite\n");
        return 1;
    }

    if (
        !finite_state(ecliptic) ||
        ecliptic[0] < 0.0 ||
        ecliptic[0] >= 360.0 ||
        ecliptic[1] < -90.0 ||
        ecliptic[1] > 90.0 ||
        ecliptic[2] <= 0.0
    ) {
        fprintf(stderr, "ecliptic state range mismatch\n");
        return 1;
    }

    if (jme_jpl_ecliptic_state_utc_naif(
            2000,
            1,
            1,
            12,
            0,
            0.0,
            JME_CALENDAR_GREGORIAN,
            301,
            399,
            JME_VECTOR_AU_PER_DAY,
            ecliptic,
            error
        ) != JME_OK) {
        fprintf(stderr, "UTC ecliptic state failed: %s\n", error);
        return 1;
    }

    if (!finite_state(ecliptic)) {
        fprintf(stderr, "UTC ecliptic state is non-finite\n");
        return 1;
    }

    if (jme_jpl_ecliptic_state_utc(
            2000,
            1,
            1,
            12,
            0,
            0.0,
            JME_CALENDAR_GREGORIAN,
            JME_BODY_MOON,
            JME_BODY_EARTH,
            JME_VECTOR_AU_PER_DAY,
            ecliptic,
            error
        ) != JME_OK) {
        fprintf(stderr, "UTC enum ecliptic state failed: %s\n", error);
        return 1;
    }

    if (!finite_state(ecliptic)) {
        fprintf(stderr, "UTC enum ecliptic state is non-finite\n");
        return 1;
    }

    if (jme_calc(2451545.0, JME_BODY_SUN, JME_CALC_XYZ | JME_CALC_J2000, calc_results, error) != JME_OK) {
        fprintf(stderr, "high-level calc failed with kernel: %s\n", error);
        return 1;
    }

    if (!finite_state(calc_results) || fabs(jme_state_distance(calc_results) - 0.98328) > 0.02) {
        fprintf(stderr, "high-level calc Sun distance mismatch\n");
        return 1;
    }

    if (jme_calc(2451545.0, JME_BODY_MOON, JME_CALC_TRUE_POSITION, calc_results, error) != JME_OK) {
        fprintf(stderr, "high-level Moon calc failed with kernel: %s\n", error);
        return 1;
    }

    if (!finite_state(calc_results) || calc_results[2] <= 0.0 || calc_results[2] > 0.01) {
        fprintf(stderr, "high-level Moon distance range mismatch\n");
        return 1;
    }

    jme_jpl_close();
    if (jme_jpl_is_open()) {
        fprintf(stderr, "kernel close state mismatch\n");
        return 1;
    }

    return 0;
}
