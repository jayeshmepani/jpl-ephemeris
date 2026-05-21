# Function Closure Checklist

This is the working checklist for closing the public behavior surface. It must be updated during every closure batch.

This file tracks behavior certification, not symbol existence. Symbol coverage is already verified separately:

```text
jme_functions_total=204
jme_functions_defined=204
jme_constants_total=462
```

## Closure Rule

A function or reference row can be checked only when all of these are true:

- the implementation is real code, not a name-only mapping
- normal valid inputs return finite, documented outputs
- invalid or unsupported inputs return `JME_ERR` or a documented safe value
- behavior is covered by unit or contract tests
- output units, frame, and domain are documented or obvious from the API contract
- no known algorithm branch is absent for the claimed behavior

Exact Swiss black-box parity is stricter than JME-native closure. Exact Swiss parity remains unclaimed until a row-by-row reference parity suite exists.

## Current Strict Counts

Current status after the recent closure batches:

| Area | Closed enough | Not closed | Total |
|---|---:|---:|---:|
| Swiss reference rows by JME-native code/tests | 83 | 23 | 106 |
| Distinct extra JME public functions outside the 106-row mapping | 91 | 1 | 92 |

Public function arithmetic:

- Total public `jme_*` functions: `204`
- Unique public `jme_*` functions used by the 106-row Swiss mapping table: `112`
- Distinct extra public `jme_*` functions not used by that table: `92`
- Therefore: `112 + 92 = 204`

Do not calculate this as `106 + 92`. The 106 Swiss rows are behavior rows, not unique JME function names. Several Swiss rows map to more than one JME function, so the 106-row table currently uses 112 distinct public `jme_*` functions.

Recent rows moved from not-closed to closed:

- [x] Row 68, `swe_set_lapse_rate` / `jme_set_lapse_rate`: lapse-rate state is now consumed by extended refraction and rise/transit altitude refraction.
- [x] Row 79, `swe_time_equ` / `jme_time_equ`: now uses the public calculation pipeline with analytic fallback and rejects null output.
- [x] Row 80, `swe_lmt_to_lat` / `jme_lmt_to_lat`: now rejects null output and is covered by round-trip tests.
- [x] Row 81, `swe_lat_to_lmt` / `jme_lat_to_lmt`: now uses fixed-point refinement, rejects null output, and is covered by round-trip tests.
- [x] Row 87, `swe_get_tid_acc` / `jme_get_tid_acc`: finite state round-trip is covered by contract tests.
- [x] Row 88, `swe_set_tid_acc` / `jme_set_tid_acc`: non-finite input is ignored so the state cannot be corrupted by NaN/Inf.
- [x] Row 89, `swe_set_delta_t_userdef` / `jme_set_delta_t_userdef`: finite input overrides all Delta-T models, and non-finite input clears the override back to model-driven Delta-T.
- [x] Row 28, `swe_set_ephe_path` / `jme_set_ephemeris_path`, `jme_ephemeris_path`: set/get/reset behavior is covered, including null reset.
- [x] Row 29, `swe_set_jpl_file` / `jme_set_jpl_file`, `jme_jpl_file`: set/get/reset behavior is covered, including null reset and JPL close side effect.
- [x] Row 31, `swe_set_topo` / `jme_set_topo`: finite/range sanitization is implemented and verified through `jme_get_topo_pos`.
- [x] Row 32, `swe_set_sid_mode` / `jme_set_sidereal_mode`, `jme_get_sidereal_mode`: set/get behavior and non-finite numeric sanitization are covered.
- [x] Row 84, `swe_set_interpolate_nut` / `jme_set_interpolate_nut`: the setter now controls observable nutation interpolation behavior, with endpoint and midpoint contract tests.
- [x] Row 77, `swe_deltat` / `jme_delta_t`: default model behavior is covered by a known J2000 value and by user-override reset tests.
- [x] Row 78, `swe_deltat_ex` / `jme_delta_t_ex`: every declared Delta-T model ID has explicit finite behavior, unsupported IDs return `NaN` plus an error, and user override applies consistently across models.
- [x] Row 6, `swe_set_astro_models` / `jme_set_astro_models`: token/profile parsing drives canonical bias, nutation, obliquity, precession, sidereal-time, and Delta-T model state, and the state is exercised through model-specific output checks.
- [x] Row 7, `swe_get_astro_models` / `jme_get_astro_models`: canonical model-state output and null-output rejection are covered by contract tests.
- [x] Row 82, `swe_sidtime0` / `jme_sidereal_time0`: J2000 sidereal-time behavior is covered by a known-value contract test.
- [x] Row 83, `swe_sidtime` / `jme_sidereal_time`: default and IAU 2006 sidereal-time behavior are both covered by known-value contract tests.
- [x] Fixed-star API hardening: `jme_fixstar*` now rejects null output pointers, Sirius is present in the built-in catalog, and both primary and alternate fixed-star entry points have direct contract coverage. The Swiss fixed-star rows stay unchecked until catalog breadth and reference-equivalent reduction behavior are complete.
- [x] Fixed-star hardening extension: null/empty star names now reject instead of reaching `strcmp`, the nutation conversion uses the full-precision degree-to-radian constant, spherical fixed-star output honors radians and sidereal longitude flags, and every built-in catalog entry has finite position/magnitude contract coverage. Rows 21-26 stay unchecked because the built-in catalog is still not broad enough for fixed-star parity.
- [x] Row 37, `swe_get_ayanamsa_name` / `jme_get_ayanamsa_name`: every declared JME sidereal constant now has a deterministic public name and unknown model IDs return a deterministic unknown-name result.
- [x] Row 38, `swe_get_current_file_data` / `jme_jpl_current_file_data`: CALCEPH-backed `de440s.bsp` success path returns non-empty path and valid coverage span; closed-kernel/CALCEPH-unavailable error paths reset outputs; null metadata output rejection is covered.
- [x] Ayanamsa hardening only: `jme_get_ayanamsa_ex` now rejects null output and unsupported sidereal model IDs instead of silently returning Lahiri for every model; Lahiri, Fagan-Bradley, user-defined mode, and UT wrapper behavior are covered. Rows 33-36 stay unchecked until every declared numeric sidereal model has source-backed formula/epoch data and known-value validation.
- [x] Row 10, `swe_calc` / `jme_calc`: null-output rejection, unsupported-body rejection, finite Sun/Moon/node/planet output, analytic fallback path, rectangular output, distance scaling, velocity-per-second scaling, radians, and sidereal longitude behavior are covered for JME-native operation.
- [x] Row 11, `swe_calc_ut` / `jme_calc_ut`: UT-to-ET wrapper behavior is covered through the same public calculation contract, including finite body outputs and flag behavior.
- [x] Row 12, `swe_calc_pctr` / `jme_calc_pctr`: body-center rectangular difference, spherical conversion, distance scaling, radians, sidereal longitude, null-output rejection, and unsupported body/center rejection are covered for JME-native operation.
- [x] Row 13, `swe_solcross` / `jme_solcross`: target normalization, root-refinement accuracy, and null-output rejection are covered.
- [x] Row 14, `swe_solcross_ut` / `jme_solcross_ut`: UT alias behavior is covered against the normalized solar crossing result.
- [x] Row 15, `swe_mooncross` / `jme_mooncross`: target normalization, root-refinement accuracy, and null-output rejection are covered.
- [x] Row 16, `swe_mooncross_ut` / `jme_mooncross_ut`: UT alias behavior is covered against the normalized lunar crossing result.
- [x] Row 17, `swe_mooncross_node` / `jme_mooncross_node`: lunar latitude zero-crossing refinement and null-output rejection are covered.
- [x] Row 18, `swe_mooncross_node_ut` / `jme_mooncross_node_ut`: UT alias behavior is covered against the lunar node crossing result.
- [x] Row 19, `swe_helio_cross` / `jme_helio_cross`: target normalization, heliocentric longitude refinement, and null-output rejection are covered.
- [x] Row 20, `swe_helio_cross_ut` / `jme_helio_cross_ut`: UT alias behavior is covered against the normalized heliocentric crossing result.
- [x] Row 73, `swe_nod_aps` / `jme_nod_aps`: lunar node/apside and non-lunar osculating node/perihelion/aphelion behavior are covered, including unsupported-body and null-output rejection.
- [x] Row 74, `swe_nod_aps_ut` / `jme_nod_aps_ut`: UT wrapper behavior is covered with finite normalized output.
- [x] Row 75, `swe_get_orbital_elements` / `jme_get_orbital_elements`: osculating element invariants, derived longitude/radius fields, unsupported-body rejection, and null-output rejection are covered.
- [x] Row 76, `swe_orbit_max_min_true_distance` / `jme_orbit_max_min_true_distance`: perihelion/aphelion time-distance output, expected Mercury distance ranges, unsupported-body rejection, and null-output rejection are covered.
- [x] Row 64, `swe_pheno` / `jme_pheno`: phase angle, illuminated fraction, elongation, apparent diameter, magnitude, distance, light-time, apparent-radius, phase-defect, and bright-limb fields are covered with finite-output and null-output tests.
- [x] Row 65, `swe_pheno_ut` / `jme_pheno_ut`: UT wrapper behavior is covered for the same finite physical-phenomena field contract and null-output rejection.
- [x] Row 46, `swe_houses` / `jme_houses`: supported house systems, finite cusp/angle output, angle relationships, invalid system rejection, and null-cusp rejection are covered.
- [x] Row 47, `swe_houses_ex` / `jme_houses_ex`: same supported-system contract as `jme_houses`, including null-cusp rejection.
- [x] Row 48, `swe_houses_ex2` / `jme_houses_ex2`: finite cusp/angle speed output and null-cusp rejection are covered.
- [x] Row 49, `swe_houses_armc` / `jme_houses_armc`: ARMC computation, Sunshine declination contract, finite output, and null-cusp rejection are covered.
- [x] Row 50, `swe_houses_armc_ex2` / `jme_houses_armc_ex2`: finite ARMC speed output and null-cusp rejection are covered.
- [x] Row 51, `swe_house_pos` / `jme_house_pos`: finite position-to-house behavior and null-input rejection are covered.
- [x] Row 53, `swe_gauquelin_sector` / `jme_gauquelin_sector`: methods 0-3, finite sector range, fractional/refraction-sensitive event methods, unsupported-method rejection, and null-output/geopos rejection are covered.
- [x] Row 71, `swe_rise_trans_true_hor` / `jme_rise_trans_true_hor`: true-horizon rise is root-refined and verified by independently recomputed apparent altitude residuals; null-output/geopos rejection is covered.
- [x] Row 72, `swe_rise_trans` / `jme_rise_trans`: Sun, Moon, planet, and fixed-star rise/set/transit paths are covered with independent altitude/hour-angle residual checks; twilight, disc-center, no-refraction, unsupported body/star/mode, no-event, and null-output/geopos paths are covered.
- [x] Heliacal hardening only: `jme_heliacal_*` and `jme_vis_limit_mag` now reject null required outputs, unsupported body IDs, non-physical node bodies, and fractional body IDs instead of silently defaulting to Venus. Rows 1-5 stay unchecked until the visual-observer model is source-backed and externally validated.
- [x] Eclipse/occultation hardening only: solar eclipse, lunar eclipse, and lunar occultation public entry points now have direct null required-output/geopos contract tests. Remaining rows 54-60 and 63 stay unchecked until independent locality/contact and occultation validation is complete.
- [x] Solar eclipse hardening only: `jme_sol_eclipse_where()` now fills circumstance magnitude from the returned shadow-surface location instead of geocentric apparent separation, fixing partial-only global eclipses that previously produced negative magnitude; the 2022-10-25 partial solar eclipse is covered as a known-value regression. Rows 54, 56, 57, and 59 stay unchecked pending broader locality/contact validation.
- [x] Row 61, `swe_lun_eclipse_how` / `jme_lun_eclipse_how`: total, partial, and penumbral lunar eclipse circumstances are covered with external known-value maxima/magnitudes, non-eclipse rejection, finite shadow fields, local visibility flag behavior, and null-output rejection.
- [x] Row 62, `swe_lun_eclipse_when` / `jme_lun_eclipse_when`: total, partial, and penumbral global search behavior is covered with external known-value maxima, backward search, contact ordering fields, and null-output rejection.
- [x] Extra `jme_vsop87_planet_state`: J2000 known values, finite AU/day velocities, null-output rejection, and unsupported-body/fallback behavior are covered.
- [x] Extra `jme_moshier_planet_state`: supported-body finite AU/day velocities, null-output rejection, and unsupported-body rejection are covered.
- [x] Extra `jme_elp2000_moon_state`: external Moon known values, finite AU/day velocity, and null-output rejection are covered.
- [x] Extra `jme_meeus_sun_state`: finite heliocentric ecliptic AU/AU-day state and null-output rejection are covered.
- [x] Extra `jme_meeus_moon_state`: finite geocentric ecliptic AU/AU-day state and null-output rejection are covered.
- [x] Extra `jme_meeus_planet_state`: Mercury-through-Pluto finite heliocentric ecliptic AU/AU-day states, null-output rejection, and unsupported-body rejection are covered.

## Swiss 106 Rows Still Not Closed

These are mapped and implemented to varying degrees, but not yet closed under the strict code-proof standard.

- [ ] Row 1, `swe_heliacal_ut` -> `jme_heliacal_ut`
- [ ] Row 2, `swe_heliacal_pheno_ut` -> `jme_heliacal_pheno_ut`
- [ ] Row 3, `swe_vis_limit_mag` -> `jme_vis_limit_mag`
- [ ] Row 4, `swe_heliacal_angle` -> `jme_heliacal_angle`
- [ ] Row 5, `swe_topo_arcus_visionis` -> `jme_topo_arcus_visionis`
- [ ] Row 21, `swe_fixstar` -> `jme_fixstar`
- [ ] Row 22, `swe_fixstar_ut` -> `jme_fixstar_ut`
- [ ] Row 23, `swe_fixstar_mag` -> `jme_fixstar_mag`
- [ ] Row 24, `swe_fixstar2` -> `jme_fixstar2`
- [ ] Row 25, `swe_fixstar2_ut` -> `jme_fixstar2_ut`
- [ ] Row 26, `swe_fixstar2_mag` -> `jme_fixstar2_mag`
- [ ] Row 33, `swe_get_ayanamsa_ex` -> `jme_get_ayanamsa_ex`
- [ ] Row 34, `swe_get_ayanamsa_ex_ut` -> `jme_get_ayanamsa_ex_ut`
- [ ] Row 35, `swe_get_ayanamsa` -> `jme_get_ayanamsa`
- [ ] Row 36, `swe_get_ayanamsa_ut` -> `jme_get_ayanamsa_ut`
- [ ] Row 54, `swe_sol_eclipse_where` -> `jme_sol_eclipse_where`
- [ ] Row 55, `swe_lun_occult_where` -> `jme_lun_occult_where`
- [ ] Row 56, `swe_sol_eclipse_how` -> `jme_sol_eclipse_how`
- [ ] Row 57, `swe_sol_eclipse_when_loc` -> `jme_sol_eclipse_when_loc`
- [ ] Row 58, `swe_lun_occult_when_loc` -> `jme_lun_occult_when_loc`
- [ ] Row 59, `swe_sol_eclipse_when_glob` -> `jme_sol_eclipse_when_glob`
- [ ] Row 60, `swe_lun_occult_when_glob` -> `jme_lun_occult_when_glob`
- [ ] Row 63, `swe_lun_eclipse_when_loc` -> `jme_lun_eclipse_when_loc`

## Extra JME Functions Still Not Closed

The 92 extra functions are split as:

- 91 closed extras
- 1 not-closed extra

### Closed Extra Utility And Metadata Functions

These 31 extra utility/metadata functions are treated as closed by current native code and tests.

- [x] `jme_body_id_from_name`
- [x] `jme_body_naif_id`
- [x] `jme_calendar_is_leap_year`
- [x] `jme_day_of_year`
- [x] `jme_days_in_month`
- [x] `jme_decimal_hour`
- [x] `jme_degrees_to_hours`
- [x] `jme_degrees_to_radians`
- [x] `jme_hours_normalize`
- [x] `jme_hours_to_degrees`
- [x] `jme_jd_add_seconds`
- [x] `jme_jd_difference_seconds`
- [x] `jme_matrix_identity`
- [x] `jme_matrix_multiply`
- [x] `jme_matrix_rotate_x`
- [x] `jme_matrix_rotate_y`
- [x] `jme_matrix_rotate_z`
- [x] `jme_matrix_transform_state`
- [x] `jme_radians_to_degrees`
- [x] `jme_rectangular_to_spherical_state`
- [x] `jme_spherical_angular_separation`
- [x] `jme_spherical_position_angle`
- [x] `jme_spherical_to_rectangular_state`
- [x] `jme_state_add`
- [x] `jme_state_convert_units`
- [x] `jme_state_distance`
- [x] `jme_state_light_time_days`
- [x] `jme_state_position_velocity_dot`
- [x] `jme_state_scale`
- [x] `jme_state_speed`
- [x] `jme_state_subtract`

### Closed Extra Model Helpers

These 5 extra model/reduction helper functions are treated as closed by current native code and tests.

- [x] `jme_get_frame_bias_matrix`
- [x] `jme_get_nutation_matrix`
- [x] `jme_get_obliquity`
- [x] `jme_get_precession_matrix`
- [x] `jme_get_topo_pos`

### Closed Analytical Engine Extras

These 6 analytical engine extras are now closed by current frame/unit/velocity contracts and validation tests.

- [x] `jme_vsop87_planet_state`
- [x] `jme_moshier_planet_state`
- [x] `jme_elp2000_moon_state`
- [x] `jme_meeus_sun_state`
- [x] `jme_meeus_moon_state`
- [x] `jme_meeus_planet_state`

### Closed JPL/CALCEPH Raw Extras

These 49 raw JPL/CALCEPH extras are now closed against a CALCEPH-enabled Linux build plus real capability-specific kernels/files. The runtime test requires finite success paths for SPK body states, text-kernel constants, file-version metadata, binary-PCK orientation, and CALCEPH rotational-angular-momentum data; unsupported capabilities in unrelated kernels are not treated as success.

Verification command:

```bash
LD_LIBRARY_PATH=/tmp/jme-calceph-root/usr/lib/x86_64-linux-gnu \
JME_TEST_JPL_KERNEL=/home/shreesoftech/projects/test1/astro_packages/jpl-ephemeris-/data/jpl/de440s.bsp \
JME_TEST_JPL_CONSTANT_KERNEL=/home/shreesoftech/projects/test1/astro_packages/jpl-ephemeris-/tests/fixtures/jpl_constants.tpc \
JME_TEST_JPL_ORIENTATION_KERNEL=/tmp/jme-kernels/moon_pa_de440_200625.bpc \
JME_TEST_JPL_VERSION_KERNEL=/tmp/calceph-4.0.5/examples/example1.dat \
JME_TEST_JPL_ROTANGMOM_KERNEL=/tmp/calceph-4.0.5/examples/example2_rotangmom.dat \
ctest --test-dir build-calceph -R jpl_runtime --output-on-failure
```

Current result:

```text
1/1 Test #3: jpl_runtime ......................   Passed
```

- [x] `jme_jpl_body_state`
- [x] `jme_jpl_body_state_naif`
- [x] `jme_jpl_body_state_native`
- [x] `jme_jpl_body_state_native_naif`
- [x] `jme_jpl_body_state_native_split`
- [x] `jme_jpl_body_state_native_split_naif`
- [x] `jme_jpl_body_state_order`
- [x] `jme_jpl_body_state_order_naif`
- [x] `jme_jpl_body_state_split`
- [x] `jme_jpl_body_state_split_naif`
- [x] `jme_jpl_body_state_utc`
- [x] `jme_jpl_body_state_utc_naif`
- [x] `jme_jpl_constant`
- [x] `jme_jpl_constant_count`
- [x] `jme_jpl_constant_index`
- [x] `jme_jpl_constant_string`
- [x] `jme_jpl_constant_string_vector`
- [x] `jme_jpl_constant_vector`
- [x] `jme_jpl_coverage`
- [x] `jme_jpl_ecliptic_state`
- [x] `jme_jpl_ecliptic_state_naif`
- [x] `jme_jpl_ecliptic_state_split`
- [x] `jme_jpl_ecliptic_state_split_naif`
- [x] `jme_jpl_ecliptic_state_utc`
- [x] `jme_jpl_ecliptic_state_utc_naif`
- [x] `jme_jpl_engine_version`
- [x] `jme_jpl_file_version`
- [x] `jme_jpl_id_by_name`
- [x] `jme_jpl_is_available`
- [x] `jme_jpl_is_open`
- [x] `jme_jpl_is_thread_safe`
- [x] `jme_jpl_max_supported_order`
- [x] `jme_jpl_name_by_id`
- [x] `jme_jpl_open`
- [x] `jme_jpl_open_array`
- [x] `jme_jpl_orientation_record_count`
- [x] `jme_jpl_orientation_record_index`
- [x] `jme_jpl_orientation_state_naif`
- [x] `jme_jpl_orientation_state_order_naif`
- [x] `jme_jpl_orientation_state_split_naif`
- [x] `jme_jpl_orientation_state_utc_naif`
- [x] `jme_jpl_position_record_count`
- [x] `jme_jpl_position_record_index`
- [x] `jme_jpl_prefetch`
- [x] `jme_jpl_rotational_angular_momentum_state_naif`
- [x] `jme_jpl_rotational_angular_momentum_state_order_naif`
- [x] `jme_jpl_rotational_angular_momentum_state_split_naif`
- [x] `jme_jpl_rotational_angular_momentum_state_utc_naif`
- [x] `jme_jpl_timescale`

### Not-Closed Model And Reduction Helper

This function is implemented, but strict closure requires full supported-model coverage. IAU 1980 and IAU 2000B are covered; IAU 2000A is still intentionally rejected until the full model is imported and validated.

- [ ] `jme_get_nutation`

## Next Recommended Batches

1. Model/reduction helper function: close `jme_get_nutation` only after full IAU 2000A support or a deliberately narrowed public model contract is finalized.
2. Extra JPL raw functions are closed. If new JPL APIs are added, require capability-specific CALCEPH files instead of closing from SPK-only coverage.
