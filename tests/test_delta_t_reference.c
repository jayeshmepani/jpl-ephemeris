/*
 * Delta-T certification against independent references (see
 * tools/gen_deltat_fixtures.py): astropy/IERS-A (observed UT1), Skyfield's
 * Table-S15 splines (Stephenson-Morrison-Hohenkerk 2016 / Morrison et al. 2021),
 * and an independent re-implementation of Espenak & Meeus 2006.
 */
#include "jme/jme.h"
#include "jme/jme_extended.h"
#include "fixtures/deltat_reference.h"

#include <math.h>
#include <stdio.h>
#include <string.h>

#define N(a) ((int)(sizeof(a) / sizeof((a)[0])))

static int check_set(const char *name, const jme_dt_fixture *f, int n, int model, double abs_tol, double rel_tol)
{
    int i;
    char error[256];
    for (i = 0; i < n; i++) {
        double dt = jme_delta_t_ex(f[i].jd, model, error);
        double tol = abs_tol + rel_tol * fabs(f[i].dt);
        if (!isfinite(dt) || fabs(dt - f[i].dt) > tol) {
            fprintf(stderr, "%s[%d] JD %.6f: got %.12f expected %.12f (tol %.3g) %s\n",
                name, i, f[i].jd, dt, f[i].dt, tol, error);
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

    jme_set_delta_t_userdef(NAN);
    failures += check_set("iers_nodes", dt_iers_nodes, N(dt_iers_nodes), JME_TIME_DELTAT_AUTOMATIC, 1.0e-6, 0.0);
    failures += check_set("iers_noon", dt_iers_noon, N(dt_iers_noon), JME_TIME_DELTAT_AUTOMATIC, 2.0e-4, 0.0);
    failures += check_set("smh2016", dt_smh2016, N(dt_smh2016), JME_MODEL_DELTAT_STEPHENSON_ETC_2016, 1.0e-9, 1.0e-13);
    failures += check_set("em2006", dt_em2006, N(dt_em2006), JME_MODEL_DELTAT_ESPENAK_MEEUS_2006, 1.0e-9, 1.0e-13);

    /* AUTOMATIC composition: SMH-2016 before the IERS table, EM-2006 after it. */
    if (jme_delta_t_iers_table_info(&first, &last_obs, &last, error) != JME_OK
        || !(first == 2441684.5) || !(last_obs > 2461000.5) || !(last >= last_obs)) {
        fprintf(stderr, "IERS table info invalid: %.1f %.1f %.1f %s\n", first, last_obs, last, error);
        failures++;
    }
    dt = jme_delta_t_ex(first - 0.25, JME_TIME_DELTAT_AUTOMATIC, error);
    dt2 = jme_delta_t_ex(first - 0.25, JME_MODEL_DELTAT_STEPHENSON_ETC_2016, error);
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
        const int models[] = {JME_TIME_DELTAT_AUTOMATIC, JME_MODEL_DELTAT_ESPENAK_MEEUS_2006, JME_MODEL_DELTAT_STEPHENSON_ETC_2016};
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
        const int refused[] = {JME_MODEL_DELTAT_STEPHENSON_MORRISON_1984, JME_MODEL_DELTAT_STEPHENSON_1997,
            JME_MODEL_DELTAT_STEPHENSON_MORRISON_2004, -1, 0};
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
        if (isfinite(dt) || strstr(error, "finite") == 0) { fprintf(stderr, "NaN JD accepted\n"); failures++; }
        dt = jme_delta_t_ex(INFINITY, JME_TIME_DELTAT_AUTOMATIC, error);
        if (isfinite(dt)) { fprintf(stderr, "Inf JD accepted\n"); failures++; }
    }
    /* Default model is AUTOMATIC. */
    jme_set_astro_models(0, 0);
    if (jme_delta_t(2451545.0) != jme_delta_t_ex(2451545.0, JME_TIME_DELTAT_AUTOMATIC, 0)) {
        fprintf(stderr, "default Delta-T model is not AUTOMATIC\n");
        failures++;
    }
    if (jme_set_astro_models_checked("DELTAT=STEPHENSON_ETC_2016", error) != JME_OK
        || jme_delta_t(2451545.0) != jme_delta_t_ex(2451545.0, JME_MODEL_DELTAT_STEPHENSON_ETC_2016, 0)) {
        fprintf(stderr, "DELTAT token not applied: %s\n", error);
        failures++;
    }
    jme_set_astro_models(0, 0);

    if (failures == 0) printf("Delta-T reference certification passed\n");
    return failures == 0 ? 0 : 1;
}
