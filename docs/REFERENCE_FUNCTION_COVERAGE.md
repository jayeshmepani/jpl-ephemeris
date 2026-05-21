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
| JME public functions currently defined | 204 |

## Resume Status

Use this section as the current handoff and resume point for Swiss-reference reality. It is intentionally descriptive so later work can continue without re-auditing the whole repo.

For the live unchecked worklist, use `docs/FUNCTION_CLOSURE_CHECKLIST.md`. That checklist is the batch-by-batch progress tracker for closing the remaining rows and extra public functions.

### How To Read This Status

- `Mapped` means a Swiss reference row has a corresponding `jme_*` API entry point.
- `Implemented` means native code exists and the row is not just a declared symbol.
- `Partial` means the row works in meaningful cases but still has one or more known algorithm, validation, or semantic-parity gaps.
- `Open` means behavior is first-pass, contract-limited, or depends on future validated algorithms before exact parity can be claimed.
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
  They are now closed for JME-native global behavior with total, partial, and penumbral known-value regressions, backward search, non-eclipse rejection, finite shadow fields, and null-output rejection.
- House-system coverage:
  a broad set of systems is implemented natively, including APC and date-aware Sunshine, rather than being left as declaration-only rows.

### Implemented But Still Partial / Not Yet Exact-Parity

These rows should not be described as missing, but they also should not be described as fully closed out. They are implemented, callable, and often tested, yet still have known gaps in either algorithm breadth, semantic parity, or independent validation depth.

- `swe_calc`, `swe_calc_ut`, `swe_calc_pctr`
  The core calculation pipeline is real, but full Swiss-style behavioral breadth across flags, edge cases, model interactions, and fallback paths is not yet fully certified.
- Fixed-star family:
  `swe_fixstar*` rows are implemented, reject invalid output pointers, and include direct contract coverage for Sirius and Spica. The catalog/reduction behavior is still a dedicated domain and not yet proven as full Swiss-equivalent behavior.
- Ayanamsa family:
  numeric ayanamsa rows are implemented and callable, but still under dedicated-domain validation rather than exact-parity signoff. The naming row is closed for all declared JME sidereal constants.
- House rows:
  closed for the JME-native supported-system contract with finite cusp/angle output, speed output, ARMC behavior, Sunshine declination handling, position-to-house behavior, and invalid-output checks.
- Gauquelin:
  closed for the JME-native method contract with methods 0-3, finite sector range, fractional/refraction-sensitive event methods, unsupported-method rejection, and invalid-output checks.
- Rise/set/transit:
  implemented and root-refined, but still marked partial until broader reference-method validation is complete.
- Longitude/node/crossing rows:
  refined searches are now closed for JME-native behavior: solar longitude, lunar longitude, lunar node, and heliocentric longitude crossings have target-normalization, root-refinement, UT-alias, and invalid-output coverage.
- Solar eclipse locality/contact precision:
  solar eclipse APIs are implemented, but exact independent validation coverage for topocentric locality/contact precision is still thinner than the rest of the library.
  `jme_sol_eclipse_where()` now fills partial-only eclipse magnitude from the returned shadow-surface location instead of geocentric apparent separation; 2022-10-25 is covered as a partial solar known-value regression.
- `swe_lun_eclipse_when_loc`
  now intersects the closed global lunar-eclipse phase window with local Moon visibility and clips contacts to the visible interval. It still does not claim a fully independent topocentric lunar-shadow contact model.
- `swe_pheno` / `swe_pheno_ut`
  closed for the JME-native physical-phenomena field contract: phase angle, illuminated fraction, elongation, apparent diameter, magnitude, distance, light-time, apparent-radius, phase-defect, and bright-limb fields are finite and tested for ET and UT entry points.
- `swe_nod_aps` / `swe_nod_aps_ut`
  closed for JME-native behavior with lunar node/apside and non-lunar osculating node/perihelion/aphelion coverage, including invalid-input checks.
- `swe_get_orbital_elements` and `swe_orbit_max_min_true_distance`
  closed for JME-native behavior with osculating element invariants, derived distance fields, Mercury distance-range checks, and invalid-input checks.
- Analytical engines and fallbacks:
  VSOP87, Moshier, Meeus, and mixed fallback paths are materially integrated, direct contract tests now enforce supported-body success / unsupported-body rejection for the Moshier and Meeus public wrappers, and the public `jme_calc` path has a regression that exercises an analytic fallback Pluto calculation, but broader independent known-value validation is still incomplete.
  The analytical validation suite also checks that representative Moshier, VSOP87, and Meeus bodies produce direct public state output at J2000.
  The public fallback regression also checks rectangular and distance-unit flag behavior for Pluto as a non-JPL path.
  The public calculation contract also checks velocity-per-second conversion for a representative analytic body.
  The same contract block checks `jme_calc_pctr()` in a build-aware way: CALCEPH builds verify the Mercury/Sun difference contract, while non-CALCEPH builds verify a clean explicit error instead of silent success.
  The same `jme_calc_pctr()` check also validates distance-unit scaling against the direct heliocentric Mercury vector norm.
- ELP2000:
  the Moon path is present in the project surface and now has multiple external modern known-value checks, but full date-range precision certification is still not closed out as complete.

### Open / First-Pass / Exact-Not-Claimed

These rows are the clearest remaining behavior gaps in the 106-row matrix. They are mapped by name, but they are not yet implemented as complete working behavior for exact Swiss-parity claims.

- Heliacal family:
  `swe_heliacal_ut`, `swe_heliacal_pheno_ut`, `swe_vis_limit_mag`, `swe_heliacal_angle`, and `swe_topo_arcus_visionis` now return first-pass native outputs.
  The native APIs now reject null required outputs, unsupported body IDs, non-physical node bodies, and fractional body IDs instead of silently falling back to Venus.
  They still require source-backed visual-observer model parity, atmosphere/observer parameter handling, and external known-value validation before they should be called complete.
- Lunar occultation family:
  `swe_lun_occult_where`, `swe_lun_occult_when_loc`, and `swe_lun_occult_when_glob` now have first-pass native Moon-target geometry.
  Global behavior uses lunar horizontal parallax as a possibility threshold; local behavior uses topocentric disk overlap.
  The public entry points now have direct null required-output/geopos rejection tests.
  Broader external validation and exact geographic-contact modeling remain open.

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

- harden and externally validate lunar occultation algorithms
- harden heliacal visibility algorithms with a source-backed visual model and known-value validation
- tighten eclipse locality/contact validation and local lunar-contact solving
- close physical-phenomena gaps
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

Currently complete areas are mostly metadata, date/time conversion, angle/string utilities, coordinate transforms, basic refraction, state-vector helpers, and the current raw JPL/CALCEPH boundary functions when CALCEPH plus the required capability-specific kernels/files are present.

Currently partial areas include fixed stars, full numeric ayanamsa model breadth, heliacal visibility, eclipse/occultation searches, and remaining broad external validation for VSOP87, Moshier, and Meeus fallbacks.

Currently open or effectively first-pass areas include full wide-date ELP2000 lunar precision certification and broader independent validation of exact topocentric eclipse/occultation locality precision.

## Constant Reality

The reference constant inventory is broader than a simple "present or missing" check.

- Many utility, house-system, rise/set, model-selection, date/calendar, and angle-format constants are present as real `JME_*` controls and are materially usable.
- Some families are only tracked at the inventory/mapping level and still need stricter semantic verification against Swiss behavior.
- Eclipse-related constants are a known semantic mismatch area: names are present, but `JME_ECLIPSE_*` values are sequential enum-style values, not Swiss-style combinable bit flags.
- Because of that, constant-name coverage alone must not be treated as full drop-in parity for all 348 reference constants.

## Coverage Matrix

| # | Reference behavior row | Mapping status | Swiss -> JME map | JME coverage |
|---:|---|---|---|---|
| 1 | `swe_heliacal_ut` | Covered | `jme_heliacal_ut` | First-pass local twilight event search with explicit body/null validation; source-backed visual-model parity and validation remain. |
| 2 | `swe_heliacal_pheno_ut` | Covered | `jme_heliacal_pheno_ut` | First-pass Sun/body altitude, arcus, limiting magnitude, apparent magnitude, elongation, visibility flag, and explicit body/null validation. |
| 3 | `swe_vis_limit_mag` | Covered | `jme_vis_limit_mag` | First-pass limiting-magnitude field with explicit output/body validation; atmosphere/observer model parity remains. |
| 4 | `swe_heliacal_angle` | Covered | `jme_heliacal_angle` | Returns Sun-body elongation from the first-pass phenomenon calculation; invalid explicit body data returns `NaN`; output semantics validation remains. |
| 5 | `swe_topo_arcus_visionis` | Covered | `jme_topo_arcus_visionis` | Returns body altitude minus Sun altitude from the first-pass topocentric calculation; invalid explicit body data returns `NaN`; output semantics validation remains. |
| 6 | `swe_set_astro_models` | Covered | `jme_set_astro_models` | Closed for JME-native behavior: parses profile/tokens into canonical bias, nutation, obliquity, precession, sidereal-time, and Delta-T model state; model-state effects are covered by direct output checks. |
| 7 | `swe_get_astro_models` | Covered | `jme_get_astro_models` | Closed for JME-native behavior: returns canonical model-state summary and rejects null output. |
| 8 | `swe_version` | Covered | `jme_version` | Version string export. |
| 9 | `swe_get_library_path` | Covered | `jme_library_path` | Library path export. |
| 10 | `swe_calc` | Covered | `jme_calc` | Closed for JME-native behavior: null-output rejection, unsupported-body rejection, finite Sun/Moon/node/planet output, analytic fallback path, rectangular output, distance scaling, velocity-per-second scaling, radians, and sidereal longitude behavior are covered. |
| 11 | `swe_calc_ut` | Covered | `jme_calc_ut` | Closed for JME-native behavior: UT-to-ET wrapper behavior is covered through the same public calculation contract, including finite body outputs and flag behavior. |
| 12 | `swe_calc_pctr` | Covered | `jme_calc_pctr` | Closed for JME-native behavior: body-center rectangular difference, spherical conversion, distance scaling, radians, sidereal longitude, null-output rejection, and unsupported body/center rejection are covered. |
| 13 | `swe_solcross` | Covered | `jme_solcross` | Closed for JME-native behavior: solar longitude crossing normalizes target longitude, rejects null output, and returns a refined crossing verified against `jme_calc_ut`. |
| 14 | `swe_solcross_ut` | Covered | `jme_solcross_ut` | Closed for JME-native behavior: UT alias behavior is verified against the normalized solar crossing result. |
| 15 | `swe_mooncross` | Covered | `jme_mooncross` | Closed for JME-native behavior: lunar longitude crossing normalizes target longitude, rejects null output, and returns a refined crossing verified against `jme_calc_ut`. |
| 16 | `swe_mooncross_ut` | Covered | `jme_mooncross_ut` | Closed for JME-native behavior: UT alias behavior is verified against the normalized lunar crossing result. |
| 17 | `swe_mooncross_node` | Covered | `jme_mooncross_node` | Closed for JME-native behavior: lunar latitude zero-crossing refinement and null-output rejection are verified. |
| 18 | `swe_mooncross_node_ut` | Covered | `jme_mooncross_node_ut` | Closed for JME-native behavior: UT alias behavior is verified against the lunar node crossing result. |
| 19 | `swe_helio_cross` | Covered | `jme_helio_cross` | Closed for JME-native behavior: heliocentric longitude crossing normalizes target longitude, rejects null output, and returns a refined crossing verified against `jme_calc_ut`. |
| 20 | `swe_helio_cross_ut` | Covered | `jme_helio_cross_ut` | Closed for JME-native behavior: UT alias behavior is verified against the normalized heliocentric crossing result. |
| 21 | `swe_fixstar` | Covered | `jme_fixstar` | Hardened but not closed: built-in catalog entries, radians/sidereal flags, null/empty star rejection, and finite output are covered; broad catalog and reference-grade reduction validation remain open. |
| 22 | `swe_fixstar_ut` | Covered | `jme_fixstar_ut` | Hardened but not closed: UT wrapper behavior and null star rejection are covered; broad catalog and reference-grade reduction validation remain open. |
| 23 | `swe_fixstar_mag` | Covered | `jme_fixstar_mag` | Hardened but not closed: all built-in magnitudes and null star/output rejection are covered; broad catalog remains open. |
| 24 | `swe_fixstar2` | Covered | `jme_fixstar2` | Hardened but not closed: alternate entry point delegates to the primary fixed-star contract; broad catalog and reference-grade reduction validation remain open. |
| 25 | `swe_fixstar2_ut` | Covered | `jme_fixstar2_ut` | Hardened but not closed: alternate UT entry point delegates to the primary UT fixed-star contract; broad catalog and reference-grade reduction validation remain open. |
| 26 | `swe_fixstar2_mag` | Covered | `jme_fixstar2_mag` | Hardened but not closed: alternate magnitude entry point delegates to the primary magnitude contract; broad catalog remains open. |
| 27 | `swe_close` | Covered | `jme_close`, `jme_jpl_close` | Runtime and JPL resource close. |
| 28 | `swe_set_ephe_path` | Covered | `jme_set_ephemeris_path`, `jme_ephemeris_path` | Ephemeris path state. |
| 29 | `swe_set_jpl_file` | Covered | `jme_set_jpl_file`, `jme_jpl_file` | JPL file state. |
| 30 | `swe_get_planet_name` | Covered | `jme_body_name`, `jme_copy_body_name` | Body name lookup and copy. |
| 31 | `swe_set_topo` | Covered | `jme_set_topo` | Topocentric observer state remains a dedicated domain. |
| 32 | `swe_set_sid_mode` | Covered | `jme_set_sidereal_mode`, `jme_get_sidereal_mode` | Sidereal mode state. |
| 33 | `swe_get_ayanamsa_ex` | Covered | `jme_get_ayanamsa_ex` | Hardened but not closed: Lahiri, Fagan-Bradley, user-defined mode, null-output rejection, and unsupported-model rejection are covered; full declared-model formula coverage remains open. |
| 34 | `swe_get_ayanamsa_ex_ut` | Covered | `jme_get_ayanamsa_ex_ut` | Hardened but not closed: UT wrapper behavior is covered for supported numeric models; full declared-model formula coverage remains open. |
| 35 | `swe_get_ayanamsa` | Covered | `jme_get_ayanamsa` | Hardened but not closed: current sidereal mode is no longer silently approximated for unsupported numeric models; full declared-model formula coverage remains open. |
| 36 | `swe_get_ayanamsa_ut` | Covered | `jme_get_ayanamsa_ut` | Hardened but not closed: UT wrapper behavior is covered for current supported sidereal mode; full declared-model formula coverage remains open. |
| 37 | `swe_get_ayanamsa_name` | Covered | `jme_get_ayanamsa_name` | Closed for JME-native behavior: every declared JME sidereal constant has a deterministic public name, and unknown model IDs return a deterministic unknown-name result. |
| 38 | `swe_get_current_file_data` | Covered | `jme_jpl_current_file_data` | Closed for JME-native behavior: CALCEPH-backed `de440s.bsp` success path returns non-empty path and valid coverage span; closed-kernel/CALCEPH-unavailable error paths reset outputs; null metadata output rejection is covered. |
| 39 | `swe_date_conversion` | Covered | `jme_date_is_valid`, `jme_julian_day`, `jme_reverse_julian_day` | Calendar validation and Julian conversion. |
| 40 | `swe_julday` | Covered | `jme_julian_day` | Julian day conversion. |
| 41 | `swe_revjul` | Covered | `jme_reverse_julian_day` | Reverse Julian day conversion. |
| 42 | `swe_utc_to_jd` | Covered | `jme_utc_to_jd` | UTC to Julian day conversion. |
| 43 | `swe_jdet_to_utc` | Covered | `jme_jd_to_utc` | ET Julian day to UTC fields. |
| 44 | `swe_jdut1_to_utc` | Covered | `jme_jd_to_utc` | UT1 Julian day to UTC fields. |
| 45 | `swe_utc_time_zone` | Covered | `jme_utc_time_zone` | Time-zone conversion. |
| 46 | `swe_houses` | Covered | `jme_houses` | Closed for JME-native behavior: supported systems, finite cusp/angle output, angle relationships, invalid system rejection, and null-cusp rejection are covered. |
| 47 | `swe_houses_ex` | Covered | `jme_houses_ex` | Closed for JME-native behavior: same supported-system contract as `jme_houses`, including null-cusp rejection. |
| 48 | `swe_houses_ex2` | Covered | `jme_houses_ex2` | Closed for JME-native behavior: finite cusp/angle speed output and null-cusp rejection are covered. |
| 49 | `swe_houses_armc` | Covered | `jme_houses_armc` | Closed for JME-native behavior: ARMC computation, Sunshine declination contract, finite output, and null-cusp rejection are covered. |
| 50 | `swe_houses_armc_ex2` | Covered | `jme_houses_armc_ex2` | Closed for JME-native behavior: finite ARMC speed output and null-cusp rejection are covered. |
| 51 | `swe_house_pos` | Covered | `jme_house_pos` | Closed for JME-native behavior: finite position-to-house behavior and null-input rejection are covered. |
| 52 | `swe_house_name` | Covered | `jme_house_system_name` | House system naming. |
| 53 | `swe_gauquelin_sector` | Covered | `jme_gauquelin_sector` | Closed for JME-native behavior: methods 0-3, finite sector range, fractional/refraction-sensitive event methods, unsupported-method rejection, and null-output/geopos rejection are covered. |
| 54 | `swe_sol_eclipse_where` | Covered | `jme_sol_eclipse_where` | Returns native geographic eclipse-center estimates plus Moon-shadow cone / Earth-sphere geometry, including centrality classification in extended attrs; partial-only magnitude now comes from the returned shadow-surface location. |
| 55 | `swe_lun_occult_where` | Covered | `jme_lun_occult_where` | First-pass native Moon-target geometry with parallax-aware global possibility attributes; broader validation remains. |
| 56 | `swe_sol_eclipse_how` | Covered | `jme_sol_eclipse_how` | Returns native local overlap, apparent diameters, altitude, and centrality-style local overlap fields when an eclipse is in progress. |
| 57 | `swe_sol_eclipse_when_loc` | Covered | `jme_sol_eclipse_when_loc` | Local solar eclipse maximum/contact search is implemented from conjunction search plus local apparent-disk overlap geometry. |
| 58 | `swe_lun_occult_when_loc` | Covered | `jme_lun_occult_when_loc` | First-pass local topocentric Moon-target disk-overlap search and contact estimates; broader validation remains. |
| 59 | `swe_sol_eclipse_when_glob` | Covered | `jme_sol_eclipse_when_glob` | Global solar eclipse maximum/contact search is implemented from conjunction search plus Moon-shadow cone / Earth-sphere geometry, including total, annular, hybrid, and partial known-value regressions. |
| 60 | `swe_lun_occult_when_glob` | Covered | `jme_lun_occult_when_glob` | First-pass global Moon-target search using lunar parallax possibility threshold; broader validation remains. |
| 61 | `swe_lun_eclipse_how` | Covered | `jme_lun_eclipse_how` | Closed for JME-native global behavior: total, partial, and penumbral circumstances are covered with external known-value maxima/magnitudes, non-eclipse rejection, finite shadow fields, local visibility flag behavior, and null-output rejection. |
| 62 | `swe_lun_eclipse_when` | Covered | `jme_lun_eclipse_when` | Closed for JME-native global behavior: total, partial, and penumbral maximum/contact search is covered with external known-value maxima, backward search, contact fields, and null-output rejection. |
| 63 | `swe_lun_eclipse_when_loc` | Covered | `jme_lun_eclipse_when_loc` | Intersects global phase contacts with local Moon-above-horizon visibility, clips local contacts, and returns visible interval fields; topocentric shadow-contact certification remains. |
| 64 | `swe_pheno` | Covered | `jme_pheno` | Closed for JME-native behavior: phase angle, illuminated fraction, elongation, apparent diameter, magnitude, distance, light-time, apparent-radius, phase-defect, and bright-limb fields are covered with finite-output and null-output tests. |
| 65 | `swe_pheno_ut` | Covered | `jme_pheno_ut` | Closed for JME-native behavior: UT wrapper behavior is covered for the same finite physical-phenomena field contract and null-output rejection. |
| 66 | `swe_refrac` | Covered | `jme_refract` | Atmospheric refraction. |
| 67 | `swe_refrac_extended` | Covered | `jme_refract_extended` | Extended atmospheric refraction. |
| 68 | `swe_set_lapse_rate` | Covered | `jme_set_lapse_rate` | Refraction lapse-rate state remains a dedicated domain. |
| 69 | `swe_azalt` | Covered | `jme_equatorial_to_horizontal` | Azimuth/altitude conversion. |
| 70 | `swe_azalt_rev` | Covered | `jme_horizontal_to_equatorial` | Reverse azimuth/altitude conversion. |
| 71 | `swe_rise_trans_true_hor` | Covered | `jme_rise_trans_true_hor` | Closed for JME-native behavior: true-horizon rise is root-refined and verified by independently recomputed apparent altitude residuals; null-output/geopos rejection is covered. |
| 72 | `swe_rise_trans` | Covered | `jme_rise_trans` | Closed for JME-native behavior: Sun, Moon, planet, and fixed-star rise/set/transit paths are covered with independent altitude/hour-angle residual checks; twilight, disc-center, no-refraction, unsupported body/star/mode, no-event, and null-output/geopos paths are covered. |
| 73 | `swe_nod_aps` | Covered | `jme_nod_aps` | Closed for JME-native behavior: lunar node/apside and non-lunar osculating node/perihelion/aphelion behavior are covered, including unsupported-body and null-output rejection. |
| 74 | `swe_nod_aps_ut` | Covered | `jme_nod_aps_ut` | Closed for JME-native behavior: UT wrapper behavior is covered with finite normalized output. |
| 75 | `swe_get_orbital_elements` | Covered | `jme_get_orbital_elements` | Closed for JME-native behavior: osculating element invariants, derived longitude/radius fields, unsupported-body rejection, and null-output rejection are covered. |
| 76 | `swe_orbit_max_min_true_distance` | Covered | `jme_orbit_max_min_true_distance` | Closed for JME-native behavior: perihelion/aphelion time-distance output, expected Mercury distance ranges, unsupported-body rejection, and null-output rejection are covered. |
| 77 | `swe_deltat` | Covered | `jme_delta_t` | Delta T model remains a dedicated domain. |
| 78 | `swe_deltat_ex` | Covered | `jme_delta_t_ex` | Delta T model with flags remains a dedicated domain. |
| 79 | `swe_time_equ` | Covered | `jme_time_equ` | Equation of time remains a dedicated domain. |
| 80 | `swe_lmt_to_lat` | Covered | `jme_lmt_to_lat` | Local mean time to local apparent time remains a dedicated domain. |
| 81 | `swe_lat_to_lmt` | Covered | `jme_lat_to_lmt` | Local apparent time to local mean time remains a dedicated domain. |
| 82 | `swe_sidtime0` | Covered | `jme_sidereal_time0` | Closed for JME-native behavior with J2000 known-value validation. |
| 83 | `swe_sidtime` | Covered | `jme_sidereal_time` | Closed for JME-native behavior with default and IAU 2006 known-value validation. |
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
