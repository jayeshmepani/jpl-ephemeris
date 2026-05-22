#include "context.h"
#include "jme/jme.h"

#include <stdlib.h>
#include <string.h>

static jme_context g_context = {
    "",
    "",
    0,
    0.0,
    0.0,
    0.0,
    0.0,
    0.0,
    "",
    JME_MODEL_BIAS_NONE,
    JME_MODEL_NUT_IAU_1980,
    JME_MODEL_OBL_IAU_1980,
    JME_MODEL_PREC_IAU_1976,
    JME_MODEL_SIDT_IAU_1976,
    JME_MODEL_DELTAT_ESPENAK_MEEUS_2006,
    JME_ENGINE_AUTO,
    0,
    0.0065,
    0,
    0.0,
    0,
    0.0
};

jme_context *jme_get_context(void)
{
    return &g_context;
}

void jme_set_string(char *dst, size_t dst_size, const char *src)
{
    size_t i = 0;

    if (dst == 0 || dst_size == 0) {
        return;
    }

    if (src == 0) {
        dst[0] = '\0';
        return;
    }

    while (i + 1 < dst_size && src[i] != '\0') {
        dst[i] = src[i];
        i++;
    }

    dst[i] = '\0';
}

void jme_set_error(char *error, const char *msg)
{
    if (error == 0) {
        return;
    }

    jme_set_string(error, 256, msg);
}

int jme_context_bias_model(void)
{
    return g_context.model_bias;
}

int jme_context_nutation_model(void)
{
    return g_context.model_nut;
}

int jme_context_obliquity_model(void)
{
    return g_context.model_obl;
}

int jme_context_precession_model(void)
{
    return g_context.model_prec;
}

int jme_context_sidereal_time_model(void)
{
    return g_context.model_sidt;
}

int jme_context_deltat_model(void)
{
    return g_context.model_deltat;
}

int jme_context_engine_policy(void)
{
    const char *engine;

    if (g_context.engine_policy_is_explicit) {
        return g_context.engine_policy;
    }

    engine = getenv("JME_ENGINE");
    if (engine != 0 && engine[0] != '\0') {
        if (strcmp(engine, "JPL") == 0 || strcmp(engine, "jpl") == 0) {
            return JME_ENGINE_JPL;
        }
        if (strcmp(engine, "MOSHIER") == 0 || strcmp(engine, "moshier") == 0) {
            return JME_ENGINE_MOSHIER;
        }
        if (strcmp(engine, "VSOP_ELP_MEEUS") == 0
            || strcmp(engine, "vsop_elp_meeus") == 0
            || strcmp(engine, "ANALYTICAL") == 0
            || strcmp(engine, "analytical") == 0) {
            return JME_ENGINE_VSOP_ELP_MEEUS;
        }
    }

    return g_context.engine_policy;
}

int jme_context_interpolate_nut(void)
{
    return g_context.interpolate_nut;
}
