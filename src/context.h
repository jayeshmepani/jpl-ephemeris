#ifndef JME_CONTEXT_H
#define JME_CONTEXT_H

#include <stddef.h>

#define JME_PATH_MAX 1024

typedef struct jme_context {
    char ephemeris_path[JME_PATH_MAX];
    char jpl_file[JME_PATH_MAX];
    int sidereal_mode;
    double sidereal_t0;
    double sidereal_ayan_t0;
    double topo_lon;
    double topo_lat;
    double topo_alt;
    char astro_models[256];
    double lapse_rate;
    int interpolate_nut;
    double tidal_acceleration;
    int delta_t_userdef_enabled;
    double delta_t_userdef;
} jme_context;

jme_context *jme_get_context(void);
void jme_set_string(char *dst, size_t dst_size, const char *src);
void jme_set_error(char *error, const char *msg);

#endif
