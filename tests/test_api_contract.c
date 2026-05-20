#include "jme/jme.h"

#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

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

    if (fabs(jme_delta_t(2451545.0) - 63.8735) > 0.001) {
        fprintf(stderr, "delta_t mismatch at J2000: %.17g\n", jme_delta_t(2451545.0));
        return 1;
    }

    if (JME_CALC_NO_ABERRATION != (1 << 10)
        || JME_CALC_NO_LIGHT_DEFLECTION != (1 << 11)
        || JME_CALC_TOPOCENTRIC != (1 << 17)
        || JME_RISE_RISE != (1 << 0)
        || JME_RISE_SET != (1 << 1)
        || JME_RISE_MERIDIAN_TRANSIT != (1 << 2)) {
        fprintf(stderr, "bit-flag constant contract mismatch\n");
        return 1;
    }

    {
        int bodies[] = {
            JME_BODY_SUN,
            JME_BODY_MOON,
            JME_BODY_MEAN_NODE,
            JME_BODY_TRUE_NODE,
            JME_BODY_MERCURY,
            JME_BODY_VENUS,
            JME_BODY_MARS,
            JME_BODY_JUPITER,
            JME_BODY_SATURN,
            JME_BODY_URANUS,
            JME_BODY_NEPTUNE,
            JME_BODY_PLUTO
        };
        int body_count = (int)(sizeof(bodies) / sizeof(bodies[0]));
        int body_index;

        if (jme_calc(2451545.0, JME_BODY_SUN, JME_CALC_NONE, 0, error) != JME_ERR) {
            fprintf(stderr, "jme_calc unexpectedly accepted a null output buffer\n");
            return 1;
        }

        for (body_index = 0; body_index < body_count; body_index++) {
            double results[6] = {0.0, 0.0, 0.0, 0.0, 0.0, 0.0};
            int rc = jme_calc_ut(2451545.0, bodies[body_index], JME_CALC_NONE, results, error);

            if (rc == JME_OK && !finite_values(results, 6)) {
                fprintf(stderr, "jme_calc_ut returned non-finite success for body %d\n", bodies[body_index]);
                return 1;
            }
        }

        {
            double deg_results[6] = {0.0, 0.0, 0.0, 0.0, 0.0, 0.0};
            double rad_results[6] = {0.0, 0.0, 0.0, 0.0, 0.0, 0.0};
            double au_results[6] = {0.0, 0.0, 0.0, 0.0, 0.0, 0.0};
            double km_results[6] = {0.0, 0.0, 0.0, 0.0, 0.0, 0.0};
            double sidereal_results[6] = {0.0, 0.0, 0.0, 0.0, 0.0, 0.0};

            if (jme_calc_ut(2451545.0, JME_BODY_MERCURY, JME_CALC_TRUE_POSITION, deg_results, error) == JME_OK
                && jme_calc_ut(2451545.0, JME_BODY_MERCURY, JME_CALC_TRUE_POSITION | JME_CALC_RADIANS, rad_results, error) == JME_OK
                && fabs(rad_results[0] - deg_results[0] * 3.14159265358979323846 / 180.0) > 1e-12) {
                fprintf(stderr, "radian output flag mismatch\n");
                return 1;
            }

            if (jme_calc_ut(2451545.0, JME_BODY_MERCURY, JME_CALC_TRUE_POSITION, au_results, error) == JME_OK
                && jme_calc_ut(2451545.0, JME_BODY_MERCURY, JME_CALC_TRUE_POSITION | JME_CALC_DISTANCE_KM, km_results, error) == JME_OK
                && fabs(km_results[2] - au_results[2] * JME_AU_KM) > 1e-3) {
                fprintf(stderr, "kilometer output flag mismatch\n");
                return 1;
            }

            if (jme_calc_ut(2451545.0, JME_BODY_MERCURY, JME_CALC_TRUE_POSITION, deg_results, error) == JME_OK
                && jme_calc_ut(2451545.0, JME_BODY_MERCURY, JME_CALC_TRUE_POSITION | JME_CALC_SIDEREAL, sidereal_results, error) == JME_OK
                && fabs(jme_degrees_difference(sidereal_results[0], deg_results[0] - jme_get_ayanamsa_ut(2451545.0))) > 1e-9) {
                fprintf(stderr, "sidereal output flag mismatch\n");
                return 1;
            }
        }

        {
            double mean_node[6] = {0.0, 0.0, 0.0, 0.0, 0.0, 0.0};
            double true_node[6] = {0.0, 0.0, 0.0, 0.0, 0.0, 0.0};

            if (jme_calc_ut(2451545.0, JME_BODY_MEAN_NODE, JME_CALC_TRUE_POSITION, mean_node, error) != JME_OK
                || jme_calc_ut(2451545.0, JME_BODY_TRUE_NODE, JME_CALC_TRUE_POSITION, true_node, error) != JME_OK) {
                fprintf(stderr, "lunar node calculation failed: %s\n", error);
                return 1;
            }

            if (!finite_values(mean_node, 6)
                || !finite_values(true_node, 6)
                || mean_node[0] < 0.0
                || mean_node[0] >= 360.0) {
                fprintf(stderr, "lunar node output mismatch\n");
                return 1;
            }
        }
    }

    {
        double geopos[3] = {0.0, 51.5, 0.0};
        double rise_time = 0.0;
        double set_time = 0.0;
        double high_horizon_rise = 0.0;
        double anti_transit = 0.0;
        double civil_rise = 0.0;
        double no_refraction_rise = 0.0;
        double sector = 0.0;

        if (jme_rise_trans(2451545.0, JME_BODY_SUN, 0, JME_CALC_TRUE_POSITION, JME_RISE_RISE, geopos, 1010.0, 10.0, &rise_time, error) == JME_OK
            && (rise_time < 2451545.0 || rise_time > 2451546.0)) {
            fprintf(stderr, "rise time outside search day\n");
            return 1;
        }

        if (jme_rise_trans(2451545.0, JME_BODY_SUN, 0, JME_CALC_TRUE_POSITION, JME_RISE_SET, geopos, 1010.0, 10.0, &set_time, error) == JME_OK
            && (set_time < 2451545.0 || set_time > 2451546.0)) {
            fprintf(stderr, "set time outside search day\n");
            return 1;
        }

        if (rise_time != 0.0
            && jme_rise_trans_true_hor(2451545.0, JME_BODY_SUN, 0, JME_CALC_TRUE_POSITION, JME_RISE_RISE, geopos, 1010.0, 10.0, 5.0, &high_horizon_rise, error) == JME_OK
            && high_horizon_rise <= rise_time) {
            fprintf(stderr, "true horizon rise did not shift after standard rise\n");
            return 1;
        }

        if (jme_rise_trans(2451545.0, JME_BODY_SUN, 0, JME_CALC_TRUE_POSITION, JME_RISE_ANTI_MERIDIAN_TRANSIT, geopos, 1010.0, 10.0, &anti_transit, error) == JME_OK
            && (anti_transit < 2451545.0 || anti_transit > 2451546.0)) {
            fprintf(stderr, "anti-meridian transit outside search day\n");
            return 1;
        }

        if (rise_time != 0.0
            && jme_rise_trans(2451545.0, JME_BODY_SUN, 0, JME_CALC_TRUE_POSITION, JME_RISE_RISE | JME_RISE_CIVIL_TWILIGHT, geopos, 1010.0, 10.0, &civil_rise, error) == JME_OK
            && civil_rise >= rise_time) {
            fprintf(stderr, "civil twilight rise did not precede standard rise\n");
            return 1;
        }

        if (rise_time != 0.0
            && jme_rise_trans(2451545.0, JME_BODY_SUN, 0, JME_CALC_TRUE_POSITION, JME_RISE_RISE | JME_RISE_NO_REFRACTION | JME_RISE_DISC_CENTER, geopos, 1010.0, 10.0, &no_refraction_rise, error) == JME_OK
            && no_refraction_rise <= rise_time) {
            fprintf(stderr, "no-refraction disc-center rise did not shift after standard rise\n");
            return 1;
        }

        if (jme_gauquelin_sector(2451545.0, JME_BODY_SUN, 0, JME_CALC_TRUE_POSITION, 0, geopos, 1010.0, 10.0, &sector, error) == JME_OK
            && (sector < 1.0 || sector > 36.0 || !isfinite(sector))) {
            fprintf(stderr, "Gauquelin sector outside valid range\n");
            return 1;
        }
    }

    {
        double cross = 0.0;
        double node_cross = 0.0;
        double sun_at_cross[6] = {0.0};
        double moon_at_node[6] = {0.0};

        if (jme_solcross(280.0, 2451545.0, JME_CALC_TRUE_POSITION, &cross, error) == JME_OK) {
            if (jme_calc_ut(cross, JME_BODY_SUN, JME_CALC_TRUE_POSITION, sun_at_cross, error) != JME_OK
                || fabs(jme_degrees_difference_signed(sun_at_cross[0], 280.0)) > 1e-6) {
                fprintf(stderr, "solar longitude crossing refinement mismatch\n");
                return 1;
            }
        }

        if (jme_mooncross_node(2451545.0, JME_CALC_TRUE_POSITION, &node_cross, error) == JME_OK) {
            if (jme_calc_ut(node_cross, JME_BODY_MOON, JME_CALC_TRUE_POSITION, moon_at_node, error) != JME_OK
                || fabs(moon_at_node[1]) > 1e-6) {
                fprintf(stderr, "lunar node crossing refinement mismatch\n");
                return 1;
            }
        }
    }

    {
        double cusps_equal[13] = {0.0};
        double cusps_placidus[13] = {0.0};
        double cusps_porphyrius[13] = {0.0};
        double cusps_morinus[13] = {0.0};
        double cusps_regiomontanus[13] = {0.0};
        double cusps_campanus[13] = {0.0};
        double cusps_koch[13] = {0.0};
        double cusps_krusinski[13] = {0.0};
        double cusps_apc[13] = {0.0};
        double cusps_sunshine[13] = {0.0};
        double cusps_alcabitius[13] = {0.0};
        double cusps_topocentric[13] = {0.0};
        double cusps_horizontal[13] = {0.0};
        double cusps_speed[13] = {0.0};
        double ascmc[10] = {0.0};
        double ascmc_speed[10] = {0.0};

        if (jme_houses(2451545.0, 51.5, 0.0, JME_HOUSE_EQUAL, cusps_equal, ascmc) != JME_OK
            || !finite_values(cusps_equal + 1, 12)
            || !finite_values(ascmc, 10)
            || fabs(jme_degrees_difference(cusps_equal[1], ascmc[0])) > 1e-12) {
            fprintf(stderr, "numeric JME equal house contract mismatch\n");
            return 1;
        }

        if (jme_houses(2451545.0, 51.5, 0.0, JME_HOUSE_PLACIDUS, cusps_placidus, ascmc) != JME_OK
            || !finite_values(cusps_placidus + 1, 12)
            || fabs(jme_degrees_difference(cusps_placidus[1], ascmc[0])) > 1e-12
            || fabs(jme_degrees_difference(cusps_placidus[10], ascmc[1])) > 1e-12
            || fabs(jme_degrees_difference(cusps_placidus[7], cusps_placidus[1] + 180.0)) > 1e-12
            || fabs(jme_degrees_difference(cusps_placidus[4], cusps_placidus[10] + 180.0)) > 1e-12) {
            fprintf(stderr, "Placidus house contract mismatch\n");
            return 1;
        }

        if (jme_houses(2451545.0, 51.5, 0.0, JME_HOUSE_PORPHYRIUS, cusps_porphyrius, ascmc) != JME_OK
            || !finite_values(cusps_porphyrius + 1, 12)) {
            fprintf(stderr, "Porphyrius house contract mismatch\n");
            return 1;
        }

        if (jme_houses(2451545.0, 51.5, 0.0, JME_HOUSE_MORINUS, cusps_morinus, ascmc) != JME_OK
            || !finite_values(cusps_morinus + 1, 12)
            || fabs(jme_degrees_difference(cusps_morinus[10], ascmc[1])) > 1e-12) {
            fprintf(stderr, "Morinus house contract mismatch\n");
            return 1;
        }

        if (jme_houses(2451545.0, 51.5, 0.0, JME_HOUSE_REGIOMONTANUS, cusps_regiomontanus, ascmc) != JME_OK
            || !finite_values(cusps_regiomontanus + 1, 12)
            || fabs(jme_degrees_difference(cusps_regiomontanus[1], ascmc[0])) > 1e-12
            || fabs(jme_degrees_difference(cusps_regiomontanus[10], ascmc[1])) > 1e-12
            || fabs(jme_degrees_difference(cusps_regiomontanus[7], cusps_regiomontanus[1] + 180.0)) > 1e-12
            || fabs(jme_degrees_difference(cusps_regiomontanus[4], cusps_regiomontanus[10] + 180.0)) > 1e-12) {
            fprintf(stderr, "Regiomontanus house contract mismatch\n");
            return 1;
        }

        if (jme_houses(2451545.0, 51.5, 0.0, JME_HOUSE_CAMPANUS, cusps_campanus, ascmc) != JME_OK
            || !finite_values(cusps_campanus + 1, 12)
            || fabs(jme_degrees_difference(cusps_campanus[1], ascmc[0])) > 1e-12
            || fabs(jme_degrees_difference(cusps_campanus[10], ascmc[1])) > 1e-12
            || fabs(jme_degrees_difference(cusps_campanus[7], cusps_campanus[1] + 180.0)) > 1e-12
            || fabs(jme_degrees_difference(cusps_campanus[4], cusps_campanus[10] + 180.0)) > 1e-12) {
            fprintf(stderr, "Campanus house contract mismatch\n");
            return 1;
        }

        if (jme_houses(2451545.0, 51.5, 0.0, JME_HOUSE_KOCH, cusps_koch, ascmc) != JME_OK
            || !finite_values(cusps_koch + 1, 12)
            || fabs(jme_degrees_difference(cusps_koch[1], ascmc[0])) > 1e-12
            || fabs(jme_degrees_difference(cusps_koch[10], ascmc[1])) > 1e-12
            || fabs(jme_degrees_difference(cusps_koch[7], cusps_koch[1] + 180.0)) > 1e-12
            || fabs(jme_degrees_difference(cusps_koch[4], cusps_koch[10] + 180.0)) > 1e-12) {
            fprintf(stderr, "Koch house contract mismatch\n");
            return 1;
        }

        if (jme_houses(2451545.0, 51.5, 0.0, JME_HOUSE_KRUSINSKI, cusps_krusinski, ascmc) != JME_OK
            || !finite_values(cusps_krusinski + 1, 12)
            || fabs(jme_degrees_difference(cusps_krusinski[1], ascmc[0])) > 1e-12
            || fabs(jme_degrees_difference(cusps_krusinski[10], ascmc[1])) > 1e-12
            || fabs(jme_degrees_difference(cusps_krusinski[7], cusps_krusinski[1] + 180.0)) > 1e-12
            || fabs(jme_degrees_difference(cusps_krusinski[4], cusps_krusinski[10] + 180.0)) > 1e-12) {
            fprintf(stderr, "Krusinski house contract mismatch\n");
            return 1;
        }

        if (jme_houses(2451545.0, 51.5, 0.0, JME_HOUSE_APC, cusps_apc, ascmc) != JME_OK
            || !finite_values(cusps_apc + 1, 12)
            || fabs(jme_degrees_difference(cusps_apc[1], ascmc[0])) > 1e-12
            || fabs(jme_degrees_difference(cusps_apc[10], ascmc[1])) > 1e-12
            || fabs(jme_degrees_difference(cusps_apc[7], cusps_apc[1] + 180.0)) > 1e-12
            || fabs(jme_degrees_difference(cusps_apc[4], cusps_apc[10] + 180.0)) > 1e-12) {
            fprintf(stderr, "APC house contract mismatch\n");
            return 1;
        }

        if (jme_houses(2451545.0, 51.5, 0.0, JME_HOUSE_SUNSHINE, cusps_sunshine, ascmc) != JME_OK
            || !finite_values(cusps_sunshine + 1, 12)
            || fabs(jme_degrees_difference(cusps_sunshine[1], ascmc[0])) > 1e-12
            || fabs(jme_degrees_difference(cusps_sunshine[10], ascmc[1])) > 1e-12
            || fabs(jme_degrees_difference(cusps_sunshine[7], cusps_sunshine[1] + 180.0)) > 1e-12
            || fabs(jme_degrees_difference(cusps_sunshine[4], cusps_sunshine[10] + 180.0)) > 1e-12) {
            fprintf(stderr, "Sunshine house contract mismatch\n");
            return 1;
        }

        if (jme_houses_armc(120.0, 51.5, 23.4392911, JME_HOUSE_SUNSHINE, cusps_sunshine, ascmc) != JME_ERR) {
            fprintf(stderr, "ARMC-only Sunshine unexpectedly succeeded without Sun declination\n");
            return 1;
        }

        if (jme_houses(2451545.0, 51.5, 0.0, JME_HOUSE_ALCABITIUS, cusps_alcabitius, ascmc) != JME_OK
            || !finite_values(cusps_alcabitius + 1, 12)
            || fabs(jme_degrees_difference(cusps_alcabitius[1], ascmc[0])) > 1e-12
            || fabs(jme_degrees_difference(cusps_alcabitius[10], ascmc[1])) > 1e-12
            || fabs(jme_degrees_difference(cusps_alcabitius[7], cusps_alcabitius[1] + 180.0)) > 1e-12
            || fabs(jme_degrees_difference(cusps_alcabitius[4], cusps_alcabitius[10] + 180.0)) > 1e-12) {
            fprintf(stderr, "Alcabitius house contract mismatch\n");
            return 1;
        }

        if (jme_houses(2451545.0, 51.5, 0.0, JME_HOUSE_POLICH_PAGE, cusps_topocentric, ascmc) != JME_OK
            || !finite_values(cusps_topocentric + 1, 12)
            || fabs(jme_degrees_difference(cusps_topocentric[1], ascmc[0])) > 1e-12
            || fabs(jme_degrees_difference(cusps_topocentric[10], ascmc[1])) > 1e-12
            || fabs(jme_degrees_difference(cusps_topocentric[7], cusps_topocentric[1] + 180.0)) > 1e-12
            || fabs(jme_degrees_difference(cusps_topocentric[4], cusps_topocentric[10] + 180.0)) > 1e-12) {
            fprintf(stderr, "Polich/Page house contract mismatch\n");
            return 1;
        }

        if (jme_houses(2451545.0, 51.5, 0.0, JME_HOUSE_MERIDIAN, cusps_morinus, ascmc) != JME_OK
            || !finite_values(cusps_morinus + 1, 12)
            || fabs(jme_degrees_difference(cusps_morinus[10], ascmc[1])) > 1e-12) {
            fprintf(stderr, "Meridian house contract mismatch\n");
            return 1;
        }

        if (jme_houses(2451545.0, 51.5, 0.0, JME_HOUSE_HORIZONTAL, cusps_horizontal, ascmc) != JME_OK
            || !finite_values(cusps_horizontal + 1, 12)
            || fabs(jme_degrees_difference(cusps_horizontal[1], ascmc[0])) > 1e-12
            || fabs(jme_degrees_difference(cusps_horizontal[10], ascmc[1])) > 1e-12
            || fabs(jme_degrees_difference(cusps_horizontal[7], cusps_horizontal[1] + 180.0)) > 1e-12) {
            fprintf(stderr, "Horizontal house contract mismatch\n");
            return 1;
        }

        if (jme_houses(2451545.0, 51.5, 0.0, JME_HOUSE_AZIMUTHAL, cusps_horizontal, ascmc) != JME_OK
            || !finite_values(cusps_horizontal + 1, 12)
            || fabs(jme_degrees_difference(cusps_horizontal[1], ascmc[0])) > 1e-12
            || fabs(jme_degrees_difference(cusps_horizontal[10], ascmc[1])) > 1e-12) {
            fprintf(stderr, "Azimuthal house contract mismatch\n");
            return 1;
        }

        if (jme_houses(2451545.0, 51.5, 0.0, 999999, cusps_equal, ascmc) != JME_ERR) {
            fprintf(stderr, "unknown house system unexpectedly succeeded\n");
            return 1;
        }

        if (jme_houses_ex2(2451545.0, JME_CALC_NONE, 51.5, 0.0, JME_HOUSE_PLACIDUS, cusps_placidus, ascmc, cusps_speed, ascmc_speed) != JME_OK
            || !finite_values(cusps_speed + 1, 12)
            || !finite_values(ascmc_speed, 10)
            || fabs(cusps_speed[1]) + fabs(cusps_speed[10]) <= 0.0
            || fabs(ascmc_speed[0]) + fabs(ascmc_speed[1]) <= 0.0) {
            fprintf(stderr, "house speed contract mismatch\n");
            return 1;
        }

        if (jme_houses_armc_ex2(120.0, 51.5, 23.4392911, JME_HOUSE_EQUAL, cusps_equal, ascmc, cusps_speed, ascmc_speed) != JME_OK
            || !finite_values(cusps_speed + 1, 12)
            || !finite_values(ascmc_speed, 10)
            || fabs(cusps_speed[1]) + fabs(ascmc_speed[0]) <= 0.0) {
            fprintf(stderr, "ARMC house speed contract mismatch\n");
            return 1;
        }
    }

    {
        double geopos[3] = {-100.0, 40.0, 0.0};
        double tret[10] = {0.0};
        double attr[20] = {0.0};
        double solar_start = jme_julian_day(2024, 4, 7, 0.0, JME_CALENDAR_GREGORIAN);
        double solar_total_max = jme_julian_day(2024, 4, 8, 18.0 + 18.0 / 60.0 + 29.0 / 3600.0, JME_CALENDAR_GREGORIAN);
        double dallas_total_max = jme_julian_day(2024, 4, 8, 18.0 + 42.0 / 60.0 + 38.0 / 3600.0, JME_CALENDAR_GREGORIAN);
        double albuquerque_geopos[3] = {-106.6504, 35.0844, 0.0};
        double annular_start = jme_julian_day(2023, 10, 14, 0.0, JME_CALENDAR_GREGORIAN);
        double annular_global_max = jme_julian_day(2023, 10, 14, 18.0 + 0.0 / 60.0 + 41.0 / 3600.0, JME_CALENDAR_GREGORIAN);
        double albuquerque_annular_max = jme_julian_day(2023, 10, 14, 16.0 + 37.0 / 60.0, JME_CALENDAR_GREGORIAN);
        double hybrid_start = jme_julian_day(2023, 4, 20, 0.0, JME_CALENDAR_GREGORIAN);
        double hybrid_global_max = jme_julian_day(2023, 4, 20, 4.0 + 17.0 / 60.0 + 56.0 / 3600.0, JME_CALENDAR_GREGORIAN);
        double lunar_total_start = jme_julian_day(2022, 5, 15, 0.0, JME_CALENDAR_GREGORIAN);
        double lunar_total_max = jme_julian_day(2022, 5, 16, 4.0 + 11.0 / 60.0 + 28.0 / 3600.0, JME_CALENDAR_GREGORIAN);
        double lunar_penumbral_start = jme_julian_day(2024, 3, 25, 0.0, JME_CALENDAR_GREGORIAN);
        double lunar_penumbral_max = jme_julian_day(2024, 3, 25, 7.0 + 12.0 / 60.0 + 49.0 / 3600.0, JME_CALENDAR_GREGORIAN);
        int solar_rc = 0;

        if ((jme_sol_eclipse_when_glob(solar_start, JME_CALC_TRUE_POSITION, 0, tret, 0, error) & JME_ECLIPSE_SOLAR_TOTAL) == 0
            || fabs(tret[0] - solar_total_max) > (20.0 / 1440.0)) {
            fprintf(stderr, "solar total eclipse global search mismatch: %s %.17g\n", error, tret[0]);
            return 1;
        }

        if ((jme_sol_eclipse_where(tret[0], JME_CALC_TRUE_POSITION, geopos, attr, error) & JME_ECLIPSE_SOLAR_TOTAL) == 0
            || !finite_values(attr, 5)) {
            fprintf(stderr, "solar total eclipse where mismatch: %s\n", error);
            return 1;
        }

        solar_rc = jme_sol_eclipse_when_loc(solar_start, JME_CALC_TRUE_POSITION, geopos, tret, attr, 0, error);
        if (solar_rc == JME_ERR
            || (attr[0] <= 0.0)
            || (attr[3] <= 0.0)
            || (attr[4] <= 0.0)
            || (attr[5] <= 0.0)
            || ((solar_rc & JME_ECLIPSE_SOLAR_PARTIAL) == 0)
            || tret[0] < solar_start
            || tret[0] > solar_start + 2.0) {
            fprintf(stderr, "solar Dallas eclipse local mismatch: %s %.17g\n", error, tret[0]);
            return 1;
        }

        solar_rc = jme_sol_eclipse_how(tret[0], JME_CALC_TRUE_POSITION, geopos, attr, error);
        if (solar_rc == JME_ERR
            || (solar_rc & JME_ECLIPSE_SOLAR_PARTIAL) == 0
            || attr[0] <= 0.0
            || attr[3] <= 0.0
            || attr[4] <= 0.0) {
            fprintf(stderr, "solar Dallas eclipse circumstance mismatch: %s\n", error);
            return 1;
        }

        if ((jme_sol_eclipse_when_glob(annular_start, JME_CALC_TRUE_POSITION, 0, tret, 0, error) & JME_ECLIPSE_SOLAR_ANNULAR) == 0
            || fabs(tret[0] - annular_global_max) > (25.0 / 1440.0)) {
            fprintf(stderr, "solar annular eclipse global mismatch: %s %.17g\n", error, tret[0]);
            return 1;
        }

        solar_rc = jme_sol_eclipse_when_glob(hybrid_start, JME_CALC_TRUE_POSITION, 0, tret, 0, error);
        if (solar_rc != JME_ECLIPSE_SOLAR_HYBRID
            || fabs(tret[0] - hybrid_global_max) > (30.0 / 1440.0)) {
            fprintf(stderr, "solar hybrid eclipse global mismatch: %s %.17g %d\n", error, tret[0], solar_rc);
            return 1;
        }

        if (jme_sol_eclipse_where(tret[0], JME_CALC_TRUE_POSITION, geopos, attr, error) != JME_ECLIPSE_SOLAR_HYBRID
            || attr[8] != JME_ECLIPSE_SOLAR_CENTRAL) {
            fprintf(stderr, "solar hybrid eclipse where mismatch: %s %.17g %.17g\n", error, attr[7], attr[8]);
            return 1;
        }

        solar_rc = jme_sol_eclipse_when_loc(annular_start, JME_CALC_TRUE_POSITION, albuquerque_geopos, tret, attr, 0, error);
        if (solar_rc == JME_ERR
            || (attr[0] <= 0.0)
            || (attr[3] <= 0.0)
            || (attr[4] <= 0.0)
            || ((solar_rc & JME_ECLIPSE_SOLAR_PARTIAL) == 0)
            || attr[5] <= 0.0
            || tret[0] < annular_start
            || tret[0] > annular_start + 2.0) {
            fprintf(stderr, "solar Albuquerque eclipse local mismatch: %s %.17g\n", error, tret[0]);
            return 1;
        }

        if ((jme_lun_eclipse_when(lunar_total_start, JME_CALC_TRUE_POSITION, 0, tret, 0, error) & JME_ECLIPSE_LUNAR_TOTAL) == 0
            || fabs(tret[0] - lunar_total_max) > (20.0 / 1440.0)) {
            fprintf(stderr, "lunar total eclipse search mismatch: %s %.17g\n", error, tret[0]);
            return 1;
        }

        if ((jme_lun_eclipse_how(tret[0], JME_CALC_TRUE_POSITION, geopos, attr, error) & JME_ECLIPSE_LUNAR_TOTAL) == 0
            || attr[0] < 1.0
            || attr[1] <= 0.0) {
            fprintf(stderr, "lunar total eclipse circumstance mismatch: %s\n", error);
            return 1;
        }

        if ((jme_lun_eclipse_when(lunar_penumbral_start, JME_CALC_TRUE_POSITION, 0, tret, 0, error) & JME_ECLIPSE_LUNAR_PENUMBRAL) == 0
            || fabs(tret[0] - lunar_penumbral_max) > (30.0 / 1440.0)) {
            fprintf(stderr, "lunar penumbral eclipse search mismatch: %s %.17g\n", error, tret[0]);
            return 1;
        }

        if ((jme_lun_eclipse_how(tret[0], JME_CALC_TRUE_POSITION, geopos, attr, error) & JME_ECLIPSE_LUNAR_PENUMBRAL) == 0
            || attr[1] <= 0.0
            || attr[0] >= 0.0) {
            fprintf(stderr, "lunar penumbral eclipse circumstance mismatch: %s\n", error);
            return 1;
        }

        if ((jme_lun_eclipse_when_loc(lunar_total_start, JME_CALC_TRUE_POSITION, geopos, tret, attr, 0, error) & JME_ECLIPSE_LUNAR_TOTAL) == 0) {
            fprintf(stderr, "lunar local eclipse search mismatch: %s\n", error);
            return 1;
        }

        if (jme_lun_occult_when_loc(lunar_penumbral_start, JME_BODY_MARS, 0, JME_CALC_TRUE_POSITION, geopos, tret, attr, 0, error) != JME_ERR
            || strstr(error, "Exact lunar occultation") == 0) {
            fprintf(stderr, "lunar occultation error contract mismatch: %s\n", error);
            return 1;
        }
    }

    {
        double elem[20] = {0.0};
        int rc = jme_get_orbital_elements(2451545.0, JME_BODY_MERCURY, JME_CALC_TRUE_POSITION, elem, error);

        if (rc == JME_OK) {
            if (!finite_values(elem, 9)
                || elem[0] <= 0.0
                || elem[1] < 0.0
                || elem[2] < 0.0
                || elem[2] > 180.0
                || elem[3] < 0.0
                || elem[3] >= 360.0
                || elem[4] < 0.0
                || elem[4] >= 360.0
                || elem[5] < 0.0
                || elem[5] >= 360.0
                || !isfinite(elem[9])
                || elem[9] < 0.0
                || elem[9] >= 360.0
                || !isfinite(elem[10])
                || elem[10] < 0.0
                || elem[10] >= 360.0
                || !isfinite(elem[11])
                || elem[11] <= 0.0
                || !isfinite(elem[12])
                || elem[12] <= 0.0
                || !isfinite(elem[13])
                || !isfinite(elem[14])
                || elem[13] <= 0.0
                || elem[14] <= elem[13]
                || fabs(elem[13] - elem[0] * (1.0 - elem[1])) > 1e-12
                || fabs(elem[14] - elem[0] * (1.0 + elem[1])) > 1e-12
                || fabs(elem[15] - jme_degree_normalize(elem[4] + elem[5])) > 1e-12
                || fabs(elem[16] - jme_degree_normalize(elem[3] + elem[4] + elem[5])) > 1e-12
                || fabs(elem[17] - jme_degree_normalize(elem[3] + elem[4])) > 1e-12) {
                fprintf(stderr, "orbital elements contract mismatch\n");
                return 1;
            }
        } else if (strstr(error, "velocity") == 0 && strstr(error, "Calculation") == 0) {
            fprintf(stderr, "orbital elements failed with unexpected error: %s\n", error);
            return 1;
        }
    }

    {
        double attr[20] = {0.0};
        if (jme_pheno(2451545.0, JME_BODY_MERCURY, JME_CALC_TRUE_POSITION, attr, error) != JME_OK
            || !finite_values(attr, 5)
            || attr[0] < 0.0
            || attr[0] > 180.0
            || attr[1] < 0.0
            || attr[1] > 1.0
            || attr[2] < 0.0
            || attr[2] > 180.0
            || attr[3] <= 0.0
            || attr[4] < -30.0
            || attr[4] > 30.0
            || attr[5] <= 0.0
            || attr[6] <= 0.0) {
            fprintf(stderr, "physical phenomena contract mismatch: %s\n", error);
            return 1;
        }
    }

    {
        double node[4] = {0.0};
        double apogee[4] = {0.0};
        double mercury_node[4] = {0.0};
        double mercury_aphelion[4] = {0.0};
        double mercury_perihelion[4] = {0.0};

        if (jme_nod_aps(2451545.0, JME_BODY_MOON, JME_CALC_TRUE_POSITION, 1, node, error) != JME_OK
            || !finite_values(node, 4)
            || node[0] < 0.0
            || node[0] >= 360.0
            || jme_nod_aps(2451545.0, JME_BODY_MOON, JME_CALC_TRUE_POSITION, 4, apogee, error) != JME_OK
            || !finite_values(apogee, 4)
            || apogee[0] < 0.0
            || apogee[0] >= 360.0) {
            fprintf(stderr, "lunar node/apside contract mismatch: %s\n", error);
            return 1;
        }

        if (jme_nod_aps(2451545.0, JME_BODY_MERCURY, JME_CALC_TRUE_POSITION, 1, mercury_node, error) != JME_OK
            || jme_nod_aps(2451545.0, JME_BODY_MERCURY, JME_CALC_TRUE_POSITION, 4, mercury_aphelion, error) != JME_OK
            || jme_nod_aps(2451545.0, JME_BODY_MERCURY, JME_CALC_TRUE_POSITION, 256, mercury_perihelion, error) != JME_OK
            || !finite_values(mercury_node, 4)
            || !finite_values(mercury_aphelion, 4)
            || !finite_values(mercury_perihelion, 4)
            || mercury_node[0] < 0.0
            || mercury_node[0] >= 360.0
            || mercury_node[1] <= 0.0
            || mercury_aphelion[1] <= mercury_perihelion[1]
            || fabs(jme_degrees_difference(mercury_aphelion[0], mercury_perihelion[0] + 180.0)) > 1e-9) {
            fprintf(stderr, "non-lunar node/apside contract mismatch: %s\n", error);
            return 1;
        }

        if (jme_nod_aps(2451545.0, JME_BODY_SUN, JME_CALC_TRUE_POSITION, 1, node, error) != JME_ERR) {
            fprintf(stderr, "node/apside unexpectedly accepted unsupported body\n");
            return 1;
        }
    }

    {
        double tmax = 0.0;
        double tmin = 0.0;
        double dmax = 0.0;
        double dmin = 0.0;

        if (jme_orbit_max_min_true_distance(2451545.0, JME_BODY_MERCURY, JME_CALC_TRUE_POSITION, &tmax, &tmin, &dmax, &dmin, error) != JME_OK
            || !isfinite(tmax)
            || !isfinite(tmin)
            || !isfinite(dmax)
            || !isfinite(dmin)
            || dmin <= 0.0
            || dmax <= dmin
            || fabs(dmin - 0.307499) > 0.02
            || fabs(dmax - 0.466699) > 0.02) {
            fprintf(stderr, "orbit max/min true distance contract mismatch: %s\n", error);
            return 1;
        }
    }

    {
        double geopos[3] = {0.0, 51.5, 0.0};
        double dat_hel[20] = {0.0};
        double hangle = jme_heliacal_angle(2451545.0, geopos, dat_hel, error);
        double arcus = jme_topo_arcus_visionis(2451545.0, geopos, dat_hel, error);

        if (!isnan(hangle) || !isnan(arcus)) {
            fprintf(stderr, "heliacal scalar helpers returned numeric output without visibility model\n");
            return 1;
        }

        if (jme_heliacal_ut(2451545.0, geopos, dat_hel, error) != JME_ERR
            || strstr(error, "Heliacal visibility") == 0) {
            fprintf(stderr, "heliacal visibility error contract mismatch: %s\n", error);
            return 1;
        }

        if (jme_vis_limit_mag(2451545.0, geopos, dat_hel, error) != JME_ERR
            || strstr(error, "Limiting visual magnitude") == 0) {
            fprintf(stderr, "visual limit error contract mismatch: %s\n", error);
            return 1;
        }
    }

    if (jme_jpl_is_available() && getenv("JME_TEST_JPL_KERNEL") != 0) {
        double results[6];
        if (jme_jpl_open(getenv("JME_TEST_JPL_KERNEL"), error) != JME_OK) {
            fprintf(stderr, "JPL kernel open for calc contract failed: %s\n", error);
            return 1;
        }

        if (jme_calc(2451545.0, JME_BODY_SUN, JME_CALC_XYZ | JME_CALC_J2000, results, error) == JME_OK) {
            if (fabs(jme_state_distance(results) - 0.98328) > 0.01) {
                fprintf(stderr, "Sun distance at J2000 mismatch: %.17g\n", jme_state_distance(results));
                return 1;
            }
        }

        jme_jpl_close();
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
