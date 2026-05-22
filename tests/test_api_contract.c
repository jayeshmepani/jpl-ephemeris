#define _POSIX_C_SOURCE 200112L

#include "jme/jme.h"

#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static int finite_values(const double *values, int count)
{
    int i;

    for (i = 0; i < count; i++) {
        if (!isfinite(values[i])) {
            return 0;
        }
    }

    return 1;
}

static double contract_altitude_at(
    double jd_ut,
    int body,
    const char *starname,
    int flags,
    double *geopos,
    double atpress,
    double attemp,
    char *error
)
{
    double results[6];
    double azimuth = 0.0;
    double altitude = 0.0;
    double hour_angle;

    jme_set_topo(geopos[0], geopos[1], geopos[2]);

    if (starname != 0 && starname[0] != '\0') {
        if (jme_fixstar_ut(starname, jd_ut, flags | JME_CALC_EQUATORIAL | JME_CALC_TOPOCENTRIC, results, error) != JME_OK) {
            return NAN;
        }
    } else if (jme_calc_ut(jd_ut, body, flags | JME_CALC_EQUATORIAL | JME_CALC_TOPOCENTRIC, results, error) != JME_OK) {
        return NAN;
    }

    hour_angle = jme_degree_normalize(jme_sidereal_time(jd_ut) * 15.0 + geopos[0] - results[0]);
    jme_equatorial_to_horizontal(hour_angle, results[1], geopos[1], &azimuth, &altitude);

    if (atpress > 0.0) {
        double refraction_meta[8];
        altitude = jme_refract_extended(
            altitude,
            geopos[2],
            atpress,
            attemp,
            0.0065,
            JME_COORD_TRUE_TO_APPARENT,
            refraction_meta
        );
    }

    return altitude;
}

static double contract_hour_angle_at(
    double jd_ut,
    int body,
    const char *starname,
    int flags,
    double *geopos,
    double target_hour_angle,
    char *error
)
{
    double results[6];

    jme_set_topo(geopos[0], geopos[1], geopos[2]);

    if (starname != 0 && starname[0] != '\0') {
        if (jme_fixstar_ut(starname, jd_ut, flags | JME_CALC_EQUATORIAL | JME_CALC_TOPOCENTRIC, results, error) != JME_OK) {
            return NAN;
        }
    } else if (jme_calc_ut(jd_ut, body, flags | JME_CALC_EQUATORIAL | JME_CALC_TOPOCENTRIC, results, error) != JME_OK) {
        return NAN;
    }

    return jme_degrees_difference_signed(
        jme_degrees_difference_signed(jme_sidereal_time(jd_ut) * 15.0 + geopos[0], results[0]),
        target_hour_angle
    );
}

int main(void)
{
    char version[64];
    double ra = 0.0;
    double dec = 0.0;
    double lon = 0.0;
    double lat = 0.0;
    double state[6] = {1.0, 1.0, 1.0, 1.0, 1.0, 1.0};
    double order_state[12] = {
        1.0, 1.0, 1.0, 1.0,
        1.0, 1.0, 1.0, 1.0,
        1.0, 1.0, 1.0, 1.0
    };
    double rectangular[6] = {1.0, 1.0, 0.0, 0.0, 1.0, 0.0};
    double spherical[6] = {0.0, 0.0, 0.0, 0.0, 0.0, 0.0};
    double converted[6] = {0.0, 0.0, 0.0, 0.0, 0.0, 0.0};
    double round_trip_state[6] = {0.0, 0.0, 0.0, 0.0, 0.0, 0.0};
    char error[256] = "";
    char name[64] = "";
    char path[256] = "";
    char values[2][64];
    double value = 1.0;
    double vector_values[2] = {1.0, 1.0};
    double first_time = 1.0;
    double last_time = 1.0;
    int y = 0;
    int m = 0;
    int d = 0;
    int h = 0;
    int min = 0;
    int mode = 0;
    int continuous = 1;
    double sec = 0.0;

    if (strcmp(jme_version(version, sizeof(version)), JME_VERSION) != 0) {
        fprintf(stderr, "version return mismatch\n");
        return 1;
    }

    if (strcmp(version, JME_VERSION) != 0) {
        fprintf(stderr, "version buffer mismatch\n");
        return 1;
    }

    jme_set_ephemeris_path(".");
    jme_set_jpl_file("kernel.bsp");
    jme_set_sidereal_mode(JME_SIDEREAL_LAHIRI, 2451545.0, 24.0);
    jme_close();

    if (strcmp(jme_ephemeris_path(), ".") != 0 || strcmp(jme_jpl_file(), "kernel.bsp") != 0) {
        fprintf(stderr, "context path getter mismatch\n");
        return 1;
    }

    jme_set_ephemeris_path(0);
    jme_set_jpl_file(0);
    if (strcmp(jme_ephemeris_path(), "") != 0 || strcmp(jme_jpl_file(), "") != 0) {
        fprintf(stderr, "context null path reset mismatch\n");
        return 1;
    }

    jme_set_ephemeris_path("data");
    jme_set_jpl_file("de440s.bsp");
    if (strcmp(jme_ephemeris_path(), "data") != 0 || strcmp(jme_jpl_file(), "de440s.bsp") != 0) {
        fprintf(stderr, "context path reset mismatch\n");
        return 1;
    }

    jme_get_sidereal_mode(&mode, &value, &sec);
    if (mode != JME_SIDEREAL_LAHIRI || value != 2451545.0 || sec != 24.0) {
        fprintf(stderr, "sidereal mode getter mismatch\n");
        return 1;
    }

    jme_set_sidereal_mode(JME_SIDEREAL_USER, NAN, INFINITY);
    jme_get_sidereal_mode(&mode, &value, &sec);
    if (mode != JME_SIDEREAL_USER || value != 0.0 || sec != 0.0) {
        fprintf(stderr, "sidereal mode sanitization mismatch\n");
        return 1;
    }

    {
        double topo_valid[3];
        double topo_invalid[3];
        double topo_clamped[3];

        jme_set_topo(0.0, 0.0, 0.0);
        if (jme_get_topo_pos(2451545.0, topo_valid, error) != JME_OK || !finite_values(topo_valid, 3)) {
            fprintf(stderr, "valid topocentric state failed: %s\n", error);
            return 1;
        }

        jme_set_topo(NAN, NAN, NAN);
        if (jme_get_topo_pos(2451545.0, topo_invalid, error) != JME_OK || !finite_values(topo_invalid, 3)) {
            fprintf(stderr, "sanitized topocentric state failed: %s\n", error);
            return 1;
        }

        if (fabs(topo_valid[0] - topo_invalid[0]) > 1.0e-15
            || fabs(topo_valid[1] - topo_invalid[1]) > 1.0e-15
            || fabs(topo_valid[2] - topo_invalid[2]) > 1.0e-15) {
            fprintf(stderr, "topocentric invalid-input sanitization mismatch\n");
            return 1;
        }

        jme_set_topo(720.0, 120.0, 0.0);
        if (jme_get_topo_pos(2451545.0, topo_clamped, error) != JME_OK
            || !finite_values(topo_clamped, 3)
            || jme_state_distance(topo_clamped) <= 0.0) {
            fprintf(stderr, "topocentric clamp state mismatch: %s\n", error);
            return 1;
        }

        if (jme_get_topo_pos(2451545.0, 0, error) != JME_ERR) {
            fprintf(stderr, "topocentric position accepted null output\n");
            return 1;
        }
    }

    jme_set_astro_models("IAU2006 NUT2000B DELTAT2016", 0);
    if (jme_get_astro_models(path, 0) != JME_OK
        || strstr(path, "BIAS=331") == 0
        || strstr(path, "NUT=335") == 0
        || strstr(path, "OBL=338") == 0
        || strstr(path, "PREC=341") == 0
        || strstr(path, "SIDT=345") == 0
        || strstr(path, "DELTAT=350") == 0
        || strstr(path, "ENGINE=AUTO") == 0) {
        fprintf(stderr, "astro model profile getter mismatch: %s\n", path);
        return 1;
    }
    if (fabs(jme_sidereal_time(2451545.0) - 18.697374828694365) > 1e-12) {
        fprintf(stderr, "IAU 2006 sidereal time mismatch: %.17g\n", jme_sidereal_time(2451545.0));
        return 1;
    }
    jme_set_astro_models("ENGINE=JPL", 0);
    if (jme_get_astro_models(path, 0) != JME_OK || strstr(path, "ENGINE=JPL") == 0) {
        fprintf(stderr, "JPL engine model summary mismatch: %s\n", path);
        return 1;
    }
    jme_jpl_close();
    if (jme_calc(2451545.0, JME_BODY_MERCURY, JME_CALC_TRUE_POSITION | JME_CALC_HELIOCENTRIC | JME_CALC_XYZ, state, error) != JME_ERR) {
        fprintf(stderr, "ENGINE=JPL unexpectedly used analytical fallback without an open JPL kernel\n");
        return 1;
    }

    jme_set_astro_models("ENGINE=MOSHIER", 0);
    if (jme_get_astro_models(path, 0) != JME_OK || strstr(path, "ENGINE=MOSHIER") == 0) {
        fprintf(stderr, "Moshier engine model summary mismatch: %s\n", path);
        return 1;
    }
    if (jme_calc(2451545.0, JME_BODY_MERCURY, JME_CALC_TRUE_POSITION | JME_CALC_HELIOCENTRIC | JME_CALC_XYZ, state, error) != JME_OK
        || !finite_values(state, 6)) {
        fprintf(stderr, "ENGINE=MOSHIER calculation failed: %s\n", error);
        return 1;
    }

    jme_set_astro_models("ENGINE=VSOP_ELP_MEEUS", 0);
    if (jme_get_astro_models(path, 0) != JME_OK || strstr(path, "ENGINE=VSOP_ELP_MEEUS") == 0) {
        fprintf(stderr, "VSOP/ELP/Meeus engine model summary mismatch: %s\n", path);
        return 1;
    }
    if (jme_calc(2451545.0, JME_BODY_MERCURY, JME_CALC_TRUE_POSITION | JME_CALC_HELIOCENTRIC | JME_CALC_XYZ, state, error) != JME_OK
        || !finite_values(state, 6)) {
        fprintf(stderr, "ENGINE=VSOP_ELP_MEEUS calculation failed: %s\n", error);
        return 1;
    }

#if defined(__unix__) || defined(__APPLE__)
    setenv("JME_ENGINE", "MOSHIER", 1);
    jme_set_astro_models(0, 0);
    if (jme_get_astro_models(path, 0) != JME_OK || strstr(path, "ENGINE=MOSHIER") == 0) {
        fprintf(stderr, "JME_ENGINE environment selection mismatch: %s\n", path);
        return 1;
    }
    unsetenv("JME_ENGINE");
#endif

    jme_set_astro_models(0, 0);
    if (jme_get_astro_models(path, 0) != JME_OK
        || strstr(path, "BIAS=332") == 0
        || strstr(path, "NUT=333") == 0
        || strstr(path, "OBL=336") == 0
        || strstr(path, "PREC=339") == 0
        || strstr(path, "DELTAT=349") == 0) {
        fprintf(stderr, "astro model default getter mismatch: %s\n", path);
        return 1;
    }
    if (jme_get_astro_models(0, 0) != JME_ERR) {
        fprintf(stderr, "astro model getter accepted null output\n");
        return 1;
    }
    {
        const int sidereal_models[] = {
            JME_SIDEREAL_FAGAN_BRADLEY,
            JME_SIDEREAL_LAHIRI,
            JME_SIDEREAL_ALDEBARAN_15TAU,
            JME_SIDEREAL_ARYABHATA,
            JME_SIDEREAL_B1950,
            JME_SIDEREAL_BABYL_ETPSC,
            JME_SIDEREAL_BABYL_HUBER,
            JME_SIDEREAL_BABYL_KUGLER1,
            JME_SIDEREAL_BABYL_KUGLER2,
            JME_SIDEREAL_BABYL_KUGLER3,
            JME_SIDEREAL_DELUCE,
            JME_SIDEREAL_GALCENT_0SAG,
            JME_SIDEREAL_HIPPARCHOS,
            JME_SIDEREAL_J1900,
            JME_SIDEREAL_J2000,
            JME_SIDEREAL_JN_BHASIN,
            JME_SIDEREAL_KRISHNAMURTI,
            JME_SIDEREAL_RAMAN,
            JME_SIDEREAL_SASSANIAN,
            JME_SIDEREAL_SS_CITRA,
            JME_SIDEREAL_SS_REVATI,
            JME_SIDEREAL_SURYASIDDHANTA,
            JME_SIDEREAL_TRUE_CITRA,
            JME_SIDEREAL_TRUE_MULA,
            JME_SIDEREAL_TRUE_PUSHYA,
            JME_SIDEREAL_TRUE_REVATI,
            JME_SIDEREAL_USER,
            JME_SIDEREAL_USHASHASHI,
            JME_SIDEREAL_YUKTESHWAR
        };
        size_t sidereal_index;

        for (sidereal_index = 0; sidereal_index < sizeof(sidereal_models) / sizeof(sidereal_models[0]); sidereal_index++) {
            const char *sidereal_name = jme_get_ayanamsa_name(sidereal_models[sidereal_index]);
            if (sidereal_name == 0 || sidereal_name[0] == '\0' || strcmp(sidereal_name, "Unknown sidereal mode") == 0) {
                fprintf(stderr, "sidereal name coverage mismatch for model %d\n", sidereal_models[sidereal_index]);
                return 1;
            }
        }
        if (strcmp(jme_get_ayanamsa_name(-1), "Unknown sidereal mode") != 0) {
            fprintf(stderr, "unknown sidereal name mismatch\n");
            return 1;
        }
    }
    {
        double ayan_lahiri = 0.0;
        double ayan_fagan = 0.0;
        double ayan_user = 0.0;
        double ayan_ut = 0.0;
        double ayan_epoch = 0.0;
        double ayan_anchor = 0.0;
        double ayan_kp = 0.0;
        double ayan_traditional = 0.0;
        double kp_zero_jd = 2415020.31352 + (291.21645153698 - 1900.0) * 365.242198781;
        double star_ecl[6] = {0.0};

        if (jme_get_ayanamsa_ex(2451545.0, JME_SIDEREAL_LAHIRI, &ayan_lahiri, error) != JME_OK
            || fabs(ayan_lahiri - 23.8570) > 1e-12
            || jme_get_ayanamsa_ex(2451545.0, JME_SIDEREAL_FAGAN_BRADLEY, &ayan_fagan, error) != JME_OK
            || fabs(ayan_fagan - 24.733333333) > 1e-12) {
            fprintf(stderr, "built-in ayanamsa known-value mismatch\n");
            return 1;
        }

        if (jme_get_ayanamsa_ex(2451545.0, JME_SIDEREAL_J2000, &ayan_epoch, error) != JME_OK
            || fabs(ayan_epoch) > 1.0e-12
            || jme_get_ayanamsa_ex(2415020.0, JME_SIDEREAL_J1900, &ayan_epoch, error) != JME_OK
            || fabs(ayan_epoch) > 1.0e-12
            || jme_get_ayanamsa_ex(2433282.42345905, JME_SIDEREAL_B1950, &ayan_epoch, error) != JME_OK
            || fabs(ayan_epoch) > 1.0e-12) {
            fprintf(stderr, "epoch-zero ayanamsa contract mismatch: %s %.17g\n", error, ayan_epoch);
            return 1;
        }

        if (jme_fixstar("Aldebaran", 2451545.0, JME_CALC_TRUE_POSITION, star_ecl, error) != JME_OK
            || jme_get_ayanamsa_ex(2451545.0, JME_SIDEREAL_ALDEBARAN_15TAU, &ayan_anchor, error) != JME_OK
            || fabs(jme_degrees_difference(star_ecl[0] - ayan_anchor, 45.0)) > 1.0e-9
            || jme_fixstar("Spica", 2451545.0, JME_CALC_TRUE_POSITION, star_ecl, error) != JME_OK
            || jme_get_ayanamsa_ex(2451545.0, JME_SIDEREAL_TRUE_CITRA, &ayan_anchor, error) != JME_OK
            || fabs(jme_degrees_difference(star_ecl[0] - ayan_anchor, 180.0)) > 1.0e-9
            || jme_fixstar("Revati", 2451545.0, JME_CALC_TRUE_POSITION, star_ecl, error) != JME_OK
            || jme_get_ayanamsa_ex(2451545.0, JME_SIDEREAL_TRUE_REVATI, &ayan_anchor, error) != JME_OK
            || fabs(jme_degrees_difference(star_ecl[0] - ayan_anchor, 0.0)) > 1.0e-9) {
            fprintf(stderr, "fixed-star anchored ayanamsa contract mismatch: %s %.17g %.17g\n", error, star_ecl[0], ayan_anchor);
            return 1;
        }
        if (jme_fixstar("Shaula", 2451545.0, JME_CALC_TRUE_POSITION, star_ecl, error) != JME_OK
            || jme_get_ayanamsa_ex(2451545.0, JME_SIDEREAL_TRUE_MULA, &ayan_anchor, error) != JME_OK
            || fabs(jme_degrees_difference_signed(star_ecl[0] - ayan_anchor, 240.0)) > 1.0e-9
            || jme_fixstar("Delta Cancri", 2451545.0, JME_CALC_TRUE_POSITION, star_ecl, error) != JME_OK
            || jme_get_ayanamsa_ex(2451545.0, JME_SIDEREAL_TRUE_PUSHYA, &ayan_anchor, error) != JME_OK
            || fabs(jme_degrees_difference_signed(star_ecl[0] - ayan_anchor, 106.0)) > 1.0e-9
            || jme_get_ayanamsa_ex(2451545.0, JME_SIDEREAL_GALCENT_0SAG, &ayan_anchor, error) != JME_OK
            || !isfinite(ayan_anchor)
            || ayan_anchor < 25.0
            || ayan_anchor > 27.0) {
            fprintf(stderr, "new fixed-anchor ayanamsa contract mismatch: %s %.17g %.17g\n", error, star_ecl[0], ayan_anchor);
            return 1;
        }

        jme_set_sidereal_mode(JME_SIDEREAL_USER, 2451545.0, 12.5);
        if (jme_get_ayanamsa_ex(2451545.0, JME_SIDEREAL_USER, &ayan_user, error) != JME_OK
            || fabs(ayan_user - 12.5) > 1e-12
            || fabs(jme_get_ayanamsa(2451545.0) - 12.5) > 1e-12) {
            fprintf(stderr, "user ayanamsa contract mismatch\n");
            return 1;
        }

        jme_set_sidereal_mode(JME_SIDEREAL_LAHIRI, 2451545.0, 23.8570);
        if (jme_get_ayanamsa_ex_ut(2451545.0, JME_SIDEREAL_LAHIRI, &ayan_ut, error) != JME_OK
            || !isfinite(ayan_ut)
            || fabs(ayan_ut - jme_get_ayanamsa_ut(2451545.0)) > 1e-12) {
            fprintf(stderr, "UT ayanamsa wrapper mismatch\n");
            return 1;
        }

        if (jme_get_ayanamsa_ex(kp_zero_jd, JME_SIDEREAL_KRISHNAMURTI, &ayan_kp, error) != JME_OK
            || fabs(jme_degrees_difference_signed(ayan_kp, 0.0)) > 1.0e-8
            || jme_get_ayanamsa_ex(2415020.31352, JME_SIDEREAL_KRISHNAMURTI, &ayan_kp, error) != JME_OK
            || fabs(ayan_kp - (80564.38104 / 3600.0)) > 1.0e-10
            || jme_get_ayanamsa_ex_ut(2451545.0, JME_SIDEREAL_KRISHNAMURTI, &ayan_kp, error) != JME_OK
            || fabs(ayan_kp - (23.0 + 46.0 / 60.0 + 31.19526 / 3600.0)) > 1.0e-7
        ) {
            fprintf(stderr, "Krishnamurti ayanamsa known-value mismatch: %s %.17g\n", error, ayan_kp);
            return 1;
        }

        if (jme_get_ayanamsa_ex(jme_julian_day(1912, 1, 1, 0.0, JME_CALENDAR_GREGORIAN), JME_SIDEREAL_RAMAN, &ayan_traditional, error) != JME_OK
            || fabs(ayan_traditional - (21.0 + 10.0 / 60.0 + 55.0 / 3600.0)) > 1.0e-12
            || jme_get_ayanamsa_ex(jme_julian_day(1918, 1, 1, 0.0, JME_CALENDAR_GREGORIAN), JME_SIDEREAL_RAMAN, &ayan_traditional, error) != JME_OK
            || fabs(ayan_traditional - (21.0 + 15.0 / 60.0 + 57.0 / 3600.0)) > 1.0e-12
            || jme_get_ayanamsa_ex(jme_julian_day(1893, 3, 20, 0.0, JME_CALENDAR_GREGORIAN), JME_SIDEREAL_YUKTESHWAR, &ayan_traditional, error) != JME_OK
            || fabs(ayan_traditional - (20.0 + 54.0 / 60.0 + 36.0 / 3600.0)) > 1.0e-12) {
            fprintf(stderr, "traditional published ayanamsa known-value mismatch: %s %.17g\n", error, ayan_traditional);
            return 1;
        }

        if (jme_get_ayanamsa_ex(2451545.0, JME_SIDEREAL_LAHIRI, 0, error) != JME_ERR
            || jme_get_ayanamsa_ex(2451545.0, JME_SIDEREAL_ARYABHATA, &ayan_user, error) != JME_ERR) {
            fprintf(stderr, "ayanamsa accepted null output or unsupported model\n");
            return 1;
        }
    }
    if (jme_get_nutation(2451545.0, JME_MODEL_NUT_IAU_1980, &ra, &dec, error) != JME_OK
        || fabs(ra - (-0.0038698621712391834)) > 1e-15
        || fabs(dec - (-0.0016041131523510195)) > 1e-15) {
        fprintf(stderr, "IAU 1980 nutation mismatch: %s %.17g %.17g\n", error, ra, dec);
        return 1;
    }
    if (jme_get_nutation(2451545.0, JME_MODEL_NUT_IAU_2000B, &ra, &dec, error) != JME_OK
        || fabs(ra - (-0.0038699066358249398)) > 1e-15
        || fabs(dec - (-0.0016026158548035686)) > 1e-15) {
        fprintf(stderr, "IAU 2000B nutation mismatch: %s %.17g %.17g\n", error, ra, dec);
        return 1;
    }
    if (jme_get_nutation(2451545.0, JME_MODEL_NUT_IAU_2000A, &ra, &dec, error) != JME_ERR
        || jme_get_nutation(2451545.0, 999999, &ra, &dec, error) != JME_ERR
        || jme_get_nutation(2451545.0, JME_MODEL_NUT_IAU_1980, 0, &dec, error) != JME_ERR
        || jme_get_nutation(2451545.0, JME_MODEL_NUT_IAU_1980, &ra, 0, error) != JME_ERR) {
        fprintf(stderr, "nutation accepted unavailable model or null output\n");
        return 1;
    }

    {
        double dp0 = 0.0;
        double de0 = 0.0;
        double dp1 = 0.0;
        double de1 = 0.0;
        double dpi = 0.0;
        double dei = 0.0;

        jme_set_interpolate_nut(0);
        if (jme_get_nutation(2451545.0, JME_MODEL_NUT_IAU_2000B, &dp0, &de0, error) != JME_OK
            || jme_get_nutation(2451546.0, JME_MODEL_NUT_IAU_2000B, &dp1, &de1, error) != JME_OK) {
            fprintf(stderr, "nutation endpoint calculation failed: %s\n", error);
            return 1;
        }

        jme_set_interpolate_nut(1);
        if (jme_get_nutation(2451545.5, JME_MODEL_NUT_IAU_2000B, &dpi, &dei, error) != JME_OK
            || fabs(dpi - (dp0 + dp1) * 0.5) > 1.0e-15
            || fabs(dei - (de0 + de1) * 0.5) > 1.0e-15) {
            fprintf(stderr, "nutation interpolation mismatch: %s %.17g %.17g\n", error, dpi, dei);
            return 1;
        }
        jme_set_interpolate_nut(0);
    }

    {
        double state[6];
        if (jme_moshier_planet_state(2451545.0, JME_BODY_MERCURY, state) != JME_OK
            || !finite_values(state, 6)) {
            fprintf(stderr, "Moshier Mercury state contract mismatch\n");
            return 1;
        }
        if (jme_moshier_planet_state(2451545.0, JME_BODY_MOON, state) != JME_ERR) {
            fprintf(stderr, "Moshier unexpectedly accepted unsupported body\n");
            return 1;
        }
        if (jme_meeus_planet_state(2451545.0, JME_BODY_MARS, state) != JME_OK
            || !finite_values(state, 6)) {
            fprintf(stderr, "Meeus Mars state contract mismatch\n");
            return 1;
        }
        if (jme_meeus_planet_state(2451545.0, JME_BODY_MOON, state) != JME_ERR) {
            fprintf(stderr, "Meeus unexpectedly accepted unsupported body\n");
            return 1;
        }
    }

    if (!jme_calendar_is_leap_year(2000, JME_CALENDAR_GREGORIAN) || jme_calendar_is_leap_year(1900, JME_CALENDAR_GREGORIAN)) {
        fprintf(stderr, "Gregorian leap-year mismatch\n");
        return 1;
    }

    if (jme_days_in_month(2024, 2, JME_CALENDAR_GREGORIAN) != 29 || jme_day_of_year(2024, 3, 1, JME_CALENDAR_GREGORIAN) != 61) {
        fprintf(stderr, "calendar day helper mismatch\n");
        return 1;
    }

    if (fabs(jme_decimal_hour(1, 30, 30.0) - 1.5083333333333333) > 1e-15) {
        fprintf(stderr, "decimal hour mismatch\n");
        return 1;
    }

    if (jme_degree_normalize(-1.0) != 359.0) {
        fprintf(stderr, "degree normalization mismatch\n");
        return 1;
    }

    if (fabs(jme_degrees_to_radians(180.0) - acos(-1.0)) > 1e-15) {
        fprintf(stderr, "degree to radian mismatch\n");
        return 1;
    }

    if (fabs(jme_radians_to_degrees(acos(-1.0)) - 180.0) > 1e-14) {
        fprintf(stderr, "radian to degree mismatch\n");
        return 1;
    }

    if (fabs(jme_hours_normalize(-1.0) - 23.0) > 1e-15) {
        fprintf(stderr, "hour normalization mismatch\n");
        return 1;
    }

    if (fabs(jme_degrees_to_hours(30.0) - 2.0) > 1e-15 || fabs(jme_hours_to_degrees(2.0) - 30.0) > 1e-15) {
        fprintf(stderr, "hour/degree conversion mismatch\n");
        return 1;
    }

    if (jme_degrees_difference_signed(10.0, 350.0) != 20.0) {
        fprintf(stderr, "signed degree difference mismatch\n");
        return 1;
    }

    {
        double bias_none[9];
        double bias_2006[9];
        double p1976[9];
        double p2006[9];
        double p_alt[9];
        double eps1980 = 0.0;
        double eps2000 = 0.0;
        double eps2006 = 0.0;
        double diff = 0.0;
        int i;

        if (jme_get_frame_bias_matrix(JME_MODEL_BIAS_NONE, bias_none) != JME_OK
            || jme_get_frame_bias_matrix(JME_MODEL_BIAS_IAU2006, bias_2006) != JME_OK
            || !finite_values(bias_none, 9)
            || !finite_values(bias_2006, 9)
            || fabs(bias_none[0] - 1.0) > 1e-15
            || fabs(bias_none[4] - 1.0) > 1e-15
            || fabs(bias_none[8] - 1.0) > 1e-15
            || fabs(bias_2006[1]) < 1e-10
            || fabs(bias_2006[2]) < 1e-10
            || fabs(bias_2006[3]) < 1e-10
            || fabs(bias_2006[5]) < 1e-10
            || fabs(bias_2006[6]) < 1e-10
            || fabs(bias_2006[7]) < 1e-10) {
            fprintf(stderr, "frame bias matrix mismatch\n");
            return 1;
        }
        if (jme_get_obliquity(2451545.0 + 36525.0, JME_MODEL_OBL_IAU_2006, &eps2006, error) != JME_OK
            || fabs(eps2006 - 23.426269736300174) > 1e-14) {
            fprintf(stderr, "IAU 2006 obliquity mismatch: %s %.17g\n", error, eps2006);
            return 1;
        }
        if (jme_get_obliquity(2451545.0, JME_MODEL_OBL_IAU_1980, &eps1980, error) != JME_OK
            || jme_get_obliquity(2451545.0, JME_MODEL_OBL_IAU_2000, &eps2000, error) != JME_OK
            || !isfinite(eps1980)
            || !isfinite(eps2000)
            || eps1980 <= 0.0
            || eps2000 <= 0.0) {
            fprintf(stderr, "obliquity model coverage mismatch: %s\n", error);
            return 1;
        }
        if (jme_get_obliquity(2451545.0, -1, &eps1980, error) != JME_ERR
            || jme_get_obliquity(2451545.0, JME_MODEL_OBL_IAU_2006, 0, error) != JME_ERR) {
            fprintf(stderr, "obliquity invalid-input contract mismatch\n");
            return 1;
        }
        if (jme_get_precession_matrix(2451545.0, 2451545.0 + 36525.0, JME_MODEL_PREC_IAU_1976, p1976) != JME_OK
            || jme_get_precession_matrix(2451545.0, 2451545.0 + 36525.0, JME_MODEL_PREC_IAU_2006, p2006) != JME_OK
            || jme_get_precession_matrix(2451545.0, 2451545.0 + 36525.0, JME_MODEL_PREC_LASKAR_1986, p_alt) != JME_OK
            || jme_get_precession_matrix(2451545.0, 2451545.0 + 36525.0, JME_MODEL_PREC_VONDRAK_2011, p_alt) != JME_OK) {
            fprintf(stderr, "precession model coverage mismatch\n");
            return 1;
        }
        if (jme_get_precession_matrix(2451545.0, 2451545.0 + 36525.0, -1, p_alt) != JME_ERR
            || jme_get_precession_matrix(2451545.0, 2451545.0 + 36525.0, JME_MODEL_PREC_IAU_2006, 0) != JME_ERR) {
            fprintf(stderr, "precession invalid-input contract mismatch\n");
            return 1;
        }
        if (!finite_values(p2006, 9)) {
            fprintf(stderr, "IAU 2006 precession matrix is not finite\n");
            return 1;
        }
        for (i = 0; i < 9; i++) {
            diff += fabs(p2006[i] - p1976[i]);
        }
        if (diff <= 1e-8 || fabs(p2006[0] - 1.0) + fabs(p2006[4] - 1.0) + fabs(p2006[8] - 1.0) <= 1e-8) {
            fprintf(stderr, "IAU 2006 precession matrix did not differ from fallback/identity\n");
            return 1;
        }
    }

    {
        double star_default[6];
        double star_2006[6];
        double star_radians[6];
        double star_sidereal[6];
        double star_hr[6];
        double star_future[6];
        double star_mag = 0.0;
        const char *built_in_stars[] = {
            "Sirius",
            "Spica",
            "Regulus",
            "Aldebaran",
            "Antares",
            "Revati"
        };
        size_t star_index;

        jme_set_astro_models(0, 0);
        if (jme_fixstar("Spica", 2451545.0, JME_CALC_TRUE_POSITION | JME_CALC_EQUATORIAL, star_default, error) != JME_OK) {
            fprintf(stderr, "default fixed-star reduction failed: %s\n", error);
            return 1;
        }
        jme_set_astro_models("IAU2006", 0);
        if (jme_fixstar("Spica", 2451545.0, JME_CALC_TRUE_POSITION | JME_CALC_EQUATORIAL, star_2006, error) != JME_OK
            || !finite_values(star_2006, 6)
            || fabs(star_2006[0] - star_default[0]) < 1e-7
            || fabs(star_2006[1] - star_default[1]) < 1e-7) {
            fprintf(stderr, "IAU 2006 bias/precession fixed-star reduction mismatch: %s\n", error);
            return 1;
        }
        jme_set_astro_models(0, 0);

        if (jme_fixstar("Sirius", 2451545.0, JME_CALC_TRUE_POSITION | JME_CALC_EQUATORIAL | JME_CALC_J2000, star_default, error) != JME_OK
            || !finite_values(star_default, 6)
            || fabs(star_default[0] - 101.287083333333) > 1.0e-10
            || fabs(star_default[1] - (-16.716111111111)) > 1.0e-10
            || fabs(star_default[3] - ((-0.553 / 3600.0) / 365.25)) > 1.0e-14
            || fabs(star_default[4] - ((-1.205 / 3600.0) / 365.25)) > 1.0e-14) {
            fprintf(stderr, "Sirius fixed-star catalog contract mismatch: %s %.17g %.17g\n", error, star_default[0], star_default[1]);
            return 1;
        }
        if (jme_fixstar("HR 2491", 2451545.0, JME_CALC_TRUE_POSITION | JME_CALC_EQUATORIAL | JME_CALC_J2000, star_hr, error) != JME_OK
            || fabs(star_hr[0] - star_default[0]) > 1.0e-12
            || fabs(star_hr[1] - star_default[1]) > 1.0e-12
            || jme_fixstar("HD48915", 2451545.0, JME_CALC_TRUE_POSITION | JME_CALC_EQUATORIAL | JME_CALC_J2000, star_hr, error) != JME_OK
            || fabs(star_hr[0] - star_default[0]) > 1.0e-12
            || fabs(star_hr[1] - star_default[1]) > 1.0e-12
            || jme_fixstar("SAO 151881", 2451545.0, JME_CALC_TRUE_POSITION | JME_CALC_EQUATORIAL | JME_CALC_J2000, star_hr, error) != JME_OK
            || fabs(star_hr[0] - star_default[0]) > 1.0e-12
            || fabs(star_hr[1] - star_default[1]) > 1.0e-12
            || jme_fixstar("9Alp CMa", 2451545.0, JME_CALC_TRUE_POSITION | JME_CALC_EQUATORIAL | JME_CALC_J2000, star_hr, error) != JME_OK
            || fabs(star_hr[0] - star_default[0]) > 1.0e-12
            || fabs(star_hr[1] - star_default[1]) > 1.0e-12
            || jme_fixstar("sirius", 2451545.0, JME_CALC_TRUE_POSITION | JME_CALC_EQUATORIAL | JME_CALC_J2000, star_hr, error) != JME_OK
            || fabs(star_hr[0] - star_default[0]) > 1.0e-12
            || fabs(star_hr[1] - star_default[1]) > 1.0e-12) {
            fprintf(stderr, "fixed-star identifier/alias lookup mismatch: %s\n", error);
            return 1;
        }
        if (jme_fixstar("HR 1", 2451545.0, JME_CALC_TRUE_POSITION | JME_CALC_EQUATORIAL | JME_CALC_J2000, star_hr, error) != JME_OK
            || !finite_values(star_hr, 6)
            || jme_fixstar("HR 9110", 2451545.0, JME_CALC_TRUE_POSITION | JME_CALC_EQUATORIAL | JME_CALC_J2000, star_hr, error) != JME_OK
            || !finite_values(star_hr, 6)) {
            fprintf(stderr, "fixed-star broad catalog endpoint lookup mismatch: %s\n", error);
            return 1;
        }
        if (jme_fixstar("Sirius", 2451545.0 + 36525.0, JME_CALC_TRUE_POSITION | JME_CALC_EQUATORIAL | JME_CALC_J2000, star_future, error) != JME_OK
            || fabs(star_future[0] - star_default[0]) < 0.001
            || fabs(star_future[1] - star_default[1]) < 0.001) {
            fprintf(stderr, "fixed-star proper-motion contract mismatch: %s %.17g %.17g\n", error, star_future[0], star_future[1]);
            return 1;
        }
        if (jme_fixstar_ut("Sirius", 2451545.0, JME_CALC_TRUE_POSITION | JME_CALC_EQUATORIAL, star_default, error) != JME_OK
            || !finite_values(star_default, 6)) {
            fprintf(stderr, "Sirius UT fixed-star contract mismatch: %s\n", error);
            return 1;
        }
        if (jme_fixstar_mag("Sirius", &star_mag, error) != JME_OK
            || fabs(star_mag - (-1.46)) > 1e-12
            || jme_fixstar2("Sirius", 2451545.0, JME_CALC_TRUE_POSITION | JME_CALC_EQUATORIAL, star_default, error) != JME_OK
            || jme_fixstar2_ut("Sirius", 2451545.0, JME_CALC_TRUE_POSITION | JME_CALC_EQUATORIAL, star_default, error) != JME_OK
            || jme_fixstar2_mag("Sirius", &star_mag, error) != JME_OK) {
            fprintf(stderr, "Sirius alternate fixed-star contract mismatch: %s\n", error);
            return 1;
        }
        for (star_index = 0; star_index < sizeof(built_in_stars) / sizeof(built_in_stars[0]); star_index++) {
            if (jme_fixstar(built_in_stars[star_index], 2451545.0, JME_CALC_TRUE_POSITION | JME_CALC_EQUATORIAL, star_default, error) != JME_OK
                || jme_fixstar_mag(built_in_stars[star_index], &star_mag, error) != JME_OK
                || !finite_values(star_default, 6)
                || !isfinite(star_mag)) {
                fprintf(stderr, "built-in fixed-star catalog breadth mismatch for %s: %s\n", built_in_stars[star_index], error);
                return 1;
            }
        }

        if (jme_fixstar("Sirius", 2451545.0, JME_CALC_TRUE_POSITION | JME_CALC_EQUATORIAL, star_default, error) != JME_OK
            || jme_fixstar("Sirius", 2451545.0, JME_CALC_TRUE_POSITION | JME_CALC_EQUATORIAL | JME_CALC_RADIANS, star_radians, error) != JME_OK
            || fabs(star_radians[0] - star_default[0] * acos(-1.0) / 180.0) > 1e-12
            || fabs(star_radians[1] - star_default[1] * acos(-1.0) / 180.0) > 1e-12) {
            fprintf(stderr, "fixed-star radians contract mismatch\n");
            return 1;
        }

        if (jme_fixstar("Sirius", 2451545.0, JME_CALC_TRUE_POSITION, star_default, error) != JME_OK
            || jme_fixstar("Sirius", 2451545.0, JME_CALC_TRUE_POSITION | JME_CALC_SIDEREAL, star_sidereal, error) != JME_OK
            || fabs(jme_degrees_difference(star_sidereal[0], star_default[0] - jme_get_ayanamsa(2451545.0))) > 1e-9
            || fabs(star_sidereal[3] - star_default[3]) < 1.0e-7) {
            fprintf(stderr, "fixed-star sidereal contract mismatch\n");
            return 1;
        }

        if (jme_fixstar("Sirius", 2451545.0, JME_CALC_TRUE_POSITION, 0, error) != JME_ERR
            || jme_fixstar(0, 2451545.0, JME_CALC_TRUE_POSITION, star_default, error) != JME_ERR
            || jme_fixstar("", 2451545.0, JME_CALC_TRUE_POSITION, star_default, error) != JME_ERR
            || jme_fixstar_ut(0, 2451545.0, JME_CALC_TRUE_POSITION, star_default, error) != JME_ERR
            || jme_fixstar2(0, 2451545.0, JME_CALC_TRUE_POSITION, star_default, error) != JME_ERR
            || jme_fixstar2_ut(0, 2451545.0, JME_CALC_TRUE_POSITION, star_default, error) != JME_ERR
            || jme_fixstar_mag("Sirius", 0, error) != JME_ERR
            || jme_fixstar_mag(0, &star_mag, error) != JME_ERR
            || jme_fixstar2_mag(0, &star_mag, error) != JME_ERR
            || jme_fixstar("UnknownStar", 2451545.0, JME_CALC_TRUE_POSITION, star_default, error) != JME_ERR
            || jme_fixstar("HR 999999", 2451545.0, JME_CALC_TRUE_POSITION, star_default, error) != JME_ERR
            || jme_fixstar("HD 999999999", 2451545.0, JME_CALC_TRUE_POSITION, star_default, error) != JME_ERR
            || jme_fixstar("SAO X", 2451545.0, JME_CALC_TRUE_POSITION, star_default, error) != JME_ERR) {
            fprintf(stderr, "fixed-star invalid-input contract mismatch\n");
            return 1;
        }
    }

    if (jme_centiseconds_normalize(-1) != 129599999) {
        fprintf(stderr, "centisecond normalization mismatch\n");
        return 1;
    }

    if (jme_double_to_long(-1.5) != -2 || jme_double_to_long(1.5) != 2) {
        fprintf(stderr, "double-to-long rounding mismatch\n");
        return 1;
    }

    if (strcmp(jme_body_name(JME_BODY_SUN), "Sun") != 0 || jme_body_id_from_name("Mars") != JME_BODY_MARS) {
        fprintf(stderr, "body metadata mismatch\n");
        return 1;
    }

    if (jme_copy_body_name(JME_BODY_MOON, name) == 0 || strcmp(name, "Moon") != 0) {
        fprintf(stderr, "body name copy mismatch\n");
        return 1;
    }

    if (jme_body_naif_id(JME_BODY_EARTH) != 399 || jme_body_naif_id(JME_BODY_MOON) != 301) {
        fprintf(stderr, "NAIF metadata mismatch\n");
        return 1;
    }

    if (
        jme_body_naif_id(JME_BODY_SOLAR_SYSTEM_BARYCENTER) != 0 ||
        jme_body_naif_id(JME_BODY_EARTH_MOON_BARYCENTER) != 3 ||
        jme_body_naif_id(JME_BODY_JUPITER_BARYCENTER) != 5
    ) {
        fprintf(stderr, "NAIF barycenter metadata mismatch\n");
        return 1;
    }

    if (strcmp(jme_house_system_name('P'), "Placidus") != 0 || strcmp(jme_house_system_name('K'), "Koch") != 0) {
        fprintf(stderr, "house system name mismatch\n");
        return 1;
    }

    if (fabs(jme_sidereal_time(2451545.0) - 18.697374558) > 1e-9) {
        fprintf(stderr, "sidereal time mismatch: %.17g\n", jme_sidereal_time(2451545.0));
        return 1;
    }

    jme_ecliptic_to_equatorial(120.0, 5.0, 23.4392911, &ra, &dec);
    jme_equatorial_to_ecliptic(ra, dec, 23.4392911, &lon, &lat);
    if (fabs(jme_degrees_difference_signed(lon, 120.0)) > 1e-10 || fabs(lat - 5.0) > 1e-10) {
        fprintf(stderr, "coordinate round-trip mismatch: %.17g %.17g\n", lon, lat);
        return 1;
    }

    if (fabs(jme_spherical_angular_separation(0.0, 0.0, 90.0, 0.0) - 90.0) > 1e-12) {
        fprintf(stderr, "spherical angular separation mismatch\n");
        return 1;
    }

    if (fabs(jme_spherical_position_angle(0.0, 0.0, 90.0, 0.0) - 90.0) > 1e-12) {
        fprintf(stderr, "spherical position angle mismatch\n");
        return 1;
    }

    if (jme_ecliptic_to_equatorial_rectangular_state(rectangular, 23.4392911, converted) != JME_OK) {
        fprintf(stderr, "ecliptic rectangular state conversion failed\n");
        return 1;
    }

    if (jme_equatorial_to_ecliptic_rectangular_state(converted, 23.4392911, round_trip_state) != JME_OK) {
        fprintf(stderr, "equatorial rectangular state conversion failed\n");
        return 1;
    }

    if (
        fabs(round_trip_state[0] - rectangular[0]) > 1e-12 ||
        fabs(round_trip_state[1] - rectangular[1]) > 1e-12 ||
        fabs(round_trip_state[2] - rectangular[2]) > 1e-12 ||
        fabs(round_trip_state[3] - rectangular[3]) > 1e-12 ||
        fabs(round_trip_state[4] - rectangular[4]) > 1e-12 ||
        fabs(round_trip_state[5] - rectangular[5]) > 1e-12
    ) {
        fprintf(stderr, "rectangular obliquity state round-trip mismatch\n");
        return 1;
    }

    jme_equatorial_to_horizontal(45.0, 20.0, 23.5, &lon, &lat);
    jme_horizontal_to_equatorial(lon, lat, 23.5, &ra, &dec);
    if (fabs(jme_degrees_difference_signed(ra, 45.0)) > 1e-10 || fabs(dec - 20.0) > 1e-10) {
        fprintf(stderr, "horizontal coordinate round-trip mismatch: %.17g %.17g\n", ra, dec);
        return 1;
    }

    if (jme_refract(15.0, 1010.0, 10.0, JME_COORD_TRUE_TO_APPARENT) <= 15.0) {
        fprintf(stderr, "refraction did not increase true altitude\n");
        return 1;
    }

    if (jme_rectangular_to_spherical_state(rectangular, spherical) != JME_OK) {
        fprintf(stderr, "rectangular to spherical state failed\n");
        return 1;
    }

    if (
        fabs(spherical[0] - 45.0) > 1e-12 ||
        fabs(spherical[1]) > 1e-12 ||
        fabs(spherical[2] - sqrt(2.0)) > 1e-12 ||
        fabs(spherical[3] - (0.5 * 57.295779513082320876798154814105170332405472466565)) > 1e-12 ||
        fabs(spherical[4]) > 1e-12 ||
        fabs(spherical[5] - (1.0 / sqrt(2.0))) > 1e-12
    ) {
        fprintf(stderr, "rectangular to spherical state mismatch\n");
        return 1;
    }

    if (jme_spherical_to_rectangular_state(spherical, round_trip_state) != JME_OK) {
        fprintf(stderr, "spherical to rectangular state failed\n");
        return 1;
    }

    if (
        fabs(round_trip_state[0] - rectangular[0]) > 1e-12 ||
        fabs(round_trip_state[1] - rectangular[1]) > 1e-12 ||
        fabs(round_trip_state[2] - rectangular[2]) > 1e-12 ||
        fabs(round_trip_state[3] - rectangular[3]) > 1e-12 ||
        fabs(round_trip_state[4] - rectangular[4]) > 1e-12 ||
        fabs(round_trip_state[5] - rectangular[5]) > 1e-12
    ) {
        fprintf(stderr, "spherical rectangular round-trip mismatch\n");
        return 1;
    }

    if (fabs(jme_state_distance(rectangular) - sqrt(2.0)) > 1e-12) {
        fprintf(stderr, "state distance mismatch\n");
        return 1;
    }

    if (fabs(jme_state_speed(rectangular) - 1.0) > 1e-12) {
        fprintf(stderr, "state speed mismatch\n");
        return 1;
    }

    if (fabs(jme_state_position_velocity_dot(rectangular) - 1.0) > 1e-12) {
        fprintf(stderr, "state position-velocity dot mismatch\n");
        return 1;
    }

    if (jme_state_add(rectangular, rectangular, round_trip_state) != JME_OK || fabs(round_trip_state[0] - 2.0) > 1e-12) {
        fprintf(stderr, "state add mismatch\n");
        return 1;
    }

    if (jme_state_subtract(round_trip_state, rectangular, round_trip_state) != JME_OK || fabs(round_trip_state[0] - 1.0) > 1e-12) {
        fprintf(stderr, "state subtract mismatch\n");
        return 1;
    }

    if (jme_state_scale(rectangular, 3.0, round_trip_state) != JME_OK || fabs(round_trip_state[1] - 3.0) > 1e-12) {
        fprintf(stderr, "state scale mismatch\n");
        return 1;
    }

    if (jme_state_convert_units(rectangular, JME_VECTOR_AU_PER_DAY, JME_VECTOR_KM_PER_SECOND, converted) != JME_OK) {
        fprintf(stderr, "state conversion failed\n");
        return 1;
    }

    if (
        fabs(converted[0] - JME_AU_KM) > 1e-6 ||
        fabs(converted[1] - JME_AU_KM) > 1e-6 ||
        fabs(converted[2]) > 1e-12 ||
        fabs(converted[3]) > 1e-12 ||
        fabs(converted[4] - (JME_AU_KM / JME_SECONDS_PER_DAY)) > 1e-9 ||
        fabs(converted[5]) > 1e-12
    ) {
        fprintf(stderr, "state conversion mismatch\n");
        return 1;
    }

    if (fabs(jme_state_light_time_days(rectangular, JME_VECTOR_AU_PER_DAY) - (sqrt(2.0) * JME_AU_KM / JME_SPEED_OF_LIGHT_KM_PER_SEC / JME_SECONDS_PER_DAY)) > 1e-15) {
        fprintf(stderr, "light-time mismatch\n");
        return 1;
    }

    jme_utc_time_zone(2026, 5, 18, 10, 30, 0.0, 5.5, &y, &m, &d, &h, &min, &sec);
    if (y != 2026 || m != 5 || d != 18 || h != 5 || min != 0 || sec > 0.000001) {
        fprintf(stderr, "timezone conversion mismatch: %d-%d-%d %d:%d %.17g\n", y, m, d, h, min, sec);
        return 1;
    }

    if (fabs(jme_jd_difference_seconds(jme_jd_add_seconds(2451545.0, 90.0), 2451545.0) - 90.0) > 1e-5) {
        fprintf(stderr, "JD seconds helper mismatch\n");
        return 1;
    }

    if (fabs(jme_delta_t(2451545.0) - 63.8735) > 0.001) {
        fprintf(stderr, "delta_t mismatch at J2000: %.17g\n", jme_delta_t(2451545.0));
        return 1;
    }

    {
        int models[] = {
            JME_TIME_DELTAT_AUTOMATIC,
            JME_MODEL_DELTAT_STEPHENSON_MORRISON_1984,
            JME_MODEL_DELTAT_STEPHENSON_1997,
            JME_MODEL_DELTAT_STEPHENSON_MORRISON_2004,
            JME_MODEL_DELTAT_ESPENAK_MEEUS_2006,
            JME_MODEL_DELTAT_STEPHENSON_ETC_2016
        };
        int i;

        for (i = 0; i < (int)(sizeof(models) / sizeof(models[0])); i++) {
            double dt = jme_delta_t_ex(2451545.0, models[i], error);
            if (!isfinite(dt) || fabs(dt - 63.8735) > 0.001) {
                fprintf(stderr, "Delta-T model mismatch for %d: %s %.17g\n", models[i], error, dt);
                return 1;
            }
        }

        if (isfinite(jme_delta_t_ex(2451545.0, -999, error)) || strstr(error, "Unsupported Delta-T model") == 0) {
            fprintf(stderr, "unsupported Delta-T model was not rejected: %s\n", error);
            return 1;
        }
    }

    {
        double default_dt = jme_delta_t(2451545.0);

        jme_set_tid_acc(-25.8);
        if (fabs(jme_get_tid_acc() - -25.8) > 1.0e-12) {
            fprintf(stderr, "tidal acceleration state mismatch\n");
            return 1;
        }

        jme_set_tid_acc(NAN);
        if (fabs(jme_get_tid_acc() - -25.8) > 1.0e-12) {
            fprintf(stderr, "tidal acceleration accepted non-finite state\n");
            return 1;
        }

        jme_set_delta_t_userdef(64.0);
        if (fabs(jme_delta_t(2451545.0) - 64.0) > 1.0e-12
            || fabs(jme_delta_t_ex(2451545.0, JME_MODEL_DELTAT_STEPHENSON_1997, error) - 64.0) > 1.0e-12) {
            fprintf(stderr, "user Delta-T override mismatch\n");
            return 1;
        }

        jme_set_delta_t_userdef(NAN);
        if (fabs(jme_delta_t(2451545.0) - default_dt) > 1.0e-12) {
            fprintf(stderr, "user Delta-T reset mismatch\n");
            return 1;
        }
    }

    {
        double eot = 0.0;
        double jd_lmt = 2451545.25;
        double jd_lat = 0.0;
        double round_trip_lmt = 0.0;

        if (jme_time_equ(2451545.0, &eot, error) != JME_OK || !isfinite(eot) || fabs(eot) > 0.02) {
            fprintf(stderr, "equation-of-time contract mismatch: %s %.17g\n", error, eot);
            return 1;
        }

        if (jme_time_equ(2451545.0, 0, error) != JME_ERR) {
            fprintf(stderr, "equation-of-time accepted null output\n");
            return 1;
        }

        if (jme_lmt_to_lat(jd_lmt, 77.0, &jd_lat, error) != JME_OK
            || jme_lat_to_lmt(jd_lat, 77.0, &round_trip_lmt, error) != JME_OK
            || fabs(round_trip_lmt - jd_lmt) > 1.0e-9) {
            fprintf(stderr, "LAT/LMT round-trip mismatch: %s %.17g %.17g\n", error, jd_lmt, round_trip_lmt);
            return 1;
        }

        if (jme_lmt_to_lat(jd_lmt, 77.0, 0, error) != JME_ERR
            || jme_lat_to_lmt(jd_lat, 77.0, 0, error) != JME_ERR) {
            fprintf(stderr, "LAT/LMT accepted null output\n");
            return 1;
        }
    }

    {
        double sea[4];
        double high[4];
        double sea_alt = jme_refract_extended(10.0, 0.0, 1013.25, 15.0, 0.0065, JME_COORD_TRUE_TO_APPARENT, sea);
        double high_alt = jme_refract_extended(10.0, 2000.0, 1013.25, 15.0, 0.0065, JME_COORD_TRUE_TO_APPARENT, high);

        if (!finite_values(sea, 4) || !finite_values(high, 4) || !isfinite(sea_alt) || !isfinite(high_alt)) {
            fprintf(stderr, "extended refraction produced non-finite values\n");
            return 1;
        }

        if (!(high[2] < sea[2] && high[3] < sea[3] && high_alt < sea_alt)) {
            fprintf(stderr, "extended refraction altitude/lapse behavior mismatch\n");
            return 1;
        }
    }

    if (JME_CALC_NO_ABERRATION != (1 << 10)
        || JME_CALC_NO_LIGHT_DEFLECTION != (1 << 11)
        || JME_CALC_TOPOCENTRIC != (1 << 17)
        || JME_RISE_RISE != (1 << 0)
        || JME_RISE_SET != (1 << 1)
        || JME_RISE_MERIDIAN_TRANSIT != (1 << 2)) {
        fprintf(stderr, "bit-flag constant contract mismatch\n");
        return 1;
    }

    {
        int bodies[] = {
            JME_BODY_SUN,
            JME_BODY_MOON,
            JME_BODY_MEAN_NODE,
            JME_BODY_TRUE_NODE,
            JME_BODY_MERCURY,
            JME_BODY_VENUS,
            JME_BODY_MARS,
            JME_BODY_JUPITER,
            JME_BODY_SATURN,
            JME_BODY_URANUS,
            JME_BODY_NEPTUNE,
            JME_BODY_PLUTO
        };
        int body_count = (int)(sizeof(bodies) / sizeof(bodies[0]));
        int body_index;

        if (jme_calc(2451545.0, JME_BODY_SUN, JME_CALC_NONE, 0, error) != JME_ERR) {
            fprintf(stderr, "jme_calc unexpectedly accepted a null output buffer\n");
            return 1;
        }

        for (body_index = 0; body_index < body_count; body_index++) {
            double results[6] = {0.0, 0.0, 0.0, 0.0, 0.0, 0.0};
            int rc = jme_calc_ut(2451545.0, bodies[body_index], JME_CALC_NONE, results, error);

            if (rc == JME_OK && !finite_values(results, 6)) {
                fprintf(stderr, "jme_calc_ut returned non-finite success for body %d\n", bodies[body_index]);
                return 1;
            }
        }

        {
            double deg_results[6] = {0.0, 0.0, 0.0, 0.0, 0.0, 0.0};
            double rad_results[6] = {0.0, 0.0, 0.0, 0.0, 0.0, 0.0};
            double au_results[6] = {0.0, 0.0, 0.0, 0.0, 0.0, 0.0};
            double km_results[6] = {0.0, 0.0, 0.0, 0.0, 0.0, 0.0};
            double sidereal_results[6] = {0.0, 0.0, 0.0, 0.0, 0.0, 0.0};

            if (jme_calc_ut(2451545.0, JME_BODY_MERCURY, JME_CALC_TRUE_POSITION, deg_results, error) == JME_OK
                && jme_calc_ut(2451545.0, JME_BODY_MERCURY, JME_CALC_TRUE_POSITION | JME_CALC_RADIANS, rad_results, error) == JME_OK
                && fabs(rad_results[0] - deg_results[0] * 3.14159265358979323846 / 180.0) > 1e-12) {
                fprintf(stderr, "radian output flag mismatch\n");
                return 1;
            }

            if (jme_calc_ut(2451545.0, JME_BODY_MERCURY, JME_CALC_TRUE_POSITION, au_results, error) == JME_OK
                && jme_calc_ut(2451545.0, JME_BODY_MERCURY, JME_CALC_TRUE_POSITION | JME_CALC_DISTANCE_KM, km_results, error) == JME_OK
                && fabs(km_results[2] - au_results[2] * JME_AU_KM) > 1e-3) {
                fprintf(stderr, "kilometer output flag mismatch\n");
                return 1;
            }

            if (jme_calc_ut(2451545.0, JME_BODY_MERCURY, JME_CALC_TRUE_POSITION | JME_CALC_XYZ, au_results, error) == JME_OK
                && jme_calc_ut(2451545.0, JME_BODY_MERCURY, JME_CALC_TRUE_POSITION | JME_CALC_XYZ | JME_CALC_VELOCITY_PER_SECOND, km_results, error) == JME_OK
                && (fabs(km_results[3] - au_results[3] / JME_SECONDS_PER_DAY) > 1e-12
                    || fabs(km_results[4] - au_results[4] / JME_SECONDS_PER_DAY) > 1e-12
                    || fabs(km_results[5] - au_results[5] / JME_SECONDS_PER_DAY) > 1e-12)) {
                fprintf(stderr, "velocity-per-second output flag mismatch\n");
                return 1;
            }

            if (jme_calc_ut(2451545.0, JME_BODY_MERCURY, JME_CALC_TRUE_POSITION, deg_results, error) == JME_OK
                && jme_calc_ut(2451545.0, JME_BODY_MERCURY, JME_CALC_TRUE_POSITION | JME_CALC_SIDEREAL, sidereal_results, error) == JME_OK
                && fabs(jme_degrees_difference(sidereal_results[0], deg_results[0] - jme_get_ayanamsa_ut(2451545.0))) > 1e-9) {
                fprintf(stderr, "sidereal output flag mismatch\n");
                return 1;
            }
        }

        {
            double mean_node[6] = {0.0, 0.0, 0.0, 0.0, 0.0, 0.0};
            double true_node[6] = {0.0, 0.0, 0.0, 0.0, 0.0, 0.0};
            double pluto[6] = {0.0, 0.0, 0.0, 0.0, 0.0, 0.0};
            double pctr_xyz[6] = {0.0, 0.0, 0.0, 0.0, 0.0, 0.0};
            double pctr_sph_deg[6] = {0.0, 0.0, 0.0, 0.0, 0.0, 0.0};
            double pctr_sph_rad[6] = {0.0, 0.0, 0.0, 0.0, 0.0, 0.0};
            double pctr_sph_sidereal[6] = {0.0, 0.0, 0.0, 0.0, 0.0, 0.0};
            double mercury_helio[6] = {0.0, 0.0, 0.0, 0.0, 0.0, 0.0};
            double sun_helio[6] = {0.0, 0.0, 0.0, 0.0, 0.0, 0.0};

            if (jme_calc_ut(2451545.0, JME_BODY_MEAN_NODE, JME_CALC_TRUE_POSITION, mean_node, error) != JME_OK
                || jme_calc_ut(2451545.0, JME_BODY_TRUE_NODE, JME_CALC_TRUE_POSITION, true_node, error) != JME_OK) {
                fprintf(stderr, "lunar node calculation failed: %s\n", error);
                return 1;
            }

            if (!finite_values(mean_node, 6)
                || !finite_values(true_node, 6)
                || mean_node[0] < 0.0
                || mean_node[0] >= 360.0) {
                fprintf(stderr, "lunar node output mismatch\n");
                return 1;
            }

            if (jme_calc_ut(2451545.0, JME_BODY_PLUTO, JME_CALC_TRUE_POSITION, pluto, error) != JME_OK
                || !finite_values(pluto, 6)) {
                fprintf(stderr, "analytic fallback Pluto calculation failed: %s\n", error);
                return 1;
            }

            if (jme_calc_ut(2451545.0, JME_BODY_PLUTO, JME_CALC_TRUE_POSITION | JME_CALC_XYZ, pluto, error) != JME_OK
                || !finite_values(pluto, 6)) {
                fprintf(stderr, "analytic fallback Pluto rectangular contract failed: %s\n", error);
                return 1;
            }

            if (jme_calc_ut(2451545.0, JME_BODY_PLUTO, JME_CALC_TRUE_POSITION | JME_CALC_DISTANCE_KM, pluto, error) != JME_OK
                || !isfinite(pluto[2])
                || pluto[2] <= 0.0) {
                fprintf(stderr, "analytic fallback Pluto distance contract failed: %s\n", error);
                return 1;
            }

            if (jme_calc_pctr(2451545.0, JME_BODY_MERCURY, JME_BODY_SUN, JME_CALC_TRUE_POSITION | JME_CALC_XYZ, pctr_xyz, error) != JME_OK
                || jme_calc(2451545.0, JME_BODY_MERCURY, JME_CALC_TRUE_POSITION | JME_CALC_HELIOCENTRIC | JME_CALC_XYZ, mercury_helio, error) != JME_OK
                || jme_calc(2451545.0, JME_BODY_SUN, JME_CALC_TRUE_POSITION | JME_CALC_HELIOCENTRIC | JME_CALC_XYZ, sun_helio, error) != JME_OK
                || fabs(pctr_xyz[0] - (mercury_helio[0] - sun_helio[0])) > 1e-12
                || fabs(pctr_xyz[1] - (mercury_helio[1] - sun_helio[1])) > 1e-12
                || fabs(pctr_xyz[2] - (mercury_helio[2] - sun_helio[2])) > 1e-12) {
                fprintf(stderr, "planet-center rectangular contract mismatch: %s\n", error);
                return 1;
            }

            if (jme_calc_pctr(2451545.0, JME_BODY_MERCURY, JME_BODY_SUN, JME_CALC_TRUE_POSITION, pctr_sph_deg, error) != JME_OK
                || jme_calc_pctr(2451545.0, JME_BODY_MERCURY, JME_BODY_SUN, JME_CALC_TRUE_POSITION | JME_CALC_RADIANS, pctr_sph_rad, error) != JME_OK
                || fabs(pctr_sph_rad[0] - pctr_sph_deg[0] * 3.14159265358979323846 / 180.0) > 1e-12
                || fabs(pctr_sph_rad[1] - pctr_sph_deg[1] * 3.14159265358979323846 / 180.0) > 1e-12) {
                fprintf(stderr, "planet-center radians contract mismatch: %s\n", error);
                return 1;
            }

            if (jme_calc_pctr(2451545.0, JME_BODY_MERCURY, JME_BODY_SUN, JME_CALC_TRUE_POSITION | JME_CALC_SIDEREAL, pctr_sph_sidereal, error) != JME_OK
                || fabs(jme_degrees_difference(pctr_sph_sidereal[0], pctr_sph_deg[0] - jme_get_ayanamsa(2451545.0))) > 1e-9) {
                fprintf(stderr, "planet-center sidereal contract mismatch: %s\n", error);
                return 1;
            }

            if (jme_calc_pctr(2451545.0, JME_BODY_MERCURY, JME_BODY_SUN, JME_CALC_TRUE_POSITION | JME_CALC_DISTANCE_KM, pctr_sph_deg, error) != JME_OK
                || fabs(pctr_sph_deg[2] - jme_state_distance(mercury_helio) * JME_AU_KM) > 1e-6) {
                fprintf(stderr, "planet-center distance contract mismatch: %s\n", error);
                return 1;
            }

            if (jme_calc(2451545.0, 999, JME_CALC_TRUE_POSITION, pluto, error) != JME_ERR
                || jme_calc_ut(2451545.0, 999, JME_CALC_TRUE_POSITION, pluto, error) != JME_ERR
                || jme_calc_pctr(2451545.0, JME_BODY_MERCURY, JME_BODY_SUN, JME_CALC_TRUE_POSITION, 0, error) != JME_ERR
                || jme_calc_pctr(2451545.0, 999, JME_BODY_SUN, JME_CALC_TRUE_POSITION, pctr_sph_deg, error) != JME_ERR
                || jme_calc_pctr(2451545.0, JME_BODY_MERCURY, 999, JME_CALC_TRUE_POSITION, pctr_sph_deg, error) != JME_ERR) {
                fprintf(stderr, "calculation pipeline accepted invalid body/output arguments\n");
                return 1;
            }
        }
    }

    {
        double geopos[3] = {0.0, 51.5, 0.0};
        double rise_time = 0.0;
        double set_time = 0.0;
        double high_horizon_rise = 0.0;
        double anti_transit = 0.0;
        double civil_rise = 0.0;
        double no_refraction_rise = 0.0;
        double no_refraction_set = 0.0;
        double meridian_transit = 0.0;
        double moon_rise = 0.0;
        double mars_set = 0.0;
        double sirius_rise = 0.0;
        double sirius_transit = 0.0;
        double sector = 0.0;
        double equator_geopos[3] = {77.2, 0.0, 0.0};
        double polar_geopos[3] = {0.0, 89.0, 0.0};
        double residual = 0.0;

        if (jme_rise_trans(2451545.0, JME_BODY_SUN, 0, JME_CALC_TRUE_POSITION, JME_RISE_RISE, geopos, 1010.0, 10.0, &rise_time, error) != JME_OK
            || rise_time < 2451545.0
            || rise_time > 2451546.0) {
            fprintf(stderr, "rise time outside search day\n");
            return 1;
        }

        if (jme_rise_trans(2451545.0, JME_BODY_SUN, 0, JME_CALC_TRUE_POSITION, JME_RISE_SET, geopos, 1010.0, 10.0, &set_time, error) != JME_OK
            || set_time < 2451545.0
            || set_time > 2451546.0) {
            fprintf(stderr, "set time outside search day\n");
            return 1;
        }

        if (jme_rise_trans_true_hor(2451545.0, JME_BODY_SUN, 0, JME_CALC_TRUE_POSITION, JME_RISE_RISE, geopos, 1010.0, 10.0, 5.0, &high_horizon_rise, error) != JME_OK
            || high_horizon_rise <= rise_time) {
            fprintf(stderr, "true horizon rise did not shift after standard rise\n");
            return 1;
        }

        if (jme_rise_trans(2451545.0, JME_BODY_SUN, 0, JME_CALC_TRUE_POSITION, JME_RISE_ANTI_MERIDIAN_TRANSIT, geopos, 1010.0, 10.0, &anti_transit, error) != JME_OK
            || anti_transit < 2451545.0
            || anti_transit > 2451546.0) {
            fprintf(stderr, "anti-meridian transit outside search day\n");
            return 1;
        }

        if (jme_rise_trans(2451545.0, JME_BODY_SUN, 0, JME_CALC_TRUE_POSITION, JME_RISE_RISE | JME_RISE_CIVIL_TWILIGHT, geopos, 1010.0, 10.0, &civil_rise, error) != JME_OK
            || civil_rise >= rise_time) {
            fprintf(stderr, "civil twilight rise did not precede standard rise\n");
            return 1;
        }

        if (jme_rise_trans(2451545.0, JME_BODY_SUN, 0, JME_CALC_TRUE_POSITION, JME_RISE_RISE | JME_RISE_NO_REFRACTION | JME_RISE_DISC_CENTER, geopos, 1010.0, 10.0, &no_refraction_rise, error) != JME_OK
            || no_refraction_rise <= rise_time) {
            fprintf(stderr, "no-refraction disc-center rise did not shift after standard rise\n");
            return 1;
        }

        if (jme_rise_trans(2451545.0, JME_BODY_SUN, 0, JME_CALC_TRUE_POSITION, JME_RISE_SET | JME_RISE_NO_REFRACTION | JME_RISE_DISC_CENTER, geopos, 0.0, 10.0, &no_refraction_set, error) != JME_OK
            || no_refraction_set < 2451545.0
            || no_refraction_set > 2451546.0) {
            fprintf(stderr, "no-refraction disc-center set outside search day\n");
            return 1;
        }

        residual = contract_altitude_at(no_refraction_rise, JME_BODY_SUN, 0, JME_CALC_TRUE_POSITION, geopos, 0.0, 10.0, error);
        if (!isfinite(residual) || fabs(residual) > 0.02) {
            fprintf(stderr, "sun no-refraction rise altitude residual too large: %.12f\n", residual);
            return 1;
        }

        residual = contract_altitude_at(no_refraction_set, JME_BODY_SUN, 0, JME_CALC_TRUE_POSITION, geopos, 0.0, 10.0, error);
        if (!isfinite(residual) || fabs(residual) > 0.02) {
            fprintf(stderr, "sun no-refraction set altitude residual too large: %.12f\n", residual);
            return 1;
        }

        residual = contract_altitude_at(high_horizon_rise, JME_BODY_SUN, 0, JME_CALC_TRUE_POSITION, geopos, 1010.0, 10.0, error);
        if (!isfinite(residual) || fabs(residual - 5.0) > 0.02) {
            fprintf(stderr, "true-horizon altitude residual too large: %.12f\n", residual);
            return 1;
        }

        residual = contract_altitude_at(civil_rise, JME_BODY_SUN, 0, JME_CALC_TRUE_POSITION, geopos, 1010.0, 10.0, error);
        if (!isfinite(residual) || fabs(residual + 6.0) > 0.02) {
            fprintf(stderr, "civil twilight altitude residual too large: %.12f\n", residual);
            return 1;
        }

        if (jme_rise_trans(2451545.0, JME_BODY_SUN, 0, JME_CALC_TRUE_POSITION, JME_RISE_MERIDIAN_TRANSIT, geopos, 0.0, 10.0, &meridian_transit, error) != JME_OK
            || meridian_transit < 2451545.0
            || meridian_transit > 2451546.0) {
            fprintf(stderr, "meridian transit outside search day\n");
            return 1;
        }

        residual = contract_hour_angle_at(meridian_transit, JME_BODY_SUN, 0, JME_CALC_TRUE_POSITION, geopos, 0.0, error);
        if (!isfinite(residual) || fabs(residual) > 0.02) {
            fprintf(stderr, "meridian transit hour-angle residual too large: %.12f\n", residual);
            return 1;
        }

        residual = contract_hour_angle_at(anti_transit, JME_BODY_SUN, 0, JME_CALC_TRUE_POSITION, geopos, 180.0, error);
        if (!isfinite(residual) || fabs(residual) > 0.02) {
            fprintf(stderr, "anti-meridian transit hour-angle residual too large: %.12f\n", residual);
            return 1;
        }

        if (jme_rise_trans(2451545.0, JME_BODY_MOON, 0, JME_CALC_TRUE_POSITION, JME_RISE_RISE | JME_RISE_NO_REFRACTION | JME_RISE_DISC_CENTER, equator_geopos, 0.0, 10.0, &moon_rise, error) != JME_OK
            || moon_rise < 2451545.0
            || moon_rise > 2451546.0) {
            fprintf(stderr, "moon rise outside search day\n");
            return 1;
        }

        residual = contract_altitude_at(moon_rise, JME_BODY_MOON, 0, JME_CALC_TRUE_POSITION, equator_geopos, 0.0, 10.0, error);
        if (!isfinite(residual) || fabs(residual) > 0.05) {
            fprintf(stderr, "moon rise altitude residual too large: %.12f\n", residual);
            return 1;
        }

        if (jme_rise_trans(2451545.0, JME_BODY_MARS, 0, JME_CALC_TRUE_POSITION, JME_RISE_SET | JME_RISE_NO_REFRACTION | JME_RISE_DISC_CENTER, equator_geopos, 0.0, 10.0, &mars_set, error) != JME_OK
            || mars_set < 2451545.0
            || mars_set > 2451546.0) {
            fprintf(stderr, "mars set outside search day\n");
            return 1;
        }

        residual = contract_altitude_at(mars_set, JME_BODY_MARS, 0, JME_CALC_TRUE_POSITION, equator_geopos, 0.0, 10.0, error);
        if (!isfinite(residual) || fabs(residual) > 0.03) {
            fprintf(stderr, "mars set altitude residual too large: %.12f\n", residual);
            return 1;
        }

        if (jme_rise_trans(2451545.0, JME_BODY_SUN, "Sirius", JME_CALC_TRUE_POSITION, JME_RISE_RISE | JME_RISE_NO_REFRACTION | JME_RISE_DISC_CENTER, equator_geopos, 0.0, 10.0, &sirius_rise, error) != JME_OK
            || sirius_rise < 2451545.0
            || sirius_rise > 2451546.0) {
            fprintf(stderr, "Sirius rise outside search day\n");
            return 1;
        }

        residual = contract_altitude_at(sirius_rise, JME_BODY_SUN, "Sirius", JME_CALC_TRUE_POSITION, equator_geopos, 0.0, 10.0, error);
        if (!isfinite(residual) || fabs(residual) > 0.02) {
            fprintf(stderr, "Sirius rise altitude residual too large: %.12f\n", residual);
            return 1;
        }

        if (jme_rise_trans(2451545.0, JME_BODY_SUN, "Sirius", JME_CALC_TRUE_POSITION, JME_RISE_MERIDIAN_TRANSIT, equator_geopos, 0.0, 10.0, &sirius_transit, error) != JME_OK
            || sirius_transit < 2451545.0
            || sirius_transit > 2451546.0) {
            fprintf(stderr, "Sirius transit outside search day\n");
            return 1;
        }

        residual = contract_hour_angle_at(sirius_transit, JME_BODY_SUN, "Sirius", JME_CALC_TRUE_POSITION, equator_geopos, 0.0, error);
        if (!isfinite(residual) || fabs(residual) > 0.02) {
            fprintf(stderr, "Sirius transit hour-angle residual too large: %.12f\n", residual);
            return 1;
        }

        if (jme_rise_trans(2451545.0, JME_BODY_SUN, 0, JME_CALC_TRUE_POSITION, JME_RISE_RISE, 0, 1010.0, 10.0, &rise_time, error) != JME_ERR
            || jme_rise_trans(2451545.0, JME_BODY_SUN, 0, JME_CALC_TRUE_POSITION, JME_RISE_RISE, geopos, 1010.0, 10.0, 0, error) != JME_ERR
            || jme_rise_trans_true_hor(2451545.0, JME_BODY_SUN, 0, JME_CALC_TRUE_POSITION, JME_RISE_RISE, 0, 1010.0, 10.0, 5.0, &rise_time, error) != JME_ERR
            || jme_rise_trans_true_hor(2451545.0, JME_BODY_SUN, 0, JME_CALC_TRUE_POSITION, JME_RISE_RISE, geopos, 1010.0, 10.0, 5.0, 0, error) != JME_ERR) {
            fprintf(stderr, "rise/transit accepted null output arguments\n");
            return 1;
        }

        if (jme_rise_trans(2451545.0, 999, 0, JME_CALC_TRUE_POSITION, JME_RISE_RISE, geopos, 0.0, 10.0, &rise_time, error) != JME_ERR
            || jme_rise_trans(2451545.0, JME_BODY_SUN, "UnknownStar", JME_CALC_TRUE_POSITION, JME_RISE_RISE, geopos, 0.0, 10.0, &rise_time, error) != JME_ERR
            || jme_rise_trans(2451545.0, JME_BODY_SUN, 0, JME_CALC_TRUE_POSITION, 0, geopos, 0.0, 10.0, &rise_time, error) != JME_ERR
            || jme_rise_trans(2451545.0, JME_BODY_SUN, 0, JME_CALC_TRUE_POSITION, JME_RISE_RISE | JME_RISE_NO_REFRACTION | JME_RISE_DISC_CENTER, polar_geopos, 0.0, 10.0, &rise_time, error) != JME_ERR) {
            fprintf(stderr, "rise/transit accepted unsupported body/star/mode/no-event case\n");
            return 1;
        }

        {
            double sector_with_lat = 0.0;
            double sector_without_lat = 0.0;
            double sector_disk_center = 0.0;
            double sector_refracted = 0.0;

            if (jme_gauquelin_sector(2451545.0, JME_BODY_SUN, 0, JME_CALC_TRUE_POSITION, 0, geopos, 1010.0, 10.0, &sector_with_lat, error) != JME_OK
                || jme_gauquelin_sector(2451545.0, JME_BODY_SUN, 0, JME_CALC_TRUE_POSITION, 1, geopos, 1010.0, 10.0, &sector_without_lat, error) != JME_OK
                || jme_gauquelin_sector(2451545.0, JME_BODY_SUN, 0, JME_CALC_TRUE_POSITION, 2, geopos, 1010.0, 10.0, &sector_disk_center, error) != JME_OK
                || jme_gauquelin_sector(2451545.0, JME_BODY_SUN, 0, JME_CALC_TRUE_POSITION, 3, geopos, 1010.0, 10.0, &sector_refracted, error) != JME_OK
                || sector_with_lat < 1.0
                || sector_with_lat >= 37.0
                || sector_without_lat < 1.0
                || sector_without_lat >= 37.0
                || sector_disk_center < 1.0
                || sector_disk_center >= 37.0
                || sector_refracted < 1.0
                || sector_refracted >= 37.0
                || !isfinite(sector_with_lat)
                || !isfinite(sector_without_lat)
                || !isfinite(sector_disk_center)
                || !isfinite(sector_refracted)) {
                fprintf(stderr, "Gauquelin sector method contract mismatch: %s\n", error);
                return 1;
            }

            if (fabs(sector_disk_center - floor(sector_disk_center)) < 1e-9
                || fabs(sector_refracted - sector_disk_center) < 1e-8) {
                fprintf(stderr, "Gauquelin event methods did not return fractional/refraction-sensitive sectors\n");
                return 1;
            }
        }

        if (jme_gauquelin_sector(2451545.0, JME_BODY_SUN, 0, JME_CALC_TRUE_POSITION, 4, geopos, 1010.0, 10.0, &sector, error) != JME_ERR) {
            fprintf(stderr, "Gauquelin sector accepted unsupported method\n");
            return 1;
        }
    }

    {
        double cross = 0.0;
        double cross_ut = 0.0;
        double moon_cross = 0.0;
        double moon_cross_ut = 0.0;
        double node_cross = 0.0;
        double node_cross_ut = 0.0;
        double helio_cross = 0.0;
        double helio_cross_ut = 0.0;
        double sun_at_cross[6] = {0.0};
        double moon_at_cross[6] = {0.0};
        double moon_at_node[6] = {0.0};
        double mars_helio_start[6] = {0.0};
        double mars_at_cross[6] = {0.0};
        double mars_target = 0.0;

        if (jme_solcross(280.0, 2451545.0, JME_CALC_TRUE_POSITION, &cross, error) != JME_OK
            || jme_solcross_ut(640.0, 2451545.0, JME_CALC_TRUE_POSITION, &cross_ut, error) != JME_OK
            || jme_calc_ut(cross, JME_BODY_SUN, JME_CALC_TRUE_POSITION, sun_at_cross, error) != JME_OK
            || fabs(jme_degrees_difference_signed(sun_at_cross[0], 280.0)) > 1e-6
            || fabs(cross - cross_ut) > 1e-8) {
            fprintf(stderr, "solar longitude crossing refinement mismatch\n");
            return 1;
        }

        if (jme_mooncross(300.0, 2451545.0, JME_CALC_TRUE_POSITION, &moon_cross, error) != JME_OK
            || jme_mooncross_ut(660.0, 2451545.0, JME_CALC_TRUE_POSITION, &moon_cross_ut, error) != JME_OK
            || jme_calc_ut(moon_cross, JME_BODY_MOON, JME_CALC_TRUE_POSITION, moon_at_cross, error) != JME_OK
            || fabs(jme_degrees_difference_signed(moon_at_cross[0], 300.0)) > 1e-6
            || fabs(moon_cross - moon_cross_ut) > 1e-8) {
            fprintf(stderr, "lunar longitude crossing refinement mismatch\n");
            return 1;
        }

        if (jme_mooncross_node(2451545.0, JME_CALC_TRUE_POSITION, &node_cross, error) != JME_OK
            || jme_mooncross_node_ut(2451545.0, JME_CALC_TRUE_POSITION, &node_cross_ut, error) != JME_OK
            || jme_calc_ut(node_cross, JME_BODY_MOON, JME_CALC_TRUE_POSITION, moon_at_node, error) != JME_OK
            || fabs(moon_at_node[1]) > 1e-6
            || fabs(node_cross - node_cross_ut) > 1e-8) {
            fprintf(stderr, "lunar node crossing refinement mismatch\n");
            return 1;
        }

        if (jme_calc_ut(2451545.0, JME_BODY_MARS, JME_CALC_TRUE_POSITION | JME_CALC_HELIOCENTRIC, mars_helio_start, error) != JME_OK) {
            fprintf(stderr, "Mars heliocentric start calculation failed: %s\n", error);
            return 1;
        }
        mars_target = jme_degree_normalize(mars_helio_start[0] + 5.0);
        if (jme_helio_cross(JME_BODY_MARS, mars_target, 2451545.0, JME_CALC_TRUE_POSITION, &helio_cross, error) != JME_OK
            || jme_helio_cross_ut(JME_BODY_MARS, mars_target + 360.0, 2451545.0, JME_CALC_TRUE_POSITION, &helio_cross_ut, error) != JME_OK
            || jme_calc_ut(helio_cross, JME_BODY_MARS, JME_CALC_TRUE_POSITION | JME_CALC_HELIOCENTRIC, mars_at_cross, error) != JME_OK
            || fabs(jme_degrees_difference_signed(mars_at_cross[0], mars_target)) > 1e-6
            || fabs(helio_cross - helio_cross_ut) > 1e-8) {
            fprintf(stderr, "heliocentric longitude crossing refinement mismatch\n");
            return 1;
        }

        if (jme_solcross(280.0, 2451545.0, JME_CALC_TRUE_POSITION, 0, error) != JME_ERR
            || jme_mooncross(300.0, 2451545.0, JME_CALC_TRUE_POSITION, 0, error) != JME_ERR
            || jme_mooncross_node(2451545.0, JME_CALC_TRUE_POSITION, 0, error) != JME_ERR
            || jme_helio_cross(JME_BODY_MARS, mars_target, 2451545.0, JME_CALC_TRUE_POSITION, 0, error) != JME_ERR) {
            fprintf(stderr, "crossing output validation mismatch\n");
            return 1;
        }
    }

    {
        double cusps_equal[13] = {0.0};
        double cusps_placidus[13] = {0.0};
        double cusps_porphyrius[13] = {0.0};
        double cusps_morinus[13] = {0.0};
        double cusps_regiomontanus[13] = {0.0};
        double cusps_campanus[13] = {0.0};
        double cusps_koch[13] = {0.0};
        double cusps_krusinski[13] = {0.0};
        double cusps_apc[13] = {0.0};
        double cusps_sunshine[13] = {0.0};
        double cusps_alcabitius[13] = {0.0};
        double cusps_topocentric[13] = {0.0};
        double cusps_horizontal[13] = {0.0};
        double cusps_speed[13] = {0.0};
        double ascmc[10] = {0.0};
        double ascmc_speed[10] = {0.0};
        double house_geopos[3] = {0.0, 51.5, 0.0};
        double house_sector = 0.0;

        if (jme_houses(2451545.0, 51.5, 0.0, JME_HOUSE_EQUAL, cusps_equal, ascmc) != JME_OK
            || !finite_values(cusps_equal + 1, 12)
            || !finite_values(ascmc, 10)
            || fabs(jme_degrees_difference(cusps_equal[1], ascmc[0])) > 1e-12) {
            fprintf(stderr, "numeric JME equal house contract mismatch\n");
            return 1;
        }

        if (jme_houses(2451545.0, 51.5, 0.0, JME_HOUSE_PLACIDUS, cusps_placidus, ascmc) != JME_OK
            || !finite_values(cusps_placidus + 1, 12)
            || fabs(jme_degrees_difference(cusps_placidus[1], ascmc[0])) > 1e-12
            || fabs(jme_degrees_difference(cusps_placidus[10], ascmc[1])) > 1e-12
            || fabs(jme_degrees_difference(cusps_placidus[7], cusps_placidus[1] + 180.0)) > 1e-12
            || fabs(jme_degrees_difference(cusps_placidus[4], cusps_placidus[10] + 180.0)) > 1e-12) {
            fprintf(stderr, "Placidus house contract mismatch\n");
            return 1;
        }

        if (jme_houses(2451545.0, 51.5, 0.0, JME_HOUSE_PORPHYRIUS, cusps_porphyrius, ascmc) != JME_OK
            || !finite_values(cusps_porphyrius + 1, 12)) {
            fprintf(stderr, "Porphyrius house contract mismatch\n");
            return 1;
        }

        if (jme_houses(2451545.0, 51.5, 0.0, JME_HOUSE_MORINUS, cusps_morinus, ascmc) != JME_OK
            || !finite_values(cusps_morinus + 1, 12)
            || fabs(jme_degrees_difference(cusps_morinus[10], ascmc[1])) > 1e-12) {
            fprintf(stderr, "Morinus house contract mismatch\n");
            return 1;
        }

        if (jme_houses(2451545.0, 51.5, 0.0, JME_HOUSE_REGIOMONTANUS, cusps_regiomontanus, ascmc) != JME_OK
            || !finite_values(cusps_regiomontanus + 1, 12)
            || fabs(jme_degrees_difference(cusps_regiomontanus[1], ascmc[0])) > 1e-12
            || fabs(jme_degrees_difference(cusps_regiomontanus[10], ascmc[1])) > 1e-12
            || fabs(jme_degrees_difference(cusps_regiomontanus[7], cusps_regiomontanus[1] + 180.0)) > 1e-12
            || fabs(jme_degrees_difference(cusps_regiomontanus[4], cusps_regiomontanus[10] + 180.0)) > 1e-12) {
            fprintf(stderr, "Regiomontanus house contract mismatch\n");
            return 1;
        }

        if (jme_houses(2451545.0, 51.5, 0.0, JME_HOUSE_CAMPANUS, cusps_campanus, ascmc) != JME_OK
            || !finite_values(cusps_campanus + 1, 12)
            || fabs(jme_degrees_difference(cusps_campanus[1], ascmc[0])) > 1e-12
            || fabs(jme_degrees_difference(cusps_campanus[10], ascmc[1])) > 1e-12
            || fabs(jme_degrees_difference(cusps_campanus[7], cusps_campanus[1] + 180.0)) > 1e-12
            || fabs(jme_degrees_difference(cusps_campanus[4], cusps_campanus[10] + 180.0)) > 1e-12) {
            fprintf(stderr, "Campanus house contract mismatch\n");
            return 1;
        }

        if (jme_houses(2451545.0, 51.5, 0.0, JME_HOUSE_KOCH, cusps_koch, ascmc) != JME_OK
            || !finite_values(cusps_koch + 1, 12)
            || fabs(jme_degrees_difference(cusps_koch[1], ascmc[0])) > 1e-12
            || fabs(jme_degrees_difference(cusps_koch[10], ascmc[1])) > 1e-12
            || fabs(jme_degrees_difference(cusps_koch[7], cusps_koch[1] + 180.0)) > 1e-12
            || fabs(jme_degrees_difference(cusps_koch[4], cusps_koch[10] + 180.0)) > 1e-12) {
            fprintf(stderr, "Koch house contract mismatch\n");
            return 1;
        }

        if (jme_houses(2451545.0, 51.5, 0.0, JME_HOUSE_KRUSINSKI, cusps_krusinski, ascmc) != JME_OK
            || !finite_values(cusps_krusinski + 1, 12)
            || fabs(jme_degrees_difference(cusps_krusinski[1], ascmc[0])) > 1e-12
            || fabs(jme_degrees_difference(cusps_krusinski[10], ascmc[1])) > 1e-12
            || fabs(jme_degrees_difference(cusps_krusinski[7], cusps_krusinski[1] + 180.0)) > 1e-12
            || fabs(jme_degrees_difference(cusps_krusinski[4], cusps_krusinski[10] + 180.0)) > 1e-12) {
            fprintf(stderr, "Krusinski house contract mismatch\n");
            return 1;
        }

        if (jme_houses(2451545.0, 51.5, 0.0, JME_HOUSE_APC, cusps_apc, ascmc) != JME_OK
            || !finite_values(cusps_apc + 1, 12)
            || fabs(jme_degrees_difference(cusps_apc[1], ascmc[0])) > 1e-12
            || fabs(jme_degrees_difference(cusps_apc[10], ascmc[1])) > 1e-12
            || fabs(jme_degrees_difference(cusps_apc[7], cusps_apc[1] + 180.0)) > 1e-12
            || fabs(jme_degrees_difference(cusps_apc[4], cusps_apc[10] + 180.0)) > 1e-12) {
            fprintf(stderr, "APC house contract mismatch\n");
            return 1;
        }

        if (jme_houses(2451545.0, 51.5, 0.0, JME_HOUSE_SUNSHINE, cusps_sunshine, ascmc) != JME_OK
            || !finite_values(cusps_sunshine + 1, 12)
            || fabs(jme_degrees_difference(cusps_sunshine[1], ascmc[0])) > 1e-12
            || fabs(jme_degrees_difference(cusps_sunshine[10], ascmc[1])) > 1e-12
            || fabs(jme_degrees_difference(cusps_sunshine[7], cusps_sunshine[1] + 180.0)) > 1e-12
            || fabs(jme_degrees_difference(cusps_sunshine[4], cusps_sunshine[10] + 180.0)) > 1e-12) {
            fprintf(stderr, "Sunshine house contract mismatch\n");
            return 1;
        }

        ascmc[9] = NAN;
        if (jme_houses_armc(120.0, 51.5, 23.4392911, JME_HOUSE_SUNSHINE, cusps_sunshine, ascmc) != JME_ERR) {
            fprintf(stderr, "ARMC-only Sunshine unexpectedly succeeded without Sun declination\n");
            return 1;
        }
        ascmc[9] = 22.0;
        if (jme_houses_armc(120.0, 51.5, 23.4392911, JME_HOUSE_SUNSHINE, cusps_sunshine, ascmc) != JME_OK
            || !finite_values(cusps_sunshine + 1, 12)
            || fabs(jme_degrees_difference(cusps_sunshine[1], ascmc[0])) > 1e-12
            || fabs(jme_degrees_difference(cusps_sunshine[10], ascmc[1])) > 1e-12
            || fabs(jme_degrees_difference(cusps_sunshine[7], cusps_sunshine[1] + 180.0)) > 1e-12
            || fabs(jme_degrees_difference(cusps_sunshine[4], cusps_sunshine[10] + 180.0)) > 1e-12
            || fabs(ascmc[9] - 22.0) > 1e-12) {
            fprintf(stderr, "ARMC Sunshine with Sun declination mismatch\n");
            return 1;
        }

        if (jme_houses(2451545.0, 51.5, 0.0, JME_HOUSE_ALCABITIUS, cusps_alcabitius, ascmc) != JME_OK
            || !finite_values(cusps_alcabitius + 1, 12)
            || fabs(jme_degrees_difference(cusps_alcabitius[1], ascmc[0])) > 1e-12
            || fabs(jme_degrees_difference(cusps_alcabitius[10], ascmc[1])) > 1e-12
            || fabs(jme_degrees_difference(cusps_alcabitius[7], cusps_alcabitius[1] + 180.0)) > 1e-12
            || fabs(jme_degrees_difference(cusps_alcabitius[4], cusps_alcabitius[10] + 180.0)) > 1e-12) {
            fprintf(stderr, "Alcabitius house contract mismatch\n");
            return 1;
        }

        if (jme_houses(2451545.0, 51.5, 0.0, JME_HOUSE_POLICH_PAGE, cusps_topocentric, ascmc) != JME_OK
            || !finite_values(cusps_topocentric + 1, 12)
            || fabs(jme_degrees_difference(cusps_topocentric[1], ascmc[0])) > 1e-12
            || fabs(jme_degrees_difference(cusps_topocentric[10], ascmc[1])) > 1e-12
            || fabs(jme_degrees_difference(cusps_topocentric[7], cusps_topocentric[1] + 180.0)) > 1e-12
            || fabs(jme_degrees_difference(cusps_topocentric[4], cusps_topocentric[10] + 180.0)) > 1e-12) {
            fprintf(stderr, "Polich/Page house contract mismatch\n");
            return 1;
        }

        if (jme_houses(2451545.0, 51.5, 0.0, JME_HOUSE_MERIDIAN, cusps_morinus, ascmc) != JME_OK
            || !finite_values(cusps_morinus + 1, 12)
            || fabs(jme_degrees_difference(cusps_morinus[10], ascmc[1])) > 1e-12) {
            fprintf(stderr, "Meridian house contract mismatch\n");
            return 1;
        }

        if (jme_houses(2451545.0, 51.5, 0.0, JME_HOUSE_HORIZONTAL, cusps_horizontal, ascmc) != JME_OK
            || !finite_values(cusps_horizontal + 1, 12)
            || fabs(jme_degrees_difference(cusps_horizontal[1], ascmc[0])) > 1e-12
            || fabs(jme_degrees_difference(cusps_horizontal[10], ascmc[1])) > 1e-12
            || fabs(jme_degrees_difference(cusps_horizontal[7], cusps_horizontal[1] + 180.0)) > 1e-12) {
            fprintf(stderr, "Horizontal house contract mismatch\n");
            return 1;
        }

        if (jme_houses(2451545.0, 51.5, 0.0, JME_HOUSE_AZIMUTHAL, cusps_horizontal, ascmc) != JME_OK
            || !finite_values(cusps_horizontal + 1, 12)
            || fabs(jme_degrees_difference(cusps_horizontal[1], ascmc[0])) > 1e-12
            || fabs(jme_degrees_difference(cusps_horizontal[10], ascmc[1])) > 1e-12) {
            fprintf(stderr, "Azimuthal house contract mismatch\n");
            return 1;
        }

        if (jme_houses(2451545.0, 51.5, 0.0, 999999, cusps_equal, ascmc) != JME_ERR) {
            fprintf(stderr, "unknown house system unexpectedly succeeded\n");
            return 1;
        }

        if (jme_houses(2451545.0, 51.5, 0.0, JME_HOUSE_EQUAL, 0, ascmc) != JME_ERR
            || jme_houses_ex(2451545.0, JME_CALC_NONE, 51.5, 0.0, JME_HOUSE_EQUAL, 0, ascmc) != JME_ERR
            || jme_houses_ex2(2451545.0, JME_CALC_NONE, 51.5, 0.0, JME_HOUSE_EQUAL, 0, ascmc, cusps_speed, ascmc_speed) != JME_ERR
            || jme_houses_armc(120.0, 51.5, 23.4392911, JME_HOUSE_EQUAL, 0, ascmc) != JME_ERR
            || jme_houses_armc_ex2(120.0, 51.5, 23.4392911, JME_HOUSE_EQUAL, 0, ascmc, cusps_speed, ascmc_speed) != JME_ERR) {
            fprintf(stderr, "house functions accepted null cusp output\n");
            return 1;
        }

        if (jme_house_pos(0.0, 0.0, 23.4392911, JME_HOUSE_EQUAL, 0, error) != 0.0) {
            fprintf(stderr, "house position accepted null input\n");
            return 1;
        }

        if (jme_houses_ex2(2451545.0, JME_CALC_NONE, 51.5, 0.0, JME_HOUSE_PLACIDUS, cusps_placidus, ascmc, cusps_speed, ascmc_speed) != JME_OK
            || !finite_values(cusps_speed + 1, 12)
            || !finite_values(ascmc_speed, 10)
            || fabs(cusps_speed[1]) + fabs(cusps_speed[10]) <= 0.0
            || fabs(ascmc_speed[0]) + fabs(ascmc_speed[1]) <= 0.0) {
            fprintf(stderr, "house speed contract mismatch\n");
            return 1;
        }

        if (jme_houses_armc_ex2(120.0, 51.5, 23.4392911, JME_HOUSE_EQUAL, cusps_equal, ascmc, cusps_speed, ascmc_speed) != JME_OK
            || !finite_values(cusps_speed + 1, 12)
            || !finite_values(ascmc_speed, 10)
            || fabs(cusps_speed[1]) + fabs(ascmc_speed[0]) <= 0.0) {
            fprintf(stderr, "ARMC house speed contract mismatch\n");
            return 1;
        }

        if (jme_gauquelin_sector(2451545.0, JME_BODY_SUN, 0, JME_CALC_TRUE_POSITION, 0, 0, 1010.0, 10.0, &house_sector, error) != JME_ERR
            || jme_gauquelin_sector(2451545.0, JME_BODY_SUN, 0, JME_CALC_TRUE_POSITION, 0, house_geopos, 1010.0, 10.0, 0, error) != JME_ERR) {
            fprintf(stderr, "Gauquelin sector accepted null output arguments\n");
            return 1;
        }
    }

    {
        double geopos[3] = {-100.0, 40.0, 0.0};
        double tret[10] = {0.0};
        double attr[20] = {0.0};
        double solar_start = jme_julian_day(2024, 4, 7, 0.0, JME_CALENDAR_GREGORIAN);
        double solar_total_max = jme_julian_day(2024, 4, 8, 18.0 + 18.0 / 60.0 + 29.0 / 3600.0, JME_CALENDAR_GREGORIAN);
        double dallas_total_max = jme_julian_day(2024, 4, 8, 18.0 + 42.0 / 60.0 + 38.0 / 3600.0, JME_CALENDAR_GREGORIAN);
        double albuquerque_geopos[3] = {-106.6504, 35.0844, 0.0};
        double lunar_visible_geopos[3] = {-100.0, 40.0, 0.0};
        double annular_start = jme_julian_day(2023, 10, 14, 0.0, JME_CALENDAR_GREGORIAN);
        double annular_global_max = jme_julian_day(2023, 10, 14, 18.0 + 0.0 / 60.0 + 41.0 / 3600.0, JME_CALENDAR_GREGORIAN);
        double albuquerque_annular_max = jme_julian_day(2023, 10, 14, 16.0 + 37.0 / 60.0, JME_CALENDAR_GREGORIAN);
        double partial_solar_start = jme_julian_day(2022, 10, 25, 0.0, JME_CALENDAR_GREGORIAN);
        double partial_solar_max = jme_julian_day(2022, 10, 25, 11.0 + 0.0 / 60.0 + 9.0 / 3600.0, JME_CALENDAR_GREGORIAN);
        double hybrid_start = jme_julian_day(2023, 4, 20, 0.0, JME_CALENDAR_GREGORIAN);
        double hybrid_global_max = jme_julian_day(2023, 4, 20, 4.0 + 17.0 / 60.0 + 56.0 / 3600.0, JME_CALENDAR_GREGORIAN);
        double lunar_total_start = jme_julian_day(2022, 5, 15, 0.0, JME_CALENDAR_GREGORIAN);
        double lunar_total_max = jme_julian_day(2022, 5, 16, 4.0 + 11.0 / 60.0 + 28.0 / 3600.0, JME_CALENDAR_GREGORIAN);
        double lunar_penumbral_start = jme_julian_day(2024, 3, 25, 0.0, JME_CALENDAR_GREGORIAN);
        double lunar_penumbral_max = jme_julian_day(2024, 3, 25, 7.0 + 12.0 / 60.0 + 49.0 / 3600.0, JME_CALENDAR_GREGORIAN);
        double lunar_partial_start = jme_julian_day(2023, 10, 28, 0.0, JME_CALENDAR_GREGORIAN);
        double lunar_partial_max = jme_julian_day(2023, 10, 28, 20.0 + 14.0 / 60.0 + 3.9 / 3600.0, JME_CALENDAR_GREGORIAN);
        double mars_occult_start = jme_julian_day(2022, 12, 8, 0.0, JME_CALENDAR_GREGORIAN);
        double los_angeles_geopos[3] = {-118.2437, 34.0522, 0.0};
        int solar_rc = 0;

        if ((jme_sol_eclipse_when_glob(solar_start, JME_CALC_TRUE_POSITION, 0, tret, 0, error) & JME_ECLIPSE_SOLAR_TOTAL) == 0
            || fabs(tret[0] - solar_total_max) > (20.0 / 1440.0)) {
            fprintf(stderr, "solar total eclipse global search mismatch: %s %.17g\n", error, tret[0]);
            return 1;
        }

        if (!(tret[2] < tret[0] && tret[0] < tret[3])
            || tret[4] != 0.0
            || tret[5] != 0.0) {
            fprintf(stderr, "solar total eclipse global contacts are not ordered\n");
            return 1;
        }

        if ((jme_sol_eclipse_when_glob(solar_start + 5.0, JME_CALC_TRUE_POSITION, 0, tret, 1, error) & JME_ECLIPSE_SOLAR_TOTAL) == 0
            || fabs(tret[0] - solar_total_max) > (20.0 / 1440.0)
            || !(tret[2] < tret[0] && tret[0] < tret[3])
            || tret[4] != 0.0
            || tret[5] != 0.0) {
            fprintf(stderr, "solar total eclipse backward search mismatch: %s %.17g\n", error, tret[0]);
            return 1;
        }

        if ((jme_sol_eclipse_where(tret[0], JME_CALC_TRUE_POSITION, geopos, attr, error) & JME_ECLIPSE_SOLAR_TOTAL) == 0
            || !finite_values(attr, 5)) {
            fprintf(stderr, "solar total eclipse where mismatch: %s\n", error);
            return 1;
        }

        solar_rc = jme_sol_eclipse_when_loc(solar_start, JME_CALC_TRUE_POSITION, geopos, tret, attr, 0, error);
        if (solar_rc == JME_ERR
            || (attr[0] <= 0.0)
            || (attr[3] <= 0.0)
            || (attr[4] <= 0.0)
            || (attr[5] <= 0.0)
            || ((solar_rc & JME_ECLIPSE_SOLAR_PARTIAL) == 0)
            || tret[0] < solar_start
            || tret[0] > solar_start + 2.0) {
            fprintf(stderr, "solar Dallas eclipse local mismatch: %s %.17g\n", error, tret[0]);
            return 1;
        }

        if (!(tret[2] < tret[0] && tret[0] < tret[3])) {
            fprintf(stderr, "solar Dallas local contacts are not ordered\n");
            return 1;
        }

        solar_rc = jme_sol_eclipse_how(tret[0], JME_CALC_TRUE_POSITION, geopos, attr, error);
        if (solar_rc == JME_ERR
            || (solar_rc & JME_ECLIPSE_SOLAR_PARTIAL) == 0
            || attr[0] <= 0.0
            || attr[3] <= 0.0
            || attr[4] <= 0.0) {
            fprintf(stderr, "solar Dallas eclipse circumstance mismatch: %s\n", error);
            return 1;
        }

        if (jme_sol_eclipse_where(2451545.0, JME_CALC_TRUE_POSITION, geopos, attr, error) != JME_ERR
            || jme_sol_eclipse_how(2451545.0, JME_CALC_TRUE_POSITION, geopos, attr, error) != JME_ERR) {
            fprintf(stderr, "solar eclipse circumstance accepted non-eclipse instant\n");
            return 1;
        }

        if ((jme_sol_eclipse_when_glob(annular_start, JME_CALC_TRUE_POSITION, 0, tret, 0, error) & JME_ECLIPSE_SOLAR_ANNULAR) == 0
            || fabs(tret[0] - annular_global_max) > (25.0 / 1440.0)
            || !(tret[2] < tret[0] && tret[0] < tret[3])
            || tret[4] != 0.0
            || tret[5] != 0.0) {
            fprintf(stderr, "solar annular eclipse global mismatch: %s %.17g\n", error, tret[0]);
            return 1;
        }

        solar_rc = jme_sol_eclipse_when_glob(partial_solar_start, JME_CALC_TRUE_POSITION, 0, tret, 0, error);
        if (solar_rc != JME_ECLIPSE_SOLAR_PARTIAL
            || fabs(tret[0] - partial_solar_max) > (25.0 / 1440.0)) {
            fprintf(stderr, "solar partial eclipse global mismatch: %s %.17g %d\n", error, tret[0], solar_rc);
            return 1;
        }

        if (jme_sol_eclipse_where(tret[0], JME_CALC_TRUE_POSITION, geopos, attr, error) != JME_ECLIPSE_SOLAR_PARTIAL
            || attr[0] <= 0.0
            || attr[2] <= 0.0
            || attr[3] <= 0.0
            || attr[4] <= 0.0) {
            fprintf(stderr, "solar partial eclipse where mismatch: %s %.17g %.17g\n", error, attr[0], attr[2]);
            return 1;
        }

        if (tret[2] != 0.0 || tret[3] != 0.0 || tret[4] != 0.0 || tret[5] != 0.0) {
            fprintf(stderr, "solar partial global search unexpectedly returned contact times\n");
            return 1;
        }

        solar_rc = jme_sol_eclipse_when_glob(hybrid_start, JME_CALC_TRUE_POSITION, 0, tret, 0, error);
        if (solar_rc != JME_ECLIPSE_SOLAR_HYBRID
            || fabs(tret[0] - hybrid_global_max) > (30.0 / 1440.0)
            || !(tret[2] < tret[0] && tret[0] < tret[3])
            || tret[4] != 0.0
            || tret[5] != 0.0) {
            fprintf(stderr, "solar hybrid eclipse global mismatch: %s %.17g %d\n", error, tret[0], solar_rc);
            return 1;
        }

        if (jme_sol_eclipse_where(tret[0], JME_CALC_TRUE_POSITION, geopos, attr, error) != JME_ECLIPSE_SOLAR_HYBRID
            || attr[8] != JME_ECLIPSE_SOLAR_CENTRAL) {
            fprintf(stderr, "solar hybrid eclipse where mismatch: %s %.17g %.17g\n", error, attr[7], attr[8]);
            return 1;
        }

        solar_rc = jme_sol_eclipse_when_loc(annular_start, JME_CALC_TRUE_POSITION, albuquerque_geopos, tret, attr, 0, error);
        if (solar_rc == JME_ERR
            || (attr[0] <= 0.0)
            || (attr[3] <= 0.0)
            || (attr[4] <= 0.0)
            || ((solar_rc & JME_ECLIPSE_SOLAR_PARTIAL) == 0)
            || attr[5] <= 0.0
            || tret[0] < annular_start
            || tret[0] > annular_start + 2.0) {
            fprintf(stderr, "solar Albuquerque eclipse local mismatch: %s %.17g\n", error, tret[0]);
            return 1;
        }

        if (!(tret[2] < tret[0] && tret[0] < tret[3])
            || tret[4] != 0.0
            || tret[5] != 0.0) {
            fprintf(stderr, "solar Albuquerque local contacts are not ordered\n");
            return 1;
        }

        if ((jme_lun_eclipse_when(lunar_total_start, JME_CALC_TRUE_POSITION, 0, tret, 0, error) & JME_ECLIPSE_LUNAR_TOTAL) == 0
            || fabs(tret[0] - lunar_total_max) > (20.0 / 1440.0)) {
            fprintf(stderr, "lunar total eclipse search mismatch: %s %.17g\n", error, tret[0]);
            return 1;
        }

        if ((jme_lun_eclipse_when(lunar_total_start + 5.0, JME_CALC_TRUE_POSITION, 0, tret, 1, error) & JME_ECLIPSE_LUNAR_TOTAL) == 0
            || fabs(tret[0] - lunar_total_max) > (20.0 / 1440.0)) {
            fprintf(stderr, "lunar total eclipse backward search mismatch: %s %.17g\n", error, tret[0]);
            return 1;
        }

        if ((jme_lun_eclipse_how(tret[0], JME_CALC_TRUE_POSITION, geopos, attr, error) & JME_ECLIPSE_LUNAR_TOTAL) == 0
            || attr[0] < 1.0
            || attr[1] <= 0.0) {
            fprintf(stderr, "lunar total eclipse circumstance mismatch: %s\n", error);
            return 1;
        }

        if (jme_lun_eclipse_how(2451545.0, JME_CALC_TRUE_POSITION, geopos, attr, error) != JME_ERR) {
            fprintf(stderr, "lunar eclipse how accepted non-eclipse instant\n");
            return 1;
        }

        if (jme_lun_eclipse_when(lunar_partial_start, JME_CALC_TRUE_POSITION, 0, tret, 0, error) != JME_ECLIPSE_LUNAR_PARTIAL
            || fabs(tret[0] - lunar_partial_max) > (20.0 / 1440.0)) {
            fprintf(stderr, "lunar partial eclipse search mismatch: %s %.17g\n", error, tret[0]);
            return 1;
        }

        if (jme_lun_eclipse_how(tret[0], JME_CALC_TRUE_POSITION, geopos, attr, error) != JME_ECLIPSE_LUNAR_PARTIAL
            || fabs(attr[0] - 0.1220) > 0.04
            || fabs(attr[1] - 1.1181) > 0.04) {
            fprintf(stderr, "lunar partial eclipse circumstance mismatch: %s %.17g %.17g\n", error, attr[0], attr[1]);
            return 1;
        }

        if ((jme_lun_eclipse_when(lunar_penumbral_start, JME_CALC_TRUE_POSITION, 0, tret, 0, error) & JME_ECLIPSE_LUNAR_PENUMBRAL) == 0
            || fabs(tret[0] - lunar_penumbral_max) > (30.0 / 1440.0)) {
            fprintf(stderr, "lunar penumbral eclipse search mismatch: %s %.17g\n", error, tret[0]);
            return 1;
        }

        if ((jme_lun_eclipse_how(tret[0], JME_CALC_TRUE_POSITION, geopos, attr, error) & JME_ECLIPSE_LUNAR_PENUMBRAL) == 0
            || attr[1] <= 0.0
            || attr[0] >= 0.0) {
            fprintf(stderr, "lunar penumbral eclipse circumstance mismatch: %s\n", error);
            return 1;
        }

        if ((jme_lun_eclipse_when_loc(lunar_total_start, JME_CALC_TRUE_POSITION, lunar_visible_geopos, tret, attr, 0, error) & JME_ECLIPSE_LUNAR_TOTAL) == 0
            || tret[8] <= 0.0
            || tret[9] <= tret[8]
            || tret[0] < tret[8]
            || tret[0] > tret[9]
            || !(tret[2] < tret[4] && tret[4] < tret[6] && tret[6] <= tret[0] && tret[0] <= tret[7] && tret[7] < tret[5] && tret[5] < tret[3])
            || attr[8] != JME_ECLIPSE_VISIBLE
            || attr[9] <= 0.0) {
            fprintf(stderr, "lunar local eclipse search mismatch: %s\n", error);
            return 1;
        }

        if ((jme_lun_eclipse_when_loc(lunar_total_start + 5.0, JME_CALC_TRUE_POSITION, lunar_visible_geopos, tret, attr, 1, error) & JME_ECLIPSE_LUNAR_TOTAL) == 0
            || fabs(tret[1] - lunar_total_max) > (20.0 / 1440.0)
            || tret[8] <= 0.0
            || tret[9] <= tret[8]
            || tret[0] < tret[8]
            || tret[0] > tret[9]
            || !(tret[2] < tret[4] && tret[4] < tret[6] && tret[6] <= tret[0] && tret[0] <= tret[7] && tret[7] < tret[5] && tret[5] < tret[3])
            || attr[8] != JME_ECLIPSE_VISIBLE
            || attr[9] <= 0.0) {
            fprintf(stderr, "lunar local eclipse backward search mismatch: %s\n", error);
            return 1;
        }

        if ((tret[2] != 0.0 && (tret[2] < tret[8] || tret[2] > tret[9]))
            || (tret[3] != 0.0 && (tret[3] < tret[8] || tret[3] > tret[9]))
            || (tret[4] != 0.0 && (tret[4] < tret[8] || tret[4] > tret[9]))
            || (tret[5] != 0.0 && (tret[5] < tret[8] || tret[5] > tret[9]))) {
            fprintf(stderr, "lunar local contacts were not clipped to visible interval\n");
            return 1;
        }

        {
            double hidden_geopos[3] = {100.0, 40.0, 0.0};
            if (jme_lun_eclipse_when_loc(lunar_total_start, JME_CALC_TRUE_POSITION, hidden_geopos, tret, attr, 0, error) != JME_ERR) {
                fprintf(stderr, "lunar local eclipse unexpectedly visible below horizon\n");
                return 1;
            }
        }

        if (jme_lun_occult_when_glob(mars_occult_start, JME_BODY_MARS, 0, JME_CALC_TRUE_POSITION, 0, tret, 0, error) != JME_OK
            || tret[0] < mars_occult_start
            || tret[0] > mars_occult_start + 2.0
            || !(tret[2] < tret[0] && tret[0] < tret[3])) {
            fprintf(stderr, "lunar Mars occultation global mismatch: %s %.17g\n", error, tret[0]);
            return 1;
        }

        if (jme_lun_occult_where(tret[0], JME_BODY_MARS, 0, JME_CALC_TRUE_POSITION, geopos, attr, error) != JME_OK
            || !finite_values(geopos, 2)
            || !finite_values(attr, 5)
            || attr[0] <= 0.0
            || attr[3] <= 0.0
            || attr[9] <= 0.0) {
            fprintf(stderr, "lunar Mars occultation where mismatch: %s\n", error);
            return 1;
        }

        if (jme_lun_occult_when_loc(mars_occult_start, JME_BODY_MARS, 0, JME_CALC_TRUE_POSITION, los_angeles_geopos, tret, attr, 0, error) != JME_OK
            || tret[0] < mars_occult_start
            || tret[0] > mars_occult_start + 2.0
            || !(tret[2] < tret[0] && tret[0] < tret[3])
            || attr[0] <= 0.0
            || attr[3] <= 0.0
            || attr[5] <= 0.0
            || attr[8] != JME_ECLIPSE_VISIBLE) {
            fprintf(stderr, "lunar Mars local occultation mismatch: %s %.17g\n", error, tret[0]);
            return 1;
        }

        if (jme_sol_eclipse_where(solar_total_max, JME_CALC_TRUE_POSITION, 0, attr, error) != JME_ERR
            || jme_sol_eclipse_where(solar_total_max, JME_CALC_TRUE_POSITION, geopos, 0, error) != JME_ERR
            || jme_sol_eclipse_how(dallas_total_max, JME_CALC_TRUE_POSITION, 0, attr, error) != JME_ERR
            || jme_sol_eclipse_how(dallas_total_max, JME_CALC_TRUE_POSITION, geopos, 0, error) != JME_ERR
            || jme_sol_eclipse_when_loc(solar_start, JME_CALC_TRUE_POSITION, 0, tret, attr, 0, error) != JME_ERR
            || jme_sol_eclipse_when_loc(solar_start, JME_CALC_TRUE_POSITION, geopos, 0, attr, 0, error) != JME_ERR
            || jme_sol_eclipse_when_loc(solar_start, JME_CALC_TRUE_POSITION, geopos, tret, 0, 0, error) != JME_ERR
            || jme_sol_eclipse_when_glob(solar_start, JME_CALC_TRUE_POSITION, 0, 0, 0, error) != JME_ERR
            || jme_lun_eclipse_how(lunar_total_max, JME_CALC_TRUE_POSITION, geopos, 0, error) != JME_ERR
            || jme_lun_eclipse_when_loc(lunar_total_start, JME_CALC_TRUE_POSITION, 0, tret, attr, 0, error) != JME_ERR
            || jme_lun_eclipse_when_loc(lunar_total_start, JME_CALC_TRUE_POSITION, lunar_visible_geopos, 0, attr, 0, error) != JME_ERR
            || jme_lun_eclipse_when_loc(lunar_total_start, JME_CALC_TRUE_POSITION, lunar_visible_geopos, tret, 0, 0, error) != JME_ERR
            || jme_lun_eclipse_when(lunar_total_start, JME_CALC_TRUE_POSITION, 0, 0, 0, error) != JME_ERR
            || jme_lun_occult_where(mars_occult_start, JME_BODY_MARS, 0, JME_CALC_TRUE_POSITION, 0, attr, error) != JME_ERR
            || jme_lun_occult_where(mars_occult_start, JME_BODY_MARS, 0, JME_CALC_TRUE_POSITION, geopos, 0, error) != JME_ERR
            || jme_lun_occult_when_loc(mars_occult_start, JME_BODY_MARS, 0, JME_CALC_TRUE_POSITION, 0, tret, attr, 0, error) != JME_ERR
            || jme_lun_occult_when_loc(mars_occult_start, JME_BODY_MARS, 0, JME_CALC_TRUE_POSITION, geopos, 0, attr, 0, error) != JME_ERR
            || jme_lun_occult_when_loc(mars_occult_start, JME_BODY_MARS, 0, JME_CALC_TRUE_POSITION, geopos, tret, 0, 0, error) != JME_ERR
            || jme_lun_occult_when_glob(mars_occult_start, JME_BODY_MARS, 0, JME_CALC_TRUE_POSITION, 0, 0, 0, error) != JME_ERR) {
            fprintf(stderr, "eclipse/occultation functions accepted null required outputs\n");
            return 1;
        }
    }

    {
        double elem[20] = {0.0};
        int rc = jme_get_orbital_elements(2451545.0, JME_BODY_MERCURY, JME_CALC_TRUE_POSITION, elem, error);

        if (rc == JME_OK) {
            if (!finite_values(elem, 9)
                || elem[0] <= 0.0
                || elem[1] < 0.0
                || elem[2] < 0.0
                || elem[2] > 180.0
                || elem[3] < 0.0
                || elem[3] >= 360.0
                || elem[4] < 0.0
                || elem[4] >= 360.0
                || elem[5] < 0.0
                || elem[5] >= 360.0
                || !isfinite(elem[9])
                || elem[9] < 0.0
                || elem[9] >= 360.0
                || !isfinite(elem[10])
                || elem[10] < 0.0
                || elem[10] >= 360.0
                || !isfinite(elem[11])
                || elem[11] <= 0.0
                || !isfinite(elem[12])
                || elem[12] <= 0.0
                || !isfinite(elem[13])
                || !isfinite(elem[14])
                || elem[13] <= 0.0
                || elem[14] <= elem[13]
                || fabs(elem[13] - elem[0] * (1.0 - elem[1])) > 1e-12
                || fabs(elem[14] - elem[0] * (1.0 + elem[1])) > 1e-12
                || fabs(elem[15] - jme_degree_normalize(elem[4] + elem[5])) > 1e-12
                || fabs(elem[16] - jme_degree_normalize(elem[3] + elem[4] + elem[5])) > 1e-12
                || fabs(elem[17] - jme_degree_normalize(elem[3] + elem[4])) > 1e-12) {
                fprintf(stderr, "orbital elements contract mismatch\n");
                return 1;
            }
        } else if (strstr(error, "velocity") == 0 && strstr(error, "Calculation") == 0) {
            fprintf(stderr, "orbital elements failed with unexpected error: %s\n", error);
            return 1;
        }
    }

    {
        double attr[20] = {0.0};
        double attr_ut[20] = {0.0};
        if (jme_pheno(2451545.0, JME_BODY_MERCURY, JME_CALC_TRUE_POSITION, attr, error) != JME_OK
            || !finite_values(attr, 20)
            || attr[0] < 0.0
            || attr[0] > 180.0
            || attr[1] < 0.0
            || attr[1] > 1.0
            || attr[2] < 0.0
            || attr[2] > 180.0
            || attr[3] <= 0.0
            || attr[4] < -30.0
            || attr[4] > 30.0
            || attr[5] <= 0.0
            || attr[6] <= 0.0
            || attr[7] <= 0.0
            || attr[8] <= 0.0
            || attr[9] < 0.0
            || attr[10] < 0.0
            || attr[10] >= 360.0) {
            fprintf(stderr, "physical phenomena contract mismatch: %s\n", error);
            return 1;
        }
        if (jme_pheno_ut(2451545.0, JME_BODY_MERCURY, JME_CALC_TRUE_POSITION, attr_ut, error) != JME_OK
            || !finite_values(attr_ut, 20)
            || attr_ut[0] < 0.0
            || attr_ut[0] > 180.0
            || attr_ut[1] < 0.0
            || attr_ut[1] > 1.0
            || attr_ut[3] <= 0.0
            || attr_ut[5] <= 0.0) {
            fprintf(stderr, "UT physical phenomena contract mismatch: %s\n", error);
            return 1;
        }
        if (jme_pheno(2451545.0, JME_BODY_MERCURY, JME_CALC_TRUE_POSITION, 0, error) != JME_ERR
            || jme_pheno_ut(2451545.0, JME_BODY_MERCURY, JME_CALC_TRUE_POSITION, 0, error) != JME_ERR) {
            fprintf(stderr, "physical phenomena accepted null output\n");
            return 1;
        }
    }

    {
        double node[4] = {0.0};
        double apogee[4] = {0.0};
        double mercury_node[4] = {0.0};
        double mercury_node_ut[4] = {0.0};
        double mercury_aphelion[4] = {0.0};
        double mercury_perihelion[4] = {0.0};

        if (jme_nod_aps(2451545.0, JME_BODY_MOON, JME_CALC_TRUE_POSITION, 1, node, error) != JME_OK
            || !finite_values(node, 4)
            || node[0] < 0.0
            || node[0] >= 360.0
            || jme_nod_aps(2451545.0, JME_BODY_MOON, JME_CALC_TRUE_POSITION, 4, apogee, error) != JME_OK
            || !finite_values(apogee, 4)
            || apogee[0] < 0.0
            || apogee[0] >= 360.0) {
            fprintf(stderr, "lunar node/apside contract mismatch: %s\n", error);
            return 1;
        }

        if (jme_nod_aps(2451545.0, JME_BODY_MERCURY, JME_CALC_TRUE_POSITION, 1, mercury_node, error) != JME_OK
            || jme_nod_aps(2451545.0, JME_BODY_MERCURY, JME_CALC_TRUE_POSITION, 4, mercury_aphelion, error) != JME_OK
            || jme_nod_aps(2451545.0, JME_BODY_MERCURY, JME_CALC_TRUE_POSITION, 256, mercury_perihelion, error) != JME_OK
            || !finite_values(mercury_node, 4)
            || !finite_values(mercury_aphelion, 4)
            || !finite_values(mercury_perihelion, 4)
            || mercury_node[0] < 0.0
            || mercury_node[0] >= 360.0
            || mercury_node[1] <= 0.0
            || mercury_aphelion[1] <= mercury_perihelion[1]
            || fabs(jme_degrees_difference(mercury_aphelion[0], mercury_perihelion[0] + 180.0)) > 1e-9) {
            fprintf(stderr, "non-lunar node/apside contract mismatch: %s\n", error);
            return 1;
        }

        if (jme_nod_aps_ut(2451545.0, JME_BODY_MERCURY, JME_CALC_TRUE_POSITION, 1, mercury_node_ut, error) != JME_OK
            || !finite_values(mercury_node_ut, 4)
            || mercury_node_ut[0] < 0.0
            || mercury_node_ut[0] >= 360.0) {
            fprintf(stderr, "UT node/apside contract mismatch: %s\n", error);
            return 1;
        }

        if (jme_nod_aps(2451545.0, JME_BODY_SUN, JME_CALC_TRUE_POSITION, 1, node, error) != JME_ERR) {
            fprintf(stderr, "node/apside unexpectedly accepted unsupported body\n");
            return 1;
        }

        if (jme_nod_aps(2451545.0, JME_BODY_MERCURY, JME_CALC_TRUE_POSITION, 1, 0, error) != JME_ERR) {
            fprintf(stderr, "node/apside accepted null output\n");
            return 1;
        }
    }

    {
        double elem[20] = {0.0};
        double tmax = 0.0;
        double tmin = 0.0;
        double dmax = 0.0;
        double dmin = 0.0;

        if (jme_orbit_max_min_true_distance(2451545.0, JME_BODY_MERCURY, JME_CALC_TRUE_POSITION, &tmax, &tmin, &dmax, &dmin, error) != JME_OK
            || !isfinite(tmax)
            || !isfinite(tmin)
            || !isfinite(dmax)
            || !isfinite(dmin)
            || dmin <= 0.0
            || dmax <= dmin
            || fabs(dmin - 0.307499) > 0.02
            || fabs(dmax - 0.466699) > 0.02) {
            fprintf(stderr, "orbit max/min true distance contract mismatch: %s\n", error);
            return 1;
        }

        if (jme_get_orbital_elements(2451545.0, JME_BODY_MERCURY, JME_CALC_TRUE_POSITION, 0, error) != JME_ERR
            || jme_get_orbital_elements(2451545.0, JME_BODY_SUN, JME_CALC_TRUE_POSITION, elem, error) != JME_ERR
            || jme_orbit_max_min_true_distance(2451545.0, JME_BODY_MERCURY, JME_CALC_TRUE_POSITION, 0, &tmin, &dmax, &dmin, error) != JME_ERR
            || jme_orbit_max_min_true_distance(2451545.0, JME_BODY_SUN, JME_CALC_TRUE_POSITION, &tmax, &tmin, &dmax, &dmin, error) != JME_ERR) {
            fprintf(stderr, "orbital element/extrema invalid-input contract mismatch\n");
            return 1;
        }
    }

    {
        double geopos[3] = {0.0, 51.5, 0.0};
        double dat_hel[20] = {0.0};
        double invalid_dat_hel[20] = {0.0};
        int visible;
        int heliacal_body;
        double hangle = jme_heliacal_angle(2451545.0, geopos, 0, error);
        double arcus = jme_topo_arcus_visionis(2451545.0, geopos, 0, error);

        if (!isfinite(hangle) || !isfinite(arcus) || hangle < 0.0 || hangle > 180.0) {
            fprintf(stderr, "heliacal scalar helpers returned invalid geometry\n");
            return 1;
        }

        memset(dat_hel, 0, sizeof(dat_hel));
        dat_hel[0] = JME_BODY_VENUS;
        if (jme_heliacal_pheno_ut(2451545.0, geopos, dat_hel, error) != JME_OK
            || !isfinite(dat_hel[2])
            || !isfinite(dat_hel[3])
            || !isfinite(dat_hel[4])
            || !isfinite(dat_hel[5])
            || !isfinite(dat_hel[6])
            || !isfinite(dat_hel[7])
            || !isfinite(dat_hel[10])
            || dat_hel[7] < 0.0
            || dat_hel[7] > 180.0
            || fabs(dat_hel[10] - (10.50 + 1.40 * dat_hel[6])) > 1.0e-12
            || fabs(dat_hel[5] - ((dat_hel[4] - 10.50) / 1.40)) > 1.0e-12) {
            fprintf(stderr, "heliacal phenomenon contract mismatch: %s\n", error);
            return 1;
        }

        if (fabs(jme_heliacal_angle(2451545.0, geopos, dat_hel, error) - dat_hel[7]) > 1.0e-12
            || fabs(jme_topo_arcus_visionis(2451545.0, geopos, dat_hel, error) - dat_hel[4]) > 1.0e-12) {
            fprintf(stderr, "heliacal scalar helper mismatch against phenomenon output\n");
            return 1;
        }

        visible = dat_hel[9] != 0.0;
        if (visible && (dat_hel[3] < 0.0 || dat_hel[2] >= 0.0 || dat_hel[4] < dat_hel[10] || dat_hel[6] > dat_hel[5])) {
            fprintf(stderr, "heliacal visibility flag contract mismatch\n");
            return 1;
        }

        for (heliacal_body = JME_BODY_MOON; heliacal_body <= JME_BODY_PLUTO; heliacal_body++) {
            memset(dat_hel, 0, sizeof(dat_hel));
            dat_hel[0] = (double)heliacal_body;
            if (jme_heliacal_pheno_ut(2451545.0, geopos, dat_hel, error) != JME_OK
                || !finite_values(dat_hel, 11)
                || dat_hel[1] != (double)heliacal_body
                || dat_hel[7] < 0.0
                || dat_hel[7] > 180.0
                || fabs(dat_hel[10] - (10.50 + 1.40 * dat_hel[6])) > 1.0e-12
                || fabs(dat_hel[5] - ((dat_hel[4] - 10.50) / 1.40)) > 1.0e-12) {
                fprintf(stderr, "heliacal body sweep contract mismatch for body %d: %s\n", heliacal_body, error);
                return 1;
            }
        }

        memset(dat_hel, 0, sizeof(dat_hel));
        dat_hel[0] = JME_BODY_VENUS;
        if (jme_vis_limit_mag(2451545.0, geopos, dat_hel, error) != JME_OK
            || !isfinite(dat_hel[5])
            || fabs(dat_hel[5] - ((dat_hel[4] - 10.50) / 1.40)) > 1.0e-12) {
            fprintf(stderr, "visual limit contract mismatch: %s\n", error);
            return 1;
        }

        memset(dat_hel, 0, sizeof(dat_hel));
        dat_hel[0] = JME_BODY_VENUS;
        if (jme_heliacal_ut(2451545.0, geopos, dat_hel, error) != JME_OK
            || !isfinite(dat_hel[0])
            || dat_hel[0] < 2451545.0
            || dat_hel[0] > 2451545.0 + 370.0
            || dat_hel[1] != (double)JME_BODY_VENUS
            || dat_hel[9] == 0.0
            || dat_hel[3] < 0.0
            || dat_hel[2] >= 0.0
            || dat_hel[4] < dat_hel[10]
            || dat_hel[6] > dat_hel[5]) {
            fprintf(stderr, "heliacal event search contract mismatch: %s\n", error);
            return 1;
        }
        {
            double event_time = dat_hel[0];
            memset(dat_hel, 0, sizeof(dat_hel));
            dat_hel[0] = JME_BODY_VENUS;
            if (jme_heliacal_pheno_ut(event_time, geopos, dat_hel, error) != JME_OK
                || dat_hel[9] == 0.0
                || fabs(dat_hel[10] - (10.50 + 1.40 * dat_hel[6])) > 1.0e-12) {
                fprintf(stderr, "heliacal event revalidation mismatch: %s\n", error);
                return 1;
            }
        }

        invalid_dat_hel[0] = JME_BODY_SUN;
        if (jme_heliacal_pheno_ut(2451545.0, geopos, invalid_dat_hel, error) != JME_ERR
            || jme_heliacal_ut(2451545.0, geopos, invalid_dat_hel, error) != JME_ERR
            || jme_vis_limit_mag(2451545.0, geopos, invalid_dat_hel, error) != JME_ERR
            || isfinite(jme_heliacal_angle(2451545.0, geopos, invalid_dat_hel, error))
            || isfinite(jme_topo_arcus_visionis(2451545.0, geopos, invalid_dat_hel, error))) {
            fprintf(stderr, "heliacal functions accepted unsupported body id\n");
            return 1;
        }

        invalid_dat_hel[0] = JME_BODY_TRUE_NODE;
        if (jme_heliacal_pheno_ut(2451545.0, geopos, invalid_dat_hel, error) != JME_ERR) {
            fprintf(stderr, "heliacal phenomena accepted non-physical node body\n");
            return 1;
        }

        invalid_dat_hel[0] = JME_BODY_VENUS + 0.5;
        if (jme_heliacal_pheno_ut(2451545.0, geopos, invalid_dat_hel, error) != JME_ERR) {
            fprintf(stderr, "heliacal phenomena accepted fractional body id\n");
            return 1;
        }

        if (jme_heliacal_pheno_ut(2451545.0, 0, dat_hel, error) != JME_ERR
            || jme_heliacal_pheno_ut(2451545.0, geopos, 0, error) != JME_ERR
            || jme_heliacal_ut(2451545.0, 0, dat_hel, error) != JME_ERR
            || jme_heliacal_ut(2451545.0, geopos, 0, error) != JME_ERR
            || jme_vis_limit_mag(2451545.0, geopos, 0, error) != JME_ERR
            || isfinite(jme_heliacal_angle(2451545.0, 0, dat_hel, error))
            || isfinite(jme_topo_arcus_visionis(2451545.0, 0, dat_hel, error))) {
            fprintf(stderr, "heliacal functions accepted null required inputs\n");
            return 1;
        }
    }

    if (jme_jpl_is_available() && getenv("JME_TEST_JPL_KERNEL") != 0) {
        double results[6];
        if (jme_jpl_open(getenv("JME_TEST_JPL_KERNEL"), error) != JME_OK) {
            fprintf(stderr, "JPL kernel open for calc contract failed: %s\n", error);
            return 1;
        }

        if (jme_calc(2451545.0, JME_BODY_SUN, JME_CALC_XYZ | JME_CALC_J2000, results, error) == JME_OK) {
            if (fabs(jme_state_distance(results) - 0.98328) > 0.01) {
                fprintf(stderr, "Sun distance at J2000 mismatch: %.17g\n", jme_state_distance(results));
                return 1;
            }
        }

        jme_jpl_close();
    }

    if (!jme_jpl_is_available()) {
        if (jme_jpl_open("", error) != JME_ERR) {
            fprintf(stderr, "JPL open unexpectedly succeeded without CALCEPH\n");
            return 1;
        }

        {
            const char *paths[1] = {"missing.bsp"};

            if (jme_jpl_open_array(1, paths, error) != JME_ERR) {
                fprintf(stderr, "JPL array open unexpectedly succeeded without CALCEPH\n");
                return 1;
            }
        }

        if (jme_jpl_prefetch(error) != JME_ERR) {
            fprintf(stderr, "JPL prefetch unexpectedly succeeded without CALCEPH\n");
            return 1;
        }

        if (jme_jpl_is_thread_safe(error) != JME_ERR) {
            fprintf(stderr, "JPL thread query unexpectedly succeeded without CALCEPH\n");
            return 1;
        }

        if (jme_jpl_id_by_name("EARTH", &h, error) != JME_ERR || h != 0) {
            fprintf(stderr, "JPL id lookup error path mismatch\n");
            return 1;
        }

        if (jme_jpl_name_by_id(399, name, sizeof(name), error) != JME_ERR || name[0] != '\0') {
            fprintf(stderr, "JPL name lookup error path mismatch\n");
            return 1;
        }

        if (jme_jpl_max_supported_order(2) != JME_ERR) {
            fprintf(stderr, "JPL max order unexpectedly succeeded without CALCEPH\n");
            return 1;
        }

        if (jme_jpl_constant_count(error) != JME_ERR) {
            fprintf(stderr, "JPL constant count unexpectedly succeeded without CALCEPH\n");
            return 1;
        }

        if (jme_jpl_constant_index(1, name, sizeof(name), &value, error) != JME_ERR || name[0] != '\0' || value != 0.0) {
            fprintf(stderr, "JPL constant index error path mismatch\n");
            return 1;
        }

        vector_values[0] = 1.0;
        vector_values[1] = 1.0;
        if (jme_jpl_constant_vector("X", vector_values, 2, error) != JME_ERR || vector_values[0] != 0.0 || vector_values[1] != 0.0) {
            fprintf(stderr, "JPL constant vector error path mismatch\n");
            return 1;
        }

        if (jme_jpl_constant_string("X", name, sizeof(name), error) != JME_ERR || name[0] != '\0') {
            fprintf(stderr, "JPL constant string error path mismatch\n");
            return 1;
        }

        values[0][0] = 'x';
        values[1][0] = 'x';
        if (jme_jpl_constant_string_vector("X", values[0], sizeof(values[0]), 2, error) != JME_ERR || values[0][0] != '\0' || values[1][0] != '\0') {
            fprintf(stderr, "JPL constant string vector error path mismatch\n");
            return 1;
        }

        if (jme_jpl_file_version(name, sizeof(name), error) != JME_ERR || name[0] != '\0') {
            fprintf(stderr, "JPL file version error path mismatch\n");
            return 1;
        }

        path[0] = 'x';
        first_time = 1.0;
        last_time = 1.0;
        continuous = 1;
        if (
            jme_jpl_current_file_data(
                path,
                sizeof(path),
                &first_time,
                &last_time,
                &continuous,
                error
            ) != JME_ERR ||
            path[0] != '\0' ||
            first_time != 0.0 ||
            last_time != 0.0 ||
            continuous != 0
        ) {
            fprintf(stderr, "JPL current file data error path mismatch\n");
            return 1;
        }

        if (jme_jpl_body_state(
                2451545.0,
                JME_BODY_MOON,
                JME_BODY_EARTH,
                JME_VECTOR_AU_PER_DAY,
                state,
                error
            ) != JME_ERR) {
            fprintf(stderr, "JPL state unexpectedly succeeded without CALCEPH\n");
            return 1;
        }

        if (state[0] != 0.0 || state[1] != 0.0 || state[2] != 0.0) {
            fprintf(stderr, "JPL error path did not clear state\n");
            return 1;
        }

        state[0] = 1.0;
        state[1] = 1.0;
        state[2] = 1.0;
        if (jme_jpl_body_state_native_naif(
                2451545.0,
                301,
                399,
                state,
                error
            ) != JME_ERR) {
            fprintf(stderr, "JPL native state unexpectedly succeeded without CALCEPH\n");
            return 1;
        }

        if (state[0] != 0.0 || state[1] != 0.0 || state[2] != 0.0) {
            fprintf(stderr, "JPL native error path did not clear state\n");
            return 1;
        }

        state[0] = 1.0;
        state[1] = 1.0;
        state[2] = 1.0;
        if (jme_jpl_body_state_native(
                2451545.0,
                JME_BODY_MOON,
                JME_BODY_EARTH,
                state,
                error
            ) != JME_ERR) {
            fprintf(stderr, "JPL native enum state unexpectedly succeeded without CALCEPH\n");
            return 1;
        }

        if (state[0] != 0.0 || state[1] != 0.0 || state[2] != 0.0) {
            fprintf(stderr, "JPL native enum error path did not clear state\n");
            return 1;
        }

        state[0] = 1.0;
        state[1] = 1.0;
        state[2] = 1.0;
        if (jme_jpl_body_state_utc_naif(
                2000,
                1,
                1,
                12,
                0,
                0.0,
                JME_CALENDAR_GREGORIAN,
                301,
                399,
                JME_VECTOR_AU_PER_DAY,
                state,
                error
            ) != JME_ERR) {
            fprintf(stderr, "JPL UTC state unexpectedly succeeded without CALCEPH\n");
            return 1;
        }

        if (state[0] != 0.0 || state[1] != 0.0 || state[2] != 0.0) {
            fprintf(stderr, "JPL UTC error path did not clear state\n");
            return 1;
        }

        state[0] = 1.0;
        state[1] = 1.0;
        state[2] = 1.0;
        if (jme_jpl_body_state_utc(
                2000,
                1,
                1,
                12,
                0,
                0.0,
                JME_CALENDAR_GREGORIAN,
                JME_BODY_MOON,
                JME_BODY_EARTH,
                JME_VECTOR_AU_PER_DAY,
                state,
                error
            ) != JME_ERR) {
            fprintf(stderr, "JPL UTC enum state unexpectedly succeeded without CALCEPH\n");
            return 1;
        }

        if (state[0] != 0.0 || state[1] != 0.0 || state[2] != 0.0) {
            fprintf(stderr, "JPL UTC enum error path did not clear state\n");
            return 1;
        }

        state[0] = 1.0;
        state[1] = 1.0;
        state[2] = 1.0;
        if (jme_jpl_body_state_split(
                2451545.0,
                0.0,
                JME_BODY_MOON,
                JME_BODY_EARTH,
                JME_VECTOR_AU_PER_DAY,
                state,
                error
            ) != JME_ERR) {
            fprintf(stderr, "JPL split state unexpectedly succeeded without CALCEPH\n");
            return 1;
        }

        if (state[0] != 0.0 || state[1] != 0.0 || state[2] != 0.0) {
            fprintf(stderr, "JPL split error path did not clear state\n");
            return 1;
        }

        state[0] = 1.0;
        state[1] = 1.0;
        state[2] = 1.0;
        if (jme_jpl_ecliptic_state(
                2451545.0,
                JME_BODY_MOON,
                JME_BODY_EARTH,
                JME_VECTOR_AU_PER_DAY,
                state,
                error
            ) != JME_ERR) {
            fprintf(stderr, "JPL ecliptic state unexpectedly succeeded without CALCEPH\n");
            return 1;
        }

        if (state[0] != 0.0 || state[1] != 0.0 || state[2] != 0.0) {
            fprintf(stderr, "JPL ecliptic error path did not clear state\n");
            return 1;
        }

        state[0] = 1.0;
        state[1] = 1.0;
        state[2] = 1.0;
        if (jme_jpl_ecliptic_state_utc_naif(
                2000,
                1,
                1,
                12,
                0,
                0.0,
                JME_CALENDAR_GREGORIAN,
                301,
                399,
                JME_VECTOR_AU_PER_DAY,
                state,
                error
            ) != JME_ERR) {
            fprintf(stderr, "JPL ecliptic UTC state unexpectedly succeeded without CALCEPH\n");
            return 1;
        }

        if (state[0] != 0.0 || state[1] != 0.0 || state[2] != 0.0) {
            fprintf(stderr, "JPL ecliptic UTC error path did not clear state\n");
            return 1;
        }

        state[0] = 1.0;
        state[1] = 1.0;
        state[2] = 1.0;
        if (jme_jpl_ecliptic_state_split(
                2451545.0,
                0.0,
                JME_BODY_MOON,
                JME_BODY_EARTH,
                JME_VECTOR_AU_PER_DAY,
                state,
                error
            ) != JME_ERR) {
            fprintf(stderr, "JPL ecliptic split state unexpectedly succeeded without CALCEPH\n");
            return 1;
        }

        if (state[0] != 0.0 || state[1] != 0.0 || state[2] != 0.0) {
            fprintf(stderr, "JPL ecliptic split error path did not clear state\n");
            return 1;
        }

        state[0] = 1.0;
        state[1] = 1.0;
        state[2] = 1.0;
        if (jme_jpl_orientation_state_naif(
                2451545.0,
                399,
                JME_ORIENTATION_RAD_PER_DAY,
                state,
                error
            ) != JME_ERR) {
            fprintf(stderr, "JPL orientation state unexpectedly succeeded without CALCEPH\n");
            return 1;
        }

        if (state[0] != 0.0 || state[1] != 0.0 || state[2] != 0.0) {
            fprintf(stderr, "JPL orientation error path did not clear state\n");
            return 1;
        }

        state[0] = 1.0;
        state[1] = 1.0;
        state[2] = 1.0;
        if (jme_jpl_orientation_state_utc_naif(
                2000,
                1,
                1,
                12,
                0,
                0.0,
                JME_CALENDAR_GREGORIAN,
                399,
                JME_ORIENTATION_RAD_PER_DAY,
                state,
                error
            ) != JME_ERR) {
            fprintf(stderr, "JPL orientation UTC state unexpectedly succeeded without CALCEPH\n");
            return 1;
        }

        if (state[0] != 0.0 || state[1] != 0.0 || state[2] != 0.0) {
            fprintf(stderr, "JPL orientation UTC error path did not clear state\n");
            return 1;
        }

        if (jme_jpl_orientation_state_order_naif(
                2451545.0,
                0.0,
                399,
                JME_ORIENTATION_RAD_PER_DAY,
                3,
                order_state,
                error
            ) != JME_ERR) {
            fprintf(stderr, "JPL orientation order state unexpectedly succeeded without CALCEPH\n");
            return 1;
        }

        if (order_state[0] != 0.0 || order_state[11] != 0.0) {
            fprintf(stderr, "JPL orientation order error path did not clear state\n");
            return 1;
        }

        order_state[0] = 1.0;
        order_state[11] = 1.0;
        if (jme_jpl_body_state_order_naif(
                2451545.0,
                0.0,
                301,
                399,
                JME_VECTOR_AU_PER_DAY,
                3,
                order_state,
                error
            ) != JME_ERR) {
            fprintf(stderr, "JPL body order state unexpectedly succeeded without CALCEPH\n");
            return 1;
        }

        if (order_state[0] != 0.0 || order_state[11] != 0.0) {
            fprintf(stderr, "JPL body order error path did not clear state\n");
            return 1;
        }

        order_state[0] = 1.0;
        order_state[11] = 1.0;
        if (jme_jpl_body_state_order(
                2451545.0,
                0.0,
                JME_BODY_MOON,
                JME_BODY_EARTH,
                JME_VECTOR_AU_PER_DAY,
                3,
                order_state,
                error
            ) != JME_ERR) {
            fprintf(stderr, "JPL body order enum state unexpectedly succeeded without CALCEPH\n");
            return 1;
        }

        if (order_state[0] != 0.0 || order_state[11] != 0.0) {
            fprintf(stderr, "JPL body order enum error path did not clear state\n");
            return 1;
        }

        state[0] = 1.0;
        state[1] = 1.0;
        state[2] = 1.0;
        if (jme_jpl_rotational_angular_momentum_state_naif(
                2451545.0,
                399,
                JME_ORIENTATION_RAD_PER_DAY,
                state,
                error
            ) != JME_ERR) {
            fprintf(stderr, "JPL rotational angular momentum unexpectedly succeeded without CALCEPH\n");
            return 1;
        }

        if (state[0] != 0.0 || state[1] != 0.0 || state[2] != 0.0) {
            fprintf(stderr, "JPL rotational angular momentum error path did not clear state\n");
            return 1;
        }

        state[0] = 1.0;
        state[1] = 1.0;
        state[2] = 1.0;
        if (jme_jpl_rotational_angular_momentum_state_utc_naif(
                2000,
                1,
                1,
                12,
                0,
                0.0,
                JME_CALENDAR_GREGORIAN,
                399,
                JME_ORIENTATION_RAD_PER_DAY,
                state,
                error
            ) != JME_ERR) {
            fprintf(stderr, "JPL rotational angular momentum UTC unexpectedly succeeded without CALCEPH\n");
            return 1;
        }

        if (state[0] != 0.0 || state[1] != 0.0 || state[2] != 0.0) {
            fprintf(stderr, "JPL rotational angular momentum UTC error path did not clear state\n");
            return 1;
        }

        if (jme_jpl_orientation_record_count(error) != JME_ERR) {
            fprintf(stderr, "JPL orientation count unexpectedly succeeded without CALCEPH\n");
            return 1;
        }
    }

    return 0;
}
