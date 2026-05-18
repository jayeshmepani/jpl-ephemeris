# Reference Function Coverage

This file tracks the 106 reference behavior rows separately from the project-owned `jme_*` API count.

The product API remains `jme_*`. A row is marked `Covered` only when the behavior area has a real `jme_*` implementation and tests. A row is marked `Open` when the behavior area still needs its own implementation work.

## Current Summary

| Area | Count |
|---|---:|
| Reference behavior rows tracked | 106 |
| Covered rows | 38 |
| Open rows | 68 |
| Reference constants tracked | 348 |
| JME public functions currently defined | 110 |

## Coverage Matrix

| # | Reference behavior row | Status | JME coverage |
|---:|---|---|---|
| 1 | `swe_heliacal_ut` | Open | Heliacal event search remains a dedicated domain. |
| 2 | `swe_heliacal_pheno_ut` | Open | Heliacal phenomenon details remain a dedicated domain. |
| 3 | `swe_vis_limit_mag` | Open | Visual limiting magnitude remains a dedicated domain. |
| 4 | `swe_heliacal_angle` | Open | Heliacal angle geometry remains a dedicated domain. |
| 5 | `swe_topo_arcus_visionis` | Open | Topocentric arcus visionis remains a dedicated domain. |
| 6 | `swe_set_astro_models` | Open | Model selection constants exist; model state selection needs full behavior. |
| 7 | `swe_get_astro_models` | Open | Model state query needs full behavior. |
| 8 | `swe_version` | Covered | `jme_version`. |
| 9 | `swe_get_library_path` | Covered | `jme_library_path`. |
| 10 | `swe_calc` | Open | Full flag-driven planet calculation pipeline remains a dedicated domain. |
| 11 | `swe_calc_ut` | Open | UT wrapper for the full calculation pipeline remains a dedicated domain. |
| 12 | `swe_calc_pctr` | Open | Planet-center calculation remains a dedicated domain. |
| 13 | `swe_solcross` | Open | Solar longitude crossing search remains a dedicated domain. |
| 14 | `swe_solcross_ut` | Open | UT solar longitude crossing search remains a dedicated domain. |
| 15 | `swe_mooncross` | Open | Lunar longitude crossing search remains a dedicated domain. |
| 16 | `swe_mooncross_ut` | Open | UT lunar longitude crossing search remains a dedicated domain. |
| 17 | `swe_mooncross_node` | Open | Lunar node crossing search remains a dedicated domain. |
| 18 | `swe_mooncross_node_ut` | Open | UT lunar node crossing search remains a dedicated domain. |
| 19 | `swe_helio_cross` | Open | Heliocentric crossing search remains a dedicated domain. |
| 20 | `swe_helio_cross_ut` | Open | UT heliocentric crossing search remains a dedicated domain. |
| 21 | `swe_fixstar` | Open | Fixed-star catalog calculation remains a dedicated domain. |
| 22 | `swe_fixstar_ut` | Open | UT fixed-star calculation remains a dedicated domain. |
| 23 | `swe_fixstar_mag` | Open | Fixed-star magnitude lookup remains a dedicated domain. |
| 24 | `swe_fixstar2` | Open | Alternate fixed-star catalog calculation remains a dedicated domain. |
| 25 | `swe_fixstar2_ut` | Open | UT alternate fixed-star calculation remains a dedicated domain. |
| 26 | `swe_fixstar2_mag` | Open | Alternate fixed-star magnitude lookup remains a dedicated domain. |
| 27 | `swe_close` | Covered | `jme_close` and `jme_jpl_close`. |
| 28 | `swe_set_ephe_path` | Covered | `jme_set_ephemeris_path` and `jme_ephemeris_path`. |
| 29 | `swe_set_jpl_file` | Covered | `jme_set_jpl_file` and `jme_jpl_file`. |
| 30 | `swe_get_planet_name` | Covered | `jme_body_name` and `jme_copy_body_name`. |
| 31 | `swe_set_topo` | Open | Topocentric observer state remains a dedicated domain. |
| 32 | `swe_set_sid_mode` | Covered | `jme_set_sidereal_mode` and `jme_get_sidereal_mode`. |
| 33 | `swe_get_ayanamsa_ex` | Open | Ayanamsa computation with flags remains a dedicated domain. |
| 34 | `swe_get_ayanamsa_ex_ut` | Open | UT ayanamsa computation with flags remains a dedicated domain. |
| 35 | `swe_get_ayanamsa` | Open | Ayanamsa computation remains a dedicated domain. |
| 36 | `swe_get_ayanamsa_ut` | Open | UT ayanamsa computation remains a dedicated domain. |
| 37 | `swe_get_ayanamsa_name` | Open | Ayanamsa naming remains a dedicated domain. |
| 38 | `swe_get_current_file_data` | Covered | `jme_jpl_current_file_data`. |
| 39 | `swe_date_conversion` | Covered | `jme_date_is_valid`, `jme_julian_day`, `jme_reverse_julian_day`. |
| 40 | `swe_julday` | Covered | `jme_julian_day`. |
| 41 | `swe_revjul` | Covered | `jme_reverse_julian_day`. |
| 42 | `swe_utc_to_jd` | Covered | `jme_utc_to_jd`. |
| 43 | `swe_jdet_to_utc` | Covered | `jme_jd_to_utc`. |
| 44 | `swe_jdut1_to_utc` | Covered | `jme_jd_to_utc`. |
| 45 | `swe_utc_time_zone` | Covered | `jme_utc_time_zone`. |
| 46 | `swe_houses` | Open | House cusp computation remains a dedicated domain. |
| 47 | `swe_houses_ex` | Open | House cusp computation with flags remains a dedicated domain. |
| 48 | `swe_houses_ex2` | Open | House cusp computation with declination/speed extras remains a dedicated domain. |
| 49 | `swe_houses_armc` | Open | ARMC-based house computation remains a dedicated domain. |
| 50 | `swe_houses_armc_ex2` | Open | ARMC-based extended house computation remains a dedicated domain. |
| 51 | `swe_house_pos` | Open | Position-to-house calculation remains a dedicated domain. |
| 52 | `swe_house_name` | Covered | `jme_house_system_name`. |
| 53 | `swe_gauquelin_sector` | Open | Gauquelin sector calculation remains a dedicated domain. |
| 54 | `swe_sol_eclipse_where` | Open | Solar eclipse locality calculation remains a dedicated domain. |
| 55 | `swe_lun_occult_where` | Open | Lunar occultation locality calculation remains a dedicated domain. |
| 56 | `swe_sol_eclipse_how` | Open | Local solar eclipse circumstances remain a dedicated domain. |
| 57 | `swe_sol_eclipse_when_loc` | Open | Local solar eclipse search remains a dedicated domain. |
| 58 | `swe_lun_occult_when_loc` | Open | Local lunar occultation search remains a dedicated domain. |
| 59 | `swe_sol_eclipse_when_glob` | Open | Global solar eclipse search remains a dedicated domain. |
| 60 | `swe_lun_occult_when_glob` | Open | Global lunar occultation search remains a dedicated domain. |
| 61 | `swe_lun_eclipse_how` | Open | Local lunar eclipse circumstances remain a dedicated domain. |
| 62 | `swe_lun_eclipse_when` | Open | Lunar eclipse search remains a dedicated domain. |
| 63 | `swe_lun_eclipse_when_loc` | Open | Local lunar eclipse search remains a dedicated domain. |
| 64 | `swe_pheno` | Open | Planetary phenomenon calculation remains a dedicated domain. |
| 65 | `swe_pheno_ut` | Open | UT planetary phenomenon calculation remains a dedicated domain. |
| 66 | `swe_refrac` | Covered | `jme_refract`. |
| 67 | `swe_refrac_extended` | Covered | `jme_refract_extended`. |
| 68 | `swe_set_lapse_rate` | Open | Refraction lapse-rate state remains a dedicated domain. |
| 69 | `swe_azalt` | Covered | `jme_equatorial_to_horizontal`. |
| 70 | `swe_azalt_rev` | Covered | `jme_horizontal_to_equatorial`. |
| 71 | `swe_rise_trans_true_hor` | Open | Rise, set, and transit search with true horizon remains a dedicated domain. |
| 72 | `swe_rise_trans` | Open | Rise, set, and transit search remains a dedicated domain. |
| 73 | `swe_nod_aps` | Open | Nodes and apsides computation remains a dedicated domain. |
| 74 | `swe_nod_aps_ut` | Open | UT nodes and apsides computation remains a dedicated domain. |
| 75 | `swe_get_orbital_elements` | Open | Orbital element derivation remains a dedicated domain. |
| 76 | `swe_orbit_max_min_true_distance` | Open | Orbit distance extrema remain a dedicated domain. |
| 77 | `swe_deltat` | Open | Delta T model remains a dedicated domain. |
| 78 | `swe_deltat_ex` | Open | Delta T model with flags remains a dedicated domain. |
| 79 | `swe_time_equ` | Open | Equation of time remains a dedicated domain. |
| 80 | `swe_lmt_to_lat` | Open | Local mean time to local apparent time remains a dedicated domain. |
| 81 | `swe_lat_to_lmt` | Open | Local apparent time to local mean time remains a dedicated domain. |
| 82 | `swe_sidtime0` | Covered | `jme_sidereal_time0`. |
| 83 | `swe_sidtime` | Covered | `jme_sidereal_time`. |
| 84 | `swe_set_interpolate_nut` | Open | Nutation interpolation selection remains a dedicated domain. |
| 85 | `swe_cotrans` | Covered | `jme_ecliptic_to_equatorial` and `jme_equatorial_to_ecliptic`. |
| 86 | `swe_cotrans_sp` | Covered | `jme_ecliptic_to_equatorial_rectangular_state` and `jme_equatorial_to_ecliptic_rectangular_state`. |
| 87 | `swe_get_tid_acc` | Open | Tidal acceleration state query remains a dedicated domain. |
| 88 | `swe_set_tid_acc` | Open | Tidal acceleration state selection remains a dedicated domain. |
| 89 | `swe_set_delta_t_userdef` | Open | User-defined Delta T state remains a dedicated domain. |
| 90 | `swe_degnorm` | Covered | `jme_degree_normalize`. |
| 91 | `swe_radnorm` | Covered | `jme_radian_normalize`. |
| 92 | `swe_rad_midp` | Covered | `jme_radian_midpoint`. |
| 93 | `swe_deg_midp` | Covered | `jme_degree_midpoint`. |
| 94 | `swe_split_deg` | Covered | `jme_split_degree`. |
| 95 | `swe_csnorm` | Covered | `jme_centiseconds_normalize`. |
| 96 | `swe_difcsn` | Covered | `jme_centiseconds_difference`. |
| 97 | `swe_difdegn` | Covered | `jme_degrees_difference`. |
| 98 | `swe_difcs2n` | Covered | `jme_centiseconds_difference_signed`. |
| 99 | `swe_difdeg2n` | Covered | `jme_degrees_difference_signed`. |
| 100 | `swe_difrad2n` | Covered | `jme_radians_difference_signed`. |
| 101 | `swe_csroundsec` | Covered | `jme_centiseconds_round_second`. |
| 102 | `swe_d2l` | Covered | `jme_double_to_long`. |
| 103 | `swe_day_of_week` | Covered | `jme_day_of_week`. |
| 104 | `swe_cs2timestr` | Open | Centisecond time string formatting remains a dedicated domain. |
| 105 | `swe_cs2lonlatstr` | Open | Centisecond longitude/latitude string formatting remains a dedicated domain. |
| 106 | `swe_cs2degstr` | Open | Centisecond degree string formatting remains a dedicated domain. |

