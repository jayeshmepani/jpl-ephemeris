# Reference Function Coverage

This file tracks the 106 reference behavior rows separately from the project-owned `jme_*` API count.

The product API remains `jme_*`. Mapping a reference row to a `jme_*` function is not the same thing as full behavioral completion. A row is only complete when the implementation is real, tested, and has no known missing algorithm branches for that reference behavior.

## Current Summary

| Area | Count |
|---|---:|
| Reference behavior rows tracked | 106 |
| Rows mapped to project-owned `jme_*` APIs | 106 |
| Rows fully behavior-complete | Not yet 106 |
| Rows still partial or open | Yes |
| Reference constants tracked | 348 |
| JME public functions currently defined | 191 |

## Behavioral Status

The 348 reference constants are tracked. The 106 function behavior rows are not all fully complete yet.

Currently complete areas are mostly metadata, date/time conversion, angle/string utilities, coordinate transforms, basic refraction, state-vector helpers, and raw JPL/CALCEPH boundary functions when CALCEPH is present.

Currently partial areas include `jme_calc`, `jme_calc_ut`, `jme_calc_pctr`, fixed stars, ayanamsa, houses, rise/set/transit, longitude crossings, eclipse searches, physical phenomena, VSOP87, Moshier, and Meeus fallbacks.

Currently open or effectively contract-only areas include heliacal visibility, visual limiting magnitude, eclipse circumstances/locality, lunar occultations, Gauquelin sector, nodes/apsides, orbital elements, orbit distance extrema, and full ELP2000 lunar integration.

## Coverage Matrix

| # | Reference behavior row | Mapping status | Swiss -> JME map | JME coverage |
|---:|---|---|---|---|
| 1 | `swe_heliacal_ut` | Covered | `jme_heliacal_ut` | Heliacal event search remains a dedicated domain. |
| 2 | `swe_heliacal_pheno_ut` | Covered | `jme_heliacal_pheno_ut` | Heliacal phenomenon details remain a dedicated domain. |
| 3 | `swe_vis_limit_mag` | Covered | `jme_vis_limit_mag` | Visual limiting magnitude remains a dedicated domain. |
| 4 | `swe_heliacal_angle` | Covered | `jme_heliacal_angle` | Heliacal angle geometry remains a dedicated domain. |
| 5 | `swe_topo_arcus_visionis` | Covered | `jme_topo_arcus_visionis` | Topocentric arcus visionis remains a dedicated domain. |
| 6 | `swe_set_astro_models` | Covered | `jme_set_astro_models` | Model selection constants exist; model state selection needs full behavior. |
| 7 | `swe_get_astro_models` | Covered | `jme_get_astro_models` | Model state query needs full behavior. |
| 8 | `swe_version` | Covered | `jme_version` | Version string export. |
| 9 | `swe_get_library_path` | Covered | `jme_library_path` | Library path export. |
| 10 | `swe_calc` | Covered | `jme_calc` | Full flag-driven planet calculation pipeline remains a dedicated domain. |
| 11 | `swe_calc_ut` | Covered | `jme_calc_ut` | UT wrapper for the full calculation pipeline remains a dedicated domain. |
| 12 | `swe_calc_pctr` | Covered | `jme_calc_pctr` | Planet-center calculation remains a dedicated domain. |
| 13 | `swe_solcross` | Covered | `jme_solcross` | Solar longitude crossing search remains a dedicated domain. |
| 14 | `swe_solcross_ut` | Covered | `jme_solcross_ut` | UT solar longitude crossing search remains a dedicated domain. |
| 15 | `swe_mooncross` | Covered | `jme_mooncross` | Lunar longitude crossing search remains a dedicated domain. |
| 16 | `swe_mooncross_ut` | Covered | `jme_mooncross_ut` | UT lunar longitude crossing search remains a dedicated domain. |
| 17 | `swe_mooncross_node` | Covered | `jme_mooncross_node` | Lunar node crossing search remains a dedicated domain. |
| 18 | `swe_mooncross_node_ut` | Covered | `jme_mooncross_node_ut` | UT lunar node crossing search remains a dedicated domain. |
| 19 | `swe_helio_cross` | Covered | `jme_helio_cross` | Heliocentric crossing search remains a dedicated domain. |
| 20 | `swe_helio_cross_ut` | Covered | `jme_helio_cross_ut` | UT heliocentric crossing search remains a dedicated domain. |
| 21 | `swe_fixstar` | Covered | `jme_fixstar` | Fixed-star catalog calculation remains a dedicated domain. |
| 22 | `swe_fixstar_ut` | Covered | `jme_fixstar_ut` | UT fixed-star calculation remains a dedicated domain. |
| 23 | `swe_fixstar_mag` | Covered | `jme_fixstar_mag` | Fixed-star magnitude lookup remains a dedicated domain. |
| 24 | `swe_fixstar2` | Covered | `jme_fixstar2` | Alternate fixed-star catalog calculation remains a dedicated domain. |
| 25 | `swe_fixstar2_ut` | Covered | `jme_fixstar2_ut` | UT alternate fixed-star calculation remains a dedicated domain. |
| 26 | `swe_fixstar2_mag` | Covered | `jme_fixstar2_mag` | Alternate fixed-star magnitude lookup remains a dedicated domain. |
| 27 | `swe_close` | Covered | `jme_close`, `jme_jpl_close` | Runtime and JPL resource close. |
| 28 | `swe_set_ephe_path` | Covered | `jme_set_ephemeris_path`, `jme_ephemeris_path` | Ephemeris path state. |
| 29 | `swe_set_jpl_file` | Covered | `jme_set_jpl_file`, `jme_jpl_file` | JPL file state. |
| 30 | `swe_get_planet_name` | Covered | `jme_body_name`, `jme_copy_body_name` | Body name lookup and copy. |
| 31 | `swe_set_topo` | Covered | `jme_set_topo` | Topocentric observer state remains a dedicated domain. |
| 32 | `swe_set_sid_mode` | Covered | `jme_set_sidereal_mode`, `jme_get_sidereal_mode` | Sidereal mode state. |
| 33 | `swe_get_ayanamsa_ex` | Covered | `jme_get_ayanamsa_ex` | Ayanamsa computation with flags remains a dedicated domain. |
| 34 | `swe_get_ayanamsa_ex_ut` | Covered | `jme_get_ayanamsa_ex_ut` | UT ayanamsa computation with flags remains a dedicated domain. |
| 35 | `swe_get_ayanamsa` | Covered | `jme_get_ayanamsa` | Ayanamsa computation remains a dedicated domain. |
| 36 | `swe_get_ayanamsa_ut` | Covered | `jme_get_ayanamsa_ut` | UT ayanamsa computation remains a dedicated domain. |
| 37 | `swe_get_ayanamsa_name` | Covered | `jme_get_ayanamsa_name` | Ayanamsa naming remains a dedicated domain. |
| 38 | `swe_get_current_file_data` | Covered | `jme_jpl_current_file_data` | Current JPL file metadata. |
| 39 | `swe_date_conversion` | Covered | `jme_date_is_valid`, `jme_julian_day`, `jme_reverse_julian_day` | Calendar validation and Julian conversion. |
| 40 | `swe_julday` | Covered | `jme_julian_day` | Julian day conversion. |
| 41 | `swe_revjul` | Covered | `jme_reverse_julian_day` | Reverse Julian day conversion. |
| 42 | `swe_utc_to_jd` | Covered | `jme_utc_to_jd` | UTC to Julian day conversion. |
| 43 | `swe_jdet_to_utc` | Covered | `jme_jd_to_utc` | ET Julian day to UTC fields. |
| 44 | `swe_jdut1_to_utc` | Covered | `jme_jd_to_utc` | UT1 Julian day to UTC fields. |
| 45 | `swe_utc_time_zone` | Covered | `jme_utc_time_zone` | Time-zone conversion. |
| 46 | `swe_houses` | Covered | `jme_houses` | House cusp computation remains a dedicated domain. |
| 47 | `swe_houses_ex` | Covered | `jme_houses_ex` | House cusp computation with flags remains a dedicated domain. |
| 48 | `swe_houses_ex2` | Covered | `jme_houses_ex2` | House cusp computation with declination/speed extras remains a dedicated domain. |
| 49 | `swe_houses_armc` | Covered | `jme_houses_armc` | ARMC-based house computation remains a dedicated domain. |
| 50 | `swe_houses_armc_ex2` | Covered | `jme_houses_armc_ex2` | ARMC-based extended house computation remains a dedicated domain. |
| 51 | `swe_house_pos` | Covered | `jme_house_pos` | Position-to-house calculation remains a dedicated domain. |
| 52 | `swe_house_name` | Covered | `jme_house_system_name` | House system naming. |
| 53 | `swe_gauquelin_sector` | Covered | `jme_gauquelin_sector` | Gauquelin sector calculation remains a dedicated domain. |
| 54 | `swe_sol_eclipse_where` | Covered | `jme_sol_eclipse_where` | Solar eclipse locality calculation remains a dedicated domain. |
| 55 | `swe_lun_occult_where` | Covered | `jme_lun_occult_where` | Lunar occultation locality calculation remains a dedicated domain. |
| 56 | `swe_sol_eclipse_how` | Covered | `jme_sol_eclipse_how` | Local solar eclipse circumstances remain a dedicated domain. |
| 57 | `swe_sol_eclipse_when_loc` | Covered | `jme_sol_eclipse_when_loc` | Local solar eclipse search remains a dedicated domain. |
| 58 | `swe_lun_occult_when_loc` | Covered | `jme_lun_occult_when_loc` | Local lunar occultation search remains a dedicated domain. |
| 59 | `swe_sol_eclipse_when_glob` | Covered | `jme_sol_eclipse_when_glob` | Global solar eclipse search remains a dedicated domain. |
| 60 | `swe_lun_occult_when_glob` | Covered | `jme_lun_occult_when_glob` | Global lunar occultation search remains a dedicated domain. |
| 61 | `swe_lun_eclipse_how` | Covered | `jme_lun_eclipse_how` | Local lunar eclipse circumstances remain a dedicated domain. |
| 62 | `swe_lun_eclipse_when` | Covered | `jme_lun_eclipse_when` | Lunar eclipse search remains a dedicated domain. |
| 63 | `swe_lun_eclipse_when_loc` | Covered | `jme_lun_eclipse_when_loc` | Local lunar eclipse search remains a dedicated domain. |
| 64 | `swe_pheno` | Covered | `jme_pheno` | Planetary phenomenon calculation remains a dedicated domain. |
| 65 | `swe_pheno_ut` | Covered | `jme_pheno_ut` | UT planetary phenomenon calculation remains a dedicated domain. |
| 66 | `swe_refrac` | Covered | `jme_refract` | Atmospheric refraction. |
| 67 | `swe_refrac_extended` | Covered | `jme_refract_extended` | Extended atmospheric refraction. |
| 68 | `swe_set_lapse_rate` | Covered | `jme_set_lapse_rate` | Refraction lapse-rate state remains a dedicated domain. |
| 69 | `swe_azalt` | Covered | `jme_equatorial_to_horizontal` | Azimuth/altitude conversion. |
| 70 | `swe_azalt_rev` | Covered | `jme_horizontal_to_equatorial` | Reverse azimuth/altitude conversion. |
| 71 | `swe_rise_trans_true_hor` | Covered | `jme_rise_trans_true_hor` | Rise, set, and transit search with true horizon remains a dedicated domain. |
| 72 | `swe_rise_trans` | Covered | `jme_rise_trans` | Rise, set, and transit search remains a dedicated domain. |
| 73 | `swe_nod_aps` | Covered | `jme_nod_aps` | Nodes and apsides computation remains a dedicated domain. |
| 74 | `swe_nod_aps_ut` | Covered | `jme_nod_aps_ut` | UT nodes and apsides computation remains a dedicated domain. |
| 75 | `swe_get_orbital_elements` | Covered | `jme_get_orbital_elements` | Orbital element derivation remains a dedicated domain. |
| 76 | `swe_orbit_max_min_true_distance` | Covered | `jme_orbit_max_min_true_distance` | Orbit distance extrema remain a dedicated domain. |
| 77 | `swe_deltat` | Covered | `jme_delta_t` | Delta T model remains a dedicated domain. |
| 78 | `swe_deltat_ex` | Covered | `jme_delta_t_ex` | Delta T model with flags remains a dedicated domain. |
| 79 | `swe_time_equ` | Covered | `jme_time_equ` | Equation of time remains a dedicated domain. |
| 80 | `swe_lmt_to_lat` | Covered | `jme_lmt_to_lat` | Local mean time to local apparent time remains a dedicated domain. |
| 81 | `swe_lat_to_lmt` | Covered | `jme_lat_to_lmt` | Local apparent time to local mean time remains a dedicated domain. |
| 82 | `swe_sidtime0` | Covered | `jme_sidereal_time0` | Sidereal time with explicit obliquity/nutation. |
| 83 | `swe_sidtime` | Covered | `jme_sidereal_time` | Sidereal time. |
| 84 | `swe_set_interpolate_nut` | Covered | `jme_set_interpolate_nut` | Nutation interpolation selection remains a dedicated domain. |
| 85 | `swe_cotrans` | Covered | `jme_ecliptic_to_equatorial`, `jme_equatorial_to_ecliptic` | Coordinate conversion. |
| 86 | `swe_cotrans_sp` | Covered | `jme_ecliptic_to_equatorial_rectangular_state`, `jme_equatorial_to_ecliptic_rectangular_state` | State-vector coordinate conversion. |
| 87 | `swe_get_tid_acc` | Covered | `jme_get_tid_acc` | Tidal acceleration state query remains a dedicated domain. |
| 88 | `swe_set_tid_acc` | Covered | `jme_set_tid_acc` | Tidal acceleration state selection remains a dedicated domain. |
| 89 | `swe_set_delta_t_userdef` | Covered | `jme_set_delta_t_userdef` | User-defined Delta T state remains a dedicated domain. |
| 90 | `swe_degnorm` | Covered | `jme_degree_normalize` | Degree normalization. |
| 91 | `swe_radnorm` | Covered | `jme_radian_normalize` | Radian normalization. |
| 92 | `swe_rad_midp` | Covered | `jme_radian_midpoint` | Radian midpoint. |
| 93 | `swe_deg_midp` | Covered | `jme_degree_midpoint` | Degree midpoint. |
| 94 | `swe_split_deg` | Covered | `jme_split_degree` | Degree splitting. |
| 95 | `swe_csnorm` | Covered | `jme_centiseconds_normalize` | Centisecond normalization. |
| 96 | `swe_difcsn` | Covered | `jme_centiseconds_difference` | Centisecond difference. |
| 97 | `swe_difdegn` | Covered | `jme_degrees_difference` | Degree difference. |
| 98 | `swe_difcs2n` | Covered | `jme_centiseconds_difference_signed` | Signed centisecond difference. |
| 99 | `swe_difdeg2n` | Covered | `jme_degrees_difference_signed` | Signed degree difference. |
| 100 | `swe_difrad2n` | Covered | `jme_radians_difference_signed` | Signed radian difference. |
| 101 | `swe_csroundsec` | Covered | `jme_centiseconds_round_second` | Centisecond rounding to second. |
| 102 | `swe_d2l` | Covered | `jme_double_to_long` | Double-to-integer conversion. |
| 103 | `swe_day_of_week` | Covered | `jme_day_of_week` | Day-of-week conversion. |
| 104 | `swe_cs2timestr` | Covered | `jme_centiseconds_to_time_string` | Centisecond time string formatting remains a dedicated domain. |
| 105 | `swe_cs2lonlatstr` | Covered | `jme_centiseconds_to_lonlat_string` | Centisecond longitude/latitude string formatting remains a dedicated domain. |
| 106 | `swe_cs2degstr` | Covered | `jme_centiseconds_to_degree_string` | Centisecond degree string formatting remains a dedicated domain. |

