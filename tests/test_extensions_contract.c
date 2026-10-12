/*
 * Contract tests for the sidereal-time, ayanamsa, engine-resolution,
 * JPL-centre-policy and validating-setter entry points added in 1.1.0.
 */
#include <math.h>
#include <stdio.h>
#include <string.h>

#include "jme/jme.h"
#include "jme/jme_extended.h"

static int failures = 0;

#define CHECK(cond, msg) do { if (!(cond)) { fprintf(stderr, "FAIL: %s (line %d)\n", msg, __LINE__); failures++; } } while (0)

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
    CHECK(jme_sidereal_time_ex(NAN, 0, &mean, err) == JME_ERR && isnan(mean), "non-finite date rejected");
}

static void test_ayanamsa_ex2_ut(void)
{
    char err[256];
    const double jd_ut = 2460000.5;
    const double jd_et = jd_ut + jme_delta_t(jd_ut) / 86400.0;
    double a_ut = NAN, a_et = NAN, r_ut = NAN, r_et = NAN;
    int flag;

    for (flag = JME_AYANAMSA_MEAN; flag <= JME_AYANAMSA_TRUE; flag++) {
        CHECK(jme_get_ayanamsa_ex2_ut(jd_ut, JME_SIDEREAL_LAHIRI, flag, &a_ut, &r_ut, err) == JME_OK, "ayanamsa UT");
        CHECK(jme_get_ayanamsa_ex2(jd_et, JME_SIDEREAL_LAHIRI, flag, &a_et, &r_et, err) == JME_OK, "ayanamsa TT");
        CHECK(a_ut == a_et && r_ut == r_et, "UT variant equals TT variant at jd_ut + delta T");
    }
    CHECK(jme_get_ayanamsa_ex2_ut(jd_ut, JME_SIDEREAL_LAHIRI, 0, 0, 0, err) == JME_ERR, "null output rejected");
}

static void test_resolve_engine(void)
{
    char err[256];
    int id = -1, substituted = -1;

    jme_set_astro_models("ENGINE=MOSHIER", 0);
    CHECK(jme_calc_resolve_engine(2451545.0, JME_BODY_MARS, &id, &substituted, err) == JME_OK, "resolve MOSHIER");
    CHECK(id == JME_ENGINE_ID_MOSHIER && substituted == 0, "MOSHIER engine id");
    jme_set_astro_models("ENGINE=VSOP_ELP_MEEUS", 0);
    CHECK(jme_calc_resolve_engine(2451545.0, JME_BODY_MOON, &id, 0, err) == JME_OK, "resolve VSOP_ELP_MEEUS");
    CHECK(id == JME_ENGINE_ID_VSOP87_ELP82B, "VSOP87/ELP 2000-82B engine id");
    CHECK(jme_calc_resolve_engine(2451545.0, JME_BODY_MARS, 0, 0, err) == JME_ERR, "null engine_id rejected");
    CHECK(jme_calc_resolve_engine(NAN, JME_BODY_MARS, &id, 0, err) == JME_ERR && id == 0, "non-finite date rejected");
    jme_set_astro_models("ENGINE=MOSHIER", 0);
}

static void test_jpl_center_policy(void)
{
    char err[256];

    CHECK(jme_get_jpl_center_policy() == JME_JPL_CENTER_STRICT, "STRICT is the default");
    CHECK(jme_set_jpl_center_policy(JME_JPL_CENTER_BARYCENTER, err) == JME_OK, "set BARYCENTER");
    CHECK(jme_get_jpl_center_policy() == JME_JPL_CENTER_BARYCENTER, "BARYCENTER stored");
    CHECK(jme_set_jpl_center_policy(99, err) == JME_ERR && err[0] != '\0', "unknown policy rejected");
    CHECK(jme_get_jpl_center_policy() == JME_JPL_CENTER_BARYCENTER, "rejected call leaves policy unchanged");
    CHECK(jme_set_jpl_center_policy(JME_JPL_CENTER_STRICT, err) == JME_OK, "restore STRICT");
}

static void test_checked_setters(void)
{
    char err[256];
    int mode = -1;
    double t0 = NAN, ayan = NAN;

    CHECK(jme_set_topo_checked(77.2, 28.6, 216.0, err) == JME_OK, "valid topocentre");
    CHECK(jme_set_topo_checked(77.2, 90.5, 216.0, err) == JME_ERR, "latitude above 90 rejected");
    CHECK(jme_set_topo_checked(NAN, 28.6, 216.0, err) == JME_ERR, "non-finite longitude rejected");
    CHECK(jme_set_topo_checked(77.2, 28.6, INFINITY, err) == JME_ERR, "non-finite altitude rejected");

    CHECK(jme_set_sidereal_mode_checked(JME_SIDEREAL_LAHIRI, 0.0, 0.0, err) == JME_OK, "valid sidereal mode");
    CHECK(jme_set_sidereal_mode_checked(-12345, 0.0, 0.0, err) == JME_ERR, "unknown sidereal mode rejected");
    CHECK(jme_set_sidereal_mode_checked(JME_SIDEREAL_FAGAN_BRADLEY, NAN, 0.0, err) == JME_ERR, "non-finite t0 rejected");
    jme_get_sidereal_mode(&mode, &t0, &ayan);
    CHECK(mode == JME_SIDEREAL_LAHIRI && t0 == 0.0 && ayan == 0.0, "rejected calls leave the mode unchanged");
}

int main(void)
{
    test_sidereal_time_ex();
    test_ayanamsa_ex2_ut();
    test_resolve_engine();
    test_jpl_center_policy();
    test_checked_setters();
    jme_close();
    if (failures != 0) {
        fprintf(stderr, "%d failure(s)\n", failures);
        return 1;
    }
    printf("extensions_contract: all checks passed\n");
    return 0;
}
