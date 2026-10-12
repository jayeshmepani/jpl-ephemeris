/*
 * Laskar 1986 precession (JME_MODEL_PREC_LASKAR_1986) against an
 * independent Python evaluation (tools/gen_laskar1986_fixtures.py) at 12
 * epochs from -5000 to +5000 yr, and against IAU 1976 near J2000: the two
 * share the general precession rate 5029.0966"/cy.
 */
#include <math.h>
#include <stdio.h>
#include "jme/jme.h"
#include "jme/jme_extended.h"
#include "fixtures/laskar1986_reference.h"

#define N(a) ((int)(sizeof(a) / sizeof((a)[0])))

static double rotation_angle_arcsec(const double *a, const double *b)
{
    /* angle of R = a b^T from its antisymmetric part (well conditioned for small angles) */
    double r[9], x, y, z;
    int i, j, k;
    for (i = 0; i < 3; i++) for (j = 0; j < 3; j++) {
        r[i * 3 + j] = 0.0;
        for (k = 0; k < 3; k++) r[i * 3 + j] += a[i * 3 + k] * b[j * 3 + k];
    }
    x = 0.5 * (r[5] - r[7]);
    y = 0.5 * (r[6] - r[2]);
    z = 0.5 * (r[1] - r[3]);
    return asin(sqrt(x * x + y * y + z * z)) * 206264.80624709636;
}

int main(void)
{
    int i, k, failures = 0;
    double worst = 0.0, m[9], m76[9];
    const double years[] = { -100.0, 100.0 };

    for (i = 0; i < N(laskar_fixtures); i++) {
        if (jme_get_precession_matrix(2451545.0, laskar_fixtures[i].jd, JME_MODEL_PREC_LASKAR_1986, m) != JME_OK) {
            fprintf(stderr, "Laskar 1986 precession refused at JD %.1f\n", laskar_fixtures[i].jd);
            return 1;
        }
        for (k = 0; k < 9; k++) {
            double d = fabs(m[k] - laskar_fixtures[i].m[k]);
            if (d > worst) worst = d;
        }
    }
    if (!(worst <= 2.0e-15)) {
        fprintf(stderr, "Laskar 1986 matrix differs from the reference by %.3e\n", worst);
        failures++;
    }
    for (i = 0; i < 2; i++) {
        double jd = 2451545.0 + 365.25 * years[i], ang;
        jme_get_precession_matrix(2451545.0, jd, JME_MODEL_PREC_LASKAR_1986, m);
        jme_get_precession_matrix(2451545.0, jd, JME_MODEL_PREC_IAU_1976, m76);
        ang = rotation_angle_arcsec(m, m76);
        printf("Laskar 1986 vs IAU 1976 at %+.0f yr: %.4f arcsec\n", years[i], ang);
        if (!(ang < 0.01)) failures++;
    }
    printf("Laskar 1986: %d epochs, worst element difference %.3e\n", N(laskar_fixtures), worst);
    return failures == 0 ? 0 : 1;
}
