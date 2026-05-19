#include "jme/jme.h"
#include "context.h"
#include <math.h>
#include <string.h>

static double get_altitude(double jd_ut, int body, const char *starname, int flags, double *geopos, double atpress, double attemp, char *error)
{
    double results[6];
    double ra, dec, hour_angle, azimuth, altitude;
    double eps, gst;
    int topo_flag = (int)JME_CALC_TOPOCENTRIC;
    int equatorial_flag = (int)JME_CALC_EQUATORIAL;
    int res;

    jme_set_topo(geopos[0], geopos[1], geopos[2]);
    flags = flags | topo_flag | equatorial_flag;

    if (starname != 0 && starname[0] != '\0') {
        res = jme_fixstar_ut(starname, jd_ut, flags, results, error);
    } else {
        res = jme_calc_ut(jd_ut, body, flags, results, error);
    }

    if (res != JME_OK) { return -100.0; }

    ra = results[0];
    dec = results[1];

    jme_get_obliquity(jd_ut, JME_MODEL_OBL_IAU_1980, &eps, 0);
    gst = jme_sidereal_time(jd_ut);
    hour_angle = jme_degree_normalize(gst * 15.0 + geopos[0] - ra);

    jme_equatorial_to_horizontal(hour_angle, dec, geopos[1], &azimuth, &altitude);

    if (atpress > 0.0) {
        altitude = jme_refract(altitude, atpress, attemp, JME_COORD_TRUE_TO_APPARENT);
    }

    return altitude;
}

int jme_rise_trans(double jd_ut, int body, const char *starname, int flags, int rsmi, double *geopos, double atpress, double attemp, double *tret, char *error)
{
    double t = jd_ut;
    double h0 = -0.5667; /* Standard for stars (refraction only) */
    double alt, alt_next;
    int i;

    if (tret != 0) { *tret = 0.0; }
    if (geopos == 0 || tret == 0) {
        jme_set_error(error, "Rise and transit output arguments are required");
        return JME_ERR;
    }

    if (body == JME_BODY_SUN || body == JME_BODY_MOON) {
        h0 = -0.8333; /* Includes semi-diameter */
    }

    if (rsmi & JME_RISE_MERIDIAN_TRANSIT) {
        /* Search for transit from the supplied Julian day across one day. */
        for (i = 0; i < 240; i++) {
            double results[6];
            double ra, gst, ha, ha_next;
            if (starname != 0 && starname[0] != '\0') {
                jme_fixstar_ut(starname, t, flags | JME_CALC_EQUATORIAL, results, error);
            } else {
                jme_calc_ut(t, body, flags | JME_CALC_EQUATORIAL, results, error);
            }
            ra = results[0];
            gst = jme_sidereal_time(t);
            ha = jme_degrees_difference_signed(gst * 15.0 + geopos[0], ra);
            
            t += 0.01; /* 14.4 mins */
            gst = jme_sidereal_time(t);
            if (starname != 0 && starname[0] != '\0') {
                jme_fixstar_ut(starname, t, flags | JME_CALC_EQUATORIAL, results, error);
            } else {
                jme_calc_ut(t, body, flags | JME_CALC_EQUATORIAL, results, error);
            }
            ra = results[0];
            ha_next = jme_degrees_difference_signed(gst * 15.0 + geopos[0], ra);

            if (ha < 0 && ha_next >= 0) {
                /* Root found between t - 0.01 and t */
                if (tret != 0) { *tret = t - 0.005; }
                return JME_OK;
            }
        }
    }

    /* Rise/set search across one day. */
    for (i = 0; i < 24; i++) {
        alt = get_altitude(t, body, starname, flags, geopos, atpress, attemp, error);
        t += 1.0 / 24.0;
        alt_next = get_altitude(t, body, starname, flags, geopos, atpress, attemp, error);

        if (rsmi & JME_RISE_RISE) {
            if (alt < h0 && alt_next >= h0) {
                if (tret != 0) { *tret = t - 0.5 / 24.0; }
                return JME_OK;
            }
        } else if (rsmi & JME_RISE_SET) {
            if (alt > h0 && alt_next <= h0) {
                if (tret != 0) { *tret = t - 0.5 / 24.0; }
                return JME_OK;
            }
        }
    }

    jme_set_error(error, "Event not found in 24h search window");
    return JME_ERR;
}

int jme_solcross(double x2cross, double jd_ut, int flags, double *tret, char *error)
{
    double t = jd_ut;
    double lon, lon_next;
    double results[6];
    int i;

    if (tret == 0) {
        jme_set_error(error, "Solar crossing output is required");
        return JME_ERR;
    }

    for (i = 0; i < 365 * 2; i++) {
        jme_calc_ut(t, JME_BODY_SUN, flags, results, error);
        lon = results[0];
        t += 0.5;
        jme_calc_ut(t, JME_BODY_SUN, flags, results, error);
        lon_next = results[0];

        if (jme_degrees_difference_signed(lon_next, x2cross) >= 0 && jme_degrees_difference_signed(lon, x2cross) < 0) {
            if (tret != 0) { *tret = t - 0.25; }
            return JME_OK;
        }
    }

    return JME_ERR;
}

int jme_mooncross(double x2cross, double jd_ut, int flags, double *tret, char *error)
{
    double t = jd_ut;
    double lon, lon_next;
    double results[6];
    int i;

    if (tret == 0) {
        jme_set_error(error, "Lunar crossing output is required");
        return JME_ERR;
    }

    for (i = 0; i < 30 * 2; i++) {
        jme_calc_ut(t, JME_BODY_MOON, flags, results, error);
        lon = results[0];
        t += 0.5;
        jme_calc_ut(t, JME_BODY_MOON, flags, results, error);
        lon_next = results[0];

        if (jme_degrees_difference_signed(lon_next, x2cross) >= 0 && jme_degrees_difference_signed(lon, x2cross) < 0) {
            if (tret != 0) { *tret = t - 0.25; }
            return JME_OK;
        }
    }

    return JME_ERR;
}

int jme_sol_eclipse_when_loc(double jd_start, int flags, double *geopos, double *tret, double *attr, int backward, char *error)
{
    double t = jd_start;
    double step = 1.0; /* 1 day */
    int i;

    (void)attr; (void)backward;

    if (geopos == 0 || tret == 0) {
        jme_set_error(error, "Solar eclipse local search output arguments are required");
        return JME_ERR;
    }

    /* Search for conjunction in longitude */
    for (i = 0; i < 400; i++) {
        double sun_pos[6], moon_pos[6];
        double diff, diff_next;

        jme_calc_ut(t, JME_BODY_SUN, flags, sun_pos, error);
        jme_calc_ut(t, JME_BODY_MOON, flags, moon_pos, error);
        diff = jme_degrees_difference_signed(moon_pos[0], sun_pos[0]);

        t += step;
        jme_calc_ut(t, JME_BODY_SUN, flags, sun_pos, error);
        jme_calc_ut(t, JME_BODY_MOON, flags, moon_pos, error);
        diff_next = jme_degrees_difference_signed(moon_pos[0], sun_pos[0]);

        if (diff < 0 && diff_next >= 0) {
            /* Conjunction found! Check latitude for eclipse potential */
            if (fabs(moon_pos[1]) < 1.5) {
                if (tret != 0) { *tret = t - 0.5; }
                return JME_OK;
            }
        }
    }

    return JME_ERR;
}

int jme_sol_eclipse_where(double jd_ut, int flags, double *geopos, double *attr, char *error)
{
    (void)jd_ut; (void)flags; (void)geopos; (void)attr; (void)error;
    return JME_ERR;
}

int jme_sol_eclipse_when_glob(double jd_start, int flags, int epheflag, double *tret, int backward, char *error)
{
    double geopos[3] = {0.0, 0.0, 0.0};
    double attr[20];
    (void)epheflag;
    return jme_sol_eclipse_when_loc(jd_start, flags, geopos, tret, attr, backward, error);
}

int jme_lun_eclipse_when(double jd_start, int flags, int iflag, double *tret, int backward, char *error)
{
    double geopos[3] = {0.0, 0.0, 0.0};
    double attr[20];
    (void)iflag;
    return jme_lun_eclipse_when_loc(jd_start, flags, geopos, tret, attr, backward, error);
}

int jme_lun_occult_where(double jd_ut, int body, const char *starname, int flags, double *geopos, double *attr, char *error)
{
    (void)jd_ut; (void)body; (void)starname; (void)flags; (void)geopos; (void)attr; (void)error;
    return JME_ERR;
}

int jme_lun_occult_when_loc(double jd_start, int body, const char *starname, int flags, double *geopos, double *tret, double *attr, int backward, char *error)
{
    (void)jd_start; (void)body; (void)starname; (void)flags; (void)geopos; (void)tret; (void)attr; (void)backward; (void)error;
    return JME_ERR;
}

int jme_lun_occult_when_glob(double jd_start, int body, const char *starname, int flags, int iflag, double *tret, int backward, char *error)
{
    (void)jd_start; (void)body; (void)starname; (void)flags; (void)iflag; (void)tret; (void)backward; (void)error;
    return JME_ERR;
}

int jme_rise_trans_true_hor(double jd_ut, int body, const char *starname, int flags, int rsmi, double *geopos, double atpress, double attemp, double horhgt, double *tret, char *error)
{
    (void)horhgt;
    return jme_rise_trans(jd_ut, body, starname, flags, rsmi, geopos, atpress, attemp, tret, error);
}

int jme_solcross_ut(double x2cross, double jd_ut, int flags, double *tret, char *error)
{
    return jme_solcross(x2cross, jd_ut, flags, tret, error);
}

int jme_mooncross_ut(double x2cross, double jd_ut, int flags, double *tret, char *error)
{
    return jme_mooncross(x2cross, jd_ut, flags, tret, error);
}

int jme_mooncross_node(double jd_ut, int flags, double *tret, char *error)
{
    double t = jd_ut;
    double pos[6];
    double lat;
    double next_lat;
    int i;

    if (tret == 0) {
        jme_set_error(error, "Lunar node crossing output is required");
        return JME_ERR;
    }

    if (jme_calc_ut(t, JME_BODY_MOON, flags, pos, error) != JME_OK) {
        return JME_ERR;
    }
    lat = pos[1];

    for (i = 0; i < 60; i++) {
        t += 0.25;
        if (jme_calc_ut(t, JME_BODY_MOON, flags, pos, error) != JME_OK) {
            return JME_ERR;
        }
        next_lat = pos[1];
        if ((lat <= 0.0 && next_lat >= 0.0) || (lat >= 0.0 && next_lat <= 0.0)) {
            *tret = t - 0.125;
            return JME_OK;
        }
        lat = next_lat;
    }

    jme_set_error(error, "Lunar node crossing not found in search window");
    return JME_ERR;
}

int jme_mooncross_node_ut(double jd_ut, int flags, double *tret, char *error)
{
    return jme_mooncross_node(jd_ut, flags, tret, error);
}

int jme_helio_cross(int body, double x2cross, double jd_ut, int flags, double *tret, char *error)
{
    double t = jd_ut;
    double pos[6];
    double lon;
    double lon_next;
    int i;

    if (tret == 0) {
        jme_set_error(error, "Heliocentric crossing output is required");
        return JME_ERR;
    }

    x2cross = jme_degree_normalize(x2cross);
    for (i = 0; i < 365 * 20; i++) {
        if (jme_calc_ut(t, body, flags | JME_CALC_HELIOCENTRIC, pos, error) != JME_OK) {
            return JME_ERR;
        }
        lon = pos[0];
        t += 0.5;
        if (jme_calc_ut(t, body, flags | JME_CALC_HELIOCENTRIC, pos, error) != JME_OK) {
            return JME_ERR;
        }
        lon_next = pos[0];
        if (jme_degrees_difference_signed(lon_next, x2cross) >= 0.0 && jme_degrees_difference_signed(lon, x2cross) < 0.0) {
            *tret = t - 0.25;
            return JME_OK;
        }
    }

    jme_set_error(error, "Heliocentric crossing not found in search window");
    return JME_ERR;
}

int jme_helio_cross_ut(int body, double x2cross, double jd_ut, int flags, double *tret, char *error)
{
    return jme_helio_cross(body, x2cross, jd_ut, flags, tret, error);
}

int jme_nod_aps_ut(double jd_ut, int body, int flags, int method, double *tret, char *error)
{
    double dt = jme_delta_t(jd_ut);
    double jd_et = jd_ut + dt / 86400.0;
    return jme_nod_aps(jd_et, body, flags, method, tret, error);
}

int jme_heliacal_pheno_ut(double jd_ut, double *geopos, double *dat_hel, char *error)
{
    (void)jd_ut; (void)geopos; (void)dat_hel; (void)error;
    return JME_ERR;
}

double jme_heliacal_angle(double jd_ut, double *geopos, double *dat_hel, char *error)
{
    (void)jd_ut; (void)geopos; (void)dat_hel; (void)error;
    return 0.0;
}

double jme_topo_arcus_visionis(double jd_ut, double *geopos, double *dat_hel, char *error)
{
    (void)jd_ut; (void)geopos; (void)dat_hel; (void)error;
    return 0.0;
}

int jme_lun_eclipse_when_loc(double jd_start, int flags, double *geopos, double *tret, double *attr, int backward, char *error)
{
    double t = jd_start;
    double step = 1.0;
    int i;

    (void)geopos; (void)attr; (void)backward;
    if (geopos == 0 || tret == 0) {
        jme_set_error(error, "Lunar eclipse local search arguments are required");
        return JME_ERR;
    }

    for (i = 0; i < 400; i++) {
        double sun_pos[6], moon_pos[6];
        double diff, diff_next;

        jme_calc_ut(t, JME_BODY_SUN, flags, sun_pos, error);
        jme_calc_ut(t, JME_BODY_MOON, flags, moon_pos, error);
        diff = jme_degrees_difference_signed(moon_pos[0], jme_degree_normalize(sun_pos[0] + 180.0));

        t += step;
        jme_calc_ut(t, JME_BODY_SUN, flags, sun_pos, error);
        jme_calc_ut(t, JME_BODY_MOON, flags, moon_pos, error);
        diff_next = jme_degrees_difference_signed(moon_pos[0], jme_degree_normalize(sun_pos[0] + 180.0));

        if (diff < 0 && diff_next >= 0) {
            if (fabs(moon_pos[1]) < 1.5) {
                if (tret != 0) { *tret = t - 0.5; }
                return JME_OK;
            }
        }
    }

    return JME_ERR;
}

int jme_sol_eclipse_how(double jd_ut, int flags, double *geopos, double *attr, char *error)
{
    (void)jd_ut; (void)flags; (void)geopos; (void)attr; (void)error;
    return JME_ERR;
}

int jme_lun_eclipse_how(double jd_ut, int flags, double *geopos, double *attr, char *error)
{
    (void)jd_ut; (void)flags; (void)geopos; (void)attr; (void)error;
    return JME_ERR;
}

int jme_nod_aps(double jd_et, int body, int flags, int method, double *tret, char *error)
{
    (void)jd_et; (void)body; (void)flags; (void)method; (void)tret; (void)error;
    return JME_ERR;
}

int jme_get_orbital_elements(double jd_et, int body, int flags, double *elem, char *error)
{
    (void)jd_et; (void)body; (void)flags; (void)elem; (void)error;
    return JME_ERR;
}

int jme_heliacal_ut(double jd_ut, double *geopos, double *dat_hel, char *error)
{
    (void)jd_ut; (void)geopos; (void)dat_hel; (void)error;
    return JME_ERR;
}

int jme_vis_limit_mag(double jd_ut, double *geopos, double *dat_hel, char *error)
{
    (void)jd_ut; (void)geopos; (void)dat_hel; (void)error;
    return JME_ERR;
}
