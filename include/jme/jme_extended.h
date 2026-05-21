#ifndef JME_EXTENDED_H
#define JME_EXTENDED_H

#ifdef __cplusplus
extern "C" {
#endif

typedef enum jme_extended_constant {
    JME_ANGLE_FORMAT_KEEP_DEG = 0,
    JME_ANGLE_FORMAT_KEEP_SIGN = 1,
    JME_ANGLE_FORMAT_NAKSHATRA = 2,
    JME_ANGLE_FORMAT_ROUND_DEG = 3,
    JME_ANGLE_FORMAT_ROUND_MIN = 4,
    JME_ANGLE_FORMAT_ROUND_SEC = 5,
    JME_ANGLE_FORMAT_ZODIACAL = 6,
    JME_BODY_ALTJIRA = 7,
    JME_BODY_AMYCUS = 8,
    JME_BODY_ARROKOTH = 9,
    JME_BODY_ASBOLUS = 10,
    JME_BODY_ASTEROID_001 = 11,
    JME_BODY_ASTEROID_002 = 12,
    JME_BODY_ASTEROID_003 = 13,
    JME_BODY_ASTEROID_004 = 14,
    JME_BODY_ASTEROID_005 = 15,
    JME_BODY_ASTEROID_006 = 16,
    JME_BODY_ASTEROID_007 = 17,
    JME_BODY_ASTEROID_008 = 18,
    JME_BODY_ASTEROID_009 = 19,
    JME_BODY_ASTEROID_010 = 20,
    JME_BODY_ASTEROID_011 = 21,
    JME_BODY_ASTEROID_012 = 22,
    JME_BODY_ASTEROID_013 = 23,
    JME_BODY_ASTEROID_014 = 24,
    JME_BODY_ASTEROID_015 = 25,
    JME_BODY_ASTEROID_016 = 26,
    JME_BODY_ASTEROID_017 = 27,
    JME_BODY_ASTEROID_018 = 28,
    JME_BODY_ASTEROID_019 = 29,
    JME_BODY_ASTEROID_020 = 30,
    JME_BODY_ASTEROID_021 = 31,
    JME_BODY_ASTEROID_022 = 32,
    JME_BODY_ASTEROID_023 = 33,
    JME_BODY_ASTEROID_024 = 34,
    JME_BODY_ASTEROID_025 = 35,
    JME_BODY_ASTEROID_026 = 36,
    JME_BODY_ASTEROID_027 = 37,
    JME_BODY_ASTEROID_028 = 38,
    JME_BODY_ASTEROID_029 = 39,
    JME_BODY_ASTEROID_030 = 40,
    JME_BODY_ASTEROID_031 = 41,
    JME_BODY_ASTEROID_032 = 42,
    JME_BODY_ASTEROID_033 = 43,
    JME_BODY_ASTEROID_034 = 44,
    JME_BODY_ASTEROID_035 = 45,
    JME_BODY_ASTEROID_036 = 46,
    JME_BODY_ASTEROID_037 = 47,
    JME_BODY_ASTEROID_038 = 48,
    JME_BODY_ASTEROID_039 = 49,
    JME_BODY_ASTEROID_040 = 50,
    JME_BODY_ASTEROID_041 = 51,
    JME_BODY_ASTEROID_042 = 52,
    JME_BODY_ASTEROID_043 = 53,
    JME_BODY_ASTEROID_044 = 54,
    JME_BODY_ASTEROID_045 = 55,
    JME_BODY_ASTEROID_046 = 56,
    JME_BODY_ASTEROID_047 = 57,
    JME_BODY_ASTEROID_048 = 58,
    JME_BODY_ASTEROID_049 = 59,
    JME_BODY_ASTEROID_050 = 60,
    JME_BODY_ASTEROID_051 = 61,
    JME_BODY_ASTEROID_052 = 62,
    JME_BODY_ASTEROID_053 = 63,
    JME_BODY_ASTEROID_054 = 64,
    JME_BODY_ASTEROID_055 = 65,
    JME_BODY_ASTEROID_056 = 66,
    JME_BODY_ASTEROID_057 = 67,
    JME_BODY_ASTEROID_058 = 68,
    JME_BODY_ASTEROID_059 = 69,
    JME_BODY_ASTEROID_060 = 70,
    JME_BODY_ASTEROID_061 = 71,
    JME_BODY_ASTEROID_062 = 72,
    JME_BODY_ASTEROID_063 = 73,
    JME_BODY_ASTEROID_064 = 74,
    JME_BODY_ASTEROID_065 = 75,
    JME_BODY_ASTEROID_066 = 76,
    JME_BODY_ASTEROID_067 = 77,
    JME_BODY_ASTEROID_068 = 78,
    JME_BODY_ASTEROID_069 = 79,
    JME_BODY_ASTEROID_070 = 80,
    JME_BODY_ASTEROID_071 = 81,
    JME_BODY_ASTEROID_072 = 82,
    JME_BODY_ASTEROID_073 = 83,
    JME_BODY_ASTEROID_074 = 84,
    JME_BODY_ASTEROID_075 = 85,
    JME_BODY_ASTEROID_076 = 86,
    JME_BODY_ASTEROID_077 = 87,
    JME_BODY_ASTEROID_078 = 88,
    JME_BODY_ASTEROID_079 = 89,
    JME_BODY_ASTEROID_080 = 90,
    JME_BODY_ASTEROID_081 = 91,
    JME_BODY_ASTEROID_082 = 92,
    JME_BODY_ASTEROID_083 = 93,
    JME_BODY_ASTEROID_084 = 94,
    JME_BODY_ASTEROID_085 = 95,
    JME_BODY_ASTEROID_086 = 96,
    JME_BODY_ASTEROID_087 = 97,
    JME_BODY_ASTEROID_088 = 98,
    JME_BODY_ASTEROID_089 = 99,
    JME_BODY_ASTEROID_090 = 100,
    JME_BODY_ASTEROID_091 = 101,
    JME_BODY_ASTEROID_092 = 102,
    JME_BODY_ASTEROID_093 = 103,
    JME_BODY_ASTEROID_094 = 104,
    JME_BODY_ASTEROID_095 = 105,
    JME_BODY_ASTEROID_096 = 106,
    JME_BODY_ASTEROID_097 = 107,
    JME_BODY_ASTEROID_098 = 108,
    JME_BODY_ASTEROID_099 = 109,
    JME_BODY_ASTEROID_100 = 110,
    JME_BODY_ASTEROID_101 = 111,
    JME_BODY_ASTEROID_102 = 112,
    JME_BODY_ASTEROID_103 = 113,
    JME_BODY_ASTEROID_104 = 114,
    JME_BODY_ASTEROID_105 = 115,
    JME_BODY_ASTEROID_106 = 116,
    JME_BODY_ASTEROID_107 = 117,
    JME_BODY_ASTEROID_108 = 118,
    JME_BODY_ASTEROID_109 = 119,
    JME_BODY_ASTEROID_110 = 120,
    JME_BODY_BIENOR = 121,
    JME_BODY_BORASISI = 122,
    JME_BODY_CERES = 123,
    JME_BODY_CHARIKLO = 124,
    JME_BODY_CHIRON = 125,
    JME_BODY_COMET_001 = 126,
    JME_BODY_COMET_002 = 127,
    JME_BODY_COMET_003 = 128,
    JME_BODY_COMET_004 = 129,
    JME_BODY_COMET_005 = 130,
    JME_BODY_COMET_006 = 131,
    JME_BODY_COMET_007 = 132,
    JME_BODY_COMET_008 = 133,
    JME_BODY_COMET_009 = 134,
    JME_BODY_COMET_010 = 135,
    JME_BODY_COMET_011 = 136,
    JME_BODY_COMET_012 = 137,
    JME_BODY_COMET_013 = 138,
    JME_BODY_COMET_014 = 139,
    JME_BODY_COMET_015 = 140,
    JME_BODY_COMET_016 = 141,
    JME_BODY_COMET_017 = 142,
    JME_BODY_COMET_018 = 143,
    JME_BODY_COMET_019 = 144,
    JME_BODY_COMET_020 = 145,
    JME_BODY_COMET_021 = 146,
    JME_BODY_COMET_022 = 147,
    JME_BODY_COMET_023 = 148,
    JME_BODY_COMET_024 = 149,
    JME_BODY_COMET_025 = 150,
    JME_BODY_COMET_026 = 151,
    JME_BODY_COMET_027 = 152,
    JME_BODY_COMET_028 = 153,
    JME_BODY_COMET_029 = 154,
    JME_BODY_COMET_030 = 155,
    JME_BODY_COMET_031 = 156,
    JME_BODY_COMET_032 = 157,
    JME_BODY_COMET_033 = 158,
    JME_BODY_COMET_034 = 159,
    JME_BODY_COMET_035 = 160,
    JME_BODY_COMET_036 = 161,
    JME_BODY_COMET_037 = 162,
    JME_BODY_COMET_038 = 163,
    JME_BODY_COMET_039 = 164,
    JME_BODY_COMET_040 = 165,
    JME_BODY_COMET_041 = 166,
    JME_BODY_COMET_042 = 167,
    JME_BODY_COMET_043 = 168,
    JME_BODY_COMET_044 = 169,
    JME_BODY_COMET_045 = 170,
    JME_BODY_COMET_046 = 171,
    JME_BODY_COMET_047 = 172,
    JME_BODY_COMET_048 = 173,
    JME_BODY_COMET_049 = 174,
    JME_BODY_COMET_050 = 175,
    JME_BODY_COMET_051 = 176,
    JME_BODY_COMET_052 = 177,
    JME_BODY_COMET_053 = 178,
    JME_BODY_COMET_054 = 179,
    JME_BODY_COMET_055 = 180,
    JME_BODY_COMET_056 = 181,
    JME_BODY_COMET_057 = 182,
    JME_BODY_COMET_058 = 183,
    JME_BODY_COMET_059 = 184,
    JME_BODY_COMET_060 = 185,
    JME_BODY_CRANTOR = 186,
    JME_BODY_CYLLARUS = 187,
    JME_BODY_DEUCALION = 188,
    JME_BODY_ECHECLUS = 189,
    JME_BODY_ELATUS = 190,
    JME_BODY_ERIS = 191,
    JME_BODY_GONGGONG = 192,
    JME_BODY_HAUMEA = 193,
    JME_BODY_HUYA = 194,
    JME_BODY_HYLONOME = 195,
    JME_BODY_IXION = 196,
    JME_BODY_JUNO = 197,
    JME_BODY_MAKEMAKE = 198,
    JME_BODY_MINOR_PLANET_001 = 199,
    JME_BODY_MINOR_PLANET_002 = 200,
    JME_BODY_MINOR_PLANET_003 = 201,
    JME_BODY_MINOR_PLANET_004 = 202,
    JME_BODY_MINOR_PLANET_005 = 203,
    JME_BODY_MINOR_PLANET_006 = 204,
    JME_BODY_MINOR_PLANET_007 = 205,
    JME_BODY_MINOR_PLANET_008 = 206,
    JME_BODY_MINOR_PLANET_009 = 207,
    JME_BODY_MINOR_PLANET_010 = 208,
    JME_BODY_MINOR_PLANET_011 = 209,
    JME_BODY_MINOR_PLANET_012 = 210,
    JME_BODY_MINOR_PLANET_013 = 211,
    JME_BODY_MINOR_PLANET_014 = 212,
    JME_BODY_MINOR_PLANET_015 = 213,
    JME_BODY_MINOR_PLANET_016 = 214,
    JME_BODY_MINOR_PLANET_017 = 215,
    JME_BODY_MINOR_PLANET_018 = 216,
    JME_BODY_MINOR_PLANET_019 = 217,
    JME_BODY_MINOR_PLANET_020 = 218,
    JME_BODY_MINOR_PLANET_021 = 219,
    JME_BODY_MINOR_PLANET_022 = 220,
    JME_BODY_MINOR_PLANET_023 = 221,
    JME_BODY_MINOR_PLANET_024 = 222,
    JME_BODY_MINOR_PLANET_025 = 223,
    JME_BODY_MINOR_PLANET_026 = 224,
    JME_BODY_MINOR_PLANET_027 = 225,
    JME_BODY_MINOR_PLANET_028 = 226,
    JME_BODY_MINOR_PLANET_029 = 227,
    JME_BODY_MINOR_PLANET_030 = 228,
    JME_BODY_MINOR_PLANET_031 = 229,
    JME_BODY_MINOR_PLANET_032 = 230,
    JME_BODY_MINOR_PLANET_033 = 231,
    JME_BODY_MINOR_PLANET_034 = 232,
    JME_BODY_MINOR_PLANET_035 = 233,
    JME_BODY_MINOR_PLANET_036 = 234,
    JME_BODY_MINOR_PLANET_037 = 235,
    JME_BODY_MINOR_PLANET_038 = 236,
    JME_BODY_MINOR_PLANET_039 = 237,
    JME_BODY_MINOR_PLANET_040 = 238,
    JME_BODY_MINOR_PLANET_041 = 239,
    JME_BODY_MINOR_PLANET_042 = 240,
    JME_BODY_MINOR_PLANET_043 = 241,
    JME_BODY_MINOR_PLANET_044 = 242,
    JME_BODY_MINOR_PLANET_045 = 243,
    JME_BODY_MINOR_PLANET_046 = 244,
    JME_BODY_MINOR_PLANET_047 = 245,
    JME_BODY_MINOR_PLANET_048 = 246,
    JME_BODY_MINOR_PLANET_049 = 247,
    JME_BODY_MINOR_PLANET_050 = 248,
    JME_BODY_MINOR_PLANET_051 = 249,
    JME_BODY_MINOR_PLANET_052 = 250,
    JME_BODY_MINOR_PLANET_053 = 251,
    JME_BODY_MINOR_PLANET_054 = 252,
    JME_BODY_MINOR_PLANET_055 = 253,
    JME_BODY_MINOR_PLANET_056 = 254,
    JME_BODY_MINOR_PLANET_057 = 255,
    JME_BODY_MINOR_PLANET_058 = 256,
    JME_BODY_MINOR_PLANET_059 = 257,
    JME_BODY_MINOR_PLANET_060 = 258,
    JME_BODY_NESSUS = 259,
    JME_BODY_OKYRHOE = 260,
    JME_BODY_ORCUS = 261,
    JME_BODY_PALLAS = 262,
    JME_BODY_PELOPS = 263,
    JME_BODY_PHOLUS = 264,
    JME_BODY_QUAOAR = 265,
    JME_BODY_RHADAMANTHUS = 266,
    JME_BODY_SALACIA = 267,
    JME_BODY_SEDNA = 268,
    JME_BODY_THEREUS = 269,
    JME_BODY_TYTHONUS = 270,
    JME_BODY_VARUNA = 271,
    JME_BODY_VESTA = 272,
    JME_CALC_APPARENT_POSITION = 0,
    JME_CALC_ASTROMETRIC = (1 << 10) | (1 << 11),
    JME_CALC_CENTER_BODY = 1 << 12,
    JME_CALC_DISTANCE_AU = 0,
    JME_CALC_DISTANCE_KM = 1 << 13,
    JME_CALC_HIGH_PRECISION = 1 << 14,
    JME_CALC_ICRS = 1 << 15,
    JME_CALC_NO_ABERRATION = 1 << 10,
    JME_CALC_NO_LIGHT_DEFLECTION = 1 << 11,
    JME_CALC_RAW_VECTOR = 1 << 2,
    JME_CALC_RECTANGULAR = 1 << 2,
    JME_CALC_SPHERICAL = 0,
    JME_CALC_STRICT = 1 << 16,
    JME_CALC_TOPOCENTRIC = 1 << 17,
    JME_CALC_VELOCITY_PER_DAY = 0,
    JME_CALC_VELOCITY_PER_SECOND = 1 << 18,
    JME_COORD_APPARENT_TO_TRUE = 289,
    JME_COORD_ECLIPTIC_TO_HORIZONTAL = 290,
    JME_COORD_EQUATORIAL_TO_HORIZONTAL = 291,
    JME_COORD_HORIZONTAL_TO_ECLIPTIC = 292,
    JME_COORD_HORIZONTAL_TO_EQUATORIAL = 293,
    JME_COORD_TRUE_TO_APPARENT = 294,
    JME_ECLIPSE_FIRST_CONTACT = 295,
    JME_ECLIPSE_FOURTH_CONTACT = 296,
    JME_ECLIPSE_LUNAR_PARTIAL = 297,
    JME_ECLIPSE_LUNAR_PENUMBRAL = 298,
    JME_ECLIPSE_LUNAR_TOTAL = 299,
    JME_ECLIPSE_MAX_VISIBLE = 300,
    JME_ECLIPSE_PENUMBRAL_BEGIN = 301,
    JME_ECLIPSE_PENUMBRAL_END = 302,
    JME_ECLIPSE_SECOND_CONTACT = 303,
    JME_ECLIPSE_SOLAR_ANNULAR = 304,
    JME_ECLIPSE_SOLAR_CENTRAL = 305,
    JME_ECLIPSE_SOLAR_HYBRID = 306,
    JME_ECLIPSE_SOLAR_NONCENTRAL = 307,
    JME_ECLIPSE_SOLAR_PARTIAL = 308,
    JME_ECLIPSE_SOLAR_TOTAL = 309,
    JME_ECLIPSE_THIRD_CONTACT = 310,
    JME_ECLIPSE_VISIBLE = 311,
    JME_HOUSE_ALCABITIUS = 312,
    JME_HOUSE_APC = 313,
    JME_HOUSE_AZIMUTHAL = 314,
    JME_HOUSE_CAMPANUS = 315,
    JME_HOUSE_EQUAL = 316,
    JME_HOUSE_GAUQUELIN = 317,
    JME_HOUSE_HORIZONTAL = 318,
    JME_HOUSE_KOCH = 319,
    JME_HOUSE_KRUSINSKI = 320,
    JME_HOUSE_MERIDIAN = 321,
    JME_HOUSE_MORINUS = 322,
    JME_HOUSE_PLACIDUS = 323,
    JME_HOUSE_POLICH_PAGE = 324,
    JME_HOUSE_PORPHYRIUS = 325,
    JME_HOUSE_REGIOMONTANUS = 326,
    JME_HOUSE_SUNSHINE = 327,
    JME_HOUSE_VEHLOW_EQUAL = 328,
    JME_HOUSE_WHOLE_SIGN = 329,
    JME_MODEL_BIAS_IAU2000 = 330,
    JME_MODEL_BIAS_IAU2006 = 331,
    JME_MODEL_BIAS_NONE = 332,
    JME_MODEL_NUT_IAU_1980 = 333,
    JME_MODEL_NUT_IAU_2000A = 334,
    JME_MODEL_NUT_IAU_2000B = 335,
    JME_MODEL_OBL_IAU_1980 = 336,
    JME_MODEL_OBL_IAU_2000 = 337,
    JME_MODEL_OBL_IAU_2006 = 338,
    JME_MODEL_PREC_IAU_1976 = 339,
    JME_MODEL_PREC_IAU_2000 = 340,
    JME_MODEL_PREC_IAU_2006 = 341,
    JME_MODEL_PREC_LASKAR_1986 = 342,
    JME_MODEL_PREC_VONDRAK_2011 = 343,
    JME_MODEL_SIDT_IAU_1976 = 344,
    JME_MODEL_SIDT_IAU_2006 = 345,
    JME_MODEL_DELTAT_STEPHENSON_MORRISON_1984 = 346,
    JME_MODEL_DELTAT_STEPHENSON_1997 = 347,
    JME_MODEL_DELTAT_STEPHENSON_MORRISON_2004 = 348,
    JME_MODEL_DELTAT_ESPENAK_MEEUS_2006 = 349,
    JME_MODEL_DELTAT_STEPHENSON_ETC_2016 = 350,
    JME_RISE_ANTI_MERIDIAN_TRANSIT = 1 << 3,
    JME_RISE_ASTRONOMICAL_TWILIGHT = 1 << 9,
    JME_RISE_CIVIL_TWILIGHT = 1 << 7,
    JME_RISE_DISC_BOTTOM = 1 << 11,
    JME_RISE_DISC_CENTER = 1 << 8,
    JME_RISE_FIXED_DISC_SIZE = 1 << 10,
    JME_RISE_HINDU_RISING = 1 << 12,
    JME_RISE_MERIDIAN_TRANSIT = 1 << 2,
    JME_RISE_NAUTICAL_TWILIGHT = 1 << 6,
    JME_RISE_NO_REFRACTION = 1 << 13,
    JME_RISE_RISE = 1 << 0,
    JME_RISE_SET = 1 << 1,
    JME_SIDEREAL_ALDEBARAN_15TAU = 363,
    JME_SIDEREAL_ARYABHATA = 364,
    JME_SIDEREAL_B1950 = 365,
    JME_SIDEREAL_BABYL_ETPSC = 366,
    JME_SIDEREAL_BABYL_HUBER = 367,
    JME_SIDEREAL_BABYL_KUGLER1 = 368,
    JME_SIDEREAL_BABYL_KUGLER2 = 369,
    JME_SIDEREAL_BABYL_KUGLER3 = 370,
    JME_SIDEREAL_DELUCE = 371,
    JME_SIDEREAL_GALCENT_0SAG = 372,
    JME_SIDEREAL_HIPPARCHOS = 373,
    JME_SIDEREAL_J1900 = 374,
    JME_SIDEREAL_J2000 = 375,
    JME_SIDEREAL_JN_BHASIN = 376,
    JME_SIDEREAL_KRISHNAMURTI = 377,
    JME_SIDEREAL_RAMAN = 378,
    JME_SIDEREAL_SASSANIAN = 379,
    JME_SIDEREAL_SS_CITRA = 380,
    JME_SIDEREAL_SS_REVATI = 381,
    JME_SIDEREAL_SURYASIDDHANTA = 382,
    JME_SIDEREAL_TRUE_CITRA = 383,
    JME_SIDEREAL_TRUE_MULA = 384,
    JME_SIDEREAL_TRUE_PUSHYA = 385,
    JME_SIDEREAL_TRUE_REVATI = 386,
    JME_SIDEREAL_USHASHASHI = 387,
    JME_SIDEREAL_YUKTESHWAR = 388,
    JME_TIME_DELTAT_AUTOMATIC = 389,
    JME_TIME_TIDAL_AUTOMATIC = 390,
    JME_TIME_TIDAL_DE200 = 391,
    JME_TIME_TIDAL_DE403 = 392,
    JME_TIME_TIDAL_DE404 = 393,
    JME_TIME_TIDAL_DE405 = 394,
    JME_TIME_TIDAL_DE406 = 395,
    JME_TIME_TIDAL_DE421 = 396,
    JME_TIME_TIDAL_DE430 = 397,
    JME_TIME_TIDAL_DE431 = 398,
    JME_TIME_TIDAL_DE441 = 399,
    JME_VERSION_ID = 400,
    JME_MODEL_REVISED_IAU_2000 = 401,
    JME_MODEL_REVISED_IAU_2006 = 402,
    JME_MODEL_REVISED_PREC_LASKAR = 403,
    JME_MODEL_REVISED_PREC_VONDRAK = 404,
    JME_MODEL_REVISED_PREC_LIESKE = 405
} jme_extended_constant;

double jme_delta_t(double jd_ut);

double jme_delta_t_ex(double jd_ut, int model, char *error);

int jme_get_obliquity(double jd_et, int model, double *eps, char *error);

int jme_get_frame_bias_matrix(int model, double *m);

int jme_get_nutation(double jd_et, int model, double *dpsi, double *deps, char *error);

int jme_get_precession_matrix(double jd_start, double jd_end, int model, double *m);

void jme_get_nutation_matrix(double dpsi_rad, double deps_rad, double eps_rad, double *m);

int jme_get_topo_pos(double jd_et, double *pos_au, char *error);

double jme_get_ayanamsa(double jd_et);
double jme_get_ayanamsa_ut(double jd_ut);
int jme_get_ayanamsa_ex(double jd_et, int model, double *ayan, char *error);
int jme_get_ayanamsa_ex_ut(double jd_ut, int model, double *ayan, char *error);

int jme_time_equ(double jd_ut, double *e, char *error);

void jme_matrix_identity(double *m);
void jme_matrix_multiply(const double *a, const double *b, double *out);
void jme_matrix_rotate_x(double angle_rad, double *m);
void jme_matrix_rotate_y(double angle_rad, double *m);
void jme_matrix_rotate_z(double angle_rad, double *m);
void jme_matrix_transform_state(const double *m, const double *input, double *output);

int jme_body_id_from_name(const char *name);

const char *jme_body_name(int body);

char *jme_copy_body_name(int body, char *buffer);

int jme_body_naif_id(int body);

int jme_centiseconds_difference(int p1, int p2);

int jme_centiseconds_difference_signed(int p1, int p2);

int jme_centiseconds_normalize(int p);

int jme_centiseconds_round_second(int x);

double jme_degree_midpoint(double x1, double x0);

double jme_degree_normalize(double x);

double jme_degrees_to_hours(double degrees);

double jme_degrees_to_radians(double degrees);

double jme_degrees_difference(double p1, double p2);

double jme_degrees_difference_signed(double p1, double p2);

int jme_double_to_long(double x);

void jme_ecliptic_to_equatorial(
    double lon,
    double lat,
    double eps,
    double *ra,
    double *dec
);

void jme_equatorial_to_ecliptic(
    double ra,
    double dec,
    double eps,
    double *lon,
    double *lat
);

int jme_ecliptic_to_equatorial_rectangular_state(
    const double *ecliptic,
    double eps,
    double *equatorial
);

int jme_equatorial_to_ecliptic_rectangular_state(
    const double *equatorial,
    double eps,
    double *ecliptic
);

void jme_equatorial_to_horizontal(
    double hour_angle,
    double dec,
    double geo_lat,
    double *azimuth,
    double *altitude
);

void jme_horizontal_to_equatorial(
    double azimuth,
    double altitude,
    double geo_lat,
    double *hour_angle,
    double *dec
);

double jme_spherical_angular_separation(double lon1, double lat1, double lon2, double lat2);

double jme_spherical_position_angle(double lon1, double lat1, double lon2, double lat2);

int jme_spherical_to_rectangular_state(const double *spherical, double *rectangular);

void jme_jd_to_utc(
    double jd,
    int calendar,
    int *year,
    int *month,
    int *day,
    int *hour,
    int *minute,
    double *second
);

double jme_jd_add_seconds(double jd, double seconds);

double jme_jd_difference_seconds(double jd_end, double jd_start);

int jme_jpl_body_state(
    double jd_time,
    int target_body,
    int center_body,
    int unit,
    double *state,
    char *error
);

int jme_jpl_body_state_split(
    double jd0,
    double time_offset,
    int target_body,
    int center_body,
    int unit,
    double *state,
    char *error
);

int jme_jpl_ecliptic_state(
    double jd_time,
    int target_body,
    int center_body,
    int unit,
    double *state,
    char *error
);

int jme_jpl_body_state_naif(
    double jd_time,
    int target_naif,
    int center_naif,
    int unit,
    double *state,
    char *error
);

int jme_jpl_body_state_split_naif(
    double jd0,
    double time_offset,
    int target_naif,
    int center_naif,
    int unit,
    double *state,
    char *error
);

int jme_jpl_body_state_native_naif(
    double jd_time,
    int target_naif,
    int center_naif,
    double *state,
    char *error
);

int jme_jpl_body_state_native(
    double jd_time,
    int target_body,
    int center_body,
    double *state,
    char *error
);

int jme_jpl_body_state_native_split_naif(
    double jd0,
    double time_offset,
    int target_naif,
    int center_naif,
    double *state,
    char *error
);

int jme_jpl_body_state_native_split(
    double jd0,
    double time_offset,
    int target_body,
    int center_body,
    double *state,
    char *error
);

int jme_jpl_body_state_order_naif(
    double jd0,
    double time_offset,
    int target_naif,
    int center_naif,
    int unit,
    int order,
    double *state,
    char *error
);

int jme_jpl_body_state_order(
    double jd0,
    double time_offset,
    int target_body,
    int center_body,
    int unit,
    int order,
    double *state,
    char *error
);

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
);

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
);

int jme_jpl_ecliptic_state_naif(
    double jd_time,
    int target_naif,
    int center_naif,
    int unit,
    double *state,
    char *error
);

int jme_jpl_ecliptic_state_split(
    double jd0,
    double time_offset,
    int target_body,
    int center_body,
    int unit,
    double *state,
    char *error
);

int jme_jpl_ecliptic_state_split_naif(
    double jd0,
    double time_offset,
    int target_naif,
    int center_naif,
    int unit,
    double *state,
    char *error
);

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
);

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
);

void jme_jpl_close(void);

int jme_jpl_coverage(double *first_time, double *last_time, int *continuous, char *error);

int jme_jpl_constant(const char *name, double *value, char *error);

int jme_jpl_constant_count(char *error);

int jme_jpl_constant_index(int index, char *name, unsigned int name_size, double *value, char *error);

int jme_jpl_constant_vector(const char *name, double *values, int value_count, char *error);

int jme_jpl_constant_string(const char *name, char *value, unsigned int value_size, char *error);

int jme_jpl_constant_string_vector(
    const char *name,
    char *values,
    unsigned int single_value_size,
    int value_count,
    char *error
);

const char *jme_jpl_engine_version(char *buffer, size_t buffer_size);

int jme_jpl_file_version(char *buffer, unsigned int buffer_size, char *error);

int jme_jpl_current_file_data(
    char *path,
    unsigned int path_size,
    double *first_time,
    double *last_time,
    int *continuous,
    char *error
);

int jme_jpl_is_available(void);

int jme_jpl_is_open(void);

int jme_jpl_open(const char *path, char *error);

int jme_jpl_open_array(int path_count, const char *const *paths, char *error);

int jme_jpl_prefetch(char *error);

int jme_jpl_is_thread_safe(char *error);

int jme_jpl_id_by_name(const char *name, int *id, char *error);

int jme_jpl_name_by_id(int id, char *name, unsigned int name_size, char *error);

int jme_jpl_max_supported_order(int segment_type);

int jme_jpl_position_record_count(char *error);

int jme_jpl_position_record_index(
    int index,
    int *target,
    int *center,
    double *first_time,
    double *last_time,
    int *frame,
    int *segment_type,
    char *error
);

int jme_jpl_orientation_state_naif(
    double jd_time,
    int target_naif,
    int unit,
    double *state,
    char *error
);

int jme_jpl_orientation_state_split_naif(
    double jd0,
    double time_offset,
    int target_naif,
    int unit,
    double *state,
    char *error
);

int jme_jpl_orientation_state_order_naif(
    double jd0,
    double time_offset,
    int target_naif,
    int unit,
    int order,
    double *state,
    char *error
);

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
);

int jme_jpl_rotational_angular_momentum_state_naif(
    double jd_time,
    int target_naif,
    int unit,
    double *state,
    char *error
);

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
);

int jme_jpl_rotational_angular_momentum_state_split_naif(
    double jd0,
    double time_offset,
    int target_naif,
    int unit,
    double *state,
    char *error
);

int jme_jpl_rotational_angular_momentum_state_order_naif(
    double jd0,
    double time_offset,
    int target_naif,
    int unit,
    int order,
    double *state,
    char *error
);

int jme_jpl_orientation_record_count(char *error);

int jme_jpl_orientation_record_index(
    int index,
    int *target,
    double *first_time,
    double *last_time,
    int *frame,
    int *segment_type,
    char *error
);

int jme_jpl_timescale(void);

const char *jme_house_system_name(int house_system);

const char *jme_library_path(void);

double jme_radian_midpoint(double x1, double x0);

double jme_radian_normalize(double x);

double jme_radians_to_degrees(double radians);

double jme_radians_difference_signed(double p1, double p2);

double jme_refract(double altitude, double pressure, double temperature, int calc_flag);

int jme_rectangular_to_spherical_state(const double *rectangular, double *spherical);

double jme_state_distance(const double *state);

int jme_state_convert_units(const double *input, int input_unit, int output_unit, double *output);

double jme_state_light_time_days(const double *state, int unit);

double jme_state_position_velocity_dot(const double *state);

double jme_state_speed(const double *state);

int jme_state_add(const double *left, const double *right, double *output);

int jme_state_scale(const double *input, double factor, double *output);

int jme_state_subtract(const double *left, const double *right, double *output);

double jme_hours_normalize(double hours);

double jme_hours_to_degrees(double hours);

double jme_refract_extended(
    double altitude,
    double geoalt,
    double pressure,
    double temperature,
    double lapse_rate,
    int calc_flag,
    double *out
);

double jme_sidereal_time(double jd_ut);

double jme_sidereal_time0(double jd_ut, double eps, double nut);

void jme_split_degree(
    double ddeg,
    int roundflag,
    int *ideg,
    int *imin,
    int *isec,
    double *dsecfr,
    int *isgn
);

void jme_utc_time_zone(
    int year,
    int month,
    int day,
    int hour,
    int minute,
    double second,
    double timezone,
    int *out_year,
    int *out_month,
    int *out_day,
    int *out_hour,
    int *out_minute,
    double *out_second
);

int jme_utc_to_jd(
    int year,
    int month,
    int day,
    int hour,
    int minute,
    double second,
    int calendar,
    double *jd_utc
);

int jme_vsop87_planet_state(double jd_et, int body, double *results);

int jme_meeus_sun_state(double jd_et, double *results);

int jme_meeus_moon_state(double jd_et, double *results);

int jme_elp2000_moon_state(double jd_et, double *results);

int jme_meeus_planet_state(double jd_et, int body, double *results);

int jme_moshier_planet_state(double jd_et, int body, double *results);

#ifdef __cplusplus
}
#endif

#endif
