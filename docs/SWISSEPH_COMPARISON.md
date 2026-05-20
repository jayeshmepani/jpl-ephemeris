# Swiss Ephemeris vs. JPL Moshier Ephemeris (JME) Coverage Status

This document tracks reference coverage against the project-owned JME API. Symbol existence is verified separately from full behavior completion.

### Current Inventory Status

| Metric | Swiss Ephemeris (Reference) | **JPL Moshier Ephemeris (Current)** | **Parity %** |
| :--- | :--- | :--- | :--- |
| **Reference Function Rows Mapped** | 106 | **106** | **100% mapping, not 100% behavior** |
| **Total Functions (API Surface)** | - | **191** | (85 extra utilities) |
| **Reference Constants** | 348 | **348** | **100%** |
| **Total Constants (API Surface)** | - | **460** | (112 extra constants) |

---

### 1. Verification Summary

1.  **Strict Constant Validation (460/460):**
    *   **Test Suite:** `tests/test_all_constants.c`
    *   **Result:** 100% Success. Verified existence and spelling for all 460 public `JME_*` constants.
2.  **Function Symbol Validation (191/191):**
    *   **Test Suite:** `tests/test_all_functions.c`
    *   **Result:** 100% Success for declared/defined symbol coverage and smoke-level call coverage. This is not a full Swiss behavior-parity claim.
3.  **Clean-Room Compliance:**
    *   **Terminology:** 0 forbidden markers remain (no "to-do", "fix-me", or "fixed").
    *   **Provenance:** Implemented logic must remain traceable to independent permitted sources before it is marked behavior-complete.
    *   **Contract:** Current tests enforce symbol coverage and reject false-success paths such as `JME_OK` with non-finite calculation output. This is not a full behavior-parity claim.

---

### 2. Full 106 Reference Function Mapping

All 106 reference behavior rows are mapped to native `jme_` equivalents. In the table below, `Verified` means the mapping/symbol is verified, not that Swiss behavior is complete. Some mapped rows are still partial or open at the algorithm level; the behavioral source of truth is `docs/REFERENCE_FUNCTION_COVERAGE.md`.

| # | Reference Function | JME Native Equivalent | Status |
|---|---|---|---|
| 1 | `swe_heliacal_ut` | `jme_heliacal_ut` | **Verified** |
| 2 | `swe_heliacal_pheno_ut` | `jme_heliacal_pheno_ut` | **Verified** |
| 3 | `swe_vis_limit_mag` | `jme_vis_limit_mag` | **Verified** |
| 4 | `swe_heliacal_angle` | `jme_heliacal_angle` | **Verified** |
| 5 | `swe_topo_arcus_visionis` | `jme_topo_arcus_visionis` | **Verified** |
| 6 | `swe_set_astro_models` | `jme_set_astro_models` | **Verified** |
| 7 | `swe_get_astro_models` | `jme_get_astro_models` | **Verified** |
| 8 | `swe_version` | `jme_version` | **Verified** |
| 9 | `swe_get_library_path` | `jme_library_path` | **Verified** |
| 10 | `swe_calc` | `jme_calc` | **Verified** |
| 11 | `swe_calc_ut` | `jme_calc_ut` | **Verified** |
| 12 | `swe_calc_pctr` | `jme_calc_pctr` | **Verified** |
| 13 | `swe_solcross` | `jme_solcross` | **Verified; refined root search** |
| 14 | `swe_solcross_ut` | `jme_solcross_ut` | **Verified; refined root search** |
| 15 | `swe_mooncross` | `jme_mooncross` | **Verified; refined root search** |
| 16 | `swe_mooncross_ut` | `jme_mooncross_ut` | **Verified; refined root search** |
| 17 | `swe_mooncross_node` | `jme_mooncross_node` | **Verified; refined root search** |
| 18 | `swe_mooncross_node_ut` | `jme_mooncross_node_ut` | **Verified; refined root search** |
| 19 | `swe_helio_cross` | `jme_helio_cross` | **Verified; refined root search** |
| 20 | `swe_helio_cross_ut` | `jme_helio_cross_ut` | **Verified; refined root search** |
| 21 | `swe_fixstar` | `jme_fixstar` | **Verified** |
| 22 | `swe_fixstar_ut` | `jme_fixstar_ut` | **Verified** |
| 23 | `swe_fixstar_mag` | `jme_fixstar_mag` | **Verified** |
| 24 | `swe_fixstar2` | `jme_fixstar2` | **Verified** |
| 25 | `swe_fixstar2_ut` | `jme_fixstar2_ut` | **Verified** |
| 26 | `swe_fixstar2_mag` | `jme_fixstar2_mag` | **Verified** |
| 27 | `swe_close` | `jme_close`, `jme_jpl_close` | **Verified** |
| 28 | `swe_set_ephe_path` | `jme_set_ephemeris_path` | **Verified** |
| 29 | `swe_set_jpl_file` | `jme_set_jpl_file` | **Verified** |
| 30 | `swe_get_planet_name` | `jme_body_name` | **Verified** |
| 31 | `swe_set_topo` | `jme_set_topo` | **Verified** |
| 32 | `swe_set_sid_mode` | `jme_set_sidereal_mode` | **Verified** |
| 33 | `swe_get_ayanamsa_ex` | `jme_get_ayanamsa_ex` | **Verified** |
| 34 | `swe_get_ayanamsa_ex_ut` | `jme_get_ayanamsa_ex_ut` | **Verified** |
| 35 | `swe_get_ayanamsa` | `jme_get_ayanamsa` | **Verified** |
| 36 | `swe_get_ayanamsa_ut` | `jme_get_ayanamsa_ut` | **Verified** |
| 37 | `swe_get_ayanamsa_name` | `jme_get_ayanamsa_name` | **Verified** |
| 38 | `swe_get_current_file_data` | `jme_jpl_current_file_data` | **Verified** |
| 39 | `swe_date_conversion` | `jme_date_is_valid`, `jme_julian_day` | **Verified** |
| 40 | `swe_julday` | `jme_julian_day` | **Verified** |
| 41 | `swe_revjul` | `jme_reverse_julian_day` | **Verified** |
| 42 | `swe_utc_to_jd` | `jme_utc_to_jd` | **Verified** |
| 43 | `swe_jdet_to_utc` | `jme_jd_to_utc` | **Verified** |
| 44 | `swe_jdut1_to_utc` | `jme_jd_to_utc` | **Verified** |
| 45 | `swe_utc_time_zone` | `jme_utc_time_zone` | **Verified** |
| 46 | `swe_houses` | `jme_houses` | **Mapped; Koch/Krusinski/APC/Sunshine and exact systems implemented** |
| 47 | `swe_houses_ex` | `jme_houses_ex` | **Mapped; Koch/Krusinski/APC/Sunshine and exact systems implemented** |
| 48 | `swe_houses_ex2` | `jme_houses_ex2` | **Mapped; finite speeds implemented** |
| 49 | `swe_houses_armc` | `jme_houses_armc` | **Mapped; Koch/Krusinski/APC and exact systems implemented** |
| 50 | `swe_houses_armc_ex2` | `jme_houses_armc_ex2` | **Mapped; finite speeds implemented** |
| 51 | `swe_house_pos` | `jme_house_pos` | **Verified** |
| 52 | `swe_house_name` | `jme_house_system_name` | **Verified** |
| 53 | `swe_gauquelin_sector` | `jme_gauquelin_sector` | **Mapped; bounded event-based sector** |
| 54 | `swe_sol_eclipse_where` | `jme_sol_eclipse_where` | **Mapped; exact pending** |
| 55 | `swe_lun_occult_where` | `jme_lun_occult_where` | **Mapped; exact pending** |
| 56 | `swe_sol_eclipse_how` | `jme_sol_eclipse_how` | **Mapped; exact pending** |
| 57 | `swe_sol_eclipse_when_loc` | `jme_sol_eclipse_when_loc` | **Mapped; exact pending** |
| 58 | `swe_lun_occult_when_loc` | `jme_lun_occult_when_loc` | **Mapped; exact pending** |
| 59 | `swe_sol_eclipse_when_glob` | `jme_sol_eclipse_when_glob` | **Mapped; exact pending** |
| 60 | `swe_lun_occult_when_glob` | `jme_lun_occult_when_glob` | **Mapped; exact pending** |
| 61 | `swe_lun_eclipse_how` | `jme_lun_eclipse_how` | **Mapped; exact pending** |
| 62 | `swe_lun_eclipse_when` | `jme_lun_eclipse_when` | **Mapped; exact pending** |
| 63 | `swe_lun_eclipse_when_loc` | `jme_lun_eclipse_when_loc` | **Mapped; exact pending** |
| 64 | `swe_pheno` | `jme_pheno` | **Verified; standard first five fields exported** |
| 65 | `swe_pheno_ut` | `jme_pheno_ut` | **Verified; standard first five fields exported** |
| 66 | `swe_refrac` | `jme_refract` | **Verified** |
| 67 | `swe_refrac_extended` | `jme_refract_extended` | **Verified** |
| 68 | `swe_set_lapse_rate` | `jme_set_lapse_rate` | **Verified** |
| 69 | `swe_azalt` | `jme_equatorial_to_horizontal` | **Verified** |
| 70 | `swe_azalt_rev` | `jme_horizontal_to_equatorial` | **Verified** |
| 71 | `swe_rise_trans_true_hor` | `jme_rise_trans_true_hor` | **Verified; true horizon honored** |
| 72 | `swe_rise_trans` | `jme_rise_trans` | **Verified; rise/set/transit/twilight flags tested** |
| 73 | `swe_nod_aps` | `jme_nod_aps` | **Mapped; lunar node/apside subset implemented** |
| 74 | `swe_nod_aps_ut` | `jme_nod_aps_ut` | **Mapped; lunar node/apside subset implemented** |
| 75 | `swe_get_orbital_elements` | `jme_get_orbital_elements` | **Verified** |
| 76 | `swe_orbit_max_min_true_distance` | `jme_orbit_max_min_true_distance` | **Verified** |
| 77 | `swe_deltat` | `jme_delta_t` | **Verified** |
| 78 | `swe_deltat_ex` | `jme_delta_t_ex` | **Verified** |
| 79 | `swe_time_equ` | `jme_time_equ` | **Verified** |
| 80 | `swe_lmt_to_lat` | `jme_lmt_to_lat` | **Verified** |
| 81 | `swe_lat_to_lmt` | `jme_lat_to_lmt` | **Verified** |
| 82 | `swe_sidtime0` | `jme_sidereal_time0` | **Verified** |
| 83 | `swe_sidtime` | `jme_sidereal_time` | **Verified** |
| 84 | `swe_set_interpolate_nut` | `jme_set_interpolate_nut` | **Verified** |
| 85 | `swe_cotrans` | `jme_ecliptic_to_equatorial` | **Verified** |
| 86 | `swe_cotrans_sp` | `jme_ecliptic_to_equatorial_rectangular_state` | **Verified** |
| 87 | `swe_get_tid_acc` | `jme_get_tid_acc` | **Verified** |
| 88 | `swe_set_tid_acc` | `jme_set_tid_acc` | **Verified** |
| 89 | `swe_set_delta_t_userdef` | `jme_set_delta_t_userdef` | **Verified** |
| 90 | `swe_degnorm` | `jme_degree_normalize` | **Verified** |
| 91 | `swe_radnorm` | `jme_radian_normalize` | **Verified** |
| 92 | `swe_rad_midp` | `jme_radian_midpoint` | **Verified** |
| 93 | `swe_deg_midp` | `jme_degree_midpoint` | **Verified** |
| 94 | `swe_split_deg` | `jme_split_degree` | **Verified** |
| 95 | `swe_csnorm` | `jme_centiseconds_normalize` | **Verified** |
| 96 | `swe_difcsn` | `jme_centiseconds_difference` | **Verified** |
| 97 | `swe_difdegn` | `jme_degrees_difference` | **Verified** |
| 98 | `swe_difcs2n` | `jme_centiseconds_difference_signed` | **Verified** |
| 99 | `swe_difdeg2n` | `jme_degrees_difference_signed` | **Verified** |
| 100 | `swe_difrad2n` | `jme_radians_difference_signed` | **Verified** |
| 101 | `swe_csroundsec` | `jme_centiseconds_round_second` | **Verified** |
| 102 | `swe_d2l` | `jme_double_to_long` | **Verified** |
| 103 | `swe_day_of_week` | `jme_day_of_week` | **Verified** |
| 104 | `swe_cs2timestr` | `jme_centiseconds_to_time_string` | **Verified** |
| 105 | `swe_cs2lonlatstr` | `jme_centiseconds_to_lonlat_string` | **Verified** |
| 106 | `swe_cs2degstr` | `jme_centiseconds_to_degree_string` | **Verified** |

---

### 3. Extended JME-Only Utilities (85 Additional Functions)

JME provides **85 additional functions** beyond the original 106 reference behavior rows, including:
*   **Matrix Algebra:** `jme_matrix_multiply`, `jme_matrix_identity`, `jme_matrix_rotate_x/y/z`.
*   **Native JPL State Lookups:** `jme_jpl_body_state_native_naif`, `jme_jpl_orientation_state_order_naif`.
*   **Coordinate Vector Transforms:** `jme_rectangular_to_spherical_state`, `jme_spherical_to_rectangular_state`.
*   **Modern Precession/Nutation:** `jme_get_nutation_matrix`, `jme_get_precession_matrix`.

**The JPL Moshier Ephemeris has full reference-row mapping and constant tracking, but full behavior parity is still in progress.**
