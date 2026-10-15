/*
 * Certification of the JPL (CALCEPH) apparent-place path against independent
 * references (tools/gen_apparent_fixtures.py): Skyfield GCRS apparent places
 * and the ERFA IAU 2006/2000A bias-precession-nutation matrix, from the same
 * DE440s kernel. Requires JME_TEST_JPL_KERNEL=<de440s.bsp> and CALCEPH;
 * otherwise the test reports "skipped" (exit 77).
 */
#include "jme/jme.h"
#include "jme/jme_extended.h"
#include "fixtures/apparent_skyfield.h"

#include <math.h>
#include <stdio.h>
#include <stdlib.h>

#define N(a) ((int)(sizeof(a) / sizeof((a)[0])))

/* Tolerances (milliarcseconds). Skyfield and JME/ERFA differ in the
 * deflector-epoch details and Skyfield uses its own TDB-TT series (micro-arcsecond level). */
#define TOL_MAS 0.02
#define TOL_AU  1.0e-11

static double max_mas[4];

static double sep_mas(double a1, double d1, double a2, double d2)
{
    double da = fmod(a1 - a2 + 540.0, 360.0) - 180.0;
    return hypot(da * cos(d2 * 0.017453292519943295769236907684886), d1 - d2) * 3.6e6;
}

static int check(
    int k,
    const char *what,
    const jme_apparent_fixture *f,
    int flags,
    const double *ref,
    int with_dist
)
{
    double r[6];
    char error[256] = "";
    double s;
    if (jme_calc(f->jd_tt, f->body, flags, r, error) != JME_OK) {
        fprintf(stderr, "%s body %d JD %.6f: %s\n", what, f->body, f->jd_tt, error);
        return 1;
    }
    s = sep_mas(r[0], r[1], ref[0], ref[1]);
    if (s > max_mas[k]) {
        max_mas[k] = s;
    }
    if (!(s <= TOL_MAS) || (with_dist && !(fabs(r[2] - ref[2]) <= TOL_AU))) {
        fprintf(
            stderr,
            "%s body %d JD %.6f: offset %.4f mas, dist diff %.3e au\n",
            what,
            f->body,
            f->jd_tt,
            s,
            with_dist ? r[2] - ref[2] : 0.0
        );
        return 1;
    }
    return 0;
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
    if (jme_jpl_open(kernel, error) != JME_OK ||
        jme_set_astro_models_checked("IAU2006A;ENGINE=JPL", error) != JME_OK) {
        fprintf(stderr, "setup failed: %s\n", error);
        return 1;
    }
    for (i = 0; i < N(apparent_fixtures); i++) {
        const jme_apparent_fixture *f = &apparent_fixtures[i];
        failures += check(0, "icrs", f, JME_CALC_EQUATORIAL | JME_CALC_ICRS, f->gcrs, 1);
        failures += check(1, "j2000", f, JME_CALC_EQUATORIAL | JME_CALC_J2000, f->j2000, 0);
        failures += check(2, "date", f, JME_CALC_EQUATORIAL, f->date, 0);
        failures += check(3, "ecliptic", f, JME_CALC_NONE, f->ecl, 0);
    }
    jme_close();
    printf(
        "apparent_reference: %d places; max offsets icrs %.4f j2000 %.4f date %.4f ecliptic "
        "%.4f mas\n",
        N(apparent_fixtures),
        max_mas[0],
        max_mas[1],
        max_mas[2],
        max_mas[3]
    );
    return failures == 0 ? 0 : 1;
}
