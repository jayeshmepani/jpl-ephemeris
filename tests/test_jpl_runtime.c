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

int main(void)
{
    const char *kernel = getenv("JME_TEST_JPL_KERNEL");
    char error[256] = "";
    double first_time = 0.0;
    double last_time = 0.0;
    int continuous = 0;
    double rectangular[6] = {0.0, 0.0, 0.0, 0.0, 0.0, 0.0};
    double ecliptic[6] = {0.0, 0.0, 0.0, 0.0, 0.0, 0.0};
    double order_state[12] = {
        0.0, 0.0, 0.0, 0.0,
        0.0, 0.0, 0.0, 0.0,
        0.0, 0.0, 0.0, 0.0
    };
    double native_state[6] = {0.0, 0.0, 0.0, 0.0, 0.0, 0.0};
    char body_name[1024] = "";
    char file_version[1024] = "";
    char file_path[1024] = "";
    char constant_name[64] = "";
    double constant_value = 0.0;
    int named_id = 0;
    int target = 0;
    int center = 0;
    int frame = 0;
    int segment_type = 0;
    int constant_count = 0;
    int orientation_count = 0;

    if (kernel == 0 || kernel[0] == '\0') {
        return 0;
    }

    if (!jme_jpl_is_available()) {
        fprintf(stderr, "JME_TEST_JPL_KERNEL is set but CALCEPH support is disabled\n");
        return 1;
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

    (void)jme_jpl_id_by_name("EARTH", &named_id, error);
    (void)jme_jpl_name_by_id(399, body_name, sizeof(body_name), error);

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

        (void)jme_jpl_constant_vector(constant_name, &constant_value, 1, error);
        (void)jme_jpl_constant_string(constant_name, body_name, sizeof(body_name), error);
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

    jme_jpl_close();
    if (jme_jpl_is_open()) {
        fprintf(stderr, "kernel close state mismatch\n");
        return 1;
    }

    return 0;
}
