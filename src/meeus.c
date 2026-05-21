#include "jme/jme.h"
#include "context.h"
#include <math.h>

#define JME_M_PI 3.1415926535897932384626433832795028841971693993751
#define JME_DEG_TO_RAD 0.017453292519943295769236907684886127134428718885417
#define JME_RAD_TO_DEG 57.295779513082320876798154814105170332405472466565

static double meeus_solve_kepler(double mean_anomaly_rad, double eccentricity)
{
    double e_anomaly = mean_anomaly_rad;
    int iter;

    if (eccentricity > 0.8) {
        e_anomaly = JME_M_PI;
    }

    for (iter = 0; iter < 20; iter++) {
        double f = e_anomaly - eccentricity * sin(e_anomaly) - mean_anomaly_rad;
        double fp = 1.0 - eccentricity * cos(e_anomaly);
        double delta;

        if (fabs(fp) < 1.0e-15) {
            break;
        }

        delta = f / fp;
        e_anomaly -= delta;

        if (fabs(delta) < 1.0e-14) {
            break;
        }
    }

    return e_anomaly;
}

static int meeus_sun_position(double jd_et, double *position)
{
    double t = (jd_et - 2451545.0) / 36525.0;
    double l0, m, e, c, sun_lon, r;
    double spherical[6] = {0,0,0,0,0,0};

    if (position == 0) {
        return JME_ERR;
    }

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

    spherical[0] = sun_lon;
    spherical[1] = 0.0;
    spherical[2] = r;

    return jme_spherical_to_rectangular_state(spherical, position);
}

int jme_meeus_sun_state(double jd_et, double *results)
{
    double prev[6];
    double next[6];
    double step = 1.0e-3;
    int i;

    if (results == 0) {
        return JME_ERR;
    }

    for (i = 0; i < 6; i++) { results[i] = 0.0; }

    if (meeus_sun_position(jd_et, results) != JME_OK) {
        return JME_ERR;
    }

    if (meeus_sun_position(jd_et - step, prev) == JME_OK
        && meeus_sun_position(jd_et + step, next) == JME_OK) {
        for (i = 0; i < 3; i++) {
            results[i + 3] = (next[i] - prev[i]) / (2.0 * step);
        }
    }

    return JME_OK;
}

static int meeus_moon_position(double jd_et, double *position)
{
    double t = (jd_et - 2451545.0) / 36525.0;
    double lp, d, m, mp, f, a1, a2, a3, e;
    double sl, sb, sr;
    double spherical[6] = {0,0,0,0,0,0};

    if (position == 0) {
        return JME_ERR;
    }

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
            + 0.114332 * sin(2.0 * f * JME_DEG_TO_RAD)
            + 0.003958 * sin(a1 * JME_DEG_TO_RAD)
            + 0.001962 * sin((lp - f) * JME_DEG_TO_RAD)
            + 0.000318 * sin(a2 * JME_DEG_TO_RAD);

    sb = 5.128122 * sin(f * JME_DEG_TO_RAD)
            + 0.280602 * sin((mp + f) * JME_DEG_TO_RAD)
            + 0.277693 * sin((mp - f) * JME_DEG_TO_RAD)
            + 0.173237 * sin((2.0 * d - f) * JME_DEG_TO_RAD)
            - 0.002235 * sin(lp * JME_DEG_TO_RAD)
            + 0.000382 * sin(a3 * JME_DEG_TO_RAD)
            + 0.000175 * sin((a1 - f) * JME_DEG_TO_RAD)
            + 0.000175 * sin((a1 + f) * JME_DEG_TO_RAD);

    sr = 385000.56 / 149597870.7  /* distance in AU */
            - 20905.355 / 149597870.7 * cos(mp * JME_DEG_TO_RAD)
            - 3699.111 / 149597870.7 * cos((2.0 * d - mp) * JME_DEG_TO_RAD)
            - 2955.968 / 149597870.7 * cos(2.0 * d * JME_DEG_TO_RAD);

    spherical[0] = jme_degree_normalize(sl);
    spherical[1] = sb;
    spherical[2] = sr;

    return jme_spherical_to_rectangular_state(spherical, position);
}

int jme_meeus_moon_state(double jd_et, double *results)
{
    double prev[6];
    double next[6];
    double step = 1.0e-3;
    int i;

    if (results == 0) {
        return JME_ERR;
    }

    for (i = 0; i < 6; i++) { results[i] = 0.0; }

    if (meeus_moon_position(jd_et, results) != JME_OK) {
        return JME_ERR;
    }

    if (meeus_moon_position(jd_et - step, prev) == JME_OK
        && meeus_moon_position(jd_et + step, next) == JME_OK) {
        for (i = 0; i < 3; i++) {
            results[i + 3] = (next[i] - prev[i]) / (2.0 * step);
        }
    }

    return JME_OK;
}

static int meeus_planet_position(double jd_et, int body, double *position)
{
    double t = (jd_et - 2451545.0) / 36525.0;
    double a, e, inc, l, long_peri, node;

    if (position == 0) {
        return JME_ERR;
    }

    /* Time-varying mean heliocentric elements referred to the J2000 ecliptic. */
    switch (body) {
    case JME_BODY_MERCURY:
        a = 0.38709927 + 0.00000037 * t;
        e = 0.20563593 + 0.00001906 * t;
        inc = 7.00497902 - 0.00594749 * t;
        l = 252.25032350 + 149472.67411175 * t;
        long_peri = 77.45779628 + 0.16047689 * t;
        node = 48.33076593 - 0.12534081 * t;
        break;
    case JME_BODY_VENUS:
        a = 0.72333566 + 0.00000390 * t;
        e = 0.00677672 - 0.00004107 * t;
        inc = 3.39467605 - 0.00078890 * t;
        l = 181.97909950 + 58517.81538729 * t;
        long_peri = 131.60246718 + 0.00268329 * t;
        node = 76.67984255 - 0.27769418 * t;
        break;
    case JME_BODY_EARTH:
        a = 1.00000261 + 0.00000562 * t;
        e = 0.01671123 - 0.00004392 * t;
        inc = -0.00001531 - 0.01294668 * t;
        l = 100.46457166 + 35999.37244981 * t;
        long_peri = 102.93768193 + 0.32327364 * t;
        node = 0.0;
        break;
    case JME_BODY_MARS:
        a = 1.52371034 + 0.00001847 * t;
        e = 0.09339410 + 0.00007882 * t;
        inc = 1.84969142 - 0.00813131 * t;
        l = -4.55343205 + 19140.30268499 * t;
        long_peri = -23.94362959 + 0.44441088 * t;
        node = 49.55953891 - 0.29257343 * t;
        break;
    case JME_BODY_JUPITER:
        a = 5.20288700 - 0.00011607 * t;
        e = 0.04838624 - 0.00013253 * t;
        inc = 1.30439695 - 0.00183714 * t;
        l = 34.39644051 + 3034.74612775 * t;
        long_peri = 14.72847983 + 0.21252668 * t;
        node = 100.47390909 + 0.20469106 * t;
        break;
    case JME_BODY_SATURN:
        a = 9.53667594 - 0.00125060 * t;
        e = 0.05386179 - 0.00050991 * t;
        inc = 2.48599187 + 0.00193609 * t;
        l = 49.95424423 + 1222.49362201 * t;
        long_peri = 92.59887831 - 0.41897216 * t;
        node = 113.66242448 - 0.28867794 * t;
        break;
    case JME_BODY_URANUS:
        a = 19.18916464 - 0.00196176 * t;
        e = 0.04725744 - 0.00004397 * t;
        inc = 0.77263783 - 0.00242939 * t;
        l = 313.23810451 + 428.48202785 * t;
        long_peri = 170.95427630 + 0.40805281 * t;
        node = 74.01692503 + 0.04240589 * t;
        break;
    case JME_BODY_NEPTUNE:
        a = 30.06992276 + 0.00026291 * t;
        e = 0.00859048 + 0.00005105 * t;
        inc = 1.77004347 + 0.00035372 * t;
        l = -55.12002969 + 218.45945325 * t;
        long_peri = 44.96476227 - 0.32241464 * t;
        node = 131.78422574 - 0.00508664 * t;
        break;
    case JME_BODY_PLUTO:
        a = 39.48211675 - 0.00031596 * t;
        e = 0.24882730 + 0.00005170 * t;
        inc = 17.14001206 + 0.00004818 * t;
        l = 238.92903833 + 145.20780515 * t;
        long_peri = 224.06891629 - 0.04062942 * t;
        node = 110.30393684 - 0.01183482 * t;
        break;
    default:
        return JME_ERR;
    }

    {
        double mean_anomaly = jme_degree_normalize(l - long_peri) * JME_DEG_TO_RAD;
        double eccentric_anomaly = meeus_solve_kepler(mean_anomaly, e);
        double xv = a * (cos(eccentric_anomaly) - e);
        double yv = a * sqrt(1.0 - e * e) * sin(eccentric_anomaly);
        double true_anomaly = atan2(yv, xv);
        double radius = sqrt(xv * xv + yv * yv);
        double node_rad = jme_degree_normalize(node) * JME_DEG_TO_RAD;
        double inc_rad = inc * JME_DEG_TO_RAD;
        double arg_peri_rad = jme_degree_normalize(long_peri - node) * JME_DEG_TO_RAD;
        double u = true_anomaly + arg_peri_rad;
        double cos_node = cos(node_rad);
        double sin_node = sin(node_rad);
        double cos_u = cos(u);
        double sin_u = sin(u);
        double cos_inc = cos(inc_rad);
        double sin_inc = sin(inc_rad);
        double x = radius * (cos_node * cos_u - sin_node * sin_u * cos_inc);
        double y = radius * (sin_node * cos_u + cos_node * sin_u * cos_inc);
        double z = radius * (sin_u * sin_inc);

        if (!isfinite(x) || !isfinite(y) || !isfinite(z)) {
            return JME_ERR;
        }

        position[0] = x;
        position[1] = y;
        position[2] = z;
        position[3] = 0.0;
        position[4] = 0.0;
        position[5] = 0.0;
    }

    return JME_OK;
}

int jme_meeus_planet_state(double jd_et, int body, double *results)
{
    double prev[6];
    double next[6];
    double step = 1.0e-3;
    int i;

    if (results == 0) {
        return JME_ERR;
    }

    for (i = 0; i < 6; i++) { results[i] = 0.0; }

    if (meeus_planet_position(jd_et, body, results) != JME_OK) {
        return JME_ERR;
    }

    if (meeus_planet_position(jd_et - step, body, prev) == JME_OK
        && meeus_planet_position(jd_et + step, body, next) == JME_OK) {
        for (i = 0; i < 3; i++) {
            results[i + 3] = (next[i] - prev[i]) / (2.0 * step);
        }
    }

    return JME_OK;
}
