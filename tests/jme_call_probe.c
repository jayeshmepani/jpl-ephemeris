#define _POSIX_C_SOURCE 200809L

#include "jme/jme.h"

#include <errno.h>
#include <float.h>
#include <inttypes.h>
#include <math.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#ifdef _WIN32
#include <windows.h>
#ifndef CLOCK_MONOTONIC
#define CLOCK_MONOTONIC 1
#endif
static int jme_probe_clock_gettime(int clock_id, struct timespec *ts)
{
    LARGE_INTEGER freq;
    LARGE_INTEGER counter;
    (void)clock_id;
    if (ts == NULL || !QueryPerformanceFrequency(&freq) || !QueryPerformanceCounter(&counter)) {
        return -1;
    }

    ts->tv_sec = (time_t)(counter.QuadPart / freq.QuadPart);
    ts->tv_nsec = (long)(((counter.QuadPart % freq.QuadPart) * 1000000000LL) / freq.QuadPart);
    return 0;
}
#define clock_gettime jme_probe_clock_gettime
#endif

#ifndef M_PI
#define M_PI 3.141592653589793238462643383279502884
#endif

#define ERRBUF_SIZE 256
#define BIGBUF_SIZE 512
#define SENTINEL_BYTE 0xA5
#if defined(DBL_DECIMAL_DIG)
#define JME_DECIMAL_DIG DBL_DECIMAL_DIG
#else
#define JME_DECIMAL_DIG 17
#endif

typedef struct fixture_def {
    const char *id;
    int year;
    int month;
    int day;
    int hour;
    int minute;
    double second;
    double lon;
    double lat;
    double altitude_m;
    const char *timezone_name;
} fixture_def;

static const fixture_def fixture_modern_bhuj = {
    "modern_bhuj_standard", 2026, 5, 22, 12, 0, 0.0, 69.666931, 23.241999, 110.0, "Asia/Kolkata"
};
static const fixture_def fixture_j2000_bhuj = {
    "j2000_bhuj", 2000, 1, 1, 12, 0, 0.0, 69.666931, 23.241999, 110.0, "Asia/Kolkata"
};
static const fixture_def fixture_ancient_bhuj = {
    "ancient_bhuj", 1000, 1, 1, 0, 0, 0.0, 69.666931, 23.241999, 110.0, "Asia/Kolkata"
};
static const fixture_def fixture_future_bhuj = {
    "future_bhuj", 2050, 1, 1, 0, 0, 0.0, 69.666931, 23.241999, 110.0, "Asia/Kolkata"
};
static const fixture_def fixture_tromso = {
    "tromso_high_latitude", 2026, 6, 21, 12, 0, 0.0, 18.9553, 69.6492, 0.0, "UTC"
};

static struct timespec g_probe_start_ts;
static int g_probe_timer_active = 0;

static void json_escape(FILE *out, const unsigned char *s, size_t n)
{
    size_t i;
    fputc('"', out);
    for (i = 0; i < n; i++) {
        unsigned char c = s[i];
        switch (c) {
        case '\\': fputs("\\\\", out); break;
        case '"': fputs("\\\"", out); break;
        case '\b': fputs("\\b", out); break;
        case '\f': fputs("\\f", out); break;
        case '\n': fputs("\\n", out); break;
        case '\r': fputs("\\r", out); break;
        case '\t': fputs("\\t", out); break;
        default:
            if (c < 0x20 || c >= 0x80) {
                fprintf(out, "\\u%04x", (unsigned)c);
            } else {
                fputc((int)c, out);
            }
            break;
        }
    }
    fputc('"', out);
}

static void print_hex_bytes(FILE *out, const void *ptr, size_t size)
{
    const unsigned char *b = (const unsigned char *)ptr;
    size_t i;
    fputc('"', out);
    for (i = 0; i < size; i++) {
        fprintf(out, "%02x", b[i]);
    }
    fputc('"', out);
}

static void print_double_obj(FILE *out, const char *name, double value, int comma)
{
    fprintf(out, "\"%s\":{", name);
    fputs("\"type\":\"double\",", out);
    fprintf(out, "\"decimal_string\":\"%.*g\",", JME_DECIMAL_DIG, value);
    fprintf(out, "\"hexfloat\":\"%a\",", value);
    fputs("\"raw_le_hex\":", out);
    print_hex_bytes(out, &value, sizeof(value));
    fputc('}', out);
    if (comma) {
        fputc(',', out);
    }
}

static void print_int_obj(FILE *out, const char *name, int value, int comma)
{
    unsigned int u = (unsigned int)value;
    fprintf(out, "\"%s\":{", name);
    fputs("\"type\":\"int32\",", out);
    fprintf(out, "\"signed_decimal\":\"%d\",", value);
    fprintf(out, "\"unsigned_decimal\":\"%u\",", u);
    fprintf(out, "\"hex\":\"0x%08x\",", u);
    fputs("\"raw_le_hex\":", out);
    print_hex_bytes(out, &value, sizeof(value));
    fputc('}', out);
    if (comma) {
        fputc(',', out);
    }
}

static void print_size_obj(FILE *out, const char *name, size_t value, int comma)
{
    fprintf(out, "\"%s\":{\"type\":\"size_t\",\"decimal_string\":\"%zu\",\"raw_le_hex\":", name, value);
    print_hex_bytes(out, &value, sizeof(value));
    fputc('}', out);
    if (comma) {
        fputc(',', out);
    }
}

static void print_uint64_obj(FILE *out, const char *name, uint64_t value, int comma)
{
    fprintf(out, "\"%s\":{", name);
    fputs("\"type\":\"uint64\",", out);
    fprintf(out, "\"unsigned_decimal\":\"%" PRIu64 "\",", value);
    fprintf(out, "\"hex\":\"0x%016" PRIx64 "\",", value);
    fputs("\"raw_le_hex\":", out);
    print_hex_bytes(out, &value, sizeof(value));
    fputc('}', out);
    if (comma) {
        fputc(',', out);
    }
}

static void print_timespec_obj(FILE *out, const char *name, const struct timespec *ts, int comma)
{
    fprintf(out, "\"%s\":{", name);
    print_uint64_obj(out, "sec", (uint64_t)ts->tv_sec, 1);
    print_uint64_obj(out, "nsec", (uint64_t)ts->tv_nsec, 0);
    fputc('}', out);
    if (comma) {
        fputc(',', out);
    }
}

static void print_string_bytes_obj(FILE *out, const char *name, const char *s, size_t cap, int comma)
{
    size_t visible_len = 0;
    int nul_found = 0;
    if (s != 0) {
        for (visible_len = 0; visible_len < cap; visible_len++) {
            if (((const unsigned char *)s)[visible_len] == 0) {
                nul_found = 1;
                break;
            }
        }
    }
    fprintf(out, "\"%s\":{", name);
    fputs("\"type\":\"buffer\",", out);
    print_size_obj(out, "capacity", cap, 1);
    fprintf(out, "\"null_terminator_within_capacity\":%s,", nul_found ? "true" : "false");
    fputs("\"raw_le_hex\":", out);
    if (s != 0) {
        print_hex_bytes(out, s, cap);
    } else {
        json_escape(out, (const unsigned char *)"", 0);
    }
    fputs(",\"text\":", out);
    if (s != 0) {
        json_escape(out, (const unsigned char *)s, nul_found ? visible_len : cap);
    } else {
        fputs("null", out);
    }
    fputc('}', out);
    if (comma) {
        fputc(',', out);
    }
}

static void print_double_array_obj(FILE *out, const char *name, const double *values, size_t count, int comma)
{
    size_t i;
    fprintf(out, "\"%s\":{", name);
    fputs("\"type\":\"double_array\",", out);
    print_size_obj(out, "length", count, 1);
    fputs("\"elements\":[", out);
    for (i = 0; i < count; i++) {
        fputs("{", out);
        fprintf(out, "\"index\":\"%zu\",", i);
        fprintf(out, "\"decimal_string\":\"%.*g\",", JME_DECIMAL_DIG, values[i]);
        fprintf(out, "\"hexfloat\":\"%a\",", values[i]);
        fputs("\"raw_le_hex\":", out);
        print_hex_bytes(out, &values[i], sizeof(values[i]));
        fputc('}', out);
        if (i + 1 != count) {
            fputc(',', out);
        }
    }
    fputs("],\"raw_le_hex\":", out);
    print_hex_bytes(out, values, count * sizeof(double));
    fputc('}', out);
    if (comma) {
        fputc(',', out);
    }
}

static double fixture_jd_ut(const fixture_def *fixture)
{
    return jme_julian_day(fixture->year, fixture->month, fixture->day, fixture->hour + fixture->minute / 60.0 + fixture->second / 3600.0, JME_CALENDAR_GREGORIAN);
}

static void reset_global_state(const char *engine, const char *kernel_path)
{
    jme_close();
    jme_set_ephemeris_path(".");
    jme_set_jpl_file(kernel_path != 0 ? kernel_path : "");
    jme_set_sidereal_mode(JME_SIDEREAL_LAHIRI, 0.0, 0.0);
    jme_set_topo(fixture_modern_bhuj.lon, fixture_modern_bhuj.lat, fixture_modern_bhuj.altitude_m);
    jme_set_lapse_rate(0.0065);
    jme_set_interpolate_nut(0);
    jme_set_tid_acc(-25.8);
    jme_set_delta_t_userdef(NAN);
    if (engine != 0) {
        char model_buf[64];
        snprintf(model_buf, sizeof(model_buf), "ENGINE=%s", engine);
        jme_set_astro_models(model_buf, 0);
    } else {
        jme_set_astro_models(0, 0);
    }
}

static void begin_record(FILE *out, const char *function_name, const char *engine, const char *fixture_id, const char *category)
{
    fputs("{", out);
    fprintf(out, "\"function\":");
    json_escape(out, (const unsigned char *)function_name, strlen(function_name));
    fprintf(out, ",\"engine\":");
    json_escape(out, (const unsigned char *)engine, strlen(engine));
    fprintf(out, ",\"fixture_id\":");
    json_escape(out, (const unsigned char *)fixture_id, strlen(fixture_id));
    fprintf(out, ",\"category\":");
    json_escape(out, (const unsigned char *)category, strlen(category));
    fputc(',', out);
}

static void end_record(FILE *out)
{
    if (g_probe_timer_active) {
        struct timespec end_ts;
        time_t sec_delta;
        long nsec_delta;
        uint64_t elapsed_ns;
        clock_gettime(CLOCK_MONOTONIC, &end_ts);
        sec_delta = end_ts.tv_sec - g_probe_start_ts.tv_sec;
        nsec_delta = end_ts.tv_nsec - g_probe_start_ts.tv_nsec;
        if (nsec_delta < 0) {
            sec_delta -= 1;
            nsec_delta += 1000000000L;
        }
        elapsed_ns = (uint64_t)sec_delta * 1000000000ULL + (uint64_t)nsec_delta;
        fputc(',', out);
        fputs("\"timing\":{", out);
        fputs("\"clock\":\"CLOCK_MONOTONIC\",", out);
        print_timespec_obj(out, "start", &g_probe_start_ts, 1);
        print_timespec_obj(out, "end", &end_ts, 1);
        print_uint64_obj(out, "elapsed_ns", elapsed_ns, 1);
        fprintf(out, "\"elapsed_seconds_decimal_string\":\"%" PRIu64 ".%09ld\"",
            (uint64_t)(elapsed_ns / 1000000000ULL), (long)(elapsed_ns % 1000000000ULL));
        fputc('}', out);
        g_probe_timer_active = 0;
    } else {
        fputs("\"timing\":null", out);
    }
    fputs("}\n", out);
}

static void emit_skipped(FILE *out, const char *function_name, const char *engine, const char *fixture_id, const char *category, const char *reason)
{
    begin_record(out, function_name, engine, fixture_id, category);
    fputs("\"status\":\"SKIPPED_WITH_REASON\",", out);
    fputs("\"reason\":", out);
    json_escape(out, (const unsigned char *)reason, strlen(reason));
    end_record(out);
}

static void emit_simple_error(FILE *out, const char *function_name, const char *engine, const char *fixture_id, const char *category, int rc, int errno_value, const char *errbuf)
{
    begin_record(out, function_name, engine, fixture_id, category);
    fputs("\"status\":\"ERROR\",", out);
    print_int_obj(out, "return_code", rc, 1);
    print_int_obj(out, "errno_after", errno_value, 1);
    print_string_bytes_obj(out, "error_buffer", errbuf, ERRBUF_SIZE, 0);
    end_record(out);
}

static int engine_env_probe(const char *engine)
{
    char buf[BIGBUF_SIZE];
    jme_set_astro_models(0, 0);
    if (jme_get_astro_models(buf, 0) != JME_OK) {
        return 0;
    }
    return strstr(buf, engine) != 0;
}

static int emit_common_engine_state(FILE *out, int comma)
{
    char models[BIGBUF_SIZE];
    if (jme_get_astro_models(models, 0) != JME_OK) {
        memset(models, 0, sizeof(models));
    }
    print_string_bytes_obj(out, "engine_state", models, sizeof(models), comma);
    return 0;
}

static int do_probe(const char *function_name, const char *engine, const char *kernel_path)
{
    FILE *out = stdout;
    char errbuf[ERRBUF_SIZE];
    char buf1[BIGBUF_SIZE];
    char buf2[BIGBUF_SIZE];
    int rc = JME_ERR;
    int saved_errno = 0;
    const fixture_def *fx = &fixture_modern_bhuj;
    double jd = fixture_jd_ut(fx);
    double results[32];
    double results2[32];
    double geopos[3] = {fx->lon, fx->lat, fx->altitude_m};
    int i;

    memset(errbuf, SENTINEL_BYTE, sizeof(errbuf));
    memset(buf1, SENTINEL_BYTE, sizeof(buf1));
    memset(buf2, SENTINEL_BYTE, sizeof(buf2));
    for (i = 0; i < 32; i++) {
        results[i] = NAN;
        results2[i] = NAN;
    }

    reset_global_state(engine, kernel_path);
    errno = 0;
    clock_gettime(CLOCK_MONOTONIC, &g_probe_start_ts);
    g_probe_timer_active = 1;

    if (strcmp(function_name, "jme_version") == 0) {
        const char *p = jme_version(buf1, sizeof(buf1));
        begin_record(out, function_name, engine, fx->id, "engine_invariant");
        fputs("\"status\":\"OK\",", out);
        emit_common_engine_state(out, 1);
        fprintf(out, "\"pointer_non_null\":%s,", p != 0 ? "true" : "false");
        print_string_bytes_obj(out, "buffer", buf1, sizeof(buf1), 0);
        end_record(out);
        return 0;
    }
    if (strcmp(function_name, "jme_ephemeris_path") == 0) {
        const char *p = jme_ephemeris_path();
        begin_record(out, function_name, engine, fx->id, "engine_invariant");
        fputs("\"status\":\"OK\",", out);
        emit_common_engine_state(out, 1);
        fprintf(out, "\"pointer_non_null\":%s,", p != 0 ? "true" : "false");
        print_string_bytes_obj(out, "pointed_string", p != 0 ? p : "", p != 0 ? strlen(p) + 1 : 1, 0);
        end_record(out);
        return 0;
    }
    if (strcmp(function_name, "jme_jpl_file") == 0) {
        const char *p = jme_jpl_file();
        begin_record(out, function_name, engine, fx->id, "engine_invariant");
        fputs("\"status\":\"OK\",", out);
        emit_common_engine_state(out, 1);
        fprintf(out, "\"pointer_non_null\":%s,", p != 0 ? "true" : "false");
        print_string_bytes_obj(out, "pointed_string", p != 0 ? p : "", p != 0 ? strlen(p) + 1 : 1, 0);
        end_record(out);
        return 0;
    }
    if (strcmp(function_name, "jme_library_path") == 0) {
        const char *p = jme_library_path();
        begin_record(out, function_name, engine, fx->id, "engine_invariant");
        fputs("\"status\":\"OK\",", out);
        emit_common_engine_state(out, 1);
        fprintf(out, "\"pointer_non_null\":%s,", p != 0 ? "true" : "false");
        if (p != 0) {
            print_string_bytes_obj(out, "pointed_string", p, strlen(p) + 1, 0);
        } else {
            print_string_bytes_obj(out, "pointed_string", "", 1, 0);
        }
        end_record(out);
        return 0;
    }
    if (strcmp(function_name, "jme_body_name") == 0) {
        const char *p = jme_body_name(JME_BODY_MARS);
        begin_record(out, function_name, engine, fx->id, "engine_invariant");
        fputs("\"status\":\"OK\",", out);
        emit_common_engine_state(out, 1);
        fprintf(out, "\"pointer_non_null\":%s,", p != 0 ? "true" : "false");
        print_string_bytes_obj(out, "pointed_string", p != 0 ? p : "", p != 0 ? strlen(p) + 1 : 1, 0);
        end_record(out);
        return 0;
    }
    if (strcmp(function_name, "jme_copy_body_name") == 0) {
        char *p = jme_copy_body_name(JME_BODY_MARS, buf1);
        begin_record(out, function_name, engine, fx->id, "engine_invariant");
        fputs("\"status\":\"OK\",", out);
        emit_common_engine_state(out, 1);
        fprintf(out, "\"pointer_non_null\":%s,", p != 0 ? "true" : "false");
        print_string_bytes_obj(out, "buffer", buf1, sizeof(buf1), 0);
        end_record(out);
        return 0;
    }
    if (strcmp(function_name, "jme_house_system_name") == 0) {
        const char *p = jme_house_system_name(JME_HOUSE_PLACIDUS);
        begin_record(out, function_name, engine, fx->id, "engine_invariant");
        fputs("\"status\":\"OK\",", out);
        emit_common_engine_state(out, 1);
        fprintf(out, "\"pointer_non_null\":%s,", p != 0 ? "true" : "false");
        print_string_bytes_obj(out, "pointed_string", p != 0 ? p : "", p != 0 ? strlen(p) + 1 : 1, 0);
        end_record(out);
        return 0;
    }
    if (strcmp(function_name, "jme_get_ayanamsa_name") == 0) {
        const char *p = jme_get_ayanamsa_name(JME_SIDEREAL_LAHIRI);
        begin_record(out, function_name, engine, fx->id, "engine_invariant");
        fputs("\"status\":\"OK\",", out);
        emit_common_engine_state(out, 1);
        fprintf(out, "\"pointer_non_null\":%s,", p != 0 ? "true" : "false");
        print_string_bytes_obj(out, "pointed_string", p != 0 ? p : "", p != 0 ? strlen(p) + 1 : 1, 0);
        end_record(out);
        return 0;
    }
    if (strcmp(function_name, "jme_set_ephemeris_path") == 0) {
        jme_set_ephemeris_path("matrix ephemeris path");
        begin_record(out, function_name, engine, fx->id, "engine_invariant");
        fputs("\"status\":\"OK\",", out);
        emit_common_engine_state(out, 1);
        print_string_bytes_obj(out, "post_path", jme_ephemeris_path(), strlen(jme_ephemeris_path()) + 1, 0);
        end_record(out);
        return 0;
    }
    if (strcmp(function_name, "jme_set_jpl_file") == 0) {
        jme_set_jpl_file("matrix-kernel.bsp");
        begin_record(out, function_name, engine, fx->id, "engine_invariant");
        fputs("\"status\":\"OK\",", out);
        emit_common_engine_state(out, 1);
        print_string_bytes_obj(out, "post_jpl_file", jme_jpl_file(), strlen(jme_jpl_file()) + 1, 0);
        end_record(out);
        return 0;
    }
    if (strcmp(function_name, "jme_set_sidereal_mode") == 0) {
        int mode = -1;
        double t0 = NAN;
        double ayan0 = NAN;
        jme_set_sidereal_mode(JME_SIDEREAL_USER, 2451545.0, 12.5);
        jme_get_sidereal_mode(&mode, &t0, &ayan0);
        begin_record(out, function_name, engine, fx->id, "engine_invariant");
        fputs("\"status\":\"OK\",", out);
        emit_common_engine_state(out, 1);
        print_int_obj(out, "mode", mode, 1);
        print_double_obj(out, "t0", t0, 1);
        print_double_obj(out, "ayan_t0", ayan0, 0);
        end_record(out);
        return 0;
    }
    if (strcmp(function_name, "jme_get_sidereal_mode") == 0) {
        int mode = -1;
        double t0 = NAN;
        double ayan0 = NAN;
        jme_get_sidereal_mode(&mode, &t0, &ayan0);
        begin_record(out, function_name, engine, fx->id, "engine_invariant");
        fputs("\"status\":\"OK\",", out);
        emit_common_engine_state(out, 1);
        print_int_obj(out, "mode", mode, 1);
        print_double_obj(out, "t0", t0, 1);
        print_double_obj(out, "ayan_t0", ayan0, 0);
        end_record(out);
        return 0;
    }
    if (strcmp(function_name, "jme_set_topo") == 0) {
        double topo[3];
        jme_set_topo(fx->lon, fx->lat, fx->altitude_m);
        rc = jme_get_topo_pos(jd, topo, errbuf);
        saved_errno = errno;
        begin_record(out, function_name, engine, fx->id, "engine_invariant");
        fputs("\"status\":\"OK\",", out);
        emit_common_engine_state(out, 1);
        print_int_obj(out, "observer_probe_rc", rc, 1);
        print_int_obj(out, "errno_after", saved_errno, 1);
        print_string_bytes_obj(out, "error_buffer", errbuf, sizeof(errbuf), 1);
        print_double_array_obj(out, "observer_probe", topo, 3, 0);
        end_record(out);
        return 0;
    }
    if (strcmp(function_name, "jme_close") == 0) {
        jme_close();
        begin_record(out, function_name, engine, fx->id, "engine_invariant");
        fputs("\"status\":\"OK\",", out);
        emit_common_engine_state(out, 1);
        print_int_obj(out, "jpl_is_open_after_close", jme_jpl_is_open(), 0);
        end_record(out);
        return 0;
    }
    if (strcmp(function_name, "jme_get_astro_models") == 0) {
        rc = jme_get_astro_models(buf1, 0);
        saved_errno = errno;
        begin_record(out, function_name, engine, fx->id, "engine_invariant");
        fputs("\"status\":\"OK\",", out);
        print_int_obj(out, "return_code", rc, 1);
        print_int_obj(out, "errno_after", saved_errno, 1);
        print_string_bytes_obj(out, "buffer", buf1, sizeof(buf1), 0);
        end_record(out);
        return 0;
    }
    if (strcmp(function_name, "jme_set_astro_models") == 0) {
        snprintf(buf1, sizeof(buf1), "ENGINE=%s", engine);
        jme_set_astro_models(buf1, 0);
        jme_get_astro_models(buf2, 0);
        begin_record(out, function_name, engine, fx->id, "engine_invariant");
        fputs("\"status\":\"OK\",", out);
        print_string_bytes_obj(out, "post_models", buf2, sizeof(buf2), 0);
        end_record(out);
        return 0;
    }
    if (strcmp(function_name, "jme_set_lapse_rate") == 0) {
        jme_set_lapse_rate(0.0042);
        begin_record(out, function_name, engine, fx->id, "engine_invariant");
        fputs("\"status\":\"OK\",", out);
        print_double_obj(out, "set_value", 0.0042, 0);
        end_record(out);
        return 0;
    }
    if (strcmp(function_name, "jme_set_interpolate_nut") == 0) {
        jme_set_interpolate_nut(1);
        begin_record(out, function_name, engine, fx->id, "engine_invariant");
        fputs("\"status\":\"OK\",", out);
        print_int_obj(out, "set_value", 1, 0);
        end_record(out);
        return 0;
    }
    if (strcmp(function_name, "jme_get_tid_acc") == 0) {
        begin_record(out, function_name, engine, fx->id, "engine_invariant");
        fputs("\"status\":\"OK\",", out);
        print_double_obj(out, "return_value", jme_get_tid_acc(), 0);
        end_record(out);
        return 0;
    }
    if (strcmp(function_name, "jme_set_tid_acc") == 0) {
        jme_set_tid_acc(-26.1);
        begin_record(out, function_name, engine, fx->id, "engine_invariant");
        fputs("\"status\":\"OK\",", out);
        print_double_obj(out, "post_value", jme_get_tid_acc(), 0);
        end_record(out);
        return 0;
    }
    if (strcmp(function_name, "jme_set_delta_t_userdef") == 0) {
        jme_set_delta_t_userdef(69.5);
        begin_record(out, function_name, engine, fx->id, "engine_invariant");
        fputs("\"status\":\"OK\",", out);
        print_double_obj(out, "probe_delta_t", jme_delta_t(jd), 0);
        end_record(out);
        return 0;
    }
    if (strcmp(function_name, "jme_body_id_from_name") == 0) {
        begin_record(out, function_name, engine, fx->id, "engine_invariant");
        fputs("\"status\":\"OK\",", out);
        print_int_obj(out, "mars", jme_body_id_from_name("Mars"), 1);
        print_int_obj(out, "sun", jme_body_id_from_name("Sun"), 1);
        print_int_obj(out, "invalid", jme_body_id_from_name("mars"), 0);
        end_record(out);
        return 0;
    }
    if (strcmp(function_name, "jme_body_naif_id") == 0) {
        begin_record(out, function_name, engine, fx->id, "engine_invariant");
        fputs("\"status\":\"OK\",", out);
        print_int_obj(out, "mars", jme_body_naif_id(JME_BODY_MARS), 1);
        print_int_obj(out, "earth", jme_body_naif_id(JME_BODY_EARTH), 1);
        print_int_obj(out, "invalid", jme_body_naif_id(999999), 0);
        end_record(out);
        return 0;
    }
    if (strcmp(function_name, "jme_calendar_is_leap_year") == 0) {
        begin_record(out, function_name, engine, fx->id, "engine_invariant");
        fputs("\"status\":\"OK\",", out);
        print_int_obj(out, "gregorian_2000", jme_calendar_is_leap_year(2000, JME_CALENDAR_GREGORIAN), 1);
        print_int_obj(out, "gregorian_1900", jme_calendar_is_leap_year(1900, JME_CALENDAR_GREGORIAN), 1);
        print_int_obj(out, "julian_1900", jme_calendar_is_leap_year(1900, JME_CALENDAR_JULIAN), 0);
        end_record(out);
        return 0;
    }
    if (strcmp(function_name, "jme_days_in_month") == 0 || strcmp(function_name, "jme_day_of_year") == 0
        || strcmp(function_name, "jme_date_is_valid") == 0 || strcmp(function_name, "jme_day_of_week") == 0) {
        begin_record(out, function_name, engine, fx->id, "engine_invariant");
        fputs("\"status\":\"OK\",", out);
        if (strcmp(function_name, "jme_days_in_month") == 0) {
            print_int_obj(out, "value", jme_days_in_month(2026, 5, JME_CALENDAR_GREGORIAN), 0);
        } else if (strcmp(function_name, "jme_day_of_year") == 0) {
            print_int_obj(out, "value", jme_day_of_year(2026, 5, 22, JME_CALENDAR_GREGORIAN), 0);
        } else if (strcmp(function_name, "jme_date_is_valid") == 0) {
            print_int_obj(out, "value", jme_date_is_valid(2026, 5, 22, JME_CALENDAR_GREGORIAN), 0);
        } else {
            print_int_obj(out, "value", jme_day_of_week(jd), 0);
        }
        end_record(out);
        return 0;
    }
    if (strcmp(function_name, "jme_julian_day") == 0 || strcmp(function_name, "jme_decimal_hour") == 0
        || strcmp(function_name, "jme_delta_t") == 0 || strcmp(function_name, "jme_get_ayanamsa") == 0
        || strcmp(function_name, "jme_get_ayanamsa_ut") == 0 || strcmp(function_name, "jme_sidereal_time") == 0
        || strcmp(function_name, "jme_hours_normalize") == 0 || strcmp(function_name, "jme_hours_to_degrees") == 0
        || strcmp(function_name, "jme_degree_normalize") == 0 || strcmp(function_name, "jme_degrees_to_hours") == 0
        || strcmp(function_name, "jme_degrees_to_radians") == 0 || strcmp(function_name, "jme_radians_to_degrees") == 0
        || strcmp(function_name, "jme_degrees_difference") == 0 || strcmp(function_name, "jme_degrees_difference_signed") == 0
        || strcmp(function_name, "jme_radian_normalize") == 0 || strcmp(function_name, "jme_radian_midpoint") == 0
        || strcmp(function_name, "jme_degree_midpoint") == 0 || strcmp(function_name, "jme_radians_difference_signed") == 0
        || strcmp(function_name, "jme_double_to_long") == 0 || strcmp(function_name, "jme_heliacal_angle") == 0
        || strcmp(function_name, "jme_topo_arcus_visionis") == 0 || strcmp(function_name, "jme_refract") == 0
        || strcmp(function_name, "jme_get_tid_acc") == 0 || strcmp(function_name, "jme_state_distance") == 0
        || strcmp(function_name, "jme_state_speed") == 0 || strcmp(function_name, "jme_state_light_time_days") == 0
        || strcmp(function_name, "jme_state_position_velocity_dot") == 0 || strcmp(function_name, "jme_house_pos") == 0
        || strcmp(function_name, "jme_delta_t_ex") == 0 || strcmp(function_name, "jme_jd_add_seconds") == 0
        || strcmp(function_name, "jme_jd_difference_seconds") == 0 || strcmp(function_name, "jme_sidereal_time0") == 0) {
        double value = NAN;
        double state[6] = {1.0, 2.0, 2.0, 0.1, -0.2, 0.2};
        double dat_hel[50] = {0.0};
        dat_hel[0] = (double)JME_BODY_VENUS;
        if (strcmp(function_name, "jme_julian_day") == 0) value = jd;
        else if (strcmp(function_name, "jme_decimal_hour") == 0) value = jme_decimal_hour(fx->hour, fx->minute, fx->second);
        else if (strcmp(function_name, "jme_delta_t") == 0) value = jme_delta_t(jd);
        else if (strcmp(function_name, "jme_delta_t_ex") == 0) value = jme_delta_t_ex(jd, JME_MODEL_DELTAT_ESPENAK_MEEUS_2006, errbuf);
        else if (strcmp(function_name, "jme_get_ayanamsa") == 0) value = jme_get_ayanamsa(jd);
        else if (strcmp(function_name, "jme_get_ayanamsa_ut") == 0) value = jme_get_ayanamsa_ut(jd);
        else if (strcmp(function_name, "jme_sidereal_time") == 0) value = jme_sidereal_time(jd);
        else if (strcmp(function_name, "jme_hours_normalize") == 0) value = jme_hours_normalize(-1.25);
        else if (strcmp(function_name, "jme_hours_to_degrees") == 0) value = jme_hours_to_degrees(1.5);
        else if (strcmp(function_name, "jme_degree_normalize") == 0) value = jme_degree_normalize(-10.0);
        else if (strcmp(function_name, "jme_degrees_to_hours") == 0) value = jme_degrees_to_hours(15.0);
        else if (strcmp(function_name, "jme_degrees_to_radians") == 0) value = jme_degrees_to_radians(180.0);
        else if (strcmp(function_name, "jme_radians_to_degrees") == 0) value = jme_radians_to_degrees(M_PI);
        else if (strcmp(function_name, "jme_degrees_difference") == 0) value = jme_degrees_difference(370.0, 10.0);
        else if (strcmp(function_name, "jme_degrees_difference_signed") == 0) value = jme_degrees_difference_signed(10.0, 350.0);
        else if (strcmp(function_name, "jme_radian_normalize") == 0) value = jme_radian_normalize(7.0);
        else if (strcmp(function_name, "jme_radian_midpoint") == 0) value = jme_radian_midpoint(0.1, 6.2);
        else if (strcmp(function_name, "jme_degree_midpoint") == 0) value = jme_degree_midpoint(10.0, 350.0);
        else if (strcmp(function_name, "jme_radians_difference_signed") == 0) value = jme_radians_difference_signed(0.1, 6.2);
        else if (strcmp(function_name, "jme_double_to_long") == 0) value = (double)jme_double_to_long(123.45);
        else if (strcmp(function_name, "jme_heliacal_angle") == 0) value = jme_heliacal_angle(jd, geopos, dat_hel, errbuf);
        else if (strcmp(function_name, "jme_topo_arcus_visionis") == 0) value = jme_topo_arcus_visionis(jd, geopos, dat_hel, errbuf);
        else if (strcmp(function_name, "jme_refract") == 0) value = jme_refract(10.0, 1013.25, 15.0, 0);
        else if (strcmp(function_name, "jme_state_distance") == 0) value = jme_state_distance(state);
        else if (strcmp(function_name, "jme_state_speed") == 0) value = jme_state_speed(state);
        else if (strcmp(function_name, "jme_state_light_time_days") == 0) value = jme_state_light_time_days(state, JME_VECTOR_AU_PER_DAY);
        else if (strcmp(function_name, "jme_state_position_velocity_dot") == 0) value = jme_state_position_velocity_dot(state);
        else if (strcmp(function_name, "jme_house_pos") == 0) value = jme_house_pos(0.0, fx->lat, 23.4, JME_HOUSE_EQUAL, geopos, errbuf);
        else if (strcmp(function_name, "jme_jd_add_seconds") == 0) value = jme_jd_add_seconds(jd, 3600.0);
        else if (strcmp(function_name, "jme_jd_difference_seconds") == 0) value = jme_jd_difference_seconds(jd + 1.0, jd);
        else if (strcmp(function_name, "jme_sidereal_time0") == 0) value = jme_sidereal_time0(jd, 23.4, 0.01);
        begin_record(out, function_name, engine, fx->id, "engine_invariant");
        fputs("\"status\":\"OK\",", out);
        print_double_obj(out, "return_value", value, 1);
        print_string_bytes_obj(out, "error_buffer", errbuf, sizeof(errbuf), 0);
        end_record(out);
        return 0;
    }
    if (strcmp(function_name, "jme_lmt_to_lat") == 0 || strcmp(function_name, "jme_lat_to_lmt") == 0) {
        rc = strcmp(function_name, "jme_lmt_to_lat") == 0
            ? jme_lmt_to_lat(jd, fx->lon, &results[0], errbuf)
            : jme_lat_to_lmt(jd, fx->lon, &results[0], errbuf);
        begin_record(out, function_name, engine, fx->id, "engine_invariant");
        fprintf(out, "\"status\":\"%s\",", rc == JME_OK ? "OK" : "ERROR");
        print_int_obj(out, "return_code", rc, 1);
        print_string_bytes_obj(out, "error_buffer", errbuf, sizeof(errbuf), 1);
        print_double_obj(out, "value", results[0], 0);
        end_record(out);
        return 0;
    }
    if (strcmp(function_name, "jme_centiseconds_difference") == 0 || strcmp(function_name, "jme_centiseconds_difference_signed") == 0
        || strcmp(function_name, "jme_centiseconds_normalize") == 0 || strcmp(function_name, "jme_centiseconds_round_second") == 0) {
        int value = 0;
        if (strcmp(function_name, "jme_centiseconds_difference") == 0) value = jme_centiseconds_difference(200, 100);
        else if (strcmp(function_name, "jme_centiseconds_difference_signed") == 0) value = jme_centiseconds_difference_signed(100, 200);
        else if (strcmp(function_name, "jme_centiseconds_normalize") == 0) value = jme_centiseconds_normalize(129600100);
        else value = jme_centiseconds_round_second(150);
        begin_record(out, function_name, engine, fx->id, "engine_invariant");
        fputs("\"status\":\"OK\",", out);
        print_int_obj(out, "return_value", value, 0);
        end_record(out);
        return 0;
    }
    if (strcmp(function_name, "jme_centiseconds_to_time_string") == 0
        || strcmp(function_name, "jme_centiseconds_to_lonlat_string") == 0
        || strcmp(function_name, "jme_centiseconds_to_degree_string") == 0) {
        char *p = 0;
        if (strcmp(function_name, "jme_centiseconds_to_time_string") == 0) p = jme_centiseconds_to_time_string(12345, buf1);
        else if (strcmp(function_name, "jme_centiseconds_to_lonlat_string") == 0) p = jme_centiseconds_to_lonlat_string(12345, buf1);
        else p = jme_centiseconds_to_degree_string(12345, buf1);
        begin_record(out, function_name, engine, fx->id, "engine_invariant");
        fputs("\"status\":\"OK\",", out);
        fprintf(out, "\"pointer_non_null\":%s,", p != 0 ? "true" : "false");
        print_string_bytes_obj(out, "buffer", buf1, sizeof(buf1), 0);
        end_record(out);
        return 0;
    }
    if (strcmp(function_name, "jme_reverse_julian_day") == 0 || strcmp(function_name, "jme_jd_to_utc") == 0 || strcmp(function_name, "jme_utc_time_zone") == 0) {
        int y = -1, m = -1, d = -1, h = -1, min = -1;
        double sec = NAN;
        if (strcmp(function_name, "jme_reverse_julian_day") == 0) {
            jme_reverse_julian_day(jd, JME_CALENDAR_GREGORIAN, &y, &m, &d, &sec);
            begin_record(out, function_name, engine, fx->id, "engine_invariant");
            fputs("\"status\":\"OK\",", out);
            print_int_obj(out, "year", y, 1); print_int_obj(out, "month", m, 1); print_int_obj(out, "day", d, 1); print_double_obj(out, "hour", sec, 0);
            end_record(out);
            return 0;
        }
        if (strcmp(function_name, "jme_jd_to_utc") == 0) {
            jme_jd_to_utc(jd, JME_CALENDAR_GREGORIAN, &y, &m, &d, &h, &min, &sec);
        } else {
            jme_utc_time_zone(fx->year, fx->month, fx->day, fx->hour, fx->minute, fx->second, 5.5, &y, &m, &d, &h, &min, &sec);
        }
        begin_record(out, function_name, engine, fx->id, "engine_invariant");
        fputs("\"status\":\"OK\",", out);
        print_int_obj(out, "year", y, 1); print_int_obj(out, "month", m, 1); print_int_obj(out, "day", d, 1);
        print_int_obj(out, "hour", h, 1); print_int_obj(out, "minute", min, 1); print_double_obj(out, "second", sec, 0);
        end_record(out);
        return 0;
    }
    if (strcmp(function_name, "jme_utc_to_jd") == 0) {
        rc = jme_utc_to_jd(fx->year, fx->month, fx->day, fx->hour, fx->minute, fx->second, JME_CALENDAR_GREGORIAN, results);
        begin_record(out, function_name, engine, fx->id, "engine_invariant");
        fputs("\"status\":\"OK\",", out);
        print_int_obj(out, "return_code", rc, 1);
        print_double_obj(out, "jd_utc", results[0], 0);
        end_record(out);
        return 0;
    }
    if (strcmp(function_name, "jme_get_obliquity") == 0 || strcmp(function_name, "jme_time_equ") == 0 || strcmp(function_name, "jme_get_ayanamsa_ex") == 0 || strcmp(function_name, "jme_get_ayanamsa_ex_ut") == 0) {
        double v = NAN;
        if (strcmp(function_name, "jme_get_obliquity") == 0) rc = jme_get_obliquity(jd, JME_MODEL_OBL_IAU_2006, &v, errbuf);
        else if (strcmp(function_name, "jme_time_equ") == 0) rc = jme_time_equ(jd, &v, errbuf);
        else if (strcmp(function_name, "jme_get_ayanamsa_ex") == 0) rc = jme_get_ayanamsa_ex(jd, JME_SIDEREAL_LAHIRI, &v, errbuf);
        else rc = jme_get_ayanamsa_ex_ut(jd, JME_SIDEREAL_LAHIRI, &v, errbuf);
        begin_record(out, function_name, engine, fx->id, strcmp(function_name, "jme_time_equ") == 0 ? "engine_sensitive" : "engine_invariant");
        fprintf(out, "\"status\":\"%s\",", rc == JME_OK ? "OK" : "ERROR");
        print_int_obj(out, "return_code", rc, 1);
        print_string_bytes_obj(out, "error_buffer", errbuf, sizeof(errbuf), 1);
        print_double_obj(out, "value", v, 0);
        end_record(out);
        return 0;
    }
    if (strcmp(function_name, "jme_get_frame_bias_matrix") == 0 || strcmp(function_name, "jme_get_precession_matrix") == 0) {
        if (strcmp(function_name, "jme_get_frame_bias_matrix") == 0) rc = jme_get_frame_bias_matrix(JME_MODEL_BIAS_IAU2006, results);
        else rc = jme_get_precession_matrix(2451545.0, jd, JME_MODEL_PREC_IAU_2006, results);
        begin_record(out, function_name, engine, fx->id, "engine_invariant");
        fprintf(out, "\"status\":\"%s\",", rc == JME_OK ? "OK" : "ERROR");
        print_int_obj(out, "return_code", rc, 1);
        print_double_array_obj(out, "matrix", results, 9, 0);
        end_record(out);
        return 0;
    }
    if (strcmp(function_name, "jme_get_nutation") == 0) {
        rc = jme_get_nutation(jd, JME_MODEL_NUT_IAU_2000B, &results[0], &results[1], errbuf);
        begin_record(out, function_name, engine, fx->id, "engine_invariant");
        fprintf(out, "\"status\":\"%s\",", rc == JME_OK ? "OK" : "ERROR");
        print_int_obj(out, "return_code", rc, 1);
        print_string_bytes_obj(out, "error_buffer", errbuf, sizeof(errbuf), 1);
        print_double_array_obj(out, "values", results, 2, 0);
        end_record(out);
        return 0;
    }
    if (strcmp(function_name, "jme_get_nutation_matrix") == 0 || strcmp(function_name, "jme_matrix_identity") == 0
        || strcmp(function_name, "jme_matrix_rotate_x") == 0 || strcmp(function_name, "jme_matrix_rotate_y") == 0
        || strcmp(function_name, "jme_matrix_rotate_z") == 0) {
        if (strcmp(function_name, "jme_get_nutation_matrix") == 0) jme_get_nutation_matrix(0.01, 0.02, 0.4, results);
        else if (strcmp(function_name, "jme_matrix_identity") == 0) jme_matrix_identity(results);
        else if (strcmp(function_name, "jme_matrix_rotate_x") == 0) jme_matrix_rotate_x(0.1, results);
        else if (strcmp(function_name, "jme_matrix_rotate_y") == 0) jme_matrix_rotate_y(0.2, results);
        else jme_matrix_rotate_z(0.3, results);
        begin_record(out, function_name, engine, fx->id, "engine_invariant");
        fputs("\"status\":\"OK\",", out);
        print_double_array_obj(out, "matrix", results, 9, 0);
        end_record(out);
        return 0;
    }
    if (strcmp(function_name, "jme_matrix_multiply") == 0 || strcmp(function_name, "jme_matrix_transform_state") == 0) {
        jme_matrix_identity(results);
        jme_matrix_rotate_z(0.3, results2);
        if (strcmp(function_name, "jme_matrix_multiply") == 0) {
            jme_matrix_multiply(results, results2, results);
            begin_record(out, function_name, engine, fx->id, "engine_invariant");
            fputs("\"status\":\"OK\",", out);
            print_double_array_obj(out, "matrix", results, 9, 0);
            end_record(out);
        } else {
            double st[6] = {1,2,3,4,5,6};
            jme_matrix_transform_state(results2, st, results);
            begin_record(out, function_name, engine, fx->id, "engine_invariant");
            fputs("\"status\":\"OK\",", out);
            print_double_array_obj(out, "state", results, 6, 0);
            end_record(out);
        }
        return 0;
    }
    if (strcmp(function_name, "jme_get_topo_pos") == 0) {
        rc = jme_get_topo_pos(jd, results, errbuf);
        begin_record(out, function_name, engine, fx->id, "engine_invariant");
        fprintf(out, "\"status\":\"%s\",", rc == JME_OK ? "OK" : "ERROR");
        print_int_obj(out, "return_code", rc, 1);
        print_string_bytes_obj(out, "error_buffer", errbuf, sizeof(errbuf), 1);
        print_double_array_obj(out, "values", results, 3, 0);
        end_record(out);
        return 0;
    }
    if (strcmp(function_name, "jme_state_add") == 0 || strcmp(function_name, "jme_state_subtract") == 0 || strcmp(function_name, "jme_state_scale") == 0 || strcmp(function_name, "jme_state_convert_units") == 0) {
        double a[6] = {1,2,2,0.1,-0.2,0.2};
        double b[6] = {-0.5,4,1,-0.4,0.3,0.0};
        if (strcmp(function_name, "jme_state_add") == 0) rc = jme_state_add(a, b, results);
        else if (strcmp(function_name, "jme_state_subtract") == 0) rc = jme_state_subtract(a, b, results);
        else if (strcmp(function_name, "jme_state_scale") == 0) rc = jme_state_scale(a, -2.0, results);
        else rc = jme_state_convert_units(a, JME_VECTOR_AU_PER_DAY, JME_VECTOR_KM_PER_SECOND, results);
        begin_record(out, function_name, engine, fx->id, "engine_invariant");
        fprintf(out, "\"status\":\"%s\",", rc == JME_OK ? "OK" : "ERROR");
        print_int_obj(out, "return_code", rc, 1);
        print_double_array_obj(out, "values", results, 6, 0);
        end_record(out);
        return 0;
    }
    if (strcmp(function_name, "jme_spherical_to_rectangular_state") == 0 || strcmp(function_name, "jme_rectangular_to_spherical_state") == 0
        || strcmp(function_name, "jme_ecliptic_to_equatorial_rectangular_state") == 0 || strcmp(function_name, "jme_equatorial_to_ecliptic_rectangular_state") == 0) {
        double a[6] = {30.0, -20.0, 2.0, 0.1, -0.2, 0.03};
        if (strcmp(function_name, "jme_spherical_to_rectangular_state") == 0) rc = jme_spherical_to_rectangular_state(a, results);
        else if (strcmp(function_name, "jme_rectangular_to_spherical_state") == 0) {
            jme_spherical_to_rectangular_state(a, results2);
            rc = jme_rectangular_to_spherical_state(results2, results);
        } else if (strcmp(function_name, "jme_ecliptic_to_equatorial_rectangular_state") == 0) {
            rc = jme_ecliptic_to_equatorial_rectangular_state(a, 0.4, results);
        } else {
            rc = jme_equatorial_to_ecliptic_rectangular_state(a, 0.4, results);
        }
        begin_record(out, function_name, engine, fx->id, "engine_invariant");
        fprintf(out, "\"status\":\"%s\",", rc == JME_OK ? "OK" : "ERROR");
        print_int_obj(out, "return_code", rc, 1);
        print_double_array_obj(out, "values", results, 6, 0);
        end_record(out);
        return 0;
    }
    if (strcmp(function_name, "jme_ecliptic_to_equatorial") == 0 || strcmp(function_name, "jme_equatorial_to_ecliptic") == 0
        || strcmp(function_name, "jme_equatorial_to_horizontal") == 0 || strcmp(function_name, "jme_horizontal_to_equatorial") == 0
        || strcmp(function_name, "jme_spherical_angular_separation") == 0 || strcmp(function_name, "jme_spherical_position_angle") == 0
        || strcmp(function_name, "jme_refract_extended") == 0 || strcmp(function_name, "jme_split_degree") == 0) {
        begin_record(out, function_name, engine, fx->id, "engine_invariant");
        fputs("\"status\":\"OK\",", out);
        if (strcmp(function_name, "jme_ecliptic_to_equatorial") == 0) {
            jme_ecliptic_to_equatorial(0,0,0.4,&results[0],&results[1]); print_double_array_obj(out,"values",results,2,0);
        } else if (strcmp(function_name, "jme_equatorial_to_ecliptic") == 0) {
            jme_equatorial_to_ecliptic(0,0,0.4,&results[0],&results[1]); print_double_array_obj(out,"values",results,2,0);
        } else if (strcmp(function_name, "jme_equatorial_to_horizontal") == 0) {
            jme_equatorial_to_horizontal(0,0,45,&results[0],&results[1]); print_double_array_obj(out,"values",results,2,0);
        } else if (strcmp(function_name, "jme_horizontal_to_equatorial") == 0) {
            jme_horizontal_to_equatorial(0,45,45,&results[0],&results[1]); print_double_array_obj(out,"values",results,2,0);
        } else if (strcmp(function_name, "jme_spherical_angular_separation") == 0) {
            print_double_obj(out,"return_value",jme_spherical_angular_separation(0,0,90,0),0);
        } else if (strcmp(function_name, "jme_spherical_position_angle") == 0) {
            print_double_obj(out,"return_value",jme_spherical_position_angle(0,0,0,10),0);
        } else if (strcmp(function_name, "jme_refract_extended") == 0) {
            print_double_obj(out,"return_value",jme_refract_extended(10,0,1013.25,15,0.0065,0,results),1); print_double_array_obj(out,"out",results,4,0);
        } else {
            int ideg=0, imin=0, isec=0, isgn=0; double dsecfr=0;
            jme_split_degree(12.3456,0,&ideg,&imin,&isec,&dsecfr,&isgn);
            print_int_obj(out,"ideg",ideg,1); print_int_obj(out,"imin",imin,1); print_int_obj(out,"isec",isec,1); print_double_obj(out,"dsecfr",dsecfr,1); print_int_obj(out,"isgn",isgn,0);
        }
        end_record(out); return 0;
    }
    if (strcmp(function_name, "jme_vsop87_planet_state") == 0 || strcmp(function_name, "jme_moshier_planet_state") == 0 || strcmp(function_name, "jme_meeus_planet_state") == 0
        || strcmp(function_name, "jme_meeus_sun_state") == 0 || strcmp(function_name, "jme_meeus_moon_state") == 0 || strcmp(function_name, "jme_elp2000_moon_state") == 0) {
        if (strcmp(function_name, "jme_vsop87_planet_state") == 0) rc = jme_vsop87_planet_state(jd, JME_BODY_MARS, results);
        else if (strcmp(function_name, "jme_moshier_planet_state") == 0) rc = jme_moshier_planet_state(jd, JME_BODY_MARS, results);
        else if (strcmp(function_name, "jme_meeus_planet_state") == 0) rc = jme_meeus_planet_state(jd, JME_BODY_MARS, results);
        else if (strcmp(function_name, "jme_meeus_sun_state") == 0) rc = jme_meeus_sun_state(jd, results);
        else if (strcmp(function_name, "jme_meeus_moon_state") == 0) rc = jme_meeus_moon_state(jd, results);
        else rc = jme_elp2000_moon_state(jd, results);
        begin_record(out, function_name, engine, fx->id, "engine_sensitive");
        fprintf(out, "\"status\":\"%s\",", rc == JME_OK ? "OK" : "ERROR");
        print_int_obj(out, "return_code", rc, 1);
        print_double_array_obj(out, "state", results, 6, 0);
        end_record(out);
        return 0;
    }
    if (strncmp(function_name, "jme_jpl_", 8) == 0) {
        if (strcmp(function_name, "jme_jpl_is_available") == 0) {
            begin_record(out, function_name, engine, fx->id, "engine_invariant");
            fputs("\"status\":\"OK\",", out); print_int_obj(out, "return_value", jme_jpl_is_available(), 0); end_record(out); return 0;
        }
        if (strcmp(function_name, "jme_jpl_is_open") == 0) {
            begin_record(out, function_name, engine, fx->id, "engine_invariant");
            fputs("\"status\":\"OK\",", out); print_int_obj(out, "return_value", jme_jpl_is_open(), 0); end_record(out); return 0;
        }
        if (strcmp(function_name, "jme_jpl_timescale") == 0) {
            begin_record(out, function_name, engine, fx->id, "engine_invariant");
            fputs("\"status\":\"OK\",", out); print_int_obj(out, "return_value", jme_jpl_timescale(), 0); end_record(out); return 0;
        }
        if (strcmp(function_name, "jme_jpl_engine_version") == 0) {
            begin_record(out, function_name, engine, fx->id, "engine_invariant");
            jme_jpl_engine_version(buf1, sizeof(buf1));
            fputs("\"status\":\"OK\",", out); print_string_bytes_obj(out, "buffer", buf1, sizeof(buf1), 0); end_record(out); return 0;
        }
        if (strcmp(function_name, "jme_jpl_open") == 0) rc = jme_jpl_open(kernel_path, errbuf);
        else if (strcmp(function_name, "jme_jpl_open_array") == 0) { const char *paths[1] = {kernel_path}; rc = jme_jpl_open_array(1, paths, errbuf); }
        else if (strcmp(function_name, "jme_jpl_close") == 0) { jme_jpl_close(); begin_record(out,function_name,engine,fx->id,"engine_sensitive"); fputs("\"status\":\"OK\",",out); print_int_obj(out,"post_is_open",jme_jpl_is_open(),0); end_record(out); return 0; }
        else if (strcmp(function_name, "jme_jpl_prefetch") == 0) rc = jme_jpl_prefetch(errbuf);
        else if (strcmp(function_name, "jme_jpl_is_thread_safe") == 0) rc = jme_jpl_is_thread_safe(errbuf);
        else if (strcmp(function_name, "jme_jpl_id_by_name") == 0) rc = jme_jpl_id_by_name("EARTH", &i, errbuf);
        else if (strcmp(function_name, "jme_jpl_name_by_id") == 0) rc = jme_jpl_name_by_id(399, buf1, sizeof(buf1), errbuf);
        else if (strcmp(function_name, "jme_jpl_max_supported_order") == 0) { begin_record(out,function_name,engine,fx->id,"engine_sensitive"); fputs("\"status\":\"OK\",",out); print_int_obj(out,"return_value",jme_jpl_max_supported_order(2),0); end_record(out); return 0; }
        else if (strcmp(function_name, "jme_jpl_coverage") == 0) rc = jme_jpl_coverage(&results[0], &results[1], &i, errbuf);
        else if (strcmp(function_name, "jme_jpl_constant") == 0) rc = jme_jpl_constant("AU", &results[0], errbuf);
        else if (strcmp(function_name, "jme_jpl_constant_count") == 0) { begin_record(out,function_name,engine,fx->id,"engine_sensitive"); print_int_obj(out,"return_value",jme_jpl_constant_count(errbuf),1); print_string_bytes_obj(out,"error_buffer",errbuf,sizeof(errbuf),0); end_record(out); return 0; }
        else if (strcmp(function_name, "jme_jpl_constant_index") == 0) rc = jme_jpl_constant_index(0, buf1, sizeof(buf1), &results[0], errbuf);
        else if (strcmp(function_name, "jme_jpl_constant_vector") == 0) rc = jme_jpl_constant_vector("AU", results, 3, errbuf);
        else if (strcmp(function_name, "jme_jpl_constant_string") == 0) rc = jme_jpl_constant_string("NAME", buf1, sizeof(buf1), errbuf);
        else if (strcmp(function_name, "jme_jpl_constant_string_vector") == 0) rc = jme_jpl_constant_string_vector("NAME", buf1, 32, 2, errbuf);
        else if (strcmp(function_name, "jme_jpl_file_version") == 0) rc = jme_jpl_file_version(buf1, sizeof(buf1), errbuf);
        else if (strcmp(function_name, "jme_jpl_current_file_data") == 0) rc = jme_jpl_current_file_data(buf1, sizeof(buf1), &results[0], &results[1], &i, errbuf);
        else if (strcmp(function_name, "jme_jpl_position_record_count") == 0) { begin_record(out,function_name,engine,fx->id,"engine_sensitive"); print_int_obj(out,"return_value",jme_jpl_position_record_count(errbuf),1); print_string_bytes_obj(out,"error_buffer",errbuf,sizeof(errbuf),0); end_record(out); return 0; }
        else if (strcmp(function_name, "jme_jpl_position_record_index") == 0) rc = jme_jpl_position_record_index(0, &i, &saved_errno, &results[0], &results[1], &((int*)results2)[0], &((int*)results2)[1], errbuf);
        else if (strcmp(function_name, "jme_jpl_orientation_record_count") == 0) { begin_record(out,function_name,engine,fx->id,"engine_sensitive"); print_int_obj(out,"return_value",jme_jpl_orientation_record_count(errbuf),1); print_string_bytes_obj(out,"error_buffer",errbuf,sizeof(errbuf),0); end_record(out); return 0; }
        else if (strcmp(function_name, "jme_jpl_orientation_record_index") == 0) rc = jme_jpl_orientation_record_index(0, &i, &results[0], &results[1], &saved_errno, &((int*)results2)[0], errbuf);
        else if (strcmp(function_name, "jme_jpl_body_state") == 0) rc = jme_jpl_body_state(jd, JME_BODY_SUN, JME_BODY_EARTH, JME_VECTOR_AU_PER_DAY, results, errbuf);
        else if (strcmp(function_name, "jme_jpl_body_state_split") == 0) rc = jme_jpl_body_state_split(jd, 0.25, JME_BODY_SUN, JME_BODY_EARTH, JME_VECTOR_AU_PER_DAY, results, errbuf);
        else if (strcmp(function_name, "jme_jpl_ecliptic_state") == 0) rc = jme_jpl_ecliptic_state(jd, JME_BODY_SUN, JME_BODY_EARTH, JME_VECTOR_AU_PER_DAY, results, errbuf);
        else if (strcmp(function_name, "jme_jpl_body_state_naif") == 0) rc = jme_jpl_body_state_naif(jd, 10, 399, JME_VECTOR_AU_PER_DAY, results, errbuf);
        else if (strcmp(function_name, "jme_jpl_body_state_split_naif") == 0) rc = jme_jpl_body_state_split_naif(jd, 0.25, 10, 399, JME_VECTOR_AU_PER_DAY, results, errbuf);
        else if (strcmp(function_name, "jme_jpl_body_state_native_naif") == 0) rc = jme_jpl_body_state_native_naif(jd, 10, 399, results, errbuf);
        else if (strcmp(function_name, "jme_jpl_body_state_native") == 0) rc = jme_jpl_body_state_native(jd, JME_BODY_SUN, JME_BODY_EARTH, results, errbuf);
        else if (strcmp(function_name, "jme_jpl_body_state_native_split_naif") == 0) rc = jme_jpl_body_state_native_split_naif(jd, 0.25, 10, 399, results, errbuf);
        else if (strcmp(function_name, "jme_jpl_body_state_native_split") == 0) rc = jme_jpl_body_state_native_split(jd, 0.25, JME_BODY_SUN, JME_BODY_EARTH, results, errbuf);
        else if (strcmp(function_name, "jme_jpl_body_state_order_naif") == 0) rc = jme_jpl_body_state_order_naif(jd, 0.25, 10, 399, JME_VECTOR_AU_PER_DAY, 1, results, errbuf);
        else if (strcmp(function_name, "jme_jpl_body_state_order") == 0) rc = jme_jpl_body_state_order(jd, 0.25, JME_BODY_SUN, JME_BODY_EARTH, JME_VECTOR_AU_PER_DAY, 1, results, errbuf);
        else if (strcmp(function_name, "jme_jpl_body_state_utc_naif") == 0) rc = jme_jpl_body_state_utc_naif(fx->year,fx->month,fx->day,fx->hour,fx->minute,fx->second,JME_CALENDAR_GREGORIAN,10,399,JME_VECTOR_AU_PER_DAY,results,errbuf);
        else if (strcmp(function_name, "jme_jpl_body_state_utc") == 0) rc = jme_jpl_body_state_utc(fx->year,fx->month,fx->day,fx->hour,fx->minute,fx->second,JME_CALENDAR_GREGORIAN,JME_BODY_SUN,JME_BODY_EARTH,JME_VECTOR_AU_PER_DAY,results,errbuf);
        else if (strcmp(function_name, "jme_jpl_ecliptic_state_naif") == 0) rc = jme_jpl_ecliptic_state_naif(jd,10,399,JME_VECTOR_AU_PER_DAY,results,errbuf);
        else if (strcmp(function_name, "jme_jpl_ecliptic_state_split") == 0) rc = jme_jpl_ecliptic_state_split(jd,0.25,JME_BODY_SUN,JME_BODY_EARTH,JME_VECTOR_AU_PER_DAY,results,errbuf);
        else if (strcmp(function_name, "jme_jpl_ecliptic_state_split_naif") == 0) rc = jme_jpl_ecliptic_state_split_naif(jd,0.25,10,399,JME_VECTOR_AU_PER_DAY,results,errbuf);
        else if (strcmp(function_name, "jme_jpl_ecliptic_state_utc_naif") == 0) rc = jme_jpl_ecliptic_state_utc_naif(fx->year,fx->month,fx->day,fx->hour,fx->minute,fx->second,JME_CALENDAR_GREGORIAN,10,399,JME_VECTOR_AU_PER_DAY,results,errbuf);
        else if (strcmp(function_name, "jme_jpl_ecliptic_state_utc") == 0) rc = jme_jpl_ecliptic_state_utc(fx->year,fx->month,fx->day,fx->hour,fx->minute,fx->second,JME_CALENDAR_GREGORIAN,JME_BODY_SUN,JME_BODY_EARTH,JME_VECTOR_AU_PER_DAY,results,errbuf);
        else if (strcmp(function_name, "jme_jpl_orientation_state_naif") == 0) rc = jme_jpl_orientation_state_naif(jd,399,JME_ORIENTATION_RAD_PER_DAY,results,errbuf);
        else if (strcmp(function_name, "jme_jpl_orientation_state_split_naif") == 0) rc = jme_jpl_orientation_state_split_naif(jd,0.25,399,JME_ORIENTATION_RAD_PER_DAY,results,errbuf);
        else if (strcmp(function_name, "jme_jpl_orientation_state_order_naif") == 0) rc = jme_jpl_orientation_state_order_naif(jd,0.25,399,JME_ORIENTATION_RAD_PER_DAY,1,results,errbuf);
        else if (strcmp(function_name, "jme_jpl_orientation_state_utc_naif") == 0) rc = jme_jpl_orientation_state_utc_naif(fx->year,fx->month,fx->day,fx->hour,fx->minute,fx->second,JME_CALENDAR_GREGORIAN,399,JME_ORIENTATION_RAD_PER_DAY,results,errbuf);
        else if (strcmp(function_name, "jme_jpl_rotational_angular_momentum_state_naif") == 0) rc = jme_jpl_rotational_angular_momentum_state_naif(jd,399,JME_ORIENTATION_RAD_PER_DAY,results,errbuf);
        else if (strcmp(function_name, "jme_jpl_rotational_angular_momentum_state_utc_naif") == 0) rc = jme_jpl_rotational_angular_momentum_state_utc_naif(fx->year,fx->month,fx->day,fx->hour,fx->minute,fx->second,JME_CALENDAR_GREGORIAN,399,JME_ORIENTATION_RAD_PER_DAY,results,errbuf);
        else if (strcmp(function_name, "jme_jpl_rotational_angular_momentum_state_split_naif") == 0) rc = jme_jpl_rotational_angular_momentum_state_split_naif(jd,0.25,399,JME_ORIENTATION_RAD_PER_DAY,results,errbuf);
        else if (strcmp(function_name, "jme_jpl_rotational_angular_momentum_state_order_naif") == 0) rc = jme_jpl_rotational_angular_momentum_state_order_naif(jd,0.25,399,JME_ORIENTATION_RAD_PER_DAY,1,results,errbuf);
        else rc = JME_ERR;
        begin_record(out, function_name, engine, fx->id, "engine_sensitive");
        fprintf(out, "\"status\":\"%s\",", rc == JME_OK ? "OK" : (strcmp(engine,"JPL")==0 ? "ERROR_JPL_UNAVAILABLE" : "ERROR"));
        print_int_obj(out, "return_code", rc, 1);
        print_string_bytes_obj(out, "error_buffer", errbuf, sizeof(errbuf), 1);
        print_double_array_obj(out, "values", results, 12, 0);
        end_record(out);
        return 0;
    }
    if (strcmp(function_name, "jme_calc") == 0 || strcmp(function_name, "jme_calc_ut") == 0 || strcmp(function_name, "jme_calc_pctr") == 0
        || strcmp(function_name, "jme_pheno") == 0 || strcmp(function_name, "jme_pheno_ut") == 0
        || strcmp(function_name, "jme_fixstar") == 0 || strcmp(function_name, "jme_fixstar_ut") == 0 || strcmp(function_name, "jme_fixstar_mag") == 0
        || strcmp(function_name, "jme_fixstar2") == 0 || strcmp(function_name, "jme_fixstar2_ut") == 0 || strcmp(function_name, "jme_fixstar2_mag") == 0
        || strcmp(function_name, "jme_houses") == 0 || strcmp(function_name, "jme_houses_ex") == 0 || strcmp(function_name, "jme_houses_ex2") == 0
        || strcmp(function_name, "jme_houses_armc") == 0 || strcmp(function_name, "jme_houses_armc_ex2") == 0
        || strcmp(function_name, "jme_gauquelin_sector") == 0 || strcmp(function_name, "jme_heliacal_ut") == 0
        || strcmp(function_name, "jme_heliacal_pheno_ut") == 0 || strcmp(function_name, "jme_vis_limit_mag") == 0
        || strcmp(function_name, "jme_solcross") == 0 || strcmp(function_name, "jme_solcross_ut") == 0
        || strcmp(function_name, "jme_mooncross") == 0 || strcmp(function_name, "jme_mooncross_ut") == 0
        || strcmp(function_name, "jme_mooncross_node") == 0 || strcmp(function_name, "jme_mooncross_node_ut") == 0
        || strcmp(function_name, "jme_helio_cross") == 0 || strcmp(function_name, "jme_helio_cross_ut") == 0
        || strcmp(function_name, "jme_nod_aps") == 0 || strcmp(function_name, "jme_nod_aps_ut") == 0
        || strcmp(function_name, "jme_get_orbital_elements") == 0 || strcmp(function_name, "jme_orbit_max_min_true_distance") == 0) {
        if (strcmp(function_name, "jme_calc") == 0) rc = jme_calc(jd, JME_BODY_MARS, JME_CALC_SPEED, results, errbuf);
        else if (strcmp(function_name, "jme_calc_ut") == 0) rc = jme_calc_ut(jd, JME_BODY_MARS, JME_CALC_SPEED, results, errbuf);
        else if (strcmp(function_name, "jme_calc_pctr") == 0) rc = jme_calc_pctr(jd, JME_BODY_MARS, JME_BODY_SUN, JME_CALC_XYZ, results, errbuf);
        else if (strcmp(function_name, "jme_pheno") == 0) rc = jme_pheno(jd, JME_BODY_MARS, 0, results, errbuf);
        else if (strcmp(function_name, "jme_pheno_ut") == 0) rc = jme_pheno_ut(jd, JME_BODY_MARS, 0, results, errbuf);
        else if (strcmp(function_name, "jme_fixstar") == 0) rc = jme_fixstar("Sirius", jd, 0, results, errbuf);
        else if (strcmp(function_name, "jme_fixstar_ut") == 0) rc = jme_fixstar_ut("Sirius", jd, 0, results, errbuf);
        else if (strcmp(function_name, "jme_fixstar_mag") == 0) rc = jme_fixstar_mag("Sirius", results, errbuf);
        else if (strcmp(function_name, "jme_fixstar2") == 0) rc = jme_fixstar2("Sirius", jd, 0, results, errbuf);
        else if (strcmp(function_name, "jme_fixstar2_ut") == 0) rc = jme_fixstar2_ut("Sirius", jd, 0, results, errbuf);
        else if (strcmp(function_name, "jme_fixstar2_mag") == 0) rc = jme_fixstar2_mag("Sirius", results, errbuf);
        else if (strcmp(function_name, "jme_houses") == 0) rc = jme_houses(jd, fx->lat, fx->lon, JME_HOUSE_PLACIDUS, results, results2);
        else if (strcmp(function_name, "jme_houses_ex") == 0) rc = jme_houses_ex(jd, 0, fx->lat, fx->lon, JME_HOUSE_PLACIDUS, results, results2);
        else if (strcmp(function_name, "jme_houses_ex2") == 0) rc = jme_houses_ex2(jd, 0, fx->lat, fx->lon, JME_HOUSE_PLACIDUS, results, results2, &results[13], &results2[10]);
        else if (strcmp(function_name, "jme_houses_armc") == 0) rc = jme_houses_armc(0.0, fx->lat, 23.4, JME_HOUSE_EQUAL, results, results2);
        else if (strcmp(function_name, "jme_houses_armc_ex2") == 0) rc = jme_houses_armc_ex2(0.0, fx->lat, 23.4, JME_HOUSE_EQUAL, results, results2, &results[13], &results2[10]);
        else if (strcmp(function_name, "jme_gauquelin_sector") == 0) rc = jme_gauquelin_sector(jd, JME_BODY_SUN, 0, 0, 0, geopos, 1013.25, 15.0, results, errbuf);
        else if (strcmp(function_name, "jme_heliacal_ut") == 0) rc = jme_heliacal_ut(jd, geopos, results, errbuf);
        else if (strcmp(function_name, "jme_heliacal_pheno_ut") == 0) rc = jme_heliacal_pheno_ut(jd, geopos, results, errbuf);
        else if (strcmp(function_name, "jme_vis_limit_mag") == 0) rc = jme_vis_limit_mag(jd, geopos, results, errbuf);
        else if (strcmp(function_name, "jme_solcross") == 0) rc = jme_solcross(0.0, jd, 0, results, errbuf);
        else if (strcmp(function_name, "jme_solcross_ut") == 0) rc = jme_solcross_ut(0.0, jd, 0, results, errbuf);
        else if (strcmp(function_name, "jme_mooncross") == 0) rc = jme_mooncross(0.0, jd, 0, results, errbuf);
        else if (strcmp(function_name, "jme_mooncross_ut") == 0) rc = jme_mooncross_ut(0.0, jd, 0, results, errbuf);
        else if (strcmp(function_name, "jme_mooncross_node") == 0) rc = jme_mooncross_node(jd, 0, results, errbuf);
        else if (strcmp(function_name, "jme_mooncross_node_ut") == 0) rc = jme_mooncross_node_ut(jd, 0, results, errbuf);
        else if (strcmp(function_name, "jme_helio_cross") == 0) rc = jme_helio_cross(JME_BODY_MARS, 0.0, jd, 0, results, errbuf);
        else if (strcmp(function_name, "jme_helio_cross_ut") == 0) rc = jme_helio_cross_ut(JME_BODY_MARS, 0.0, jd, 0, results, errbuf);
        else if (strcmp(function_name, "jme_nod_aps") == 0) rc = jme_nod_aps(jd, JME_BODY_MARS, 0, 0, results, errbuf);
        else if (strcmp(function_name, "jme_nod_aps_ut") == 0) rc = jme_nod_aps_ut(jd, JME_BODY_MARS, 0, 0, results, errbuf);
        else if (strcmp(function_name, "jme_get_orbital_elements") == 0) rc = jme_get_orbital_elements(jd, JME_BODY_MARS, 0, results, errbuf);
        else rc = jme_orbit_max_min_true_distance(jd, JME_BODY_MERCURY, 0, &results[0], &results[1], &results[2], &results[3], errbuf);
        begin_record(out, function_name, engine, fx->id, "engine_sensitive");
        fprintf(out, "\"status\":\"%s\",", rc == JME_OK ? "OK" : (strcmp(engine,"JPL")==0 ? "ERROR_JPL_UNAVAILABLE" : "ERROR"));
        print_int_obj(out, "return_code", rc, 1);
        print_string_bytes_obj(out, "error_buffer", errbuf, sizeof(errbuf), 1);
        print_double_array_obj(out, "values", results, 32, 0);
        end_record(out);
        return 0;
    }
    if (strcmp(function_name, "jme_sol_eclipse_where") == 0 || strcmp(function_name, "jme_sol_eclipse_how") == 0
        || strcmp(function_name, "jme_sol_eclipse_when_loc") == 0 || strcmp(function_name, "jme_sol_eclipse_when_glob") == 0
        || strcmp(function_name, "jme_lun_eclipse_how") == 0 || strcmp(function_name, "jme_lun_eclipse_when_loc") == 0
        || strcmp(function_name, "jme_lun_eclipse_when") == 0 || strcmp(function_name, "jme_lun_occult_where") == 0
        || strcmp(function_name, "jme_lun_occult_when_loc") == 0 || strcmp(function_name, "jme_lun_occult_when_glob") == 0
        || strcmp(function_name, "jme_rise_trans") == 0 || strcmp(function_name, "jme_rise_trans_true_hor") == 0) {
        const fixture_def *efx = (strcmp(function_name, "jme_rise_trans") == 0 || strcmp(function_name, "jme_rise_trans_true_hor") == 0) ? &fixture_tromso : &fixture_modern_bhuj;
        double ejd = fixture_jd_ut(efx);
        double ego[3] = {efx->lon, efx->lat, efx->altitude_m};
        if (strcmp(function_name, "jme_sol_eclipse_where") == 0) rc = jme_sol_eclipse_where(ejd, 0, ego, results, errbuf);
        else if (strcmp(function_name, "jme_sol_eclipse_how") == 0) rc = jme_sol_eclipse_how(ejd, 0, ego, results, errbuf);
        else if (strcmp(function_name, "jme_sol_eclipse_when_loc") == 0) rc = jme_sol_eclipse_when_loc(ejd, 0, ego, results, results2, 0, errbuf);
        else if (strcmp(function_name, "jme_sol_eclipse_when_glob") == 0) rc = jme_sol_eclipse_when_glob(ejd, 0, 0, results, 0, errbuf);
        else if (strcmp(function_name, "jme_lun_eclipse_how") == 0) rc = jme_lun_eclipse_how(ejd, 0, ego, results, errbuf);
        else if (strcmp(function_name, "jme_lun_eclipse_when_loc") == 0) rc = jme_lun_eclipse_when_loc(ejd, 0, ego, results, results2, 0, errbuf);
        else if (strcmp(function_name, "jme_lun_eclipse_when") == 0) rc = jme_lun_eclipse_when(ejd, 0, 0, results, 0, errbuf);
        else if (strcmp(function_name, "jme_lun_occult_where") == 0) rc = jme_lun_occult_where(ejd, JME_BODY_MARS, 0, 0, ego, results, errbuf);
        else if (strcmp(function_name, "jme_lun_occult_when_loc") == 0) rc = jme_lun_occult_when_loc(ejd, JME_BODY_MARS, 0, 0, ego, results, results2, 0, errbuf);
        else if (strcmp(function_name, "jme_lun_occult_when_glob") == 0) rc = jme_lun_occult_when_glob(ejd, JME_BODY_MARS, 0, 0, 0, results, 0, errbuf);
        else if (strcmp(function_name, "jme_rise_trans") == 0) rc = jme_rise_trans(ejd, JME_BODY_SUN, 0, 0, JME_RISE_RISE, ego, 1013.25, 15.0, results, errbuf);
        else rc = jme_rise_trans_true_hor(ejd, JME_BODY_SUN, 0, 0, JME_RISE_RISE, ego, 1013.25, 15.0, 0.0, results, errbuf);
        begin_record(out, function_name, engine, efx->id, "engine_sensitive");
        fprintf(out, "\"status\":\"%s\",", rc == JME_OK ? "OK" : (strcmp(engine,"JPL")==0 ? "ERROR_JPL_UNAVAILABLE" : "ERROR"));
        print_int_obj(out, "return_code", rc, 1);
        print_string_bytes_obj(out, "error_buffer", errbuf, sizeof(errbuf), 1);
        print_double_array_obj(out, "values", results, 32, 0);
        end_record(out);
        return 0;
    }

    emit_skipped(out, function_name, engine, fx->id, "unadapted", "adapter not yet implemented in probe");
    return 0;
}

int main(int argc, char **argv)
{
    const char *function_name = 0;
    const char *engine = 0;
    const char *kernel_path = 0;
    int i;

    for (i = 1; i < argc; i++) {
        if (strcmp(argv[i], "--function") == 0 && i + 1 < argc) {
            function_name = argv[++i];
        } else if (strcmp(argv[i], "--engine") == 0 && i + 1 < argc) {
            engine = argv[++i];
        } else if (strcmp(argv[i], "--kernel") == 0 && i + 1 < argc) {
            kernel_path = argv[++i];
        }
    }

    if (function_name == 0 || engine == 0) {
        fprintf(stderr, "usage: %s --function NAME --engine ENGINE [--kernel PATH]\n", argv[0]);
        return 2;
    }

    return do_probe(function_name, engine, kernel_path);
}
