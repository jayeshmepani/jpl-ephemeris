/*
 * Certification of the house systems against an independent vector-geometry
 * re-implementation of their definitions (tools/gen_houses_fixtures.py), and
 * of the jme_houses() time chain (UT1/TT, apparent sidereal time, true
 * obliquity) against ERFA gst06a / obl06 / nut06a under the IAU2006A models.
 */
#include "jme/jme.h"
#include "jme/jme_extended.h"
#include "fixtures/houses_reference.h"

#include <math.h>
#include <stdio.h>

#define N(a) ((int)(sizeof(a) / sizeof((a)[0])))

static double adiff(double a, double b)
{
    return fabs(jme_degrees_difference_signed(a, b));
}

int main(void)
{
    char error[256] = "";
    int i, k, failures = 0;
    double worst = 0.0, worst_t = 0.0;

    for (i = 0; i < N(house_fixtures); i++) {
        const jme_house_fixture *f = &house_fixtures[i];
        double cusps[13], ascmc[10];
        if (jme_houses_armc(f->armc, f->lat, f->eps, f->hsys, cusps, ascmc) != JME_OK) {
            fprintf(stderr, "houses_armc failed hsys %d armc %.9f lat %.9f eps %.7f\n", f->hsys, f->armc, f->lat, f->eps);
            failures++;
            continue;
        }
        /* Whole sign: when the Ascendant lies within 1e-9 deg of a sign
         * boundary (e.g. ARMC 270, where it is analytically 0), the sign it
         * falls in is decided by the last binary64 bit; either adjacent
         * sign is then a correct result, so a uniform 30 deg shift is
         * accepted in that case only. */
        double ws_shift = 0.0;
        if (f->hsys == JME_HOUSE_WHOLE_SIGN) {
            double b = fmod(ascmc[0], 30.0);
            if (b < 1e-9 || 30.0 - b < 1e-9) {
                double s = jme_degrees_difference_signed(cusps[1], f->cusp[0]);
                if (fabs(fabs(s) - 30.0) < 1e-9) ws_shift = s;
            }
        }
        for (k = 0; k < 12; k++) {
            double d = adiff(cusps[k + 1] - ws_shift, f->cusp[k]);
            if (d > worst) worst = d;
            if (!(d <= 1.0e-9)) {
                fprintf(stderr, "hsys %d armc %.9f lat %.9f eps %.7f cusp %d: JME %.12f ref %.12f\n",
                    f->hsys, f->armc, f->lat, f->eps, k + 1, cusps[k + 1], f->cusp[k]);
                failures++;
                break;
            }
        }
    }

    if (jme_set_astro_models_checked("IAU2006A", error) != JME_OK) {
        fprintf(stderr, "model setup failed: %s\n", error);
        return 1;
    }
    for (i = 0; i < N(house_time_fixtures); i++) {
        const jme_house_time_fixture *f = &house_time_fixtures[i];
        double cusps[13], ascmc[10], d;
        jme_set_delta_t_userdef(f->delta_t);
        if (jme_houses(f->jd_ut, f->lat, f->lon, JME_HOUSE_EQUAL, cusps, ascmc) != JME_OK) {
            fprintf(stderr, "jme_houses failed at %.6f\n", f->jd_ut);
            failures++;
            continue;
        }
        d = adiff(ascmc[6], f->armc);
        if (fabs(ascmc[7] - f->eps) > d) d = fabs(ascmc[7] - f->eps);
        if (adiff(ascmc[0], f->asc) > d) d = adiff(ascmc[0], f->asc);
        if (adiff(ascmc[1], f->mc) > d) d = adiff(ascmc[1], f->mc);
        if (d > worst_t) worst_t = d;
        /* 1e-8 deg = 36 microarcsec: equinox-based GAST (gmst06 + ee) vs ERFA gst06a (CIO-based) */
        if (!(d <= 1.0e-8)) {
            fprintf(stderr, "time chain JD %.6f lon %.4f: ramc %.12f/%.12f asc %.12f/%.12f mc %.12f/%.12f\n",
                f->jd_ut, f->lon, ascmc[6], f->armc, ascmc[0], f->asc, ascmc[1], f->mc);
            failures++;
        }
    }
    jme_set_delta_t_userdef(NAN);
    printf("houses_reference: %d cusp sets (worst %.3g deg), %d time-chain cases (worst %.3g deg)\n",
        N(house_fixtures), worst, N(house_time_fixtures), worst_t);
    return failures == 0 ? 0 : 1;
}
