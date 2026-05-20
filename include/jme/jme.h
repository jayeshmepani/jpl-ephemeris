#ifndef JME_JME_H
#define JME_JME_H

#include <stddef.h>

#ifdef __cplusplus
extern "C" {
#endif

#define JME_VERSION "0.1.0"
#define JME_AU_KM 149597870.700
#define JME_SPEED_OF_LIGHT_KM_PER_SEC 299792.458
#define JME_SECONDS_PER_DAY 86400.0

typedef enum jme_status {
    JME_OK = 0,
    JME_ERR = -1
} jme_status;

typedef enum jme_calendar {
    JME_CALENDAR_JULIAN = 0,
    JME_CALENDAR_GREGORIAN = 1
} jme_calendar;

typedef enum jme_body {
    JME_BODY_SUN = 0,
    JME_BODY_MOON = 1,
    JME_BODY_MERCURY = 2,
    JME_BODY_VENUS = 3,
    JME_BODY_MARS = 4,
    JME_BODY_JUPITER = 5,
    JME_BODY_SATURN = 6,
    JME_BODY_URANUS = 7,
    JME_BODY_NEPTUNE = 8,
    JME_BODY_PLUTO = 9,
    JME_BODY_EARTH = 10,
    JME_BODY_SOLAR_SYSTEM_BARYCENTER = 11,
    JME_BODY_MERCURY_BARYCENTER = 12,
    JME_BODY_VENUS_BARYCENTER = 13,
    JME_BODY_EARTH_MOON_BARYCENTER = 14,
    JME_BODY_MARS_BARYCENTER = 15,
    JME_BODY_JUPITER_BARYCENTER = 16,
    JME_BODY_SATURN_BARYCENTER = 17,
    JME_BODY_URANUS_BARYCENTER = 18,
    JME_BODY_NEPTUNE_BARYCENTER = 19,
    JME_BODY_PLUTO_BARYCENTER = 20,
    JME_BODY_MEAN_NODE = 21,
    JME_BODY_TRUE_NODE = 22
} jme_body;

typedef enum jme_calc_flags {
    JME_CALC_NONE = 0,
    JME_CALC_SPEED = 1 << 0,
    JME_CALC_EQUATORIAL = 1 << 1,
    JME_CALC_XYZ = 1 << 2,
    JME_CALC_RADIANS = 1 << 3,
    JME_CALC_BARYCENTRIC = 1 << 4,
    JME_CALC_HELIOCENTRIC = 1 << 5,
    JME_CALC_TRUE_POSITION = 1 << 6,
    JME_CALC_J2000 = 1 << 7,
    JME_CALC_NO_NUTATION = 1 << 8,
    JME_CALC_SIDEREAL = 1 << 9
} jme_calc_flags;

typedef enum jme_sidereal_mode {
    JME_SIDEREAL_FAGAN_BRADLEY = 0,
    JME_SIDEREAL_LAHIRI = 1,
    JME_SIDEREAL_USER = 255
} jme_sidereal_mode;

typedef enum jme_vector_unit {
    JME_VECTOR_AU_PER_DAY = 0,
    JME_VECTOR_KM_PER_DAY = 1,
    JME_VECTOR_AU_PER_SECOND = 2,
    JME_VECTOR_KM_PER_SECOND = 3
} jme_vector_unit;

typedef enum jme_orientation_unit {
    JME_ORIENTATION_RAD_PER_DAY = 0,
    JME_ORIENTATION_RAD_PER_SECOND = 1
} jme_orientation_unit;

typedef enum jme_jpl_time_scale {
    JME_JPL_TIMESCALE_UNKNOWN = 0,
    JME_JPL_TIMESCALE_TDB = 1,
    JME_JPL_TIMESCALE_TCB = 2
} jme_jpl_time_scale;

const char *jme_version(char *buffer, size_t buffer_size);
void jme_close(void);
const char *jme_ephemeris_path(void);
void jme_set_ephemeris_path(const char *path);
const char *jme_jpl_file(void);
void jme_set_jpl_file(const char *path);
void jme_set_sidereal_mode(int sidereal_mode, double t0, double ayan_t0);
void jme_get_sidereal_mode(int *sidereal_mode, double *t0, double *ayan_t0);
void jme_set_topo(double lon, double lat, double altitude);

int jme_calc(double jd_et, int body, int flags, double *results, char *error);
int jme_calc_ut(double jd_ut, int body, int flags, double *results, char *error);
int jme_calc_pctr(double jd_et, int body, int center, int flags, double *results, char *error);
int jme_pheno(double jd_et, int body, int flags, double *attr, char *error);
int jme_pheno_ut(double jd_ut, int body, int flags, double *attr, char *error);
int jme_fixstar(const char *star, double jd_et, int flags, double *results, char *error);
int jme_fixstar_ut(const char *star, double jd_ut, int flags, double *results, char *error);
int jme_fixstar_mag(const char *star, double *mag, char *error);
int jme_fixstar2(const char *star, double jd_et, int flags, double *results, char *error);
int jme_fixstar2_ut(const char *star, double jd_ut, int flags, double *results, char *error);
int jme_fixstar2_mag(const char *star, double *mag, char *error);

int jme_houses(double jd_ut, double geo_lat, double geo_lon, int house_system, double *cusps, double *ascmc);
int jme_houses_ex(double jd_ut, int flags, double geo_lat, double geo_lon, int house_system, double *cusps, double *ascmc);
int jme_houses_ex2(double jd_ut, int flags, double geo_lat, double geo_lon, int house_system, double *cusps, double *ascmc, double *cusps_speed, double *ascmc_speed);
int jme_houses_armc(double armc, double geo_lat, double eps, int house_system, double *cusps, double *ascmc);
int jme_houses_armc_ex2(double armc, double geo_lat, double eps, int house_system, double *cusps, double *ascmc, double *cusps_speed, double *ascmc_speed);
double jme_house_pos(double armc, double geo_lat, double eps, int house_system, double *xpin, char *error);

int jme_gauquelin_sector(double jd_ut, int body, const char *starname, int flags, int imeth, double *geopos, double atpress, double attemp, double *dgsect, char *error);

int jme_sol_eclipse_where(double jd_ut, int flags, double *geopos, double *attr, char *error);
int jme_sol_eclipse_how(double jd_ut, int flags, double *geopos, double *attr, char *error);
int jme_sol_eclipse_when_loc(double jd_start, int flags, double *geopos, double *tret, double *attr, int backward, char *error);
int jme_sol_eclipse_when_glob(double jd_start, int flags, int epheflag, double *tret, int backward, char *error);
int jme_lun_eclipse_how(double jd_ut, int flags, double *geopos, double *attr, char *error);
int jme_lun_eclipse_when_loc(double jd_start, int flags, double *geopos, double *tret, double *attr, int backward, char *error);
int jme_lun_eclipse_when(double jd_start, int flags, int iflag, double *tret, int backward, char *error);
int jme_lun_occult_where(double jd_ut, int body, const char *starname, int flags, double *geopos, double *attr, char *error);
int jme_lun_occult_when_loc(double jd_start, int body, const char *starname, int flags, double *geopos, double *tret, double *attr, int backward, char *error);
int jme_lun_occult_when_glob(double jd_start, int body, const char *starname, int flags, int iflag, double *tret, int backward, char *error);

int jme_rise_trans(double jd_ut, int body, const char *starname, int flags, int rsmi, double *geopos, double atpress, double attemp, double *tret, char *error);
int jme_rise_trans_true_hor(double jd_ut, int body, const char *starname, int flags, int rsmi, double *geopos, double atpress, double attemp, double horhgt, double *tret, char *error);

int jme_solcross(double x2cross, double jd_ut, int flags, double *tret, char *error);
int jme_solcross_ut(double x2cross, double jd_ut, int flags, double *tret, char *error);
int jme_mooncross(double x2cross, double jd_ut, int flags, double *tret, char *error);
int jme_mooncross_ut(double x2cross, double jd_ut, int flags, double *tret, char *error);
int jme_mooncross_node(double jd_ut, int flags, double *tret, char *error);
int jme_mooncross_node_ut(double jd_ut, int flags, double *tret, char *error);
int jme_helio_cross(int body, double x2cross, double jd_ut, int flags, double *tret, char *error);
int jme_helio_cross_ut(int body, double x2cross, double jd_ut, int flags, double *tret, char *error);

int jme_nod_aps(double jd_et, int body, int flags, int method, double *tret, char *error);
int jme_nod_aps_ut(double jd_ut, int body, int flags, int method, double *tret, char *error);
int jme_get_orbital_elements(double jd_et, int body, int flags, double *elem, char *error);
int jme_orbit_max_min_true_distance(double jd_et, int body, int flags, double *tmax, double *tmin, double *dmax, double *dmin, char *error);

int jme_heliacal_ut(double jd_ut, double *geopos, double *dat_hel, char *error);
int jme_heliacal_pheno_ut(double jd_ut, double *geopos, double *dat_hel, char *error);
int jme_vis_limit_mag(double jd_ut, double *geopos, double *dat_hel, char *error);
double jme_heliacal_angle(double jd_ut, double *geopos, double *dat_hel, char *error);
double jme_topo_arcus_visionis(double jd_ut, double *geopos, double *dat_hel, char *error);

void jme_set_astro_models(const char *models, int flags);
int jme_get_astro_models(char *models, int flags);
void jme_set_lapse_rate(double lapse_rate);
void jme_set_interpolate_nut(int on);
double jme_get_tid_acc(void);
void jme_set_tid_acc(double t_acc);
void jme_set_delta_t_userdef(double dt);
const char *jme_get_ayanamsa_name(int model);

int jme_lmt_to_lat(double jd_lmt, double geo_lon, double *jd_lat, char *error);
int jme_lat_to_lmt(double jd_lat, double geo_lon, double *jd_lmt, char *error);

char *jme_centiseconds_to_time_string(int cs, char *buffer);
char *jme_centiseconds_to_lonlat_string(int cs, char *buffer);
char *jme_centiseconds_to_degree_string(int cs, char *buffer);

double jme_julian_day(int year, int month, int day, double hour, int calendar);
int jme_calendar_is_leap_year(int year, int calendar);
int jme_date_is_valid(int year, int month, int day, int calendar);
int jme_days_in_month(int year, int month, int calendar);
int jme_day_of_year(int year, int month, int day, int calendar);
double jme_decimal_hour(int hour, int minute, double second);
int jme_day_of_week(double jd);
void jme_reverse_julian_day(
    double jd,
    int calendar,
    int *year,
    int *month,
    int *day,
    double *hour
);

#include "jme/jme_extended.h"

#ifdef __cplusplus
}
#endif

#endif
