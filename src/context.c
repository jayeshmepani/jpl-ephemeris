#include "context.h"

#include <string.h>

static jme_context g_context = {
    "",
    "",
    0,
    0.0,
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
