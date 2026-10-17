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
    /*
     * Extended-body identifiers use a JME-private negative range, separate
     * from core body identifiers and external ephemeris numbering schemes.
     * Historical values overlapped core bodies and external body-ID ranges.
     */
    JME_BODY_ALTJIRA = -1000,
    JME_BODY_AMYCUS = -1001,
    JME_BODY_ARROKOTH = -1002,
    JME_BODY_ASBOLUS = -1003,
    JME_BODY_ASTEROID_001 = -1004,
    JME_BODY_ASTEROID_002 = -1005,
    JME_BODY_ASTEROID_003 = -1006,
    JME_BODY_ASTEROID_004 = -1007,
    JME_BODY_ASTEROID_005 = -1008,
    JME_BODY_ASTEROID_006 = -1009,
    JME_BODY_ASTEROID_007 = -1010,
    JME_BODY_ASTEROID_008 = -1011,
    JME_BODY_ASTEROID_009 = -1012,
    JME_BODY_ASTEROID_010 = -1013,
    JME_BODY_ASTEROID_011 = -1014,
    JME_BODY_ASTEROID_012 = -1015,
    JME_BODY_ASTEROID_013 = -1016,
    JME_BODY_ASTEROID_014 = -1017,
    JME_BODY_ASTEROID_015 = -1018,
    JME_BODY_ASTEROID_016 = -1019,
    JME_BODY_ASTEROID_017 = -1020,
    JME_BODY_ASTEROID_018 = -1021,
    JME_BODY_ASTEROID_019 = -1022,
    JME_BODY_ASTEROID_020 = -1023,
    JME_BODY_ASTEROID_021 = -1024,
    JME_BODY_ASTEROID_022 = -1025,
    JME_BODY_ASTEROID_023 = -1026,
    JME_BODY_ASTEROID_024 = -1027,
    JME_BODY_ASTEROID_025 = -1028,
    JME_BODY_ASTEROID_026 = -1029,
    JME_BODY_ASTEROID_027 = -1030,
    JME_BODY_ASTEROID_028 = -1031,
    JME_BODY_ASTEROID_029 = -1032,
    JME_BODY_ASTEROID_030 = -1033,
    JME_BODY_ASTEROID_031 = -1034,
    JME_BODY_ASTEROID_032 = -1035,
    JME_BODY_ASTEROID_033 = -1036,
    JME_BODY_ASTEROID_034 = -1037,
    JME_BODY_ASTEROID_035 = -1038,
    JME_BODY_ASTEROID_036 = -1039,
    JME_BODY_ASTEROID_037 = -1040,
    JME_BODY_ASTEROID_038 = -1041,
    JME_BODY_ASTEROID_039 = -1042,
    JME_BODY_ASTEROID_040 = -1043,
    JME_BODY_ASTEROID_041 = -1044,
    JME_BODY_ASTEROID_042 = -1045,
    JME_BODY_ASTEROID_043 = -1046,
    JME_BODY_ASTEROID_044 = -1047,
    JME_BODY_ASTEROID_045 = -1048,
    JME_BODY_ASTEROID_046 = -1049,
    JME_BODY_ASTEROID_047 = -1050,
    JME_BODY_ASTEROID_048 = -1051,
    JME_BODY_ASTEROID_049 = -1052,
    JME_BODY_ASTEROID_050 = -1053,
    JME_BODY_ASTEROID_051 = -1054,
    JME_BODY_ASTEROID_052 = -1055,
    JME_BODY_ASTEROID_053 = -1056,
    JME_BODY_ASTEROID_054 = -1057,
    JME_BODY_ASTEROID_055 = -1058,
    JME_BODY_ASTEROID_056 = -1059,
    JME_BODY_ASTEROID_057 = -1060,
    JME_BODY_ASTEROID_058 = -1061,
    JME_BODY_ASTEROID_059 = -1062,
    JME_BODY_ASTEROID_060 = -1063,
    JME_BODY_ASTEROID_061 = -1064,
    JME_BODY_ASTEROID_062 = -1065,
    JME_BODY_ASTEROID_063 = -1066,
    JME_BODY_ASTEROID_064 = -1067,
    JME_BODY_ASTEROID_065 = -1068,
    JME_BODY_ASTEROID_066 = -1069,
    JME_BODY_ASTEROID_067 = -1070,
    JME_BODY_ASTEROID_068 = -1071,
    JME_BODY_ASTEROID_069 = -1072,
    JME_BODY_ASTEROID_070 = -1073,
    JME_BODY_ASTEROID_071 = -1074,
    JME_BODY_ASTEROID_072 = -1075,
    JME_BODY_ASTEROID_073 = -1076,
    JME_BODY_ASTEROID_074 = -1077,
    JME_BODY_ASTEROID_075 = -1078,
    JME_BODY_ASTEROID_076 = -1079,
    JME_BODY_ASTEROID_077 = -1080,
    JME_BODY_ASTEROID_078 = -1081,
    JME_BODY_ASTEROID_079 = -1082,
    JME_BODY_ASTEROID_080 = -1083,
    JME_BODY_ASTEROID_081 = -1084,
    JME_BODY_ASTEROID_082 = -1085,
    JME_BODY_ASTEROID_083 = -1086,
    JME_BODY_ASTEROID_084 = -1087,
    JME_BODY_ASTEROID_085 = -1088,
    JME_BODY_ASTEROID_086 = -1089,
    JME_BODY_ASTEROID_087 = -1090,
    JME_BODY_ASTEROID_088 = -1091,
    JME_BODY_ASTEROID_089 = -1092,
    JME_BODY_ASTEROID_090 = -1093,
    JME_BODY_ASTEROID_091 = -1094,
    JME_BODY_ASTEROID_092 = -1095,
    JME_BODY_ASTEROID_093 = -1096,
    JME_BODY_ASTEROID_094 = -1097,
    JME_BODY_ASTEROID_095 = -1098,
    JME_BODY_ASTEROID_096 = -1099,
    JME_BODY_ASTEROID_097 = -1100,
    JME_BODY_ASTEROID_098 = -1101,
    JME_BODY_ASTEROID_099 = -1102,
    JME_BODY_ASTEROID_100 = -1103,
    JME_BODY_ASTEROID_101 = -1104,
    JME_BODY_ASTEROID_102 = -1105,
    JME_BODY_ASTEROID_103 = -1106,
    JME_BODY_ASTEROID_104 = -1107,
    JME_BODY_ASTEROID_105 = -1108,
    JME_BODY_ASTEROID_106 = -1109,
    JME_BODY_ASTEROID_107 = -1110,
    JME_BODY_ASTEROID_108 = -1111,
    JME_BODY_ASTEROID_109 = -1112,
    JME_BODY_ASTEROID_110 = -1113,
    JME_BODY_BIENOR = -1114,
    JME_BODY_BORASISI = -1115,
    JME_BODY_CERES = -1116,
    JME_BODY_CHARIKLO = -1117,
    JME_BODY_CHIRON = -1118,
    JME_BODY_COMET_001 = -1119,
    JME_BODY_COMET_002 = -1120,
    JME_BODY_COMET_003 = -1121,
    JME_BODY_COMET_004 = -1122,
    JME_BODY_COMET_005 = -1123,
    JME_BODY_COMET_006 = -1124,
    JME_BODY_COMET_007 = -1125,
    JME_BODY_COMET_008 = -1126,
    JME_BODY_COMET_009 = -1127,
    JME_BODY_COMET_010 = -1128,
    JME_BODY_COMET_011 = -1129,
    JME_BODY_COMET_012 = -1130,
    JME_BODY_COMET_013 = -1131,
    JME_BODY_COMET_014 = -1132,
    JME_BODY_COMET_015 = -1133,
    JME_BODY_COMET_016 = -1134,
    JME_BODY_COMET_017 = -1135,
    JME_BODY_COMET_018 = -1136,
    JME_BODY_COMET_019 = -1137,
    JME_BODY_COMET_020 = -1138,
    JME_BODY_COMET_021 = -1139,
    JME_BODY_COMET_022 = -1140,
    JME_BODY_COMET_023 = -1141,
    JME_BODY_COMET_024 = -1142,
    JME_BODY_COMET_025 = -1143,
    JME_BODY_COMET_026 = -1144,
    JME_BODY_COMET_027 = -1145,
    JME_BODY_COMET_028 = -1146,
    JME_BODY_COMET_029 = -1147,
    JME_BODY_COMET_030 = -1148,
    JME_BODY_COMET_031 = -1149,
    JME_BODY_COMET_032 = -1150,
    JME_BODY_COMET_033 = -1151,
    JME_BODY_COMET_034 = -1152,
    JME_BODY_COMET_035 = -1153,
    JME_BODY_COMET_036 = -1154,
    JME_BODY_COMET_037 = -1155,
    JME_BODY_COMET_038 = -1156,
    JME_BODY_COMET_039 = -1157,
    JME_BODY_COMET_040 = -1158,
    JME_BODY_COMET_041 = -1159,
    JME_BODY_COMET_042 = -1160,
    JME_BODY_COMET_043 = -1161,
    JME_BODY_COMET_044 = -1162,
    JME_BODY_COMET_045 = -1163,
    JME_BODY_COMET_046 = -1164,
    JME_BODY_COMET_047 = -1165,
    JME_BODY_COMET_048 = -1166,
    JME_BODY_COMET_049 = -1167,
    JME_BODY_COMET_050 = -1168,
    JME_BODY_COMET_051 = -1169,
    JME_BODY_COMET_052 = -1170,
    JME_BODY_COMET_053 = -1171,
    JME_BODY_COMET_054 = -1172,
    JME_BODY_COMET_055 = -1173,
    JME_BODY_COMET_056 = -1174,
    JME_BODY_COMET_057 = -1175,
    JME_BODY_COMET_058 = -1176,
    JME_BODY_COMET_059 = -1177,
    JME_BODY_COMET_060 = -1178,
    JME_BODY_CRANTOR = -1179,
    JME_BODY_CYLLARUS = -1180,
    JME_BODY_DEUCALION = -1181,
    JME_BODY_ECHECLUS = -1182,
    JME_BODY_ELATUS = -1183,
    JME_BODY_ERIS = -1184,
    JME_BODY_GONGGONG = -1185,
    JME_BODY_HAUMEA = -1186,
    JME_BODY_HUYA = -1187,
    JME_BODY_HYLONOME = -1188,
    JME_BODY_IXION = -1189,
    JME_BODY_JUNO = -1190,
    JME_BODY_MAKEMAKE = -1191,
    JME_BODY_MINOR_PLANET_001 = -1192,
    JME_BODY_MINOR_PLANET_002 = -1193,
    JME_BODY_MINOR_PLANET_003 = -1194,
    JME_BODY_MINOR_PLANET_004 = -1195,
    JME_BODY_MINOR_PLANET_005 = -1196,
    JME_BODY_MINOR_PLANET_006 = -1197,
    JME_BODY_MINOR_PLANET_007 = -1198,
    JME_BODY_MINOR_PLANET_008 = -1199,
    JME_BODY_MINOR_PLANET_009 = -1200,
    JME_BODY_MINOR_PLANET_010 = -1201,
    JME_BODY_MINOR_PLANET_011 = -1202,
    JME_BODY_MINOR_PLANET_012 = -1203,
    JME_BODY_MINOR_PLANET_013 = -1204,
    JME_BODY_MINOR_PLANET_014 = -1205,
    JME_BODY_MINOR_PLANET_015 = -1206,
    JME_BODY_MINOR_PLANET_016 = -1207,
    JME_BODY_MINOR_PLANET_017 = -1208,
    JME_BODY_MINOR_PLANET_018 = -1209,
    JME_BODY_MINOR_PLANET_019 = -1210,
    JME_BODY_MINOR_PLANET_020 = -1211,
    JME_BODY_MINOR_PLANET_021 = -1212,
    JME_BODY_MINOR_PLANET_022 = -1213,
    JME_BODY_MINOR_PLANET_023 = -1214,
    JME_BODY_MINOR_PLANET_024 = -1215,
    JME_BODY_MINOR_PLANET_025 = -1216,
    JME_BODY_MINOR_PLANET_026 = -1217,
    JME_BODY_MINOR_PLANET_027 = -1218,
    JME_BODY_MINOR_PLANET_028 = -1219,
    JME_BODY_MINOR_PLANET_029 = -1220,
    JME_BODY_MINOR_PLANET_030 = -1221,
    JME_BODY_MINOR_PLANET_031 = -1222,
    JME_BODY_MINOR_PLANET_032 = -1223,
    JME_BODY_MINOR_PLANET_033 = -1224,
    JME_BODY_MINOR_PLANET_034 = -1225,
    JME_BODY_MINOR_PLANET_035 = -1226,
    JME_BODY_MINOR_PLANET_036 = -1227,
    JME_BODY_MINOR_PLANET_037 = -1228,
    JME_BODY_MINOR_PLANET_038 = -1229,
    JME_BODY_MINOR_PLANET_039 = -1230,
    JME_BODY_MINOR_PLANET_040 = -1231,
    JME_BODY_MINOR_PLANET_041 = -1232,
    JME_BODY_MINOR_PLANET_042 = -1233,
    JME_BODY_MINOR_PLANET_043 = -1234,
    JME_BODY_MINOR_PLANET_044 = -1235,
    JME_BODY_MINOR_PLANET_045 = -1236,
    JME_BODY_MINOR_PLANET_046 = -1237,
    JME_BODY_MINOR_PLANET_047 = -1238,
    JME_BODY_MINOR_PLANET_048 = -1239,
    JME_BODY_MINOR_PLANET_049 = -1240,
    JME_BODY_MINOR_PLANET_050 = -1241,
    JME_BODY_MINOR_PLANET_051 = -1242,
    JME_BODY_MINOR_PLANET_052 = -1243,
    JME_BODY_MINOR_PLANET_053 = -1244,
    JME_BODY_MINOR_PLANET_054 = -1245,
    JME_BODY_MINOR_PLANET_055 = -1246,
    JME_BODY_MINOR_PLANET_056 = -1247,
    JME_BODY_MINOR_PLANET_057 = -1248,
    JME_BODY_MINOR_PLANET_058 = -1249,
    JME_BODY_MINOR_PLANET_059 = -1250,
    JME_BODY_MINOR_PLANET_060 = -1251,
    JME_BODY_NESSUS = -1252,
    JME_BODY_OKYRHOE = -1253,
    JME_BODY_ORCUS = -1254,
    JME_BODY_PALLAS = -1255,
    JME_BODY_PELOPS = -1256,
    JME_BODY_PHOLUS = -1257,
    JME_BODY_QUAOAR = -1258,
    JME_BODY_RHADAMANTHUS = -1259,
    JME_BODY_SALACIA = -1260,
    JME_BODY_SEDNA = -1261,
    JME_BODY_THEREUS = -1262,
    JME_BODY_TYTHONUS = -1263,
    JME_BODY_VARUNA = -1264,
    JME_BODY_VESTA = -1265,
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
    JME_MODEL_DELTAT_STEPHENSON_ETC_2021 = 407,
    JME_MODEL_DELTAT_MS2004_TABLE1_LINEAR = 408,
    /* IAU 2000A nutation with the IAU 2006 (P03) adjustments, IERS Conventions
     * 2010 eq. 5.35 (ERFA eraNut06a): the nutation consistent with IAU 2006 precession. */
    JME_MODEL_NUT_IAU_2006A = 351,
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
    JME_MODEL_REVISED_PREC_LIESKE = 405,
    JME_SIDEREAL_LAHIRI_ICRC = 406
} jme_extended_constant;

/*
 * Time scales for jme_time_scale_convert(). Values are part of the ABI.
 * UT1 is realised with the context Delta-T model (no IERS EOP data).
 */
typedef enum jme_time_scale {
    JME_TIME_SCALE_UTC = 0,
    JME_TIME_SCALE_UT1 = 1,
    JME_TIME_SCALE_TAI = 2,
    JME_TIME_SCALE_TT = 3,
    JME_TIME_SCALE_TDB = 4,
    JME_TIME_SCALE_TCG = 5,
    JME_TIME_SCALE_TCB = 6
} jme_time_scale;

/*
 * Convert a two-part Julian date (jd1 + jd2, split preserved where possible)
 * between time scales. UTC uses the ERFA/SOFA quasi-JD convention for
 * leap-second days and is defined from 1960-01-01 until the leap-second table
 * expiry (see jme_leap_second_table_info). Returns JME_OK or JME_ERR.
 */
int jme_time_scale_convert(
    double jd1,
    double jd2,
    int from_scale,
    int to_scale,
    double *out1,
    double *out2,
    char *error
);
const char *jme_time_scale_name(int scale);

/*
 * Embedded leap-second (TAI-UTC) table information. jd_utc_valid_until is the
 * first UTC Julian date NOT covered by the table (currently 2027-07-01 0h UTC,
 * from IERS Bulletin C 72); last_tai_minus_utc is the latest TAI-UTC (s).
 * Either pointer may be NULL (not both).
 */
int jme_leap_second_table_info(double *jd_utc_valid_until, double *last_tai_minus_utc, char *error);

/* Flags for jme_get_ayanamsa_ex2(). */
typedef enum jme_ayanamsa_flags { JME_AYANAMSA_MEAN = 0, JME_AYANAMSA_TRUE = 1 } jme_ayanamsa_flags;

double jme_delta_t(double jd_ut);

double jme_delta_t_ex(double jd_ut, int model, char *error);

/*
 * Embedded IERS Delta-T table (finals2000A, used by JME_TIME_DELTAT_AUTOMATIC):
 * first day, last IERS-observed day and last predicted day (UTC Julian dates).
 * Update procedure: docs/TIME_SCALES.md.
 */
int jme_delta_t_iers_table_info(
    double *jd_first,
    double *jd_last_observed,
    double *jd_last,
    char *error
);

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
/*
 * Ayanamsa with explicit mean/true selection. flags: JME_AYANAMSA_MEAN (mean
 * equinox of date) or JME_AYANAMSA_TRUE (true equinox of date: mean + nutation
 * in longitude of the context nutation model). rate (optional, may be NULL)
 * receives the rate of the MEAN ayanamsa in degrees/day. jme_get_ayanamsa_ex()
 * returns the mean ayanamsa. See docs/AYANAMSA_MODELS.md.
 */
int jme_get_ayanamsa_ex2(
    double jd_et,
    int model,
    int flags,
    double *ayan,
    double *rate,
    char *error
);
int jme_get_ayanamsa_ex2_ut(
    double jd_ut,
    int model,
    int flags,
    double *ayan,
    double *rate,
    char *error
);

/*
 * Planet-centre policy of the JPL engine (jme_calc and related functions).
 * The JPL DE4xx planetary kernels contain the Mars..Pluto system barycentres
 * but not the planet centres (those come from satellite kernels such as
 * mar097.bsp or jup365.bsp, which can be loaded with jme_jpl_open_array).
 *   JME_JPL_CENTER_STRICT (default): a planet centre that the loaded kernels
 *     do not provide is an error (ENGINE=JPL) or makes ENGINE=AUTO use the
 *     analytical engine for that call (see jme_calc_resolve_engine).
 *   JME_JPL_CENTER_BARYCENTER (opt-in): when the planet centre is missing,
 *     the planetary-system barycentre is used instead. Centre-barycentre
 *     offsets: Mars ~0.2 m, Jupiter up to ~230 km, Saturn ~290 km,
 *     Uranus ~tens of km, Neptune ~75 km, Pluto ~2100 km.
 * Also settable with jme_set_astro_models("JPL_CENTER=STRICT|BARYCENTER").
 */
typedef enum jme_jpl_center_policy {
    JME_JPL_CENTER_STRICT = 0,
    JME_JPL_CENTER_BARYCENTER = 1
} jme_jpl_center_policy;

int jme_set_jpl_center_policy(int policy, char *error);
int jme_get_jpl_center_policy(void);

/* Engines reported by jme_calc_resolve_engine(). */
typedef enum jme_engine_id {
    JME_ENGINE_ID_JPL = 1,
    JME_ENGINE_ID_MOSHIER = 2,
    JME_ENGINE_ID_VSOP87_ELP82B = 3
} jme_engine_id;

/*
 * Reports which engine jme_calc() uses for body at jd_et (TT) under the
 * current settings, so that ENGINE=AUTO never substitutes a model silently.
 * *engine_id receives a jme_engine_id; *jpl_center_substituted (optional)
 * is 1 when the JPL planet centre was replaced by its system barycentre
 * under JME_JPL_CENTER_BARYCENTER. Returns JME_OK or JME_ERR.
 */
int jme_calc_resolve_engine(
    double jd_et,
    int body,
    int *engine_id,
    int *jpl_center_substituted,
    char *error
);

/*
 * Validating setters. Unlike the legacy void setters (kept unchanged), these
 * reject invalid input with JME_ERR and an error message and leave the
 * configuration untouched:
 *   jme_set_astro_models_checked: unknown or over-long tokens are errors;
 *   jme_set_topo_checked: non-finite values or |lat| > 90 are errors
 *     (longitude is normalised to [-180, 180));
 *   jme_set_sidereal_mode_checked: unknown modes or non-finite t0/ayan_t0
 *     are errors.
 */
int jme_set_astro_models_checked(const char *models, char *error);
int jme_set_topo_checked(double lon, double lat, double altitude, char *error);
int jme_set_sidereal_mode_checked(int sidereal_mode, double t0, double ayan_t0, char *error);

int jme_time_equ(double jd_ut, double *e, char *error);

void jme_matrix_identity(double *m);
void jme_matrix_multiply(const double *a, const double *b, double *out);
void jme_matrix_rotate_x(double angle_rad, double *m);
void jme_matrix_rotate_y(double angle_rad, double *m);
void jme_matrix_rotate_z(double angle_rad, double *m);
void jme_matrix_transform_state(const double *m, const double *input, double *output);

int jme_body_id_from_name(const char *name);

/*
 * Returns a library-owned name. Generated names for Asteroid 013..110,
 * Comet 001..060, and Minor Planet 001..060 use a thread-local buffer that
 * is overwritten by the next generated body name returned on the same
 * thread; copy the text if it must outlive that call.
 */
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

void jme_ecliptic_to_equatorial(double lon, double lat, double eps, double *ra, double *dec);

void jme_equatorial_to_ecliptic(double ra, double dec, double eps, double *lon, double *lat);

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

/*
 * Low-level JPL numeric APIs take the currently opened kernel's native time
 * scale (TDB or TCB); split variants preserve that two-part Julian date.
 * High-level jme_calc/jme_calc_ut accept TT/UT and convert internally.
 * UTC calendar variants convert UTC -> TAI -> TT -> kernel-native time.
 */
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

int jme_jpl_constant_index(
    int index,
    char *name,
    unsigned int name_size,
    double *value,
    char *error
);

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

/* Greenwich sidereal time in hours for UT1 jd_ut (Delta-T model gives TT).
 * apparent = 0: mean sidereal time; apparent != 0: apparent sidereal time
 * (adds the equation of the equinoxes for the context nutation model). */
int jme_sidereal_time_ex(double jd_ut, int apparent, double *hours, char *error);

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

/*
 * IERS Earth orientation: polar motion.
 *
 * jme_eop_load_file reads an IERS finals2000A or EOP C04 file and replaces
 * any loaded series. jme_eop_polar_motion returns xp, yp (arcsec) by 4-point
 * Lagrange interpolation at an MJD (predicted != 0 when a prediction row
 * contributed). jme_set_polar_motion(1) makes topocentric observer
 * positions, horizontal coordinates and eclipse geographic mappings apply
 * the polar-motion matrix W = eraPom00(xp, yp, s'); with it enabled, those
 * computations fail with an error when no EOP data cover the date. The
 * default (0) applies no polar motion. The loaded series is process-global:
 * load it before using it from several threads.
 */
int jme_eop_load_file(const char *path, char *error);
void jme_eop_clear(void);
int jme_eop_coverage(double *mjd_first, double *mjd_last, char *error);
int jme_eop_polar_motion(
    double mjd,
    double *xp_arcsec,
    double *yp_arcsec,
    int *predicted,
    char *error
);
void jme_set_polar_motion(int enabled);
int jme_get_polar_motion(void);

#ifdef __cplusplus
}
#endif

#endif
