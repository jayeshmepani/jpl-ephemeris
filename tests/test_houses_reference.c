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

/*
 * Gauquelin 36 sectors: every cusp k must lie exactly at the start of
 * sector k, evaluated independently from the hour angle and the semi-arcs
 * of the ecliptic point (the definition used by jme_gauquelin_sector), and
 * the cusps at multiples of 1/3 quadrant must equal the Placidus cusps.
 */
static int check_gauquelin(double *worst_sector, int *count)
{
    static const int gq[8] = {4, 7, 13, 16, 22, 25, 31, 34};
    static const int pl[8] = {12, 11, 9, 8, 6, 5, 3, 2};
    const double d2r = 0.017453292519943295769;
    char error[256] = "";
    int failures = 0, a, b, k;
    for (a = 0; a < 72; a++) {
        for (b = 0; b < 13; b++) {
            double armc = a * 5.0 + 0.37, lat = -60.0 + b * 10.0, eps = 23.4392911;
            double g[37], p[13], ascmc[10];
            if (jme_houses_gauquelin_armc(armc, lat, eps, g, ascmc, error) != JME_OK ||
                jme_houses_armc(armc, lat, eps, JME_HOUSE_PLACIDUS, p, 0) != JME_OK) {
                fprintf(stderr, "gauquelin failed armc %.2f lat %.1f: %s\n", armc, lat, error);
                failures++;
                continue;
            }
            (*count)++;
            for (k = 1; k <= 36; k++) {
                double l = g[k] * d2r, e = eps * d2r;
                double ra = atan2(sin(l) * cos(e), cos(l)) / d2r;
                double dec = asin(sin(l) * sin(e));
                double h = jme_degrees_difference_signed(armc, ra);
                double h0 = acos(-tan(lat * d2r) * tan(dec)) / d2r;
                double sector, err;
                if (k <= 18) {
                    sector = 10.0 + 9.0 * h / h0;
                } else if (h >= 0.0) {
                    sector = 19.0 + 9.0 * (h - h0) / (180.0 - h0);
                } else {
                    sector = 28.0 + 9.0 * (h + 180.0) / (180.0 - h0);
                }
                err = fabs(sector - k);
                if (err > *worst_sector) {
                    *worst_sector = err;
                }
                if (err > 1e-9) {
                    fprintf(
                        stderr,
                        "gauquelin armc %.2f lat %.1f cusp %d at sector %.12f\n",
                        armc,
                        lat,
                        k,
                        sector
                    );
                    failures++;
                }
            }
            for (k = 0; k < 8; k++) {
                if (adiff(g[gq[k]], p[pl[k]]) > 1e-11) {
                    fprintf(
                        stderr,
                        "gauquelin cusp %d %.12f != placidus %d %.12f\n",
                        gq[k],
                        g[gq[k]],
                        pl[k],
                        p[pl[k]]
                    );
                    failures++;
                }
            }
            if (adiff(g[1], ascmc[0]) > 0.0 || adiff(g[10], ascmc[1]) > 0.0) {
                failures++;
            }
        }
    }
    {
        double g[37];
        if (jme_houses_gauquelin_armc(10.0, 80.0, 23.44, g, 0, error) == JME_OK) {
            fprintf(stderr, "gauquelin at latitude 80 must fail (circumpolar ecliptic points)\n");
            failures++;
        }
        if (jme_houses_gauquelin_armc(10.0, 40.0, 23.44, 0, 0, error) == JME_OK) {
            failures++;
        }
        if (jme_houses(2460000.5, 40.0, 10.0, JME_HOUSE_PLACIDUS, g, 0) != JME_OK) {
            failures++;
        }
        if (jme_houses_gauquelin(2460000.5, 0, 40.0, 10.0, g, 0, error) != JME_OK) {
            fprintf(stderr, "gauquelin jd: %s\n", error);
            failures++;
        }
    }
    return failures;
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
            fprintf(
                stderr,
                "houses_armc failed hsys %d armc %.9f lat %.9f eps %.7f\n",
                f->hsys,
                f->armc,
                f->lat,
                f->eps
            );
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
                if (fabs(fabs(s) - 30.0) < 1e-9) {
                    ws_shift = s;
                }
            }
        }
        for (k = 0; k < 12; k++) {
            double d = adiff(cusps[k + 1] - ws_shift, f->cusp[k]);
            if (d > worst) {
                worst = d;
            }
            if (!(d <= 1.0e-9)) {
                fprintf(
                    stderr,
                    "hsys %d armc %.9f lat %.9f eps %.7f cusp %d: JME %.12f ref %.12f\n",
                    f->hsys,
                    f->armc,
                    f->lat,
                    f->eps,
                    k + 1,
                    cusps[k + 1],
                    f->cusp[k]
                );
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
        if (fabs(ascmc[7] - f->eps) > d) {
            d = fabs(ascmc[7] - f->eps);
        }
        if (adiff(ascmc[0], f->asc) > d) {
            d = adiff(ascmc[0], f->asc);
        }
        if (adiff(ascmc[1], f->mc) > d) {
            d = adiff(ascmc[1], f->mc);
        }
        if (d > worst_t) {
            worst_t = d;
        }
        /* 1e-8 deg = 36 microarcsec: equinox-based GAST (gmst06 + ee) vs ERFA gst06a (CIO-based) */
        if (!(d <= 1.0e-8)) {
            fprintf(
                stderr,
                "time chain JD %.6f lon %.4f: ramc %.12f/%.12f asc %.12f/%.12f mc %.12f/%.12f\n",
                f->jd_ut,
                f->lon,
                ascmc[6],
                f->armc,
                ascmc[0],
                f->asc,
                ascmc[1],
                f->mc
            );
            failures++;
        }
    }
    jme_set_delta_t_userdef(NAN);
    {
        double worst_g = 0.0;
        int count_g = 0;
        failures += check_gauquelin(&worst_g, &count_g);
        printf("gauquelin: %d sector sets, worst sector residual %.3g\n", count_g, worst_g);
    }
    printf(
        "houses_reference: %d cusp sets (worst %.3g deg), %d time-chain cases (worst %.3g deg)\n",
        N(house_fixtures),
        worst,
        N(house_time_fixtures),
        worst_t
    );
    return failures == 0 ? 0 : 1;
}
