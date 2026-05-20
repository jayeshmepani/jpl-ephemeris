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
| Reference constants guaranteed drop-in semantic parity | Not yet all 348 |
| JME public functions currently defined | 191 |

## Resume Status

Use this section as the current handoff and resume point for Swiss-reference reality. It is intentionally descriptive so later work can continue without re-auditing the whole repo.

### How To Read This Status

- `Mapped` means a Swiss reference row has a corresponding `jme_*` API entry point.
- `Implemented` means native code exists and the row is not just a placeholder symbol.
- `Partial` means the row works in meaningful cases but still has one or more known algorithm, validation, or semantic-parity gaps.
- `Open` means behavior is still contract-only, explicitly errors, or depends on future validated algorithms.
- `Exact parity` should only be claimed when the row is implemented, materially validated, and has no known semantic mismatch against the reference behavior being tracked.

### Done / Strongly Implemented

These areas are the strongest parts of the Swiss-reference coverage today. They are the safest places to treat as real implementation rather than just symbolic mapping.

- Calendar and Julian-date behavior:
  `swe_date_conversion`, `swe_julday`, `swe_revjul`, `swe_utc_to_jd`, `swe_jdet_to_utc`, `swe_jdut1_to_utc`, `swe_utc_time_zone`, and `swe_day_of_week` are implemented and exercised as real calendar/time conversions.
- Numeric utility behavior:
  normalization, midpoint, difference, split-degree, centisecond conversion, and number-format rows are implemented as direct native utilities rather than wrappers around missing behavior.
- Coordinate conversion behavior:
  `swe_cotrans`, `swe_cotrans_sp`, `swe_azalt`, and `swe_azalt_rev` are implemented as real transforms.
- Basic atmospheric/refraction behavior:
  `swe_refrac`, `swe_refrac_extended`, and lapse-rate state support are implemented.
- Solar eclipse family:
  `swe_sol_eclipse_when_glob`, `swe_sol_eclipse_when_loc`, `swe_sol_eclipse_where`, and `swe_sol_eclipse_how` are implemented in native code.
  Global classification now uses Moon-shadow cone versus Earth-sphere geometry rather than only simple apparent-disk overlap.
  Hybrid solar classification is now exercised by a real regression case.
- Lunar eclipse core:
  `swe_lun_eclipse_when` and `swe_lun_eclipse_how` are implemented from full-moon opposition search plus conical shadow geometry.
- House-system coverage:
  a broad set of systems is implemented natively, including APC and date-aware Sunshine, rather than being left as placeholders.

### Implemented But Still Partial / Not Yet Exact-Parity

These rows should not be described as missing, but they also should not be described as fully closed out. They are implemented, callable, and often tested, yet still have known gaps in either algorithm breadth, semantic parity, or independent validation depth.

- `swe_calc`, `swe_calc_ut`, `swe_calc_pctr`
  The core calculation pipeline is real, but full Swiss-style behavioral breadth across flags, edge cases, model interactions, and fallback paths is not yet fully certified.
- Fixed-star family:
  `swe_fixstar*` rows are implemented, but the catalog/reduction behavior is still a dedicated domain and not yet proven as full Swiss-equivalent behavior.
- Ayanamsa family:
  implemented and callable, but still under dedicated-domain validation rather than exact-parity signoff.
- House rows:
  broadly implemented, but still partial where flag semantics, edge-domain behavior, Gauquelin parity, and ARMC-only Sunshine expectations are concerned.
- Gauquelin:
  `swe_gauquelin_sector` works via refined rise/set-event logic, but it is not yet certified as full reference-method parity.
- Rise/set/transit:
  implemented and root-refined, but still marked partial until broader reference-method validation is complete.
- Longitude/node/crossing rows:
  real refined searches exist, but they still need broader known-value validation before being promoted to exact-parity status.
- Solar eclipse locality/contact precision:
  solar eclipse APIs are implemented, but exact independent validation coverage for topocentric locality/contact precision is still thinner than the rest of the library.
- `swe_lun_eclipse_when_loc`
  still reuses global contact times and adds local visibility, rather than solving a completely separate local-contact geometry.
- `swe_pheno` / `swe_pheno_ut`
  the standard first phenomenon fields are present, but the broader physical ephemeris surface is still incomplete.
- `swe_nod_aps` / `swe_nod_aps_ut`
  lunar subsets and non-lunar osculating node/perihelion/aphelion support exist, but full method parity is still open.
- `swe_get_orbital_elements` and `swe_orbit_max_min_true_distance`
  implemented, but still treated as dedicated-domain behavior rather than fully closed Swiss-equivalent behavior.
- Analytical engines and fallbacks:
  VSOP87, Moshier, Meeus, and mixed fallback paths are materially integrated, but broader independent known-value validation is still incomplete.
- ELP2000:
  the Moon path is present in the project surface, but full ELP2000 lunar integration validation is still not closed out as complete.

### Open / Contract-Only / Explicit Error

These rows are the clearest remaining behavior gaps in the 106-row matrix. They are mapped by name, but they are not yet implemented as complete working behavior.

- Heliacal family:
  `swe_heliacal_ut`, `swe_heliacal_pheno_ut`, `swe_vis_limit_mag`, `swe_heliacal_angle`, and `swe_topo_arcus_visionis` remain open.
  These still require a validated visibility model rather than the current placeholder/error-contract status.
- Lunar occultation family:
  `swe_lun_occult_where`, `swe_lun_occult_when_loc`, and `swe_lun_occult_when_glob` remain open.
  These still return explicit validated-algorithm errors rather than real occultation search/circumstance behavior.

### Constant Reality

The constant story needs to be read more carefully than a simple count.

- The Swiss reference inventory of `348` constants is tracked in documentation.
- That inventory count means the symbolic surface has been audited and mapped. It does not by itself prove `348/348` exact drop-in Swiss semantic parity.
- Some constant families are structurally strong and materially usable now:
  date/calendar constants, angle-format constants, many house-system constants, many rise/set constants, and much of the model-selection surface.
- Some constant families still need stricter semantic auditing against Swiss behavior rather than name-only presence checks.
- The most important currently known semantic mismatch is the eclipse constant family.
  `JME_ECLIPSE_*` names exist, but in the current JME surface they are sequential enum-style values, not Swiss-style combinable bit flags.
  That means constant-name coverage exists, but exact Swiss-style flag semantics do not yet hold there.
- Because of that, the correct wording today is:
  `348 constants tracked`
  not
  `348 constants proven drop-in equivalent`

### Resume Guidance

If work resumes later and the goal is exact Swiss-reference reality rather than symbolic coverage, the priority order should be:

- finish lunar occultation algorithms
- finish heliacal visibility algorithms
- tighten eclipse locality/contact validation and local lunar-contact solving
- close physical-phenomena gaps
- finish Gauquelin parity work
- audit constant semantics family-by-family, especially event/flag families
- expand independent known-value validation across analytical engines and fallback paths

## Behavioral Status

The 348 reference constants are tracked as inventory. That does not mean all 348 already have drop-in Swiss-compatible semantics or exact value-model parity in every call path.

The 106 function behavior rows are all mapped to `jme_*` APIs, but they are not all fully complete yet.

Important reality check:

- `Covered` in the matrix below means a reference row is mapped to one or more `jme_*` APIs.
- `Covered` does not mean exact behavioral parity.
- `Tracked` for constants means the symbolic surface is inventoried; it does not by itself prove exact Swiss-compatible semantics.
- Some constant families are structurally compatible and safe as named controls, while some event/effect constants still differ in semantic model from Swiss expectations.

Currently complete areas are mostly metadata, date/time conversion, angle/string utilities, coordinate transforms, basic refraction, state-vector helpers, and raw JPL/CALCEPH boundary functions when CALCEPH is present.

Currently partial areas include `jme_calc`, `jme_calc_ut`, `jme_calc_pctr`, fixed stars, ayanamsa, houses, rise/set/transit, eclipse searches, physical phenomena, VSOP87, Moshier, and Meeus fallbacks. Longitude and lunar-node crossings now use refined root searches, but still need independent known-value validation before they are marked complete.

Currently open or effectively contract-only areas include heliacal visibility, lunar occultations, full Gauquelin parity, full ELP2000 lunar integration, and broader independent validation of exact topocentric eclipse/occultation locality precision.

## Constant Reality

The reference constant inventory is broader than a simple "present or missing" check.

- Many utility, house-system, rise/set, model-selection, date/calendar, and angle-format constants are present as real `JME_*` controls and are materially usable.
- Some families are only tracked at the inventory/mapping level and still need stricter semantic verification against Swiss behavior.
- Eclipse-related constants are a known semantic mismatch area: names are present, but `JME_ECLIPSE_*` values are sequential enum-style values, not Swiss-style combinable bit flags.
- Because of that, constant-name coverage alone must not be treated as full drop-in parity for all 348 reference constants.

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
| 13 | `swe_solcross` | Covered | `jme_solcross` | Solar longitude crossing uses bracketing plus root refinement; known-value validation remains. |
| 14 | `swe_solcross_ut` | Covered | `jme_solcross_ut` | UT solar longitude crossing uses the same refined search. |
| 15 | `swe_mooncross` | Covered | `jme_mooncross` | Lunar longitude crossing uses bracketing plus root refinement; known-value validation remains. |
| 16 | `swe_mooncross_ut` | Covered | `jme_mooncross_ut` | UT lunar longitude crossing uses the same refined search. |
| 17 | `swe_mooncross_node` | Covered | `jme_mooncross_node` | Lunar node crossing uses latitude sign bracketing plus root refinement; known-value validation remains. |
| 18 | `swe_mooncross_node_ut` | Covered | `jme_mooncross_node_ut` | UT lunar node crossing uses the same refined search. |
| 19 | `swe_helio_cross` | Covered | `jme_helio_cross` | Heliocentric crossing uses bracketing plus root refinement; known-value validation remains. |
| 20 | `swe_helio_cross_ut` | Covered | `jme_helio_cross_ut` | UT heliocentric crossing uses the same refined search. |
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
| 46 | `swe_houses` | Covered | `jme_houses` | Supported systems include Equal, Vehlow, Whole Sign, Porphyrius, Koch, Krusinski/Pisa/Goelzer, APC, date-aware Sunshine, Placidus, Regiomontanus, Campanus, Alcabitius, Morinus/Meridian, Polich/Page, Horizontal, and Azimuthal. |
| 47 | `swe_houses_ex` | Covered | `jme_houses_ex` | Same supported systems as `jme_houses`; ARMC-only Sunshine remains unavailable because Sun declination is required; full flag semantics remain under validation. |
| 48 | `swe_houses_ex2` | Covered | `jme_houses_ex2` | Computes finite cusp and angle speeds by central difference for supported systems. |
| 49 | `swe_houses_armc` | Covered | `jme_houses_armc` | ARMC-based computation for supported systems. |
| 50 | `swe_houses_armc_ex2` | Covered | `jme_houses_armc_ex2` | Computes finite ARMC-based cusp and angle speeds by central difference for supported systems. |
| 51 | `swe_house_pos` | Covered | `jme_house_pos` | Position-to-house calculation remains a dedicated domain. |
| 52 | `swe_house_name` | Covered | `jme_house_system_name` | House system naming. |
| 53 | `swe_gauquelin_sector` | Covered | `jme_gauquelin_sector` | Uses refined surrounding rise/set events and returns a bounded sector only when those events exist; full reference-method parity remains. |
| 54 | `swe_sol_eclipse_where` | Covered | `jme_sol_eclipse_where` | Returns native geographic eclipse-center estimates plus Moon-shadow cone / Earth-sphere geometry, including centrality classification in extended attrs. |
| 55 | `swe_lun_occult_where` | Covered | `jme_lun_occult_where` | Exact lunar occultation locality remains contract-only and returns `JME_ERR`. |
| 56 | `swe_sol_eclipse_how` | Covered | `jme_sol_eclipse_how` | Returns native local overlap, apparent diameters, altitude, and centrality-style local overlap fields when an eclipse is in progress. |
| 57 | `swe_sol_eclipse_when_loc` | Covered | `jme_sol_eclipse_when_loc` | Local solar eclipse maximum/contact search is implemented from conjunction search plus local apparent-disk overlap geometry. |
| 58 | `swe_lun_occult_when_loc` | Covered | `jme_lun_occult_when_loc` | Exact local lunar occultation search remains contract-only and returns `JME_ERR`. |
| 59 | `swe_sol_eclipse_when_glob` | Covered | `jme_sol_eclipse_when_glob` | Global solar eclipse maximum/contact search is implemented from conjunction search plus Moon-shadow cone / Earth-sphere geometry, including hybrid detection. |
| 60 | `swe_lun_occult_when_glob` | Covered | `jme_lun_occult_when_glob` | Exact global lunar occultation search remains contract-only and returns `JME_ERR`. |
| 61 | `swe_lun_eclipse_how` | Covered | `jme_lun_eclipse_how` | Geocentric lunar shadow geometry, eclipse type, magnitudes, and local Moon-above-horizon visibility are implemented. |
| 62 | `swe_lun_eclipse_when` | Covered | `jme_lun_eclipse_when` | Lunar eclipse maximum/contact search is implemented from full-moon opposition plus conical shadow geometry. |
| 63 | `swe_lun_eclipse_when_loc` | Covered | `jme_lun_eclipse_when_loc` | Local lunar eclipse search currently reuses global contact times and adds local visibility from Moon altitude. |
| 64 | `swe_pheno` | Covered | `jme_pheno` | Returns phase angle, illuminated fraction, elongation, apparent diameter, and magnitude; extended rotational physical ephemeris attributes remain open. |
| 65 | `swe_pheno_ut` | Covered | `jme_pheno_ut` | UT wrapper for the standard first five phenomenon fields. |
| 66 | `swe_refrac` | Covered | `jme_refract` | Atmospheric refraction. |
| 67 | `swe_refrac_extended` | Covered | `jme_refract_extended` | Extended atmospheric refraction. |
| 68 | `swe_set_lapse_rate` | Covered | `jme_set_lapse_rate` | Refraction lapse-rate state remains a dedicated domain. |
| 69 | `swe_azalt` | Covered | `jme_equatorial_to_horizontal` | Azimuth/altitude conversion. |
| 70 | `swe_azalt_rev` | Covered | `jme_horizontal_to_equatorial` | Reverse azimuth/altitude conversion. |
| 71 | `swe_rise_trans_true_hor` | Covered | `jme_rise_trans_true_hor` | True horizon is honored and root-refined; full reference-method validation remains. |
| 72 | `swe_rise_trans` | Covered | `jme_rise_trans` | Rise, set, meridian transit, anti-meridian transit, no-refraction, disc-center, and twilight flags are implemented/tested; full reference-method validation remains. |
| 73 | `swe_nod_aps` | Covered | `jme_nod_aps` | Lunar node/apside subset plus non-lunar osculating node, perihelion, and aphelion longitudes/distances are implemented; full method parity remains open. |
| 74 | `swe_nod_aps_ut` | Covered | `jme_nod_aps_ut` | UT wrapper for the implemented lunar and non-lunar node/apside subset. |
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
