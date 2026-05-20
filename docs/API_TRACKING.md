# API Tracking

This file tracks the project-owned public API. The product surface is `jme_*` functions and `JME_*` constants.

## Current Counts

| Area | Count | Source |
|---|---:|---|
| JME public functions | 191 | `include/jme/jme.h`, `include/jme/jme_extended.h` |
| JME public constants | 462 | `include/jme/jme.h`, `include/jme/jme_extended.h` |

Function count and behavior coverage are separate checks. The full behavior matrix is in `docs/REFERENCE_FUNCTION_COVERAGE.md`.

## JME Public Functions

| # | Function | Status |
|---:|---|---|
| 1 | `jme_body_id_from_name` | Declared and defined |
| 2 | `jme_body_naif_id` | Declared and defined |
| 3 | `jme_calc` | Declared and defined |
| 4 | `jme_calc_pctr` | Declared and defined |
| 5 | `jme_calc_ut` | Declared and defined |
| 6 | `jme_calendar_is_leap_year` | Declared and defined |
| 7 | `jme_centiseconds_difference` | Declared and defined |
| 8 | `jme_centiseconds_difference_signed` | Declared and defined |
| 9 | `jme_centiseconds_normalize` | Declared and defined |
| 10 | `jme_centiseconds_round_second` | Declared and defined |
| 11 | `jme_close` | Declared and defined |
| 12 | `jme_date_is_valid` | Declared and defined |
| 13 | `jme_day_of_week` | Declared and defined |
| 14 | `jme_day_of_year` | Declared and defined |
| 15 | `jme_days_in_month` | Declared and defined |
| 16 | `jme_decimal_hour` | Declared and defined |
| 17 | `jme_degree_midpoint` | Declared and defined |
| 18 | `jme_degree_normalize` | Declared and defined |
| 19 | `jme_degrees_difference` | Declared and defined |
| 20 | `jme_degrees_difference_signed` | Declared and defined |
| 21 | `jme_degrees_to_hours` | Declared and defined |
| 22 | `jme_degrees_to_radians` | Declared and defined |
| 23 | `jme_delta_t` | Declared and defined |
| 24 | `jme_delta_t_ex` | Declared and defined |
| 25 | `jme_double_to_long` | Declared and defined |
| 26 | `jme_ecliptic_to_equatorial` | Declared and defined |
| 27 | `jme_ecliptic_to_equatorial_rectangular_state` | Declared and defined |
| 28 | `jme_elp2000_moon_state` | Declared and defined |
| 29 | `jme_equatorial_to_ecliptic` | Declared and defined |
| 30 | `jme_equatorial_to_ecliptic_rectangular_state` | Declared and defined |
| 31 | `jme_equatorial_to_horizontal` | Declared and defined |
| 32 | `jme_fixstar` | Declared and defined |
| 33 | `jme_fixstar_mag` | Declared and defined |
| 34 | `jme_fixstar_ut` | Declared and defined |
| 35 | `jme_fixstar2` | Declared and defined |
| 36 | `jme_fixstar2_mag` | Declared and defined |
| 37 | `jme_fixstar2_ut` | Declared and defined |
| 38 | `jme_gauquelin_sector` | Declared and defined |
| 39 | `jme_get_astro_models` | Declared and defined |
| 40 | `jme_get_ayanamsa` | Declared and defined |
| 41 | `jme_get_ayanamsa_ex` | Declared and defined |
| 42 | `jme_get_ayanamsa_ex_ut` | Declared and defined |
| 43 | `jme_get_ayanamsa_ut` | Declared and defined |
| 44 | `jme_get_nutation` | Declared and defined |
| 45 | `jme_get_nutation_matrix` | Declared and defined |
| 46 | `jme_get_obliquity` | Declared and defined |
| 47 | `jme_get_orbital_elements` | Declared and defined |
| 48 | `jme_get_precession_matrix` | Declared and defined |
| 49 | `jme_get_sidereal_mode` | Declared and defined |
| 50 | `jme_get_tid_acc` | Declared and defined |
| 51 | `jme_get_topo_pos` | Declared and defined |
| 52 | `jme_heliacal_angle` | Declared and defined |
| 53 | `jme_heliacal_pheno_ut` | Declared and defined |
| 54 | `jme_heliacal_ut` | Declared and defined |
| 55 | `jme_helio_cross` | Declared and defined |
| 56 | `jme_helio_cross_ut` | Declared and defined |
| 57 | `jme_horizontal_to_equatorial` | Declared and defined |
| 58 | `jme_hours_normalize` | Declared and defined |
| 59 | `jme_hours_to_degrees` | Declared and defined |
| 60 | `jme_house_pos` | Declared and defined |
| 61 | `jme_houses` | Declared and defined |
| 62 | `jme_houses_armc` | Declared and defined |
| 63 | `jme_houses_armc_ex2` | Declared and defined |
| 64 | `jme_houses_ex` | Declared and defined |
| 65 | `jme_houses_ex2` | Declared and defined |
| 66 | `jme_jd_add_seconds` | Declared and defined |
| 67 | `jme_jd_difference_seconds` | Declared and defined |
| 68 | `jme_jd_to_utc` | Declared and defined |
| 69 | `jme_jpl_body_state` | Declared and defined |
| 70 | `jme_jpl_body_state_naif` | Declared and defined |
| 71 | `jme_jpl_body_state_native` | Declared and defined |
| 72 | `jme_jpl_body_state_native_naif` | Declared and defined |
| 73 | `jme_jpl_body_state_native_split` | Declared and defined |
| 74 | `jme_jpl_body_state_native_split_naif` | Declared and defined |
| 75 | `jme_jpl_body_state_order` | Declared and defined |
| 76 | `jme_jpl_body_state_order_naif` | Declared and defined |
| 77 | `jme_jpl_body_state_split` | Declared and defined |
| 78 | `jme_jpl_body_state_split_naif` | Declared and defined |
| 79 | `jme_jpl_body_state_utc` | Declared and defined |
| 80 | `jme_jpl_body_state_utc_naif` | Declared and defined |
| 81 | `jme_jpl_close` | Declared and defined |
| 82 | `jme_jpl_constant` | Declared and defined |
| 83 | `jme_jpl_constant_count` | Declared and defined |
| 84 | `jme_jpl_constant_index` | Declared and defined |
| 85 | `jme_jpl_constant_string` | Declared and defined |
| 86 | `jme_jpl_constant_string_vector` | Declared and defined |
| 87 | `jme_jpl_constant_vector` | Declared and defined |
| 88 | `jme_jpl_coverage` | Declared and defined |
| 89 | `jme_jpl_current_file_data` | Declared and defined |
| 90 | `jme_jpl_ecliptic_state` | Declared and defined |
| 91 | `jme_jpl_ecliptic_state_naif` | Declared and defined |
| 92 | `jme_jpl_ecliptic_state_split` | Declared and defined |
| 93 | `jme_jpl_ecliptic_state_split_naif` | Declared and defined |
| 94 | `jme_jpl_ecliptic_state_utc` | Declared and defined |
| 95 | `jme_jpl_ecliptic_state_utc_naif` | Declared and defined |
| 96 | `jme_jpl_file_version` | Declared and defined |
| 97 | `jme_jpl_id_by_name` | Declared and defined |
| 98 | `jme_jpl_is_available` | Declared and defined |
| 99 | `jme_jpl_is_open` | Declared and defined |
| 100 | `jme_jpl_is_thread_safe` | Declared and defined |
| 101 | `jme_jpl_max_supported_order` | Declared and defined |
| 102 | `jme_jpl_name_by_id` | Declared and defined |
| 103 | `jme_jpl_open` | Declared and defined |
| 104 | `jme_jpl_open_array` | Declared and defined |
| 105 | `jme_jpl_orientation_record_count` | Declared and defined |
| 106 | `jme_jpl_orientation_record_index` | Declared and defined |
| 107 | `jme_jpl_orientation_state_naif` | Declared and defined |
| 108 | `jme_jpl_orientation_state_order_naif` | Declared and defined |
| 109 | `jme_jpl_orientation_state_split_naif` | Declared and defined |
| 110 | `jme_jpl_orientation_state_utc_naif` | Declared and defined |
| 111 | `jme_jpl_position_record_count` | Declared and defined |
| 112 | `jme_jpl_position_record_index` | Declared and defined |
| 113 | `jme_jpl_prefetch` | Declared and defined |
| 114 | `jme_jpl_rotational_angular_momentum_state_naif` | Declared and defined |
| 115 | `jme_jpl_rotational_angular_momentum_state_order_naif` | Declared and defined |
| 116 | `jme_jpl_rotational_angular_momentum_state_split_naif` | Declared and defined |
| 117 | `jme_jpl_rotational_angular_momentum_state_utc_naif` | Declared and defined |
| 118 | `jme_jpl_timescale` | Declared and defined |
| 119 | `jme_julian_day` | Declared and defined |
| 120 | `jme_lat_to_lmt` | Declared and defined |
| 121 | `jme_lmt_to_lat` | Declared and defined |
| 122 | `jme_lun_eclipse_how` | Declared and defined |
| 123 | `jme_lun_eclipse_when` | Declared and defined |
| 124 | `jme_lun_eclipse_when_loc` | Declared and defined |
| 125 | `jme_lun_occult_when_glob` | Declared and defined |
| 126 | `jme_lun_occult_when_loc` | Declared and defined |
| 127 | `jme_lun_occult_where` | Declared and defined |
| 128 | `jme_matrix_identity` | Declared and defined |
| 129 | `jme_matrix_multiply` | Declared and defined |
| 130 | `jme_matrix_rotate_x` | Declared and defined |
| 131 | `jme_matrix_rotate_y` | Declared and defined |
| 132 | `jme_matrix_rotate_z` | Declared and defined |
| 133 | `jme_matrix_transform_state` | Declared and defined |
| 134 | `jme_meeus_moon_state` | Declared and defined |
| 135 | `jme_meeus_planet_state` | Declared and defined |
| 136 | `jme_meeus_sun_state` | Declared and defined |
| 137 | `jme_mooncross` | Declared and defined |
| 138 | `jme_mooncross_node` | Declared and defined |
| 139 | `jme_mooncross_node_ut` | Declared and defined |
| 140 | `jme_mooncross_ut` | Declared and defined |
| 141 | `jme_moshier_planet_state` | Declared and defined |
| 142 | `jme_nod_aps` | Declared and defined |
| 143 | `jme_nod_aps_ut` | Declared and defined |
| 144 | `jme_orbit_max_min_true_distance` | Declared and defined |
| 145 | `jme_pheno` | Declared and defined |
| 146 | `jme_pheno_ut` | Declared and defined |
| 147 | `jme_radian_midpoint` | Declared and defined |
| 148 | `jme_radian_normalize` | Declared and defined |
| 149 | `jme_radians_difference_signed` | Declared and defined |
| 150 | `jme_radians_to_degrees` | Declared and defined |
| 151 | `jme_rectangular_to_spherical_state` | Declared and defined |
| 152 | `jme_refract` | Declared and defined |
| 153 | `jme_refract_extended` | Declared and defined |
| 154 | `jme_reverse_julian_day` | Declared and defined |
| 155 | `jme_rise_trans` | Declared and defined |
| 156 | `jme_rise_trans_true_hor` | Declared and defined |
| 157 | `jme_set_astro_models` | Declared and defined |
| 158 | `jme_set_delta_t_userdef` | Declared and defined |
| 159 | `jme_set_ephemeris_path` | Declared and defined |
| 160 | `jme_set_interpolate_nut` | Declared and defined |
| 161 | `jme_set_jpl_file` | Declared and defined |
| 162 | `jme_set_lapse_rate` | Declared and defined |
| 163 | `jme_set_sidereal_mode` | Declared and defined |
| 164 | `jme_set_tid_acc` | Declared and defined |
| 165 | `jme_set_topo` | Declared and defined |
| 166 | `jme_sidereal_time` | Declared and defined |
| 167 | `jme_sidereal_time0` | Declared and defined |
| 168 | `jme_sol_eclipse_how` | Declared and defined |
| 169 | `jme_sol_eclipse_when_glob` | Declared and defined |
| 170 | `jme_sol_eclipse_when_loc` | Declared and defined |
| 171 | `jme_sol_eclipse_where` | Declared and defined |
| 172 | `jme_solcross` | Declared and defined |
| 173 | `jme_solcross_ut` | Declared and defined |
| 174 | `jme_spherical_angular_separation` | Declared and defined |
| 175 | `jme_spherical_position_angle` | Declared and defined |
| 176 | `jme_spherical_to_rectangular_state` | Declared and defined |
| 177 | `jme_split_degree` | Declared and defined |
| 178 | `jme_state_add` | Declared and defined |
| 179 | `jme_state_convert_units` | Declared and defined |
| 180 | `jme_state_distance` | Declared and defined |
| 181 | `jme_state_light_time_days` | Declared and defined |
| 182 | `jme_state_position_velocity_dot` | Declared and defined |
| 183 | `jme_state_scale` | Declared and defined |
| 184 | `jme_state_speed` | Declared and defined |
| 185 | `jme_state_subtract` | Declared and defined |
| 186 | `jme_time_equ` | Declared and defined |
| 187 | `jme_topo_arcus_visionis` | Declared and defined |
| 188 | `jme_utc_time_zone` | Declared and defined |
| 189 | `jme_utc_to_jd` | Declared and defined |
| 190 | `jme_vis_limit_mag` | Declared and defined |
| 191 | `jme_vsop87_planet_state` | Declared and defined |

## JME Public Constants

| # | Constant |
|---:|---|
| 1 | `JME_ANGLE_FORMAT_KEEP_DEG` |
| 2 | `JME_ANGLE_FORMAT_KEEP_SIGN` |
| 3 | `JME_ANGLE_FORMAT_NAKSHATRA` |
| 4 | `JME_ANGLE_FORMAT_ROUND_DEG` |
| 5 | `JME_ANGLE_FORMAT_ROUND_MIN` |
| 6 | `JME_ANGLE_FORMAT_ROUND_SEC` |
| 7 | `JME_ANGLE_FORMAT_ZODIACAL` |
| 8 | `JME_AU_KM` |
| 9 | `JME_BODY_ALTJIRA` |
| 10 | `JME_BODY_AMYCUS` |
| 11 | `JME_BODY_ARROKOTH` |
| 12 | `JME_BODY_ASBOLUS` |
| 13 | `JME_BODY_ASTEROID_001` |
| 14 | `JME_BODY_ASTEROID_002` |
| 15 | `JME_BODY_ASTEROID_003` |
| 16 | `JME_BODY_ASTEROID_004` |
| 17 | `JME_BODY_ASTEROID_005` |
| 18 | `JME_BODY_ASTEROID_006` |
| 19 | `JME_BODY_ASTEROID_007` |
| 20 | `JME_BODY_ASTEROID_008` |
| 21 | `JME_BODY_ASTEROID_009` |
| 22 | `JME_BODY_ASTEROID_010` |
| 23 | `JME_BODY_ASTEROID_011` |
| 24 | `JME_BODY_ASTEROID_012` |
| 25 | `JME_BODY_ASTEROID_013` |
| 26 | `JME_BODY_ASTEROID_014` |
| 27 | `JME_BODY_ASTEROID_015` |
| 28 | `JME_BODY_ASTEROID_016` |
| 29 | `JME_BODY_ASTEROID_017` |
| 30 | `JME_BODY_ASTEROID_018` |
| 31 | `JME_BODY_ASTEROID_019` |
| 32 | `JME_BODY_ASTEROID_020` |
| 33 | `JME_BODY_ASTEROID_021` |
| 34 | `JME_BODY_ASTEROID_022` |
| 35 | `JME_BODY_ASTEROID_023` |
| 36 | `JME_BODY_ASTEROID_024` |
| 37 | `JME_BODY_ASTEROID_025` |
| 38 | `JME_BODY_ASTEROID_026` |
| 39 | `JME_BODY_ASTEROID_027` |
| 40 | `JME_BODY_ASTEROID_028` |
| 41 | `JME_BODY_ASTEROID_029` |
| 42 | `JME_BODY_ASTEROID_030` |
| 43 | `JME_BODY_ASTEROID_031` |
| 44 | `JME_BODY_ASTEROID_032` |
| 45 | `JME_BODY_ASTEROID_033` |
| 46 | `JME_BODY_ASTEROID_034` |
| 47 | `JME_BODY_ASTEROID_035` |
| 48 | `JME_BODY_ASTEROID_036` |
| 49 | `JME_BODY_ASTEROID_037` |
| 50 | `JME_BODY_ASTEROID_038` |
| 51 | `JME_BODY_ASTEROID_039` |
| 52 | `JME_BODY_ASTEROID_040` |
| 53 | `JME_BODY_ASTEROID_041` |
| 54 | `JME_BODY_ASTEROID_042` |
| 55 | `JME_BODY_ASTEROID_043` |
| 56 | `JME_BODY_ASTEROID_044` |
| 57 | `JME_BODY_ASTEROID_045` |
| 58 | `JME_BODY_ASTEROID_046` |
| 59 | `JME_BODY_ASTEROID_047` |
| 60 | `JME_BODY_ASTEROID_048` |
| 61 | `JME_BODY_ASTEROID_049` |
| 62 | `JME_BODY_ASTEROID_050` |
| 63 | `JME_BODY_ASTEROID_051` |
| 64 | `JME_BODY_ASTEROID_052` |
| 65 | `JME_BODY_ASTEROID_053` |
| 66 | `JME_BODY_ASTEROID_054` |
| 67 | `JME_BODY_ASTEROID_055` |
| 68 | `JME_BODY_ASTEROID_056` |
| 69 | `JME_BODY_ASTEROID_057` |
| 70 | `JME_BODY_ASTEROID_058` |
| 71 | `JME_BODY_ASTEROID_059` |
| 72 | `JME_BODY_ASTEROID_060` |
| 73 | `JME_BODY_ASTEROID_061` |
| 74 | `JME_BODY_ASTEROID_062` |
| 75 | `JME_BODY_ASTEROID_063` |
| 76 | `JME_BODY_ASTEROID_064` |
| 77 | `JME_BODY_ASTEROID_065` |
| 78 | `JME_BODY_ASTEROID_066` |
| 79 | `JME_BODY_ASTEROID_067` |
| 80 | `JME_BODY_ASTEROID_068` |
| 81 | `JME_BODY_ASTEROID_069` |
| 82 | `JME_BODY_ASTEROID_070` |
| 83 | `JME_BODY_ASTEROID_071` |
| 84 | `JME_BODY_ASTEROID_072` |
| 85 | `JME_BODY_ASTEROID_073` |
| 86 | `JME_BODY_ASTEROID_074` |
| 87 | `JME_BODY_ASTEROID_075` |
| 88 | `JME_BODY_ASTEROID_076` |
| 89 | `JME_BODY_ASTEROID_077` |
| 90 | `JME_BODY_ASTEROID_078` |
| 91 | `JME_BODY_ASTEROID_079` |
| 92 | `JME_BODY_ASTEROID_080` |
| 93 | `JME_BODY_ASTEROID_081` |
| 94 | `JME_BODY_ASTEROID_082` |
| 95 | `JME_BODY_ASTEROID_083` |
| 96 | `JME_BODY_ASTEROID_084` |
| 97 | `JME_BODY_ASTEROID_085` |
| 98 | `JME_BODY_ASTEROID_086` |
| 99 | `JME_BODY_ASTEROID_087` |
| 100 | `JME_BODY_ASTEROID_088` |
| 101 | `JME_BODY_ASTEROID_089` |
| 102 | `JME_BODY_ASTEROID_090` |
| 103 | `JME_BODY_ASTEROID_091` |
| 104 | `JME_BODY_ASTEROID_092` |
| 105 | `JME_BODY_ASTEROID_093` |
| 106 | `JME_BODY_ASTEROID_094` |
| 107 | `JME_BODY_ASTEROID_095` |
| 108 | `JME_BODY_ASTEROID_096` |
| 109 | `JME_BODY_ASTEROID_097` |
| 110 | `JME_BODY_ASTEROID_098` |
| 111 | `JME_BODY_ASTEROID_099` |
| 112 | `JME_BODY_ASTEROID_100` |
| 113 | `JME_BODY_ASTEROID_101` |
| 114 | `JME_BODY_ASTEROID_102` |
| 115 | `JME_BODY_ASTEROID_103` |
| 116 | `JME_BODY_ASTEROID_104` |
| 117 | `JME_BODY_ASTEROID_105` |
| 118 | `JME_BODY_ASTEROID_106` |
| 119 | `JME_BODY_ASTEROID_107` |
| 120 | `JME_BODY_ASTEROID_108` |
| 121 | `JME_BODY_ASTEROID_109` |
| 122 | `JME_BODY_ASTEROID_110` |
| 123 | `JME_BODY_BIENOR` |
| 124 | `JME_BODY_BORASISI` |
| 125 | `JME_BODY_CERES` |
| 126 | `JME_BODY_CHARIKLO` |
| 127 | `JME_BODY_CHIRON` |
| 128 | `JME_BODY_COMET_001` |
| 129 | `JME_BODY_COMET_002` |
| 130 | `JME_BODY_COMET_003` |
| 131 | `JME_BODY_COMET_004` |
| 132 | `JME_BODY_COMET_005` |
| 133 | `JME_BODY_COMET_006` |
| 134 | `JME_BODY_COMET_007` |
| 135 | `JME_BODY_COMET_008` |
| 136 | `JME_BODY_COMET_009` |
| 137 | `JME_BODY_COMET_010` |
| 138 | `JME_BODY_COMET_011` |
| 139 | `JME_BODY_COMET_012` |
| 140 | `JME_BODY_COMET_013` |
| 141 | `JME_BODY_COMET_014` |
| 142 | `JME_BODY_COMET_015` |
| 143 | `JME_BODY_COMET_016` |
| 144 | `JME_BODY_COMET_017` |
| 145 | `JME_BODY_COMET_018` |
| 146 | `JME_BODY_COMET_019` |
| 147 | `JME_BODY_COMET_020` |
| 148 | `JME_BODY_COMET_021` |
| 149 | `JME_BODY_COMET_022` |
| 150 | `JME_BODY_COMET_023` |
| 151 | `JME_BODY_COMET_024` |
| 152 | `JME_BODY_COMET_025` |
| 153 | `JME_BODY_COMET_026` |
| 154 | `JME_BODY_COMET_027` |
| 155 | `JME_BODY_COMET_028` |
| 156 | `JME_BODY_COMET_029` |
| 157 | `JME_BODY_COMET_030` |
| 158 | `JME_BODY_COMET_031` |
| 159 | `JME_BODY_COMET_032` |
| 160 | `JME_BODY_COMET_033` |
| 161 | `JME_BODY_COMET_034` |
| 162 | `JME_BODY_COMET_035` |
| 163 | `JME_BODY_COMET_036` |
| 164 | `JME_BODY_COMET_037` |
| 165 | `JME_BODY_COMET_038` |
| 166 | `JME_BODY_COMET_039` |
| 167 | `JME_BODY_COMET_040` |
| 168 | `JME_BODY_COMET_041` |
| 169 | `JME_BODY_COMET_042` |
| 170 | `JME_BODY_COMET_043` |
| 171 | `JME_BODY_COMET_044` |
| 172 | `JME_BODY_COMET_045` |
| 173 | `JME_BODY_COMET_046` |
| 174 | `JME_BODY_COMET_047` |
| 175 | `JME_BODY_COMET_048` |
| 176 | `JME_BODY_COMET_049` |
| 177 | `JME_BODY_COMET_050` |
| 178 | `JME_BODY_COMET_051` |
| 179 | `JME_BODY_COMET_052` |
| 180 | `JME_BODY_COMET_053` |
| 181 | `JME_BODY_COMET_054` |
| 182 | `JME_BODY_COMET_055` |
| 183 | `JME_BODY_COMET_056` |
| 184 | `JME_BODY_COMET_057` |
| 185 | `JME_BODY_COMET_058` |
| 186 | `JME_BODY_COMET_059` |
| 187 | `JME_BODY_COMET_060` |
| 188 | `JME_BODY_CRANTOR` |
| 189 | `JME_BODY_CYLLARUS` |
| 190 | `JME_BODY_DEUCALION` |
| 191 | `JME_BODY_EARTH` |
| 192 | `JME_BODY_EARTH_MOON_BARYCENTER` |
| 193 | `JME_BODY_ECHECLUS` |
| 194 | `JME_BODY_ELATUS` |
| 195 | `JME_BODY_ERIS` |
| 196 | `JME_BODY_GONGGONG` |
| 197 | `JME_BODY_HAUMEA` |
| 198 | `JME_BODY_HUYA` |
| 199 | `JME_BODY_HYLONOME` |
| 200 | `JME_BODY_IXION` |
| 201 | `JME_BODY_JUNO` |
| 202 | `JME_BODY_JUPITER` |
| 203 | `JME_BODY_JUPITER_BARYCENTER` |
| 204 | `JME_BODY_MAKEMAKE` |
| 205 | `JME_BODY_MARS` |
| 206 | `JME_BODY_MARS_BARYCENTER` |
| 207 | `JME_BODY_MERCURY` |
| 208 | `JME_BODY_MERCURY_BARYCENTER` |
| 209 | `JME_BODY_MINOR_PLANET_001` |
| 210 | `JME_BODY_MINOR_PLANET_002` |
| 211 | `JME_BODY_MINOR_PLANET_003` |
| 212 | `JME_BODY_MINOR_PLANET_004` |
| 213 | `JME_BODY_MINOR_PLANET_005` |
| 214 | `JME_BODY_MINOR_PLANET_006` |
| 215 | `JME_BODY_MINOR_PLANET_007` |
| 216 | `JME_BODY_MINOR_PLANET_008` |
| 217 | `JME_BODY_MINOR_PLANET_009` |
| 218 | `JME_BODY_MINOR_PLANET_010` |
| 219 | `JME_BODY_MINOR_PLANET_011` |
| 220 | `JME_BODY_MINOR_PLANET_012` |
| 221 | `JME_BODY_MINOR_PLANET_013` |
| 222 | `JME_BODY_MINOR_PLANET_014` |
| 223 | `JME_BODY_MINOR_PLANET_015` |
| 224 | `JME_BODY_MINOR_PLANET_016` |
| 225 | `JME_BODY_MINOR_PLANET_017` |
| 226 | `JME_BODY_MINOR_PLANET_018` |
| 227 | `JME_BODY_MINOR_PLANET_019` |
| 228 | `JME_BODY_MINOR_PLANET_020` |
| 229 | `JME_BODY_MINOR_PLANET_021` |
| 230 | `JME_BODY_MINOR_PLANET_022` |
| 231 | `JME_BODY_MINOR_PLANET_023` |
| 232 | `JME_BODY_MINOR_PLANET_024` |
| 233 | `JME_BODY_MINOR_PLANET_025` |
| 234 | `JME_BODY_MINOR_PLANET_026` |
| 235 | `JME_BODY_MINOR_PLANET_027` |
| 236 | `JME_BODY_MINOR_PLANET_028` |
| 237 | `JME_BODY_MINOR_PLANET_029` |
| 238 | `JME_BODY_MINOR_PLANET_030` |
| 239 | `JME_BODY_MINOR_PLANET_031` |
| 240 | `JME_BODY_MINOR_PLANET_032` |
| 241 | `JME_BODY_MINOR_PLANET_033` |
| 242 | `JME_BODY_MINOR_PLANET_034` |
| 243 | `JME_BODY_MINOR_PLANET_035` |
| 244 | `JME_BODY_MINOR_PLANET_036` |
| 245 | `JME_BODY_MINOR_PLANET_037` |
| 246 | `JME_BODY_MINOR_PLANET_038` |
| 247 | `JME_BODY_MINOR_PLANET_039` |
| 248 | `JME_BODY_MINOR_PLANET_040` |
| 249 | `JME_BODY_MINOR_PLANET_041` |
| 250 | `JME_BODY_MINOR_PLANET_042` |
| 251 | `JME_BODY_MINOR_PLANET_043` |
| 252 | `JME_BODY_MINOR_PLANET_044` |
| 253 | `JME_BODY_MINOR_PLANET_045` |
| 254 | `JME_BODY_MINOR_PLANET_046` |
| 255 | `JME_BODY_MINOR_PLANET_047` |
| 256 | `JME_BODY_MINOR_PLANET_048` |
| 257 | `JME_BODY_MINOR_PLANET_049` |
| 258 | `JME_BODY_MINOR_PLANET_050` |
| 259 | `JME_BODY_MINOR_PLANET_051` |
| 260 | `JME_BODY_MINOR_PLANET_052` |
| 261 | `JME_BODY_MINOR_PLANET_053` |
| 262 | `JME_BODY_MINOR_PLANET_054` |
| 263 | `JME_BODY_MINOR_PLANET_055` |
| 264 | `JME_BODY_MINOR_PLANET_056` |
| 265 | `JME_BODY_MINOR_PLANET_057` |
| 266 | `JME_BODY_MINOR_PLANET_058` |
| 267 | `JME_BODY_MINOR_PLANET_059` |
| 268 | `JME_BODY_MINOR_PLANET_060` |
| 269 | `JME_BODY_MOON` |
| 270 | `JME_BODY_NEPTUNE` |
| 271 | `JME_BODY_NEPTUNE_BARYCENTER` |
| 272 | `JME_BODY_NESSUS` |
| 273 | `JME_BODY_OKYRHOE` |
| 274 | `JME_BODY_ORCUS` |
| 275 | `JME_BODY_PALLAS` |
| 276 | `JME_BODY_PELOPS` |
| 277 | `JME_BODY_PHOLUS` |
| 278 | `JME_BODY_PLUTO` |
| 279 | `JME_BODY_PLUTO_BARYCENTER` |
| 280 | `JME_BODY_QUAOAR` |
| 281 | `JME_BODY_RHADAMANTHUS` |
| 282 | `JME_BODY_SALACIA` |
| 283 | `JME_BODY_SATURN` |
| 284 | `JME_BODY_SATURN_BARYCENTER` |
| 285 | `JME_BODY_SEDNA` |
| 286 | `JME_BODY_SOLAR_SYSTEM_BARYCENTER` |
| 287 | `JME_BODY_SUN` |
| 288 | `JME_BODY_THEREUS` |
| 289 | `JME_BODY_TYTHONUS` |
| 290 | `JME_BODY_URANUS` |
| 291 | `JME_BODY_URANUS_BARYCENTER` |
| 292 | `JME_BODY_VARUNA` |
| 293 | `JME_BODY_VENUS` |
| 294 | `JME_BODY_VENUS_BARYCENTER` |
| 295 | `JME_BODY_VESTA` |
| 296 | `JME_CALC_APPARENT_POSITION` |
| 297 | `JME_CALC_ASTROMETRIC` |
| 298 | `JME_CALC_BARYCENTRIC` |
| 299 | `JME_CALC_CENTER_BODY` |
| 300 | `JME_CALC_DISTANCE_AU` |
| 301 | `JME_CALC_DISTANCE_KM` |
| 302 | `JME_CALC_EQUATORIAL` |
| 303 | `JME_CALC_HELIOCENTRIC` |
| 304 | `JME_CALC_HIGH_PRECISION` |
| 305 | `JME_CALC_ICRS` |
| 306 | `JME_CALC_J2000` |
| 307 | `JME_CALC_NO_ABERRATION` |
| 308 | `JME_CALC_NO_LIGHT_DEFLECTION` |
| 309 | `JME_CALC_NO_NUTATION` |
| 310 | `JME_CALC_NONE` |
| 311 | `JME_CALC_RADIANS` |
| 312 | `JME_CALC_RAW_VECTOR` |
| 313 | `JME_CALC_RECTANGULAR` |
| 314 | `JME_CALC_SIDEREAL` |
| 315 | `JME_CALC_SPEED` |
| 316 | `JME_CALC_SPHERICAL` |
| 317 | `JME_CALC_STRICT` |
| 318 | `JME_CALC_TOPOCENTRIC` |
| 319 | `JME_CALC_TRUE_POSITION` |
| 320 | `JME_CALC_VELOCITY_PER_DAY` |
| 321 | `JME_CALC_VELOCITY_PER_SECOND` |
| 322 | `JME_CALC_XYZ` |
| 323 | `JME_CALENDAR_GREGORIAN` |
| 324 | `JME_CALENDAR_JULIAN` |
| 325 | `JME_COORD_APPARENT_TO_TRUE` |
| 326 | `JME_COORD_ECLIPTIC_TO_HORIZONTAL` |
| 327 | `JME_COORD_EQUATORIAL_TO_HORIZONTAL` |
| 328 | `JME_COORD_HORIZONTAL_TO_ECLIPTIC` |
| 329 | `JME_COORD_HORIZONTAL_TO_EQUATORIAL` |
| 330 | `JME_COORD_TRUE_TO_APPARENT` |
| 331 | `JME_ECLIPSE_FIRST_CONTACT` |
| 332 | `JME_ECLIPSE_FOURTH_CONTACT` |
| 333 | `JME_ECLIPSE_LUNAR_PARTIAL` |
| 334 | `JME_ECLIPSE_LUNAR_PENUMBRAL` |
| 335 | `JME_ECLIPSE_LUNAR_TOTAL` |
| 336 | `JME_ECLIPSE_MAX_VISIBLE` |
| 337 | `JME_ECLIPSE_PENUMBRAL_BEGIN` |
| 338 | `JME_ECLIPSE_PENUMBRAL_END` |
| 339 | `JME_ECLIPSE_SECOND_CONTACT` |
| 340 | `JME_ECLIPSE_SOLAR_ANNULAR` |
| 341 | `JME_ECLIPSE_SOLAR_CENTRAL` |
| 342 | `JME_ECLIPSE_SOLAR_HYBRID` |
| 343 | `JME_ECLIPSE_SOLAR_NONCENTRAL` |
| 344 | `JME_ECLIPSE_SOLAR_PARTIAL` |
| 345 | `JME_ECLIPSE_SOLAR_TOTAL` |
| 346 | `JME_ECLIPSE_THIRD_CONTACT` |
| 347 | `JME_ECLIPSE_VISIBLE` |
| 348 | `JME_ERR` |
| 349 | `JME_EXTENDED_H` |
| 350 | `JME_HOUSE_ALCABITIUS` |
| 351 | `JME_HOUSE_APC` |
| 352 | `JME_HOUSE_AZIMUTHAL` |
| 353 | `JME_HOUSE_CAMPANUS` |
| 354 | `JME_HOUSE_EQUAL` |
| 355 | `JME_HOUSE_GAUQUELIN` |
| 356 | `JME_HOUSE_HORIZONTAL` |
| 357 | `JME_HOUSE_KOCH` |
| 358 | `JME_HOUSE_KRUSINSKI` |
| 359 | `JME_HOUSE_MERIDIAN` |
| 360 | `JME_HOUSE_MORINUS` |
| 361 | `JME_HOUSE_PLACIDUS` |
| 362 | `JME_HOUSE_POLICH_PAGE` |
| 363 | `JME_HOUSE_PORPHYRIUS` |
| 364 | `JME_HOUSE_REGIOMONTANUS` |
| 365 | `JME_HOUSE_SUNSHINE` |
| 366 | `JME_HOUSE_VEHLOW_EQUAL` |
| 367 | `JME_HOUSE_WHOLE_SIGN` |
| 368 | `JME_JME_H` |
| 369 | `JME_JPL_TIMESCALE_TCB` |
| 370 | `JME_JPL_TIMESCALE_TDB` |
| 371 | `JME_JPL_TIMESCALE_UNKNOWN` |
| 372 | `JME_MODEL_BIAS_IAU2000` |
| 373 | `JME_MODEL_BIAS_IAU2006` |
| 374 | `JME_MODEL_BIAS_NONE` |
| 375 | `JME_MODEL_DELTAT_ESPENAK_MEEUS_2006` |
| 376 | `JME_MODEL_DELTAT_STEPHENSON_1997` |
| 377 | `JME_MODEL_DELTAT_STEPHENSON_ETC_2016` |
| 378 | `JME_MODEL_DELTAT_STEPHENSON_MORRISON_1984` |
| 379 | `JME_MODEL_DELTAT_STEPHENSON_MORRISON_2004` |
| 380 | `JME_MODEL_NUT_IAU_1980` |
| 381 | `JME_MODEL_NUT_IAU_2000A` |
| 382 | `JME_MODEL_NUT_IAU_2000B` |
| 383 | `JME_MODEL_OBL_IAU_1980` |
| 384 | `JME_MODEL_OBL_IAU_2000` |
| 385 | `JME_MODEL_OBL_IAU_2006` |
| 386 | `JME_MODEL_PREC_IAU_1976` |
| 387 | `JME_MODEL_PREC_IAU_2000` |
| 388 | `JME_MODEL_PREC_IAU_2006` |
| 389 | `JME_MODEL_PREC_LASKAR_1986` |
| 390 | `JME_MODEL_PREC_VONDRAK_2011` |
| 391 | `JME_MODEL_REVISED_IAU_2000` |
| 392 | `JME_MODEL_REVISED_IAU_2006` |
| 393 | `JME_MODEL_REVISED_PREC_LASKAR` |
| 394 | `JME_MODEL_REVISED_PREC_LIESKE` |
| 395 | `JME_MODEL_REVISED_PREC_VONDRAK` |
| 396 | `JME_MODEL_SIDT_IAU_1976` |
| 397 | `JME_MODEL_SIDT_IAU_2006` |
| 398 | `JME_OK` |
| 399 | `JME_ORIENTATION_RAD_PER_DAY` |
| 400 | `JME_ORIENTATION_RAD_PER_SECOND` |
| 401 | `JME_RISE_ANTI_MERIDIAN_TRANSIT` |
| 402 | `JME_RISE_ASTRONOMICAL_TWILIGHT` |
| 403 | `JME_RISE_CIVIL_TWILIGHT` |
| 404 | `JME_RISE_DISC_BOTTOM` |
| 405 | `JME_RISE_DISC_CENTER` |
| 406 | `JME_RISE_FIXED_DISC_SIZE` |
| 407 | `JME_RISE_HINDU_RISING` |
| 408 | `JME_RISE_MERIDIAN_TRANSIT` |
| 409 | `JME_RISE_NAUTICAL_TWILIGHT` |
| 410 | `JME_RISE_NO_REFRACTION` |
| 411 | `JME_RISE_RISE` |
| 412 | `JME_RISE_SET` |
| 413 | `JME_SECONDS_PER_DAY` |
| 414 | `JME_SIDEREAL_ALDEBARAN_15TAU` |
| 415 | `JME_SIDEREAL_ARYABHATA` |
| 416 | `JME_SIDEREAL_B1950` |
| 417 | `JME_SIDEREAL_BABYL_ETPSC` |
| 418 | `JME_SIDEREAL_BABYL_HUBER` |
| 419 | `JME_SIDEREAL_BABYL_KUGLER1` |
| 420 | `JME_SIDEREAL_BABYL_KUGLER2` |
| 421 | `JME_SIDEREAL_BABYL_KUGLER3` |
| 422 | `JME_SIDEREAL_DELUCE` |
| 423 | `JME_SIDEREAL_FAGAN_BRADLEY` |
| 424 | `JME_SIDEREAL_GALCENT_0SAG` |
| 425 | `JME_SIDEREAL_HIPPARCHOS` |
| 426 | `JME_SIDEREAL_J1900` |
| 427 | `JME_SIDEREAL_J2000` |
| 428 | `JME_SIDEREAL_JN_BHASIN` |
| 429 | `JME_SIDEREAL_KRISHNAMURTI` |
| 430 | `JME_SIDEREAL_LAHIRI` |
| 431 | `JME_SIDEREAL_RAMAN` |
| 432 | `JME_SIDEREAL_SASSANIAN` |
| 433 | `JME_SIDEREAL_SS_CITRA` |
| 434 | `JME_SIDEREAL_SS_REVATI` |
| 435 | `JME_SIDEREAL_SURYASIDDHANTA` |
| 436 | `JME_SIDEREAL_TRUE_CITRA` |
| 437 | `JME_SIDEREAL_TRUE_MULA` |
| 438 | `JME_SIDEREAL_TRUE_PUSHYA` |
| 439 | `JME_SIDEREAL_TRUE_REVATI` |
| 440 | `JME_SIDEREAL_USER` |
| 441 | `JME_SIDEREAL_USHASHASHI` |
| 442 | `JME_SIDEREAL_YUKTESHWAR` |
| 443 | `JME_SPEED_OF_LIGHT_KM_PER_SEC` |
| 444 | `JME_TIME_DELTAT_AUTOMATIC` |
| 445 | `JME_TIME_TIDAL_AUTOMATIC` |
| 446 | `JME_TIME_TIDAL_DE200` |
| 447 | `JME_TIME_TIDAL_DE403` |
| 448 | `JME_TIME_TIDAL_DE404` |
| 449 | `JME_TIME_TIDAL_DE405` |
| 450 | `JME_TIME_TIDAL_DE406` |
| 451 | `JME_TIME_TIDAL_DE421` |
| 452 | `JME_TIME_TIDAL_DE430` |
| 453 | `JME_TIME_TIDAL_DE431` |
| 454 | `JME_TIME_TIDAL_DE441` |
| 455 | `JME_VECTOR_AU_PER_DAY` |
| 456 | `JME_VECTOR_AU_PER_SECOND` |
| 457 | `JME_VECTOR_KM_PER_DAY` |
| 458 | `JME_VECTOR_KM_PER_SECOND` |
| 459 | `JME_VERSION` |
| 460 | `JME_VERSION_ID` |

## Reference Parity Constants

| # | Reference constant |
|---:|---|
| 1 | `ERR` |
| 2 | `OK` |
| 3 | `SE_ACRONYCHAL_RISING` |
| 4 | `SE_ACRONYCHAL_SETTING` |
| 5 | `SE_ADMETOS` |
| 6 | `SE_APOLLON` |
| 7 | `SE_APP_TO_TRUE` |
| 8 | `SE_ARMC` |
| 9 | `SE_ASC` |
| 10 | `SE_AST_OFFSET` |
| 11 | `SE_ASTNAMFILE` |
| 12 | `SE_AUNIT_TO_KM` |
| 13 | `SE_AUNIT_TO_LIGHTYEAR` |
| 14 | `SE_AUNIT_TO_PARSEC` |
| 15 | `SE_BIT_ASTRO_TWILIGHT` |
| 16 | `SE_BIT_CIVIL_TWILIGHT` |
| 17 | `SE_BIT_DISC_BOTTOM` |
| 18 | `SE_BIT_DISC_CENTER` |
| 19 | `SE_BIT_FIXED_DISC_SIZE` |
| 20 | `SE_BIT_FORCE_SLOW_METHOD` |
| 21 | `SE_BIT_GEOCTR_NO_ECL_LAT` |
| 22 | `SE_BIT_HINDU_RISING` |
| 23 | `SE_BIT_NAUTIC_TWILIGHT` |
| 24 | `SE_BIT_NO_REFRACTION` |
| 25 | `SE_CALC_ITRANSIT` |
| 26 | `SE_CALC_MTRANSIT` |
| 27 | `SE_CALC_RISE` |
| 28 | `SE_CALC_SET` |
| 29 | `SE_CERES` |
| 30 | `SE_CHIRON` |
| 31 | `SE_COASC1` |
| 32 | `SE_COASC2` |
| 33 | `SE_COMET_OFFSET` |
| 34 | `SE_COSMICAL_SETTING` |
| 35 | `SE_CUPIDO` |
| 36 | `SE_DE_NUMBER` |
| 37 | `SE_DELTAT_AUTOMATIC` |
| 38 | `SE_EARTH` |
| 39 | `SE_ECL_1ST_VISIBLE` |
| 40 | `SE_ECL_2ND_VISIBLE` |
| 41 | `SE_ECL_3RD_VISIBLE` |
| 42 | `SE_ECL_4TH_VISIBLE` |
| 43 | `SE_ECL_ALLTYPES_LUNAR` |
| 44 | `SE_ECL_ALLTYPES_SOLAR` |
| 45 | `SE_ECL_ANNULAR` |
| 46 | `SE_ECL_ANNULAR_TOTAL` |
| 47 | `SE_ECL_CENTRAL` |
| 48 | `SE_ECL_HYBRID` |
| 49 | `SE_ECL_MAX_VISIBLE` |
| 50 | `SE_ECL_NONCENTRAL` |
| 51 | `SE_ECL_NUT` |
| 52 | `SE_ECL_OCC_BEG_DAYLIGHT` |
| 53 | `SE_ECL_OCC_END_DAYLIGHT` |
| 54 | `SE_ECL_ONE_TRY` |
| 55 | `SE_ECL_PARTBEG_VISIBLE` |
| 56 | `SE_ECL_PARTEND_VISIBLE` |
| 57 | `SE_ECL_PARTIAL` |
| 58 | `SE_ECL_PENUMBBEG_VISIBLE` |
| 59 | `SE_ECL_PENUMBEND_VISIBLE` |
| 60 | `SE_ECL_PENUMBRAL` |
| 61 | `SE_ECL_TOTAL` |
| 62 | `SE_ECL_TOTBEG_VISIBLE` |
| 63 | `SE_ECL_TOTEND_VISIBLE` |
| 64 | `SE_ECL_VISIBLE` |
| 65 | `SE_ECL2HOR` |
| 66 | `SE_EPHE_PATH` |
| 67 | `SE_EQU2HOR` |
| 68 | `SE_EQUASC` |
| 69 | `SE_EVENING_FIRST` |
| 70 | `SE_EVENING_LAST` |
| 71 | `SE_FICT_MAX` |
| 72 | `SE_FICT_OFFSET` |
| 73 | `SE_FICT_OFFSET_1` |
| 74 | `SE_FICTFILE` |
| 75 | `SE_FIXSTAR` |
| 76 | `SE_FNAME_DE200` |
| 77 | `SE_FNAME_DE403` |
| 78 | `SE_FNAME_DE404` |
| 79 | `SE_FNAME_DE405` |
| 80 | `SE_FNAME_DE406` |
| 81 | `SE_FNAME_DE431` |
| 82 | `SE_FNAME_DFT` |
| 83 | `SE_FNAME_DFT2` |
| 84 | `SE_GREG_CAL` |
| 85 | `SE_HADES` |
| 86 | `SE_HARRINGTON` |
| 87 | `SE_HELFLAG_AV` |
| 88 | `SE_HELFLAG_AVKIND` |
| 89 | `SE_HELFLAG_AVKIND_MIN7` |
| 90 | `SE_HELFLAG_AVKIND_MIN9` |
| 91 | `SE_HELFLAG_AVKIND_PTO` |
| 92 | `SE_HELFLAG_AVKIND_VR` |
| 93 | `SE_HELFLAG_HIGH_PRECISION` |
| 94 | `SE_HELFLAG_LONG_SEARCH` |
| 95 | `SE_HELFLAG_NO_DETAILS` |
| 96 | `SE_HELFLAG_OPTICAL_PARAMS` |
| 97 | `SE_HELFLAG_SEARCH_1_PERIOD` |
| 98 | `SE_HELFLAG_VISLIM_DARK` |
| 99 | `SE_HELFLAG_VISLIM_NOMOON` |
| 100 | `SE_HELFLAG_VISLIM_PHOTOPIC` |
| 101 | `SE_HELFLAG_VISLIM_SCOTOPIC` |
| 102 | `SE_HELIACAL_AVKIND` |
| 103 | `SE_HELIACAL_AVKIND_MIN7` |
| 104 | `SE_HELIACAL_AVKIND_MIN9` |
| 105 | `SE_HELIACAL_AVKIND_PTO` |
| 106 | `SE_HELIACAL_AVKIND_VR` |
| 107 | `SE_HELIACAL_HIGH_PRECISION` |
| 108 | `SE_HELIACAL_LONG_SEARCH` |
| 109 | `SE_HELIACAL_NO_DETAILS` |
| 110 | `SE_HELIACAL_OPTICAL_PARAMS` |
| 111 | `SE_HELIACAL_RISING` |
| 112 | `SE_HELIACAL_SEARCH_1_PERIOD` |
| 113 | `SE_HELIACAL_SETTING` |
| 114 | `SE_HELIACAL_VISLIM_DARK` |
| 115 | `SE_HELIACAL_VISLIM_NOMOON` |
| 116 | `SE_HELIACAL_VISLIM_PHOTOPIC` |
| 117 | `SE_HOR2ECL` |
| 118 | `SE_HOR2EQU` |
| 119 | `SE_HOUSES_ALCABITUS` |
| 120 | `SE_HOUSES_CAMPANO` |
| 121 | `SE_HOUSES_EQUAL` |
| 122 | `SE_HOUSES_EQUAL_VEHIC` |
| 123 | `SE_HOUSES_KOCH` |
| 124 | `SE_HOUSES_KRUSINSKI` |
| 125 | `SE_HOUSES_MORINUS` |
| 126 | `SE_HOUSES_PLACIDUS` |
| 127 | `SE_HOUSES_POLICH_PAGE` |
| 128 | `SE_HOUSES_PORPHYRIUS` |
| 129 | `SE_HOUSES_REGIOMONTANUS` |
| 130 | `SE_INTP_APOG` |
| 131 | `SE_INTP_PERG` |
| 132 | `SE_ISIS` |
| 133 | `SE_JUL_CAL` |
| 134 | `SE_JUNO` |
| 135 | `SE_JUPITER` |
| 136 | `SE_KRONOS` |
| 137 | `SE_MARS` |
| 138 | `SE_MAX_STNAME` |
| 139 | `SE_MC` |
| 140 | `SE_MEAN_APOG` |
| 141 | `SE_MEAN_NODE` |
| 142 | `SE_MERCURY` |
| 143 | `SE_MIXEDOPIC_FLAG` |
| 144 | `SE_MODEL_BIAS` |
| 145 | `SE_MODEL_DELTAT` |
| 146 | `SE_MODEL_JPLHOR_MODE` |
| 147 | `SE_MODEL_JPLHORA_MODE` |
| 148 | `SE_MODEL_NUT` |
| 149 | `SE_MODEL_PREC_LONGTERM` |
| 150 | `SE_MODEL_PREC_SHORTTERM` |
| 151 | `SE_MODEL_SIDT` |
| 152 | `SE_MOON` |
| 153 | `SE_MORNING_FIRST` |
| 154 | `SE_MORNING_LAST` |
| 155 | `SE_NALL_NAT_POINTS` |
| 156 | `SE_NASCMC` |
| 157 | `SE_NEPTUNE` |
| 158 | `SE_NEPTUNE_ADAMS` |
| 159 | `SE_NEPTUNE_LEVERRIER` |
| 160 | `SE_NFICT_ELEM` |
| 161 | `SE_NIBIRU` |
| 162 | `SE_NODBIT_FOPOINT` |
| 163 | `SE_NODBIT_MEAN` |
| 164 | `SE_NODBIT_OSCU` |
| 165 | `SE_NODBIT_OSCU_BAR` |
| 166 | `SE_NPLANETS` |
| 167 | `SE_NSIDM_PREDEF` |
| 168 | `SE_OSCU_APOG` |
| 169 | `SE_PALLAS` |
| 170 | `SE_PHOLUS` |
| 171 | `SE_PHOTOPIC_FLAG` |
| 172 | `SE_PLMOON_OFFSET` |
| 173 | `SE_PLUTO` |
| 174 | `SE_PLUTO_LOWELL` |
| 175 | `SE_PLUTO_PICKERING` |
| 176 | `SE_POLASC` |
| 177 | `SE_POSEIDON` |
| 178 | `SE_PROSERPINA` |
| 179 | `SE_SATURN` |
| 180 | `SE_SCOTOPIC_FLAG` |
| 181 | `SE_SIDBIT_ECL_DATE` |
| 182 | `SE_SIDBIT_ECL_T0` |
| 183 | `SE_SIDBIT_NO_PREC_OFFSET` |
| 184 | `SE_SIDBIT_PREC_ORIG` |
| 185 | `SE_SIDBIT_SSY_PLANE` |
| 186 | `SE_SIDBIT_USER_UT` |
| 187 | `SE_SIDBITS` |
| 188 | `SE_SIDM_ALDEBARAN_15TAU` |
| 189 | `SE_SIDM_ARYABHATA` |
| 190 | `SE_SIDM_ARYABHATA_522` |
| 191 | `SE_SIDM_ARYABHATA_MSUN` |
| 192 | `SE_SIDM_B1950` |
| 193 | `SE_SIDM_BABYL_BRITTON` |
| 194 | `SE_SIDM_BABYL_ETPSC` |
| 195 | `SE_SIDM_BABYL_HUBER` |
| 196 | `SE_SIDM_BABYL_KUGLER1` |
| 197 | `SE_SIDM_BABYL_KUGLER2` |
| 198 | `SE_SIDM_BABYL_KUGLER3` |
| 199 | `SE_SIDM_DELUCE` |
| 200 | `SE_SIDM_DJWHAL_KHUL` |
| 201 | `SE_SIDM_FAGAN_BRADLEY` |
| 202 | `SE_SIDM_GALALIGN_MARDYKS` |
| 203 | `SE_SIDM_GALCENT_0SAG` |
| 204 | `SE_SIDM_GALCENT_COCHRANE` |
| 205 | `SE_SIDM_GALCENT_MULA_WILHELM` |
| 206 | `SE_SIDM_GALCENT_RGILBRAND` |
| 207 | `SE_SIDM_GALEQU_FIORENZA` |
| 208 | `SE_SIDM_GALEQU_IAU1958` |
| 209 | `SE_SIDM_GALEQU_MULA` |
| 210 | `SE_SIDM_GALEQU_TRUE` |
| 211 | `SE_SIDM_HIPPARCHOS` |
| 212 | `SE_SIDM_J1900` |
| 213 | `SE_SIDM_J2000` |
| 214 | `SE_SIDM_JN_BHASIN` |
| 215 | `SE_SIDM_KRISHNAMURTI` |
| 216 | `SE_SIDM_KRISHNAMURTI_VP291` |
| 217 | `SE_SIDM_LAHIRI` |
| 218 | `SE_SIDM_LAHIRI_1940` |
| 219 | `SE_SIDM_LAHIRI_ICRC` |
| 220 | `SE_SIDM_LAHIRI_VP285` |
| 221 | `SE_SIDM_RAMAN` |
| 222 | `SE_SIDM_SASSANIAN` |
| 223 | `SE_SIDM_SS_CITRA` |
| 224 | `SE_SIDM_SS_REVATI` |
| 225 | `SE_SIDM_SURYASIDDHANTA` |
| 226 | `SE_SIDM_SURYASIDDHANTA_MSUN` |
| 227 | `SE_SIDM_TRUE_CITRA` |
| 228 | `SE_SIDM_TRUE_MULA` |
| 229 | `SE_SIDM_TRUE_PUSHYA` |
| 230 | `SE_SIDM_TRUE_REVATI` |
| 231 | `SE_SIDM_TRUE_SHEORAN` |
| 232 | `SE_SIDM_USER` |
| 233 | `SE_SIDM_USHASHASHI` |
| 234 | `SE_SIDM_VALENS_MOON` |
| 235 | `SE_SIDM_YUKTESHWAR` |
| 236 | `SE_SPLIT_DEG_KEEP_DEG` |
| 237 | `SE_SPLIT_DEG_KEEP_SIGN` |
| 238 | `SE_SPLIT_DEG_NAKSHATRA` |
| 239 | `SE_SPLIT_DEG_ROUND_DEG` |
| 240 | `SE_SPLIT_DEG_ROUND_MIN` |
| 241 | `SE_SPLIT_DEG_ROUND_SEC` |
| 242 | `SE_SPLIT_DEG_ZODIACAL` |
| 243 | `SE_STARFILE` |
| 244 | `SE_STARFILE_OLD` |
| 245 | `SE_SUN` |
| 246 | `SE_TIDAL_26` |
| 247 | `SE_TIDAL_AUTOMATIC` |
| 248 | `SE_TIDAL_DE200` |
| 249 | `SE_TIDAL_DE403` |
| 250 | `SE_TIDAL_DE404` |
| 251 | `SE_TIDAL_DE405` |
| 252 | `SE_TIDAL_DE406` |
| 253 | `SE_TIDAL_DE421` |
| 254 | `SE_TIDAL_DE422` |
| 255 | `SE_TIDAL_DE430` |
| 256 | `SE_TIDAL_DE431` |
| 257 | `SE_TIDAL_DE441` |
| 258 | `SE_TIDAL_DEFAULT` |
| 259 | `SE_TIDAL_JPLEPH` |
| 260 | `SE_TIDAL_MOSEPH` |
| 261 | `SE_TIDAL_STEPHENSON_2016` |
| 262 | `SE_TIDAL_SWIEPH` |
| 263 | `SE_TRUE_NODE` |
| 264 | `SE_TRUE_TO_APP` |
| 265 | `SE_URANUS` |
| 266 | `SE_VARUNA` |
| 267 | `SE_VENUS` |
| 268 | `SE_VERTEX` |
| 269 | `SE_VESTA` |
| 270 | `SE_VULCAN` |
| 271 | `SE_VULKANUS` |
| 272 | `SE_WALDEMATH` |
| 273 | `SE_WHITE_MOON` |
| 274 | `SE_ZEUS` |
| 275 | `SEFLG_ASTROMETRIC` |
| 276 | `SEFLG_BARYCTR` |
| 277 | `SEFLG_CENTER_BODY` |
| 278 | `SEFLG_DEFAULTEPH` |
| 279 | `SEFLG_DPSIDEPS_1980` |
| 280 | `SEFLG_EQUATORIAL` |
| 281 | `SEFLG_HELCTR` |
| 282 | `SEFLG_ICRS` |
| 283 | `SEFLG_J2000` |
| 284 | `SEFLG_JPLEPH` |
| 285 | `SEFLG_JPLHOR` |
| 286 | `SEFLG_JPLHOR_APPROX` |
| 287 | `SEFLG_MOSEPH` |
| 288 | `SEFLG_NOABERR` |
| 289 | `SEFLG_NOGDEFL` |
| 290 | `SEFLG_NONUT` |
| 291 | `SEFLG_ORBEL_AA` |
| 292 | `SEFLG_RADIANS` |
| 293 | `SEFLG_SIDEREAL` |
| 294 | `SEFLG_SPEED` |
| 295 | `SEFLG_SPEED3` |
| 296 | `SEFLG_SWIEPH` |
| 297 | `SEFLG_TEST_PLMOON` |
| 298 | `SEFLG_TOPOCTR` |
| 299 | `SEFLG_TROPICAL` |
| 300 | `SEFLG_TRUEPOS` |
| 301 | `SEFLG_XYZ` |
| 302 | `SEMOD_BIAS_DEFAULT` |
| 303 | `SEMOD_BIAS_IAU2000` |
| 304 | `SEMOD_BIAS_IAU2006` |
| 305 | `SEMOD_BIAS_NONE` |
| 306 | `SEMOD_DELTAT_DEFAULT` |
| 307 | `SEMOD_DELTAT_ESPENAK_MEEUS_2006` |
| 308 | `SEMOD_DELTAT_STEPHENSON_1997` |
| 309 | `SEMOD_DELTAT_STEPHENSON_ETC_2016` |
| 310 | `SEMOD_DELTAT_STEPHENSON_MORRISON_1984` |
| 311 | `SEMOD_DELTAT_STEPHENSON_MORRISON_2004` |
| 312 | `SEMOD_JPLHOR_DEFAULT` |
| 313 | `SEMOD_JPLHOR_LONG_AGREEMENT` |
| 314 | `SEMOD_JPLHORA_1` |
| 315 | `SEMOD_JPLHORA_2` |
| 316 | `SEMOD_JPLHORA_3` |
| 317 | `SEMOD_JPLHORA_DEFAULT` |
| 318 | `SEMOD_NBIAS` |
| 319 | `SEMOD_NDELTAT` |
| 320 | `SEMOD_NJPLHOR` |
| 321 | `SEMOD_NJPLHORA` |
| 322 | `SEMOD_NNUT` |
| 323 | `SEMOD_NPREC` |
| 324 | `SEMOD_NSIDT` |
| 325 | `SEMOD_NUT_DEFAULT` |
| 326 | `SEMOD_NUT_IAU_1980` |
| 327 | `SEMOD_NUT_IAU_2000A` |
| 328 | `SEMOD_NUT_IAU_2000B` |
| 329 | `SEMOD_NUT_IAU_CORR_1987` |
| 330 | `SEMOD_NUT_WOOLARD` |
| 331 | `SEMOD_PREC_BRETAGNON_2003` |
| 332 | `SEMOD_PREC_DEFAULT` |
| 333 | `SEMOD_PREC_DEFAULT_SHORT` |
| 334 | `SEMOD_PREC_IAU_1976` |
| 335 | `SEMOD_PREC_IAU_2000` |
| 336 | `SEMOD_PREC_IAU_2006` |
| 337 | `SEMOD_PREC_LASKAR_1986` |
| 338 | `SEMOD_PREC_NEWCOMB` |
| 339 | `SEMOD_PREC_OWEN_1990` |
| 340 | `SEMOD_PREC_SIMON_1994` |
| 341 | `SEMOD_PREC_VONDRAK_2011` |
| 342 | `SEMOD_PREC_WILL_EPS_LASK` |
| 343 | `SEMOD_PREC_WILLIAMS_1994` |
| 344 | `SEMOD_SIDT_DEFAULT` |
| 345 | `SEMOD_SIDT_IAU_1976` |
| 346 | `SEMOD_SIDT_IAU_2006` |
| 347 | `SEMOD_SIDT_IERS_CONV_2010` |
| 348 | `SEMOD_SIDT_LONGTERM` |
