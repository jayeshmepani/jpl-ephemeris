#include "jme/jme.h"
#include "moshier/plantbl.h"
#include <math.h>

#define JME_MOSHIER_NARGS 18
#define JME_MOSHIER_MAX_HARMONIC 31
#define JME_MOSHIER_MODS3600(x) ((x) - 1296000.0 * floor((x) / 1296000.0))

extern struct plantbl moonlr;
extern struct plantbl moonlat;

static double g_args[JME_MOSHIER_NARGS];
static double g_ss[JME_MOSHIER_NARGS][JME_MOSHIER_MAX_HARMONIC];
static double g_cc[JME_MOSHIER_NARGS][JME_MOSHIER_MAX_HARMONIC];

static void moshier_sscc(int k, double arg, int n)
{
    double su = sin(arg);
    double cu = cos(arg);
    double sv;
    double cv;
    double s;
    int i;

    if (k < 0 || k >= JME_MOSHIER_NARGS || n <= 0) {
        return;
    }
    if (n > JME_MOSHIER_MAX_HARMONIC) {
        n = JME_MOSHIER_MAX_HARMONIC;
    }

    g_ss[k][0] = su;
    g_cc[k][0] = cu;
    if (n == 1) {
        return;
    }

    sv = 2.0 * su * cu;
    cv = cu * cu - su * su;
    g_ss[k][1] = sv;
    g_cc[k][1] = cv;

    for (i = 2; i < n; i++) {
        s = su * cv + cu * sv;
        cv = cu * cv - su * sv;
        sv = s;
        g_ss[k][i] = sv;
        g_cc[k][i] = cv;
    }
}

static void moshier_mean_elements(double jd)
{
    double x;
    double t = (jd - 2451545.0) / 36525.0;
    double t2 = t * t;

    x = JME_MOSHIER_MODS3600(538101628.6889819 * t + 908103.213);
    x += (6.39e-6 * t - 0.0192789) * t2;
    g_args[0] = STR * x;

    x = JME_MOSHIER_MODS3600(210664136.4335482 * t + 655127.236);
    x += (-6.27e-6 * t + 0.0059381) * t2;
    g_args[1] = STR * x;

    x = JME_MOSHIER_MODS3600(129597742.283429 * t + 361679.198);
    x += (-5.23e-6 * t - 2.04411e-2) * t2;
    g_args[2] = STR * x;

    x = JME_MOSHIER_MODS3600(68905077.493988 * t + 1279558.751);
    x += (-1.043e-5 * t + 0.0094264) * t2;
    g_args[3] = STR * x;

    x = JME_MOSHIER_MODS3600(10925660.377991 * t + 123665.420);
    x += ((((-3.4e-10 * t + 5.91e-8) * t + 4.667e-6) * t + 5.706e-5) * t - 3.060378e-1) * t2;
    g_args[4] = STR * x;

    x = JME_MOSHIER_MODS3600(4399609.855372 * t + 180278.752);
    x += ((((8.3e-10 * t - 1.452e-7) * t - 1.1484e-5) * t - 1.6618e-4) * t + 7.561614e-1) * t2;
    g_args[5] = STR * x;

    x = JME_MOSHIER_MODS3600(1542481.193933 * t + 1130597.971) + (0.00002156 * t - 0.0175083) * t2;
    g_args[6] = STR * x;

    x = JME_MOSHIER_MODS3600(786550.320744 * t + 1095655.149) + (-0.00000895 * t + 0.0021103) * t2;
    g_args[7] = STR * x;

    g_args[8] = 0.0;

    x = JME_MOSHIER_MODS3600(1.6029616009939659e9 * t + 1.0722612202445078e6);
    x += (((((-3.207663637426e-13 * t + 2.555243317839e-11) * t + 2.560078201452e-9) * t - 3.702060118571e-5) * t + 6.9492746836058421e-3) * t - 6.7352202374457519) * t2;
    g_args[9] = STR * x;

    x = JME_MOSHIER_MODS3600(1.7395272628437717e9 * t + 3.3577951412884740e5);
    x += (((((4.474984866301e-13 * t + 4.189032191814e-11) * t - 2.790392351314e-9) * t - 2.165750777942e-6) * t - 7.5311878482337989e-4) * t - 1.3117809789650071e1) * t2;
    g_args[10] = STR * x;

    x = JME_MOSHIER_MODS3600(1.2959658102304320e8 * t + 1.2871027407441526e6);
    x += ((((((((1.62e-20 * t - 1.0390e-17) * t - 3.83508e-15) * t + 4.237343e-13) * t + 8.8555011e-11) * t - 4.77258489e-8) * t - 1.1297037031e-5) * t + 8.7473717367324703e-5) * t - 5.5281306421783094e-1) * t2;
    g_args[11] = STR * x;

    x = JME_MOSHIER_MODS3600(1.7179159228846793e9 * t + 4.8586817465825332e5);
    x += (((((-1.755312760154e-12 * t + 3.452144225877e-11) * t - 2.506365935364e-8) * t - 2.536291235258e-4) * t + 5.2099641302735818e-2) * t + 3.1501359071894147e1) * t2;
    g_args[12] = STR * x;

    x = JME_MOSHIER_MODS3600(1.7325643720442266e9 * t + 7.8593980921052420e5);
    x += (((((7.200592540556e-14 * t + 2.235210987108e-10) * t - 1.024222633731e-8) * t - 6.073960534117e-5) * t + 6.9017248528380490e-3) * t - 5.6550460027471399) * t2;
    g_args[13] = STR * x;

    x = JME_MOSHIER_MODS3600(4.48175409e7 * t + 8.060457e5);
    g_args[14] = STR * x;

    x = JME_MOSHIER_MODS3600(5.36486787e6 * t - 391702.8);
    g_args[15] = STR * x;

    g_args[16] = 0.0;

    x = JME_MOSHIER_MODS3600(1.73573e6 * t);
    g_args[17] = STR * x;
}

static double moshier_g1plan(double jd, struct plantbl *plan)
{
    int i;
    int j;
    int k;
    int m;
    int k1;
    int ip;
    int np;
    int nt;
    signed char *p;
    long *pl;
    double su;
    double cu;
    double sv;
    double cv;
    double t;
    double sl = 0.0;
    double tt;

    moshier_mean_elements(jd);
    tt = (jd - J2000) / plan->timescale;

    for (i = 0; i < plan->maxargs && i < JME_MOSHIER_NARGS; i++) {
        if ((j = plan->max_harmonic[i]) > 0) {
            moshier_sscc(i, g_args[i], j);
        }
    }

    p = (signed char *) plan->arg_tbl;
    pl = (long *) plan->lon_tbl;

    for (;;) {
        np = *p++;
        if (np < 0) {
            break;
        }
        if (np == 0) {
            nt = *p++;
            cu = *pl++;
            for (ip = 0; ip < nt; ip++) {
                cu = cu * tt + *pl++;
            }
            sl += cu;
            continue;
        }

        k1 = 0;
        cv = 0.0;
        sv = 0.0;
        for (ip = 0; ip < np; ip++) {
            j = *p++;
            m = *p++ - 1;
            if (j != 0 && m >= 0 && m < JME_MOSHIER_NARGS) {
                k = j < 0 ? -j : j;
                k -= 1;
                if (k >= 0 && k < JME_MOSHIER_MAX_HARMONIC) {
                    su = g_ss[m][k];
                    if (j < 0) {
                        su = -su;
                    }
                    cu = g_cc[m][k];
                    if (k1 == 0) {
                        sv = su;
                        cv = cu;
                        k1 = 1;
                    } else {
                        t = su * cv + cu * sv;
                        cv = cu * cv - su * sv;
                        sv = t;
                    }
                }
            }
        }

        nt = *p++;
        cu = *pl++;
        su = *pl++;
        for (ip = 0; ip < nt; ip++) {
            cu = cu * tt + *pl++;
            su = su * tt + *pl++;
        }
        sl += cu * cv + su * sv;
    }

    return plan->trunclvl * sl;
}

static int moshier_g2plan(double jd, struct plantbl *plan, double *pobj)
{
    int i;
    int j;
    int k;
    int m;
    int k1;
    int ip;
    int np;
    int nt;
    signed char *p;
    long *pl;
    long *pr;
    double su;
    double cu;
    double sv;
    double cv;
    double t;
    double sl = 0.0;
    double sr = 0.0;
    double tt;

    moshier_mean_elements(jd);
    tt = (jd - J2000) / plan->timescale;

    for (i = 0; i < plan->maxargs && i < JME_MOSHIER_NARGS; i++) {
        if ((j = plan->max_harmonic[i]) > 0) {
            moshier_sscc(i, g_args[i], j);
        }
    }

    p = (signed char *) plan->arg_tbl;
    pl = (long *) plan->lon_tbl;
    pr = (long *) plan->rad_tbl;

    for (;;) {
        np = *p++;
        if (np < 0) {
            break;
        }
        if (np == 0) {
            nt = *p++;
            cu = *pl++;
            for (ip = 0; ip < nt; ip++) {
                cu = cu * tt + *pl++;
            }
            sl += cu;

            cu = *pr++;
            for (ip = 0; ip < nt; ip++) {
                cu = cu * tt + *pr++;
            }
            sr += cu;
            continue;
        }

        k1 = 0;
        cv = 0.0;
        sv = 0.0;
        for (ip = 0; ip < np; ip++) {
            j = *p++;
            m = *p++ - 1;
            if (j != 0 && m >= 0 && m < JME_MOSHIER_NARGS) {
                k = j < 0 ? -j : j;
                k -= 1;
                if (k >= 0 && k < JME_MOSHIER_MAX_HARMONIC) {
                    su = g_ss[m][k];
                    if (j < 0) {
                        su = -su;
                    }
                    cu = g_cc[m][k];
                    if (k1 == 0) {
                        sv = su;
                        cv = cu;
                        k1 = 1;
                    } else {
                        t = su * cv + cu * sv;
                        cv = cu * cv - su * sv;
                        sv = t;
                    }
                }
            }
        }

        nt = *p++;
        cu = *pl++;
        su = *pl++;
        for (ip = 0; ip < nt; ip++) {
            cu = cu * tt + *pl++;
            su = su * tt + *pl++;
        }
        sl += cu * cv + su * sv;

        cu = *pr++;
        su = *pr++;
        for (ip = 0; ip < nt; ip++) {
            cu = cu * tt + *pr++;
            su = su * tt + *pr++;
        }
        sr += cu * cv + su * sv;
    }

    pobj[0] = plan->trunclvl * sl;
    pobj[2] = plan->trunclvl * sr;
    return 0;
}

static int moshier_moon_position(double jd_et, double *results)
{
    double pol[3];
    double lon_arcsec;
    double lon;
    double lat;
    double radius;
    double cos_lat;

    if (results == 0) {
        return JME_ERR;
    }

    moshier_g2plan(jd_et, &moonlr, pol);
    lon_arcsec = pol[0] + (g_args[13] / STR);
    if (lon_arcsec < -648000.0) {
        lon_arcsec += 1296000.0;
    }
    if (lon_arcsec > 648000.0) {
        lon_arcsec -= 1296000.0;
    }

    lon = STR * lon_arcsec;
    lat = STR * moshier_g1plan(jd_et, &moonlat);
    radius = (1.0 + STR * pol[2]) * moonlr.distance;

    if (!isfinite(lon) || !isfinite(lat) || !isfinite(radius) || radius <= 0.0) {
        return JME_ERR;
    }

    cos_lat = cos(lat);
    results[0] = radius * cos_lat * cos(lon);
    results[1] = radius * cos_lat * sin(lon);
    results[2] = radius * sin(lat);
    results[3] = 0.0;
    results[4] = 0.0;
    results[5] = 0.0;

    return JME_OK;
}

#if defined(__GNUC__)
__attribute__((visibility("hidden")))
#endif
int jme_internal_moshier_moon_position(double jd_et, double *results)
{
    int i;

    if (results == 0) {
        return JME_ERR;
    }

    for (i = 0; i < 6; i++) {
        results[i] = 0.0;
    }

    return moshier_moon_position(jd_et, results);
}

#if defined(__GNUC__)
__attribute__((visibility("hidden")))
#endif
int jme_internal_moshier_moon_state(double jd_et, double *results)
{
    double prev[6];
    double next[6];
    const double step = 1.0e-3;
    int i;

    if (results == 0) {
        return JME_ERR;
    }

    for (i = 0; i < 6; i++) {
        results[i] = 0.0;
    }

    if (moshier_moon_position(jd_et, results) != JME_OK) {
        return JME_ERR;
    }

    if (moshier_moon_position(jd_et - step, prev) == JME_OK
        && moshier_moon_position(jd_et + step, next) == JME_OK) {
        for (i = 0; i < 3; i++) {
            results[i + 3] = (next[i] - prev[i]) / (2.0 * step);
        }
    }

    return JME_OK;
}
