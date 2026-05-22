#include "jme/jme.h"

#include "context.h"

#include <math.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>

static int token_matches(const char *token, const char *expected)
{
    size_t i = 0;

    if (token == 0 || expected == 0) {
        return 0;
    }

    while (token[i] != '\0' && expected[i] != '\0') {
        char a = token[i];
        char b = expected[i];
        if (a >= 'a' && a <= 'z') { a = (char)(a - 'a' + 'A'); }
        if (b >= 'a' && b <= 'z') { b = (char)(b - 'a' + 'A'); }
        if (a != b) {
            return 0;
        }
        i++;
    }

    return token[i] == '\0' && expected[i] == '\0';
}

static void set_default_models(jme_context *ctx)
{
    ctx->model_bias = JME_MODEL_BIAS_NONE;
    ctx->model_nut = JME_MODEL_NUT_IAU_1980;
    ctx->model_obl = JME_MODEL_OBL_IAU_1980;
    ctx->model_prec = JME_MODEL_PREC_IAU_1976;
    ctx->model_sidt = JME_MODEL_SIDT_IAU_1976;
    ctx->model_deltat = JME_MODEL_DELTAT_ESPENAK_MEEUS_2006;
}

static const char *engine_policy_name(int engine_policy)
{
    switch (engine_policy) {
    case JME_ENGINE_JPL: return "JPL";
    case JME_ENGINE_MOSHIER: return "MOSHIER";
    case JME_ENGINE_VSOP_ELP_MEEUS: return "VSOP_ELP_MEEUS";
    case JME_ENGINE_AUTO:
    default:
        return "AUTO";
    }
}

static void write_model_summary(jme_context *ctx)
{
    snprintf(ctx->astro_models, sizeof(ctx->astro_models),
        "BIAS=%d;NUT=%d;OBL=%d;PREC=%d;SIDT=%d;DELTAT=%d;ENGINE=%s",
        ctx->model_bias,
        ctx->model_nut,
        ctx->model_obl,
        ctx->model_prec,
        ctx->model_sidt,
        ctx->model_deltat,
        engine_policy_name(ctx->engine_policy));
}

static void set_engine_policy(jme_context *ctx, int engine_policy)
{
    ctx->engine_policy = engine_policy;
    ctx->engine_policy_is_explicit = 1;
}

static int token_value_matches(const char *token, const char *prefix, const char *expected)
{
    size_t i = 0;

    if (token == 0 || prefix == 0 || expected == 0) {
        return 0;
    }

    while (prefix[i] != '\0') {
        char a = token[i];
        char b = prefix[i];
        if (a >= 'a' && a <= 'z') { a = (char)(a - 'a' + 'A'); }
        if (b >= 'a' && b <= 'z') { b = (char)(b - 'a' + 'A'); }
        if (a != b) {
            return 0;
        }
        i++;
    }

    return token_matches(token + i, expected);
}

static void apply_model_token(jme_context *ctx, const char *token)
{
    if (token_matches(token, "DEFAULT") || token_matches(token, "IAU1980") || token_matches(token, "IAU1976")) {
        set_default_models(ctx);
    } else if (token_matches(token, "IAU2000")) {
        ctx->model_bias = JME_MODEL_BIAS_IAU2000;
        ctx->model_nut = JME_MODEL_NUT_IAU_2000B;
        ctx->model_obl = JME_MODEL_OBL_IAU_2000;
        ctx->model_prec = JME_MODEL_PREC_IAU_2000;
    } else if (token_matches(token, "IAU2006")) {
        ctx->model_bias = JME_MODEL_BIAS_IAU2006;
        ctx->model_nut = JME_MODEL_NUT_IAU_2000B;
        ctx->model_obl = JME_MODEL_OBL_IAU_2006;
        ctx->model_prec = JME_MODEL_PREC_IAU_2006;
        ctx->model_sidt = JME_MODEL_SIDT_IAU_2006;
    } else if (token_matches(token, "BIASNONE") || token_value_matches(token, "BIAS=", "NONE") || token_value_matches(token, "BIAS=", "332")) {
        ctx->model_bias = JME_MODEL_BIAS_NONE;
    } else if (token_matches(token, "BIAS2000") || token_value_matches(token, "BIAS=", "IAU2000") || token_value_matches(token, "BIAS=", "330")) {
        ctx->model_bias = JME_MODEL_BIAS_IAU2000;
    } else if (token_matches(token, "BIAS2006") || token_value_matches(token, "BIAS=", "IAU2006") || token_value_matches(token, "BIAS=", "331")) {
        ctx->model_bias = JME_MODEL_BIAS_IAU2006;
    } else if (token_matches(token, "NUT1980") || token_value_matches(token, "NUT=", "IAU1980") || token_value_matches(token, "NUT=", "333")) {
        ctx->model_nut = JME_MODEL_NUT_IAU_1980;
    } else if (token_matches(token, "NUT2000B") || token_value_matches(token, "NUT=", "IAU2000B") || token_value_matches(token, "NUT=", "335")) {
        ctx->model_nut = JME_MODEL_NUT_IAU_2000B;
    } else if (token_matches(token, "OBL1980") || token_value_matches(token, "OBL=", "IAU1980") || token_value_matches(token, "OBL=", "336")) {
        ctx->model_obl = JME_MODEL_OBL_IAU_1980;
    } else if (token_matches(token, "OBL2000") || token_value_matches(token, "OBL=", "IAU2000") || token_value_matches(token, "OBL=", "337")) {
        ctx->model_obl = JME_MODEL_OBL_IAU_2000;
    } else if (token_matches(token, "OBL2006") || token_value_matches(token, "OBL=", "IAU2006") || token_value_matches(token, "OBL=", "338")) {
        ctx->model_obl = JME_MODEL_OBL_IAU_2006;
    } else if (token_matches(token, "PREC1976") || token_value_matches(token, "PREC=", "IAU1976") || token_value_matches(token, "PREC=", "339")) {
        ctx->model_prec = JME_MODEL_PREC_IAU_1976;
    } else if (token_matches(token, "PREC2000") || token_value_matches(token, "PREC=", "IAU2000") || token_value_matches(token, "PREC=", "340")) {
        ctx->model_prec = JME_MODEL_PREC_IAU_2000;
    } else if (token_matches(token, "PREC2006") || token_value_matches(token, "PREC=", "IAU2006") || token_value_matches(token, "PREC=", "341")) {
        ctx->model_prec = JME_MODEL_PREC_IAU_2006;
    } else if (token_matches(token, "SIDT1976") || token_value_matches(token, "SIDT=", "IAU1976") || token_value_matches(token, "SIDT=", "344")) {
        ctx->model_sidt = JME_MODEL_SIDT_IAU_1976;
    } else if (token_matches(token, "SIDT2006") || token_value_matches(token, "SIDT=", "IAU2006") || token_value_matches(token, "SIDT=", "345")) {
        ctx->model_sidt = JME_MODEL_SIDT_IAU_2006;
    } else if (token_matches(token, "DELTAT1984") || token_value_matches(token, "DELTAT=", "STEPHENSON_MORRISON_1984") || token_value_matches(token, "DELTAT=", "346")) {
        ctx->model_deltat = JME_MODEL_DELTAT_STEPHENSON_MORRISON_1984;
    } else if (token_matches(token, "DELTAT1997") || token_value_matches(token, "DELTAT=", "STEPHENSON_1997") || token_value_matches(token, "DELTAT=", "347")) {
        ctx->model_deltat = JME_MODEL_DELTAT_STEPHENSON_1997;
    } else if (token_matches(token, "DELTAT2004") || token_value_matches(token, "DELTAT=", "STEPHENSON_MORRISON_2004") || token_value_matches(token, "DELTAT=", "348")) {
        ctx->model_deltat = JME_MODEL_DELTAT_STEPHENSON_MORRISON_2004;
    } else if (token_matches(token, "DELTAT2006") || token_value_matches(token, "DELTAT=", "ESPENAK_MEEUS_2006") || token_value_matches(token, "DELTAT=", "349")) {
        ctx->model_deltat = JME_MODEL_DELTAT_ESPENAK_MEEUS_2006;
    } else if (token_matches(token, "DELTAT2016") || token_value_matches(token, "DELTAT=", "STEPHENSON_ETC_2016") || token_value_matches(token, "DELTAT=", "350")) {
        ctx->model_deltat = JME_MODEL_DELTAT_STEPHENSON_ETC_2016;
    } else if (token_matches(token, "ENGINE_AUTO") || token_value_matches(token, "ENGINE=", "AUTO")) {
        set_engine_policy(ctx, JME_ENGINE_AUTO);
    } else if (token_matches(token, "ENGINE_JPL") || token_value_matches(token, "ENGINE=", "JPL")) {
        set_engine_policy(ctx, JME_ENGINE_JPL);
    } else if (token_matches(token, "ENGINE_MOSHIER") || token_value_matches(token, "ENGINE=", "MOSHIER")) {
        set_engine_policy(ctx, JME_ENGINE_MOSHIER);
    } else if (token_matches(token, "ENGINE_VSOP_ELP_MEEUS")
        || token_matches(token, "ENGINE_ANALYTICAL")
        || token_value_matches(token, "ENGINE=", "VSOP_ELP_MEEUS")
        || token_value_matches(token, "ENGINE=", "ANALYTICAL")) {
        set_engine_policy(ctx, JME_ENGINE_VSOP_ELP_MEEUS);
    }
}

static int engine_policy_from_env(void)
{
    const char *engine = getenv("JME_ENGINE");

    if (engine == 0 || engine[0] == '\0') {
        return JME_ENGINE_AUTO;
    }

    if (token_matches(engine, "JPL")) {
        return JME_ENGINE_JPL;
    }
    if (token_matches(engine, "MOSHIER")) {
        return JME_ENGINE_MOSHIER;
    }
    if (token_matches(engine, "VSOP_ELP_MEEUS") || token_matches(engine, "ANALYTICAL")) {
        return JME_ENGINE_VSOP_ELP_MEEUS;
    }

    return JME_ENGINE_AUTO;
}

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
    ctx->sidereal_t0 = isfinite(t0) ? t0 : 0.0;
    ctx->sidereal_ayan_t0 = isfinite(ayan_t0) ? ayan_t0 : 0.0;
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
    if (!isfinite(lon)) { lon = 0.0; }
    if (!isfinite(lat)) { lat = 0.0; }
    if (!isfinite(altitude)) { altitude = 0.0; }

    ctx->topo_lon = jme_degrees_difference_signed(lon, 0.0);
    if (lat > 90.0) { lat = 90.0; }
    if (lat < -90.0) { lat = -90.0; }
    ctx->topo_lat = lat;
    ctx->topo_alt = altitude;
}

void jme_set_astro_models(const char *models, int flags)
{
    jme_context *ctx = jme_get_context();
    char token[64];
    size_t i = 0;
    size_t j = 0;
    (void)flags;

    if (models == 0 || models[0] == '\0') {
        set_default_models(ctx);
        ctx->engine_policy = engine_policy_from_env();
        ctx->engine_policy_is_explicit = 0;
        write_model_summary(ctx);
        return;
    }

    while (models[i] != '\0') {
        char c = models[i++];
        if (c == ',' || c == ';' || c == ' ' || c == '\t' || c == '\n') {
            if (j > 0) {
                token[j] = '\0';
                apply_model_token(ctx, token);
                j = 0;
            }
        } else if (j + 1 < sizeof(token)) {
            token[j++] = c;
        }
    }

    if (j > 0) {
        token[j] = '\0';
        apply_model_token(ctx, token);
    }

    write_model_summary(ctx);
}

int jme_get_astro_models(char *models, int flags)
{
    jme_context *ctx = jme_get_context();
    (void)flags;
    if (models == 0) {
        return JME_ERR;
    }
    jme_set_string(models, 256, ctx->astro_models);
    return JME_OK;
}

void jme_set_lapse_rate(double lapse_rate)
{
    jme_get_context()->lapse_rate = lapse_rate;
}

void jme_set_interpolate_nut(int on)
{
    jme_get_context()->interpolate_nut = on != 0 ? 1 : 0;
}

double jme_get_tid_acc(void)
{
    return jme_get_context()->tidal_acceleration;
}

void jme_set_tid_acc(double t_acc)
{
    if (isfinite(t_acc)) {
        jme_get_context()->tidal_acceleration = t_acc;
    }
}

void jme_set_delta_t_userdef(double dt)
{
    jme_context *ctx = jme_get_context();

    if (!isfinite(dt)) {
        ctx->delta_t_userdef = 0.0;
        ctx->delta_t_userdef_enabled = 0;
        return;
    }

    ctx->delta_t_userdef = dt;
    ctx->delta_t_userdef_enabled = 1;
}
