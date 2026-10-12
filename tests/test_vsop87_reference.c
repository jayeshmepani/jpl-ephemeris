/* VSOP87A against the official IMCCE check values (vsop87.chk): positions
 * and velocities printed to 1e-10 au and 1e-10 au/d. Velocities are the
 * analytic derivative of the series. */
#include "jme/jme.h"
#include "jme/jme_extended.h"
#include <math.h>
#include <stdio.h>
#include "fixtures/vsop87a_check.h"

int main(void)
{
    size_t n = sizeof(vsop87a_check_rows) / sizeof(vsop87a_check_rows[0]);
    size_t k;
    int failures = 0;
    double worst_p = 0.0, worst_v = 0.0;

    for (k = 0; k < n; k++) {
        const vsop87a_check_row *r = &vsop87a_check_rows[k];
        double s[6];
        int i;
        if (jme_vsop87_planet_state(r->jd, r->body, s) != JME_OK) {
            fprintf(stderr, "row %zu: evaluation failed\n", k);
            return 1;
        }
        for (i = 0; i < 3; i++) {
            double dp = fabs(s[i] - r->pos[i]);
            double dv = fabs(s[i + 3] - r->vel[i]);
            if (dp > worst_p) worst_p = dp;
            if (dv > worst_v) worst_v = dv;
            /* printed values are rounded to 10 decimals: |error| <= 5e-11 + FP */
            if (dp > 5.0e-11 + 1e-15 || dv > 5.0e-11 + 1e-15) {
                fprintf(stderr, "row %zu body %d jd %.1f coord %d: dp %.3e dv %.3e\n", k, r->body, r->jd, i, dp, dv);
                failures++;
            }
        }
    }
    printf("VSOP87A check: %zu epochs, worst position %.3e au, worst velocity %.3e au/d\n", n, worst_p, worst_v);
    return failures == 0 ? 0 : 1;
}
