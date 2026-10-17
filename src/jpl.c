#include "jme/jme.h"
#include "jme/jme_extended.h"

#include "context.h"
#include "erfa/jme_erfa.h"

#include <ctype.h>
#include <stddef.h>
#include <string.h>
#include <math.h>

#define JME_J2000_MEAN_OBLIQUITY_DEG 23.439291111111111111111111111111111111111111111111
#define JME_DEG_TO_RAD               0.017453292519943295769236907684886127134428718885417

#ifdef JME_HAVE_CALCEPH
#include <calceph.h>
static t_calcephbin *g_kernel = 0;
static char g_kernel_path[JME_PATH_MAX] = "";

/*
 * CALCEPH diagnostics are captured per thread (error-handler type 3) instead
 * of being printed to stderr, and are appended to the JME error message.
 */
static JME_THREAD_LOCAL char g_calceph_message[192];

static void jme_calceph_error_capture(const char *message)
{
    jme_set_string(g_calceph_message, sizeof(g_calceph_message), message);
}

static void jme_calceph_install_handler(void)
{
    calceph_seterrorhandler(3, jme_calceph_error_capture);
}

static void jme_calceph_clear_message(void)
{
    g_calceph_message[0] = '\0';
}

static void jme_calceph_fail(char *error, const char *what)
{
    if (error == 0)
        return;
    if (g_calceph_message[0] != '\0') {
        char buf[256];
        size_t n = strlen(what), i = 0, k;
        const char *m = g_calceph_message;
        while (*m == ' ' || *m == '\n')
            m++;
        if (n > 120)
            n = 120;
        memcpy(buf, what, n);
        i = n;
        buf[i++] = ':';
        buf[i++] = ' ';
        for (k = 0; m[k] != '\0' && i + 1 < sizeof(buf) && i < 255; k++) {
            buf[i++] = (m[k] == '\n') ? ' ' : m[k];
        }
        while (i > 0 && buf[i - 1] == ' ')
            i--;
        buf[i] = '\0';
        jme_set_error(error, buf);
    } else {
        jme_set_error(error, what);
    }
}
#endif

/* Integer-day split for CALCEPH: jd0 = floor(jd) keeps fraction = jd - jd0 exact. */
static double jme_jpl_day_floor(double jd_time)
{
    return isfinite(jd_time) ? floor(jd_time) : NAN;
}

#ifdef JME_HAVE_CALCEPH
static int jme_jpl_path_is_absolute(const char *path)
{
#ifdef _WIN32
    if (path[0] == '/' || path[0] == '\\') {
        return 1;
    }
    return isalpha((unsigned char)path[0]) && path[1] == ':' && (path[2] == '/' || path[2] == '\\');
#else
    return path[0] == '/';
#endif
}

/*
 * Resolve a kernel path against the configured ephemeris directory. An
 * absolute path (or an empty directory) is copied unchanged; a relative name
 * is joined to the ephemeris path. Paths that do not fit are rejected.
 */
static int jme_jpl_resolve_path(const char *path, char *out, size_t out_size, char *error)
{
    jme_context *ctx = jme_get_context();
    const char *file = (path != 0 && path[0] != '\0') ? path : ctx->jpl_file;
    size_t dir_len;
    size_t file_len;
    size_t need;
    int add_separator;
    char last;

    if (out == 0 || out_size == 0) {
        jme_set_error(error, "JPL kernel path buffer is required");
        return JME_ERR;
    }
    out[0] = '\0';
    if (file == 0 || file[0] == '\0') {
        jme_set_error(error, "JPL kernel path is empty (call jme_set_jpl_file or jme_jpl_open)");
        return JME_ERR;
    }
    if (jme_jpl_path_is_absolute(file) || ctx->ephemeris_path[0] == '\0') {
        if (strlen(file) >= out_size) {
            jme_set_error(error, "JPL kernel path exceeds the supported length");
            return JME_ERR;
        }
        jme_set_string(out, out_size, file);
        return JME_OK;
    }

    dir_len = strlen(ctx->ephemeris_path);
    file_len = strlen(file);
    last = dir_len > 0 ? ctx->ephemeris_path[dir_len - 1] : '\0';
    add_separator = dir_len > 0 && last != '/' && last != '\\';
    need = dir_len + (size_t)add_separator + file_len + 1;
    if (need > out_size) {
        jme_set_error(error, "JPL kernel path exceeds the supported length");
        return JME_ERR;
    }
    memcpy(out, ctx->ephemeris_path, dir_len);
    if (add_separator) {
        out[dir_len++] = '/';
    }
    memcpy(out + dir_len, file, file_len + 1);
    return JME_OK;
}
#endif

static void jme_zero_state(double *state)
{
    int i;

    if (state == 0) {
        return;
    }

    for (i = 0; i < 6; i++) {
        state[i] = 0.0;
    }
}

static void jme_zero_order_state(double *state, int order)
{
    int i;
    int count;

    if (state == 0) {
        return;
    }

    count = 3 * (order + 1);
    for (i = 0; i < count; i++) {
        state[i] = 0.0;
    }
}

static void jme_equatorial_rectangular_to_j2000_ecliptic(const double *equatorial, double *ecliptic)
{
    double eps = JME_J2000_MEAN_OBLIQUITY_DEG * JME_DEG_TO_RAD;
    double ce = cos(eps);
    double se = sin(eps);

    ecliptic[0] = equatorial[0];
    ecliptic[1] = (equatorial[1] * ce) + (equatorial[2] * se);
    ecliptic[2] = (-equatorial[1] * se) + (equatorial[2] * ce);
    ecliptic[3] = equatorial[3];
    ecliptic[4] = (equatorial[4] * ce) + (equatorial[5] * se);
    ecliptic[5] = (-equatorial[4] * se) + (equatorial[5] * ce);
}

const char *jme_jpl_engine_version(char *buffer, size_t buffer_size)
{
#ifdef JME_HAVE_CALCEPH
    char version[256];

    calceph_getversion_str(version);
    if (buffer != 0 && buffer_size > 0) {
        jme_set_string(buffer, buffer_size, version);
    }

    return buffer != 0 ? buffer : CALCEPH_VERSION_STRING;
#else
    if (buffer != 0 && buffer_size > 0) {
        jme_set_string(buffer, buffer_size, "");
    }

    return "";
#endif
}

int jme_jpl_is_available(void)
{
#ifdef JME_HAVE_CALCEPH
    return 1;
#else
    return 0;
#endif
}

int jme_jpl_is_open(void)
{
#ifdef JME_HAVE_CALCEPH
    return g_kernel != 0;
#else
    return 0;
#endif
}

void jme_jpl_close(void)
{
#ifdef JME_HAVE_CALCEPH
    if (g_kernel != 0) {
        calceph_close(g_kernel);
        g_kernel = 0;
    }
    g_kernel_path[0] = '\0';
#endif
}

int jme_jpl_open(const char *path, char *error)
{
#ifdef JME_HAVE_CALCEPH
    char resolved_path[JME_PATH_MAX];
    jme_context *ctx = jme_get_context();

    if (jme_jpl_resolve_path(path, resolved_path, sizeof resolved_path, error) != JME_OK) {
        return JME_ERR;
    }

    if (g_kernel != 0 && strcmp(g_kernel_path, resolved_path) == 0) {
        return JME_OK;
    }

    jme_jpl_close();
    jme_calceph_install_handler();
    g_kernel = calceph_open(resolved_path);

    if (g_kernel == 0) {
        jme_set_error(error, "JPL kernel open failed");
        return JME_ERR;
    }

    jme_set_string(g_kernel_path, sizeof(g_kernel_path), resolved_path);
    /* Keep the configured name relative when the caller supplied one. */
    if (path != 0 && path[0] != '\0') {
        jme_set_string(ctx->jpl_file, sizeof(ctx->jpl_file), path);
    }

    return JME_OK;
#else
    (void)path;
    jme_set_error(error, "CALCEPH support is unavailable in this build");
    return JME_ERR;
#endif
}

int internal_jpl_ensure_open(char *error)
{
#ifdef JME_HAVE_CALCEPH
    if (g_kernel != 0) {
        return JME_OK;
    }
    return jme_jpl_open(0, error);
#else
    jme_set_error(error, "CALCEPH support is unavailable in this build");
    return JME_ERR;
#endif
}

int jme_jpl_open_array(int path_count, const char *const *paths, char *error)
{
#ifdef JME_HAVE_CALCEPH
    jme_context *ctx = jme_get_context();
    int i;

    if (path_count <= 0 || paths == 0) {
        jme_set_error(error, "JPL kernel path array is empty");
        return JME_ERR;
    }

    for (i = 0; i < path_count; i++) {
        if (paths[i] == 0 || paths[i][0] == '\0') {
            jme_set_error(error, "JPL kernel path array contains empty path");
            return JME_ERR;
        }
    }

    jme_jpl_close();
    jme_calceph_install_handler();
    g_kernel = calceph_open_array(path_count, paths);

    if (g_kernel == 0) {
        jme_set_error(error, "JPL kernel array open failed");
        return JME_ERR;
    }

    jme_set_string(g_kernel_path, sizeof(g_kernel_path), paths[0]);
    jme_set_string(ctx->jpl_file, sizeof(ctx->jpl_file), paths[0]);
    return JME_OK;
#else
    (void)path_count;
    (void)paths;
    jme_set_error(error, "CALCEPH support is unavailable in this build");
    return JME_ERR;
#endif
}

int jme_jpl_prefetch(char *error)
{
#ifdef JME_HAVE_CALCEPH
    if (g_kernel == 0) {
        jme_set_error(error, "JPL kernel is closed");
        return JME_ERR;
    }

    if (calceph_prefetch(g_kernel) == 0) {
        jme_set_error(error, "JPL kernel prefetch failed");
        return JME_ERR;
    }

    return JME_OK;
#else
    jme_set_error(error, "CALCEPH support is unavailable in this build");
    return JME_ERR;
#endif
}

int jme_jpl_is_thread_safe(char *error)
{
#ifdef JME_HAVE_CALCEPH
    if (g_kernel == 0) {
        jme_set_error(error, "JPL kernel is closed");
        return JME_ERR;
    }

    return calceph_isthreadsafe(g_kernel);
#else
    jme_set_error(error, "CALCEPH support is unavailable in this build");
    return JME_ERR;
#endif
}

int jme_jpl_id_by_name(const char *name, int *id, char *error)
{
#ifdef JME_HAVE_CALCEPH
    if (id != 0) {
        *id = 0;
    }

    if (g_kernel == 0) {
        jme_set_error(error, "JPL kernel is closed");
        return JME_ERR;
    }

    if (name == 0 || id == 0) {
        jme_set_error(error, "JPL kernel id lookup argument is null");
        return JME_ERR;
    }

    if (calceph_getidbyname(g_kernel, name, CALCEPH_USE_NAIFID, id) == 0) {
        jme_set_error(error, "JPL kernel id lookup failed");
        return JME_ERR;
    }

    return JME_OK;
#else
    (void)name;
    if (id != 0) {
        *id = 0;
    }
    jme_set_error(error, "CALCEPH support is unavailable in this build");
    return JME_ERR;
#endif
}

int jme_jpl_name_by_id(int id, char *name, unsigned int name_size, char *error)
{
#ifdef JME_HAVE_CALCEPH
    t_calcephcharvalue value;

    if (name != 0 && name_size > 0U) {
        name[0] = '\0';
    }

    if (g_kernel == 0) {
        jme_set_error(error, "JPL kernel is closed");
        return JME_ERR;
    }

    if (name == 0 || name_size == 0U) {
        jme_set_error(error, "JPL kernel name lookup output is null");
        return JME_ERR;
    }

    if (calceph_getnamebyidss(g_kernel, id, CALCEPH_USE_NAIFID, value) == 0) {
        jme_set_error(error, "JPL kernel name lookup failed");
        return JME_ERR;
    }

    jme_set_string(name, name_size, value);
    return JME_OK;
#else
    (void)id;
    if (name != 0 && name_size > 0U) {
        name[0] = '\0';
    }
    jme_set_error(error, "CALCEPH support is unavailable in this build");
    return JME_ERR;
#endif
}

int jme_jpl_max_supported_order(int segment_type)
{
#ifdef JME_HAVE_CALCEPH
    return calceph_getmaxsupportedorder(segment_type);
#else
    (void)segment_type;
    return JME_ERR;
#endif
}

int jme_jpl_timescale(void)
{
#ifdef JME_HAVE_CALCEPH
    if (g_kernel == 0) {
        return JME_JPL_TIMESCALE_UNKNOWN;
    }

    return calceph_gettimescale(g_kernel);
#else
    return JME_JPL_TIMESCALE_UNKNOWN;
#endif
}

int jme_jpl_coverage(double *first_time, double *last_time, int *continuous, char *error)
{
#ifdef JME_HAVE_CALCEPH
    if (g_kernel == 0) {
        jme_set_error(error, "JPL kernel is closed");
        return JME_ERR;
    }

    if (calceph_gettimespan(g_kernel, first_time, last_time, continuous) == 0) {
        jme_set_error(error, "JPL kernel coverage query failed");
        return JME_ERR;
    }

    return JME_OK;
#else
    (void)first_time;
    (void)last_time;
    (void)continuous;
    jme_set_error(error, "CALCEPH support is unavailable in this build");
    return JME_ERR;
#endif
}

int jme_jpl_file_version(char *buffer, unsigned int buffer_size, char *error)
{
#ifdef JME_HAVE_CALCEPH
    char version[CALCEPH_MAX_CONSTANTVALUE];

    if (buffer != 0 && buffer_size > 0) {
        buffer[0] = '\0';
    }

    if (g_kernel == 0) {
        jme_set_error(error, "JPL kernel is closed");
        return JME_ERR;
    }

    if (calceph_getfileversion(g_kernel, version) == 0) {
        jme_set_error(error, "JPL kernel file-version query failed");
        return JME_ERR;
    }

    if (buffer != 0 && buffer_size > 0) {
        jme_set_string(buffer, buffer_size, version);
    }

    return JME_OK;
#else
    if (buffer != 0 && buffer_size > 0) {
        buffer[0] = '\0';
    }
    jme_set_error(error, "CALCEPH support is unavailable in this build");
    return JME_ERR;
#endif
}

int jme_jpl_current_file_data(
    char *path,
    unsigned int path_size,
    double *first_time,
    double *last_time,
    int *continuous,
    char *error
)
{
#ifdef JME_HAVE_CALCEPH
    if (path != 0 && path_size > 0U) {
        path[0] = '\0';
    }
    if (first_time != 0) {
        *first_time = 0.0;
    }
    if (last_time != 0) {
        *last_time = 0.0;
    }
    if (continuous != 0) {
        *continuous = 0;
    }

    if (g_kernel == 0) {
        jme_set_error(error, "JPL kernel is closed");
        return JME_ERR;
    }

    if (first_time == 0 || last_time == 0 || continuous == 0) {
        jme_set_error(error, "JPL current file data output is null");
        return JME_ERR;
    }

    if (calceph_gettimespan(g_kernel, first_time, last_time, continuous) == 0) {
        *first_time = 0.0;
        *last_time = 0.0;
        *continuous = 0;
        jme_set_error(error, "JPL kernel coverage query failed");
        return JME_ERR;
    }

    if (path != 0 && path_size > 0U) {
        jme_set_string(path, path_size, g_kernel_path);
    }

    return JME_OK;
#else
    if (path != 0 && path_size > 0U) {
        path[0] = '\0';
    }
    if (first_time != 0) {
        *first_time = 0.0;
    }
    if (last_time != 0) {
        *last_time = 0.0;
    }
    if (continuous != 0) {
        *continuous = 0;
    }
    jme_set_error(error, "CALCEPH support is unavailable in this build");
    return JME_ERR;
#endif
}

int jme_jpl_constant(const char *name, double *value, char *error)
{
#ifdef JME_HAVE_CALCEPH
    if (value != 0) {
        *value = 0.0;
    }

    if (g_kernel == 0) {
        jme_set_error(error, "JPL kernel is closed");
        return JME_ERR;
    }

    if (name == 0 || value == 0) {
        jme_set_error(error, "JPL kernel constant query argument is null");
        return JME_ERR;
    }

    if (calceph_getconstant(g_kernel, name, value) == 0) {
        jme_set_error(error, "JPL kernel constant query failed");
        return JME_ERR;
    }

    return JME_OK;
#else
    (void)name;
    if (value != 0) {
        *value = 0.0;
    }
    jme_set_error(error, "CALCEPH support is unavailable in this build");
    return JME_ERR;
#endif
}

int jme_jpl_constant_count(char *error)
{
#ifdef JME_HAVE_CALCEPH
    int count;

    if (g_kernel == 0) {
        jme_set_error(error, "JPL kernel is closed");
        return JME_ERR;
    }

    count = calceph_getconstantcount(g_kernel);
    if (count < 0) {
        jme_set_error(error, "JPL kernel constant-count query failed");
        return JME_ERR;
    }

    return count;
#else
    jme_set_error(error, "CALCEPH support is unavailable in this build");
    return JME_ERR;
#endif
}

int jme_jpl_constant_index(
    int index,
    char *name,
    unsigned int name_size,
    double *value,
    char *error
)
{
#ifdef JME_HAVE_CALCEPH
    char local_name[CALCEPH_MAX_CONSTANTNAME];

    if (name != 0 && name_size > 0U) {
        name[0] = '\0';
    }
    if (value != 0) {
        *value = 0.0;
    }

    if (g_kernel == 0) {
        jme_set_error(error, "JPL kernel is closed");
        return JME_ERR;
    }

    if (value == 0) {
        jme_set_error(error, "JPL kernel constant-index value output is null");
        return JME_ERR;
    }

    if (calceph_getconstantindex(g_kernel, index, local_name, value) == 0) {
        jme_set_error(error, "JPL kernel constant-index query failed");
        return JME_ERR;
    }

    if (name != 0 && name_size > 0U) {
        jme_set_string(name, name_size, local_name);
    }

    return JME_OK;
#else
    (void)index;
    if (name != 0 && name_size > 0U) {
        name[0] = '\0';
    }
    if (value != 0) {
        *value = 0.0;
    }
    jme_set_error(error, "CALCEPH support is unavailable in this build");
    return JME_ERR;
#endif
}

int jme_jpl_constant_vector(const char *name, double *values, int value_count, char *error)
{
#ifdef JME_HAVE_CALCEPH
    int i;

    if (values != 0) {
        for (i = 0; i < value_count; i++) {
            values[i] = 0.0;
        }
    }

    if (g_kernel == 0) {
        jme_set_error(error, "JPL kernel is closed");
        return JME_ERR;
    }

    if (name == 0 || values == 0 || value_count <= 0) {
        jme_set_error(error, "JPL kernel constant-vector query argument is invalid");
        return JME_ERR;
    }

    if (calceph_getconstantvd(g_kernel, name, values, value_count) == 0) {
        jme_set_error(error, "JPL kernel constant-vector query failed");
        return JME_ERR;
    }

    return JME_OK;
#else
    int i;

    (void)name;
    if (values != 0) {
        for (i = 0; i < value_count; i++) {
            values[i] = 0.0;
        }
    }
    jme_set_error(error, "CALCEPH support is unavailable in this build");
    return JME_ERR;
#endif
}

int jme_jpl_constant_string(const char *name, char *value, unsigned int value_size, char *error)
{
#ifdef JME_HAVE_CALCEPH
    t_calcephcharvalue local_value;

    if (value != 0 && value_size > 0U) {
        value[0] = '\0';
    }

    if (g_kernel == 0) {
        jme_set_error(error, "JPL kernel is closed");
        return JME_ERR;
    }

    if (name == 0 || value == 0 || value_size == 0U) {
        jme_set_error(error, "JPL kernel constant-string query argument is invalid");
        return JME_ERR;
    }

    if (calceph_getconstantss(g_kernel, name, local_value) == 0) {
        jme_set_error(error, "JPL kernel constant-string query failed");
        return JME_ERR;
    }

    jme_set_string(value, value_size, local_value);
    return JME_OK;
#else
    (void)name;
    if (value != 0 && value_size > 0U) {
        value[0] = '\0';
    }
    jme_set_error(error, "CALCEPH support is unavailable in this build");
    return JME_ERR;
#endif
}

int jme_jpl_constant_string_vector(
    const char *name,
    char *values,
    unsigned int single_value_size,
    int value_count,
    char *error
)
{
#ifdef JME_HAVE_CALCEPH
    int i;
    t_calcephcharvalue local_values[16];

    if (values != 0 && single_value_size > 0U) {
        for (i = 0; i < value_count; i++) {
            values[i * (int)single_value_size] = '\0';
        }
    }

    if (g_kernel == 0) {
        jme_set_error(error, "JPL kernel is closed");
        return JME_ERR;
    }

    if (name == 0 || values == 0 || single_value_size == 0U || value_count <= 0 ||
        value_count > 16) {
        jme_set_error(error, "JPL kernel constant-string-vector query argument is invalid");
        return JME_ERR;
    }

    if (calceph_getconstantvs(g_kernel, name, local_values, value_count) == 0) {
        jme_set_error(error, "JPL kernel constant-string-vector query failed");
        return JME_ERR;
    }

    for (i = 0; i < value_count; i++) {
        jme_set_string(values + (i * (int)single_value_size), single_value_size, local_values[i]);
    }

    return JME_OK;
#else
    int i;

    (void)name;
    if (values != 0 && single_value_size > 0U) {
        for (i = 0; i < value_count; i++) {
            values[i * (int)single_value_size] = '\0';
        }
    }
    jme_set_error(error, "CALCEPH support is unavailable in this build");
    return JME_ERR;
#endif
}

int jme_jpl_position_record_count(char *error)
{
#ifdef JME_HAVE_CALCEPH
    int count;

    if (g_kernel == 0) {
        jme_set_error(error, "JPL kernel is closed");
        return JME_ERR;
    }

    count = calceph_getpositionrecordcount(g_kernel);
    if (count < 0) {
        jme_set_error(error, "JPL kernel position-record-count query failed");
        return JME_ERR;
    }

    return count;
#else
    jme_set_error(error, "CALCEPH support is unavailable in this build");
    return JME_ERR;
#endif
}

int jme_jpl_position_record_index(
    int index,
    int *target,
    int *center,
    double *first_time,
    double *last_time,
    int *frame,
    int *segment_type,
    char *error
)
{
#ifdef JME_HAVE_CALCEPH
    if (target != 0) {
        *target = 0;
    }
    if (center != 0) {
        *center = 0;
    }
    if (first_time != 0) {
        *first_time = 0.0;
    }
    if (last_time != 0) {
        *last_time = 0.0;
    }
    if (frame != 0) {
        *frame = 0;
    }
    if (segment_type != 0) {
        *segment_type = 0;
    }

    if (g_kernel == 0) {
        jme_set_error(error, "JPL kernel is closed");
        return JME_ERR;
    }

    if (target == 0 || center == 0 || first_time == 0 || last_time == 0 || frame == 0 ||
        segment_type == 0) {
        jme_set_error(error, "JPL kernel position-record output is null");
        return JME_ERR;
    }

    if (calceph_getpositionrecordindex2(
            g_kernel,
            index,
            target,
            center,
            first_time,
            last_time,
            frame,
            segment_type
        ) == 0) {
        jme_set_error(error, "JPL kernel position-record query failed");
        return JME_ERR;
    }

    return JME_OK;
#else
    (void)index;
    if (target != 0) {
        *target = 0;
    }
    if (center != 0) {
        *center = 0;
    }
    if (first_time != 0) {
        *first_time = 0.0;
    }
    if (last_time != 0) {
        *last_time = 0.0;
    }
    if (frame != 0) {
        *frame = 0;
    }
    if (segment_type != 0) {
        *segment_type = 0;
    }
    jme_set_error(error, "CALCEPH support is unavailable in this build");
    return JME_ERR;
#endif
}

int jme_jpl_orientation_record_count(char *error)
{
#ifdef JME_HAVE_CALCEPH
    int count;

    if (g_kernel == 0) {
        jme_set_error(error, "JPL kernel is closed");
        return JME_ERR;
    }

    count = calceph_getorientrecordcount(g_kernel);
    if (count < 0) {
        jme_set_error(error, "JPL kernel orientation-record-count query failed");
        return JME_ERR;
    }

    return count;
#else
    jme_set_error(error, "CALCEPH support is unavailable in this build");
    return JME_ERR;
#endif
}

int jme_jpl_orientation_record_index(
    int index,
    int *target,
    double *first_time,
    double *last_time,
    int *frame,
    int *segment_type,
    char *error
)
{
#ifdef JME_HAVE_CALCEPH
    if (target != 0) {
        *target = 0;
    }
    if (first_time != 0) {
        *first_time = 0.0;
    }
    if (last_time != 0) {
        *last_time = 0.0;
    }
    if (frame != 0) {
        *frame = 0;
    }
    if (segment_type != 0) {
        *segment_type = 0;
    }

    if (g_kernel == 0) {
        jme_set_error(error, "JPL kernel is closed");
        return JME_ERR;
    }

    if (target == 0 || first_time == 0 || last_time == 0 || frame == 0 || segment_type == 0) {
        jme_set_error(error, "JPL kernel orientation-record output is null");
        return JME_ERR;
    }

    if (calceph_getorientrecordindex2(
            g_kernel,
            index,
            target,
            first_time,
            last_time,
            frame,
            segment_type
        ) == 0) {
        jme_set_error(error, "JPL kernel orientation-record query failed");
        return JME_ERR;
    }

    return JME_OK;
#else
    (void)index;
    if (target != 0) {
        *target = 0;
    }
    if (first_time != 0) {
        *first_time = 0.0;
    }
    if (last_time != 0) {
        *last_time = 0.0;
    }
    if (frame != 0) {
        *frame = 0;
    }
    if (segment_type != 0) {
        *segment_type = 0;
    }
    jme_set_error(error, "CALCEPH support is unavailable in this build");
    return JME_ERR;
#endif
}

#ifdef JME_HAVE_CALCEPH
static int jme_calceph_unit(int unit)
{
#ifdef JME_HAVE_CALCEPH
    switch (unit) {
    case JME_VECTOR_AU_PER_DAY:
        return CALCEPH_USE_NAIFID | CALCEPH_UNIT_KM | CALCEPH_UNIT_DAY;
    case JME_VECTOR_KM_PER_DAY:
        return CALCEPH_USE_NAIFID | CALCEPH_UNIT_KM | CALCEPH_UNIT_DAY;
    case JME_VECTOR_AU_PER_SECOND:
        return CALCEPH_USE_NAIFID | CALCEPH_UNIT_KM | CALCEPH_UNIT_SEC;
    case JME_VECTOR_KM_PER_SECOND:
        return CALCEPH_USE_NAIFID | CALCEPH_UNIT_KM | CALCEPH_UNIT_SEC;
    default:
        return 0;
    }
#else
    (void)unit;
    return 0;
#endif
}

static int jme_calceph_orientation_unit(int unit)
{
#ifdef JME_HAVE_CALCEPH
    switch (unit) {
    case JME_ORIENTATION_RAD_PER_DAY:
        return CALCEPH_USE_NAIFID | CALCEPH_UNIT_RAD | CALCEPH_UNIT_DAY;
    case JME_ORIENTATION_RAD_PER_SECOND:
        return CALCEPH_USE_NAIFID | CALCEPH_UNIT_RAD | CALCEPH_UNIT_SEC;
    default:
        return 0;
    }
#else
    (void)unit;
    return 0;
#endif
}
#endif

static int jme_order_is_valid(int order)
{
    return order >= 0 && order <= 3;
}

/*
 * Split-time TT -> TDB using the ERFA Fairhead-Bretagnon model. Iterate the
 * periodic correction against the resulting TDB date until binary64 reaches
 * a fixed point; no display rounding or arbitrary time quantization is used.
 */
int internal_tt_to_tdb_split(
    double jd0_tt,
    double fraction_tt,
    double *jd0_tdb,
    double *fraction_tdb,
    char *error
)
{
    double dtr;
    double dtr_next;
    double trial_fraction;
    double trial_day;
    double carry;
    int iteration;
    int converged = 0;

    if (error != 0) {
        error[0] = '\0';
    }
    if (jd0_tdb == 0 || fraction_tdb == 0) {
        jme_set_error(error, "TDB two-part Julian-date outputs are required");
        return JME_ERR;
    }
    *jd0_tdb = NAN;
    *fraction_tdb = NAN;
    if (!isfinite(jd0_tt) || !isfinite(fraction_tt) || floor(jd0_tt) != jd0_tt ||
        fraction_tt < 0.0 || fraction_tt >= 1.0) {
        jme_set_error(error, "TT input must be a normalized two-part Julian date");
        return JME_ERR;
    }

    dtr = internal_dtdb(jd0_tt, fraction_tt);
    if (!isfinite(dtr)) {
        jme_set_error(error, "ERFA TDB-TT conversion returned a non-finite offset");
        return JME_ERR;
    }

    for (iteration = 0; iteration < 16; iteration++) {
        trial_fraction = fraction_tt + dtr / JME_SECONDS_PER_DAY;
        carry = floor(trial_fraction);
        trial_day = jd0_tt + carry;
        trial_fraction -= carry;

        dtr_next = internal_dtdb(trial_day, trial_fraction);
        if (!isfinite(dtr_next)) {
            jme_set_error(error, "ERFA TDB-TT iteration returned a non-finite offset");
            return JME_ERR;
        }
        if (dtr_next == dtr) {
            converged = 1;
            dtr = dtr_next;
            break;
        }
        dtr = dtr_next;
    }

    if (!converged) {
        jme_set_error(error, "TT-to-TDB iteration did not reach a binary64 fixed point");
        return JME_ERR;
    }

    trial_fraction = fraction_tt + dtr / JME_SECONDS_PER_DAY;
    carry = floor(trial_fraction);
    *jd0_tdb = jd0_tt + carry;
    *fraction_tdb = trial_fraction - carry;
    if (!isfinite(*jd0_tdb) || !isfinite(*fraction_tdb) || floor(*jd0_tdb) != *jd0_tdb ||
        *fraction_tdb < 0.0 || *fraction_tdb >= 1.0) {
        *jd0_tdb = NAN;
        *fraction_tdb = NAN;
        jme_set_error(error, "TT-to-TDB conversion did not produce a normalized date");
        return JME_ERR;
    }

    return JME_OK;
}

/* Single-double convenience wrapper: TT JD -> TDB JD (geocentre, ERFA dtdb). */
int internal_tt_to_tdb_jd(double jd_tt, double *jd_tdb, char *error)
{
    double d0, f0;
    if (jd_tdb == 0) {
        jme_set_error(error, "TDB output is required");
        return JME_ERR;
    }
    *jd_tdb = NAN;
    if (!isfinite(jd_tt)) {
        jme_set_error(error, "TT Julian date must be finite");
        return JME_ERR;
    }
    {
        double day = floor(jd_tt);
        if (internal_tt_to_tdb_split(day, jd_tt - day, &d0, &f0, error) != JME_OK) {
            return JME_ERR;
        }
    }
    *jd_tdb = d0 + f0;
    return JME_OK;
}

/* IAU 2006 time-scale relation, delegated to the ERFA implementation. */
int internal_tdb_to_tcb_split(
    double jd0_tdb,
    double fraction_tdb,
    double *jd0_tcb,
    double *fraction_tcb,
    char *error
)
{
    double carry;

    if (error != 0) {
        error[0] = '\0';
    }
    if (jd0_tcb == 0 || fraction_tcb == 0) {
        jme_set_error(error, "TCB two-part Julian-date outputs are required");
        return JME_ERR;
    }
    *jd0_tcb = NAN;
    *fraction_tcb = NAN;

    if (!isfinite(jd0_tdb) || !isfinite(fraction_tdb) || floor(jd0_tdb) != jd0_tdb ||
        fraction_tdb < 0.0 || fraction_tdb >= 1.0) {
        jme_set_error(error, "TDB input must be a normalized two-part Julian date");
        return JME_ERR;
    }
    if (jme_erfa_tdbtcb(jd0_tdb, fraction_tdb, jd0_tcb, fraction_tcb) != 0 || !isfinite(*jd0_tcb) ||
        !isfinite(*fraction_tcb)) {
        *jd0_tcb = NAN;
        *fraction_tcb = NAN;
        jme_set_error(error, "ERFA TDB-to-TCB conversion failed");
        return JME_ERR;
    }

    carry = floor(*fraction_tcb);
    *jd0_tcb += carry;
    *fraction_tcb -= carry;
    if (floor(*jd0_tcb) != *jd0_tcb || *fraction_tcb < 0.0 || *fraction_tcb >= 1.0) {
        *jd0_tcb = NAN;
        *fraction_tcb = NAN;
        jme_set_error(error, "TDB-to-TCB conversion did not produce a normalized date");
        return JME_ERR;
    }
    return JME_OK;
}

int internal_tt_split_to_jpl_time_split(
    double jd0_tt,
    double fraction_tt,
    double *jd0_native,
    double *fraction_native,
    char *error
)
{
    int native_scale;
    double jd0_tdb;
    double fraction_tdb;

    if (error != 0) {
        error[0] = '\0';
    }
    if (jd0_native == 0 || fraction_native == 0) {
        jme_set_error(error, "Native time-scale output pair is required");
        return JME_ERR;
    }
    *jd0_native = NAN;
    *fraction_native = NAN;

    native_scale = jme_jpl_timescale();
    if (native_scale != JME_JPL_TIMESCALE_TDB && native_scale != JME_JPL_TIMESCALE_TCB) {
        jme_set_error(error, "A loaded JPL kernel with a known TDB or TCB time scale is required");
        return JME_ERR;
    }

    if (internal_tt_to_tdb_split(jd0_tt, fraction_tt, &jd0_tdb, &fraction_tdb, error) != JME_OK) {
        return JME_ERR;
    }
    if (native_scale == JME_JPL_TIMESCALE_TDB) {
        *jd0_native = jd0_tdb;
        *fraction_native = fraction_tdb;
        return JME_OK;
    }

    return internal_tdb_to_tcb_split(jd0_tdb, fraction_tdb, jd0_native, fraction_native, error);
}

int internal_tt_to_jpl_time_split(
    double jd_tt,
    double *jd0_native,
    double *fraction_native,
    char *error
)
{
    double jd0_tt;
    double fraction_tt;

    if (!isfinite(jd_tt)) {
        if (jd0_native != 0) {
            *jd0_native = NAN;
        }
        if (fraction_native != 0) {
            *fraction_native = NAN;
        }
        jme_set_error(error, "TT Julian date must be finite");
        return JME_ERR;
    }

    jd0_tt = floor(jd_tt);
    fraction_tt = jd_tt - jd0_tt;
    return internal_tt_split_to_jpl_time_split(
        jd0_tt,
        fraction_tt,
        jd0_native,
        fraction_native,
        error
    );
}

static JME_THREAD_LOCAL int g_jpl_center_substituted;

int internal_jpl_last_center_substituted(void)
{
    return g_jpl_center_substituted;
}

static int jme_planet_system_barycenter(int body)
{
    switch (body) {
    case JME_BODY_MERCURY:
        return JME_BODY_MERCURY_BARYCENTER;
    case JME_BODY_VENUS:
        return JME_BODY_VENUS_BARYCENTER;
    case JME_BODY_MARS:
        return JME_BODY_MARS_BARYCENTER;
    case JME_BODY_JUPITER:
        return JME_BODY_JUPITER_BARYCENTER;
    case JME_BODY_SATURN:
        return JME_BODY_SATURN_BARYCENTER;
    case JME_BODY_URANUS:
        return JME_BODY_URANUS_BARYCENTER;
    case JME_BODY_NEPTUNE:
        return JME_BODY_NEPTUNE_BARYCENTER;
    case JME_BODY_PLUTO:
        return JME_BODY_PLUTO_BARYCENTER;
    default:
        return -1;
    }
}

int internal_jpl_body_state_tt(
    double jd_tt,
    int target_body,
    int center_body,
    int unit,
    double *state,
    char *error
)
{
    return internal_jpl_body_state_tt_offset(
        jd_tt,
        0.0,
        target_body,
        center_body,
        unit,
        state,
        error
    );
}

/*
 * State at TT = jd_tt + offset_days. The offset (e.g. minus a light time) is
 * added to the day fraction, not to jd_tt, so that it is not rounded to the
 * binary64 spacing of a full Julian date (2^-31 day = 40 microseconds near
 * JD 2.4e6, i.e. up to 0.6 m at 30 km/s).
 */
int internal_jpl_body_state_tt_offset(
    double jd_tt,
    double offset_days,
    int target_body,
    int center_body,
    int unit,
    double *state,
    char *error
)
{
    double jd0_native;
    double fraction_native;
    double jd0_tt, fraction_tt, carry;
    int bary;

    g_jpl_center_substituted = 0;
    if (state == 0) {
        jme_set_error(error, "JPL body-state output is required");
        return JME_ERR;
    }
    jme_zero_state(state);
    if (!isfinite(jd_tt) || !isfinite(offset_days)) {
        jme_set_error(error, "TT Julian date must be finite");
        return JME_ERR;
    }
    if (internal_jpl_ensure_open(error) != JME_OK) {
        return JME_ERR;
    }
    jd0_tt = floor(jd_tt);
    fraction_tt = (jd_tt - jd0_tt) + offset_days;
    carry = floor(fraction_tt);
    jd0_tt += carry;
    fraction_tt -= carry;
    if (fraction_tt >= 1.0) {
        jd0_tt += 1.0;
        fraction_tt = 0.0;
    }

    if (internal_tt_split_to_jpl_time_split(
            jd0_tt,
            fraction_tt,
            &jd0_native,
            &fraction_native,
            error
        ) != JME_OK) {
        return JME_ERR;
    }

    if (jme_jpl_body_state_split(
            jd0_native,
            fraction_native,
            target_body,
            center_body,
            unit,
            state,
            error
        ) == JME_OK) {
        return JME_OK;
    }
    /* Opt-in planet-centre -> system-barycentre substitution (never silent: see
     * jme_calc_resolve_engine and docs/ACCURACY.md). */
    bary = jme_planet_system_barycenter(target_body);
    if (jme_context_jpl_center_policy() == JME_JPL_CENTER_BARYCENTER && bary >= 0 &&
        jme_jpl_body_state_split(
            jd0_native,
            fraction_native,
            bary,
            center_body,
            unit,
            state,
            error
        ) == JME_OK) {
        g_jpl_center_substituted = 1;
        jme_set_error(error, "");
        return JME_OK;
    }
    return JME_ERR;
}

static int jme_utc_to_jpl_jd_split(
    int year,
    int month,
    int day,
    int hour,
    int minute,
    double second,
    int calendar,
    double *jd0_native,
    double *fraction_native,
    char *error
)
{
    double jd0_tt;
    double fraction_tt;

    if (jd0_native == 0 || fraction_native == 0) {
        jme_set_error(error, "Native time-scale output pair is required");
        return JME_ERR;
    }
    *jd0_native = NAN;
    *fraction_native = NAN;

    if (internal_utc_to_tt_split(
            year,
            month,
            day,
            hour,
            minute,
            second,
            calendar,
            &jd0_tt,
            &fraction_tt,
            error
        ) != JME_OK) {
        return JME_ERR;
    }

    return internal_tt_split_to_jpl_time_split(
        jd0_tt,
        fraction_tt,
        jd0_native,
        fraction_native,
        error
    );
}

#ifdef JME_HAVE_CALCEPH
static void jme_convert_order_state_from_km(double *state, int order, int output_unit)
{
    int i;
    int count = 3 * (order + 1);

    if (output_unit != JME_VECTOR_AU_PER_DAY && output_unit != JME_VECTOR_AU_PER_SECOND) {
        return;
    }

    for (i = 0; i < count; i++) {
        state[i] /= JME_AU_KM;
    }
}
#endif

int jme_jpl_orientation_state_naif(
    double jd_time,
    int target_naif,
    int unit,
    double *state,
    char *error
)
{
    double jd0 = jme_jpl_day_floor(jd_time);
    double fraction = jd_time - jd0;

    return jme_jpl_orientation_state_split_naif(jd0, fraction, target_naif, unit, state, error);
}

int jme_jpl_orientation_state_split_naif(
    double jd0,
    double time_offset,
    int target_naif,
    int unit,
    double *state,
    char *error
)
{
#ifdef JME_HAVE_CALCEPH
    int calceph_unit;

    jme_zero_state(state);

    if (state == 0) {
        jme_set_error(error, "orientation state output is null");
        return JME_ERR;
    }

    if (g_kernel == 0) {
        jme_set_error(error, "JPL kernel is closed");
        return JME_ERR;
    }

    calceph_unit = jme_calceph_orientation_unit(unit);
    if (calceph_unit == 0) {
        jme_set_error(error, "JPL orientation unit is invalid");
        return JME_ERR;
    }

    if (!isfinite(jd0) || !isfinite(time_offset)) {
        jme_set_error(error, "JPL time argument is not finite");
        return JME_ERR;
    }
    jme_calceph_clear_message();
    if (calceph_orient_unit(g_kernel, jd0, time_offset, target_naif, calceph_unit, state) == 0) {
        jme_zero_state(state);
        jme_calceph_fail(error, "JPL orientation state computation failed");
        return JME_ERR;
    }

    return JME_OK;
#else
    (void)jd0;
    (void)time_offset;
    (void)target_naif;
    (void)unit;
    jme_zero_state(state);
    jme_set_error(error, "CALCEPH support is unavailable in this build");
    return JME_ERR;
#endif
}

int jme_jpl_orientation_state_order_naif(
    double jd0,
    double time_offset,
    int target_naif,
    int unit,
    int order,
    double *state,
    char *error
)
{
#ifdef JME_HAVE_CALCEPH
    int calceph_unit;

    if (!jme_order_is_valid(order)) {
        jme_set_error(error, "JPL derivative order is invalid");
        return JME_ERR;
    }

    jme_zero_order_state(state, order);

    if (state == 0) {
        jme_set_error(error, "orientation order state output is null");
        return JME_ERR;
    }

    if (g_kernel == 0) {
        jme_set_error(error, "JPL kernel is closed");
        return JME_ERR;
    }

    calceph_unit = jme_calceph_orientation_unit(unit);
    if (calceph_unit == 0) {
        jme_set_error(error, "JPL orientation unit is invalid");
        return JME_ERR;
    }

    if (!isfinite(jd0) || !isfinite(time_offset)) {
        jme_set_error(error, "JPL time argument is not finite");
        return JME_ERR;
    }
    jme_calceph_clear_message();
    if (calceph_orient_order(g_kernel, jd0, time_offset, target_naif, calceph_unit, order, state) ==
        0) {
        jme_zero_order_state(state, order);
        jme_calceph_fail(error, "JPL orientation order state computation failed");
        return JME_ERR;
    }

    return JME_OK;
#else
    (void)jd0;
    (void)time_offset;
    (void)target_naif;
    (void)unit;
    if (jme_order_is_valid(order)) {
        jme_zero_order_state(state, order);
    }
    jme_set_error(error, "CALCEPH support is unavailable in this build");
    return JME_ERR;
#endif
}

int jme_jpl_orientation_state_utc_naif(
    int year,
    int month,
    int day,
    int hour,
    int minute,
    double second,
    int calendar,
    int target_naif,
    int unit,
    double *state,
    char *error
)
{
    double jd0_native = 0.0;
    double fraction_native = 0.0;

    if (jme_utc_to_jpl_jd_split(
            year,
            month,
            day,
            hour,
            minute,
            second,
            calendar,
            &jd0_native,
            &fraction_native,
            error
        ) != JME_OK) {
        jme_zero_state(state);
        return JME_ERR;
    }

    return jme_jpl_orientation_state_split_naif(
        jd0_native,
        fraction_native,
        target_naif,
        unit,
        state,
        error
    );
}

int jme_jpl_rotational_angular_momentum_state_naif(
    double jd_time,
    int target_naif,
    int unit,
    double *state,
    char *error
)
{
    double jd0 = jme_jpl_day_floor(jd_time);
    double fraction = jd_time - jd0;

    return jme_jpl_rotational_angular_momentum_state_split_naif(
        jd0,
        fraction,
        target_naif,
        unit,
        state,
        error
    );
}

int jme_jpl_rotational_angular_momentum_state_utc_naif(
    int year,
    int month,
    int day,
    int hour,
    int minute,
    double second,
    int calendar,
    int target_naif,
    int unit,
    double *state,
    char *error
)
{
    double jd0_native = 0.0;
    double fraction_native = 0.0;

    if (jme_utc_to_jpl_jd_split(
            year,
            month,
            day,
            hour,
            minute,
            second,
            calendar,
            &jd0_native,
            &fraction_native,
            error
        ) != JME_OK) {
        jme_zero_state(state);
        return JME_ERR;
    }

    return jme_jpl_rotational_angular_momentum_state_split_naif(
        jd0_native,
        fraction_native,
        target_naif,
        unit,
        state,
        error
    );
}

int jme_jpl_rotational_angular_momentum_state_split_naif(
    double jd0,
    double time_offset,
    int target_naif,
    int unit,
    double *state,
    char *error
)
{
#ifdef JME_HAVE_CALCEPH
    int calceph_unit;

    jme_zero_state(state);

    if (state == 0) {
        jme_set_error(error, "rotational angular momentum state output is null");
        return JME_ERR;
    }

    if (g_kernel == 0) {
        jme_set_error(error, "JPL kernel is closed");
        return JME_ERR;
    }

    calceph_unit = jme_calceph_orientation_unit(unit);
    if (calceph_unit == 0) {
        jme_set_error(error, "JPL rotational angular momentum unit is invalid");
        return JME_ERR;
    }

    if (!isfinite(jd0) || !isfinite(time_offset)) {
        jme_set_error(error, "JPL time argument is not finite");
        return JME_ERR;
    }
    jme_calceph_clear_message();
    if (calceph_rotangmom_unit(g_kernel, jd0, time_offset, target_naif, calceph_unit, state) == 0) {
        jme_zero_state(state);
        jme_calceph_fail(error, "JPL rotational angular momentum state computation failed");
        return JME_ERR;
    }

    return JME_OK;
#else
    (void)jd0;
    (void)time_offset;
    (void)target_naif;
    (void)unit;
    jme_zero_state(state);
    jme_set_error(error, "CALCEPH support is unavailable in this build");
    return JME_ERR;
#endif
}

int jme_jpl_rotational_angular_momentum_state_order_naif(
    double jd0,
    double time_offset,
    int target_naif,
    int unit,
    int order,
    double *state,
    char *error
)
{
#ifdef JME_HAVE_CALCEPH
    int calceph_unit;

    if (!jme_order_is_valid(order)) {
        jme_set_error(error, "JPL derivative order is invalid");
        return JME_ERR;
    }

    jme_zero_order_state(state, order);

    if (state == 0) {
        jme_set_error(error, "rotational angular momentum order state output is null");
        return JME_ERR;
    }

    if (g_kernel == 0) {
        jme_set_error(error, "JPL kernel is closed");
        return JME_ERR;
    }

    calceph_unit = jme_calceph_orientation_unit(unit);
    if (calceph_unit == 0) {
        jme_set_error(error, "JPL rotational angular momentum unit is invalid");
        return JME_ERR;
    }

    if (!isfinite(jd0) || !isfinite(time_offset)) {
        jme_set_error(error, "JPL time argument is not finite");
        return JME_ERR;
    }
    jme_calceph_clear_message();
    if (calceph_rotangmom_order(
            g_kernel,
            jd0,
            time_offset,
            target_naif,
            calceph_unit,
            order,
            state
        ) == 0) {
        jme_zero_order_state(state, order);
        jme_calceph_fail(error, "JPL rotational angular momentum order state computation failed");
        return JME_ERR;
    }

    return JME_OK;
#else
    (void)jd0;
    (void)time_offset;
    (void)target_naif;
    (void)unit;
    if (jme_order_is_valid(order)) {
        jme_zero_order_state(state, order);
    }
    jme_set_error(error, "CALCEPH support is unavailable in this build");
    return JME_ERR;
#endif
}

int jme_jpl_body_state_naif(
    double jd_time,
    int target_naif,
    int center_naif,
    int unit,
    double *state,
    char *error
)
{
    double jd0 = jme_jpl_day_floor(jd_time);
    double fraction = jd_time - jd0;

    return jme_jpl_body_state_split_naif(
        jd0,
        fraction,
        target_naif,
        center_naif,
        unit,
        state,
        error
    );
}

int jme_jpl_body_state_split_naif(
    double jd0,
    double time_offset,
    int target_naif,
    int center_naif,
    int unit,
    double *state,
    char *error
)
{
#ifdef JME_HAVE_CALCEPH
    double raw_state[6];
    int calceph_unit;
    int raw_unit;

    jme_zero_state(state);

    if (state == 0) {
        jme_set_error(error, "state output is null");
        return JME_ERR;
    }

    if (g_kernel == 0) {
        jme_set_error(error, "JPL kernel is closed");
        return JME_ERR;
    }

    calceph_unit = jme_calceph_unit(unit);
    if (calceph_unit == 0) {
        jme_set_error(error, "JPL vector unit is invalid");
        return JME_ERR;
    }

    raw_unit = unit == JME_VECTOR_AU_PER_DAY      ? JME_VECTOR_KM_PER_DAY
               : unit == JME_VECTOR_AU_PER_SECOND ? JME_VECTOR_KM_PER_SECOND
                                                  : unit;

    if (!isfinite(jd0) || !isfinite(time_offset)) {
        jme_set_error(error, "JPL time argument is not finite");
        return JME_ERR;
    }
    jme_calceph_clear_message();
    if (calceph_compute_unit(
            g_kernel,
            jd0,
            time_offset,
            target_naif,
            center_naif,
            calceph_unit,
            raw_state
        ) == 0) {
        jme_zero_state(state);
        jme_calceph_fail(error, "JPL body state computation failed");
        return JME_ERR;
    }

    if (jme_state_convert_units(raw_state, raw_unit, unit, state) != JME_OK) {
        jme_zero_state(state);
        jme_set_error(error, "JPL body state unit conversion failed");
        return JME_ERR;
    }

    return JME_OK;
#else
    (void)jd0;
    (void)time_offset;
    (void)target_naif;
    (void)center_naif;
    (void)unit;
    jme_zero_state(state);
    jme_set_error(error, "CALCEPH support is unavailable in this build");
    return JME_ERR;
#endif
}

int jme_jpl_body_state_native_naif(
    double jd_time,
    int target_naif,
    int center_naif,
    double *state,
    char *error
)
{
    double jd0 = jme_jpl_day_floor(jd_time);
    double fraction = jd_time - jd0;

    return jme_jpl_body_state_native_split_naif(
        jd0,
        fraction,
        target_naif,
        center_naif,
        state,
        error
    );
}

int jme_jpl_body_state_native(
    double jd_time,
    int target_body,
    int center_body,
    double *state,
    char *error
)
{
    int target_naif = jme_body_naif_id(target_body);
    int center_naif = jme_body_naif_id(center_body);

    if (target_naif < 0 || center_naif < 0) {
        jme_zero_state(state);
        jme_set_error(error, "JPL body identifier is invalid");
        return JME_ERR;
    }

    return jme_jpl_body_state_native_naif(jd_time, target_naif, center_naif, state, error);
}

int jme_jpl_body_state_native_split_naif(
    double jd0,
    double time_offset,
    int target_naif,
    int center_naif,
    double *state,
    char *error
)
{
    return jme_jpl_body_state_split_naif(
        jd0,
        time_offset,
        target_naif,
        center_naif,
        JME_VECTOR_AU_PER_DAY,
        state,
        error
    );
}

int jme_jpl_body_state_native_split(
    double jd0,
    double time_offset,
    int target_body,
    int center_body,
    double *state,
    char *error
)
{
    int target_naif = jme_body_naif_id(target_body);
    int center_naif = jme_body_naif_id(center_body);

    if (target_naif < 0 || center_naif < 0) {
        jme_zero_state(state);
        jme_set_error(error, "JPL body identifier is invalid");
        return JME_ERR;
    }

    return jme_jpl_body_state_native_split_naif(
        jd0,
        time_offset,
        target_naif,
        center_naif,
        state,
        error
    );
}

int jme_jpl_body_state_order_naif(
    double jd0,
    double time_offset,
    int target_naif,
    int center_naif,
    int unit,
    int order,
    double *state,
    char *error
)
{
#ifdef JME_HAVE_CALCEPH
    int calceph_unit;

    if (!jme_order_is_valid(order)) {
        jme_set_error(error, "JPL derivative order is invalid");
        return JME_ERR;
    }

    jme_zero_order_state(state, order);

    if (state == 0) {
        jme_set_error(error, "body order state output is null");
        return JME_ERR;
    }

    if (g_kernel == 0) {
        jme_set_error(error, "JPL kernel is closed");
        return JME_ERR;
    }

    calceph_unit = jme_calceph_unit(unit);
    if (calceph_unit == 0) {
        jme_set_error(error, "JPL vector unit is invalid");
        return JME_ERR;
    }

    if (!isfinite(jd0) || !isfinite(time_offset)) {
        jme_set_error(error, "JPL time argument is not finite");
        return JME_ERR;
    }
    jme_calceph_clear_message();
    if (calceph_compute_order(
            g_kernel,
            jd0,
            time_offset,
            target_naif,
            center_naif,
            calceph_unit,
            order,
            state
        ) == 0) {
        jme_zero_order_state(state, order);
        jme_calceph_fail(error, "JPL body order state computation failed");
        return JME_ERR;
    }

    jme_convert_order_state_from_km(state, order, unit);
    return JME_OK;
#else
    (void)jd0;
    (void)time_offset;
    (void)target_naif;
    (void)center_naif;
    (void)unit;
    if (jme_order_is_valid(order)) {
        jme_zero_order_state(state, order);
    }
    jme_set_error(error, "CALCEPH support is unavailable in this build");
    return JME_ERR;
#endif
}

int jme_jpl_body_state_order(
    double jd0,
    double time_offset,
    int target_body,
    int center_body,
    int unit,
    int order,
    double *state,
    char *error
)
{
    int target_naif = jme_body_naif_id(target_body);
    int center_naif = jme_body_naif_id(center_body);

    if (target_naif < 0 || center_naif < 0) {
        if (jme_order_is_valid(order)) {
            jme_zero_order_state(state, order);
        }
        jme_set_error(error, "JPL body identifier is invalid");
        return JME_ERR;
    }

    return jme_jpl_body_state_order_naif(
        jd0,
        time_offset,
        target_naif,
        center_naif,
        unit,
        order,
        state,
        error
    );
}

int jme_jpl_body_state_utc_naif(
    int year,
    int month,
    int day,
    int hour,
    int minute,
    double second,
    int calendar,
    int target_naif,
    int center_naif,
    int unit,
    double *state,
    char *error
)
{
    double jd0_native = 0.0;
    double fraction_native = 0.0;

    if (jme_utc_to_jpl_jd_split(
            year,
            month,
            day,
            hour,
            minute,
            second,
            calendar,
            &jd0_native,
            &fraction_native,
            error
        ) != JME_OK) {
        jme_zero_state(state);
        return JME_ERR;
    }

    return jme_jpl_body_state_split_naif(
        jd0_native,
        fraction_native,
        target_naif,
        center_naif,
        unit,
        state,
        error
    );
}

int jme_jpl_body_state_utc(
    int year,
    int month,
    int day,
    int hour,
    int minute,
    double second,
    int calendar,
    int target_body,
    int center_body,
    int unit,
    double *state,
    char *error
)
{
    int target_naif = jme_body_naif_id(target_body);
    int center_naif = jme_body_naif_id(center_body);

    if (target_naif < 0 || center_naif < 0) {
        jme_zero_state(state);
        jme_set_error(error, "JPL body identifier is invalid");
        return JME_ERR;
    }

    return jme_jpl_body_state_utc_naif(
        year,
        month,
        day,
        hour,
        minute,
        second,
        calendar,
        target_naif,
        center_naif,
        unit,
        state,
        error
    );
}

int jme_jpl_body_state(
    double jd_time,
    int target_body,
    int center_body,
    int unit,
    double *state,
    char *error
)
{
    int target_naif = jme_body_naif_id(target_body);
    int center_naif = jme_body_naif_id(center_body);

    if (target_naif < 0 || center_naif < 0) {
        jme_zero_state(state);
        jme_set_error(error, "JPL body identifier is invalid");
        return JME_ERR;
    }

    return jme_jpl_body_state_naif(jd_time, target_naif, center_naif, unit, state, error);
}

int jme_jpl_body_state_split(
    double jd0,
    double time_offset,
    int target_body,
    int center_body,
    int unit,
    double *state,
    char *error
)
{
    int target_naif = jme_body_naif_id(target_body);
    int center_naif = jme_body_naif_id(center_body);

    if (target_naif < 0 || center_naif < 0) {
        jme_zero_state(state);
        jme_set_error(error, "JPL body identifier is invalid");
        return JME_ERR;
    }

    return jme_jpl_body_state_split_naif(
        jd0,
        time_offset,
        target_naif,
        center_naif,
        unit,
        state,
        error
    );
}

int jme_jpl_ecliptic_state_naif(
    double jd_time,
    int target_naif,
    int center_naif,
    int unit,
    double *state,
    char *error
)
{
    double jd0 = jme_jpl_day_floor(jd_time);
    double fraction = jd_time - jd0;

    return jme_jpl_ecliptic_state_split_naif(
        jd0,
        fraction,
        target_naif,
        center_naif,
        unit,
        state,
        error
    );
}

int jme_jpl_ecliptic_state_split_naif(
    double jd0,
    double time_offset,
    int target_naif,
    int center_naif,
    int unit,
    double *state,
    char *error
)
{
    double rectangular[6];
    double ecliptic_rectangular[6];

    jme_zero_state(state);

    if (state == 0) {
        jme_set_error(error, "state output is null");
        return JME_ERR;
    }

    if (jme_jpl_body_state_split_naif(
            jd0,
            time_offset,
            target_naif,
            center_naif,
            unit,
            rectangular,
            error
        ) != JME_OK) {
        return JME_ERR;
    }

    jme_equatorial_rectangular_to_j2000_ecliptic(rectangular, ecliptic_rectangular);
    if (jme_rectangular_to_spherical_state(ecliptic_rectangular, state) != JME_OK) {
        jme_zero_state(state);
        jme_set_error(error, "JPL spherical state conversion failed");
        return JME_ERR;
    }

    return JME_OK;
}

int jme_jpl_ecliptic_state_utc_naif(
    int year,
    int month,
    int day,
    int hour,
    int minute,
    double second,
    int calendar,
    int target_naif,
    int center_naif,
    int unit,
    double *state,
    char *error
)
{
    double jd0_native = 0.0;
    double fraction_native = 0.0;

    if (jme_utc_to_jpl_jd_split(
            year,
            month,
            day,
            hour,
            minute,
            second,
            calendar,
            &jd0_native,
            &fraction_native,
            error
        ) != JME_OK) {
        jme_zero_state(state);
        return JME_ERR;
    }

    return jme_jpl_ecliptic_state_split_naif(
        jd0_native,
        fraction_native,
        target_naif,
        center_naif,
        unit,
        state,
        error
    );
}

int jme_jpl_ecliptic_state_utc(
    int year,
    int month,
    int day,
    int hour,
    int minute,
    double second,
    int calendar,
    int target_body,
    int center_body,
    int unit,
    double *state,
    char *error
)
{
    int target_naif = jme_body_naif_id(target_body);
    int center_naif = jme_body_naif_id(center_body);

    if (target_naif < 0 || center_naif < 0) {
        jme_zero_state(state);
        jme_set_error(error, "JPL body identifier is invalid");
        return JME_ERR;
    }

    return jme_jpl_ecliptic_state_utc_naif(
        year,
        month,
        day,
        hour,
        minute,
        second,
        calendar,
        target_naif,
        center_naif,
        unit,
        state,
        error
    );
}

int jme_jpl_ecliptic_state(
    double jd_time,
    int target_body,
    int center_body,
    int unit,
    double *state,
    char *error
)
{
    int target_naif = jme_body_naif_id(target_body);
    int center_naif = jme_body_naif_id(center_body);

    if (target_naif < 0 || center_naif < 0) {
        jme_zero_state(state);
        jme_set_error(error, "JPL body identifier is invalid");
        return JME_ERR;
    }

    return jme_jpl_ecliptic_state_naif(jd_time, target_naif, center_naif, unit, state, error);
}

int jme_jpl_ecliptic_state_split(
    double jd0,
    double time_offset,
    int target_body,
    int center_body,
    int unit,
    double *state,
    char *error
)
{
    int target_naif = jme_body_naif_id(target_body);
    int center_naif = jme_body_naif_id(center_body);

    if (target_naif < 0 || center_naif < 0) {
        jme_zero_state(state);
        jme_set_error(error, "JPL body identifier is invalid");
        return JME_ERR;
    }

    return jme_jpl_ecliptic_state_split_naif(
        jd0,
        time_offset,
        target_naif,
        center_naif,
        unit,
        state,
        error
    );
}
