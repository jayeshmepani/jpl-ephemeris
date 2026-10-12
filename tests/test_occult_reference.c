/*
 * Certification of the lunar-occultation searches on the JPL (CALCEPH) path
 * against independent Skyfield computations (tools/gen_occult_fixtures.py):
 * local disappearance/reappearance, inner contacts and maximum for planets
 * and stars at eight sites; global first/last contact on the Earth, which
 * the generator finds by direct minimisation over the WGS84 surface (JME uses
 * the fundamental-plane construction). Requires JME_TEST_JPL_KERNEL and
 * CALCEPH; otherwise exit 77.
 */
#include "jme/jme.h"
#include "jme/jme_extended.h"
#include "fixtures/occult_reference.h"

#include <math.h>
#include <stdio.h>
#include <stdlib.h>

#define N(a) ((int)(sizeof(a) / sizeof((a)[0])))
#define TOL_CONTACT_S 0.002
#define TOL_MAX_S 0.01
#define TOL_GLOBAL_S 0.05

static double worst[3];

static int near(int k, double a, double b, double tol_s)
{
    double d = fabs(a - b) * 86400.0;
    if (d > worst[k]) worst[k] = d;
    return d <= tol_s;
}

int main(void)
{
    const char *kernel = getenv("JME_TEST_JPL_KERNEL");
    char error[256] = "";
    int i, failures = 0;

    if (kernel == 0 || kernel[0] == '\0' || !jme_jpl_is_available()) {
        printf("skipped: needs CALCEPH and JME_TEST_JPL_KERNEL\n");
        return 77;
    }
    if (jme_jpl_open(kernel, error) != JME_OK
        || jme_set_astro_models_checked("IAU2006A;ENGINE=JPL;JPL_CENTER=BARYCENTER", error) != JME_OK) {
        fprintf(stderr, "setup failed: %s\n", error);
        return 1;
    }
    for (i = 0; i < N(occult_local_fixtures); i++) {
        const jme_occult_local_fixture *f = &occult_local_fixtures[i];
        double geopos[3] = {f->lon, f->lat, f->height}, tret[10], attr[20];
        int ok;
        jme_set_delta_t_userdef(f->delta_t);
        if (jme_lun_occult_when_loc(f->jd_start, f->body, f->star, 0, geopos, tret, attr, 0, error) != JME_OK) {
            fprintf(stderr, "local %d %s (%.4f, %.4f): %s\n", f->body, f->star ? f->star : "", f->lon, f->lat, error);
            failures++;
            continue;
        }
        ok = near(0, tret[2], f->c1, TOL_CONTACT_S) & near(0, tret[3], f->c4, TOL_CONTACT_S)
            & near(1, tret[0], f->tmax, TOL_MAX_S);
        if (f->i1 != 0.0) ok &= near(0, tret[4], f->i1, TOL_CONTACT_S) & near(0, tret[5], f->i4, TOL_CONTACT_S);
        if (!ok || geopos[0] != f->lon || geopos[1] != f->lat || geopos[2] != f->height) {
            fprintf(stderr, "local %d %s (%.4f, %.4f): max %+.4f s c1 %+.4f s c4 %+.4f s i1 %+.4f s i4 %+.4f s\n",
                f->body, f->star ? f->star : "", f->lon, f->lat,
                (tret[0] - f->tmax) * 86400.0, (tret[2] - f->c1) * 86400.0, (tret[3] - f->c4) * 86400.0,
                f->i1 != 0.0 ? (tret[4] - f->i1) * 86400.0 : 0.0, f->i1 != 0.0 ? (tret[5] - f->i4) * 86400.0 : 0.0);
            failures++;
        }
    }
    for (i = 0; i < N(occult_glob_fixtures); i++) {
        const jme_occult_glob_fixture *f = &occult_glob_fixtures[i];
        double tret[10];
        jme_set_delta_t_userdef(f->delta_t);
        if (jme_lun_occult_when_glob(f->jd_start, f->body, f->star, 0, 0, tret, 0, error) != JME_OK) {
            fprintf(stderr, "global %d %s: %s\n", f->body, f->star ? f->star : "", error);
            failures++;
            continue;
        }
        if (!(near(2, tret[2], f->p1, TOL_GLOBAL_S) & near(2, tret[3], f->p4, TOL_GLOBAL_S))) {
            fprintf(stderr, "global %d %s: P1 %+.4f s P4 %+.4f s\n", f->body, f->star ? f->star : "",
                (tret[2] - f->p1) * 86400.0, (tret[3] - f->p4) * 86400.0);
            failures++;
        }
    }
    jme_close();
    printf("occult_reference: %d local, %d global; worst contact %.5f s, maximum %.5f s, global %.5f s\n",
        N(occult_local_fixtures), N(occult_glob_fixtures), worst[0], worst[1], worst[2]);
    return failures == 0 ? 0 : 1;
}
