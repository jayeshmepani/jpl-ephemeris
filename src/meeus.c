#include "jme/jme.h"
#include "context.h"
#include <math.h>

#define JME_DEG_TO_RAD 0.017453292519943295769236907684886127134428718885417
#define JME_RAD_TO_DEG 57.295779513082320876798154814105170332405472466565

int jme_meeus_sun_state(double jd_et, double *results)
{
    double t = (jd_et - 2451545.0) / 36525.0;
    double l0, m, e, c, sun_lon, r;
    double spherical[6];

    /* Meeus Chapter 25 */
    l0 = 280.46646 + 36000.76983 * t + 0.0003032 * t * t;
    m = 357.52911 + 35999.05029 * t - 0.0001537 * t * t;
    e = 0.016708634 - 0.000042037 * t - 0.0000001267 * t * t;

    l0 = jme_degree_normalize(l0);
    m = jme_degree_normalize(m);

    c = (1.914602 - 0.004817 * t - 0.000014 * t * t) * sin(m * JME_DEG_TO_RAD)
        + (0.019993 - 0.000101 * t) * sin(2.0 * m * JME_DEG_TO_RAD)
        + 0.000289 * sin(3.0 * m * JME_DEG_TO_RAD);

    sun_lon = l0 + c;
    r = (1.00014061 * (1.0 - e * e)) / (1.0 + e * cos((m + c) * JME_DEG_TO_RAD));

    if (results != 0) {
        spherical[0] = sun_lon;
        spherical[1] = 0.0;
        spherical[2] = r;
        spherical[3] = 0.0;
        spherical[4] = 0.0;
        spherical[5] = 0.0;
        jme_spherical_to_rectangular_state(spherical, results);
    }

    return JME_OK;
}

int jme_meeus_moon_state(double jd_et, double *results)
{
    double t = (jd_et - 2451545.0) / 36525.0;
    double lp, d, m, mp, f, a1, a2, a3, e;
    double sl, sb, sr;

    /* Meeus Chapter 47 */
    lp = 218.3164477 + 481267.88123421 * t - 0.0015786 * t * t + t * t * t / 538841.0 - t * t * t * t / 65194000.0;
    d = 297.8501921 + 445267.1114034 * t - 0.0018819 * t * t + t * t * t / 545868.0 - t * t * t * t / 113065000.0;
    m = 357.5291092 + 35999.0502909 * t - 0.0001536 * t * t + t * t * t / 24490000.0;
    mp = 134.9633964 + 477198.8675055 * t + 0.0087414 * t * t + t * t * t / 69699.0 - t * t * t * t / 14712000.0;
    f = 93.2720950 + 483202.0175233 * t - 0.0036539 * t * t - t * t * t / 3526000.0 + t * t * t * t / 863310000.0;

    a1 = 119.75 + 131.849 * t;
    a2 = 53.09 + 479264.290 * t;
    a3 = 313.45 + 481266.484 * t;

    e = 1.0 - 0.002516 * t - 0.0000074 * t * t;

    lp = jme_degree_normalize(lp);
    d = jme_degree_normalize(d);
    m = jme_degree_normalize(m);
    mp = jme_degree_normalize(mp);
    f = jme_degree_normalize(f);

    /* Sum of most significant terms for Moon */
    sl = lp + 6.288774 * sin(mp * JME_DEG_TO_RAD)
            + 1.274027 * sin((2.0 * d - mp) * JME_DEG_TO_RAD)
            + 0.658311 * sin(2.0 * d * JME_DEG_TO_RAD)
            + 0.213618 * sin(2.0 * mp * JME_DEG_TO_RAD)
            - e * 0.185116 * sin(m * JME_DEG_TO_RAD)
            + 0.114332 * sin(2.0 * f * JME_DEG_TO_RAD);

    sb = 5.128122 * sin(f * JME_DEG_TO_RAD)
            + 0.280602 * sin((mp + f) * JME_DEG_TO_RAD)
            + 0.277693 * sin((mp - f) * JME_DEG_TO_RAD)
            + 0.173237 * sin((2.0 * d - f) * JME_DEG_TO_RAD);

    sr = 385000.56 / 149597870.7  /* distance in AU */
            - 20905.355 / 149597870.7 * cos(mp * JME_DEG_TO_RAD)
            - 3699.111 / 149597870.7 * cos((2.0 * d - mp) * JME_DEG_TO_RAD)
            - 2955.968 / 149597870.7 * cos(2.0 * d * JME_DEG_TO_RAD);

    if (results != 0) {
        double spherical[6];
        spherical[0] = jme_degree_normalize(sl);
        spherical[1] = sb;
        spherical[2] = sr;
        spherical[3] = 0.0;
        spherical[4] = 0.0;
        spherical[5] = 0.0;
        jme_spherical_to_rectangular_state(spherical, results);
    }

    return JME_OK;
}

int jme_meeus_planet_state(double jd_et, int body, double *results)
{
    double t = (jd_et - 2451545.0) / 36525.0;
    double l, a, e, i, om, w, m;

    if (results != 0) {
        int k;
        for (k = 0; k < 6; k++) { results[k] = 0.0; }
    }

    /* Standard orbital elements (J2000) - Meeus Appendix II */
    switch (body) {
    case JME_BODY_MERCURY:
        l = 252.25084 + 149472.67411 * t;
        a = 0.38709893;
        e = 0.20563069 + 0.000020407 * t;
        i = 7.00487 + 0.001866 * t;
        om = 48.33167 - 0.125184 * t;
        w = 77.45645 + 0.15846 * t;
        break;
    case JME_BODY_VENUS:
        l = 181.97973 + 58517.81538 * t;
        a = 0.72333199;
        e = 0.00677323 - 0.000049238 * t;
        i = 3.39471 - 0.000788 * t;
        om = 76.68069 - 0.277694 * t;
        w = 131.53298 + 0.00214 * t;
        break;
    case JME_BODY_MARS:
        l = 355.45332 + 19140.30268 * t;
        a = 1.52366231;
        e = 0.09341233 + 0.000119019 * t;
        i = 1.85061 - 0.001047 * t;
        om = 49.55747 - 0.292573 * t;
        w = 336.04084 + 0.44390 * t;
        break;
    case JME_BODY_JUPITER:
        l = 34.40438 + 3034.74612 * t;
        a = 5.20336301;
        e = 0.04839266 - 0.000128802 * t;
        i = 1.30530 - 0.004117 * t;
        om = 100.55615 - 0.211242 * t;
        w = 14.75385 + 0.19041 * t;
        break;
    case JME_BODY_SATURN:
        l = 49.94432 + 1222.11394 * t;
        a = 9.53707032;
        e = 0.05415060 - 0.000165136 * t;
        i = 2.48446 + 0.006114 * t;
        om = 113.71504 - 0.259207 * t;
        w = 92.43194 - 0.11145 * t;
        break;
    default:
        return JME_ERR;
    }

    l = jme_degree_normalize(l);
    om = jme_degree_normalize(om);
    w = jme_degree_normalize(w);
    m = jme_degree_normalize(l - w);

    /* Equation of center (simplified) */
    {
        double ec, v, r, x, y, z;
        double sin_m = sin(m * JME_DEG_TO_RAD);
        ec = (2.0 * e - 0.25 * e * e * e) * sin_m + 1.25 * e * e * sin(2.0 * m * JME_DEG_TO_RAD);
        v = m + ec * JME_RAD_TO_DEG;
        r = a * (1.0 - e * e) / (1.0 + e * cos(v * JME_DEG_TO_RAD));

        /* Heliocentric rectangular J2000 approx */
        x = r * (cos(om * JME_DEG_TO_RAD) * cos((v + w - om) * JME_DEG_TO_RAD) - sin(om * JME_DEG_TO_RAD) * sin((v + w - om) * JME_DEG_TO_RAD) * cos(i * JME_DEG_TO_RAD));
        y = r * (sin(om * JME_DEG_TO_RAD) * cos((v + w - om) * JME_DEG_TO_RAD) + cos(om * JME_DEG_TO_RAD) * sin((v + w - om) * JME_DEG_TO_RAD) * cos(i * JME_DEG_TO_RAD));
        z = r * (sin((v + w - om) * JME_DEG_TO_RAD) * sin(i * JME_DEG_TO_RAD));

        if (results != 0) {
            results[0] = x;
            results[1] = y;
            results[2] = z;
            results[3] = 0.0;
            results[4] = 0.0;
            results[5] = 0.0;
        }
    }

    return JME_OK;
}
