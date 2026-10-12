/*
 * Galactic Centre 0 Sagittarius ayanamsa against independent ERFA values
 * (tests/fixtures/galcent_reference.h, tools/gen_galcent_fixtures.py).
 */
#include <math.h>
#include <stdio.h>
#include "jme/jme.h"
#include "jme/jme_extended.h"
#include "fixtures/galcent_reference.h"

static int check(int iau2006)
{
    char error[256] = {0};
    int failures = 0;
    size_t k;
    jme_set_astro_models(iau2006 ? "IAU2006" : "DEFAULT", 0);
    for (k = 0; k < sizeof(galcent_reference) / sizeof(galcent_reference[0]); k++) {
        double got = NAN;
        double want = iau2006 ? galcent_reference[k].ayan_iau2006 : galcent_reference[k].ayan_default;
        double diff;
        if (jme_get_ayanamsa_ex(galcent_reference[k].jd, JME_SIDEREAL_GALCENT_0SAG, &got, error) != JME_OK) {
            fprintf(stderr, "galcent %s jd %.8f failed: %s\n", iau2006 ? "IAU2006" : "default",
                galcent_reference[k].jd, error);
            failures++;
            continue;
        }
        diff = fabs(jme_degrees_difference_signed(got, want)) * 3600.0;
        if (!(diff <= 1.0e-6)) {
            fprintf(stderr, "galcent %s jd %.8f: got %.12f want %.12f (%.3e arcsec)\n",
                iau2006 ? "IAU2006" : "default", galcent_reference[k].jd, got, want, diff);
            failures++;
        }
    }
    jme_set_astro_models(0, 0);
    return failures;
}

int main(void)
{
    int failures = check(0) + check(1);
    if (failures == 0) printf("galcent_reference: all checks passed\n");
    return failures == 0 ? 0 : 1;
}
