#include "jme/jme.h"
#include "context.h"
#include "jme_erfa.h"

#include <math.h>
#include <stdio.h>
#include <string.h>

static int fail(const char *message, const char *error)
{
    fprintf(stderr, "time_scale_contract: %s", message);
    if (error != 0 && error[0] != '\0') {
        fprintf(stderr, " (error: %s)", error);
    }
    fputc('\n', stderr);
    return 1;
}

static double pair_difference_seconds(
    double day_a, double fraction_a,
    double day_b, double fraction_b
)
{
    return ((day_b - day_a) + (fraction_b - fraction_a)) * JME_SECONDS_PER_DAY;
}

static int pair_is_normalized(double day, double fraction)
{
    return isfinite(day) && isfinite(fraction)
        && floor(day) == day && fraction >= 0.0 && fraction < 1.0;
}

int main(void)
{
    char error[512] = "";
    double tt_day, tt_fraction;
    double next_day, next_fraction;
    double tdb_day, tdb_fraction;
    double tcb_day, tcb_fraction;
    double dtr;
    double elapsed;
    int rc;

    /* UTC is converted using TAI-UTC + the exact 32.184-second TAI-to-TT step. */
    rc = internal_utc_to_tt_split(
        2000, 1, 1, 12, 0, 0.0, JME_CALENDAR_GREGORIAN,
        &tt_day, &tt_fraction, error
    );
    if (rc != JME_OK || !pair_is_normalized(tt_day, tt_fraction)) {
        return fail("J2000 UTC-to-TT conversion failed", error);
    }
    if (tt_day != 2451545.0
        || fabs((tt_fraction - (64.184 / JME_SECONDS_PER_DAY)) * JME_SECONDS_PER_DAY) > 1.0e-8) {
        return fail("J2000 UTC-to-TT offset does not equal 64.184 seconds", error);
    }

    /* A valid inserted second and the following UTC midnight are one SI second apart. */
    error[0] = '\0';
    rc = internal_utc_to_tt_split(
        2016, 12, 31, 23, 59, 60.0, JME_CALENDAR_GREGORIAN,
        &tt_day, &tt_fraction, error
    );
    if (rc != JME_OK || !pair_is_normalized(tt_day, tt_fraction)) {
        return fail("2016 leap second was not accepted", error);
    }
    error[0] = '\0';
    rc = internal_utc_to_tt_split(
        2017, 1, 1, 0, 0, 0.0, JME_CALENDAR_GREGORIAN,
        &next_day, &next_fraction, error
    );
    if (rc != JME_OK || !pair_is_normalized(next_day, next_fraction)) {
        return fail("UTC immediately after leap second was not accepted", error);
    }
    elapsed = pair_difference_seconds(tt_day, tt_fraction, next_day, next_fraction);
    if (fabs(elapsed - 1.0) > 1.0e-8) {
        return fail("UTC leap-second boundary is not continuous in TT", error);
    }

    error[0] = '\0';
    rc = internal_utc_to_tt_split(
        2016, 12, 30, 23, 59, 60.0, JME_CALENDAR_GREGORIAN,
        &tt_day, &tt_fraction, error
    );
    if (rc != JME_ERR || strstr(error, "announced positive UTC leap second") == 0) {
        return fail("an unannounced second 60 was accepted", error);
    }

    error[0] = '\0';
    rc = internal_utc_to_tt_split(
        2027, 1, 1, 0, 0, 0.0, JME_CALENDAR_GREGORIAN,
        &tt_day, &tt_fraction, error
    );
    if (rc != JME_ERR || strstr(error, "future leap seconds") == 0) {
        return fail("future UTC offset uncertainty was not reported", error);
    }

    error[0] = '\0';
    rc = internal_utc_to_tt_split(
        1959, 12, 31, 23, 59, 59.0, JME_CALENDAR_GREGORIAN,
        &tt_day, &tt_fraction, error
    );
    if (rc != JME_ERR || strstr(error, "before 1960") == 0) {
        return fail("pre-1960 UTC conversion was not rejected", error);
    }

    /* Pre-1972 rate-adjusted UTC remains supported, rather than using Delta-T as TAI-UTC. */
    error[0] = '\0';
    rc = internal_utc_to_tt_split(
        1965, 6, 1, 12, 0, 0.0, JME_CALENDAR_GREGORIAN,
        &tt_day, &tt_fraction, error
    );
    if (rc != JME_OK || !pair_is_normalized(tt_day, tt_fraction)) {
        return fail("historical rate-adjusted UTC conversion failed", error);
    }

    /* TT -> TDB uses ERFA's complete series and retains the two-part Julian date. */
    error[0] = '\0';
    rc = internal_tt_to_tdb_split(
        2451545.0, 0.1234567890123, &tdb_day, &tdb_fraction, error
    );
    if (rc != JME_OK || !pair_is_normalized(tdb_day, tdb_fraction)) {
        return fail("TT-to-TDB split conversion failed", error);
    }
    dtr = jme_erfa_dtdb(tdb_day, tdb_fraction, 0.0, 0.0, 0.0, 0.0);
    elapsed = pair_difference_seconds(
        2451545.0, 0.1234567890123, tdb_day, tdb_fraction
    );
    if (!isfinite(dtr) || fabs(elapsed - dtr) > 1.0e-6) {
        return fail("TT-to-TDB result is inconsistent with the ERFA series", error);
    }

    /* TDB -> TCB at J2000 is about eleven seconds; the full affine relation is retained. */
    error[0] = '\0';
    rc = internal_tdb_to_tcb_split(
        tdb_day, tdb_fraction, &tcb_day, &tcb_fraction, error
    );
    if (rc != JME_OK || !pair_is_normalized(tcb_day, tcb_fraction)) {
        return fail("TDB-to-TCB split conversion failed", error);
    }
    elapsed = pair_difference_seconds(tdb_day, tdb_fraction, tcb_day, tcb_fraction);
    if (!(elapsed > 10.0 && elapsed < 12.0)) {
        return fail("TDB-to-TCB conversion is outside its J2000 expected range", error);
    }

    /* All conversion entry points reject invalid/non-normalized data, not silent substitutions. */
    error[0] = '\0';
    if (internal_tt_to_tdb_split(NAN, 0.0, &tdb_day, &tdb_fraction, error) != JME_ERR
        || strstr(error, "normalized two-part") == 0) {
        return fail("non-finite TT time was accepted by TT-to-TDB conversion", error);
    }
    error[0] = '\0';
    if (internal_tdb_to_tcb_split(2451545.0, 1.0, &tcb_day, &tcb_fraction, error) != JME_ERR
        || strstr(error, "normalized two-part") == 0) {
        return fail("non-normalized TDB date was accepted by TDB-to-TCB conversion", error);
    }

    return 0;
}
