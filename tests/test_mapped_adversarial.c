#include <jme/jme.h>

#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#ifndef M_PI
#define M_PI 3.141592653589793238462643383279502884
#endif

#if defined(__unix__) || defined(__APPLE__)
#include <pthread.h>
#define JME_HAS_PTHREAD 1
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

static int finite_array(const double *values, size_t count)
{
    size_t i;
    for (i = 0; i < count; i++) {
        if (!isfinite(values[i])) {
            return 0;
        }
    }
    return 1;
}

static unsigned int lcg_next(unsigned int *state)
{
    *state = (*state * 1664525u) + 1013904223u;
    return *state;
}

static double random_range(unsigned int *state, double min_value, double max_value)
{
    double unit = (double)(lcg_next(state) & 0x00ffffffu) / (double)0x01000000u;
    return min_value + (max_value - min_value) * unit;
}

static int test_angle_time_coordinate_fuzz(void)
{
    unsigned int seed = 0x5eed1234u;
    int i;

    for (i = 0; i < 20000; i++) {
        double a = random_range(&seed, -1.0e9, 1.0e9);
        double b = random_range(&seed, -1.0e9, 1.0e9);
        double lon = random_range(&seed, -720.0, 720.0);
        double lat = random_range(&seed, -89.999, 89.999);
        double eps = random_range(&seed, -30.0, 30.0) * (M_PI / 180.0);
        double ra = 0.0;
        double dec = 0.0;
        double lon2 = 0.0;
        double lat2 = 0.0;
        int cs = (int)(lcg_next(&seed) % 1000000000u) - 500000000;
        int norm_cs = jme_centiseconds_normalize(cs);
        double norm_deg = jme_degree_normalize(a);
        double norm_rad = jme_radian_normalize(a);

        ASSERT_TRUE(isfinite(norm_deg) && norm_deg >= 0.0 && norm_deg < 360.0);
        ASSERT_TRUE(isfinite(norm_rad) && norm_rad >= 0.0 && norm_rad < (2.0 * M_PI));
        ASSERT_TRUE(norm_cs >= 0 && norm_cs < 129600000);
        ASSERT_TRUE(fabs(jme_degrees_difference_signed(a, b)) <= 180.0);
        ASSERT_TRUE(jme_degrees_difference(a, b) >= 0.0 && jme_degrees_difference(a, b) < 360.0);
        ASSERT_TRUE(fabs(jme_radians_difference_signed(a, b)) <= M_PI);
        ASSERT_TRUE(isfinite(jme_degree_midpoint(a, b)));
        ASSERT_TRUE(isfinite(jme_radian_midpoint(a, b)));

        jme_ecliptic_to_equatorial(lon, lat, eps, &ra, &dec);
        jme_equatorial_to_ecliptic(ra, dec, eps, &lon2, &lat2);
        ASSERT_TRUE(fabs(jme_degrees_difference_signed(lon2, lon)) < 1.0e-7);
        ASSERT_TRUE(fabs(lat2 - lat) < 1.0e-7);
    }

    return 0;
}

static int test_calendar_and_string_boundaries(void)
{
    const int years[] = {-4000, -1, 0, 1, 1582, 1900, 2000, 2400, 9999};
    char buffer[64];
    int i;

    ASSERT_TRUE(jme_date_is_valid(2000, 2, 29, JME_CALENDAR_GREGORIAN) == 1);
    ASSERT_TRUE(jme_date_is_valid(1900, 2, 29, JME_CALENDAR_GREGORIAN) == 0);
    ASSERT_TRUE(jme_date_is_valid(2000, 13, 1, JME_CALENDAR_GREGORIAN) == 0);
    ASSERT_TRUE(jme_date_is_valid(2000, 1, 0, JME_CALENDAR_GREGORIAN) == 0);

    for (i = 0; i < (int)(sizeof(years) / sizeof(years[0])); i++) {
        int y = 0;
        int m = 0;
        int d = 0;
        double h = 0.0;
        double jd = jme_julian_day(years[i], 3, 1, 12.25, JME_CALENDAR_GREGORIAN);
        jme_reverse_julian_day(jd, JME_CALENDAR_GREGORIAN, &y, &m, &d, &h);
        ASSERT_TRUE(y == years[i] && m == 3 && d == 1 && fabs(h - 12.25) < 1.0e-8);
        ASSERT_TRUE(jme_day_of_week(jd) >= 0 && jme_day_of_week(jd) <= 6);
    }

    ASSERT_TRUE(jme_centiseconds_to_time_string(-1, buffer) == buffer);
    ASSERT_TRUE(jme_centiseconds_to_lonlat_string(-129600001, buffer) == buffer);
    ASSERT_TRUE(jme_centiseconds_to_degree_string(129600001, buffer) == buffer);
    ASSERT_TRUE(buffer[0] != '\0');

    return 0;
}

static int test_malformed_inputs_and_null_contracts(void)
{
    char error[256];
    double values[32];
    double geopos[3] = {0.0, 51.5, 0.0};
    double cusps[13];
    double ascmc[10];
    double tret[20];
    double attr[20];

    ASSERT_ERR(jme_calc(2451545.0, 999999, 0, values, error));
    ASSERT_ERR(jme_calc(2451545.0, JME_BODY_SUN, 0, 0, error));
    ASSERT_ERR(jme_calc_pctr(2451545.0, JME_BODY_MARS, 999999, 0, values, error));
    ASSERT_ERR(jme_pheno(2451545.0, JME_BODY_MARS, 0, 0, error));
    ASSERT_ERR(jme_fixstar(0, 2451545.0, 0, values, error));
    ASSERT_ERR(jme_fixstar("", 2451545.0, 0, values, error));
    ASSERT_ERR(jme_fixstar("☉\xff\xfe", 2451545.0, 0, values, error));
    ASSERT_ERR(jme_fixstar_mag("Not A Catalog Star", values, error));
    ASSERT_ERR(jme_houses(2451545.0, 0.0, 0.0, 'P', 0, ascmc));
    ASSERT_OK(jme_houses_ex2(2451545.0, 0, 0.0, 0.0, 'P', cusps, ascmc, 0, 0));
    ASSERT_TRUE(isnan(jme_house_pos(0.0, 0.0, 0.4, 'P', 0, error)));
    ASSERT_ERR(jme_gauquelin_sector(2451545.0, JME_BODY_SUN, 0, 0, 99, geopos, 1013.25, 15.0, values, error));
    ASSERT_ERR(jme_sol_eclipse_where(2451545.0, 0, 0, attr, error));
    ASSERT_ERR(jme_sol_eclipse_how(2451545.0, 0, geopos, 0, error));
    ASSERT_ERR(jme_sol_eclipse_when_loc(2451545.0, 0, geopos, 0, attr, 0, error));
    ASSERT_ERR(jme_sol_eclipse_when_glob(2451545.0, 0, 0, 0, 0, error));
    ASSERT_ERR(jme_lun_eclipse_how(2451545.0, 0, geopos, 0, error));
    ASSERT_ERR(jme_lun_eclipse_when_loc(2451545.0, 0, 0, tret, attr, 0, error));
    ASSERT_ERR(jme_lun_eclipse_when(2451545.0, 0, 0, 0, 0, error));
    ASSERT_ERR(jme_lun_occult_where(2451545.0, JME_BODY_MARS, 0, 0, 0, attr, error));
    ASSERT_ERR(jme_lun_occult_when_loc(2451545.0, JME_BODY_MARS, 0, 0, geopos, 0, attr, 0, error));
    ASSERT_ERR(jme_lun_occult_when_glob(2451545.0, JME_BODY_MARS, 0, 0, 0, tret, 0, error));
    ASSERT_ERR(jme_rise_trans(2451545.0, JME_BODY_SUN, 0, 0, JME_RISE_RISE, 0, 1013.25, 15.0, values, error));
    ASSERT_ERR(jme_rise_trans_true_hor(2451545.0, JME_BODY_SUN, 0, 0, JME_RISE_RISE, geopos, 1013.25, 15.0, 0.0, 0, error));
    ASSERT_ERR(jme_solcross(0.0, 2451545.0, 0, 0, error));
    ASSERT_ERR(jme_mooncross(0.0, 2451545.0, 0, 0, error));
    ASSERT_ERR(jme_mooncross_node(2451545.0, 0, 0, error));
    ASSERT_ERR(jme_helio_cross(999999, 0.0, 2451545.0, 0, values, error));
    ASSERT_ERR(jme_nod_aps(2451545.0, 999999, 0, 0, values, error));
    ASSERT_ERR(jme_get_orbital_elements(2451545.0, 999999, 0, values, error));
    ASSERT_ERR(jme_orbit_max_min_true_distance(2451545.0, 999999, 0, &values[0], &values[1], &values[2], &values[3], error));
    ASSERT_ERR(jme_heliacal_ut(2451545.0, 0, attr, error));
    ASSERT_ERR(jme_heliacal_pheno_ut(2451545.0, geopos, 0, error));
    ASSERT_ERR(jme_vis_limit_mag(2451545.0, geopos, 0, error));
    ASSERT_TRUE(isnan(jme_heliacal_angle(2451545.0, 0, attr, error)));
    ASSERT_TRUE(isnan(jme_topo_arcus_visionis(2451545.0, 0, attr, error)));
    ASSERT_ERR(jme_lmt_to_lat(2451545.0, 0.0, 0, error));
    ASSERT_ERR(jme_lat_to_lmt(2451545.0, 0.0, 0, error));
    ASSERT_ERR(jme_get_ayanamsa_ex(2451545.0, -999, values, error));
    ASSERT_ERR(jme_get_ayanamsa_ex(2451545.0, JME_SIDEREAL_LAHIRI, 0, error));
    ASSERT_ERR(jme_time_equ(2451545.0, 0, error));
    ASSERT_ERR(jme_jpl_current_file_data(0, 0, &values[0], &values[1], (int *)&values[2], error));

    return 0;
}

static int test_production_like_workflows(void)
{
    const double dates[] = {1674484.0, 1903396.789532, 2415020.5, 2451545.0, 2460310.5, 2816787.5};
    const int bodies[] = {JME_BODY_SUN, JME_BODY_MOON, JME_BODY_MERCURY, JME_BODY_VENUS, JME_BODY_MARS, JME_BODY_JUPITER, JME_BODY_SATURN, JME_BODY_URANUS, JME_BODY_NEPTUNE, JME_BODY_PLUTO};
    const double locations[][3] = {
        {0.0, 0.0, 0.0},
        {-74.0060, 40.7128, 10.0},
        {77.2090, 28.6139, 216.0},
        {139.6917, 35.6895, 40.0},
        {0.0, 89.0, 0.0},
        {0.0, -89.0, 0.0}
    };
    char error[256];
    double res1[32];
    double res2[32];
    double attr[32];
    double cusps[13];
    double ascmc[10];
    size_t i;
    size_t j;

    jme_set_astro_models("ENGINE=MOSHIER", 0);
    for (i = 0; i < sizeof(dates) / sizeof(dates[0]); i++) {
        for (j = 0; j < sizeof(bodies) / sizeof(bodies[0]); j++) {
            ASSERT_OK(jme_calc(dates[i], bodies[j], JME_CALC_SPEED, res1, error));
            ASSERT_OK(jme_calc(dates[i], bodies[j], JME_CALC_SPEED, res2, error));
            ASSERT_TRUE(finite_array(res1, 6));
            ASSERT_TRUE(memcmp(res1, res2, sizeof(double) * 6) == 0);
            ASSERT_OK(jme_calc_ut(dates[i], bodies[j], JME_CALC_SPEED | JME_CALC_RADIANS, res1, error));
            ASSERT_TRUE(finite_array(res1, 6));
            ASSERT_OK(jme_pheno(dates[i], bodies[j], 0, attr, error));
            ASSERT_TRUE(finite_array(attr, 10));
        }
        ASSERT_OK(jme_fixstar("Sirius", dates[i], JME_CALC_SPEED, res1, error));
        ASSERT_TRUE(finite_array(res1, 6));
        ASSERT_ERR(jme_fixstar("सिरियस", dates[i], 0, res1, error));
        ASSERT_OK(jme_get_ayanamsa_ex(dates[i], JME_SIDEREAL_USHASHASHI, res1, error));
        ASSERT_TRUE(isfinite(res1[0]) && res1[0] >= 0.0 && res1[0] < 360.0);
    }

    for (i = 0; i < sizeof(locations) / sizeof(locations[0]); i++) {
        ASSERT_OK(jme_houses(2451545.0, locations[i][1], locations[i][0], fabs(locations[i][1]) > 80.0 ? 'E' : 'P', cusps, ascmc));
        ASSERT_TRUE(finite_array(cusps + 1, 12));
        ASSERT_OK(jme_rise_trans(2451545.0, JME_BODY_SUN, 0, 0, JME_RISE_MERIDIAN_TRANSIT, (double *)locations[i], 1013.25, 15.0, res1, error));
        ASSERT_TRUE(isfinite(res1[0]));
    }
    jme_set_astro_models(0, 0);

    return 0;
}

static int test_serialized_state_and_model_roundtrip(void)
{
    char models[512];
    char body[64];
    int mode = 0;
    double t0 = 0.0;
    double ayan = 0.0;

    jme_set_ephemeris_path("/tmp/jme data/देव");
    ASSERT_TRUE(strcmp(jme_ephemeris_path(), "/tmp/jme data/देव") == 0);
    jme_set_jpl_file("kernel-测试.bsp");
    ASSERT_TRUE(strcmp(jme_jpl_file(), "kernel-测试.bsp") == 0);

    jme_set_sidereal_mode(JME_SIDEREAL_USER, 2451545.0, 12.25);
    jme_get_sidereal_mode(&mode, &t0, &ayan);
    ASSERT_TRUE(mode == JME_SIDEREAL_USER && t0 == 2451545.0 && ayan == 12.25);

    jme_set_astro_models("ENGINE=VSOP_ELP_MEEUS;BIAS=NONE;NUT=IAU2000B;OBL=IAU2006;PREC=IAU2006;SIDT=IAU2006;DELTAT=ESPENAK_MEEUS_2006", 0);
    ASSERT_OK(jme_get_astro_models(models, 0));
    ASSERT_TRUE(strstr(models, "ENGINE=VSOP_ELP_MEEUS") != 0);
    ASSERT_TRUE(strstr(models, "NUT=335") != 0);
    ASSERT_TRUE(strstr(models, "SIDT=345") != 0);

    ASSERT_TRUE(jme_copy_body_name(JME_BODY_JUPITER, body) == body);
    ASSERT_TRUE(strcmp(body, "Jupiter") == 0);
    ASSERT_TRUE(strcmp(jme_body_name(999999), "Unknown") == 0);
    ASSERT_TRUE(strcmp(jme_get_ayanamsa_name(-999), "Unknown sidereal mode") == 0);
    jme_set_astro_models(0, 0);

    return 0;
}

#if defined(JME_HAS_PTHREAD)
static void *thread_worker(void *arg)
{
    int index = *(int *)arg;
    int i;
    char error[256];
    double out[32];

    for (i = 0; i < 250; i++) {
        double jd = 2451545.0 + (double)((index * 250) + i);
        if (jme_calc(jd, JME_BODY_MARS, JME_CALC_SPEED, out, error) != JME_OK || !finite_array(out, 6)) {
            return (void *)1;
        }
        if (jme_get_ayanamsa_ex(jd, JME_SIDEREAL_LAHIRI, out, error) != JME_OK || !isfinite(out[0])) {
            return (void *)1;
        }
        if (jme_fixstar("Sirius", jd, 0, out, error) != JME_OK || !finite_array(out, 3)) {
            return (void *)1;
        }
        if (!isfinite(jme_sidereal_time(jd))) {
            return (void *)1;
        }
    }
    return 0;
}

static int test_concurrent_read_only_calls(void)
{
    pthread_t threads[4];
    int indexes[4] = {0, 1, 2, 3};
    int i;

    for (i = 0; i < 4; i++) {
        ASSERT_TRUE(pthread_create(&threads[i], 0, thread_worker, &indexes[i]) == 0);
    }
    for (i = 0; i < 4; i++) {
        void *status = 0;
        ASSERT_TRUE(pthread_join(threads[i], &status) == 0);
        ASSERT_TRUE(status == 0);
    }
    return 0;
}
#else
static int test_concurrent_read_only_calls(void)
{
    return 0;
}
#endif

int main(void)
{
    int status = 0;

    status |= test_angle_time_coordinate_fuzz();
    status |= test_calendar_and_string_boundaries();
    status |= test_malformed_inputs_and_null_contracts();
    status |= test_production_like_workflows();
    status |= test_serialized_state_and_model_roundtrip();
    status |= test_concurrent_read_only_calls();

    if (status != 0) {
        return 1;
    }
    printf("Mapped adversarial validation passed\n");
    return 0;
}
