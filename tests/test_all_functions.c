#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include <math.h>
#include <jme/jme.h>

#define TEST_ASSERT(cond) \
    do { \
        if (!(cond)) { \
            fprintf(stderr, "Assertion failed: %s at %s:%d\n", #cond, __FILE__, __LINE__); \
            exit(1); \
        } \
    } while (0)

void test_metadata_and_paths() {
    printf("Testing metadata and paths...\n");
    char buf[256];
    TEST_ASSERT(jme_version(buf, sizeof(buf)) != NULL);
    jme_close();
    
    // Invalid file inputs must return JME_ERR.
    TEST_ASSERT(jme_jpl_open("/non/existent/path", buf) == JME_ERR);
    const char *paths[] = {"/invalid1", "/invalid2"};
    TEST_ASSERT(jme_jpl_open_array(2, paths, buf) == JME_ERR);

    // Getters for paths
    jme_ephemeris_path();
    jme_jpl_file();
    jme_library_path();
    
    // Setters
    jme_set_ephemeris_path("./data");
    jme_set_jpl_file("./data/jpl/de440s.bsp");
}

void test_utility_math() {
    printf("Testing utility math functions...\n");
    // Normalization
    TEST_ASSERT(jme_degree_normalize(370.0) == 10.0);
    TEST_ASSERT(jme_degree_normalize(-10.0) == 350.0);
    TEST_ASSERT(jme_radian_normalize(6.283185307179586 * 1.5) > 3.0 && jme_radian_normalize(6.283185307179586 * 1.5) < 3.2);
    TEST_ASSERT(jme_hours_normalize(25.0) == 1.0);
    
    // Conversion
    TEST_ASSERT(jme_degrees_to_radians(180.0) > 3.14 && jme_degrees_to_radians(180.0) < 3.15);
    TEST_ASSERT(jme_radians_to_degrees(3.141592653589793) == 180.0);
    TEST_ASSERT(jme_degrees_to_hours(15.0) == 1.0);
    TEST_ASSERT(jme_hours_to_degrees(1.0) == 15.0);
    
    // Difference
    TEST_ASSERT(jme_degrees_difference(370.0, 10.0) == 0.0);
    TEST_ASSERT(jme_degrees_difference_signed(10.0, 350.0) == 20.0);
    double rad_diff = jme_radians_difference_signed(0.1, 6.2);
    TEST_ASSERT(rad_diff > 0.1 && rad_diff < 0.2);
    
    // Midpoint
    TEST_ASSERT(jme_degree_midpoint(10.0, 350.0) == 0.0);
    double rad_mid = jme_radian_midpoint(0.1, 6.2);
    TEST_ASSERT(rad_mid > 6.2 || rad_mid < 0.1);
    
    // Other
    TEST_ASSERT(jme_double_to_long(123.45) == 123);
    
    // Spherical
    TEST_ASSERT(jme_spherical_angular_separation(0, 0, 90, 0) == 90.0);
    TEST_ASSERT(jme_spherical_position_angle(0, 0, 0, 10) == 0.0);
    
    double rect[6] = {1, 0, 0, 0, 1, 0};
    double sph[6];
    jme_spherical_to_rectangular_state(rect, rect); // Just call it
    jme_rectangular_to_spherical_state(rect, sph);
    
    // Centiseconds
    TEST_ASSERT(jme_centiseconds_normalize(129600000 + 100) == 100);
    TEST_ASSERT(jme_centiseconds_difference(200, 100) == 100);
    TEST_ASSERT(jme_centiseconds_difference_signed(100, 200) == -100);
    TEST_ASSERT(jme_centiseconds_round_second(150) == 200);
    
    char buf[64];
    jme_centiseconds_to_time_string(12345, buf);
    jme_centiseconds_to_lonlat_string(12345, buf);
    jme_centiseconds_to_degree_string(12345, buf);
}

void test_time_and_calendar() {
    printf("Testing time and calendar functions...\n");
    TEST_ASSERT(jme_julian_day(2000, 1, 1, 12.0, JME_CALENDAR_GREGORIAN) == 2451545.0);
    TEST_ASSERT(jme_calendar_is_leap_year(2000, JME_CALENDAR_GREGORIAN) == 1);
    TEST_ASSERT(jme_calendar_is_leap_year(1900, JME_CALENDAR_GREGORIAN) == 0);
    TEST_ASSERT(jme_date_is_valid(2000, 2, 29, JME_CALENDAR_GREGORIAN) == 1);
    TEST_ASSERT(jme_days_in_month(2000, 2, JME_CALENDAR_GREGORIAN) == 29);
    TEST_ASSERT(jme_day_of_year(2000, 12, 31, JME_CALENDAR_GREGORIAN) == 366);
    TEST_ASSERT(jme_decimal_hour(12, 30, 0) == 12.5);
    TEST_ASSERT(jme_day_of_week(2451545.0) == 6); // 2000-01-01 was Saturday
    
    int y, m, d, h, min;
    double sec;
    jme_reverse_julian_day(2451545.0, JME_CALENDAR_GREGORIAN, &y, &m, &d, &sec);
    jme_jd_to_utc(2451545.0, JME_CALENDAR_GREGORIAN, &y, &m, &d, &h, &min, &sec);
    
    TEST_ASSERT(jme_jd_add_seconds(2451545.0, 86400.0) == 2451546.0);
    TEST_ASSERT(jme_jd_difference_seconds(2451546.0, 2451545.0) == 86400.0);
    
    jme_utc_time_zone(2000, 1, 1, 12, 0, 0, 5.5, &y, &m, &d, &h, &min, &sec);
    double jd;
    jme_utc_to_jd(2000, 1, 1, 12, 0, 0, JME_CALENDAR_GREGORIAN, &jd);
    
    jme_delta_t(2451545.0);
    char err[256];
    jme_delta_t_ex(2451545.0, JME_MODEL_DELTAT_ESPENAK_MEEUS_2006, err);
}

void test_coordinate_transforms() {
    printf("Testing coordinate transformations...\n");
    double ra, dec, lon, lat, eps = 23.4392911 * 3.141592653589793 / 180.0;
    jme_ecliptic_to_equatorial(0, 0, eps, &ra, &dec);
    jme_equatorial_to_ecliptic(ra, dec, eps, &lon, &lat);
    
    double ecl[6] = {1, 0, 0, 0, 0, 0}, equ[6];
    jme_ecliptic_to_equatorial_rectangular_state(ecl, eps, equ);
    jme_equatorial_to_ecliptic_rectangular_state(equ, eps, ecl);
    
    double az, alt, ha;
    jme_equatorial_to_horizontal(0, 0, 45, &az, &alt);
    jme_horizontal_to_equatorial(az, alt, 45, &ha, &dec);
    
    jme_refract(10.0, 1013.25, 15.0, 0);
    double out_ref[4];
    jme_refract_extended(10.0, 0, 1013.25, 15.0, 0.0065, 0, out_ref);
    
    int ideg, imin, isec, isgn;
    double dsecfr;
    jme_split_degree(12.3456, 0, &ideg, &imin, &isec, &dsecfr, &isgn);
}

void test_matrix_ops() {
    printf("Testing matrix operations...\n");
    double m1[9], m2[9], m3[9];
    jme_matrix_identity(m1);
    jme_matrix_rotate_x(0.1, m2);
    jme_matrix_rotate_y(0.2, m2);
    jme_matrix_rotate_z(0.3, m2);
    jme_matrix_multiply(m1, m2, m3);
    
    double state_in[6] = {1, 2, 3, 4, 5, 6}, state_out[6];
    jme_matrix_transform_state(m3, state_in, state_out);
    
    jme_get_nutation_matrix(0.01, 0.02, 0.4, m1);
}

void test_body_info() {
    printf("Testing body info functions...\n");
    TEST_ASSERT(jme_body_id_from_name("Sun") == JME_BODY_SUN);
    TEST_ASSERT(strcmp(jme_body_name(JME_BODY_SUN), "Sun") == 0);
    char buf[64];
    jme_copy_body_name(JME_BODY_SUN, buf);
    TEST_ASSERT(jme_body_naif_id(JME_BODY_SUN) == 10);
}

void test_state_ops() {
    printf("Testing state operations...\n");
    double s1[6] = {3, 4, 0, 0, 0, 0};
    TEST_ASSERT(jme_state_distance(s1) == 5.0);
    double s2[6] = {0, 0, 0, 6, 8, 0};
    TEST_ASSERT(jme_state_speed(s2) == 10.0);
    
    double out[6];
    jme_state_add(s1, s2, out);
    jme_state_subtract(s1, s2, out);
    jme_state_scale(s1, 2.0, out);
    jme_state_convert_units(s1, JME_VECTOR_AU_PER_DAY, JME_VECTOR_KM_PER_SECOND, out);
    jme_state_light_time_days(s1, JME_VECTOR_AU_PER_DAY);
    jme_state_position_velocity_dot(s1);
}

void test_ayanamsa_and_sidereal() {
    printf("Testing ayanamsa and sidereal functions...\n");
    jme_set_sidereal_mode(JME_SIDEREAL_LAHIRI, 0, 0);
    int mode;
    double t0, ayan_t0;
    jme_get_sidereal_mode(&mode, &t0, &ayan_t0);
    
    jme_get_ayanamsa(2451545.0);
    jme_get_ayanamsa_ut(2451545.0);
    double ayan;
    char err[256];
    jme_get_ayanamsa_ex(2451545.0, JME_SIDEREAL_LAHIRI, &ayan, err);
    jme_get_ayanamsa_ex_ut(2451545.0, JME_SIDEREAL_LAHIRI, &ayan, err);
    jme_get_ayanamsa_name(JME_SIDEREAL_LAHIRI);
    
    jme_sidereal_time(2451545.0);
    jme_sidereal_time0(2451545.0, 0.4, 0.001);
}

void test_astro_models_and_params() {
    printf("Testing astro models and parameters...\n");
    char models[256];
    jme_set_astro_models("IAU2006", 0);
    TEST_ASSERT(jme_get_astro_models(models, 0) == JME_OK);
    TEST_ASSERT(strstr(models, "OBL=338") != NULL);
    TEST_ASSERT(strstr(models, "PREC=341") != NULL);
    
    jme_set_lapse_rate(0.0065);
    jme_set_interpolate_nut(1);
    jme_set_tid_acc(-25.8);
    TEST_ASSERT(fabs(jme_get_tid_acc() - -25.8) < 1e-12);
    jme_set_delta_t_userdef(64.0);
    TEST_ASSERT(fabs(jme_delta_t(2451545.0) - 64.0) < 1e-12);
    jme_set_topo(0, 0, 0);
}

void test_ephemeris_calls() {
    printf("Testing ephemeris and catalog functions...\n");
    double res[20];
    char err[256];
    jme_calc(2451545.0, JME_BODY_SUN, JME_CALC_EQUATORIAL, res, err);
    jme_calc_ut(2451545.0, JME_BODY_SUN, JME_CALC_EQUATORIAL, res, err);
    TEST_ASSERT(jme_calc_pctr(2451545.0, JME_BODY_MARS, JME_BODY_SUN, JME_CALC_XYZ, res, err) == JME_OK);
    jme_pheno(2451545.0, JME_BODY_SUN, 0, res, err);
    jme_pheno_ut(2451545.0, JME_BODY_SUN, 0, res, err);
    
    jme_fixstar("Sirius", 2451545.0, 0, res, err);
    jme_fixstar_ut("Sirius", 2451545.0, 0, res, err);
    jme_fixstar_mag("Sirius", res, err);
    jme_fixstar2("Sirius", 2451545.0, 0, res, err);
    jme_fixstar2_ut("Sirius", 2451545.0, 0, res, err);
    jme_fixstar2_mag("Sirius", res, err);
    
    double cusps[13], ascmc[10];
    jme_houses(2451545.0, 0, 0, 'P', cusps, ascmc);
    jme_houses_ex(2451545.0, 0, 0, 0, 'P', cusps, ascmc);
    double cusps_speed[13], ascmc_speed[10];
    jme_houses_ex2(2451545.0, 0, 0, 0, 'P', cusps, ascmc, cusps_speed, ascmc_speed);
    TEST_ASSERT(jme_houses_armc(0, 0, 0.4, 'E', cusps, ascmc) == JME_OK);
    TEST_ASSERT(jme_houses_armc_ex2(0, 0, 0.4, 'E', cusps, ascmc, cusps_speed, ascmc_speed) == JME_OK);
    res[0] = cusps[1];
    res[1] = 0.0;
    TEST_ASSERT(jme_house_pos(0, 0, 0.4, 'E', res, err) >= 1.0);
    jme_house_system_name('P');
    
    double geopos[3] = {0, 0, 0};
    jme_gauquelin_sector(2451545.0, JME_BODY_SUN, NULL, 0, 0, geopos, 1013, 15, res, err);
}

void test_eclipses_and_phenomena() {
    printf("Testing event function contracts...\n");
    double geopos[3] = {0, 0, 0}, attr[20], tret[20];
    char err[256];
    
    jme_sol_eclipse_where(2451545.0, 0, geopos, attr, err);
    jme_sol_eclipse_how(2451545.0, 0, geopos, attr, err);
    jme_sol_eclipse_when_loc(2451545.0, 0, 0, tret, attr, 0, err);
    jme_sol_eclipse_when_glob(2451545.0, 0, 0, tret, 0, err);
    
    jme_lun_eclipse_how(2451545.0, 0, geopos, attr, err);
    jme_lun_eclipse_when_loc(2451545.0, 0, 0, tret, attr, 0, err);
    jme_lun_eclipse_when(2451545.0, 0, 0, tret, 0, err);
    
    jme_lun_occult_where(2451545.0, JME_BODY_SUN, NULL, 0, geopos, attr, err);
    jme_lun_occult_when_loc(2451545.0, JME_BODY_SUN, NULL, 0, 0, tret, attr, 0, err);
    jme_lun_occult_when_glob(2451545.0, JME_BODY_SUN, NULL, 0, 0, tret, 0, err);
    
    jme_rise_trans(2451545.0, JME_BODY_SUN, NULL, 0, 0, 0, 1013, 15, tret, err);
    jme_rise_trans_true_hor(2451545.0, JME_BODY_SUN, NULL, 0, 0, 0, 1013, 15, 0, tret, err);
    
    jme_solcross(0, 2451545.0, 0, 0, err);
    jme_solcross_ut(0, 2451545.0, 0, 0, err);
    jme_mooncross(0, 2451545.0, 0, 0, err);
    jme_mooncross_ut(0, 2451545.0, 0, 0, err);
    jme_mooncross_node(2451545.0, 0, 0, err);
    jme_mooncross_node_ut(2451545.0, 0, 0, err);
    jme_helio_cross(JME_BODY_MARS, 0, 2451545.0, 0, 0, err);
    jme_helio_cross_ut(JME_BODY_MARS, 0, 2451545.0, 0, 0, err);
    
    jme_nod_aps(2451545.0, JME_BODY_SUN, 0, 0, tret, err);
    jme_nod_aps_ut(2451545.0, JME_BODY_SUN, 0, 0, tret, err);
    jme_get_orbital_elements(2451545.0, JME_BODY_SUN, 0, attr, err);
    double tmax, tmin, dmax, dmin;
    jme_orbit_max_min_true_distance(2451545.0, JME_BODY_SUN, 0, &tmax, &tmin, &dmax, &dmin, err);
    
    jme_heliacal_ut(2451545.0, geopos, attr, err);
    jme_heliacal_pheno_ut(2451545.0, geopos, attr, err);
    jme_vis_limit_mag(2451545.0, geopos, attr, err);
    jme_heliacal_angle(2451545.0, geopos, attr, err);
    jme_topo_arcus_visionis(2451545.0, geopos, attr, err);
}

void test_jpl_calls() {
    printf("Testing jpl_* functions (may return JME_ERR)...\n");
    double state[12];
    char err[256];
    
    jme_jpl_body_state(2451545.0, JME_BODY_SUN, JME_BODY_EARTH, JME_VECTOR_KM_PER_SECOND, state, err);
    jme_jpl_body_state_split(2451545.0, 0, JME_BODY_SUN, JME_BODY_EARTH, JME_VECTOR_KM_PER_SECOND, state, err);
    jme_jpl_ecliptic_state(2451545.0, JME_BODY_SUN, JME_BODY_EARTH, JME_VECTOR_KM_PER_SECOND, state, err);
    jme_jpl_body_state_naif(2451545.0, 10, 399, JME_VECTOR_KM_PER_SECOND, state, err);
    jme_jpl_body_state_split_naif(2451545.0, 0, 10, 399, JME_VECTOR_KM_PER_SECOND, state, err);
    jme_jpl_body_state_native_naif(2451545.0, 10, 399, state, err);
    jme_jpl_body_state_native(2451545.0, JME_BODY_SUN, JME_BODY_EARTH, state, err);
    jme_jpl_body_state_native_split_naif(2451545.0, 0, 10, 399, state, err);
    jme_jpl_body_state_native_split(2451545.0, 0, JME_BODY_SUN, JME_BODY_EARTH, state, err);
    jme_jpl_body_state_order_naif(2451545.0, 0, 10, 399, JME_VECTOR_KM_PER_SECOND, 1, state, err);
    jme_jpl_body_state_order(2451545.0, 0, JME_BODY_SUN, JME_BODY_EARTH, JME_VECTOR_KM_PER_SECOND, 1, state, err);
    jme_jpl_body_state_utc_naif(2000, 1, 1, 12, 0, 0, 1, 10, 399, JME_VECTOR_KM_PER_SECOND, state, err);
    jme_jpl_body_state_utc(2000, 1, 1, 12, 0, 0, 1, JME_BODY_SUN, JME_BODY_EARTH, JME_VECTOR_KM_PER_SECOND, state, err);
    jme_jpl_ecliptic_state_naif(2451545.0, 10, 399, JME_VECTOR_KM_PER_SECOND, state, err);
    jme_jpl_ecliptic_state_split(2451545.0, 0, JME_BODY_SUN, JME_BODY_EARTH, JME_VECTOR_KM_PER_SECOND, state, err);
    jme_jpl_ecliptic_state_split_naif(2451545.0, 0, 10, 399, JME_VECTOR_KM_PER_SECOND, state, err);
    jme_jpl_ecliptic_state_utc_naif(2000, 1, 1, 12, 0, 0, 1, 10, 399, JME_VECTOR_KM_PER_SECOND, state, err);
    jme_jpl_ecliptic_state_utc(2000, 1, 1, 12, 0, 0, 1, JME_BODY_SUN, JME_BODY_EARTH, JME_VECTOR_KM_PER_SECOND, state, err);
    
    double t1, t2;
    int cont;
    jme_jpl_coverage(&t1, &t2, &cont, err);
    jme_jpl_constant("AU", &t1, err);
    jme_jpl_constant_count(err);
    char name[64];
    jme_jpl_constant_index(0, name, sizeof(name), &t1, err);
    jme_jpl_constant_vector("GROUP", state, 3, err);
    jme_jpl_constant_string("NAME", name, sizeof(name), err);
    jme_jpl_constant_string_vector("NAMES", name, sizeof(name), 1, err);
    
    jme_jpl_engine_version(name, sizeof(name));
    jme_jpl_file_version(name, sizeof(name), err);
    jme_jpl_current_file_data(name, sizeof(name), &t1, &t2, &cont, err);
    
    jme_jpl_is_available();
    jme_jpl_is_open();
    jme_jpl_prefetch(err);
    jme_jpl_is_thread_safe(err);
    
    int id;
    jme_jpl_id_by_name("EARTH", &id, err);
    jme_jpl_name_by_id(399, name, sizeof(name), err);
    jme_jpl_max_supported_order(1);
    
    jme_jpl_position_record_count(err);
    int target, center, frame, seg;
    jme_jpl_position_record_index(0, &target, &center, &t1, &t2, &frame, &seg, err);
    
    jme_jpl_orientation_state_naif(2451545.0, 399, JME_ORIENTATION_RAD_PER_SECOND, state, err);
    jme_jpl_orientation_state_split_naif(2451545.0, 0, 399, JME_ORIENTATION_RAD_PER_SECOND, state, err);
    jme_jpl_orientation_state_order_naif(2451545.0, 0, 399, JME_ORIENTATION_RAD_PER_SECOND, 1, state, err);
    jme_jpl_orientation_state_utc_naif(2000, 1, 1, 12, 0, 0, 1, 399, JME_ORIENTATION_RAD_PER_SECOND, state, err);
    
    jme_jpl_rotational_angular_momentum_state_naif(2451545.0, 399, JME_ORIENTATION_RAD_PER_SECOND, state, err);
    jme_jpl_rotational_angular_momentum_state_utc_naif(2000, 1, 1, 12, 0, 0, 1, 399, JME_ORIENTATION_RAD_PER_SECOND, state, err);
    jme_jpl_rotational_angular_momentum_state_split_naif(2451545.0, 0, 399, JME_ORIENTATION_RAD_PER_SECOND, state, err);
    jme_jpl_rotational_angular_momentum_state_order_naif(2451545.0, 0, 399, JME_ORIENTATION_RAD_PER_SECOND, 1, state, err);
    
    jme_jpl_orientation_record_count(err);
    jme_jpl_orientation_record_index(0, &target, &t1, &t2, &frame, &seg, err);
    jme_jpl_timescale();
    
    jme_jpl_close();
}

void test_extended_functions() {
    printf("Testing extended functions...\n");
    double d1, d2;
    char err[256];
    jme_get_obliquity(2451545.0, JME_MODEL_OBL_IAU_2006, &d1, err);
    jme_get_nutation(2451545.0, JME_MODEL_NUT_IAU_2000A, &d1, &d2, err);
    double m[9];
    jme_get_precession_matrix(2451545.0, 2451545.0 + 365.25, JME_MODEL_PREC_IAU_2006, m);
    double pos[3];
    jme_get_topo_pos(2451545.0, pos, err);
    jme_time_equ(2451545.0, &d1, err);
    
    jme_lmt_to_lat(2451545.0, 0, &d1, err);
    jme_lat_to_lmt(2451545.0, 0, &d1, err);
}

int main() {
    setbuf(stdout, NULL);
    test_metadata_and_paths();
    test_utility_math();
    test_time_and_calendar();
    test_coordinate_transforms();
    test_matrix_ops();
    test_body_info();
    test_state_ops();
    test_ayanamsa_and_sidereal();
    test_astro_models_and_params();
    test_ephemeris_calls();
    test_eclipses_and_phenomena();
    test_jpl_calls();
    test_extended_functions();
    
    printf("\nAll functions tested successfully!\n");
    return 0;
}
