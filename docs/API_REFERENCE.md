# API Reference

## Current Counts

| Area | Count | Source |
|---|---:|---|
| Public `jme_*` functions | 204 | `include/jme/jme.h`, `include/jme/jme_extended.h` |
| Public `JME_*` constants | 462 | `include/jme/jme.h`, `include/jme/jme_extended.h` |

## Current Implementation Map

| Area | Current implementation files |
|---|---|
| Engine/context selection | `src/api.c`, `src/context.c`, `src/context.h` |
| Main calculation pipeline | `src/calc.c` |
| JPL/CALCEPH kernel API | `src/jpl.c` |
| Moshier analytical backend | `src/moshier.c`, `src/moshier_full.c`, `src/moshier_moon.c`, `src/moshier/*` |
| VSOP87, ELP2000, Meeus backends | `src/vsop87.c`, `src/vsop87a_full.c`, `src/vsop87d_full.c`, `src/elp2000.c`, `src/elp2000/*`, `src/meeus.c` |
| Eclipse, occultation, rise/set, nodal/apsidal, heliacal events | `src/events.c` |
| Astrometry, coordinates, sidereal/time, houses, fixed stars, metadata | `src/astrometry.c`, `src/coordinates.c`, `src/sidereal.c`, `src/time.c`, `src/houses.c`, `src/fixstar.c`, `src/metadata.c` |
| Calendar and angular utility functions | `src/julian.c`, `src/angle.c` |

## Current Public Functions

| # | Function | Status |
|---:|---|---|
| 1 | `jme_body_id_from_name` | Declared and defined |
| 2 | `jme_body_naif_id` | Declared and defined |
| 3 | `jme_body_name` | Declared and defined |
| 4 | `jme_calc` | Declared and defined |
| 5 | `jme_calc_pctr` | Declared and defined |
| 6 | `jme_calc_ut` | Declared and defined |
| 7 | `jme_calendar_is_leap_year` | Declared and defined |
| 8 | `jme_centiseconds_difference` | Declared and defined |
| 9 | `jme_centiseconds_difference_signed` | Declared and defined |
| 10 | `jme_centiseconds_normalize` | Declared and defined |
| 11 | `jme_centiseconds_round_second` | Declared and defined |
| 12 | `jme_centiseconds_to_degree_string` | Declared and defined |
| 13 | `jme_centiseconds_to_lonlat_string` | Declared and defined |
| 14 | `jme_centiseconds_to_time_string` | Declared and defined |
| 15 | `jme_close` | Declared and defined |
| 16 | `jme_copy_body_name` | Declared and defined |
| 17 | `jme_date_is_valid` | Declared and defined |
| 18 | `jme_day_of_week` | Declared and defined |
| 19 | `jme_day_of_year` | Declared and defined |
| 20 | `jme_days_in_month` | Declared and defined |
| 21 | `jme_decimal_hour` | Declared and defined |
| 22 | `jme_degree_midpoint` | Declared and defined |
| 23 | `jme_degree_normalize` | Declared and defined |
| 24 | `jme_degrees_difference` | Declared and defined |
| 25 | `jme_degrees_difference_signed` | Declared and defined |
| 26 | `jme_degrees_to_hours` | Declared and defined |
| 27 | `jme_degrees_to_radians` | Declared and defined |
| 28 | `jme_delta_t` | Declared and defined |
| 29 | `jme_delta_t_ex` | Declared and defined |
| 30 | `jme_double_to_long` | Declared and defined |
| 31 | `jme_ecliptic_to_equatorial` | Declared and defined |
| 32 | `jme_ecliptic_to_equatorial_rectangular_state` | Declared and defined |
| 33 | `jme_elp2000_moon_state` | Declared and defined |
| 34 | `jme_ephemeris_path` | Declared and defined |
| 35 | `jme_equatorial_to_ecliptic` | Declared and defined |
| 36 | `jme_equatorial_to_ecliptic_rectangular_state` | Declared and defined |
| 37 | `jme_equatorial_to_horizontal` | Declared and defined |
| 38 | `jme_fixstar` | Declared and defined |
| 39 | `jme_fixstar_mag` | Declared and defined |
| 40 | `jme_fixstar_ut` | Declared and defined |
| 41 | `jme_fixstar2` | Declared and defined |
| 42 | `jme_fixstar2_mag` | Declared and defined |
| 43 | `jme_fixstar2_ut` | Declared and defined |
| 44 | `jme_gauquelin_sector` | Declared and defined |
| 45 | `jme_get_astro_models` | Declared and defined |
| 46 | `jme_get_ayanamsa` | Declared and defined |
| 47 | `jme_get_ayanamsa_ex` | Declared and defined |
| 48 | `jme_get_ayanamsa_ex_ut` | Declared and defined |
| 49 | `jme_get_ayanamsa_name` | Declared and defined |
| 50 | `jme_get_ayanamsa_ut` | Declared and defined |
| 51 | `jme_get_frame_bias_matrix` | Declared and defined |
| 52 | `jme_get_nutation` | Declared and defined |
| 53 | `jme_get_nutation_matrix` | Declared and defined |
| 54 | `jme_get_obliquity` | Declared and defined |
| 55 | `jme_get_orbital_elements` | Declared and defined |
| 56 | `jme_get_precession_matrix` | Declared and defined |
| 57 | `jme_get_sidereal_mode` | Declared and defined |
| 58 | `jme_get_tid_acc` | Declared and defined |
| 59 | `jme_get_topo_pos` | Declared and defined |
| 60 | `jme_heliacal_angle` | Declared and defined |
| 61 | `jme_heliacal_pheno_ut` | Declared and defined |
| 62 | `jme_heliacal_ut` | Declared and defined |
| 63 | `jme_helio_cross` | Declared and defined |
| 64 | `jme_helio_cross_ut` | Declared and defined |
| 65 | `jme_horizontal_to_equatorial` | Declared and defined |
| 66 | `jme_hours_normalize` | Declared and defined |
| 67 | `jme_hours_to_degrees` | Declared and defined |
| 68 | `jme_house_pos` | Declared and defined |
| 69 | `jme_house_system_name` | Declared and defined |
| 70 | `jme_houses` | Declared and defined |
| 71 | `jme_houses_armc` | Declared and defined |
| 72 | `jme_houses_armc_ex2` | Declared and defined |
| 73 | `jme_houses_ex` | Declared and defined |
| 74 | `jme_houses_ex2` | Declared and defined |
| 75 | `jme_jd_add_seconds` | Declared and defined |
| 76 | `jme_jd_difference_seconds` | Declared and defined |
| 77 | `jme_jd_to_utc` | Declared and defined |
| 78 | `jme_jpl_body_state` | Declared and defined |
| 79 | `jme_jpl_body_state_naif` | Declared and defined |
| 80 | `jme_jpl_body_state_native` | Declared and defined |
| 81 | `jme_jpl_body_state_native_naif` | Declared and defined |
| 82 | `jme_jpl_body_state_native_split` | Declared and defined |
| 83 | `jme_jpl_body_state_native_split_naif` | Declared and defined |
| 84 | `jme_jpl_body_state_order` | Declared and defined |
| 85 | `jme_jpl_body_state_order_naif` | Declared and defined |
| 86 | `jme_jpl_body_state_split` | Declared and defined |
| 87 | `jme_jpl_body_state_split_naif` | Declared and defined |
| 88 | `jme_jpl_body_state_utc` | Declared and defined |
| 89 | `jme_jpl_body_state_utc_naif` | Declared and defined |
| 90 | `jme_jpl_close` | Declared and defined |
| 91 | `jme_jpl_constant` | Declared and defined |
| 92 | `jme_jpl_constant_count` | Declared and defined |
| 93 | `jme_jpl_constant_index` | Declared and defined |
| 94 | `jme_jpl_constant_string` | Declared and defined |
| 95 | `jme_jpl_constant_string_vector` | Declared and defined |
| 96 | `jme_jpl_constant_vector` | Declared and defined |
| 97 | `jme_jpl_coverage` | Declared and defined |
| 98 | `jme_jpl_current_file_data` | Declared and defined |
| 99 | `jme_jpl_ecliptic_state` | Declared and defined |
| 100 | `jme_jpl_ecliptic_state_naif` | Declared and defined |
| 101 | `jme_jpl_ecliptic_state_split` | Declared and defined |
| 102 | `jme_jpl_ecliptic_state_split_naif` | Declared and defined |
| 103 | `jme_jpl_ecliptic_state_utc` | Declared and defined |
| 104 | `jme_jpl_ecliptic_state_utc_naif` | Declared and defined |
| 105 | `jme_jpl_engine_version` | Declared and defined |
| 106 | `jme_jpl_file` | Declared and defined |
| 107 | `jme_jpl_file_version` | Declared and defined |
| 108 | `jme_jpl_id_by_name` | Declared and defined |
| 109 | `jme_jpl_is_available` | Declared and defined |
| 110 | `jme_jpl_is_open` | Declared and defined |
| 111 | `jme_jpl_is_thread_safe` | Declared and defined |
| 112 | `jme_jpl_max_supported_order` | Declared and defined |
| 113 | `jme_jpl_name_by_id` | Declared and defined |
| 114 | `jme_jpl_open` | Declared and defined |
| 115 | `jme_jpl_open_array` | Declared and defined |
| 116 | `jme_jpl_orientation_record_count` | Declared and defined |
| 117 | `jme_jpl_orientation_record_index` | Declared and defined |
| 118 | `jme_jpl_orientation_state_naif` | Declared and defined |
| 119 | `jme_jpl_orientation_state_order_naif` | Declared and defined |
| 120 | `jme_jpl_orientation_state_split_naif` | Declared and defined |
| 121 | `jme_jpl_orientation_state_utc_naif` | Declared and defined |
| 122 | `jme_jpl_position_record_count` | Declared and defined |
| 123 | `jme_jpl_position_record_index` | Declared and defined |
| 124 | `jme_jpl_prefetch` | Declared and defined |
| 125 | `jme_jpl_rotational_angular_momentum_state_naif` | Declared and defined |
| 126 | `jme_jpl_rotational_angular_momentum_state_order_naif` | Declared and defined |
| 127 | `jme_jpl_rotational_angular_momentum_state_split_naif` | Declared and defined |
| 128 | `jme_jpl_rotational_angular_momentum_state_utc_naif` | Declared and defined |
| 129 | `jme_jpl_timescale` | Declared and defined |
| 130 | `jme_julian_day` | Declared and defined |
| 131 | `jme_lat_to_lmt` | Declared and defined |
| 132 | `jme_library_path` | Declared and defined |
| 133 | `jme_lmt_to_lat` | Declared and defined |
| 134 | `jme_lun_eclipse_how` | Declared and defined |
| 135 | `jme_lun_eclipse_when` | Declared and defined |
| 136 | `jme_lun_eclipse_when_loc` | Declared and defined |
| 137 | `jme_lun_occult_when_glob` | Declared and defined |
| 138 | `jme_lun_occult_when_loc` | Declared and defined |
| 139 | `jme_lun_occult_where` | Declared and defined |
| 140 | `jme_matrix_identity` | Declared and defined |
| 141 | `jme_matrix_multiply` | Declared and defined |
| 142 | `jme_matrix_rotate_x` | Declared and defined |
| 143 | `jme_matrix_rotate_y` | Declared and defined |
| 144 | `jme_matrix_rotate_z` | Declared and defined |
| 145 | `jme_matrix_transform_state` | Declared and defined |
| 146 | `jme_meeus_moon_state` | Declared and defined |
| 147 | `jme_meeus_planet_state` | Declared and defined |
| 148 | `jme_meeus_sun_state` | Declared and defined |
| 149 | `jme_mooncross` | Declared and defined |
| 150 | `jme_mooncross_node` | Declared and defined |
| 151 | `jme_mooncross_node_ut` | Declared and defined |
| 152 | `jme_mooncross_ut` | Declared and defined |
| 153 | `jme_moshier_planet_state` | Declared and defined |
| 154 | `jme_nod_aps` | Declared and defined |
| 155 | `jme_nod_aps_ut` | Declared and defined |
| 156 | `jme_orbit_max_min_true_distance` | Declared and defined |
| 157 | `jme_pheno` | Declared and defined |
| 158 | `jme_pheno_ut` | Declared and defined |
| 159 | `jme_radian_midpoint` | Declared and defined |
| 160 | `jme_radian_normalize` | Declared and defined |
| 161 | `jme_radians_difference_signed` | Declared and defined |
| 162 | `jme_radians_to_degrees` | Declared and defined |
| 163 | `jme_rectangular_to_spherical_state` | Declared and defined |
| 164 | `jme_refract` | Declared and defined |
| 165 | `jme_refract_extended` | Declared and defined |
| 166 | `jme_reverse_julian_day` | Declared and defined |
| 167 | `jme_rise_trans` | Declared and defined |
| 168 | `jme_rise_trans_true_hor` | Declared and defined |
| 169 | `jme_set_astro_models` | Declared and defined |
| 170 | `jme_set_delta_t_userdef` | Declared and defined |
| 171 | `jme_set_ephemeris_path` | Declared and defined |
| 172 | `jme_set_interpolate_nut` | Declared and defined |
| 173 | `jme_set_jpl_file` | Declared and defined |
| 174 | `jme_set_lapse_rate` | Declared and defined |
| 175 | `jme_set_sidereal_mode` | Declared and defined |
| 176 | `jme_set_tid_acc` | Declared and defined |
| 177 | `jme_set_topo` | Declared and defined |
| 178 | `jme_sidereal_time` | Declared and defined |
| 179 | `jme_sidereal_time0` | Declared and defined |
| 180 | `jme_sol_eclipse_how` | Declared and defined |
| 181 | `jme_sol_eclipse_when_glob` | Declared and defined |
| 182 | `jme_sol_eclipse_when_loc` | Declared and defined |
| 183 | `jme_sol_eclipse_where` | Declared and defined |
| 184 | `jme_solcross` | Declared and defined |
| 185 | `jme_solcross_ut` | Declared and defined |
| 186 | `jme_spherical_angular_separation` | Declared and defined |
| 187 | `jme_spherical_position_angle` | Declared and defined |
| 188 | `jme_spherical_to_rectangular_state` | Declared and defined |
| 189 | `jme_split_degree` | Declared and defined |
| 190 | `jme_state_add` | Declared and defined |
| 191 | `jme_state_convert_units` | Declared and defined |
| 192 | `jme_state_distance` | Declared and defined |
| 193 | `jme_state_light_time_days` | Declared and defined |
| 194 | `jme_state_position_velocity_dot` | Declared and defined |
| 195 | `jme_state_scale` | Declared and defined |
| 196 | `jme_state_speed` | Declared and defined |
| 197 | `jme_state_subtract` | Declared and defined |
| 198 | `jme_time_equ` | Declared and defined |
| 199 | `jme_topo_arcus_visionis` | Declared and defined |
| 200 | `jme_utc_time_zone` | Declared and defined |
| 201 | `jme_utc_to_jd` | Declared and defined |
| 202 | `jme_version` | Declared and defined |
| 203 | `jme_vis_limit_mag` | Declared and defined |
| 204 | `jme_vsop87_planet_state` | Declared and defined |

## Current Public Constants

| # | Constant | Status |
|---:|---|---|
| 1 | `JME_ANGLE_FORMAT_KEEP_DEG` | Public constant |
| 2 | `JME_ANGLE_FORMAT_KEEP_SIGN` | Public constant |
| 3 | `JME_ANGLE_FORMAT_NAKSHATRA` | Public constant |
| 4 | `JME_ANGLE_FORMAT_ROUND_DEG` | Public constant |
| 5 | `JME_ANGLE_FORMAT_ROUND_MIN` | Public constant |
| 6 | `JME_ANGLE_FORMAT_ROUND_SEC` | Public constant |
| 7 | `JME_ANGLE_FORMAT_ZODIACAL` | Public constant |
| 8 | `JME_AU_KM` | Public constant |
| 9 | `JME_BODY_ALTJIRA` | Public constant |
| 10 | `JME_BODY_AMYCUS` | Public constant |
| 11 | `JME_BODY_ARROKOTH` | Public constant |
| 12 | `JME_BODY_ASBOLUS` | Public constant |
| 13 | `JME_BODY_ASTEROID_001` | Public constant |
| 14 | `JME_BODY_ASTEROID_002` | Public constant |
| 15 | `JME_BODY_ASTEROID_003` | Public constant |
| 16 | `JME_BODY_ASTEROID_004` | Public constant |
| 17 | `JME_BODY_ASTEROID_005` | Public constant |
| 18 | `JME_BODY_ASTEROID_006` | Public constant |
| 19 | `JME_BODY_ASTEROID_007` | Public constant |
| 20 | `JME_BODY_ASTEROID_008` | Public constant |
| 21 | `JME_BODY_ASTEROID_009` | Public constant |
| 22 | `JME_BODY_ASTEROID_010` | Public constant |
| 23 | `JME_BODY_ASTEROID_011` | Public constant |
| 24 | `JME_BODY_ASTEROID_012` | Public constant |
| 25 | `JME_BODY_ASTEROID_013` | Public constant |
| 26 | `JME_BODY_ASTEROID_014` | Public constant |
| 27 | `JME_BODY_ASTEROID_015` | Public constant |
| 28 | `JME_BODY_ASTEROID_016` | Public constant |
| 29 | `JME_BODY_ASTEROID_017` | Public constant |
| 30 | `JME_BODY_ASTEROID_018` | Public constant |
| 31 | `JME_BODY_ASTEROID_019` | Public constant |
| 32 | `JME_BODY_ASTEROID_020` | Public constant |
| 33 | `JME_BODY_ASTEROID_021` | Public constant |
| 34 | `JME_BODY_ASTEROID_022` | Public constant |
| 35 | `JME_BODY_ASTEROID_023` | Public constant |
| 36 | `JME_BODY_ASTEROID_024` | Public constant |
| 37 | `JME_BODY_ASTEROID_025` | Public constant |
| 38 | `JME_BODY_ASTEROID_026` | Public constant |
| 39 | `JME_BODY_ASTEROID_027` | Public constant |
| 40 | `JME_BODY_ASTEROID_028` | Public constant |
| 41 | `JME_BODY_ASTEROID_029` | Public constant |
| 42 | `JME_BODY_ASTEROID_030` | Public constant |
| 43 | `JME_BODY_ASTEROID_031` | Public constant |
| 44 | `JME_BODY_ASTEROID_032` | Public constant |
| 45 | `JME_BODY_ASTEROID_033` | Public constant |
| 46 | `JME_BODY_ASTEROID_034` | Public constant |
| 47 | `JME_BODY_ASTEROID_035` | Public constant |
| 48 | `JME_BODY_ASTEROID_036` | Public constant |
| 49 | `JME_BODY_ASTEROID_037` | Public constant |
| 50 | `JME_BODY_ASTEROID_038` | Public constant |
| 51 | `JME_BODY_ASTEROID_039` | Public constant |
| 52 | `JME_BODY_ASTEROID_040` | Public constant |
| 53 | `JME_BODY_ASTEROID_041` | Public constant |
| 54 | `JME_BODY_ASTEROID_042` | Public constant |
| 55 | `JME_BODY_ASTEROID_043` | Public constant |
| 56 | `JME_BODY_ASTEROID_044` | Public constant |
| 57 | `JME_BODY_ASTEROID_045` | Public constant |
| 58 | `JME_BODY_ASTEROID_046` | Public constant |
| 59 | `JME_BODY_ASTEROID_047` | Public constant |
| 60 | `JME_BODY_ASTEROID_048` | Public constant |
| 61 | `JME_BODY_ASTEROID_049` | Public constant |
| 62 | `JME_BODY_ASTEROID_050` | Public constant |
| 63 | `JME_BODY_ASTEROID_051` | Public constant |
| 64 | `JME_BODY_ASTEROID_052` | Public constant |
| 65 | `JME_BODY_ASTEROID_053` | Public constant |
| 66 | `JME_BODY_ASTEROID_054` | Public constant |
| 67 | `JME_BODY_ASTEROID_055` | Public constant |
| 68 | `JME_BODY_ASTEROID_056` | Public constant |
| 69 | `JME_BODY_ASTEROID_057` | Public constant |
| 70 | `JME_BODY_ASTEROID_058` | Public constant |
| 71 | `JME_BODY_ASTEROID_059` | Public constant |
| 72 | `JME_BODY_ASTEROID_060` | Public constant |
| 73 | `JME_BODY_ASTEROID_061` | Public constant |
| 74 | `JME_BODY_ASTEROID_062` | Public constant |
| 75 | `JME_BODY_ASTEROID_063` | Public constant |
| 76 | `JME_BODY_ASTEROID_064` | Public constant |
| 77 | `JME_BODY_ASTEROID_065` | Public constant |
| 78 | `JME_BODY_ASTEROID_066` | Public constant |
| 79 | `JME_BODY_ASTEROID_067` | Public constant |
| 80 | `JME_BODY_ASTEROID_068` | Public constant |
| 81 | `JME_BODY_ASTEROID_069` | Public constant |
| 82 | `JME_BODY_ASTEROID_070` | Public constant |
| 83 | `JME_BODY_ASTEROID_071` | Public constant |
| 84 | `JME_BODY_ASTEROID_072` | Public constant |
| 85 | `JME_BODY_ASTEROID_073` | Public constant |
| 86 | `JME_BODY_ASTEROID_074` | Public constant |
| 87 | `JME_BODY_ASTEROID_075` | Public constant |
| 88 | `JME_BODY_ASTEROID_076` | Public constant |
| 89 | `JME_BODY_ASTEROID_077` | Public constant |
| 90 | `JME_BODY_ASTEROID_078` | Public constant |
| 91 | `JME_BODY_ASTEROID_079` | Public constant |
| 92 | `JME_BODY_ASTEROID_080` | Public constant |
| 93 | `JME_BODY_ASTEROID_081` | Public constant |
| 94 | `JME_BODY_ASTEROID_082` | Public constant |
| 95 | `JME_BODY_ASTEROID_083` | Public constant |
| 96 | `JME_BODY_ASTEROID_084` | Public constant |
| 97 | `JME_BODY_ASTEROID_085` | Public constant |
| 98 | `JME_BODY_ASTEROID_086` | Public constant |
| 99 | `JME_BODY_ASTEROID_087` | Public constant |
| 100 | `JME_BODY_ASTEROID_088` | Public constant |
| 101 | `JME_BODY_ASTEROID_089` | Public constant |
| 102 | `JME_BODY_ASTEROID_090` | Public constant |
| 103 | `JME_BODY_ASTEROID_091` | Public constant |
| 104 | `JME_BODY_ASTEROID_092` | Public constant |
| 105 | `JME_BODY_ASTEROID_093` | Public constant |
| 106 | `JME_BODY_ASTEROID_094` | Public constant |
| 107 | `JME_BODY_ASTEROID_095` | Public constant |
| 108 | `JME_BODY_ASTEROID_096` | Public constant |
| 109 | `JME_BODY_ASTEROID_097` | Public constant |
| 110 | `JME_BODY_ASTEROID_098` | Public constant |
| 111 | `JME_BODY_ASTEROID_099` | Public constant |
| 112 | `JME_BODY_ASTEROID_100` | Public constant |
| 113 | `JME_BODY_ASTEROID_101` | Public constant |
| 114 | `JME_BODY_ASTEROID_102` | Public constant |
| 115 | `JME_BODY_ASTEROID_103` | Public constant |
| 116 | `JME_BODY_ASTEROID_104` | Public constant |
| 117 | `JME_BODY_ASTEROID_105` | Public constant |
| 118 | `JME_BODY_ASTEROID_106` | Public constant |
| 119 | `JME_BODY_ASTEROID_107` | Public constant |
| 120 | `JME_BODY_ASTEROID_108` | Public constant |
| 121 | `JME_BODY_ASTEROID_109` | Public constant |
| 122 | `JME_BODY_ASTEROID_110` | Public constant |
| 123 | `JME_BODY_BIENOR` | Public constant |
| 124 | `JME_BODY_BORASISI` | Public constant |
| 125 | `JME_BODY_CERES` | Public constant |
| 126 | `JME_BODY_CHARIKLO` | Public constant |
| 127 | `JME_BODY_CHIRON` | Public constant |
| 128 | `JME_BODY_COMET_001` | Public constant |
| 129 | `JME_BODY_COMET_002` | Public constant |
| 130 | `JME_BODY_COMET_003` | Public constant |
| 131 | `JME_BODY_COMET_004` | Public constant |
| 132 | `JME_BODY_COMET_005` | Public constant |
| 133 | `JME_BODY_COMET_006` | Public constant |
| 134 | `JME_BODY_COMET_007` | Public constant |
| 135 | `JME_BODY_COMET_008` | Public constant |
| 136 | `JME_BODY_COMET_009` | Public constant |
| 137 | `JME_BODY_COMET_010` | Public constant |
| 138 | `JME_BODY_COMET_011` | Public constant |
| 139 | `JME_BODY_COMET_012` | Public constant |
| 140 | `JME_BODY_COMET_013` | Public constant |
| 141 | `JME_BODY_COMET_014` | Public constant |
| 142 | `JME_BODY_COMET_015` | Public constant |
| 143 | `JME_BODY_COMET_016` | Public constant |
| 144 | `JME_BODY_COMET_017` | Public constant |
| 145 | `JME_BODY_COMET_018` | Public constant |
| 146 | `JME_BODY_COMET_019` | Public constant |
| 147 | `JME_BODY_COMET_020` | Public constant |
| 148 | `JME_BODY_COMET_021` | Public constant |
| 149 | `JME_BODY_COMET_022` | Public constant |
| 150 | `JME_BODY_COMET_023` | Public constant |
| 151 | `JME_BODY_COMET_024` | Public constant |
| 152 | `JME_BODY_COMET_025` | Public constant |
| 153 | `JME_BODY_COMET_026` | Public constant |
| 154 | `JME_BODY_COMET_027` | Public constant |
| 155 | `JME_BODY_COMET_028` | Public constant |
| 156 | `JME_BODY_COMET_029` | Public constant |
| 157 | `JME_BODY_COMET_030` | Public constant |
| 158 | `JME_BODY_COMET_031` | Public constant |
| 159 | `JME_BODY_COMET_032` | Public constant |
| 160 | `JME_BODY_COMET_033` | Public constant |
| 161 | `JME_BODY_COMET_034` | Public constant |
| 162 | `JME_BODY_COMET_035` | Public constant |
| 163 | `JME_BODY_COMET_036` | Public constant |
| 164 | `JME_BODY_COMET_037` | Public constant |
| 165 | `JME_BODY_COMET_038` | Public constant |
| 166 | `JME_BODY_COMET_039` | Public constant |
| 167 | `JME_BODY_COMET_040` | Public constant |
| 168 | `JME_BODY_COMET_041` | Public constant |
| 169 | `JME_BODY_COMET_042` | Public constant |
| 170 | `JME_BODY_COMET_043` | Public constant |
| 171 | `JME_BODY_COMET_044` | Public constant |
| 172 | `JME_BODY_COMET_045` | Public constant |
| 173 | `JME_BODY_COMET_046` | Public constant |
| 174 | `JME_BODY_COMET_047` | Public constant |
| 175 | `JME_BODY_COMET_048` | Public constant |
| 176 | `JME_BODY_COMET_049` | Public constant |
| 177 | `JME_BODY_COMET_050` | Public constant |
| 178 | `JME_BODY_COMET_051` | Public constant |
| 179 | `JME_BODY_COMET_052` | Public constant |
| 180 | `JME_BODY_COMET_053` | Public constant |
| 181 | `JME_BODY_COMET_054` | Public constant |
| 182 | `JME_BODY_COMET_055` | Public constant |
| 183 | `JME_BODY_COMET_056` | Public constant |
| 184 | `JME_BODY_COMET_057` | Public constant |
| 185 | `JME_BODY_COMET_058` | Public constant |
| 186 | `JME_BODY_COMET_059` | Public constant |
| 187 | `JME_BODY_COMET_060` | Public constant |
| 188 | `JME_BODY_CRANTOR` | Public constant |
| 189 | `JME_BODY_CYLLARUS` | Public constant |
| 190 | `JME_BODY_DEUCALION` | Public constant |
| 191 | `JME_BODY_EARTH` | Public constant |
| 192 | `JME_BODY_EARTH_MOON_BARYCENTER` | Public constant |
| 193 | `JME_BODY_ECHECLUS` | Public constant |
| 194 | `JME_BODY_ELATUS` | Public constant |
| 195 | `JME_BODY_ERIS` | Public constant |
| 196 | `JME_BODY_GONGGONG` | Public constant |
| 197 | `JME_BODY_HAUMEA` | Public constant |
| 198 | `JME_BODY_HUYA` | Public constant |
| 199 | `JME_BODY_HYLONOME` | Public constant |
| 200 | `JME_BODY_IXION` | Public constant |
| 201 | `JME_BODY_JUNO` | Public constant |
| 202 | `JME_BODY_JUPITER` | Public constant |
| 203 | `JME_BODY_JUPITER_BARYCENTER` | Public constant |
| 204 | `JME_BODY_MAKEMAKE` | Public constant |
| 205 | `JME_BODY_MARS` | Public constant |
| 206 | `JME_BODY_MARS_BARYCENTER` | Public constant |
| 207 | `JME_BODY_MEAN_NODE` | Public constant |
| 208 | `JME_BODY_MERCURY` | Public constant |
| 209 | `JME_BODY_MERCURY_BARYCENTER` | Public constant |
| 210 | `JME_BODY_MINOR_PLANET_001` | Public constant |
| 211 | `JME_BODY_MINOR_PLANET_002` | Public constant |
| 212 | `JME_BODY_MINOR_PLANET_003` | Public constant |
| 213 | `JME_BODY_MINOR_PLANET_004` | Public constant |
| 214 | `JME_BODY_MINOR_PLANET_005` | Public constant |
| 215 | `JME_BODY_MINOR_PLANET_006` | Public constant |
| 216 | `JME_BODY_MINOR_PLANET_007` | Public constant |
| 217 | `JME_BODY_MINOR_PLANET_008` | Public constant |
| 218 | `JME_BODY_MINOR_PLANET_009` | Public constant |
| 219 | `JME_BODY_MINOR_PLANET_010` | Public constant |
| 220 | `JME_BODY_MINOR_PLANET_011` | Public constant |
| 221 | `JME_BODY_MINOR_PLANET_012` | Public constant |
| 222 | `JME_BODY_MINOR_PLANET_013` | Public constant |
| 223 | `JME_BODY_MINOR_PLANET_014` | Public constant |
| 224 | `JME_BODY_MINOR_PLANET_015` | Public constant |
| 225 | `JME_BODY_MINOR_PLANET_016` | Public constant |
| 226 | `JME_BODY_MINOR_PLANET_017` | Public constant |
| 227 | `JME_BODY_MINOR_PLANET_018` | Public constant |
| 228 | `JME_BODY_MINOR_PLANET_019` | Public constant |
| 229 | `JME_BODY_MINOR_PLANET_020` | Public constant |
| 230 | `JME_BODY_MINOR_PLANET_021` | Public constant |
| 231 | `JME_BODY_MINOR_PLANET_022` | Public constant |
| 232 | `JME_BODY_MINOR_PLANET_023` | Public constant |
| 233 | `JME_BODY_MINOR_PLANET_024` | Public constant |
| 234 | `JME_BODY_MINOR_PLANET_025` | Public constant |
| 235 | `JME_BODY_MINOR_PLANET_026` | Public constant |
| 236 | `JME_BODY_MINOR_PLANET_027` | Public constant |
| 237 | `JME_BODY_MINOR_PLANET_028` | Public constant |
| 238 | `JME_BODY_MINOR_PLANET_029` | Public constant |
| 239 | `JME_BODY_MINOR_PLANET_030` | Public constant |
| 240 | `JME_BODY_MINOR_PLANET_031` | Public constant |
| 241 | `JME_BODY_MINOR_PLANET_032` | Public constant |
| 242 | `JME_BODY_MINOR_PLANET_033` | Public constant |
| 243 | `JME_BODY_MINOR_PLANET_034` | Public constant |
| 244 | `JME_BODY_MINOR_PLANET_035` | Public constant |
| 245 | `JME_BODY_MINOR_PLANET_036` | Public constant |
| 246 | `JME_BODY_MINOR_PLANET_037` | Public constant |
| 247 | `JME_BODY_MINOR_PLANET_038` | Public constant |
| 248 | `JME_BODY_MINOR_PLANET_039` | Public constant |
| 249 | `JME_BODY_MINOR_PLANET_040` | Public constant |
| 250 | `JME_BODY_MINOR_PLANET_041` | Public constant |
| 251 | `JME_BODY_MINOR_PLANET_042` | Public constant |
| 252 | `JME_BODY_MINOR_PLANET_043` | Public constant |
| 253 | `JME_BODY_MINOR_PLANET_044` | Public constant |
| 254 | `JME_BODY_MINOR_PLANET_045` | Public constant |
| 255 | `JME_BODY_MINOR_PLANET_046` | Public constant |
| 256 | `JME_BODY_MINOR_PLANET_047` | Public constant |
| 257 | `JME_BODY_MINOR_PLANET_048` | Public constant |
| 258 | `JME_BODY_MINOR_PLANET_049` | Public constant |
| 259 | `JME_BODY_MINOR_PLANET_050` | Public constant |
| 260 | `JME_BODY_MINOR_PLANET_051` | Public constant |
| 261 | `JME_BODY_MINOR_PLANET_052` | Public constant |
| 262 | `JME_BODY_MINOR_PLANET_053` | Public constant |
| 263 | `JME_BODY_MINOR_PLANET_054` | Public constant |
| 264 | `JME_BODY_MINOR_PLANET_055` | Public constant |
| 265 | `JME_BODY_MINOR_PLANET_056` | Public constant |
| 266 | `JME_BODY_MINOR_PLANET_057` | Public constant |
| 267 | `JME_BODY_MINOR_PLANET_058` | Public constant |
| 268 | `JME_BODY_MINOR_PLANET_059` | Public constant |
| 269 | `JME_BODY_MINOR_PLANET_060` | Public constant |
| 270 | `JME_BODY_MOON` | Public constant |
| 271 | `JME_BODY_NEPTUNE` | Public constant |
| 272 | `JME_BODY_NEPTUNE_BARYCENTER` | Public constant |
| 273 | `JME_BODY_NESSUS` | Public constant |
| 274 | `JME_BODY_OKYRHOE` | Public constant |
| 275 | `JME_BODY_ORCUS` | Public constant |
| 276 | `JME_BODY_PALLAS` | Public constant |
| 277 | `JME_BODY_PELOPS` | Public constant |
| 278 | `JME_BODY_PHOLUS` | Public constant |
| 279 | `JME_BODY_PLUTO` | Public constant |
| 280 | `JME_BODY_PLUTO_BARYCENTER` | Public constant |
| 281 | `JME_BODY_QUAOAR` | Public constant |
| 282 | `JME_BODY_RHADAMANTHUS` | Public constant |
| 283 | `JME_BODY_SALACIA` | Public constant |
| 284 | `JME_BODY_SATURN` | Public constant |
| 285 | `JME_BODY_SATURN_BARYCENTER` | Public constant |
| 286 | `JME_BODY_SEDNA` | Public constant |
| 287 | `JME_BODY_SOLAR_SYSTEM_BARYCENTER` | Public constant |
| 288 | `JME_BODY_SUN` | Public constant |
| 289 | `JME_BODY_THEREUS` | Public constant |
| 290 | `JME_BODY_TRUE_NODE` | Public constant |
| 291 | `JME_BODY_TYTHONUS` | Public constant |
| 292 | `JME_BODY_URANUS` | Public constant |
| 293 | `JME_BODY_URANUS_BARYCENTER` | Public constant |
| 294 | `JME_BODY_VARUNA` | Public constant |
| 295 | `JME_BODY_VENUS` | Public constant |
| 296 | `JME_BODY_VENUS_BARYCENTER` | Public constant |
| 297 | `JME_BODY_VESTA` | Public constant |
| 298 | `JME_CALC_APPARENT_POSITION` | Public constant |
| 299 | `JME_CALC_ASTROMETRIC` | Public constant |
| 300 | `JME_CALC_BARYCENTRIC` | Public constant |
| 301 | `JME_CALC_CENTER_BODY` | Public constant |
| 302 | `JME_CALC_DISTANCE_AU` | Public constant |
| 303 | `JME_CALC_DISTANCE_KM` | Public constant |
| 304 | `JME_CALC_EQUATORIAL` | Public constant |
| 305 | `JME_CALC_HELIOCENTRIC` | Public constant |
| 306 | `JME_CALC_HIGH_PRECISION` | Public constant |
| 307 | `JME_CALC_ICRS` | Public constant |
| 308 | `JME_CALC_J2000` | Public constant |
| 309 | `JME_CALC_NO_ABERRATION` | Public constant |
| 310 | `JME_CALC_NO_LIGHT_DEFLECTION` | Public constant |
| 311 | `JME_CALC_NO_NUTATION` | Public constant |
| 312 | `JME_CALC_NONE` | Public constant |
| 313 | `JME_CALC_RADIANS` | Public constant |
| 314 | `JME_CALC_RAW_VECTOR` | Public constant |
| 315 | `JME_CALC_RECTANGULAR` | Public constant |
| 316 | `JME_CALC_SIDEREAL` | Public constant |
| 317 | `JME_CALC_SPEED` | Public constant |
| 318 | `JME_CALC_SPHERICAL` | Public constant |
| 319 | `JME_CALC_STRICT` | Public constant |
| 320 | `JME_CALC_TOPOCENTRIC` | Public constant |
| 321 | `JME_CALC_TRUE_POSITION` | Public constant |
| 322 | `JME_CALC_VELOCITY_PER_DAY` | Public constant |
| 323 | `JME_CALC_VELOCITY_PER_SECOND` | Public constant |
| 324 | `JME_CALC_XYZ` | Public constant |
| 325 | `JME_CALENDAR_GREGORIAN` | Public constant |
| 326 | `JME_CALENDAR_JULIAN` | Public constant |
| 327 | `JME_COORD_APPARENT_TO_TRUE` | Public constant |
| 328 | `JME_COORD_ECLIPTIC_TO_HORIZONTAL` | Public constant |
| 329 | `JME_COORD_EQUATORIAL_TO_HORIZONTAL` | Public constant |
| 330 | `JME_COORD_HORIZONTAL_TO_ECLIPTIC` | Public constant |
| 331 | `JME_COORD_HORIZONTAL_TO_EQUATORIAL` | Public constant |
| 332 | `JME_COORD_TRUE_TO_APPARENT` | Public constant |
| 333 | `JME_ECLIPSE_FIRST_CONTACT` | Public constant |
| 334 | `JME_ECLIPSE_FOURTH_CONTACT` | Public constant |
| 335 | `JME_ECLIPSE_LUNAR_PARTIAL` | Public constant |
| 336 | `JME_ECLIPSE_LUNAR_PENUMBRAL` | Public constant |
| 337 | `JME_ECLIPSE_LUNAR_TOTAL` | Public constant |
| 338 | `JME_ECLIPSE_MAX_VISIBLE` | Public constant |
| 339 | `JME_ECLIPSE_PENUMBRAL_BEGIN` | Public constant |
| 340 | `JME_ECLIPSE_PENUMBRAL_END` | Public constant |
| 341 | `JME_ECLIPSE_SECOND_CONTACT` | Public constant |
| 342 | `JME_ECLIPSE_SOLAR_ANNULAR` | Public constant |
| 343 | `JME_ECLIPSE_SOLAR_CENTRAL` | Public constant |
| 344 | `JME_ECLIPSE_SOLAR_HYBRID` | Public constant |
| 345 | `JME_ECLIPSE_SOLAR_NONCENTRAL` | Public constant |
| 346 | `JME_ECLIPSE_SOLAR_PARTIAL` | Public constant |
| 347 | `JME_ECLIPSE_SOLAR_TOTAL` | Public constant |
| 348 | `JME_ECLIPSE_THIRD_CONTACT` | Public constant |
| 349 | `JME_ECLIPSE_VISIBLE` | Public constant |
| 350 | `JME_ERR` | Public constant |
| 351 | `JME_EXTENDED_H` | Public constant |
| 352 | `JME_HOUSE_ALCABITIUS` | Public constant |
| 353 | `JME_HOUSE_APC` | Public constant |
| 354 | `JME_HOUSE_AZIMUTHAL` | Public constant |
| 355 | `JME_HOUSE_CAMPANUS` | Public constant |
| 356 | `JME_HOUSE_EQUAL` | Public constant |
| 357 | `JME_HOUSE_GAUQUELIN` | Public constant |
| 358 | `JME_HOUSE_HORIZONTAL` | Public constant |
| 359 | `JME_HOUSE_KOCH` | Public constant |
| 360 | `JME_HOUSE_KRUSINSKI` | Public constant |
| 361 | `JME_HOUSE_MERIDIAN` | Public constant |
| 362 | `JME_HOUSE_MORINUS` | Public constant |
| 363 | `JME_HOUSE_PLACIDUS` | Public constant |
| 364 | `JME_HOUSE_POLICH_PAGE` | Public constant |
| 365 | `JME_HOUSE_PORPHYRIUS` | Public constant |
| 366 | `JME_HOUSE_REGIOMONTANUS` | Public constant |
| 367 | `JME_HOUSE_SUNSHINE` | Public constant |
| 368 | `JME_HOUSE_VEHLOW_EQUAL` | Public constant |
| 369 | `JME_HOUSE_WHOLE_SIGN` | Public constant |
| 370 | `JME_JME_H` | Public constant |
| 371 | `JME_JPL_TIMESCALE_TCB` | Public constant |
| 372 | `JME_JPL_TIMESCALE_TDB` | Public constant |
| 373 | `JME_JPL_TIMESCALE_UNKNOWN` | Public constant |
| 374 | `JME_MODEL_BIAS_IAU2000` | Public constant |
| 375 | `JME_MODEL_BIAS_IAU2006` | Public constant |
| 376 | `JME_MODEL_BIAS_NONE` | Public constant |
| 377 | `JME_MODEL_DELTAT_ESPENAK_MEEUS_2006` | Public constant |
| 378 | `JME_MODEL_DELTAT_STEPHENSON_1997` | Public constant |
| 379 | `JME_MODEL_DELTAT_STEPHENSON_ETC_2016` | Public constant |
| 380 | `JME_MODEL_DELTAT_STEPHENSON_MORRISON_1984` | Public constant |
| 381 | `JME_MODEL_DELTAT_STEPHENSON_MORRISON_2004` | Public constant |
| 382 | `JME_MODEL_NUT_IAU_1980` | Public constant |
| 383 | `JME_MODEL_NUT_IAU_2000A` | Public constant |
| 384 | `JME_MODEL_NUT_IAU_2000B` | Public constant |
| 385 | `JME_MODEL_OBL_IAU_1980` | Public constant |
| 386 | `JME_MODEL_OBL_IAU_2000` | Public constant |
| 387 | `JME_MODEL_OBL_IAU_2006` | Public constant |
| 388 | `JME_MODEL_PREC_IAU_1976` | Public constant |
| 389 | `JME_MODEL_PREC_IAU_2000` | Public constant |
| 390 | `JME_MODEL_PREC_IAU_2006` | Public constant |
| 391 | `JME_MODEL_PREC_LASKAR_1986` | Public constant |
| 392 | `JME_MODEL_PREC_VONDRAK_2011` | Public constant |
| 393 | `JME_MODEL_REVISED_IAU_2000` | Public constant |
| 394 | `JME_MODEL_REVISED_IAU_2006` | Public constant |
| 395 | `JME_MODEL_REVISED_PREC_LASKAR` | Public constant |
| 396 | `JME_MODEL_REVISED_PREC_LIESKE` | Public constant |
| 397 | `JME_MODEL_REVISED_PREC_VONDRAK` | Public constant |
| 398 | `JME_MODEL_SIDT_IAU_1976` | Public constant |
| 399 | `JME_MODEL_SIDT_IAU_2006` | Public constant |
| 400 | `JME_OK` | Public constant |
| 401 | `JME_ORIENTATION_RAD_PER_DAY` | Public constant |
| 402 | `JME_ORIENTATION_RAD_PER_SECOND` | Public constant |
| 403 | `JME_RISE_ANTI_MERIDIAN_TRANSIT` | Public constant |
| 404 | `JME_RISE_ASTRONOMICAL_TWILIGHT` | Public constant |
| 405 | `JME_RISE_CIVIL_TWILIGHT` | Public constant |
| 406 | `JME_RISE_DISC_BOTTOM` | Public constant |
| 407 | `JME_RISE_DISC_CENTER` | Public constant |
| 408 | `JME_RISE_FIXED_DISC_SIZE` | Public constant |
| 409 | `JME_RISE_HINDU_RISING` | Public constant |
| 410 | `JME_RISE_MERIDIAN_TRANSIT` | Public constant |
| 411 | `JME_RISE_NAUTICAL_TWILIGHT` | Public constant |
| 412 | `JME_RISE_NO_REFRACTION` | Public constant |
| 413 | `JME_RISE_RISE` | Public constant |
| 414 | `JME_RISE_SET` | Public constant |
| 415 | `JME_SECONDS_PER_DAY` | Public constant |
| 416 | `JME_SIDEREAL_ALDEBARAN_15TAU` | Public constant |
| 417 | `JME_SIDEREAL_ARYABHATA` | Public constant |
| 418 | `JME_SIDEREAL_B1950` | Public constant |
| 419 | `JME_SIDEREAL_BABYL_ETPSC` | Public constant |
| 420 | `JME_SIDEREAL_BABYL_HUBER` | Public constant |
| 421 | `JME_SIDEREAL_BABYL_KUGLER1` | Public constant |
| 422 | `JME_SIDEREAL_BABYL_KUGLER2` | Public constant |
| 423 | `JME_SIDEREAL_BABYL_KUGLER3` | Public constant |
| 424 | `JME_SIDEREAL_DELUCE` | Public constant |
| 425 | `JME_SIDEREAL_FAGAN_BRADLEY` | Public constant |
| 426 | `JME_SIDEREAL_GALCENT_0SAG` | Public constant |
| 427 | `JME_SIDEREAL_HIPPARCHOS` | Public constant |
| 428 | `JME_SIDEREAL_J1900` | Public constant |
| 429 | `JME_SIDEREAL_J2000` | Public constant |
| 430 | `JME_SIDEREAL_JN_BHASIN` | Public constant |
| 431 | `JME_SIDEREAL_KRISHNAMURTI` | Public constant |
| 432 | `JME_SIDEREAL_LAHIRI` | Public constant |
| 433 | `JME_SIDEREAL_RAMAN` | Public constant |
| 434 | `JME_SIDEREAL_SASSANIAN` | Public constant |
| 435 | `JME_SIDEREAL_SS_CITRA` | Public constant |
| 436 | `JME_SIDEREAL_SS_REVATI` | Public constant |
| 437 | `JME_SIDEREAL_SURYASIDDHANTA` | Public constant |
| 438 | `JME_SIDEREAL_TRUE_CITRA` | Public constant |
| 439 | `JME_SIDEREAL_TRUE_MULA` | Public constant |
| 440 | `JME_SIDEREAL_TRUE_PUSHYA` | Public constant |
| 441 | `JME_SIDEREAL_TRUE_REVATI` | Public constant |
| 442 | `JME_SIDEREAL_USER` | Public constant |
| 443 | `JME_SIDEREAL_USHASHASHI` | Public constant |
| 444 | `JME_SIDEREAL_YUKTESHWAR` | Public constant |
| 445 | `JME_SPEED_OF_LIGHT_KM_PER_SEC` | Public constant |
| 446 | `JME_TIME_DELTAT_AUTOMATIC` | Public constant |
| 447 | `JME_TIME_TIDAL_AUTOMATIC` | Public constant |
| 448 | `JME_TIME_TIDAL_DE200` | Public constant |
| 449 | `JME_TIME_TIDAL_DE403` | Public constant |
| 450 | `JME_TIME_TIDAL_DE404` | Public constant |
| 451 | `JME_TIME_TIDAL_DE405` | Public constant |
| 452 | `JME_TIME_TIDAL_DE406` | Public constant |
| 453 | `JME_TIME_TIDAL_DE421` | Public constant |
| 454 | `JME_TIME_TIDAL_DE430` | Public constant |
| 455 | `JME_TIME_TIDAL_DE431` | Public constant |
| 456 | `JME_TIME_TIDAL_DE441` | Public constant |
| 457 | `JME_VECTOR_AU_PER_DAY` | Public constant |
| 458 | `JME_VECTOR_AU_PER_SECOND` | Public constant |
| 459 | `JME_VECTOR_KM_PER_DAY` | Public constant |
| 460 | `JME_VECTOR_KM_PER_SECOND` | Public constant |
| 461 | `JME_VERSION` | Public constant |
| 462 | `JME_VERSION_ID` | Public constant |
