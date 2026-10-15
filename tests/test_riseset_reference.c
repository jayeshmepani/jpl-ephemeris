/*
 * Certification of jme_rise_trans() (rise, set, upper and lower transit) on
 * the JPL (CALCEPH) path against Skyfield (tools/gen_riseset_fixtures.py):
 * DE440s, WGS84 observers incl. the date line and 78 deg N, disc centre at
 * geometric altitude 0 without refraction, identical fixed Delta-T.
 * Requires JME_TEST_JPL_KERNEL=<de440s.bsp> and CALCEPH, else skipped (77).
 */
#include "jme/jme.h"
#include "jme/jme_extended.h"
#include "fixtures/riseset_skyfield.h"

#include <math.h>
#include <stdio.h>
#include <stdlib.h>

#define N(a) ((int)(sizeof(a) / sizeof((a)[0])))
/* Tolerance in seconds: JME refines to 1e-10 day, the reference to 1e-11 day;
 * worst observed 0.16 ms (slow polar rates dominate). */
#define TOL_S 0.001

int main(void)
{
    const char *kernel = getenv("JME_TEST_JPL_KERNEL");
    char error[256] = "";
    int i, failures = 0;
    double worst = 0.0;

    if (kernel == 0 || kernel[0] == '\0' || !jme_jpl_is_available()) {
        printf("skipped: needs CALCEPH and JME_TEST_JPL_KERNEL\n");
        return 77;
    }
    if (jme_jpl_open(kernel, error) != JME_OK ||
        jme_set_astro_models_checked("IAU2006A;ENGINE=JPL", error) != JME_OK) {
        fprintf(stderr, "setup failed: %s\n", error);
        return 1;
    }
    for (i = 0; i < N(riseset_fixtures); i++) {
        const jme_riseset_fixture *f = &riseset_fixtures[i];
        double geopos[3] = {f->lon, f->lat, f->height};
        double tret[10] = {0};
        double topo_before[3], topo_after[3];
        int transit = (f->rsmi & (JME_RISE_MERIDIAN_TRANSIT | JME_RISE_ANTI_MERIDIAN_TRANSIT)) != 0;
        int rsmi = transit ? f->rsmi : (f->rsmi | JME_RISE_DISC_CENTER | JME_RISE_NO_REFRACTION);
        int flags = transit ? JME_CALC_TOPOCENTRIC : 0;
        double d;

        jme_set_topo(10.0, 20.0, 30.0);
        jme_set_delta_t_userdef(f->delta_t);
        jme_get_topo_pos(2451545.0, topo_before, error);
        if (jme_rise_trans(f->jd_start, f->body, 0, flags, rsmi, geopos, 0.0, 0.0, tret, error) !=
            JME_OK) {
            fprintf(
                stderr,
                "case %d body %d rsmi %d (%.4f, %.4f): %s\n",
                i,
                f->body,
                f->rsmi,
                f->lon,
                f->lat,
                error
            );
            failures++;
            continue;
        }
        jme_get_topo_pos(2451545.0, topo_after, error);
        if (topo_before[0] != topo_after[0] || topo_before[1] != topo_after[1] ||
            topo_before[2] != topo_after[2]) {
            fprintf(stderr, "case %d: jme_rise_trans modified the caller's topocentre\n", i);
            failures++;
        }
        d = fabs(tret[0] - f->jd_event) * 86400.0;
        if (d > worst) {
            worst = d;
        }
        if (!(d <= TOL_S)) {
            fprintf(
                stderr,
                "case %d body %d rsmi %d (%.4f, %.4f): JME %.9f ref %.9f diff %.4f s\n",
                i,
                f->body,
                f->rsmi,
                f->lon,
                f->lat,
                tret[0],
                f->jd_event,
                (tret[0] - f->jd_event) * 86400.0
            );
            failures++;
        }
    }
    jme_set_delta_t_userdef(NAN);
    jme_close();
    printf("riseset_reference: %d events, worst |diff| %.5f s\n", N(riseset_fixtures), worst);
    return failures == 0 ? 0 : 1;
}
