#ifndef JME_MOSHIER_PLANTBL_DEFINED
#define JME_MOSHIER_PLANTBL_DEFINED

// define "far" if require by your compiler
#ifdef _MSC_VER
#define FAR
#else
#define FAR
#endif

#ifndef SIGNED
#define SIGNED signed
#endif

struct plantbl {
  char maxargs;
  char max_harmonic[18];
  char max_power_of_t;
  signed char FAR *arg_tbl;
  void FAR *lon_tbl;
  void FAR *lat_tbl;
  void FAR *rad_tbl;
  double distance;
  double timescale;
  double trunclvl;
};

#define J2000 2451545.0
#define STR 4.8481368110953599359e-6

#endif
