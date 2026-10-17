/*
 * Contract tests for the sidereal-time, ayanamsa, engine-resolution,
 * JPL-centre-policy and validating-setter entry points added in 1.1.0.
 */
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "jme/jme.h"
#include "jme/jme_extended.h"

static int failures = 0;

#define CHECK(cond, msg)                                                                           \
    do {                                                                                           \
        if (!(cond)) {                                                                             \
            fprintf(stderr, "FAIL: %s (line %d)\n", msg, __LINE__);                                \
            failures++;                                                                            \
        }                                                                                          \
    } while (0)

static void test_sidereal_time_ex(void)
{
    char err[256];
    double mean = NAN, apparent = NAN;
    const double jd = 2451545.0;

    CHECK(jme_sidereal_time_ex(jd, 0, &mean, err) == JME_OK, "mean sidereal time");
    CHECK(jme_sidereal_time_ex(jd, 1, &apparent, err) == JME_OK, "apparent sidereal time");
    /* jme_sidereal_time() is defined as the mean branch of this function. */
    CHECK(mean == jme_sidereal_time(jd), "mean branch equals jme_sidereal_time");
    /* GMST at 2000-01-01 12h UT1 is 18.697374558 h (IAU 1982 expression);
     * the IAU 2006 expression differs from it by less than 1e-7 h here. */
    CHECK(fabs(mean - 18.697374558) < 1e-7, "GMST at J2000.0");
    /* The equation of the equinoxes stays below 1.2 s of time. */
    CHECK(fabs(apparent - mean) * 3600.0 < 1.2 && apparent != mean, "equation of the equinoxes");
    CHECK(jme_sidereal_time_ex(jd, 0, 0, err) == JME_ERR, "null output rejected");
    CHECK(
        jme_sidereal_time_ex(NAN, 0, &mean, err) == JME_ERR && isnan(mean),
        "non-finite date rejected"
    );
}

static void test_ayanamsa_ex2_ut(void)
{
    char err[256];
    const double jd_ut = 2460000.5;
    const double jd_et = jd_ut + jme_delta_t(jd_ut) / 86400.0;
    double a_ut = NAN, a_et = NAN, r_ut = NAN, r_et = NAN;
    int flag;

    for (flag = JME_AYANAMSA_MEAN; flag <= JME_AYANAMSA_TRUE; flag++) {
        CHECK(
            jme_get_ayanamsa_ex2_ut(jd_ut, JME_SIDEREAL_LAHIRI, flag, &a_ut, &r_ut, err) == JME_OK,
            "ayanamsa UT"
        );
        CHECK(
            jme_get_ayanamsa_ex2(jd_et, JME_SIDEREAL_LAHIRI, flag, &a_et, &r_et, err) == JME_OK,
            "ayanamsa TT"
        );
        CHECK(a_ut == a_et && r_ut == r_et, "UT variant equals TT variant at jd_ut + delta T");
    }
    CHECK(
        jme_get_ayanamsa_ex2_ut(jd_ut, JME_SIDEREAL_LAHIRI, 0, 0, 0, err) == JME_ERR,
        "null output rejected"
    );
}

static void test_resolve_engine(void)
{
    char err[256];
    int id = -1, substituted = -1;

    jme_set_astro_models("ENGINE=MOSHIER", 0);
    CHECK(
        jme_calc_resolve_engine(2451545.0, JME_BODY_MARS, &id, &substituted, err) == JME_OK,
        "resolve MOSHIER"
    );
    CHECK(id == JME_ENGINE_ID_MOSHIER && substituted == 0, "MOSHIER engine id");
    jme_set_astro_models("ENGINE=VSOP_ELP_MEEUS", 0);
    CHECK(
        jme_calc_resolve_engine(2451545.0, JME_BODY_MOON, &id, 0, err) == JME_OK,
        "resolve VSOP_ELP_MEEUS"
    );
    CHECK(id == JME_ENGINE_ID_VSOP87_ELP82B, "VSOP87/ELP 2000-82B engine id");
    CHECK(
        jme_calc_resolve_engine(2451545.0, JME_BODY_MARS, 0, 0, err) == JME_ERR,
        "null engine_id rejected"
    );
    CHECK(
        jme_calc_resolve_engine(NAN, JME_BODY_MARS, &id, 0, err) == JME_ERR && id == 0,
        "non-finite date rejected"
    );
    jme_set_astro_models("ENGINE=MOSHIER", 0);
}

static void test_jpl_center_policy(void)
{
    char err[256];

    CHECK(jme_get_jpl_center_policy() == JME_JPL_CENTER_STRICT, "STRICT is the default");
    CHECK(jme_set_jpl_center_policy(JME_JPL_CENTER_BARYCENTER, err) == JME_OK, "set BARYCENTER");
    CHECK(jme_get_jpl_center_policy() == JME_JPL_CENTER_BARYCENTER, "BARYCENTER stored");
    CHECK(
        jme_set_jpl_center_policy(99, err) == JME_ERR && err[0] != '\0',
        "unknown policy rejected"
    );
    CHECK(
        jme_get_jpl_center_policy() == JME_JPL_CENTER_BARYCENTER,
        "rejected call leaves policy unchanged"
    );
    CHECK(jme_set_jpl_center_policy(JME_JPL_CENTER_STRICT, err) == JME_OK, "restore STRICT");
}

static void test_body_and_house_names(void)
{
    static const int extended_bodies[] = {
        JME_BODY_ALTJIRA,
        JME_BODY_AMYCUS,
        JME_BODY_ARROKOTH,
        JME_BODY_ASBOLUS,
        JME_BODY_ASTEROID_001,
        JME_BODY_ASTEROID_002,
        JME_BODY_ASTEROID_003,
        JME_BODY_ASTEROID_004,
        JME_BODY_ASTEROID_005,
        JME_BODY_ASTEROID_006,
        JME_BODY_ASTEROID_007,
        JME_BODY_ASTEROID_008,
        JME_BODY_ASTEROID_009,
        JME_BODY_ASTEROID_010,
        JME_BODY_ASTEROID_011,
        JME_BODY_ASTEROID_012,
        JME_BODY_CERES,
        JME_BODY_ERIS,
        JME_BODY_VESTA
    };
    double results[6];
    char error[256];
    size_t i;

    CHECK((int)JME_BODY_ALTJIRA != (int)JME_BODY_URANUS, "Altjira does not alias Uranus");
    CHECK((int)JME_BODY_AMYCUS != (int)JME_BODY_NEPTUNE, "Amycus does not alias Neptune");
    CHECK((int)JME_BODY_ARROKOTH != (int)JME_BODY_PLUTO, "Arrokoth does not alias Pluto");
    CHECK((int)JME_BODY_ASBOLUS != (int)JME_BODY_EARTH, "Asbolus does not alias Earth");
    CHECK(
        (int)JME_BODY_ASTEROID_001 != (int)JME_BODY_SOLAR_SYSTEM_BARYCENTER,
        "asteroid slot 1 is distinct"
    );
    CHECK((int)JME_BODY_ASTEROID_012 != (int)JME_BODY_TRUE_NODE, "asteroid slot 12 is distinct");
    CHECK(JME_BODY_ALTJIRA == -1000, "extended IDs start in the JME-private range");
    CHECK(JME_BODY_VESTA == -1265, "extended IDs end in the JME-private range");
    CHECK(JME_BODY_ASTEROID_110 < 0, "numbered asteroid IDs are outside positive external ranges");
    CHECK(JME_BODY_COMET_001 < 0, "numbered comet IDs do not use a positive comet offset");
    CHECK(JME_BODY_MINOR_PLANET_001 < 0, "numbered minor-planet IDs are JME-private");

    /* Extended IDs must not silently alias a core body. */
    for (i = 0; i < sizeof(extended_bodies) / sizeof(extended_bodies[0]); i++) {
        CHECK(
            strcmp(jme_body_name(extended_bodies[i]), "Unknown") != 0,
            "extended body has a display name"
        );
        CHECK(jme_body_naif_id(extended_bodies[i]) == -1, "extended body has no implicit NAIF id");
        CHECK(
            jme_calc_ut(2451545.0, extended_bodies[i], 0, results, error) == JME_ERR,
            "extended body without a kernel rejects instead of aliasing"
        );
    }
    CHECK(jme_body_id_from_name("Altjira") == JME_BODY_ALTJIRA, "Altjira name lookup");
    CHECK(jme_body_id_from_name("Asteroid 001") == JME_BODY_ASTEROID_001, "Asteroid name lookup");
    CHECK(jme_body_id_from_name("Ceres") == JME_BODY_CERES, "Ceres name lookup");
    CHECK(
        strcmp(jme_body_name(JME_BODY_ASTEROID_110), "Asteroid 110") == 0,
        "numbered asteroid display name"
    );
    CHECK(
        jme_body_id_from_name("Asteroid 110") == JME_BODY_ASTEROID_110,
        "numbered asteroid name lookup"
    );
    CHECK(
        strcmp(jme_body_name(JME_BODY_COMET_060), "Comet 060") == 0,
        "numbered comet display name"
    );

    CHECK(
        strcmp(jme_house_system_name(JME_HOUSE_PLACIDUS), "Placidus") == 0,
        "numeric Placidus name"
    );
    CHECK(strcmp(jme_house_system_name(JME_HOUSE_KOCH), "Koch") == 0, "numeric Koch name");
    CHECK(
        strcmp(jme_house_system_name(JME_HOUSE_GAUQUELIN), "Gauquelin 36 sectors") == 0,
        "numeric Gauquelin name"
    );
    CHECK(jme_house_system_name(9999) == 0, "unknown house system has no name");
}

static void test_jpl_lazy_open_contract(void)
{
    char error[256];
    double results[6];
    const char *kernel = getenv("JME_TEST_JPL_KERNEL");

    jme_set_ephemeris_path("/tmp/jme-contract-kernels");
    jme_set_jpl_file("definitely-not-a-kernel.bsp");
    CHECK(jme_jpl_is_open() == 0, "set_jpl_file does not leave a stale kernel open");
    jme_set_astro_models("ENGINE=JPL", 0);
    CHECK(
        jme_calc_ut(2451545.0, JME_BODY_MARS, 0, results, error) == JME_ERR,
        "high-level JPL path reports a missing/invalid kernel"
    );
    CHECK(error[0] != '\0', "lazy-open failure sets an error message");

    if (jme_jpl_is_available()) {
        char overlong_path[2048];

        memset(overlong_path, 'x', sizeof(overlong_path));
        overlong_path[0] = '/';
        overlong_path[sizeof(overlong_path) - 1] = '\0';
        CHECK(
            jme_jpl_open(overlong_path, error) == JME_ERR &&
                strstr(error, "path exceeds the supported length") != 0,
            "overlong absolute kernel path is rejected rather than truncated"
        );
    }

    /* Exercise successful lazy opening with a real kernel when one is available. */
    if (kernel != 0 && kernel[0] != '\0' && jme_jpl_is_available()) {
        char directory[1024];
        char filename[1024];
        const char *slash = strrchr(kernel, '/');
        const char *backslash = strrchr(kernel, '\\');
        const char *separator = slash;
        size_t directory_length;
        int i;
        int finite = 1;

        if (backslash != 0 && (separator == 0 || backslash > separator)) {
            separator = backslash;
        }
        if (separator == 0) {
            snprintf(directory, sizeof(directory), ".");
            snprintf(filename, sizeof(filename), "%s", kernel);
        } else {
            directory_length = (size_t)(separator - kernel);
            if (directory_length == 0) {
                directory_length = 1;
            }
            if (directory_length >= sizeof(directory) ||
                strlen(separator + 1) >= sizeof(filename)) {
                CHECK(0, "configured JPL kernel path fits test buffers");
            } else {
                memcpy(directory, kernel, directory_length);
                directory[directory_length] = '\0';
                snprintf(filename, sizeof(filename), "%s", separator + 1);
                jme_set_ephemeris_path(directory);
                jme_set_jpl_file(filename);
                jme_set_astro_models("ENGINE=JPL", 0);
                CHECK(
                    jme_calc_ut(2451545.0, JME_BODY_MARS_BARYCENTER, 0, results, error) == JME_OK,
                    "relative kernel path opens lazily for high-level JPL calculations"
                );
                for (i = 0; i < 3; i++) {
                    finite = finite && isfinite(results[i]);
                }
                CHECK(finite, "lazy-open JPL body state is finite");

                jme_set_ephemeris_path("/tmp/jme-contract-kernels-missing");
                CHECK(
                    jme_calc_ut(2451545.0, JME_BODY_MARS_BARYCENTER, 0, results, error) == JME_ERR,
                    "changing the ephemeris directory re-resolves a configured relative kernel"
                );
                jme_set_ephemeris_path(directory);
                CHECK(
                    jme_calc_ut(2451545.0, JME_BODY_MARS_BARYCENTER, 0, results, error) == JME_OK,
                    "restoring the ephemeris directory reopens the relative kernel"
                );
            }
        }
    }

    jme_set_astro_models("ENGINE=MOSHIER", 0);
    jme_set_jpl_file(0);
    jme_set_ephemeris_path(0);
}

static void test_rise_flag_contract(void)
{
    char error[256];
    double geopos[3] = {77.2, 28.6, 216.0};
    double tret = NAN;

    CHECK(
        jme_rise_trans(
            2451545.0,
            JME_BODY_SUN,
            0,
            0,
            JME_RISE_RISE | JME_RISE_DISC_BOTTOM,
            geopos,
            1013.25,
            15.0,
            &tret,
            error
        ) == JME_ERR,
        "DISC_BOTTOM is refused instead of being ignored"
    );
    CHECK(
        jme_rise_trans(
            2451545.0,
            JME_BODY_SUN,
            0,
            0,
            JME_RISE_RISE | JME_RISE_HINDU_RISING,
            geopos,
            1013.25,
            15.0,
            &tret,
            error
        ) == JME_ERR,
        "HINDU_RISING is refused instead of being ignored"
    );
    CHECK(
        jme_rise_trans(
            2451545.0,
            JME_BODY_SUN,
            0,
            0,
            JME_RISE_RISE | JME_RISE_DISC_CENTER,
            geopos,
            1013.25,
            15.0,
            &tret,
            error
        ) == JME_OK,
        "DISC_CENTER remains supported"
    );
}

static void test_checked_setters(void)
{
    char err[256];
    int mode = -1;
    double t0 = NAN, ayan = NAN;

    CHECK(jme_set_topo_checked(77.2, 28.6, 216.0, err) == JME_OK, "valid topocentre");
    CHECK(jme_set_topo_checked(77.2, 90.5, 216.0, err) == JME_ERR, "latitude above 90 rejected");
    CHECK(jme_set_topo_checked(NAN, 28.6, 216.0, err) == JME_ERR, "non-finite longitude rejected");
    CHECK(
        jme_set_topo_checked(77.2, 28.6, INFINITY, err) == JME_ERR,
        "non-finite altitude rejected"
    );

    CHECK(
        jme_set_sidereal_mode_checked(JME_SIDEREAL_LAHIRI, 0.0, 0.0, err) == JME_OK,
        "valid sidereal mode"
    );
    CHECK(
        jme_set_sidereal_mode_checked(-12345, 0.0, 0.0, err) == JME_ERR,
        "unknown sidereal mode rejected"
    );
    CHECK(
        jme_set_sidereal_mode_checked(JME_SIDEREAL_FAGAN_BRADLEY, NAN, 0.0, err) == JME_ERR,
        "non-finite t0 rejected"
    );
    jme_get_sidereal_mode(&mode, &t0, &ayan);
    CHECK(
        mode == JME_SIDEREAL_LAHIRI && t0 == 0.0 && ayan == 0.0,
        "rejected calls leave the mode unchanged"
    );
}

int main(void)
{
    test_sidereal_time_ex();
    test_ayanamsa_ex2_ut();
    test_resolve_engine();
    test_jpl_center_policy();
    test_body_and_house_names();
    test_jpl_lazy_open_contract();
    test_rise_flag_contract();
    test_checked_setters();
    jme_close();
    if (failures != 0) {
        fprintf(stderr, "%d failure(s)\n", failures);
        return 1;
    }
    printf("extensions_contract: all checks passed\n");
    return 0;
}
