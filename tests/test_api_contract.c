#include "jme/jme.h"

#include <math.h>
#include <stdio.h>
#include <string.h>

int main(void)
{
    char version[64];
    double ra = 0.0;
    double dec = 0.0;
    double lon = 0.0;
    double lat = 0.0;
    double state[6] = {1.0, 1.0, 1.0, 1.0, 1.0, 1.0};
    double order_state[12] = {
        1.0, 1.0, 1.0, 1.0,
        1.0, 1.0, 1.0, 1.0,
        1.0, 1.0, 1.0, 1.0
    };
    double rectangular[6] = {1.0, 1.0, 0.0, 0.0, 1.0, 0.0};
    double spherical[6] = {0.0, 0.0, 0.0, 0.0, 0.0, 0.0};
    double converted[6] = {0.0, 0.0, 0.0, 0.0, 0.0, 0.0};
    double round_trip_state[6] = {0.0, 0.0, 0.0, 0.0, 0.0, 0.0};
    char error[256] = "";
    char name[64] = "";
    char path[256] = "";
    char values[2][64];
    double value = 1.0;
    double vector_values[2] = {1.0, 1.0};
    double first_time = 1.0;
    double last_time = 1.0;
    int y = 0;
    int m = 0;
    int d = 0;
    int h = 0;
    int min = 0;
    int mode = 0;
    int continuous = 1;
    double sec = 0.0;

    if (strcmp(jme_version(version, sizeof(version)), JME_VERSION) != 0) {
        fprintf(stderr, "version return mismatch\n");
        return 1;
    }

    if (strcmp(version, JME_VERSION) != 0) {
        fprintf(stderr, "version buffer mismatch\n");
        return 1;
    }

    jme_set_ephemeris_path(".");
    jme_set_jpl_file("kernel.bsp");
    jme_set_sidereal_mode(JME_SIDEREAL_LAHIRI, 2451545.0, 24.0);
    jme_close();

    if (strcmp(jme_ephemeris_path(), ".") != 0 || strcmp(jme_jpl_file(), "kernel.bsp") != 0) {
        fprintf(stderr, "context path getter mismatch\n");
        return 1;
    }

    jme_get_sidereal_mode(&mode, &value, &sec);
    if (mode != JME_SIDEREAL_LAHIRI || value != 2451545.0 || sec != 24.0) {
        fprintf(stderr, "sidereal mode getter mismatch\n");
        return 1;
    }

    if (!jme_calendar_is_leap_year(2000, JME_CALENDAR_GREGORIAN) || jme_calendar_is_leap_year(1900, JME_CALENDAR_GREGORIAN)) {
        fprintf(stderr, "Gregorian leap-year mismatch\n");
        return 1;
    }

    if (jme_days_in_month(2024, 2, JME_CALENDAR_GREGORIAN) != 29 || jme_day_of_year(2024, 3, 1, JME_CALENDAR_GREGORIAN) != 61) {
        fprintf(stderr, "calendar day helper mismatch\n");
        return 1;
    }

    if (fabs(jme_decimal_hour(1, 30, 30.0) - 1.5083333333333333) > 1e-15) {
        fprintf(stderr, "decimal hour mismatch\n");
        return 1;
    }

    if (jme_degree_normalize(-1.0) != 359.0) {
        fprintf(stderr, "degree normalization mismatch\n");
        return 1;
    }

    if (fabs(jme_degrees_to_radians(180.0) - acos(-1.0)) > 1e-15) {
        fprintf(stderr, "degree to radian mismatch\n");
        return 1;
    }

    if (fabs(jme_radians_to_degrees(acos(-1.0)) - 180.0) > 1e-14) {
        fprintf(stderr, "radian to degree mismatch\n");
        return 1;
    }

    if (fabs(jme_hours_normalize(-1.0) - 23.0) > 1e-15) {
        fprintf(stderr, "hour normalization mismatch\n");
        return 1;
    }

    if (fabs(jme_degrees_to_hours(30.0) - 2.0) > 1e-15 || fabs(jme_hours_to_degrees(2.0) - 30.0) > 1e-15) {
        fprintf(stderr, "hour/degree conversion mismatch\n");
        return 1;
    }

    if (jme_degrees_difference_signed(10.0, 350.0) != 20.0) {
        fprintf(stderr, "signed degree difference mismatch\n");
        return 1;
    }

    if (jme_centiseconds_normalize(-1) != 129599999) {
        fprintf(stderr, "centisecond normalization mismatch\n");
        return 1;
    }

    if (jme_double_to_long(-1.5) != -2 || jme_double_to_long(1.5) != 2) {
        fprintf(stderr, "double-to-long rounding mismatch\n");
        return 1;
    }

    if (strcmp(jme_body_name(JME_BODY_SUN), "Sun") != 0 || jme_body_id_from_name("Mars") != JME_BODY_MARS) {
        fprintf(stderr, "body metadata mismatch\n");
        return 1;
    }

    if (jme_copy_body_name(JME_BODY_MOON, name) == 0 || strcmp(name, "Moon") != 0) {
        fprintf(stderr, "body name copy mismatch\n");
        return 1;
    }

    if (jme_body_naif_id(JME_BODY_EARTH) != 399 || jme_body_naif_id(JME_BODY_MOON) != 301) {
        fprintf(stderr, "NAIF metadata mismatch\n");
        return 1;
    }

    if (
        jme_body_naif_id(JME_BODY_SOLAR_SYSTEM_BARYCENTER) != 0 ||
        jme_body_naif_id(JME_BODY_EARTH_MOON_BARYCENTER) != 3 ||
        jme_body_naif_id(JME_BODY_JUPITER_BARYCENTER) != 5
    ) {
        fprintf(stderr, "NAIF barycenter metadata mismatch\n");
        return 1;
    }

    if (strcmp(jme_house_system_name('P'), "Placidus") != 0 || strcmp(jme_house_system_name('K'), "Koch") != 0) {
        fprintf(stderr, "house system name mismatch\n");
        return 1;
    }

    if (fabs(jme_sidereal_time(2451545.0) - 18.697374558) > 1e-9) {
        fprintf(stderr, "sidereal time mismatch: %.17g\n", jme_sidereal_time(2451545.0));
        return 1;
    }

    jme_ecliptic_to_equatorial(120.0, 5.0, 23.4392911, &ra, &dec);
    jme_equatorial_to_ecliptic(ra, dec, 23.4392911, &lon, &lat);
    if (fabs(jme_degrees_difference_signed(lon, 120.0)) > 1e-10 || fabs(lat - 5.0) > 1e-10) {
        fprintf(stderr, "coordinate round-trip mismatch: %.17g %.17g\n", lon, lat);
        return 1;
    }

    if (fabs(jme_spherical_angular_separation(0.0, 0.0, 90.0, 0.0) - 90.0) > 1e-12) {
        fprintf(stderr, "spherical angular separation mismatch\n");
        return 1;
    }

    if (fabs(jme_spherical_position_angle(0.0, 0.0, 90.0, 0.0) - 90.0) > 1e-12) {
        fprintf(stderr, "spherical position angle mismatch\n");
        return 1;
    }

    if (jme_ecliptic_to_equatorial_rectangular_state(rectangular, 23.4392911, converted) != JME_OK) {
        fprintf(stderr, "ecliptic rectangular state conversion failed\n");
        return 1;
    }

    if (jme_equatorial_to_ecliptic_rectangular_state(converted, 23.4392911, round_trip_state) != JME_OK) {
        fprintf(stderr, "equatorial rectangular state conversion failed\n");
        return 1;
    }

    if (
        fabs(round_trip_state[0] - rectangular[0]) > 1e-12 ||
        fabs(round_trip_state[1] - rectangular[1]) > 1e-12 ||
        fabs(round_trip_state[2] - rectangular[2]) > 1e-12 ||
        fabs(round_trip_state[3] - rectangular[3]) > 1e-12 ||
        fabs(round_trip_state[4] - rectangular[4]) > 1e-12 ||
        fabs(round_trip_state[5] - rectangular[5]) > 1e-12
    ) {
        fprintf(stderr, "rectangular obliquity state round-trip mismatch\n");
        return 1;
    }

    jme_equatorial_to_horizontal(45.0, 20.0, 23.5, &lon, &lat);
    jme_horizontal_to_equatorial(lon, lat, 23.5, &ra, &dec);
    if (fabs(jme_degrees_difference_signed(ra, 45.0)) > 1e-10 || fabs(dec - 20.0) > 1e-10) {
        fprintf(stderr, "horizontal coordinate round-trip mismatch: %.17g %.17g\n", ra, dec);
        return 1;
    }

    if (jme_refract(15.0, 1010.0, 10.0, JME_COORD_TRUE_TO_APPARENT) <= 15.0) {
        fprintf(stderr, "refraction did not increase true altitude\n");
        return 1;
    }

    if (jme_rectangular_to_spherical_state(rectangular, spherical) != JME_OK) {
        fprintf(stderr, "rectangular to spherical state failed\n");
        return 1;
    }

    if (
        fabs(spherical[0] - 45.0) > 1e-12 ||
        fabs(spherical[1]) > 1e-12 ||
        fabs(spherical[2] - sqrt(2.0)) > 1e-12 ||
        fabs(spherical[3] - (0.5 * 57.295779513082320876798154814105170332405472466565)) > 1e-12 ||
        fabs(spherical[4]) > 1e-12 ||
        fabs(spherical[5] - (1.0 / sqrt(2.0))) > 1e-12
    ) {
        fprintf(stderr, "rectangular to spherical state mismatch\n");
        return 1;
    }

    if (jme_spherical_to_rectangular_state(spherical, round_trip_state) != JME_OK) {
        fprintf(stderr, "spherical to rectangular state failed\n");
        return 1;
    }

    if (
        fabs(round_trip_state[0] - rectangular[0]) > 1e-12 ||
        fabs(round_trip_state[1] - rectangular[1]) > 1e-12 ||
        fabs(round_trip_state[2] - rectangular[2]) > 1e-12 ||
        fabs(round_trip_state[3] - rectangular[3]) > 1e-12 ||
        fabs(round_trip_state[4] - rectangular[4]) > 1e-12 ||
        fabs(round_trip_state[5] - rectangular[5]) > 1e-12
    ) {
        fprintf(stderr, "spherical rectangular round-trip mismatch\n");
        return 1;
    }

    if (fabs(jme_state_distance(rectangular) - sqrt(2.0)) > 1e-12) {
        fprintf(stderr, "state distance mismatch\n");
        return 1;
    }

    if (fabs(jme_state_speed(rectangular) - 1.0) > 1e-12) {
        fprintf(stderr, "state speed mismatch\n");
        return 1;
    }

    if (fabs(jme_state_position_velocity_dot(rectangular) - 1.0) > 1e-12) {
        fprintf(stderr, "state position-velocity dot mismatch\n");
        return 1;
    }

    if (jme_state_add(rectangular, rectangular, round_trip_state) != JME_OK || fabs(round_trip_state[0] - 2.0) > 1e-12) {
        fprintf(stderr, "state add mismatch\n");
        return 1;
    }

    if (jme_state_subtract(round_trip_state, rectangular, round_trip_state) != JME_OK || fabs(round_trip_state[0] - 1.0) > 1e-12) {
        fprintf(stderr, "state subtract mismatch\n");
        return 1;
    }

    if (jme_state_scale(rectangular, 3.0, round_trip_state) != JME_OK || fabs(round_trip_state[1] - 3.0) > 1e-12) {
        fprintf(stderr, "state scale mismatch\n");
        return 1;
    }

    if (jme_state_convert_units(rectangular, JME_VECTOR_AU_PER_DAY, JME_VECTOR_KM_PER_SECOND, converted) != JME_OK) {
        fprintf(stderr, "state conversion failed\n");
        return 1;
    }

    if (
        fabs(converted[0] - JME_AU_KM) > 1e-6 ||
        fabs(converted[1] - JME_AU_KM) > 1e-6 ||
        fabs(converted[2]) > 1e-12 ||
        fabs(converted[3]) > 1e-12 ||
        fabs(converted[4] - (JME_AU_KM / JME_SECONDS_PER_DAY)) > 1e-9 ||
        fabs(converted[5]) > 1e-12
    ) {
        fprintf(stderr, "state conversion mismatch\n");
        return 1;
    }

    if (fabs(jme_state_light_time_days(rectangular, JME_VECTOR_AU_PER_DAY) - (sqrt(2.0) * JME_AU_KM / JME_SPEED_OF_LIGHT_KM_PER_SEC / JME_SECONDS_PER_DAY)) > 1e-15) {
        fprintf(stderr, "light-time mismatch\n");
        return 1;
    }

    jme_utc_time_zone(2026, 5, 18, 10, 30, 0.0, 5.5, &y, &m, &d, &h, &min, &sec);
    if (y != 2026 || m != 5 || d != 18 || h != 5 || min != 0 || sec > 0.000001) {
        fprintf(stderr, "timezone conversion mismatch: %d-%d-%d %d:%d %.17g\n", y, m, d, h, min, sec);
        return 1;
    }

    if (fabs(jme_jd_difference_seconds(jme_jd_add_seconds(2451545.0, 90.0), 2451545.0) - 90.0) > 1e-5) {
        fprintf(stderr, "JD seconds helper mismatch\n");
        return 1;
    }

    if (!jme_jpl_is_available()) {
        if (jme_jpl_open("", error) != JME_ERR) {
            fprintf(stderr, "JPL open unexpectedly succeeded without CALCEPH\n");
            return 1;
        }

        {
            const char *paths[1] = {"missing.bsp"};

            if (jme_jpl_open_array(1, paths, error) != JME_ERR) {
                fprintf(stderr, "JPL array open unexpectedly succeeded without CALCEPH\n");
                return 1;
            }
        }

        if (jme_jpl_prefetch(error) != JME_ERR) {
            fprintf(stderr, "JPL prefetch unexpectedly succeeded without CALCEPH\n");
            return 1;
        }

        if (jme_jpl_is_thread_safe(error) != JME_ERR) {
            fprintf(stderr, "JPL thread query unexpectedly succeeded without CALCEPH\n");
            return 1;
        }

        if (jme_jpl_id_by_name("EARTH", &h, error) != JME_ERR || h != 0) {
            fprintf(stderr, "JPL id lookup error path mismatch\n");
            return 1;
        }

        if (jme_jpl_name_by_id(399, name, sizeof(name), error) != JME_ERR || name[0] != '\0') {
            fprintf(stderr, "JPL name lookup error path mismatch\n");
            return 1;
        }

        if (jme_jpl_max_supported_order(2) != JME_ERR) {
            fprintf(stderr, "JPL max order unexpectedly succeeded without CALCEPH\n");
            return 1;
        }

        if (jme_jpl_constant_count(error) != JME_ERR) {
            fprintf(stderr, "JPL constant count unexpectedly succeeded without CALCEPH\n");
            return 1;
        }

        if (jme_jpl_constant_index(1, name, sizeof(name), &value, error) != JME_ERR || name[0] != '\0' || value != 0.0) {
            fprintf(stderr, "JPL constant index error path mismatch\n");
            return 1;
        }

        vector_values[0] = 1.0;
        vector_values[1] = 1.0;
        if (jme_jpl_constant_vector("X", vector_values, 2, error) != JME_ERR || vector_values[0] != 0.0 || vector_values[1] != 0.0) {
            fprintf(stderr, "JPL constant vector error path mismatch\n");
            return 1;
        }

        if (jme_jpl_constant_string("X", name, sizeof(name), error) != JME_ERR || name[0] != '\0') {
            fprintf(stderr, "JPL constant string error path mismatch\n");
            return 1;
        }

        values[0][0] = 'x';
        values[1][0] = 'x';
        if (jme_jpl_constant_string_vector("X", values[0], sizeof(values[0]), 2, error) != JME_ERR || values[0][0] != '\0' || values[1][0] != '\0') {
            fprintf(stderr, "JPL constant string vector error path mismatch\n");
            return 1;
        }

        if (jme_jpl_file_version(name, sizeof(name), error) != JME_ERR || name[0] != '\0') {
            fprintf(stderr, "JPL file version error path mismatch\n");
            return 1;
        }

        path[0] = 'x';
        first_time = 1.0;
        last_time = 1.0;
        continuous = 1;
        if (
            jme_jpl_current_file_data(
                path,
                sizeof(path),
                &first_time,
                &last_time,
                &continuous,
                error
            ) != JME_ERR ||
            path[0] != '\0' ||
            first_time != 0.0 ||
            last_time != 0.0 ||
            continuous != 0
        ) {
            fprintf(stderr, "JPL current file data error path mismatch\n");
            return 1;
        }

        if (jme_jpl_body_state(
                2451545.0,
                JME_BODY_MOON,
                JME_BODY_EARTH,
                JME_VECTOR_AU_PER_DAY,
                state,
                error
            ) != JME_ERR) {
            fprintf(stderr, "JPL state unexpectedly succeeded without CALCEPH\n");
            return 1;
        }

        if (state[0] != 0.0 || state[1] != 0.0 || state[2] != 0.0) {
            fprintf(stderr, "JPL error path did not clear state\n");
            return 1;
        }

        state[0] = 1.0;
        state[1] = 1.0;
        state[2] = 1.0;
        if (jme_jpl_body_state_native_naif(
                2451545.0,
                301,
                399,
                state,
                error
            ) != JME_ERR) {
            fprintf(stderr, "JPL native state unexpectedly succeeded without CALCEPH\n");
            return 1;
        }

        if (state[0] != 0.0 || state[1] != 0.0 || state[2] != 0.0) {
            fprintf(stderr, "JPL native error path did not clear state\n");
            return 1;
        }

        state[0] = 1.0;
        state[1] = 1.0;
        state[2] = 1.0;
        if (jme_jpl_body_state_native(
                2451545.0,
                JME_BODY_MOON,
                JME_BODY_EARTH,
                state,
                error
            ) != JME_ERR) {
            fprintf(stderr, "JPL native enum state unexpectedly succeeded without CALCEPH\n");
            return 1;
        }

        if (state[0] != 0.0 || state[1] != 0.0 || state[2] != 0.0) {
            fprintf(stderr, "JPL native enum error path did not clear state\n");
            return 1;
        }

        state[0] = 1.0;
        state[1] = 1.0;
        state[2] = 1.0;
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
                state,
                error
            ) != JME_ERR) {
            fprintf(stderr, "JPL UTC state unexpectedly succeeded without CALCEPH\n");
            return 1;
        }

        if (state[0] != 0.0 || state[1] != 0.0 || state[2] != 0.0) {
            fprintf(stderr, "JPL UTC error path did not clear state\n");
            return 1;
        }

        state[0] = 1.0;
        state[1] = 1.0;
        state[2] = 1.0;
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
                state,
                error
            ) != JME_ERR) {
            fprintf(stderr, "JPL UTC enum state unexpectedly succeeded without CALCEPH\n");
            return 1;
        }

        if (state[0] != 0.0 || state[1] != 0.0 || state[2] != 0.0) {
            fprintf(stderr, "JPL UTC enum error path did not clear state\n");
            return 1;
        }

        state[0] = 1.0;
        state[1] = 1.0;
        state[2] = 1.0;
        if (jme_jpl_body_state_split(
                2451545.0,
                0.0,
                JME_BODY_MOON,
                JME_BODY_EARTH,
                JME_VECTOR_AU_PER_DAY,
                state,
                error
            ) != JME_ERR) {
            fprintf(stderr, "JPL split state unexpectedly succeeded without CALCEPH\n");
            return 1;
        }

        if (state[0] != 0.0 || state[1] != 0.0 || state[2] != 0.0) {
            fprintf(stderr, "JPL split error path did not clear state\n");
            return 1;
        }

        state[0] = 1.0;
        state[1] = 1.0;
        state[2] = 1.0;
        if (jme_jpl_ecliptic_state(
                2451545.0,
                JME_BODY_MOON,
                JME_BODY_EARTH,
                JME_VECTOR_AU_PER_DAY,
                state,
                error
            ) != JME_ERR) {
            fprintf(stderr, "JPL ecliptic state unexpectedly succeeded without CALCEPH\n");
            return 1;
        }

        if (state[0] != 0.0 || state[1] != 0.0 || state[2] != 0.0) {
            fprintf(stderr, "JPL ecliptic error path did not clear state\n");
            return 1;
        }

        state[0] = 1.0;
        state[1] = 1.0;
        state[2] = 1.0;
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
                state,
                error
            ) != JME_ERR) {
            fprintf(stderr, "JPL ecliptic UTC state unexpectedly succeeded without CALCEPH\n");
            return 1;
        }

        if (state[0] != 0.0 || state[1] != 0.0 || state[2] != 0.0) {
            fprintf(stderr, "JPL ecliptic UTC error path did not clear state\n");
            return 1;
        }

        state[0] = 1.0;
        state[1] = 1.0;
        state[2] = 1.0;
        if (jme_jpl_ecliptic_state_split(
                2451545.0,
                0.0,
                JME_BODY_MOON,
                JME_BODY_EARTH,
                JME_VECTOR_AU_PER_DAY,
                state,
                error
            ) != JME_ERR) {
            fprintf(stderr, "JPL ecliptic split state unexpectedly succeeded without CALCEPH\n");
            return 1;
        }

        if (state[0] != 0.0 || state[1] != 0.0 || state[2] != 0.0) {
            fprintf(stderr, "JPL ecliptic split error path did not clear state\n");
            return 1;
        }

        state[0] = 1.0;
        state[1] = 1.0;
        state[2] = 1.0;
        if (jme_jpl_orientation_state_naif(
                2451545.0,
                399,
                JME_ORIENTATION_RAD_PER_DAY,
                state,
                error
            ) != JME_ERR) {
            fprintf(stderr, "JPL orientation state unexpectedly succeeded without CALCEPH\n");
            return 1;
        }

        if (state[0] != 0.0 || state[1] != 0.0 || state[2] != 0.0) {
            fprintf(stderr, "JPL orientation error path did not clear state\n");
            return 1;
        }

        state[0] = 1.0;
        state[1] = 1.0;
        state[2] = 1.0;
        if (jme_jpl_orientation_state_utc_naif(
                2000,
                1,
                1,
                12,
                0,
                0.0,
                JME_CALENDAR_GREGORIAN,
                399,
                JME_ORIENTATION_RAD_PER_DAY,
                state,
                error
            ) != JME_ERR) {
            fprintf(stderr, "JPL orientation UTC state unexpectedly succeeded without CALCEPH\n");
            return 1;
        }

        if (state[0] != 0.0 || state[1] != 0.0 || state[2] != 0.0) {
            fprintf(stderr, "JPL orientation UTC error path did not clear state\n");
            return 1;
        }

        if (jme_jpl_orientation_state_order_naif(
                2451545.0,
                0.0,
                399,
                JME_ORIENTATION_RAD_PER_DAY,
                3,
                order_state,
                error
            ) != JME_ERR) {
            fprintf(stderr, "JPL orientation order state unexpectedly succeeded without CALCEPH\n");
            return 1;
        }

        if (order_state[0] != 0.0 || order_state[11] != 0.0) {
            fprintf(stderr, "JPL orientation order error path did not clear state\n");
            return 1;
        }

        order_state[0] = 1.0;
        order_state[11] = 1.0;
        if (jme_jpl_body_state_order_naif(
                2451545.0,
                0.0,
                301,
                399,
                JME_VECTOR_AU_PER_DAY,
                3,
                order_state,
                error
            ) != JME_ERR) {
            fprintf(stderr, "JPL body order state unexpectedly succeeded without CALCEPH\n");
            return 1;
        }

        if (order_state[0] != 0.0 || order_state[11] != 0.0) {
            fprintf(stderr, "JPL body order error path did not clear state\n");
            return 1;
        }

        order_state[0] = 1.0;
        order_state[11] = 1.0;
        if (jme_jpl_body_state_order(
                2451545.0,
                0.0,
                JME_BODY_MOON,
                JME_BODY_EARTH,
                JME_VECTOR_AU_PER_DAY,
                3,
                order_state,
                error
            ) != JME_ERR) {
            fprintf(stderr, "JPL body order enum state unexpectedly succeeded without CALCEPH\n");
            return 1;
        }

        if (order_state[0] != 0.0 || order_state[11] != 0.0) {
            fprintf(stderr, "JPL body order enum error path did not clear state\n");
            return 1;
        }

        state[0] = 1.0;
        state[1] = 1.0;
        state[2] = 1.0;
        if (jme_jpl_rotational_angular_momentum_state_naif(
                2451545.0,
                399,
                JME_ORIENTATION_RAD_PER_DAY,
                state,
                error
            ) != JME_ERR) {
            fprintf(stderr, "JPL rotational angular momentum unexpectedly succeeded without CALCEPH\n");
            return 1;
        }

        if (state[0] != 0.0 || state[1] != 0.0 || state[2] != 0.0) {
            fprintf(stderr, "JPL rotational angular momentum error path did not clear state\n");
            return 1;
        }

        state[0] = 1.0;
        state[1] = 1.0;
        state[2] = 1.0;
        if (jme_jpl_rotational_angular_momentum_state_utc_naif(
                2000,
                1,
                1,
                12,
                0,
                0.0,
                JME_CALENDAR_GREGORIAN,
                399,
                JME_ORIENTATION_RAD_PER_DAY,
                state,
                error
            ) != JME_ERR) {
            fprintf(stderr, "JPL rotational angular momentum UTC unexpectedly succeeded without CALCEPH\n");
            return 1;
        }

        if (state[0] != 0.0 || state[1] != 0.0 || state[2] != 0.0) {
            fprintf(stderr, "JPL rotational angular momentum UTC error path did not clear state\n");
            return 1;
        }

        if (jme_jpl_orientation_record_count(error) != JME_ERR) {
            fprintf(stderr, "JPL orientation count unexpectedly succeeded without CALCEPH\n");
            return 1;
        }
    }

    return 0;
}
