/*
 * Certification of the lunar-eclipse shadow model against the NASA Five
 * Millennium Catalog of Lunar Eclipses (Espenak & Meeus 2009), all 228
 * eclipses 2001-2100: TD of greatest eclipse (catalog rounds to 1 s) and
 * umbral/penumbral magnitudes (4 decimals).
 *
 * The shadow at the Moon was cast by the Earth one Earth-Moon light time
 * before the Moon is seen, so relative to the astrometric Moon the axis
 * is displaced by 2 tau v_earth (about 41 arcsec). Apparent Moon minus
 * negated apparent Sun reproduces this to first order in v/c. A model
 * without this retardation misses the catalog times by about 78 s, which
 * the tolerance below rejects.
 */
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include "jme/jme.h"
#include "jme/jme_extended.h"
#include "fixtures/lunar_eclipse_canon.h"

#define N(a)       ((int)(sizeof(a) / sizeof((a)[0])))
#define TOL_TIME_S 3.0
#define TOL_MAG    0.0015

int main(void)
{
    const char *kernel = getenv("JME_TEST_JPL_KERNEL");
    char error[256] = "";
    int i, failures = 0;
    double wt = 0.0, wu = 0.0, wp = 0.0;

    if (kernel == 0 || kernel[0] == '\0' || !jme_jpl_is_available()) {
        printf("skipped: needs CALCEPH and JME_TEST_JPL_KERNEL\n");
        return 77;
    }
    if (jme_jpl_open(kernel, error) != JME_OK ||
        jme_set_astro_models_checked("IAU2006A;ENGINE=JPL", error) != JME_OK) {
        fprintf(stderr, "setup failed: %s\n", error);
        return 1;
    }
    for (i = 0; i < N(lunar_canon_fixtures); i++) {
        const jme_lunar_canon_fixture *f = &lunar_canon_fixtures[i];
        double td = jme_julian_day(
            f->y,
            f->mo,
            f->d,
            f->h + f->mi / 60.0 + f->s / 3600.0,
            JME_CALENDAR_GREGORIAN
        );
        double tret[10] = {0}, attr[20] = {0}, geopos[3] = {0, 0, 0};
        double dt_s, du, dp;

        if (jme_lun_eclipse_when(td - 2.0, 0, 0, tret, 0, error) < 0 ||
            jme_lun_eclipse_how(tret[0], 0, geopos, attr, error) < 0) {
            fprintf(stderr, "%d-%02d-%02d: %s\n", f->y, f->mo, f->d, error);
            failures++;
            continue;
        }
        dt_s = (tret[0] - td) * 86400.0 + jme_delta_t(tret[0]);
        du = attr[0] - f->umb_mag;
        dp = attr[1] - f->pen_mag;
        if (fabs(dt_s) > wt) {
            wt = fabs(dt_s);
        }
        if (fabs(du) > wu) {
            wu = fabs(du);
        }
        if (fabs(dp) > wp) {
            wp = fabs(dp);
        }
        if (!(fabs(dt_s) <= TOL_TIME_S && fabs(du) <= TOL_MAG && fabs(dp) <= TOL_MAG)) {
            fprintf(
                stderr,
                "%d-%02d-%02d %c: dt=%+.2f s dU=%+.5f dP=%+.5f\n",
                f->y,
                f->mo,
                f->d,
                f->type,
                dt_s,
                du,
                dp
            );
            failures++;
        }
    }
    printf(
        "lunar canon: %d eclipses, worst |dt| %.2f s, |dU| %.5f, |dP| %.5f, failures %d\n",
        N(lunar_canon_fixtures),
        wt,
        wu,
        wp,
        failures
    );
    return failures == 0 ? 0 : 1;
}
