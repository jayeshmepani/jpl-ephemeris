#include "jme/jme.h"
#include "jme/jme_extended.h"

#include <math.h>
#include <stdio.h>
#include <string.h>

static int all_finite(const double *values, int count)
{
    int i;
    for (i = 0; i < count; i++) {
        if (!isfinite(values[i])) {
            return 0;
        }
    }
    return 1;
}

static int fail(const char *message, const char *error)
{
    fprintf(stderr, "precision_contract: %s", message);
    if (error != 0 && error[0] != '\0') {
        fprintf(stderr, " (error: %s)", error);
    }
    fputc('\n', stderr);
    return 1;
}

int main(void)
{
    char error[256] = "";
    double dt;
    const int unsupported_models[] = {
        JME_MODEL_DELTAT_STEPHENSON_MORRISON_1984,
        JME_MODEL_DELTAT_STEPHENSON_1997,
        JME_MODEL_DELTAT_STEPHENSON_MORRISON_2004,
        JME_MODEL_DELTAT_STEPHENSON_ETC_2016
    };
    size_t i;
    double apparent[6];
    double without_aberration[6];
    double ra_difference;
    int rc;

    /* Direct TT APIs must reject NaN/infinite times instead of reporting success. */
    {
        double values[6];
        double attrs[20];
        double tret[4];
        error[0] = '\0';
        if (jme_calc(NAN, JME_BODY_SUN, JME_CALC_EQUATORIAL, values, error) != JME_ERR
            || strstr(error, "finite") == 0) {
            return fail("calc accepted a non-finite TT date", error);
        }
        error[0] = '\0';
        if (jme_calc(INFINITY, JME_BODY_SUN, JME_CALC_EQUATORIAL, values, error) != JME_ERR
            || strstr(error, "finite") == 0) {
            return fail("calc accepted an infinite TT date", error);
        }
        error[0] = '\0';
        if (jme_pheno(NAN, JME_BODY_MARS, 0, attrs, error) != JME_ERR
            || strstr(error, "finite") == 0) {
            return fail("pheno accepted a non-finite TT date", error);
        }
        error[0] = '\0';
        if (jme_fixstar("Sirius", NAN, 0, values, error) != JME_ERR
            || strstr(error, "finite") == 0) {
            return fail("fixed-star API accepted a non-finite TT date", error);
        }
        error[0] = '\0';
        if (jme_nod_aps(NAN, JME_BODY_MOON, JME_CALC_TRUE_POSITION, 1, tret, error) != JME_ERR
            || strstr(error, "finite") == 0) {
            return fail("nodal/apsidal API accepted a non-finite TT date", error);
        }
    }

    /* The implemented EM-2006 path is finite and uses the published 2000-era fit. */
    dt = jme_delta_t_ex(2451544.5, JME_MODEL_DELTAT_ESPENAK_MEEUS_2006, error);
    if (!isfinite(dt) || error[0] != '\0' || dt < 63.8 || dt > 64.0) {
        return fail("Espenak-Meeus 2006 result is invalid", error);
    }

    /* An unsupported model must never be silently evaluated as EM-2006. */
    for (i = 0; i < sizeof(unsupported_models) / sizeof(unsupported_models[0]); i++) {
        error[0] = '\0';
        dt = jme_delta_t_ex(2451544.5, unsupported_models[i], error);
        if (isfinite(dt) || strstr(error, "no validated implementation") == 0) {
            return fail("unsupported Delta-T model did not fail closed", error);
        }
    }

    error[0] = '\0';
    dt = jme_delta_t_ex(NAN, JME_MODEL_DELTAT_ESPENAK_MEEUS_2006, error);
    if (isfinite(dt) || strstr(error, "finite") == 0) {
        return fail("non-finite Julian date was not rejected", error);
    }

    /*
     * Status-returning UT entry points must reject failed Delta-T conversion
     * rather than forward a non-finite TT Julian date to calculation routines.
     */
    jme_set_astro_models("DELTAT2016 ENGINE=MOSHIER", 0);
    error[0]='\0';
    if (jme_calc_ut(2451545.0,JME_BODY_SUN,JME_CALC_EQUATORIAL,apparent,error)!=JME_ERR
        || strstr(error,"no validated implementation")==0) return fail("calc_ut did not propagate Delta-T failure",error);
    error[0]='\0';
    {
        double attributes[20];
        if (jme_pheno_ut(2451545.0,JME_BODY_MARS,0,attributes,error)!=JME_ERR
            || strstr(error,"no validated implementation")==0) return fail("pheno_ut did not propagate Delta-T failure",error);
    }
    error[0]='\0';
    if (jme_fixstar_ut("Sirius",2451545.0,JME_CALC_EQUATORIAL,apparent,error)!=JME_ERR
        || strstr(error,"no validated implementation")==0) return fail("fixstar_ut did not propagate Delta-T failure",error);
    error[0]='\0';
    if (jme_get_ayanamsa_ex_ut(2451545.0,JME_SIDEREAL_LAHIRI,&dt,error)!=JME_ERR
        || strstr(error,"no validated implementation")==0) return fail("ayanamsa_ex_ut did not propagate Delta-T failure",error);
    error[0]='\0';
    if (jme_time_equ(2451545.0,&dt,error)!=JME_ERR
        || strstr(error,"no validated implementation")==0) return fail("time_equ did not propagate Delta-T failure",error);
    error[0]='\0';
    if (jme_nod_aps_ut(2451545.0,JME_BODY_MERCURY,JME_CALC_TRUE_POSITION,1,&dt,error)!=JME_ERR
        || strstr(error,"no validated implementation")==0) return fail("nod_aps_ut did not propagate Delta-T failure",error);

    jme_set_astro_models("DELTAT2006 ENGINE=MOSHIER", 0);

    /*
     * The Moshier lunar path uses ecliptic-of-date coordinates. Ensure that
     * apparent corrections with observer/Sun vectors transformed into that
     * frame remain numerically valid.
     */
    jme_set_astro_models("DELTAT2006 ENGINE=MOSHIER", 0);
    error[0] = '\0';
    rc = jme_calc(2451545.0, JME_BODY_MOON, JME_CALC_TRUE_POSITION, apparent, error);
    if (rc != JME_OK || !all_finite(apparent, 6)) {
        return fail("Moshier Moon true-position calculation failed", error);
    }
    error[0] = '\0';
    rc = jme_calc(2451545.0, JME_BODY_MOON, 0, apparent, error);
    if (rc != JME_OK || !all_finite(apparent, 6)) {
        return fail("Moshier Moon apparent-position calculation failed", error);
    }

    /*
     * Without a JPL kernel, analytical fixed-star calculations must still
     * apply aberration.
     */
    jme_jpl_close();
    jme_set_astro_models("ENGINE=MOSHIER", 0);

    error[0] = '\0';
    rc = jme_fixstar("Sirius", 2451545.0, JME_CALC_EQUATORIAL, apparent, error);
    if (rc != JME_OK || !all_finite(apparent, 6)) {
        return fail("apparent fixed-star calculation failed in Moshier mode", error);
    }

    error[0] = '\0';
    rc = jme_fixstar("Sirius", 2451545.0,
        JME_CALC_EQUATORIAL | JME_CALC_NO_ABERRATION,
        without_aberration, error);
    if (rc != JME_OK || !all_finite(without_aberration, 6)) {
        return fail("fixed-star no-aberration calculation failed", error);
    }

    ra_difference = jme_degrees_difference_signed(apparent[0], without_aberration[0]);
    if (fabs(ra_difference) + fabs(apparent[1] - without_aberration[1]) < 1.0e-6) {
        return fail("analytical fixed-star calculation silently omitted aberration", error);
    }

    /* Explicit JPL mode must fail if its required kernel state is unavailable. */
    jme_jpl_close();
    jme_set_astro_models("ENGINE=JPL", 0);
    error[0] = '\0';
    rc = jme_fixstar("Sirius", 2451545.0, JME_CALC_EQUATORIAL, apparent, error);
    if (rc != JME_ERR || error[0] == '\0') {
        return fail("explicit JPL mode silently omitted unavailable aberration state", error);
    }

    /* Keep subsequent tests independent of this test's engine selection. */
    (void)jme_set_astro_models("ENGINE=AUTO", 0);
    return 0;
}
