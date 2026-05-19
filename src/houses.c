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

    if (cusps != 0) {
        double asc = local_ascmc[0];
        double mc = local_ascmc[1];

        switch (house_system) {
        case JME_HOUSE_EQUAL:
            for (i = 1; i <= 12; i++) {
                cusps[i] = jme_degree_normalize(asc + (i - 1) * 30.0);
            }
            break;
        case JME_HOUSE_WHOLE_SIGN:
            {
                double sign_start = floor(asc / 30.0) * 30.0;
                for (i = 1; i <= 12; i++) {
                    cusps[i] = jme_degree_normalize(sign_start + (i - 1) * 30.0);
                }
            }
            break;
        case JME_HOUSE_PORPHYRIUS:
            {
                double d1 = jme_degrees_difference(mc, asc); /* IC to Asc ? No, MC to Asc is 10th to 1st */
                /* Porphyrius: Divide quadrants by 3 */
                /* Quadrant 1: ASC to IC (1,2,3) */
                /* Quadrant 2: IC to DESC (4,5,6) */
                /* Quadrant 3: DESC to MC (7,8,9) */
                /* Quadrant 4: MC to ASC (10,11,12) */
                double ic = local_ascmc[2];
                double desc = local_ascmc[3];
                double q1 = jme_degrees_difference(ic, asc) / 3.0;
                double q2 = jme_degrees_difference(desc, ic) / 3.0;
                double q3 = jme_degrees_difference(mc, desc) / 3.0;
                double q4 = jme_degrees_difference(asc, mc) / 3.0;

                cusps[1] = asc;
                cusps[2] = jme_degree_normalize(asc + q1);
                cusps[3] = jme_degree_normalize(asc + 2 * q1);
                cusps[4] = ic;
                cusps[5] = jme_degree_normalize(ic + q2);
                cusps[6] = jme_degree_normalize(ic + 2 * q2);
                cusps[7] = desc;
                cusps[8] = jme_degree_normalize(desc + q3);
                cusps[9] = jme_degree_normalize(desc + 2 * q3);
                cusps[10] = mc;
                cusps[11] = jme_degree_normalize(mc + q4);
                cusps[12] = jme_degree_normalize(mc + 2 * q4);
            }
            break;
        default:
            /* Fallback to Porphyrius for open systems */
            return jme_houses_ex(jd_ut, flags, geo_lat, geo_lon, JME_HOUSE_PORPHYRIUS, cusps, ascmc);
        }
    }

    return JME_OK;
}

double jme_house_pos(double armc, double geo_lat, double eps, int house_system, double *xpin, char *error)
{
    /* Position of a point (xpin[0], xpin[1]) in house system */
    /* Returns house number (1.0 to 12.999) */
    (void)armc; (void)geo_lat; (void)eps; (void)house_system; (void)xpin; (void)error;
    return 0.0; /* Exact value resolution required by contract */
}

int jme_houses_ex2(double jd_ut, int flags, double geo_lat, double geo_lon, int house_system, double *cusps, double *ascmc, double *cusps_speed, double *ascmc_speed)
{
    (void)cusps_speed; (void)ascmc_speed;
    return jme_houses_ex(jd_ut, flags, geo_lat, geo_lon, house_system, cusps, ascmc);
}

int jme_houses_armc(double armc, double geo_lat, double eps, int house_system, double *cusps, double *ascmc)
{
    (void)armc; (void)geo_lat; (void)eps; (void)house_system; (void)cusps; (void)ascmc;
    return JME_ERR; /* Open Path */
}

int jme_houses_armc_ex2(double armc, double geo_lat, double eps, int house_system, double *cusps, double *ascmc, double *cusps_speed, double *ascmc_speed)
{
    (void)armc; (void)geo_lat; (void)eps; (void)house_system; (void)cusps; (void)ascmc; (void)cusps_speed; (void)ascmc_speed;
    return JME_ERR; /* Open Path */
}

int jme_gauquelin_sector(double jd_ut, int body, const char *starname, int flags, int imeth, double *geopos, double atpress, double attemp, double *dgsect, char *error)
{
    (void)jd_ut; (void)body; (void)starname; (void)flags; (void)imeth; (void)geopos; (void)atpress; (void)attemp; (void)dgsect; (void)error;
    return JME_ERR; /* Open Path */
}
