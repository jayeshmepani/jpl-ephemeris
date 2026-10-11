#include "jme/jme.h"
#include "context.h"
#include "jme_erfa_vendor.h"
#include <ctype.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>

#define JME_FIXSTAR_DEG_TO_RAD 0.017453292519943295769236907684886127134428718885417

typedef struct jme_star_entry {
    int hr;
    int hd;
    int sao;
    const char *name;
    double ra; /* J2000 degrees */
    double dec; /* J2000 degrees */
    double pm_ra; /* arcsec/year */
    double pm_dec; /* arcsec/year */
    double mag;
} jme_star_entry;

typedef struct jme_star_alias {
    const char *name;
    int hr;
} jme_star_alias;

/* Common-name aliases point into the generated Bright Star catalog. */
static const jme_star_alias star_aliases[] = {
    {"Achernar", 472},
    {"Aldebaran", 1457},
    {"Altair", 7557},
    {"Antares", 6134},
    {"Arcturus", 5340},
    {"Betelgeuse", 2061},
    {"Canopus", 2326},
    {"Capella", 1708},
    {"Deneb", 7924},
    {"Delta Cancri", 3461},
    {"Fomalhaut", 8728},
    {"Pollux", 2990},
    {"Procyon", 2943},
    {"Regulus", 3982},
    {"Revati", 361},
    {"Rigel", 1713},
    {"Shaula", 6527},
    {"Sirius", 2491},
    {"Spica", 5056},
    {"Vega", 7001},
    {"Asellus Australis", 3461},
    {"Lambda Scorpii", 6527},
    {"Eta Psc", 437},
    {"Zeta Psc", 361}
};

#include "fixstar_catalog.inc"

static int ascii_case_equal(const char *a, const char *b)
{
    while (*a != '\0' && *b != '\0') {
        if (tolower((unsigned char)*a) != tolower((unsigned char)*b)) {
            return 0;
        }
        a++;
        b++;
    }

    return *a == '\0' && *b == '\0';
}

static int parse_prefixed_number(const char *text, const char *prefix, int *value)
{
    const char *p = text;
    char *end = 0;
    long parsed;

    while (isspace((unsigned char)*p)) {
        p++;
    }

    while (*prefix != '\0') {
        if (tolower((unsigned char)*p) != tolower((unsigned char)*prefix)) {
            return 0;
        }
        p++;
        prefix++;
    }

    while (isspace((unsigned char)*p) || *p == '-') {
        p++;
    }

    if (!isdigit((unsigned char)*p)) {
        return 0;
    }

    parsed = strtol(p, &end, 10);
    while (end != 0 && isspace((unsigned char)*end)) {
        end++;
    }

    if (end == 0 || *end != '\0' || parsed <= 0 || parsed > 999999999L) {
        return 0;
    }

    *value = (int)parsed;
    return 1;
}

static const jme_star_entry *find_star_by_hr(int hr)
{
    size_t i;

    for (i = 0; i < sizeof(star_catalog) / sizeof(star_catalog[0]); i++) {
        if (star_catalog[i].hr == hr) {
            return &star_catalog[i];
        }
    }

    return 0;
}

static const jme_star_entry *find_star(const char *name)
{
    size_t i;
    int value = 0;

    if (name == 0 || name[0] == '\0') {
        return 0;
    }

    if (parse_prefixed_number(name, "HR", &value)) {
        return find_star_by_hr(value);
    }

    if (parse_prefixed_number(name, "HD", &value)) {
        for (i = 0; i < sizeof(star_catalog) / sizeof(star_catalog[0]); i++) {
            if (star_catalog[i].hd == value) {
                return &star_catalog[i];
            }
        }
        return 0;
    }

    if (parse_prefixed_number(name, "SAO", &value)) {
        for (i = 0; i < sizeof(star_catalog) / sizeof(star_catalog[0]); i++) {
            if (star_catalog[i].sao == value) {
                return &star_catalog[i];
            }
        }
        return 0;
    }

    for (i = 0; i < sizeof(star_aliases) / sizeof(star_aliases[0]); i++) {
        if (ascii_case_equal(star_aliases[i].name, name)) {
            return find_star_by_hr(star_aliases[i].hr);
        }
    }

    for (i = 0; i < sizeof(star_catalog) / sizeof(star_catalog[0]); i++) {
        if (star_catalog[i].name[0] != '\0' && ascii_case_equal(star_catalog[i].name, name)) {
            return &star_catalog[i];
        }
    }
    return 0;
}

typedef struct jme_star_hip_entry {
    int hr;
    int hip;
    double ra;      /* ICRS degrees, epoch J1991.25 */
    double dec;     /* ICRS degrees, epoch J1991.25 */
    double pm_ra;   /* mu_alpha * cos(delta), mas/yr */
    double pm_dec;  /* mas/yr */
    double plx;     /* mas; <= 0 means unknown/not significant */
    double rv;      /* km/s */
    int has_rv;
} jme_star_hip_entry;

#include "fixstar_hipparcos.inc"

static const jme_star_hip_entry *find_hipparcos(int hr)
{
    size_t lo = 0, hi = sizeof(star_hipparcos) / sizeof(star_hipparcos[0]);
    while (lo < hi) {
        size_t mid = (lo + hi) / 2;
        if (star_hipparcos[mid].hr == hr) return &star_hipparcos[mid];
        if (star_hipparcos[mid].hr < hr) lo = mid + 1; else hi = mid;
    }
    return 0;
}

#define JME_MAS_TO_RAD (JME_FIXSTAR_DEG_TO_RAD / 3600000.0)
#define JME_AU_PER_PARSEC 206264.80624709635515647

/*
 * Barycentric ICRS direction (unit vector) of a catalogue star at jd_et (TT,
 * used as TDB: the difference is < 2 ms) and its distance in AU (0 when the
 * parallax is unknown). Space motion is propagated rigorously with ERFA
 * eraStarpm (proper motion, parallax, radial velocity, light time).
 *   - Hipparcos new reduction (van Leeuwen 2007) astrometry at J1991.25 when
 *     the star is cross-identified (8995 of 9096 BSC entries);
 *   - otherwise the BSC5 J2000 (FK5) position and proper motion, with no
 *     parallax; distance is reported as unknown (0).
 */
int internal_fixstar_barycentric(const char *star, double jd_et, double *u, double *dist_au, char *error)
{
    const jme_star_entry *entry = find_star(star);
    const jme_star_hip_entry *hip;
    double ra1, dec1, pmr, pmd, px, rv, ep1;
    double ra2, dec2, pmr2, pmd2, px2, rv2;
    int status;

    if (entry == 0) {
        jme_set_error(error, "Star not found in catalog");
        return JME_ERR;
    }
    hip = find_hipparcos(entry->hr);
    if (hip != 0) {
        ra1 = hip->ra * JME_FIXSTAR_DEG_TO_RAD;
        dec1 = hip->dec * JME_FIXSTAR_DEG_TO_RAD;
        pmr = hip->pm_ra * JME_MAS_TO_RAD / cos(dec1);
        pmd = hip->pm_dec * JME_MAS_TO_RAD;
        px = hip->plx > 0.0 ? hip->plx / 1000.0 : 0.0;
        rv = hip->rv;
        ep1 = 2448349.0625; /* J1991.25 */
    } else {
        ra1 = entry->ra * JME_FIXSTAR_DEG_TO_RAD;
        dec1 = entry->dec * JME_FIXSTAR_DEG_TO_RAD;
        pmr = entry->pm_ra / 3600.0 * JME_FIXSTAR_DEG_TO_RAD / cos(dec1);
        pmd = entry->pm_dec / 3600.0 * JME_FIXSTAR_DEG_TO_RAD;
        px = 0.0;
        rv = 0.0;
        ep1 = 2451545.0;
    }
    if (!isfinite(pmr)) pmr = 0.0; /* exactly at a pole */
    {
        double jd_tdb;
        if (internal_tt_to_tdb_jd(jd_et, &jd_tdb, error) != JME_OK) return JME_ERR;
        status = jme_eraStarpm(ra1, dec1, pmr, pmd, px, rv, ep1, 0.0, jd_tdb, 0.0,
            &ra2, &dec2, &pmr2, &pmd2, &px2, &rv2);
    }
    /*
     * eraStarpm status: 1 = distance overridden (expected when the parallax is
     * unknown); 2 = excessive speed, 4 = no convergence, <0 = system error.
     */
    if (status < 0 || (status & 6) != 0) {
        jme_set_error(error, "Star space-motion propagation failed");
        return JME_ERR;
    }
    jme_eraS2c(ra2, dec2, u);
    *dist_au = (px > 0.0 && px2 > 0.0) ? JME_AU_PER_PARSEC / px2 : 0.0;
    if (!isfinite(u[0]) || !isfinite(u[1]) || !isfinite(u[2]) || !isfinite(*dist_au)) {
        jme_set_error(error, "Star propagation produced non-finite values");
        return JME_ERR;
    }
    return JME_OK;
}

int internal_fixstar_mean_ecliptic_of_date(const char *star, double jd_et, double *ecl, char *error)
{
    double u[3], dist, pos[3], sph[6], st[6];
    int i;
    if (internal_fixstar_barycentric(star, jd_et, u, &dist, error) != JME_OK) return JME_ERR;
    if (internal_rotate_to_output(jd_et, JME_CALC_NO_NUTATION, 1, u, pos, error) != JME_OK) return JME_ERR;
    for (i = 0; i < 3; i++) { st[i] = pos[i]; st[i + 3] = 0.0; }
    if (jme_rectangular_to_spherical_state(st, sph) != JME_OK) {
        jme_set_error(error, "Invalid star vector");
        return JME_ERR;
    }
    for (i = 0; i < 6; i++) ecl[i] = sph[i];
    return JME_OK;
}

static int fixstar_output_position(const char *star, double jd_et, int flags, double *pos, double *dist_out, char *error)
{
    double u[3], dist, v[3];
    if (internal_fixstar_barycentric(star, jd_et, u, &dist, error) != JME_OK) return JME_ERR;
    if (internal_star_observer_vector(jd_et, flags, u, dist, v, error) != JME_OK) return JME_ERR;
    if (dist_out != 0) *dist_out = dist;
    return internal_rotate_to_output(jd_et, flags, 1, v, pos, error);
}

int jme_fixstar_ut(const char *star, double jd_ut, int flags, double *results, char *error)
{
    double jd_et;
    int i;
    if (results == 0) {
        jme_set_error(error, "Fixed-star output is required");
        return JME_ERR;
    }
    for (i=0;i<6;i++) results[i]=0.0;
    if (internal_ut_to_et(jd_ut, &jd_et, error) != JME_OK) return JME_ERR;
    return jme_fixstar(star, jd_et, flags, results, error);
}

/*
 * Fixed-star position. Distance (results[2]) is in AU (km with
 * JME_CALC_DISTANCE_KM) from the Hipparcos parallax; it is 0 when the
 * parallax is unknown, in which case JME_CALC_XYZ returns a unit vector.
 * Speeds: five-point central differences (step 1 day) of the full reduction.
 */
int jme_fixstar(const char *star, double jd_et, int flags, double *results, char *error)
{
    double p0[3], q[4][3], dist = 0.0;
    const double *stencil[4];
    const double h = 1.0;
    static const double offs[4] = {-2.0, -1.0, 1.0, 2.0};
    int i;

    if (results == 0) {
        jme_set_error(error, "Fixed-star output is required");
        return JME_ERR;
    }
    for (i = 0; i < 6; i++) { results[i] = 0.0; }
    if (error != 0) error[0] = '\0';
    if (!isfinite(jd_et)) {
        jme_set_error(error, "Terrestrial-time Julian date must be finite");
        return JME_ERR;
    }
    if (find_star(star) == 0) {
        jme_set_error(error, "Star not found in catalog");
        return JME_ERR;
    }
    if (fixstar_output_position(star, jd_et, flags, p0, &dist, error) != JME_OK) {
        return JME_ERR;
    }
    for (i = 0; i < 4; i++) {
        if (fixstar_output_position(star, jd_et + offs[i] * h, flags, q[i], 0, error) != JME_OK) {
            for (i = 0; i < 6; i++) results[i] = 0.0;
            return JME_ERR;
        }
        stencil[i] = q[i];
    }
    if (internal_finish_output(p0, stencil, h, flags, results, error) != JME_OK) {
        return JME_ERR;
    }
    if (!(dist > 0.0) && !(flags & JME_CALC_XYZ)) {
        results[2] = 0.0;
        results[5] = 0.0;
    }
    if (error != 0) error[0] = '\0';
    return JME_OK;
}

int jme_fixstar_mag(const char *star, double *mag, char *error)
{
    const jme_star_entry *entry = find_star(star);
    if (mag == 0) {
        jme_set_error(error, "Fixed-star magnitude output is required");
        return JME_ERR;
    }
    if (entry == 0) {
        jme_set_error(error, "Star not found");
        return JME_ERR;
    }
    *mag = entry->mag;
    return JME_OK;
}

int jme_fixstar2(const char *star, double jd_et, int flags, double *results, char *error)
{
    return jme_fixstar(star, jd_et, flags, results, error);
}

int jme_fixstar2_ut(const char *star, double jd_ut, int flags, double *results, char *error)
{
    return jme_fixstar_ut(star, jd_ut, flags, results, error);
}

int jme_fixstar2_mag(const char *star, double *mag, char *error)
{
    return jme_fixstar_mag(star, mag, error);
}
