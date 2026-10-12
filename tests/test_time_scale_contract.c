#include "jme/jme.h"
#include "context.h"
#include "jme_erfa.h"

#include <math.h>
#include <stdio.h>
#include <string.h>
#include "jme/jme_extended.h"
#include "fixtures/time_scales_erfa.h"

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

    /* IERS Bulletin C 72: no leap second at end of 2026; table valid through 2027-06-30. */
    error[0] = '\0';
    rc = internal_utc_to_tt_split(
        2027, 1, 1, 0, 0, 0.0, JME_CALENDAR_GREGORIAN,
        &tt_day, &tt_fraction, error
    );
    if (rc != JME_OK) {
        return fail("UTC 2027-01-01 (covered by Bulletin C 72) was rejected", error);
    }
    error[0] = '\0';
    rc = internal_utc_to_tt_split(
        2026, 12, 31, 23, 59, 60.0, JME_CALENDAR_GREGORIAN,
        &tt_day, &tt_fraction, error
    );
    if (rc != JME_ERR) {
        return fail("non-existent leap second 2026-12-31T23:59:60 was accepted", error);
    }
    error[0] = '\0';
    rc = internal_utc_to_tt_split(
        2027, 7, 1, 0, 0, 0.0, JME_CALENDAR_GREGORIAN,
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

    /* jme_time_scale_convert against independent pyerfa values (tests/fixtures/time_scales_erfa.h). */
    {
        size_t i;
        const int scales[6] = {JME_TIME_SCALE_UTC, JME_TIME_SCALE_TAI, JME_TIME_SCALE_TT,
            JME_TIME_SCALE_TDB, JME_TIME_SCALE_TCB, JME_TIME_SCALE_TCG};
        for (i = 0; i < sizeof(time_fixtures) / sizeof(time_fixtures[0]); i++) {
            const jme_time_fixture *f = &time_fixtures[i];
            const double *ref[6] = {f->utc, f->tai, f->tt, f->tdb, f->tcb, f->tcg};
            int a, b;
            for (a = 0; a < 6; a++) {
                for (b = 0; b < 6; b++) {
                    double o1, o2, diff;
                    char msg[256];
                    error[0] = '\0';
                    if (jme_time_scale_convert(ref[a][0], ref[a][1], scales[a], scales[b], &o1, &o2, error) != JME_OK) {
                        snprintf(msg, sizeof(msg), "time_scale_convert %s->%s failed for fixture %d",
                            jme_time_scale_name(scales[a]), jme_time_scale_name(scales[b]), (int)i);
                        return fail(msg, error);
                    }
                    diff = ((o1 - ref[b][0]) + (o2 - ref[b][1])) * 86400.0;
                    if (!(fabs(diff) < 5.0e-11)) {
                        snprintf(msg, sizeof(msg), "time_scale_convert %s->%s fixture %d differs by %.3e s",
                            jme_time_scale_name(scales[a]), jme_time_scale_name(scales[b]), (int)i, diff);
                        return fail(msg, error);
                    }
                }
            }
        }
    }

    /*
     * Split-date Delta-T must preserve a sub-ULP offset when it crosses the
     * end of the embedded IERS table. The ordinary double JD cannot represent
     * this offset, so compare with the next representable JD on the EM-2006
     * branch. The offset is large enough to survive subtraction of the MJD
     * epoch, but far smaller than one ULP of the full Julian date.
     */
    {
        double first, last_observed, last;
        double split_dt, reference_dt;
        error[0] = '\\0';
        if (jme_delta_t_iers_table_info(&first, &last_observed, &last, error) != JME_OK) {
            return fail("IERS table metadata unavailable for split-date test", error);
        }
        split_dt = internal_delta_t_ex_split(last, 1.0e-11,
            JME_TIME_DELTAT_AUTOMATIC, error);
        if (!isfinite(split_dt) || error[0] != '\\0') {
            return fail("split-date Delta-T failed immediately beyond the IERS table", error);
        }
        reference_dt = jme_delta_t_ex(nextafter(last, INFINITY),
            JME_MODEL_DELTAT_ESPENAK_MEEUS_2006, error);
        if (!isfinite(reference_dt) || error[0] != '\\0'
            || fabs(split_dt - reference_dt) > 1.0e-6) {
            return fail("split-date Delta-T lost a sub-ULP offset at the IERS boundary", error);
        }
    }

    /* Leap-second table expiry is reported and enforced. */
    {
        double until = 0.0, last = 0.0, o1, o2;
        if (jme_leap_second_table_info(&until, &last, error) != JME_OK
            || until != 2461587.5 || last != 37.0) {
            return fail("leap-second table info mismatch", error);
        }
        if (jme_time_scale_convert(until, 0.25, JME_TIME_SCALE_UTC, JME_TIME_SCALE_TT, &o1, &o2, error) != JME_ERR
            || !isnan(o1)) {
            return fail("UTC beyond the leap-second table was converted", error);
        }
        if (jme_time_scale_convert(1959.0 * 365.25, 0.0, JME_TIME_SCALE_UTC, JME_TIME_SCALE_TT, &o1, &o2, error) != JME_ERR) {
            return fail("UTC before 1960 was converted", error);
        }
        if (jme_time_scale_convert(NAN, 0.0, JME_TIME_SCALE_TT, JME_TIME_SCALE_TDB, &o1, &o2, error) != JME_ERR
            || jme_time_scale_convert(2451545.0, 0.0, 99, JME_TIME_SCALE_TDB, &o1, &o2, error) != JME_ERR
            || jme_time_scale_convert(2451545.0, 0.0, JME_TIME_SCALE_TT, -1, &o1, &o2, error) != JME_ERR) {
            return fail("invalid time-scale inputs were accepted", error);
        }
        /* TT <-> UT1 round trip through the context Delta-T model. */
        if (jme_time_scale_convert(2451545.0, 0.0, JME_TIME_SCALE_TT, JME_TIME_SCALE_UT1, &o1, &o2, error) != JME_OK) {
            return fail("TT->UT1 failed", error);
        }
        {
            double b1, b2;
            if (jme_time_scale_convert(o1, o2, JME_TIME_SCALE_UT1, JME_TIME_SCALE_TT, &b1, &b2, error) != JME_OK
                || fabs(((b1 - 2451545.0) + b2) * 86400.0) > 1.0e-8) {
                return fail("UT1<->TT round trip mismatch", error);
            }
        }
    }

    return 0;
}
