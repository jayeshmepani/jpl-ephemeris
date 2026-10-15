/*
 * Polar motion: IERS EOP loading/interpolation and the topocentric observer
 * vector against ERFA eraC2t06a (tools/gen_polar_motion_fixtures.py).
 */
#include <math.h>
#include <stdio.h>
#include <string.h>
#include "jme/jme.h"
#include "jme/jme_extended.h"
#include "fixtures/polar_motion_reference.h"

#ifndef JME_TEST_FIXTURE_DIR
#define JME_TEST_FIXTURE_DIR "tests/fixtures"
#endif
#define N(a)  (sizeof(a) / sizeof((a)[0]))
#define AU_KM 149597870.7

static int observer_km(double jd_tt, double lon, double lat, double alt, double *km, char *error)
{
    double au[3];
    int i;
    jme_set_topo(lon, lat, alt);
    if (jme_get_topo_pos(jd_tt, au, error) != JME_OK) {
        return JME_ERR;
    }
    for (i = 0; i < 3; i++) {
        km[i] = au[i] * AU_KM;
    }
    return JME_OK;
}

int main(void)
{
    char error[512] = {0};
    int failures = 0;
    size_t k;
    double x, y, km[3];
    double max_d = 0.0, max_d0 = 0.0, min_effect = 1.0e300;
    int predicted;

    jme_set_astro_models("IAU2006A", 0);
    jme_set_delta_t_userdef(PM_DELTA_T);

    /* Without EOP data, enabling polar motion must fail, never assume zero. */
    jme_eop_clear();
    jme_set_polar_motion(1);
    if (jme_get_polar_motion() != 1 ||
        observer_km(pm_gcrs_reference[0].jd_tt, 0.0, 0.0, 0.0, km, error) == JME_OK ||
        strstr(error, "EOP") == 0 || jme_eop_coverage(&x, &y, error) == JME_OK ||
        jme_eop_polar_motion(58849.0, &x, &y, &predicted, error) == JME_OK) {
        fprintf(stderr, "polar motion without EOP data must be an error: %s\n", error);
        failures++;
    }

    /* EOP C04 parsing and interpolation. */
    if (jme_eop_load_file(JME_TEST_FIXTURE_DIR "/eop_c04_excerpt.txt", error) != JME_OK) {
        fprintf(stderr, "C04 load failed: %s\n", error);
        return 1;
    }
    for (k = 0; k < N(pm_c04_reference); k++) {
        if (jme_eop_polar_motion(pm_c04_reference[k].mjd_ut1, &x, &y, &predicted, error) !=
                JME_OK ||
            fabs(x - pm_c04_reference[k].xp) > 1.0e-12 ||
            fabs(y - pm_c04_reference[k].yp) > 1.0e-12 || predicted) {
            fprintf(
                stderr,
                "C04 interpolation mismatch at %.6f: %s\n",
                pm_c04_reference[k].mjd_ut1,
                error
            );
            failures++;
        }
    }

    /* finals2000A parsing and interpolation, coverage and gap errors. */
    if (jme_eop_load_file(JME_TEST_FIXTURE_DIR "/eop_finals2000A_excerpt.txt", error) != JME_OK) {
        fprintf(stderr, "finals2000A load failed: %s\n", error);
        return 1;
    }
    for (k = 0; k < N(pm_interp_reference); k++) {
        if (jme_eop_polar_motion(pm_interp_reference[k].mjd_ut1, &x, &y, &predicted, error) !=
                JME_OK ||
            fabs(x - pm_interp_reference[k].xp) > 1.0e-12 ||
            fabs(y - pm_interp_reference[k].yp) > 1.0e-12) {
            fprintf(
                stderr,
                "finals interpolation mismatch at %.6f: %s\n",
                pm_interp_reference[k].mjd_ut1,
                error
            );
            failures++;
        }
    }
    if (jme_eop_coverage(&x, &y, error) != JME_OK || x != 58845.0 ||
        jme_eop_polar_motion(58844.0, &x, &y, &predicted, error) == JME_OK ||
        jme_eop_polar_motion(59000.0, &x, &y, &predicted, error) == JME_OK ||
        jme_eop_load_file(JME_TEST_FIXTURE_DIR "/no_such_eop_file.txt", error) == JME_OK) {
        fprintf(stderr, "EOP coverage/gap contract failed: %s\n", error);
        failures++;
    }

    /* Observer GCRS vector against eraC2t06a, with and without polar motion. */
    for (k = 0; k < N(pm_gcrs_reference); k++) {
        double d = 0.0, d0 = 0.0, effect = 0.0, km0[3];
        int i;
        jme_set_polar_motion(1);
        if (observer_km(
                pm_gcrs_reference[k].jd_tt,
                pm_gcrs_reference[k].lon,
                pm_gcrs_reference[k].lat,
                pm_gcrs_reference[k].alt,
                km,
                error
            ) != JME_OK) {
            fprintf(stderr, "observer with polar motion failed: %s\n", error);
            failures++;
            continue;
        }
        jme_set_polar_motion(0);
        if (observer_km(
                pm_gcrs_reference[k].jd_tt,
                pm_gcrs_reference[k].lon,
                pm_gcrs_reference[k].lat,
                pm_gcrs_reference[k].alt,
                km0,
                error
            ) != JME_OK) {
            failures++;
            continue;
        }
        for (i = 0; i < 3; i++) {
            d += pow(km[i] - pm_gcrs_reference[k].gcrs_km[i], 2);
            d0 += pow(km0[i] - pm_gcrs_reference[k].gcrs_km_no_pm[i], 2);
            effect +=
                pow(pm_gcrs_reference[k].gcrs_km[i] - pm_gcrs_reference[k].gcrs_km_no_pm[i], 2);
        }
        d = sqrt(d) * 1.0e6; /* mm */
        d0 = sqrt(d0) * 1.0e6;
        effect = sqrt(effect) * 1.0e6;
        if (d > max_d) {
            max_d = d;
        }
        if (d0 > max_d0) {
            max_d0 = d0;
        }
        if (effect < min_effect) {
            min_effect = effect;
        }
        if (!(d < 1.0) || !(d0 < 1.0) || !(effect > 1000.0)) {
            fprintf(
                stderr,
                "GCRS observer mismatch case %u: pm %.3f mm, no-pm %.3f mm, effect %.1f mm\n",
                (unsigned)k,
                d,
                d0,
                effect
            );
            failures++;
        }
    }

    /*
     * Horizontal coordinates (rise/set altitude function): zero xp, yp must
     * reproduce the hour-angle formula; real EOP moves the rise by far less
     * than 0.05 s; without EOP coverage the search must fail.
     */
    {
        double geopos[3] = {77.21, 28.61, 216.0};
        int rsmi = 1 | JME_RISE_DISC_CENTER | JME_RISE_NO_REFRACTION;
        double t_off[10] = {0}, t_zero[10] = {0}, t_pm[10] = {0};
        FILE *f = fopen("pm_zero_eop.txt", "w");
        int d;
        for (d = 0; d < 10 && f != 0; d++) {
            fprintf(f, "2020 1 %d 0 %.2f 0.000000 0.000000\n", d + 1, 58849.0 + d);
        }
        if (f != 0) {
            fclose(f);
        }
        jme_set_polar_motion(0);
        if (jme_rise_trans(2458851.5, JME_BODY_SUN, 0, 0, rsmi, geopos, 0.0, 0.0, t_off, error) !=
                JME_OK ||
            jme_eop_load_file("pm_zero_eop.txt", error) != JME_OK) {
            fprintf(stderr, "rise baseline failed: %s\n", error);
            failures++;
        } else {
            jme_set_polar_motion(1);
            if (jme_rise_trans(
                    2458851.5,
                    JME_BODY_SUN,
                    0,
                    0,
                    rsmi,
                    geopos,
                    0.0,
                    0.0,
                    t_zero,
                    error
                ) != JME_OK ||
                fabs(t_zero[0] - t_off[0]) * 86400.0 > 1.0e-4) {
                fprintf(
                    stderr,
                    "zero polar motion changed the rise: %.9f s (%s)\n",
                    (t_zero[0] - t_off[0]) * 86400.0,
                    error
                );
                failures++;
            }
            if (jme_eop_load_file(JME_TEST_FIXTURE_DIR "/eop_finals2000A_excerpt.txt", error) !=
                    JME_OK ||
                jme_rise_trans(
                    2458851.5,
                    JME_BODY_SUN,
                    0,
                    0,
                    rsmi,
                    geopos,
                    0.0,
                    0.0,
                    t_pm,
                    error
                ) != JME_OK ||
                !(fabs(t_pm[0] - t_off[0]) * 86400.0 < 0.05)) {
                fprintf(stderr, "rise with polar motion failed or moved too far: %s\n", error);
                failures++;
            } else {
                printf(
                    "sunrise shift from polar motion (Delhi, 2020-01-03): %.6f s\n",
                    (t_pm[0] - t_off[0]) * 86400.0
                );
            }
            if (jme_rise_trans(
                    2459200.5,
                    JME_BODY_SUN,
                    0,
                    0,
                    rsmi,
                    geopos,
                    0.0,
                    0.0,
                    t_pm,
                    error
                ) == JME_OK) {
                fprintf(stderr, "rise outside EOP coverage must fail when polar motion is on\n");
                failures++;
            }
        }
        remove("pm_zero_eop.txt");
    }

    jme_set_polar_motion(0);
    jme_eop_clear();
    jme_set_delta_t_userdef(NAN);
    jme_set_astro_models(0, 0);
    printf(
        "max |engine - eraC2t06a| = %.6f mm (polar motion), %.6f mm (none); min polar-motion "
        "effect %.1f mm\n",
        max_d,
        max_d0,
        min_effect
    );
    if (failures == 0) {
        printf("polar_motion_reference: all checks passed\n");
    }
    return failures == 0 ? 0 : 1;
}
