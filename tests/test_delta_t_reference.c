/*
 * Delta-T certification against independent references (see
 * tools/gen_deltat_fixtures.py): astropy/IERS-A (observed UT1), Skyfield's
 * Table-S15 splines (Stephenson-Morrison-Hohenkerk 2016 / Morrison et al. 2021),
 * and an independent re-implementation of Espenak & Meeus 2006.
 */
#include "jme/jme.h"
#include "jme/jme_extended.h"
#include "fixtures/deltat_reference.h"
#include "fixtures/deltat_smh2016_reference.h"

#include <math.h>
#include <stdio.h>
#include <string.h>

#define N(a) ((int)(sizeof(a) / sizeof((a)[0])))

static int check_set(
    const char *name,
    const jme_dt_fixture *f,
    int n,
    int model,
    double abs_tol,
    double rel_tol
)
{
    int i;
    char error[256];
    for (i = 0; i < n; i++) {
        double dt = jme_delta_t_ex(f[i].jd, model, error);
        double tol = abs_tol + rel_tol * fabs(f[i].dt);
        if (!isfinite(dt) || fabs(dt - f[i].dt) > tol) {
            fprintf(
                stderr,
                "%s[%d] JD %.6f: got %.12f expected %.12f (tol %.3g) %s\n",
                name,
                i,
                f[i].jd,
                dt,
                f[i].dt,
                tol,
                error
            );
            return 1;
        }
    }
    return 0;
}

int main(void)
{
    char error[256];
    double first, last_obs, last, dt, dt2;
    int failures = 0;

    if (jme_get_tid_acc() != (double)JME_TIME_TIDAL_AUTOMATIC) {
        fprintf(stderr, "default tidal-acceleration selector is not AUTOMATIC\n");
        failures++;
    }

    jme_set_delta_t_userdef(NAN);
    failures += check_set(
        "iers_nodes",
        dt_iers_nodes,
        N(dt_iers_nodes),
        JME_TIME_DELTAT_AUTOMATIC,
        1.0e-6,
        0.0
    );
    failures += check_set(
        "iers_noon",
        dt_iers_noon,
        N(dt_iers_noon),
        JME_TIME_DELTAT_AUTOMATIC,
        2.0e-4,
        0.0
    );
    /* dt_smh2016 (Skyfield) evaluates the 2020 revision of Table S15. */
    failures += check_set(
        "smh2021",
        dt_smh2016,
        N(dt_smh2016),
        JME_MODEL_DELTAT_STEPHENSON_ETC_2021,
        1.0e-9,
        1.0e-13
    );
    /* SMH 2016 Table S15 as published, rule S5.1 (independent exact evaluation). */
    {
        size_t k;
        for (k = 0; k < N(dt_smh2016_table); k++) {
            double v =
                jme_delta_t_ex(dt_smh2016_table[k].jd, JME_MODEL_DELTAT_STEPHENSON_ETC_2016, error);
            if (!(fabs(v - dt_smh2016_table[k].dt) <=
                  1.0e-9 + 1.0e-13 * fabs(dt_smh2016_table[k].dt))) {
                fprintf(
                    stderr,
                    "SMH-2016 Table S15 mismatch at JD %.6f: %.12f vs %.12f\n",
                    dt_smh2016_table[k].jd,
                    v,
                    dt_smh2016_table[k].dt
                );
                failures++;
            }
        }
    }
    failures += check_set(
        "em2006",
        dt_em2006,
        N(dt_em2006),
        JME_MODEL_DELTAT_ESPENAK_MEEUS_2006,
        1.0e-9,
        1.0e-13
    );

    /*
     * MS2004 Table-1 reconstruction: linear between rows of Table 1 (scan-
     * verified), parabola -20 + 32((y - 1820)/100)^2 outside; the spline model
     * itself stays refused.
     */
    {
        const struct {
            int year;
            int julian;
            double expected;
        } ms[] = {
            {-1000, 1, 25400.0                   },
            {-750,  1, 21500.0                   },
            {0,     1, 10580.0                   },
            {1000,  1, 1570.0                    },
            {1705,  0, 9.5                       },
            {1835,  0, 7.0                       },
            {1995,  0, 61.0                      },
            {2000,  0, 65.0                      },
            {-1500, 1, -20.0 + 32.0 * 33.2 * 33.2},
            {2100,  0, -20.0 + 32.0 * 2.8 * 2.8  }
        };

        size_t k;
        for (k = 0; k < N(ms); k++) {
            double jd = jme_julian_day(
                ms[k].year,
                1,
                1,
                0.0,
                ms[k].julian ? JME_CALENDAR_JULIAN : JME_CALENDAR_GREGORIAN
            );
            double v = jme_delta_t_ex(jd, JME_MODEL_DELTAT_MS2004_TABLE1_LINEAR, error);
            if (!(fabs(v - ms[k].expected) < 1.0e-9 * (1.0 + fabs(ms[k].expected)))) {
                fprintf(
                    stderr,
                    "MS2004 Table-1 reconstruction %d: %.9f vs %.9f\n",
                    ms[k].year,
                    v,
                    ms[k].expected
                );
                failures++;
            }
        }
        if (!isnan(jme_delta_t_ex(2451545.0, JME_MODEL_DELTAT_STEPHENSON_MORRISON_2004, error)) ||
            jme_set_astro_models_checked("DELTAT=MS2004_TABLE1_LINEAR", error) != JME_OK ||
            jme_delta_t(2451545.0) !=
                jme_delta_t_ex(2451545.0, JME_MODEL_DELTAT_MS2004_TABLE1_LINEAR, 0)) {
            fprintf(stderr, "MS2004 refusal/token contract failed: %s\n", error);
            failures++;
        }
        jme_set_astro_models(0, 0);
    }

    /* AUTOMATIC composition: SMH-2016 before the IERS table, EM-2006 after it. */
    if (jme_delta_t_iers_table_info(&first, &last_obs, &last, error) != JME_OK ||
        !(first == 2441684.5) || !(last_obs > 2461000.5) || !(last >= last_obs)) {
        fprintf(
            stderr,
            "IERS table info invalid: %.1f %.1f %.1f %s\n",
            first,
            last_obs,
            last,
            error
        );
        failures++;
    }
    dt = jme_delta_t_ex(first - 0.25, JME_TIME_DELTAT_AUTOMATIC, error);
    dt2 = jme_delta_t_ex(first - 0.25, JME_MODEL_DELTAT_STEPHENSON_ETC_2021, error);
    if (!(dt == dt2) || fabs(dt - 43.1526) > 0.001) {
        fprintf(stderr, "AUTOMATIC before IERS table: %.9f vs SMH %.9f\n", dt, dt2);
        failures++;
    }
    dt = jme_delta_t_ex(last + 1.0, JME_TIME_DELTAT_AUTOMATIC, error);
    dt2 = jme_delta_t_ex(last + 1.0, JME_MODEL_DELTAT_ESPENAK_MEEUS_2006, error);
    if (!(dt == dt2)) {
        fprintf(stderr, "AUTOMATIC after IERS table: %.9f vs EM %.9f\n", dt, dt2);
        failures++;
    }
    /* Continuity inside the IERS table across the 2016-12-31 leap second. */
    dt = jme_delta_t_ex(2457753.999, JME_TIME_DELTAT_AUTOMATIC, error);
    dt2 = jme_delta_t_ex(2457754.001, JME_TIME_DELTAT_AUTOMATIC, error);
    if (!(fabs(dt - dt2) < 1.0e-3) || fabs(dt - 68.59) > 0.05) {
        fprintf(stderr, "Delta-T not continuous across leap second: %.9f %.9f\n", dt, dt2);
        failures++;
    }
    /* SMH-2016 long-term parabola outside the splines (published, not patched). */
    {
        double jd = 2451545.0 + (-1000.0 - 2000.0) * 365.25, u = (-1000.0 - 1825.0) / 100.0;
        dt = jme_delta_t_ex(jd, JME_MODEL_DELTAT_STEPHENSON_ETC_2016, error);
        if (fabs(dt - (-320.0 + 32.5 * u * u)) > 1.0e-9) {
            fprintf(stderr, "SMH-2016 parabola mismatch: %.9f\n", dt);
            failures++;
        }
        dt = jme_delta_t_ex(jd, JME_TIME_DELTAT_AUTOMATIC, error);
        if (fabs(dt - (-320.0 + 32.5 * u * u)) > 1.0e-9) {
            fprintf(stderr, "AUTOMATIC long-term parabola mismatch: %.9f\n", dt);
            failures++;
        }
    }
    /* -3000 .. +3000 sweep: finite and monotone-in-sign sanity for every model. */
    {
        const int models[] = {
            JME_TIME_DELTAT_AUTOMATIC,
            JME_MODEL_DELTAT_ESPENAK_MEEUS_2006,
            JME_MODEL_DELTAT_STEPHENSON_ETC_2016,
            JME_MODEL_DELTAT_STEPHENSON_ETC_2021
        };
        int m;
        double jd;
        for (m = 0; m < N(models); m++) {
            for (jd = 625673.5; jd <= 2817151.5; jd += 3652.5) {
                dt = jme_delta_t_ex(jd, models[m], error);
                if (!isfinite(dt) || dt < -400.0 || dt > 120000.0 || error[0] != '\0') {
                    fprintf(stderr, "model %d sweep JD %.1f: %.6f %s\n", models[m], jd, dt, error);
                    failures++;
                    break;
                }
            }
        }
    }
    /* Unavailable models and invalid input fail closed. */
    {
        const int refused[] = {
            JME_MODEL_DELTAT_STEPHENSON_MORRISON_1984,
            JME_MODEL_DELTAT_STEPHENSON_1997,
            JME_MODEL_DELTAT_STEPHENSON_MORRISON_2004,
            -1,
            0
        };
        int k;
        for (k = 0; k < N(refused); k++) {
            error[0] = '\0';
            dt = jme_delta_t_ex(2451545.0, refused[k], error);
            if (isfinite(dt) || error[0] == '\0') {
                fprintf(stderr, "model %d not refused\n", refused[k]);
                failures++;
            }
        }
        dt = jme_delta_t_ex(NAN, JME_TIME_DELTAT_AUTOMATIC, error);
        if (isfinite(dt) || strstr(error, "finite") == 0) {
            fprintf(stderr, "NaN JD accepted\n");
            failures++;
        }
        dt = jme_delta_t_ex(INFINITY, JME_TIME_DELTAT_AUTOMATIC, error);
        if (isfinite(dt)) {
            fprintf(stderr, "Inf JD accepted\n");
            failures++;
        }
    }
    /* Default model is AUTOMATIC. */
    jme_set_astro_models(0, 0);
    if (jme_delta_t(2451545.0) != jme_delta_t_ex(2451545.0, JME_TIME_DELTAT_AUTOMATIC, 0)) {
        fprintf(stderr, "default Delta-T model is not AUTOMATIC\n");
        failures++;
    }
    if (jme_set_astro_models_checked("DELTAT=STEPHENSON_ETC_2016", error) != JME_OK ||
        jme_delta_t(2451545.0) !=
            jme_delta_t_ex(2451545.0, JME_MODEL_DELTAT_STEPHENSON_ETC_2016, 0)) {
        fprintf(stderr, "DELTAT token not applied: %s\n", error);
        failures++;
    }
    jme_set_astro_models(0, 0);

    /*
     * Named tidal selectors resolve to physical accelerations and adjust
     * historical Espenak-Meeus Delta-T by NASA's published secular-acceleration
     * correction. The setting must not alter observations from 1955 onward.
     */
    {
        const double jd_year_zero = jme_julian_day(
            0,
            1,
            1,
            0.0,
            JME_CALENDAR_JULIAN
        ); /* EM-2006 years are Julian before 1582 */
        const double jd_year_2000 = 2451545.0;
        double baseline, adjusted, expected, y, y_smh;
        int year, month, day;
        double hour;

        {
            const struct {
                int selector;
                double acceleration;
            } cases[] = {
                {JME_TIME_TIDAL_DE200, -23.8946},
                {JME_TIME_TIDAL_DE403, -25.580 },
                {JME_TIME_TIDAL_DE404, -25.580 },
                {JME_TIME_TIDAL_DE405, -25.826 },
                {JME_TIME_TIDAL_DE406, -25.826 },
                {JME_TIME_TIDAL_DE421, -25.85  },
                {JME_TIME_TIDAL_DE430, -25.82  },
                {JME_TIME_TIDAL_DE431, -25.80  }
            };

            int k;
            jme_set_tid_acc(JME_TIME_TIDAL_AUTOMATIC);
            baseline = jme_delta_t_ex(jd_year_zero, JME_MODEL_DELTAT_ESPENAK_MEEUS_2006, error);
            jme_reverse_julian_day(jd_year_zero, JME_CALENDAR_JULIAN, &year, &month, &day, &hour);
            y = (double)year;
            for (k = 0; k < N(cases); k++) {
                jme_set_tid_acc((double)cases[k].selector);
                if (jme_get_tid_acc() != (double)cases[k].selector) {
                    fprintf(
                        stderr,
                        "tidal selector %d was not preserved by getter\n",
                        cases[k].selector
                    );
                    failures++;
                }
                adjusted = jme_delta_t_ex(jd_year_zero, JME_MODEL_DELTAT_ESPENAK_MEEUS_2006, error);
                expected = -0.91072 * (cases[k].acceleration + 26.0) * ((y - 1955.0) / 100.0) *
                           ((y - 1955.0) / 100.0);
                if (!isfinite(adjusted) || fabs((adjusted - baseline) - expected) > 1.0e-8) {
                    fprintf(
                        stderr,
                        "tidal selector %d correction mismatch: got %.12f expected delta %.12f\n",
                        cases[k].selector,
                        adjusted - baseline,
                        expected
                    );
                    failures++;
                }
            }
        }

        /*
         * DE441 has no published tidal acceleration: historical model Delta-T
         * must fail explicitly, while observational dates are unaffected.
         */
        jme_set_tid_acc((double)JME_TIME_TIDAL_DE441);
        if (jme_get_tid_acc() != (double)JME_TIME_TIDAL_DE441 ||
            !isnan(jme_delta_t_ex(jd_year_zero, JME_MODEL_DELTAT_ESPENAK_MEEUS_2006, error)) ||
            strstr(error, "DE441") == 0 ||
            !isnan(jme_delta_t_ex(jd_year_zero, JME_MODEL_DELTAT_STEPHENSON_ETC_2016, error)) ||
            !isnan(jme_delta_t_ex(jd_year_zero, JME_TIME_DELTAT_AUTOMATIC, error)) ||
            !isfinite(jme_delta_t_ex(2451545.0, JME_MODEL_DELTAT_ESPENAK_MEEUS_2006, error)) ||
            !isfinite(jme_delta_t_ex(2451545.0, JME_TIME_DELTAT_AUTOMATIC, error))) {
            fprintf(stderr, "DE441 unresolved tidal selector contract failed: %s\n", error);
            failures++;
        }
        jme_set_delta_t_userdef(69.0);
        if (jme_delta_t_ex(jd_year_zero, JME_MODEL_DELTAT_ESPENAK_MEEUS_2006, error) != 69.0) {
            fprintf(stderr, "user-fixed Delta-T must bypass the tidal correction\n");
            failures++;
        }
        jme_set_delta_t_userdef(NAN);

        jme_set_tid_acc(-25.858);
        adjusted = jme_delta_t_ex(jd_year_zero, JME_MODEL_DELTAT_ESPENAK_MEEUS_2006, error);
        expected = -0.91072 * (-25.858 + 26.0) * ((y - 1955.0) / 100.0) * ((y - 1955.0) / 100.0);
        if (!isfinite(adjusted) || fabs((adjusted - baseline) - expected) > 1.0e-8) {
            fprintf(
                stderr,
                "numeric tidal correction mismatch: got %.12f expected delta %.12f\n",
                adjusted - baseline,
                expected
            );
            failures++;
        }

        /*
         * SMH-2016/2021 values correspond to DE430, -25.82 arcsec/century^2
         * (Stephenson et al. 2016, sect. 2(a)); the correction uses that baseline.
         */
        jme_set_tid_acc(JME_TIME_TIDAL_AUTOMATIC);
        baseline = jme_delta_t_ex(jd_year_zero, JME_MODEL_DELTAT_STEPHENSON_ETC_2016, error);
        jme_set_tid_acc(-25.858);
        adjusted = jme_delta_t_ex(jd_year_zero, JME_MODEL_DELTAT_STEPHENSON_ETC_2016, error);
        y_smh = 2000.0 + (jd_year_zero - 2451545.0) / 365.25;
        expected =
            -0.91072 * (-25.858 + 25.82) * ((y_smh - 1955.0) / 100.0) * ((y_smh - 1955.0) / 100.0);
        if (!isfinite(adjusted) || fabs((adjusted - baseline) - expected) > 1.0e-8) {
            fprintf(
                stderr,
                "SMH-2016 tidal correction mismatch: got %.12f expected delta %.12f\n",
                adjusted - baseline,
                expected
            );
            failures++;
        }

        jme_set_tid_acc(JME_TIME_TIDAL_DE200);
        baseline = jme_delta_t_ex(jd_year_2000, JME_MODEL_DELTAT_ESPENAK_MEEUS_2006, error);
        jme_set_tid_acc(JME_TIME_TIDAL_AUTOMATIC);
        adjusted = jme_delta_t_ex(jd_year_2000, JME_MODEL_DELTAT_ESPENAK_MEEUS_2006, error);
        if (fabs(adjusted - baseline) > 1.0e-10) {
            fprintf(stderr, "tidal correction changed the 1955-2005 observational interval\n");
            failures++;
        }
    }
    jme_set_tid_acc(JME_TIME_TIDAL_AUTOMATIC);

    if (failures == 0) {
        printf("Delta-T reference certification passed\n");
    }
    return failures == 0 ? 0 : 1;
}
