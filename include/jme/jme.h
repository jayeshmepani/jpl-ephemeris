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
    JME_BODY_PLUTO_BARYCENTER = 20
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
