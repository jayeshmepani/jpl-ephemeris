#include "jme/jme.h"

#include "context.h"

#include <stddef.h>
#include <string.h>
#include <math.h>

#define JME_J2000_MEAN_OBLIQUITY_DEG 23.439291111111111111111111111111111111111111111111
#define JME_DEG_TO_RAD 0.017453292519943295769236907684886127134428718885417

#ifdef JME_HAVE_CALCEPH
#include <calceph.h>
static t_calcephbin *g_kernel = 0;
static char g_kernel_path[JME_PATH_MAX] = "";
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
    const char *resolved_path = path;
    jme_context *ctx = jme_get_context();

    if (resolved_path == 0 || resolved_path[0] == '\0') {
        resolved_path = ctx->jpl_file;
    }

    if (resolved_path == 0 || resolved_path[0] == '\0') {
        jme_set_error(error, "JPL kernel path is empty");
        return JME_ERR;
    }

    if (g_kernel != 0 && strcmp(g_kernel_path, resolved_path) == 0) {
        return JME_OK;
    }

    jme_jpl_close();
    calceph_seterrorhandler(1, 0);
    g_kernel = calceph_open(resolved_path);

    if (g_kernel == 0) {
        jme_set_error(error, "JPL kernel open failed");
        return JME_ERR;
    }

    jme_set_string(g_kernel_path, sizeof(g_kernel_path), resolved_path);
    jme_set_string(ctx->jpl_file, sizeof(ctx->jpl_file), resolved_path);

    return JME_OK;
#else
    (void)path;
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
    calceph_seterrorhandler(1, 0);
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
    if (first_time != 0) { *first_time = 0.0; }
    if (last_time != 0) { *last_time = 0.0; }
    if (continuous != 0) { *continuous = 0; }

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
    if (first_time != 0) { *first_time = 0.0; }
    if (last_time != 0) { *last_time = 0.0; }
    if (continuous != 0) { *continuous = 0; }
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

int jme_jpl_constant_index(int index, char *name, unsigned int name_size, double *value, char *error)
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

    if (name == 0 || values == 0 || single_value_size == 0U || value_count <= 0 || value_count > 16) {
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
    if (target != 0) { *target = 0; }
    if (center != 0) { *center = 0; }
    if (first_time != 0) { *first_time = 0.0; }
    if (last_time != 0) { *last_time = 0.0; }
    if (frame != 0) { *frame = 0; }
    if (segment_type != 0) { *segment_type = 0; }

    if (g_kernel == 0) {
        jme_set_error(error, "JPL kernel is closed");
        return JME_ERR;
    }

    if (
        target == 0 ||
        center == 0 ||
        first_time == 0 ||
        last_time == 0 ||
        frame == 0 ||
        segment_type == 0
    ) {
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
    if (target != 0) { *target = 0; }
    if (center != 0) { *center = 0; }
    if (first_time != 0) { *first_time = 0.0; }
    if (last_time != 0) { *last_time = 0.0; }
    if (frame != 0) { *frame = 0; }
    if (segment_type != 0) { *segment_type = 0; }
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
    if (target != 0) { *target = 0; }
    if (first_time != 0) { *first_time = 0.0; }
    if (last_time != 0) { *last_time = 0.0; }
    if (frame != 0) { *frame = 0; }
    if (segment_type != 0) { *segment_type = 0; }

    if (g_kernel == 0) {
        jme_set_error(error, "JPL kernel is closed");
        return JME_ERR;
    }

    if (target == 0 || first_time == 0 || last_time == 0 || frame == 0 || segment_type == 0) {
        jme_set_error(error, "JPL kernel orientation-record output is null");
        return JME_ERR;
    }

    if (calceph_getorientrecordindex2(g_kernel, index, target, first_time, last_time, frame, segment_type) == 0) {
        jme_set_error(error, "JPL kernel orientation-record query failed");
        return JME_ERR;
    }

    return JME_OK;
#else
    (void)index;
    if (target != 0) { *target = 0; }
    if (first_time != 0) { *first_time = 0.0; }
    if (last_time != 0) { *last_time = 0.0; }
    if (frame != 0) { *frame = 0; }
    if (segment_type != 0) { *segment_type = 0; }
    jme_set_error(error, "CALCEPH support is unavailable in this build");
    return JME_ERR;
#endif
}

static int jme_calceph_unit(int unit)
{
#ifdef JME_HAVE_CALCEPH
    switch (unit) {
    case JME_VECTOR_AU_PER_DAY: return CALCEPH_USE_NAIFID | CALCEPH_UNIT_KM | CALCEPH_UNIT_DAY;
    case JME_VECTOR_KM_PER_DAY: return CALCEPH_USE_NAIFID | CALCEPH_UNIT_KM | CALCEPH_UNIT_DAY;
    case JME_VECTOR_AU_PER_SECOND: return CALCEPH_USE_NAIFID | CALCEPH_UNIT_KM | CALCEPH_UNIT_SEC;
    case JME_VECTOR_KM_PER_SECOND: return CALCEPH_USE_NAIFID | CALCEPH_UNIT_KM | CALCEPH_UNIT_SEC;
    default: return 0;
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
    case JME_ORIENTATION_RAD_PER_DAY: return CALCEPH_USE_NAIFID | CALCEPH_UNIT_RAD | CALCEPH_UNIT_DAY;
    case JME_ORIENTATION_RAD_PER_SECOND: return CALCEPH_USE_NAIFID | CALCEPH_UNIT_RAD | CALCEPH_UNIT_SEC;
    default: return 0;
    }
#else
    (void)unit;
    return 0;
#endif
}

static int jme_order_is_valid(int order)
{
    return order >= 0 && order <= 3;
}

static int jme_utc_to_jpl_jd(
    int year,
    int month,
    int day,
    int hour,
    int minute,
    double second,
    int calendar,
    double *jd_time
)
{
    double jd_utc = 0.0;

    if (jd_time == 0) {
        return JME_ERR;
    }

    if (jme_utc_to_jd(year, month, day, hour, minute, second, calendar, &jd_utc) != JME_OK) {
        return JME_ERR;
    }

    *jd_time = jd_utc + jme_delta_t(jd_utc) / JME_SECONDS_PER_DAY;
    return JME_OK;
}

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

int jme_jpl_orientation_state_naif(
    double jd_time,
    int target_naif,
    int unit,
    double *state,
    char *error
)
{
    double jd0 = (double)((long)jd_time);
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

    if (calceph_orient_unit(g_kernel, jd0, time_offset, target_naif, calceph_unit, state) == 0) {
        jme_zero_state(state);
        jme_set_error(error, "JPL orientation state computation failed");
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

    if (calceph_orient_order(g_kernel, jd0, time_offset, target_naif, calceph_unit, order, state) == 0) {
        jme_zero_order_state(state, order);
        jme_set_error(error, "JPL orientation order state computation failed");
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
    double jd = 0.0;

    if (jme_utc_to_jpl_jd(year, month, day, hour, minute, second, calendar, &jd) != JME_OK) {
        jme_zero_state(state);
        jme_set_error(error, "UTC calendar date is invalid");
        return JME_ERR;
    }

    return jme_jpl_orientation_state_naif(jd, target_naif, unit, state, error);
}

int jme_jpl_rotational_angular_momentum_state_naif(
    double jd_time,
    int target_naif,
    int unit,
    double *state,
    char *error
)
{
    double jd0 = (double)((long)jd_time);
    double fraction = jd_time - jd0;

    return jme_jpl_rotational_angular_momentum_state_split_naif(jd0, fraction, target_naif, unit, state, error);
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
    double jd = 0.0;

    if (jme_utc_to_jpl_jd(year, month, day, hour, minute, second, calendar, &jd) != JME_OK) {
        jme_zero_state(state);
        jme_set_error(error, "UTC calendar date is invalid");
        return JME_ERR;
    }

    return jme_jpl_rotational_angular_momentum_state_naif(jd, target_naif, unit, state, error);
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

    if (calceph_rotangmom_unit(g_kernel, jd0, time_offset, target_naif, calceph_unit, state) == 0) {
        jme_zero_state(state);
        jme_set_error(error, "JPL rotational angular momentum state computation failed");
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

    if (calceph_rotangmom_order(g_kernel, jd0, time_offset, target_naif, calceph_unit, order, state) == 0) {
        jme_zero_order_state(state, order);
        jme_set_error(error, "JPL rotational angular momentum order state computation failed");
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
    double jd0 = (double)((long)jd_time);
    double fraction = jd_time - jd0;

    return jme_jpl_body_state_split_naif(jd0, fraction, target_naif, center_naif, unit, state, error);
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

    raw_unit = unit == JME_VECTOR_AU_PER_DAY ? JME_VECTOR_KM_PER_DAY :
        unit == JME_VECTOR_AU_PER_SECOND ? JME_VECTOR_KM_PER_SECOND :
        unit;

    if (calceph_compute_unit(g_kernel, jd0, time_offset, target_naif, center_naif, calceph_unit, raw_state) == 0) {
        jme_zero_state(state);
        jme_set_error(error, "JPL body state computation failed");
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
    double jd0 = (double)((long)jd_time);
    double fraction = jd_time - jd0;

    return jme_jpl_body_state_native_split_naif(jd0, fraction, target_naif, center_naif, state, error);
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

    return jme_jpl_body_state_native_split_naif(jd0, time_offset, target_naif, center_naif, state, error);
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

    if (calceph_compute_order(g_kernel, jd0, time_offset, target_naif, center_naif, calceph_unit, order, state) == 0) {
        jme_zero_order_state(state, order);
        jme_set_error(error, "JPL body order state computation failed");
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

    return jme_jpl_body_state_order_naif(jd0, time_offset, target_naif, center_naif, unit, order, state, error);
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
    double jd = 0.0;

    if (jme_utc_to_jpl_jd(year, month, day, hour, minute, second, calendar, &jd) != JME_OK) {
        jme_zero_state(state);
        jme_set_error(error, "UTC calendar date is invalid");
        return JME_ERR;
    }

    return jme_jpl_body_state_naif(jd, target_naif, center_naif, unit, state, error);
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

    return jme_jpl_body_state_split_naif(jd0, time_offset, target_naif, center_naif, unit, state, error);
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
    double jd0 = (double)((long)jd_time);
    double fraction = jd_time - jd0;

    return jme_jpl_ecliptic_state_split_naif(jd0, fraction, target_naif, center_naif, unit, state, error);
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

    if (jme_jpl_body_state_split_naif(jd0, time_offset, target_naif, center_naif, unit, rectangular, error) != JME_OK) {
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
    double jd = 0.0;

    if (jme_utc_to_jpl_jd(year, month, day, hour, minute, second, calendar, &jd) != JME_OK) {
        jme_zero_state(state);
        jme_set_error(error, "UTC calendar date is invalid");
        return JME_ERR;
    }

    return jme_jpl_ecliptic_state_naif(jd, target_naif, center_naif, unit, state, error);
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

    return jme_jpl_ecliptic_state_split_naif(jd0, time_offset, target_naif, center_naif, unit, state, error);
}
