#ifndef JME_CONTEXT_H
#define JME_CONTEXT_H

#include <stddef.h>

#define JME_PATH_MAX 1024

/* Derived from exact SI constants; kept internal to avoid changing the FFI constant inventory. */
#define JME_C_AU_PER_DAY ((JME_SPEED_OF_LIGHT_KM_PER_SEC * JME_SECONDS_PER_DAY) / JME_AU_KM)

typedef enum jme_engine_policy {
    JME_ENGINE_AUTO = 0,
    JME_ENGINE_JPL = 1,
    JME_ENGINE_MOSHIER = 2,
    JME_ENGINE_VSOP_ELP_MEEUS = 3
} jme_engine_policy;

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
    int engine_policy;
    int engine_policy_is_explicit;
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
int jme_context_engine_policy(void);
int jme_context_interpolate_nut(void);
int jme_get_frame_bias_matrix(int model, double *m);
int jme_get_topo_pos_true_equator(double jd_et, double *pos_au, char *error);

/* Internal helper shared by the main and fixed-star apparent-position paths. */
int internal_earth_aberration_state(double jd_et, double *state, char *error);
int internal_ut_to_et(double jd_ut, double *jd_et, char *error);
int internal_utc_to_tt_split(
    int year, int month, int day, int hour, int minute, double second, int calendar,
    double *jd0_tt, double *fraction_tt, char *error
);
int internal_tt_to_tdb_split(double jd0_tt, double fraction_tt, double *jd0_tdb, double *fraction_tdb, char *error);
int internal_tdb_to_tcb_split(double jd0_tdb, double fraction_tdb, double *jd0_tcb, double *fraction_tcb, char *error);
int internal_tt_to_jpl_time_split(double jd_tt, double *jd0_native, double *fraction_native, char *error);
int internal_tt_split_to_jpl_time_split(
    double jd0_tt, double fraction_tt, double *jd0_native, double *fraction_native, char *error
);
int internal_jpl_body_state_tt(
    double jd_tt, int target_body, int center_body, int unit, double *state, char *error
);
double jme_erfa_dtdb(double date1, double date2, double ut, double elong, double u, double v);
int jme_erfa_tdbtcb(double tdb1, double tdb2, double *tcb1, double *tcb2);

#endif
