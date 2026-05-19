#include "jme/jme.h"
#include "context.h"
#include <math.h>

#define JME_DEG_TO_RAD 0.017453292519943295769236907684886127134428718885417
#define JME_RAD_TO_DEG 57.295779513082320876798154814105170332405472466565

static double atan2d(double y, double x)
{
    return atan2(y, x) * JME_RAD_TO_DEG;
}

static void calc_ascmc(double ramc, double lat, double eps, double *ascmc)
{
    double r = ramc * JME_DEG_TO_RAD;
    double l = lat * JME_DEG_TO_RAD;
    double e = eps * JME_DEG_TO_RAD;
    double mc, asc, vertex, ep;

    /* MC */
    mc = atan2d(sin(r), cos(r) * cos(e));
    ascmc[1] = jme_degree_normalize(mc);

    /* Ascendant */
    asc = atan2d(cos(r), -sin(r) * cos(e) - tan(l) * sin(e));
    ascmc[0] = jme_degree_normalize(asc);

    /* IC */
    ascmc[2] = jme_degree_normalize(mc + 180.0);

    /* Descendant */
    ascmc[3] = jme_degree_normalize(asc + 180.0);

    /* Vertex */
    vertex = atan2d(-cos(r + 90.0 * JME_DEG_TO_RAD), sin(r + 90.0 * JME_DEG_TO_RAD) * cos(e) - tan(l) * sin(e));
    ascmc[4] = jme_degree_normalize(vertex);

    /* Equatorial Ascendant */
    ep = atan2d(cos(r), -sin(r) * cos(e));
    ascmc[5] = jme_degree_normalize(ep);
}

static int fill_house_cusps_from_angles(double asc, double mc, int house_system, double *cusps)
{
    int i;

    if (cusps == 0) {
        return JME_OK;
    }

    switch (house_system) {
    case JME_HOUSE_EQUAL:
    case 'E':
        for (i = 1; i <= 12; i++) {
            cusps[i] = jme_degree_normalize(asc + (i - 1) * 30.0);
        }
        return JME_OK;
    case JME_HOUSE_WHOLE_SIGN:
    case 'W':
        {
            double sign_start = floor(asc / 30.0) * 30.0;
            for (i = 1; i <= 12; i++) {
                cusps[i] = jme_degree_normalize(sign_start + (i - 1) * 30.0);
            }
        }
        return JME_OK;
    case JME_HOUSE_PORPHYRIUS:
    case 'O':
    case 'P':
        {
            double ic = jme_degree_normalize(mc + 180.0);
            double desc = jme_degree_normalize(asc + 180.0);
            double q1 = jme_degrees_difference(ic, asc) / 3.0;
            double q2 = jme_degrees_difference(desc, ic) / 3.0;
            double q3 = jme_degrees_difference(mc, desc) / 3.0;
            double q4 = jme_degrees_difference(asc, mc) / 3.0;

            cusps[1] = asc;
            cusps[2] = jme_degree_normalize(asc + q1);
            cusps[3] = jme_degree_normalize(asc + 2.0 * q1);
            cusps[4] = ic;
            cusps[5] = jme_degree_normalize(ic + q2);
            cusps[6] = jme_degree_normalize(ic + 2.0 * q2);
            cusps[7] = desc;
            cusps[8] = jme_degree_normalize(desc + q3);
            cusps[9] = jme_degree_normalize(desc + 2.0 * q3);
            cusps[10] = mc;
            cusps[11] = jme_degree_normalize(mc + q4);
            cusps[12] = jme_degree_normalize(mc + 2.0 * q4);
        }
        return JME_OK;
    default:
        return JME_ERR;
    }
}

int jme_houses(double jd_ut, double geo_lat, double geo_lon, int house_system, double *cusps, double *ascmc)
{
    return jme_houses_ex(jd_ut, 0, geo_lat, geo_lon, house_system, cusps, ascmc);
}

int jme_houses_ex(double jd_ut, int flags, double geo_lat, double geo_lon, int house_system, double *cusps, double *ascmc)
{
    double eps, dpsi, deps;
    double gst, ramc;
    double local_ascmc[10];
    int i;

    (void)flags;

    /* 1. Need Obliquity and Nutation for RAMC and coordinate transform */
    jme_get_obliquity(jd_ut, JME_MODEL_OBL_IAU_1980, &eps, 0);
    jme_get_nutation(jd_ut, JME_MODEL_NUT_IAU_1980, &dpsi, &deps, 0);
    eps += deps; /* True obliquity */

    gst = jme_sidereal_time0(jd_ut, eps - deps, dpsi);
    ramc = jme_degree_normalize(gst * 15.0 + geo_lon);

    calc_ascmc(ramc, geo_lat, eps, local_ascmc);
    if (ascmc != 0) {
        for (i = 0; i < 10; i++) { ascmc[i] = local_ascmc[i]; }
    }

    if (fill_house_cusps_from_angles(local_ascmc[0], local_ascmc[1], house_system, cusps) != JME_OK) {
        return JME_ERR;
    }

    return JME_OK;
}

double jme_house_pos(double armc, double geo_lat, double eps, int house_system, double *xpin, char *error)
{
    double ascmc[10];
    double cusps[13];
    double lon;
    int i;

    if (xpin == 0) {
        jme_set_error(error, "House position input is required");
        return 0.0;
    }

    if (jme_houses_armc(armc, geo_lat, eps, house_system, cusps, ascmc) != JME_OK) {
        jme_set_error(error, "House system is not available");
        return 0.0;
    }

    lon = jme_degree_normalize(xpin[0]);
    for (i = 1; i <= 12; i++) {
        int next = i == 12 ? 1 : i + 1;
        double width = jme_degrees_difference(cusps[next], cusps[i]);
        double delta = jme_degrees_difference(lon, cusps[i]);
        if (delta < width || (i == 12 && delta <= width)) {
            return (double)i + (width > 0.0 ? delta / width : 0.0);
        }
    }

    return 0.0;
}

int jme_houses_ex2(double jd_ut, int flags, double geo_lat, double geo_lon, int house_system, double *cusps, double *ascmc, double *cusps_speed, double *ascmc_speed)
{
    (void)cusps_speed; (void)ascmc_speed;
    return jme_houses_ex(jd_ut, flags, geo_lat, geo_lon, house_system, cusps, ascmc);
}

int jme_houses_armc(double armc, double geo_lat, double eps, int house_system, double *cusps, double *ascmc)
{
    double local_ascmc[10];
    int i;

    calc_ascmc(armc, geo_lat, eps, local_ascmc);
    if (ascmc != 0) {
        for (i = 0; i < 10; i++) {
            ascmc[i] = local_ascmc[i];
        }
    }

    return fill_house_cusps_from_angles(local_ascmc[0], local_ascmc[1], house_system, cusps);
}

int jme_houses_armc_ex2(double armc, double geo_lat, double eps, int house_system, double *cusps, double *ascmc, double *cusps_speed, double *ascmc_speed)
{
    int i;
    int rc = jme_houses_armc(armc, geo_lat, eps, house_system, cusps, ascmc);
    if (cusps_speed != 0) {
        for (i = 0; i < 13; i++) {
            cusps_speed[i] = 0.0;
        }
    }
    if (ascmc_speed != 0) {
        for (i = 0; i < 10; i++) {
            ascmc_speed[i] = 0.0;
        }
    }
    return rc;
}

int jme_gauquelin_sector(double jd_ut, int body, const char *starname, int flags, int imeth, double *geopos, double atpress, double attemp, double *dgsect, char *error)
{
    (void)jd_ut; (void)body; (void)starname; (void)flags; (void)imeth; (void)geopos; (void)atpress; (void)attemp; (void)dgsect; (void)error;
    return JME_ERR;
}
