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
    int model_bias;
    int model_nut;
    int model_obl;
    int model_prec;
    int model_sidt;
    int model_deltat;
    double lapse_rate;
    int interpolate_nut;
    double tidal_acceleration;
    int delta_t_userdef_enabled;
    double delta_t_userdef;
} jme_context;

jme_context *jme_get_context(void);
void jme_set_string(char *dst, size_t dst_size, const char *src);
void jme_set_error(char *error, const char *msg);
int jme_context_bias_model(void);
int jme_context_nutation_model(void);
int jme_context_obliquity_model(void);
int jme_context_precession_model(void);
int jme_context_sidereal_time_model(void);
int jme_context_deltat_model(void);
int jme_context_interpolate_nut(void);
int jme_get_frame_bias_matrix(int model, double *m);

#endif
