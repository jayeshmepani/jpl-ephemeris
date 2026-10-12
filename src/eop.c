/*
 * IERS Earth orientation parameters: polar motion (xp, yp).
 *
 * Accepted files (format detected per line):
 *   - IERS Rapid Service finals2000A (finals2000A.all / .data / .daily):
 *     fixed columns per readme.finals2000A: MJD cols 8-15, PM flag col 17
 *     (I = IERS, P = prediction), PM-x cols 19-27 and PM-y cols 38-46
 *     (Bulletin A, arcsec). Rows whose PM fields are blank end the series.
 *   - IERS EOP C04 series (EOP 20 C04 "eopc04.1962-now" and the older
 *     14 C04 layout): whitespace-separated, '#' or text header lines
 *     skipped; MJD is the first field with a decimal point, followed by
 *     x and y in arcsec.
 *
 * Interpolation: 4-point (cubic) Lagrange interpolation in MJD on the
 * daily series, the scheme of the IERS interp.f routine (LAGINT). Two
 * tabulated days are required on each side of the requested instant, the
 * four days must be consecutive (spacing 1 day); otherwise an error is
 * returned. There is no extrapolation. The sub-daily ocean-tide and
 * libration terms of IERS Conventions (2010) sect. 5.5.1 are not added
 * (amplitudes below 0.5 mas; documented in docs/TIME_SCALES.md).
 */
#include <ctype.h>
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "context.h"
#include "jme/jme.h"
#include "jme/jme_extended.h"

typedef struct jme_eop_row {
    double mjd;
    double xp;
    double yp;
    int predicted;
} jme_eop_row;

static jme_eop_row *g_eop_rows;
static size_t g_eop_count;

void jme_eop_clear(void)
{
    free(g_eop_rows);
    g_eop_rows = 0;
    g_eop_count = 0;
}

static int field_to_double(const char *line, size_t len, size_t col_from, size_t col_to, double *out)
{
    char buf[32];
    size_t n, i, k = 0;
    char *end;
    if (len < col_to) return 0;
    n = col_to - col_from + 1;
    if (n >= sizeof(buf)) return 0;
    for (i = 0; i < n; i++) {
        char c = line[col_from - 1 + i];
        if (c != ' ' && c != '\t') buf[k++] = c;
    }
    buf[k] = '\0';
    if (k == 0) return 0;
    *out = strtod(buf, &end);
    return *end == '\0' && isfinite(*out);
}

/* finals2000A row: returns 1 parsed, 0 not this format, -1 end of PM data. */
static int parse_finals(const char *line, size_t len, jme_eop_row *row)
{
    double mjd, x, y;
    if (len < 46 || line[6] != ' ' || line[15] != ' ') return 0;
    if (!field_to_double(line, len, 8, 15, &mjd)) return 0;
    if (line[16] != 'I' && line[16] != 'P') return -1;
    if (!field_to_double(line, len, 19, 27, &x) || !field_to_double(line, len, 38, 46, &y)) return -1;
    row->mjd = mjd;
    row->xp = x;
    row->yp = y;
    row->predicted = line[16] == 'P';
    return 1;
}

static int parse_c04(const char *line, jme_eop_row *row)
{
    double v[8];
    int n = 0, mjd_index = -1;
    const char *p = line;
    char *end;
    while (*p != '\0' && n < 8) {
        while (*p == ' ' || *p == '\t') p++;
        if (*p == '\0' || *p == '\n' || *p == '\r') break;
        v[n] = strtod(p, &end);
        if (end == p) return 0;
        if (mjd_index < 0 && memchr(p, '.', (size_t)(end - p)) != 0) mjd_index = n;
        n++;
        p = end;
        if (*p != ' ' && *p != '\t' && *p != '\0' && *p != '\n' && *p != '\r') return 0;
    }
    if (mjd_index < 3 || mjd_index + 2 >= n) return 0;
    if (!(v[mjd_index] > 15000.0 && v[mjd_index] < 200000.0)) return 0;
    row->mjd = v[mjd_index];
    row->xp = v[mjd_index + 1];
    row->yp = v[mjd_index + 2];
    row->predicted = 0;
    return isfinite(row->xp) && isfinite(row->yp);
}

int jme_eop_load_file(const char *path, char *error)
{
    FILE *f;
    char line[1024];
    jme_eop_row *rows = 0, row;
    size_t count = 0, cap = 0;
    int finals_ended = 0;

    if (path == 0 || path[0] == '\0') {
        jme_set_error(error, "EOP file path is required");
        return JME_ERR;
    }
    f = fopen(path, "r");
    if (f == 0) {
        jme_set_error(error, "EOP file could not be opened");
        return JME_ERR;
    }
    while (fgets(line, sizeof(line), f) != 0) {
        size_t len = strcspn(line, "\r\n");
        int r;
        line[len] = '\0';
        if (len == 0 || line[0] == '#') continue;
        r = parse_finals(line, len, &row);
        if (r < 0) { finals_ended = 1; continue; }
        if (r == 0 && !parse_c04(line, &row)) continue;
        if (finals_ended) continue;
        if (count > 0 && !(row.mjd > rows[count - 1].mjd)) {
            fclose(f);
            free(rows);
            jme_set_error(error, "EOP file dates are not strictly increasing");
            return JME_ERR;
        }
        if (count == cap) {
            size_t ncap = cap == 0 ? 4096 : cap * 2;
            jme_eop_row *nr = (jme_eop_row *)realloc(rows, ncap * sizeof(*rows));
            if (nr == 0) {
                fclose(f);
                free(rows);
                jme_set_error(error, "Out of memory reading EOP file");
                return JME_ERR;
            }
            rows = nr;
            cap = ncap;
        }
        rows[count++] = row;
    }
    fclose(f);
    if (count < 4) {
        free(rows);
        jme_set_error(error, "EOP file contains fewer than four polar-motion rows");
        return JME_ERR;
    }
    jme_eop_clear();
    g_eop_rows = rows;
    g_eop_count = count;
    jme_set_error(error, "");
    return JME_OK;
}

int jme_eop_coverage(double *mjd_first, double *mjd_last, char *error)
{
    if (g_eop_count == 0) {
        jme_set_error(error, "No EOP data loaded (jme_eop_load_file)");
        return JME_ERR;
    }
    if (mjd_first != 0) *mjd_first = g_eop_rows[0].mjd;
    if (mjd_last != 0) *mjd_last = g_eop_rows[g_eop_count - 1].mjd;
    jme_set_error(error, "");
    return JME_OK;
}

int jme_eop_polar_motion(double mjd, double *xp_arcsec, double *yp_arcsec, int *predicted, char *error)
{
    size_t lo, hi, i0;
    int k, j;
    double x = 0.0, y = 0.0;

    if (xp_arcsec != 0) *xp_arcsec = NAN;
    if (yp_arcsec != 0) *yp_arcsec = NAN;
    if (g_eop_count == 0) {
        jme_set_error(error, "Polar motion requested but no EOP data loaded (jme_eop_load_file)");
        return JME_ERR;
    }
    if (!isfinite(mjd) || mjd < g_eop_rows[1].mjd || mjd > g_eop_rows[g_eop_count - 2].mjd) {
        jme_set_error(error, "Date outside the loaded EOP coverage (two tabulated days needed on each side)");
        return JME_ERR;
    }
    lo = 0;
    hi = g_eop_count - 1;
    while (hi - lo > 1) {
        size_t mid = (lo + hi) / 2;
        if (g_eop_rows[mid].mjd <= mjd) lo = mid; else hi = mid;
    }
    /* lo: last row with mjd_row <= mjd; use rows lo-1 .. lo+2. */
    if (lo + 2 >= g_eop_count) lo = g_eop_count - 3;
    i0 = lo - 1;
    for (k = 1; k < 4; k++) {
        if (g_eop_rows[i0 + (size_t)k].mjd - g_eop_rows[i0 + (size_t)k - 1].mjd != 1.0) {
            jme_set_error(error, "EOP series is not daily around the requested date");
            return JME_ERR;
        }
    }
    for (k = 0; k < 4; k++) {
        double w = 1.0;
        for (j = 0; j < 4; j++) {
            if (j != k) {
                w *= (mjd - g_eop_rows[i0 + (size_t)j].mjd)
                    / (g_eop_rows[i0 + (size_t)k].mjd - g_eop_rows[i0 + (size_t)j].mjd);
            }
        }
        x += w * g_eop_rows[i0 + (size_t)k].xp;
        y += w * g_eop_rows[i0 + (size_t)k].yp;
    }
    if (xp_arcsec != 0) *xp_arcsec = x;
    if (yp_arcsec != 0) *yp_arcsec = y;
    if (predicted != 0) {
        *predicted = 0;
        for (k = 0; k < 4; k++) *predicted |= g_eop_rows[i0 + (size_t)k].predicted;
    }
    jme_set_error(error, "");
    return JME_OK;
}

void jme_set_polar_motion(int enabled)
{
    jme_get_context()->polar_motion_enabled = enabled != 0;
}

int jme_get_polar_motion(void)
{
    return jme_get_context()->polar_motion_enabled;
}
