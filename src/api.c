#include "jme/jme.h"

#include "context.h"

#include <string.h>

const char *jme_version(char *buffer, size_t buffer_size)
{
    if (buffer != 0 && buffer_size > 0) {
        size_t n = sizeof(JME_VERSION);
        if (n > buffer_size) {
            n = buffer_size;
        }

        memcpy(buffer, JME_VERSION, n);
        buffer[buffer_size - 1] = '\0';
    }

    return JME_VERSION;
}

void jme_close(void)
{
    jme_jpl_close();
}

const char *jme_ephemeris_path(void)
{
    return jme_get_context()->ephemeris_path;
}

void jme_set_ephemeris_path(const char *path)
{
    jme_context *ctx = jme_get_context();
    jme_set_string(ctx->ephemeris_path, sizeof(ctx->ephemeris_path), path);
}

const char *jme_jpl_file(void)
{
    return jme_get_context()->jpl_file;
}

void jme_set_jpl_file(const char *path)
{
    jme_context *ctx = jme_get_context();
    jme_set_string(ctx->jpl_file, sizeof(ctx->jpl_file), path);
    jme_jpl_close();
}

void jme_set_sidereal_mode(int sidereal_mode, double t0, double ayan_t0)
{
    jme_context *ctx = jme_get_context();
    ctx->sidereal_mode = sidereal_mode;
    ctx->sidereal_t0 = t0;
    ctx->sidereal_ayan_t0 = ayan_t0;
}

void jme_get_sidereal_mode(int *sidereal_mode, double *t0, double *ayan_t0)
{
    jme_context *ctx = jme_get_context();

    if (sidereal_mode != 0) {
        *sidereal_mode = ctx->sidereal_mode;
    }
    if (t0 != 0) {
        *t0 = ctx->sidereal_t0;
    }
    if (ayan_t0 != 0) {
        *ayan_t0 = ctx->sidereal_ayan_t0;
    }
}

void jme_set_topo(double lon, double lat, double altitude)
{
    jme_context *ctx = jme_get_context();
    ctx->topo_lon = lon;
    ctx->topo_lat = lat;
    ctx->topo_alt = altitude;
}

void jme_set_astro_models(const char *models, int flags)
{
    (void)models; (void)flags;
}

int jme_get_astro_models(char *models, int flags)
{
    (void)models; (void)flags;
    return JME_ERR; /* Open Path */
}

void jme_set_lapse_rate(double lapse_rate)
{
    (void)lapse_rate;
}

void jme_set_interpolate_nut(int on)
{
    (void)on;
}

double jme_get_tid_acc(void)
{
    return 0.0;
}

void jme_set_tid_acc(double t_acc)
{
    (void)t_acc;
}

void jme_set_delta_t_userdef(double dt)
{
    (void)dt;
}
