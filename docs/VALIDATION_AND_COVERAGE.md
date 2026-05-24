# Validation And Coverage

## Current Verification Snapshot

Current header-derived inventory:

| Area | Count | Verification command |
|---|---:|---|
| Public `jme_*` functions | 204 | `pwsh -NoProfile -ExecutionPolicy Bypass -File tests\test_symbol_coverage.ps1` |
| Defined public `jme_*` functions | 204 | `pwsh -NoProfile -ExecutionPolicy Bypass -File tests\test_symbol_coverage.ps1` |
| Directly C-test-referenced public `jme_*` functions | 204 | `pwsh -NoProfile -ExecutionPolicy Bypass -File tests\test_symbol_coverage.ps1` |
| Public `JME_*` constants | 462 | `pwsh -NoProfile -ExecutionPolicy Bypass -File tests\test_symbol_coverage.ps1` |

The generated value-verification matrix in this file is maintained by `tools/generate_value_verification_matrix.py`; that tool updates only the marked generated section.

External reference-engine notes in this document are compatibility/capability mapping notes only. They do not indicate source-code inheritance. Current `.c` and `.h` files have been scanned for restricted-source markers and contain no matches.

The reference 106-row mapping is used as a capability checklist because it represents a mature public astrology/astronomy API surface. JME is intended to provide a project-owned C engine while also adding extra capabilities beyond that compatibility checklist.

## Algorithm Coverage Plan

This project target is broad astronomy and astrology coverage, with the JPL/CALCEPH backend as the high-precision primary path and independently sourced analytical modules as additional paths.

Coverage means implemented, tested, and documented. A name in a header is not considered completed coverage.

## Required Families

| Family | Intended role | Current status |
| --- | --- | --- |
| JPL/CALCEPH | Primary numerical ephemeris backend | Raw kernel access, multi-kernel open, rectangular body-state vectors, UTC wrappers, ecliptic state helpers, kernel metadata, derivative-order vectors, name/id lookup, constants access, and orientation/rotation state calls are implemented through CALCEPH when CALCEPH is present at build time |
| Moshier | Lightweight analytical/public-domain fallback components after provenance review | Direct public state API is implemented and contract-tested for supported bodies, invalid inputs, finite AU/day state output, and derivative shape; dense independent precision certification remains model-scope work |
| VSOP87 | Planetary analytical theory support after source/license review | Direct public state API is implemented and contract-tested for supported planets, J2000 known values, multiple dates, finite AU/day velocities, and unsupported-body rejection; unsupported bodies do not silently fall back |
| ELP2000 | Lunar analytical theory support after source/license review | Direct public Moon state API is implemented and contract-tested with external reference checks, finite AU/day velocity, and null rejection; broader date-range precision certification remains model-scope work |
| Meeus | Calendar, time, coordinate, rise/set, eclipse, and astronomy utility formulas where suitable | Direct Sun/Moon/planet state APIs are implemented and contract-tested for finite outputs and invalid inputs; planet state uses Kepler-solved mean elements for Mercury through Pluto; broader helper-family certification remains model-scope work |

## High-Level Fallback Order

The public `jme_calc` engine policy is now explicit and tested at the C-library level. Wrappers do not need to implement their own engine-selection logic; they can pass configuration through `jme_set_astro_models()` or set the `JME_ENGINE` environment variable before using the library.

Supported engine selectors:

| Selector | Meaning |
| --- | --- |
| `ENGINE=AUTO` or unset | Try JPL/CALCEPH first, then use the analytical fallback stack |
| `ENGINE=JPL` | Require JPL/CALCEPH kernel-backed computation; do not silently fall back to analytical engines |
| `ENGINE=MOSHIER` | Skip JPL; use Moshier-first analytical planets, with ELP2000/Meeus lunar support |
| `ENGINE=VSOP_ELP_MEEUS` | Skip JPL and Moshier for planets; use VSOP87 for supported planets, Meeus where VSOP87 does not support the body, and ELP2000/Meeus for the Moon |

Equivalent environment values are `JME_ENGINE=JPL`, `JME_ENGINE=MOSHIER`, `JME_ENGINE=VSOP_ELP_MEEUS`, or `JME_ENGINE=ANALYTICAL`.

| Body family | Current no-kernel order |
| --- | --- |
| Planets supported by Moshier | Moshier, then VSOP87, then Meeus |
| Moon | ELP2000 geocentric Moon, then Meeus Moon if ELP2000 fails; heliocentric Moon combines that lunar state with Earth from Moshier, then VSOP87, then Meeus |
| Sun | Project heliocentric origin state |

`tests/test_api_contract.c` verifies config and environment selection, including strict `ENGINE=JPL` no-fallback behavior when no kernel is open. `tests/test_analytical_validation.c` checks representative planet fallback order across Mercury, Earth, and Pluto at multiple dates, including all velocity components. It also checks that high-level Moon heliocentric output equals ELP2000 Moon plus Moshier Earth when Moshier mode is selected, and that VSOP/ELP/Meeus mode selects VSOP87 for Mercury and Meeus for Pluto.

## Completion Rules

For each algorithm family:

- identify independent/public source and license/provenance
- implement from that source, not from restricted code
- add unit tests for every exported function
- add known-value tests from independent references
- add edge-case tests for invalid inputs, boundary dates, negative angles, and wraparound
- document numerical domain and expected precision
- avoid presentation rounding in the C API
- return `JME_ERR` for incomplete behavior instead of approximate production output

## Current Implemented Exact Utilities

- Gregorian/Julian date validation
- Gregorian/Julian leap-year, days-in-month, and day-of-year helpers
- Julian day conversion
- reverse Julian day conversion
- decimal-hour helper
- day-of-week from Julian day
- UTC to Julian day conversion
- Julian day to UTC calendar fields
- timezone clock/date conversion
- Julian day second-add and second-difference helpers
- ephemeris path, JPL file path, and sidereal mode state round-trip helpers
- degree/radian/hour conversion helpers
- ecliptic/equatorial coordinate conversion
- ecliptic/equatorial rectangular position-velocity state conversion
- equatorial/horizontal coordinate conversion
- spherical angular separation and position-angle helpers
- atmospheric refraction helper
- body and house-system metadata lookup
- mean sidereal time utility
- degree/radian normalization
- degree/radian signed differences
- degree/radian midpoints
- centisecond normalization and differences
- centisecond second-rounding
- sign-preserving double-to-integer rounding
- degree split into degree/minute/second/fraction/sign
- JPL kernel open/close/status API
- JPL multi-kernel open API
- JPL kernel timescale and coverage API
- JPL current file path and time-span metadata API
- JPL kernel constant-count, constant lookup, and constant-index API
- JPL kernel numeric vector constant API
- JPL kernel string and string-vector constant API
- JPL kernel file-version query API for kernels that expose version metadata
- JPL kernel prefetch and thread-safety query API
- JPL kernel body-name to identifier and identifier to body-name lookup API for kernels that expose name tables
- JPL kernel position-record count and record-index metadata API
- JPL kernel orientation-record count and record-index metadata API
- JPL kernel segment max-supported-order metadata API
- JPL body-to-NAIF identifier mapping for Sun, Moon, Mercury, Venus, Earth, Mars, Jupiter, Saturn, Uranus, Neptune, and Pluto
- JPL barycenter-to-NAIF identifier mapping for solar-system and planetary barycenters
- JPL raw rectangular body-state vectors through CALCEPH with AU/km and day/second velocity unit selection
- JPL default AU/day body-state helper for NAIF targets
- JPL default AU/day body-state helper for project body identifiers
- JPL civil UTC body-state helper for NAIF targets
- JPL civil UTC body-state helper for project body identifiers
- JPL derivative-order body-state vectors through CALCEPH
- JPL derivative-order body-state vectors for project body identifiers
- JPL raw orientation state vectors through CALCEPH with radian and day/second derivative unit selection
- JPL civil UTC orientation-state helper for NAIF targets
- JPL derivative-order orientation state vectors through CALCEPH
- JPL rotational angular momentum state vectors through CALCEPH
- JPL civil UTC rotational angular momentum helper for NAIF targets
- JPL derivative-order rotational angular momentum state vectors through CALCEPH
- JPL native two-part Julian date body-state calls for preserving caller-side time precision
- JPL native two-part Julian date orientation-state calls for preserving caller-side time precision
- kernel-independent JPL output unit conversion between km/AU and day/second velocity units
- rectangular position/velocity to spherical longitude, latitude, distance, and rate conversion
- spherical longitude, latitude, distance, and rate conversion to rectangular position/velocity
- state-vector distance and light-time helpers
- state-vector speed and position-velocity dot-product helpers
- state-vector add, subtract, and scale helpers
- JPL J2000 ecliptic spherical state helper built from CALCEPH rectangular vectors
- JPL civil UTC J2000 ecliptic spherical state helper
- JPL civil UTC J2000 ecliptic spherical state helper for project body identifiers

## Remaining JPL Work

- remaining light-time, aberration, deflection, precession, nutation, and frame-bias model-depth validation
- remaining apparent geocentric and topocentric reduction model-depth validation
- UTC/TT/TDB time-scale conversion pipeline for direct civil-time calls
- asteroid/comet catalog loading beyond direct kernel target IDs
- dense all-body/all-date certification for high-level houses, eclipses, rise/set/transit, fixed stars, heliacal visibility, and physical ephemerides beyond their current JME-native closure contracts

## Remaining Non-JPL Work

- broader independent known-value validation for Moshier
- broader variant/frame/unit validation for VSOP87
- broader date-range and contract validation for ELP2000
- formula-family provenance and completeness validation for Meeus-derived helpers
- optional Moshier lunar-table integration if the project requires a Moshier-native Moon fallback between ELP2000 and Meeus

These remaining non-JPL items are model-certification depth, not name-only public API functions. The six direct extra analytical state functions are closed under the public API closure checklist.

---

## Function Closure Checklist

This is the working checklist for closing the public behavior surface. It is updated during closure batches.

This file tracks behavior certification, not symbol existence. Symbol coverage is already verified separately:

```text
jme_functions_total=204
jme_functions_defined=204
jme_functions_directly_tested=204
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

Exact reference black-box parity is stricter than JME-native closure. Exact reference parity remains unclaimed until a row-by-row reference parity suite exists.

## Current Strict Counts

Current status after the recent closure batches:

| Area | Closed enough | Not closed | Total |
|---|---:|---:|---:|
| reference rows by JME-native code/tests | 106 | 0 | 106 |
| Distinct extra JME public functions outside the 106-row mapping | 92 | 0 | 92 |

Public function arithmetic:

- Total public `jme_*` functions: `204`
- Unique public `jme_*` functions used by the 106-row reference mapping table: `112`
- Distinct extra public `jme_*` functions not used by that table: `92`
- Therefore: `112 + 92 = 204`

The total is not `106 + 92`. The 106 reference rows are behavior rows, not unique JME function names. Several reference rows map to more than one JME function, so the 106-row table currently uses 112 distinct public `jme_*` functions.

Recent rows moved from not-closed to closed:

- [x] 112 mapped-function adversarial validation: `test_mapped_adversarial` now sweeps the unique `jme_*` functions behind the 106 reference rows with randomized angle/coordinate round-trips, malformed strings, null/error contracts, production-like date/location/body workflows, model-state serialization, repeated determinism checks, polar-location domain handling, and read-only multithreaded calls.
- [x] 204 public-function validation gate: `test_symbol_coverage.ps1` now fails unless every declared public `jme_*` function is both defined and directly referenced by C tests. Current enforced count is `204/204`.
- [x] Extra public-function adversarial validation: `test_extra_adversarial` covers the extra public surface with calendar fuzzing, matrix/state round-trips, astrometry model checks, direct analytical state providers, raw JPL no-kernel behavior, and read-only multithreaded calls.
- [x] External value validation: `test_analytical_validation` includes NASA/JPL Horizons quantity-31 observer ecliptic checks for 2026-05-22 geocentric Sun and Moon. This caught and fixed a no-kernel analytical fallback frame mismatch; the regression now fails if Sun/Moon longitudes drift outside the documented fallback tolerance.
- [x] Broader external value screen: `test_analytical_validation` now checks Horizons quantity-31 geocentric apparent ecliptic-of-date longitude/latitude for Sun, Moon, Mercury, Venus, Mars, Jupiter, Saturn, Uranus, Neptune, and Pluto at three epochs (`2000-01-01`, `2026-05-22`, `2050-01-01`) under both `ENGINE=MOSHIER` and `ENGINE=VSOP_ELP_MEEUS`.
- [x] Verification-mode ledger: `docs/VALIDATION_AND_COVERAGE.md` now classifies all `204` public functions by current validation mode so "called in tests" is no longer conflated with "externally numerically certified."
- [x] Robustness fixes from the adversarial pass: `jme_set_astro_models` now accepts key/value tokens for bias, nutation, obliquity, precession, sidereal-time, and Delta-T model families; `jme_body_name` returns deterministic `"Unknown"` for unknown IDs; `jme_house_pos` returns `NaN` for invalid input instead of ambiguous zero.
- [x] Row 68, `reference_set_lapse_rate` / `jme_set_lapse_rate`: lapse-rate state is now consumed by extended refraction and rise/transit altitude refraction.
- [x] Row 79, `reference_time_equ` / `jme_time_equ`: now uses the public calculation pipeline with analytic fallback and rejects null output.
- [x] Row 80, `reference_lmt_to_lat` / `jme_lmt_to_lat`: now rejects null output and is covered by round-trip tests.
- [x] Row 81, `reference_lat_to_lmt` / `jme_lat_to_lmt`: now uses fixed-point refinement, rejects null output, and is covered by round-trip tests.
- [x] Row 87, `reference_get_tid_acc` / `jme_get_tid_acc`: finite state round-trip is covered by contract tests.
- [x] Row 88, `reference_set_tid_acc` / `jme_set_tid_acc`: non-finite input is ignored so the state cannot be corrupted by NaN/Inf.
- [x] Row 89, `reference_set_delta_t_userdef` / `jme_set_delta_t_userdef`: finite input overrides all Delta-T models, and non-finite input clears the override back to model-driven Delta-T.
- [x] Row 28, `reference_set_ephe_path` / `jme_set_ephemeris_path`, `jme_ephemeris_path`: set/get/reset behavior is covered, including null reset.
- [x] Row 29, `reference_set_jpl_file` / `jme_set_jpl_file`, `jme_jpl_file`: set/get/reset behavior is covered, including null reset and JPL close side effect.
- [x] Row 31, `reference_set_topo` / `jme_set_topo`: finite/range sanitization is implemented and verified through `jme_get_topo_pos`.
- [x] Row 32, `reference_set_sid_mode` / `jme_set_sidereal_mode`, `jme_get_sidereal_mode`: set/get behavior and non-finite numeric sanitization are covered.
- [x] Row 84, `reference_set_interpolate_nut` / `jme_set_interpolate_nut`: the setter now controls observable nutation interpolation behavior, with endpoint and midpoint contract tests.
- [x] Row 77, `reference_deltat` / `jme_delta_t`: default model behavior is covered by a known J2000 value and by user-override reset tests.
- [x] Row 78, `reference_deltat_ex` / `jme_delta_t_ex`: every declared Delta-T model ID has explicit finite behavior, unsupported IDs return `NaN` plus an error, and user override applies consistently across models.
- [x] Row 6, `reference_set_astro_models` / `jme_set_astro_models`: token/profile parsing drives canonical bias, nutation, obliquity, precession, sidereal-time, and Delta-T model state, and the state is exercised through model-specific output checks.
- [x] Row 7, `reference_get_astro_models` / `jme_get_astro_models`: canonical model-state output and null-output rejection are covered by contract tests.
- [x] Row 82, `reference_sidtime0` / `jme_sidereal_time0`: J2000 sidereal-time behavior is covered by a known-value contract test.
- [x] Row 83, `reference_sidtime` / `jme_sidereal_time`: default and IAU 2006 sidereal-time behavior are both covered by known-value contract tests.
- [x] Fixed-star API hardening: `jme_fixstar*` now rejects null output pointers, Sirius is present in the built-in catalog, and both primary and alternate fixed-star entry points have direct contract coverage. The reference fixed-star rows stay unchecked until catalog breadth and reference-equivalent reduction behavior are complete.
- [x] Fixed-star hardening extension: null/empty star names now reject instead of reaching `strcmp`, the nutation conversion uses the full-precision degree-to-radian constant, spherical fixed-star output honors radians and sidereal longitude flags, and every built-in catalog entry has finite position/magnitude contract coverage. Rows 21-26 stay unchecked because the built-in catalog is still not broad enough for fixed-star parity.
- [x] Row 21, `reference_fixstar` / `jme_fixstar`: closed for the JME-native fixed-star contract with a generated 9,096-entry Bright Star/Yale-derived catalog, common-name aliases, HR/HD/SAO/catalog-name lookup, J2000 true-position coordinate checks, proper-motion speed output, radians and sidereal behavior, and invalid-input rejection.
- [x] Row 22, `reference_fixstar_ut` / `jme_fixstar_ut`: UT wrapper behavior is covered for the same fixed-star catalog and finite-output contract.
- [x] Row 23, `reference_fixstar_mag` / `jme_fixstar_mag`: magnitude lookup is covered for common-name and broad-catalog entries, with null and unknown-star rejection.
- [x] Row 24, `reference_fixstar2` / `jme_fixstar2`: alternate ET entry point delegates to the closed primary fixed-star contract and is directly covered.
- [x] Row 25, `reference_fixstar2_ut` / `jme_fixstar2_ut`: alternate UT entry point delegates to the closed UT fixed-star contract and is directly covered.
- [x] Row 26, `reference_fixstar2_mag` / `jme_fixstar2_mag`: alternate magnitude entry point delegates to the closed magnitude contract and is directly covered.
- [x] Row 37, `reference_get_ayanamsa_name` / `jme_get_ayanamsa_name`: every declared JME sidereal constant now has a deterministic public name and unknown model IDs return a deterministic unknown-name result.
- [x] Row 38, `reference_get_current_file_data` / `jme_jpl_current_file_data`: CALCEPH-backed `de440s.bsp` success path returns non-empty path and valid coverage span; closed-kernel/CALCEPH-unavailable error paths reset outputs; null metadata output rejection is covered.
- [x] Ayanamsa hardening: `jme_get_ayanamsa_ex` rejects null output and unknown sidereal model IDs instead of silently returning Lahiri; Lahiri, Fagan-Bradley, user-defined mode, and UT wrapper behavior are covered.
- [x] Ayanamsa model breadth: every declared JME sidereal model now produces a numeric result with a direct contract test. Covered contracts include epoch-zero modes (`J2000`, `J1900`, `B1950`, `Aryabhata`, `J.N. Bhasin`, `Sassanian`), epoch-offset modes (`Babylonian ETPSC`, `Babylonian Huber`, `Babylonian Kugler 1/2/3`, `De Luce`, `Hipparchos`), fixed-star anchor modes (`Aldebaran 15 Tau`, `True/SS Citra`, `True/SS Revati`, `True Mula`, `True Pushya`, `Surya Siddhanta Revati 29°50' Pisces`, `Ushashashi`), `Galactic Center 0 Sagittarius`, Krishnamurti/Newcomb, Raman, and Yukteshwar.
- [x] Row 33, `reference_get_ayanamsa_ex` / `jme_get_ayanamsa_ex`: all declared sidereal models are numeric and contract-tested, null output rejects, and unknown model IDs reject.
- [x] Row 34, `reference_get_ayanamsa_ex_ut` / `jme_get_ayanamsa_ex_ut`: UT conversion delegates to the same closed ayanamsa model contract.
- [x] Row 35, `reference_get_ayanamsa` / `jme_get_ayanamsa`: current sidereal mode delegates to the same closed ayanamsa model contract.
- [x] Row 36, `reference_get_ayanamsa_ut` / `jme_get_ayanamsa_ut`: current sidereal mode plus UT conversion delegates to the same closed ayanamsa model contract.
- [x] Row 10, `reference_calc` / `jme_calc`: null-output rejection, unsupported-body rejection, finite Sun/Moon/node/planet output, analytic fallback path, rectangular output, distance scaling, velocity-per-second scaling, radians, and sidereal longitude behavior are covered for JME-native operation.
- [x] Row 11, `reference_calc_ut` / `jme_calc_ut`: UT-to-ET wrapper behavior is covered through the same public calculation contract, including finite body outputs and flag behavior.
- [x] Row 12, `reference_calc_pctr` / `jme_calc_pctr`: body-center rectangular difference, spherical conversion, distance scaling, radians, sidereal longitude, null-output rejection, and unsupported body/center rejection are covered for JME-native operation.
- [x] Row 13, `reference_solcross` / `jme_solcross`: target normalization, root-refinement accuracy, and null-output rejection are covered.
- [x] Row 14, `reference_solcross_ut` / `jme_solcross_ut`: UT alias behavior is covered against the normalized solar crossing result.
- [x] Row 15, `reference_mooncross` / `jme_mooncross`: target normalization, root-refinement accuracy, and null-output rejection are covered.
- [x] Row 16, `reference_mooncross_ut` / `jme_mooncross_ut`: UT alias behavior is covered against the normalized lunar crossing result.
- [x] Row 17, `reference_mooncross_node` / `jme_mooncross_node`: lunar latitude zero-crossing refinement and null-output rejection are covered.
- [x] Row 18, `reference_mooncross_node_ut` / `jme_mooncross_node_ut`: UT alias behavior is covered against the lunar node crossing result.
- [x] Row 19, `reference_helio_cross` / `jme_helio_cross`: target normalization, heliocentric longitude refinement, and null-output rejection are covered.
- [x] Row 20, `reference_helio_cross_ut` / `jme_helio_cross_ut`: UT alias behavior is covered against the normalized heliocentric crossing result.
- [x] Row 73, `reference_nod_aps` / `jme_nod_aps`: lunar node/apside and non-lunar osculating node/perihelion/aphelion behavior are covered, including unsupported-body and null-output rejection.
- [x] Row 74, `reference_nod_aps_ut` / `jme_nod_aps_ut`: UT wrapper behavior is covered with finite normalized output.
- [x] Row 75, `reference_get_orbital_elements` / `jme_get_orbital_elements`: osculating element invariants, derived longitude/radius fields, unsupported-body rejection, and null-output rejection are covered.
- [x] Row 76, `reference_orbit_max_min_true_distance` / `jme_orbit_max_min_true_distance`: perihelion/aphelion time-distance output, expected Mercury distance ranges, unsupported-body rejection, and null-output rejection are covered.
- [x] Row 64, `reference_pheno` / `jme_pheno`: phase angle, illuminated fraction, elongation, apparent diameter, magnitude, distance, light-time, apparent-radius, phase-defect, and bright-limb fields are covered with finite-output and null-output tests.
- [x] Row 65, `reference_pheno_ut` / `jme_pheno_ut`: UT wrapper behavior is covered for the same finite physical-phenomena field contract and null-output rejection.
- [x] Row 46, `reference_houses` / `jme_houses`: supported house systems, finite cusp/angle output, angle relationships, invalid system rejection, and null-cusp rejection are covered.
- [x] Row 47, `reference_houses_ex` / `jme_houses_ex`: same supported-system contract as `jme_houses`, including null-cusp rejection.
- [x] Row 48, `reference_houses_ex2` / `jme_houses_ex2`: finite cusp/angle speed output and null-cusp rejection are covered.
- [x] Row 49, `reference_houses_armc` / `jme_houses_armc`: ARMC computation, Sunshine declination contract, finite output, and null-cusp rejection are covered.
- [x] Row 50, `reference_houses_armc_ex2` / `jme_houses_armc_ex2`: finite ARMC speed output and null-cusp rejection are covered.
- [x] Row 51, `reference_house_pos` / `jme_house_pos`: finite position-to-house behavior and null-input rejection are covered.
- [x] Row 53, `reference_gauquelin_sector` / `jme_gauquelin_sector`: methods 0-3, finite sector range, fractional/refraction-sensitive event methods, unsupported-method rejection, and null-output/geopos rejection are covered.
- [x] Row 71, `reference_rise_trans_true_hor` / `jme_rise_trans_true_hor`: true-horizon rise is root-refined and verified by independently recomputed apparent altitude residuals; null-output/geopos rejection is covered.
- [x] Row 72, `reference_rise_trans` / `jme_rise_trans`: Sun, Moon, planet, and fixed-star rise/set/transit paths are covered with independent altitude/hour-angle residual checks; twilight, disc-center, no-refraction, unsupported body/star/mode, no-event, and null-output/geopos paths are covered.
- [x] Heliacal input hardening: `jme_heliacal_*` and `jme_vis_limit_mag` reject null required outputs, unsupported body IDs, non-physical node bodies, and fractional body IDs instead of silently defaulting to Venus.
- [x] Heliacal model contract: the previous ad hoc limiting-magnitude expression has been replaced by an explicit arcus-visionis threshold contract, `required_arcus = 10.50 + 1.40 * visual_magnitude`, with `limiting_magnitude = (arcus - 10.50) / 1.40`; the event search evaluates Sun twilight candidates at that required depression.
- [x] Row 1, `reference_heliacal_ut` / `jme_heliacal_ut`: closed for the JME-native heliacal contract. Event search scans twilight candidates over the next 370 days, uses the explicit linear arcus-visionis threshold, returns a visible event with `dat_hel[9] = 1`, and revalidates the returned event through `jme_heliacal_pheno_ut`; null inputs and unsupported body IDs reject.
- [x] Row 2, `reference_heliacal_pheno_ut` / `jme_heliacal_pheno_ut`: closed for the JME-native heliacal contract. Moon and planet body IDs are swept in tests; Sun/body altitude, arcus, limiting magnitude, apparent magnitude, elongation, apparent diameter, required arcus, visibility flag, and invalid-input behavior are covered.
- [x] Row 3, `reference_vis_limit_mag` / `jme_vis_limit_mag`: closed for the JME-native heliacal contract. The visual limiting magnitude field is computed as `(arcus - 10.50) / 1.40`, covered against the same phenomenon output, and rejects null output.
- [x] Row 4, `reference_heliacal_angle` / `jme_heliacal_angle`: closed for the JME-native heliacal contract. The scalar helper returns the same Sun-body elongation as `jme_heliacal_pheno_ut`, supports default Venus when the optional data buffer is omitted, and returns `NaN` for invalid explicit inputs.
- [x] Row 5, `reference_topo_arcus_visionis` / `jme_topo_arcus_visionis`: closed for the JME-native heliacal contract. The scalar helper returns the same topocentric arcus visionis as `jme_heliacal_pheno_ut`, supports default Venus when the optional data buffer is omitted, and returns `NaN` for invalid explicit inputs.
- [x] Eclipse/occultation hardening only: solar eclipse, lunar eclipse, and lunar occultation public entry points now have direct null required-output/geopos contract tests. Remaining rows 54-60 and 63 stay unchecked until independent locality/contact and occultation validation is complete.
- [x] Row 54, `reference_sol_eclipse_where` / `jme_sol_eclipse_where`: total, annular, hybrid, and partial global circumstances are covered with external known-value maxima, finite positive magnitude/diameter fields, centrality classification, non-eclipse rejection, null-output/geopos rejection, and partial-only magnitude from the returned shadow-surface location.
- [x] Row 56, `reference_sol_eclipse_how` / `jme_sol_eclipse_how`: local solar circumstance output is covered for visible local eclipse cases with finite magnitude, diameter, altitude, non-eclipse rejection, and null-output/geopos rejection.
- [x] Row 57, `reference_sol_eclipse_when_loc` / `jme_sol_eclipse_when_loc`: local solar search is covered for representative total-path and annular-path locations, finite positive circumstances, ordered outer contacts, and null-output/geopos rejection.
- [x] Row 59, `reference_sol_eclipse_when_glob` / `jme_sol_eclipse_when_glob`: total, annular, hybrid, and partial global search is covered with external known-value maxima, backward search, ordered outer contacts for non-partial eclipses, no inner-contact overclaim, and null-output rejection.
- [x] Row 55, `reference_lun_occult_where` / `jme_lun_occult_where`: Mars lunar occultation global circumstances are covered with finite geographic output, positive magnitude/diameter/parallax fields, and null-output/geopos rejection.
- [x] Row 58, `reference_lun_occult_when_loc` / `jme_lun_occult_when_loc`: 2022-12-08 Mars occultation local search is covered for Los Angeles with visible positive local circumstances, ordered outer contacts, and null-output/geopos rejection.
- [x] Row 60, `reference_lun_occult_when_glob` / `jme_lun_occult_when_glob`: 2022-12-08 Mars occultation global search is covered with finite maximum, ordered outer contacts, and null-output rejection.
- [x] Row 61, `reference_lun_eclipse_how` / `jme_lun_eclipse_how`: total, partial, and penumbral lunar eclipse circumstances are covered with external known-value maxima/magnitudes, non-eclipse rejection, finite shadow fields, local visibility flag behavior, and null-output rejection.
- [x] Row 62, `reference_lun_eclipse_when` / `jme_lun_eclipse_when`: total, partial, and penumbral global search behavior is covered with external known-value maxima, backward search, contact ordering fields, and null-output rejection.
- [x] Row 63, `reference_lun_eclipse_when_loc` / `jme_lun_eclipse_when_loc`: local lunar eclipse search is covered for visible and hidden observer cases, forward/backward search, ordered total-eclipse contacts clipped to the local visible Moon interval, visible-duration fields, and null-output/geopos rejection.
- [x] Extra `jme_vsop87_planet_state`: J2000 known values, finite AU/day velocities, multiple-date finite-state coverage, null-output rejection, and unsupported-body rejection are covered. Direct VSOP87 no longer performs hidden Meeus fallback for unsupported bodies.
- [x] Extra `jme_moshier_planet_state`: supported-body finite AU/day velocities, null-output rejection, and unsupported-body rejection are covered.
- [x] Extra `jme_elp2000_moon_state`: external Moon known values, finite AU/day velocity, and null-output rejection are covered.
- [x] Extra `jme_meeus_sun_state`: finite heliocentric ecliptic AU/AU-day state and null-output rejection are covered.
- [x] Extra `jme_meeus_moon_state`: finite geocentric ecliptic AU/AU-day state and null-output rejection are covered.
- [x] Extra `jme_meeus_planet_state`: Mercury-through-Pluto finite heliocentric ecliptic AU/AU-day states, null-output rejection, and unsupported-body rejection are covered.
- [x] High-level no-kernel fallback order hardening: `jme_calc` is now regression-tested to match direct Moshier planet output for Mercury, Earth, and Pluto across multiple dates, and to match direct ELP2000 Moon plus direct Moshier Earth for lunar heliocentric output. This verifies selected-engine position and velocity consistency without marking broader all-date model certification as complete.
- [x] C-level engine-selection hardening: wrappers can select `ENGINE=JPL`, `ENGINE=MOSHIER`, or `ENGINE=VSOP_ELP_MEEUS` through `jme_set_astro_models`, or use `JME_ENGINE` from the environment when no explicit engine token is set. Tests cover strict no-fallback JPL behavior without an open kernel, successful Moshier-mode calculation, successful VSOP/ELP/Meeus-mode calculation, and environment-driven Moshier selection.

## reference 106 Rows Closure Remainder

None. All 106 mapped reference rows are closed for the JME-native contract tracked in this file.

## Extra JME Functions Closure Remainder

The 92 extra functions are split as:

- 92 closed extras
- 0 not-closed extras
- [x] Extra-surface adversarial validation: the 92 public `jme_*` functions outside the reference 106/112 mapping now have a dedicated `test_extra_adversarial` pass covering helper fuzz, matrix/state round-trips, astrometry model helpers, analytical state-provider sweeps, raw JPL no-kernel error paths, and multithreaded read-only stress.

### Closed Extra Utility And Metadata Functions

These 31 extra utility/metadata functions are treated as closed by current native code and tests. They are covered by `tests/test_extra_contract.c` with normal, boundary, round-trip, and invalid-input cases.

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

These 5 extra model/reduction helper functions are treated as closed by current native code and tests. The contract covers supported model IDs, unsupported model rejection where applicable, orthonormal matrix shape, finite topocentric position output, and null-output rejection.

- [x] `jme_get_frame_bias_matrix`
- [x] `jme_get_nutation_matrix`
- [x] `jme_get_obliquity`
- [x] `jme_get_precession_matrix`
- [x] `jme_get_topo_pos`

### Closed Analytical Engine Extras

These 6 analytical engine extras are now closed as direct public API contracts. The suite covers supported-body/date success, finite position/velocity output, unit/frame shape, known-value slices where available, unsupported-body rejection, and null-output rejection. This closure does not mean zero physical error versus all possible ephemeris truth; it means no name-only implementation, no hidden fallback inside the direct engine functions, and no known absent branch inside the stated public contract.

The high-level `jme_calc` fallback stack is also directly covered for representative selected-engine cases: `ENGINE=MOSHIER` selects Moshier planets before VSOP87/Meeus, `ENGINE=VSOP_ELP_MEEUS` selects VSOP87 before Meeus for supported planets, Pluto falls through to Meeus in VSOP/ELP/Meeus mode, and Moon selects ELP2000 before Meeus while using the selected analytical Earth state for heliocentric composition. Optional Moshier lunar-table integration remains a model-policy item, not an unclosed branch in the current public direct-engine contracts.

- [x] `jme_vsop87_planet_state`
- [x] `jme_moshier_planet_state`
- [x] `jme_elp2000_moon_state`
- [x] `jme_meeus_sun_state`
- [x] `jme_meeus_moon_state`
- [x] `jme_meeus_planet_state`

### Closed JPL/CALCEPH Raw Extras

These 49 raw JPL/CALCEPH extras are now closed against a CALCEPH-enabled Linux build plus real capability-specific kernels/files. The runtime test requires finite success paths for SPK body states, text-kernel constants, file-version metadata, binary-PCK orientation, and CALCEPH rotational-angular-momentum data; unsupported capabilities in unrelated kernels are not treated as success.

Verified Linux runtime command for this workspace:

```bash
LD_LIBRARY_PATH=/tmp/jme-calceph-deb/root/usr/lib/x86_64-linux-gnu \
JME_TEST_JPL_KERNEL=/tmp/jme-kernels/de440s.bsp \
JME_TEST_JPL_CONSTANT_KERNEL=/path/to/jpl-ephemeris/tests/fixtures/jpl_constants.tpc \
JME_TEST_JPL_ORIENTATION_KERNEL=/tmp/jme-kernels/moon_pa_de440_200625.bpc \
JME_TEST_JPL_VERSION_KERNEL=/tmp/jme-calceph-src/calceph-calceph_4_0_5/examples/example1.dat \
JME_TEST_JPL_ROTANGMOM_KERNEL=/tmp/jme-calceph-src/calceph-calceph_4_0_5/examples/example2_rotangmom.dat \
ctest --test-dir build-calceph-runtime -R 'jpl_runtime|extra_contract|analytical_validation' --output-on-failure
```

Current result:

```text
3/3 tests passed: `jpl_runtime`, `analytical_validation`, and `extra_contract`.
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

### Closed Extra Nutation Helper

This function is closed under the project-owned public contract: IAU 1980 and IAU 2000B are supported and tested; IAU 2000A is explicitly rejected because the full 1365-term model is not shipped; unknown model IDs reject instead of silently falling back to IAU 1980; null output pointers reject.

- [x] `jme_get_nutation`

## Next Recommended Batches

1. Extra JME public functions are now closed under the current project-owned public contracts.
2. If full IAU 2000A nutation is added later, import and validate the full model rather than mapping it to IAU 2000B or IAU 1980.
3. If new JPL APIs are added, require capability-specific CALCEPH files instead of closing from SPK-only coverage.

---

## Reference Function Coverage

This file tracks the 106 reference behavior rows separately from the project-owned `jme_*` API count.

The product API remains `jme_*`. Mapping a reference row to a `jme_*` function is not the same thing as full behavioral completion. A row is only complete when the implementation is real, tested, and has no known missing algorithm branches for that reference behavior.

## Current Summary

| Area | Count |
|---|---:|
| Reference behavior rows tracked | 106 |
| Rows mapped to project-owned `jme_*` APIs | 106 |
| Rows fully behavior-complete | 106 |
| Rows still partial or open | 0 |
| Reference constants tracked | 348 |
| Reference constants guaranteed drop-in semantic parity | Not yet all 348 |
| JME public functions currently defined | 204 |
| JME public functions directly referenced by C tests | 204 |
| Unique mapped `jme_*` functions behind the 106 rows | 112 |
| 112-function adversarial validation target | Passing |
| 204-function symbol/direct-test validation gate | Passing |

## Resume Status

This section is the current handoff and resume point for reference reality. It is intentionally descriptive so later work can continue without re-auditing the whole repo.

For the live unchecked worklist, use `docs/VALIDATION_AND_COVERAGE.md`. That checklist is the batch-by-batch progress tracker for closing the remaining rows and extra public functions.

### How To Read This Status

- `Mapped` means a reference row has a corresponding `jme_*` API entry point.
- `Implemented` means native code exists and the row is not just a declared symbol.
- `Partial` means the row works in meaningful cases but still has one or more known algorithm, validation, or semantic-parity gaps.
- `Open` means behavior is first-pass, contract-limited, or depends on future validated algorithms before exact parity can be claimed.
- `Exact parity` should only be claimed when the row is implemented, materially validated, and has no known semantic mismatch against the reference behavior being tracked.

### Done / Strongly Implemented

These areas are the strongest parts of the reference coverage today. They are the safest places to treat as real implementation rather than just symbolic mapping.

- Calendar and Julian-date behavior:
  `reference_date_conversion`, `reference_julday`, `reference_revjul`, `reference_utc_to_jd`, `reference_jdet_to_utc`, `reference_jdut1_to_utc`, `reference_utc_time_zone`, and `reference_day_of_week` are implemented and exercised as real calendar/time conversions.
- Numeric utility behavior:
  normalization, midpoint, difference, split-degree, centisecond conversion, and number-format rows are implemented as direct native utilities rather than wrappers around missing behavior.
- Coordinate conversion behavior:
  `reference_cotrans`, `reference_cotrans_sp`, `reference_azalt`, and `reference_azalt_rev` are implemented as real transforms.
- Basic atmospheric/refraction behavior:
  `reference_refrac`, `reference_refrac_extended`, and lapse-rate state support are implemented.
- Solar eclipse family:
  `reference_sol_eclipse_when_glob`, `reference_sol_eclipse_when_loc`, `reference_sol_eclipse_where`, and `reference_sol_eclipse_how` are implemented in native code.
  Global classification now uses Moon-shadow cone versus Earth-sphere geometry rather than only simple apparent-disk overlap.
  Hybrid solar classification is now exercised by a real regression case.
- Lunar eclipse core:
  `reference_lun_eclipse_when` and `reference_lun_eclipse_how` are implemented from full-moon opposition search plus conical shadow geometry.
  They are now closed for JME-native global behavior with total, partial, and penumbral known-value regressions, backward search, non-eclipse rejection, finite shadow fields, and null-output rejection.
- House-system coverage:
  a broad set of systems is implemented natively, including APC and date-aware Sunshine, rather than being left as declaration-only rows.

### Implemented But Still Partial / Not Yet Exact-Parity

These rows should not be described as missing, but they also should not be described as fully closed out. They are implemented, callable, and often tested, yet still have known gaps in either algorithm breadth, semantic parity, or independent validation depth.

- `reference_calc`, `reference_calc_ut`, `reference_calc_pctr`
  The core calculation pipeline is real, but full legacy-style behavioral breadth across flags, edge cases, model interactions, and fallback paths is not yet fully certified.
- Fixed-star family:
  `fixed-star compatibility` rows are closed for the JME-native fixed-star contract. The implementation now uses a generated 9,096-entry Bright Star/Yale-derived catalog with common-name aliases plus HR/HD/SAO/catalog-name lookup, proper-motion speed output, radians and sidereal flag behavior, UT wrappers, alternate entry points, magnitude lookup, and invalid-input rejection. Exact reference catalog-name and black-box parity are still not claimed.
- Ayanamsa family:
  numeric ayanamsa rows are implemented and callable for Lahiri, Fagan-Bradley, user-defined mode, source-definable epoch-zero modes (`J2000`, `J1900`, `B1950`), fixed-star anchor modes (`Aldebaran 15 Tau`, `True/SS Citra`, `True/SS Revati`, `True Mula`, `True Pushya`), `Galactic Center 0 Sagittarius`, Krishnamurti/Newcomb, Raman, and Yukteshwar.
  The remaining declared traditional modes still require independent source-backed epoch/offset definitions before numeric support can be claimed. The naming row is closed for all declared JME sidereal constants.
- House rows:
  closed for the JME-native supported-system contract with finite cusp/angle output, speed output, ARMC behavior, Sunshine declination handling, position-to-house behavior, and invalid-output checks.
- Gauquelin:
  closed for the JME-native method contract with methods 0-3, finite sector range, fractional/refraction-sensitive event methods, unsupported-method rejection, and invalid-output checks.
- Rise/set/transit:
  implemented and root-refined, but still marked partial until broader reference-method validation is complete.
- Longitude/node/crossing rows:
  refined searches are now closed for JME-native behavior: solar longitude, lunar longitude, lunar node, and heliocentric longitude crossings have target-normalization, root-refinement, UT-alias, and invalid-output coverage.
- Solar eclipse family:
  `reference_sol_eclipse_where`, `reference_sol_eclipse_how`, `reference_sol_eclipse_when_loc`, and `reference_sol_eclipse_when_glob` are now closed for the current JME-native contract.
  The covered contract includes total, annular, hybrid, and partial global known-value regressions, local visible circumstances, backward search, ordered outer contacts, non-eclipse rejection, null-output/geopos rejection, centrality classification, and partial-only magnitude from the returned shadow-surface location.
  The API does not overclaim second/third global or local contacts where the current search returns zero.
- `reference_lun_eclipse_when_loc`
  is closed for the current JME-native local visibility contract: it intersects the closed global lunar-eclipse phase window with local Moon visibility, clips contacts to the visible interval, supports forward/backward search, rejects hidden observer cases, and validates null-output/geopos paths.
  It intentionally does not claim a separate topocentric lunar-shadow model beyond local visibility clipping of the global shadow contacts.
- `reference_pheno` / `reference_pheno_ut`
  closed for the JME-native physical-phenomena field contract: phase angle, illuminated fraction, elongation, apparent diameter, magnitude, distance, light-time, apparent-radius, phase-defect, and bright-limb fields are finite and tested for ET and UT entry points.
- `reference_nod_aps` / `reference_nod_aps_ut`
  closed for JME-native behavior with lunar node/apside and non-lunar osculating node/perihelion/aphelion coverage, including invalid-input checks.
- `reference_get_orbital_elements` and `reference_orbit_max_min_true_distance`
  closed for JME-native behavior with osculating element invariants, derived distance fields, Mercury distance-range checks, and invalid-input checks.
- Analytical engines and fallbacks:
  VSOP87, Moshier, Meeus, and mixed fallback paths are materially integrated, direct contract tests now enforce supported-body success / unsupported-body rejection for the Moshier and Meeus public wrappers, and the public `jme_calc` path has a regression that exercises an analytic fallback Pluto calculation, but broader independent known-value validation is still incomplete.
  The analytical validation suite also checks that representative Moshier, VSOP87, and Meeus bodies produce direct public state output at J2000.
  The public fallback regression also checks rectangular and distance-unit flag behavior for Pluto as a non-JPL path.
  The public calculation contract also checks velocity-per-second conversion for a representative analytic body.
  The same contract block checks `jme_calc_pctr()` in a build-aware way: CALCEPH builds verify the Mercury/Sun difference contract, while non-CALCEPH builds verify a clean explicit error instead of silent success.
  The same `jme_calc_pctr()` check also validates distance-unit scaling against the direct heliocentric Mercury vector norm.
- ELP2000:
  the Moon path is present in the project surface and now has multiple external modern known-value checks. Dense date-range precision certification is tracked as model-certification work separate from function closure.

### Additional Exact-Parity Notes

These notes distinguish the current JME-native closure contract from black-box behavior of other libraries.

- Heliacal family:
  `reference_heliacal_ut`, `reference_heliacal_pheno_ut`, `reference_vis_limit_mag`, `reference_heliacal_angle`, and `reference_topo_arcus_visionis` are closed for the JME-native heliacal contract.
  The native contract uses local Sun/body altitude, topocentric arcus visionis, apparent magnitude, elongation, apparent diameter, `required_arcus = 10.50 + 1.40 * visual_magnitude`, and `limiting_magnitude = (arcus - 10.50) / 1.40`.
  Tests sweep Moon and planetary body IDs, revalidate returned events, compare scalar helpers against array outputs, and reject null/unsupported inputs.
  Full reference black-box visual-observer parity is still not claimed.
- Lunar occultation family:
  `reference_lun_occult_where`, `reference_lun_occult_when_loc`, and `reference_lun_occult_when_glob` are now closed for the current JME-native Mars occultation contract.
  The covered contract includes global maximum search, geographic circumstances, local Los Angeles visibility for the 2022-12-08 Mars occultation, positive magnitude/diameter/parallax fields, ordered outer contacts, and null required-output/geopos rejection.
  Broader star/planet catalog validation can still be added later, but the mapped behavior rows are no longer open under the current JME-native contract.

### Constant Compatibility Notes

The constant story needs to be read more carefully than a simple count.

- The reference inventory of `348` constants is tracked in documentation.
- That inventory count means the symbolic surface has been audited and mapped. It does not by itself prove `348/348` exact drop-in reference semantic parity.
- Some constant families are structurally strong and materially usable now:
  date/calendar constants, angle-format constants, many house-system constants, many rise/set constants, and much of the model-selection surface.
- Some constant families still need stricter semantic auditing against reference behavior rather than name-only presence checks.
- The most important currently known semantic mismatch is the eclipse constant family.
  `JME_ECLIPlegacy constants` names exist, but in the current JME surface they are sequential enum-style values, not legacy-style combinable bit flags.
  That means constant-name coverage exists, but exact legacy-style flag semantics do not yet hold there.
- Because of that, the correct wording today is:
  `348 constants tracked`
  not
  `348 constants proven drop-in equivalent`

### Resume Guidance

If work resumes later and the goal is exact reference reality rather than symbolic coverage, the priority order should be:

- harden and externally validate lunar occultation algorithms
- keep heliacal visibility within its documented JME-native arcus-visionis contract unless a fuller atmosphere/observer model is explicitly added
- tighten eclipse locality/contact validation and local lunar-contact solving
- close physical-phenomena gaps
- expand independent known-value validation across analytical engines and fallback paths

## Behavioral Status

The 348 reference constants are tracked as inventory. That does not mean all 348 already have drop-in reference-compatible semantics or exact value-model parity in every call path.

The 106 function behavior rows are all mapped to `jme_*` APIs and closed for the JME-native behavior contract tracked by this repository.

Important reality check:

- `Covered` in the matrix below means a reference row is mapped to one or more `jme_*` APIs.
- `Covered` does not mean exact behavioral parity.
- `Tracked` for constants means the symbolic surface is inventoried; it does not by itself prove exact reference-compatible semantics.
- Some constant families are structurally compatible and safe as named controls, while some event/effect constants still differ in semantic model from reference expectations.

Currently complete areas are mostly metadata, date/time conversion, angle/string utilities, coordinate transforms, basic refraction, state-vector helpers, and the current raw JPL/CALCEPH boundary functions when CALCEPH plus the required capability-specific kernels/files are present.

Currently partial areas include full numeric ayanamsa model breadth and remaining broad external validation for VSOP87, Moshier, and Meeus fallbacks.

Currently open or effectively first-pass areas include full wide-date ELP2000 lunar precision certification and broader independent validation of exact topocentric eclipse/occultation locality precision.

## Reference Constant Inventory

The reference constant inventory is broader than a simple "present or missing" check.

- Many utility, house-system, rise/set, model-selection, date/calendar, and angle-format constants are present as real `JME_*` controls and are materially usable.
- Some families are only tracked at the inventory/mapping level and still need stricter semantic verification against reference behavior.
- Eclipse-related constants are a known semantic mismatch area: names are present, but `JME_ECLIPlegacy constants` values are sequential enum-style values, not legacy-style combinable bit flags.
- Because of that, constant-name coverage alone is not treated as full drop-in parity for all 348 reference constants.

## Coverage Matrix

| # | Reference behavior row | Mapping status | reference -> JME map | JME coverage |
|---:|---|---|---|---|
| 1 | `reference_heliacal_ut` | Covered | `jme_heliacal_ut` | Closed for JME-native behavior: local twilight event search uses required-arcus solar-depression candidates over the next 370 days, returns a revalidated visible event, and rejects null/unsupported inputs. |
| 2 | `reference_heliacal_pheno_ut` | Covered | `jme_heliacal_pheno_ut` | Closed for JME-native behavior: Moon and planet bodies are covered; Sun/body altitude, arcus, limiting magnitude, apparent magnitude, elongation, apparent diameter, required arcus, visibility flag, and invalid-input behavior are covered. |
| 3 | `reference_vis_limit_mag` | Covered | `jme_vis_limit_mag` | Closed for JME-native behavior: limiting magnitude is `(arcus - 10.50) / 1.40`, matches phenomenon output, and null output rejects. |
| 4 | `reference_heliacal_angle` | Covered | `jme_heliacal_angle` | Closed for JME-native behavior: returns the same Sun-body elongation as the phenomenon calculation, with default Venus support and invalid-input `NaN`. |
| 5 | `reference_topo_arcus_visionis` | Covered | `jme_topo_arcus_visionis` | Closed for JME-native behavior: returns the same topocentric arcus visionis as the phenomenon calculation, with default Venus support and invalid-input `NaN`. |
| 6 | `reference_set_astro_models` | Covered | `jme_set_astro_models` | Closed for JME-native behavior: parses profile/tokens into canonical bias, nutation, obliquity, precession, sidereal-time, and Delta-T model state; model-state effects are covered by direct output checks. |
| 7 | `reference_get_astro_models` | Covered | `jme_get_astro_models` | Closed for JME-native behavior: returns canonical model-state summary and rejects null output. |
| 8 | `reference_version` | Covered | `jme_version` | Version string export. |
| 9 | `reference_get_library_path` | Covered | `jme_library_path` | Library path export. |
| 10 | `reference_calc` | Covered | `jme_calc` | Closed for JME-native behavior: null-output rejection, unsupported-body rejection, finite Sun/Moon/node/planet output, analytic fallback path, rectangular output, distance scaling, velocity-per-second scaling, radians, and sidereal longitude behavior are covered. |
| 11 | `reference_calc_ut` | Covered | `jme_calc_ut` | Closed for JME-native behavior: UT-to-ET wrapper behavior is covered through the same public calculation contract, including finite body outputs and flag behavior. |
| 12 | `reference_calc_pctr` | Covered | `jme_calc_pctr` | Closed for JME-native behavior: body-center rectangular difference, spherical conversion, distance scaling, radians, sidereal longitude, null-output rejection, and unsupported body/center rejection are covered. |
| 13 | `reference_solcross` | Covered | `jme_solcross` | Closed for JME-native behavior: solar longitude crossing normalizes target longitude, rejects null output, and returns a refined crossing verified against `jme_calc_ut`. |
| 14 | `reference_solcross_ut` | Covered | `jme_solcross_ut` | Closed for JME-native behavior: UT alias behavior is verified against the normalized solar crossing result. |
| 15 | `reference_mooncross` | Covered | `jme_mooncross` | Closed for JME-native behavior: lunar longitude crossing normalizes target longitude, rejects null output, and returns a refined crossing verified against `jme_calc_ut`. |
| 16 | `reference_mooncross_ut` | Covered | `jme_mooncross_ut` | Closed for JME-native behavior: UT alias behavior is verified against the normalized lunar crossing result. |
| 17 | `reference_mooncross_node` | Covered | `jme_mooncross_node` | Closed for JME-native behavior: lunar latitude zero-crossing refinement and null-output rejection are verified. |
| 18 | `reference_mooncross_node_ut` | Covered | `jme_mooncross_node_ut` | Closed for JME-native behavior: UT alias behavior is verified against the lunar node crossing result. |
| 19 | `reference_helio_cross` | Covered | `jme_helio_cross` | Closed for JME-native behavior: heliocentric longitude crossing normalizes target longitude, rejects null output, and returns a refined crossing verified against `jme_calc_ut`. |
| 20 | `reference_helio_cross_ut` | Covered | `jme_helio_cross_ut` | Closed for JME-native behavior: UT alias behavior is verified against the normalized heliocentric crossing result. |
| 21 | `reference_fixstar` | Covered | `jme_fixstar` | Closed for JME-native behavior: 9,096-entry Bright Star/Yale-derived catalog, common-name aliases, HR/HD/SAO/catalog-name lookup, J2000 true-position checks, proper-motion speed output, radians/sidereal behavior, and invalid-input rejection are covered. |
| 22 | `reference_fixstar_ut` | Covered | `jme_fixstar_ut` | Closed for JME-native behavior: UT wrapper behavior is covered for the same catalog and finite-output contract. |
| 23 | `reference_fixstar_mag` | Covered | `jme_fixstar_mag` | Closed for JME-native behavior: magnitude lookup is covered for common-name and broad-catalog entries, with null and unknown-star rejection. |
| 24 | `reference_fixstar2` | Covered | `jme_fixstar2` | Closed for JME-native behavior: alternate ET entry point delegates to the primary fixed-star contract and is directly covered. |
| 25 | `reference_fixstar2_ut` | Covered | `jme_fixstar2_ut` | Closed for JME-native behavior: alternate UT entry point delegates to the UT fixed-star contract and is directly covered. |
| 26 | `reference_fixstar2_mag` | Covered | `jme_fixstar2_mag` | Closed for JME-native behavior: alternate magnitude entry point delegates to the magnitude contract and is directly covered. |
| 27 | `reference_close` | Covered | `jme_close`, `jme_jpl_close` | Runtime and JPL resource close. |
| 28 | `reference_set_ephe_path` | Covered | `jme_set_ephemeris_path`, `jme_ephemeris_path` | Ephemeris path state. |
| 29 | `reference_set_jpl_file` | Covered | `jme_set_jpl_file`, `jme_jpl_file` | JPL file state. |
| 30 | `reference_get_planet_name` | Covered | `jme_body_name`, `jme_copy_body_name` | Body name lookup and copy. |
| 31 | `reference_set_topo` | Covered | `jme_set_topo` | Topocentric observer state remains a dedicated domain. |
| 32 | `reference_set_sid_mode` | Covered | `jme_set_sidereal_mode`, `jme_get_sidereal_mode` | Sidereal mode state. |
| 33 | `reference_get_ayanamsa_ex` | Covered | `jme_get_ayanamsa_ex` | Closed for JME-native behavior: every declared sidereal model is numeric and contract-tested; null-output and unknown-model rejection are covered. |
| 34 | `reference_get_ayanamsa_ex_ut` | Covered | `jme_get_ayanamsa_ex_ut` | Closed for JME-native behavior: UT conversion delegates to the closed ayanamsa model contract. |
| 35 | `reference_get_ayanamsa` | Covered | `jme_get_ayanamsa` | Closed for JME-native behavior: current sidereal mode delegates to the closed ayanamsa model contract. |
| 36 | `reference_get_ayanamsa_ut` | Covered | `jme_get_ayanamsa_ut` | Closed for JME-native behavior: current sidereal mode plus UT conversion delegates to the closed ayanamsa model contract. |
| 37 | `reference_get_ayanamsa_name` | Covered | `jme_get_ayanamsa_name` | Closed for JME-native behavior: every declared JME sidereal constant has a deterministic public name, and unknown model IDs return a deterministic unknown-name result. |
| 38 | `reference_get_current_file_data` | Covered | `jme_jpl_current_file_data` | Closed for JME-native behavior: CALCEPH-backed `de440s.bsp` success path returns non-empty path and valid coverage span; closed-kernel/CALCEPH-unavailable error paths reset outputs; null metadata output rejection is covered. |
| 39 | `reference_date_conversion` | Covered | `jme_date_is_valid`, `jme_julian_day`, `jme_reverse_julian_day` | Calendar validation and Julian conversion. |
| 40 | `reference_julday` | Covered | `jme_julian_day` | Julian day conversion. |
| 41 | `reference_revjul` | Covered | `jme_reverse_julian_day` | Reverse Julian day conversion. |
| 42 | `reference_utc_to_jd` | Covered | `jme_utc_to_jd` | UTC to Julian day conversion. |
| 43 | `reference_jdet_to_utc` | Covered | `jme_jd_to_utc` | ET Julian day to UTC fields. |
| 44 | `reference_jdut1_to_utc` | Covered | `jme_jd_to_utc` | UT1 Julian day to UTC fields. |
| 45 | `reference_utc_time_zone` | Covered | `jme_utc_time_zone` | Time-zone conversion. |
| 46 | `reference_houses` | Covered | `jme_houses` | Closed for JME-native behavior: supported systems, finite cusp/angle output, angle relationships, invalid system rejection, and null-cusp rejection are covered. |
| 47 | `reference_houses_ex` | Covered | `jme_houses_ex` | Closed for JME-native behavior: same supported-system contract as `jme_houses`, including null-cusp rejection. |
| 48 | `reference_houses_ex2` | Covered | `jme_houses_ex2` | Closed for JME-native behavior: finite cusp/angle speed output and null-cusp rejection are covered. |
| 49 | `reference_houses_armc` | Covered | `jme_houses_armc` | Closed for JME-native behavior: ARMC computation, Sunshine declination contract, finite output, and null-cusp rejection are covered. |
| 50 | `reference_houses_armc_ex2` | Covered | `jme_houses_armc_ex2` | Closed for JME-native behavior: finite ARMC speed output and null-cusp rejection are covered. |
| 51 | `reference_house_pos` | Covered | `jme_house_pos` | Closed for JME-native behavior: finite position-to-house behavior and null-input rejection are covered. |
| 52 | `reference_house_name` | Covered | `jme_house_system_name` | House system naming. |
| 53 | `reference_gauquelin_sector` | Covered | `jme_gauquelin_sector` | Closed for JME-native behavior: methods 0-3, finite sector range, fractional/refraction-sensitive event methods, unsupported-method rejection, and null-output/geopos rejection are covered. |
| 54 | `reference_sol_eclipse_where` | Covered | `jme_sol_eclipse_where` | Closed for JME-native behavior: total, annular, hybrid, and partial global circumstances have known-value coverage, finite positive magnitude/diameter fields, centrality classification, non-eclipse rejection, null-output/geopos rejection, and partial-only magnitude from the returned shadow-surface location. |
| 55 | `reference_lun_occult_where` | Covered | `jme_lun_occult_where` | Closed for JME-native behavior: Mars occultation geographic circumstances return finite location, positive magnitude/diameter/parallax fields, and null-output/geopos rejection. |
| 56 | `reference_sol_eclipse_how` | Covered | `jme_sol_eclipse_how` | Closed for JME-native behavior: visible local eclipse circumstances return finite magnitude, apparent diameters, altitude, non-eclipse rejection, and null-output/geopos rejection. |
| 57 | `reference_sol_eclipse_when_loc` | Covered | `jme_sol_eclipse_when_loc` | Closed for JME-native behavior: representative total-path and annular-path local searches return finite positive circumstances, ordered outer contacts, and null-output/geopos rejection. |
| 58 | `reference_lun_occult_when_loc` | Covered | `jme_lun_occult_when_loc` | Closed for JME-native behavior: the 2022-12-08 Mars occultation local Los Angeles search returns visible positive circumstances, ordered outer contacts, and null-output/geopos rejection. |
| 59 | `reference_sol_eclipse_when_glob` | Covered | `jme_sol_eclipse_when_glob` | Closed for JME-native behavior: total, annular, hybrid, and partial global search has known-value maxima, backward search, ordered outer contacts for non-partial eclipses, no inner-contact overclaim, and null-output rejection. |
| 60 | `reference_lun_occult_when_glob` | Covered | `jme_lun_occult_when_glob` | Closed for JME-native behavior: the 2022-12-08 Mars occultation global search returns finite maximum, ordered outer contacts, and null-output rejection. |
| 61 | `reference_lun_eclipse_how` | Covered | `jme_lun_eclipse_how` | Closed for JME-native global behavior: total, partial, and penumbral circumstances are covered with external known-value maxima/magnitudes, non-eclipse rejection, finite shadow fields, local visibility flag behavior, and null-output rejection. |
| 62 | `reference_lun_eclipse_when` | Covered | `jme_lun_eclipse_when` | Closed for JME-native global behavior: total, partial, and penumbral maximum/contact search is covered with external known-value maxima, backward search, contact fields, and null-output rejection. |
| 63 | `reference_lun_eclipse_when_loc` | Covered | `jme_lun_eclipse_when_loc` | Closed for JME-native behavior: visible and hidden observer cases, forward/backward search, ordered total-eclipse contacts clipped to the local visible Moon interval, visible-duration fields, and null-output/geopos rejection are covered. |
| 64 | `reference_pheno` | Covered | `jme_pheno` | Closed for JME-native behavior: phase angle, illuminated fraction, elongation, apparent diameter, magnitude, distance, light-time, apparent-radius, phase-defect, and bright-limb fields are covered with finite-output and null-output tests. |
| 65 | `reference_pheno_ut` | Covered | `jme_pheno_ut` | Closed for JME-native behavior: UT wrapper behavior is covered for the same finite physical-phenomena field contract and null-output rejection. |
| 66 | `reference_refrac` | Covered | `jme_refract` | Atmospheric refraction. |
| 67 | `reference_refrac_extended` | Covered | `jme_refract_extended` | Extended atmospheric refraction. |
| 68 | `reference_set_lapse_rate` | Covered | `jme_set_lapse_rate` | Refraction lapse-rate state remains a dedicated domain. |
| 69 | `reference_azalt` | Covered | `jme_equatorial_to_horizontal` | Azimuth/altitude conversion. |
| 70 | `reference_azalt_rev` | Covered | `jme_horizontal_to_equatorial` | Reverse azimuth/altitude conversion. |
| 71 | `reference_rise_trans_true_hor` | Covered | `jme_rise_trans_true_hor` | Closed for JME-native behavior: true-horizon rise is root-refined and verified by independently recomputed apparent altitude residuals; null-output/geopos rejection is covered. |
| 72 | `reference_rise_trans` | Covered | `jme_rise_trans` | Closed for JME-native behavior: Sun, Moon, planet, and fixed-star rise/set/transit paths are covered with independent altitude/hour-angle residual checks; twilight, disc-center, no-refraction, unsupported body/star/mode, no-event, and null-output/geopos paths are covered. |
| 73 | `reference_nod_aps` | Covered | `jme_nod_aps` | Closed for JME-native behavior: lunar node/apside and non-lunar osculating node/perihelion/aphelion behavior are covered, including unsupported-body and null-output rejection. |
| 74 | `reference_nod_aps_ut` | Covered | `jme_nod_aps_ut` | Closed for JME-native behavior: UT wrapper behavior is covered with finite normalized output. |
| 75 | `reference_get_orbital_elements` | Covered | `jme_get_orbital_elements` | Closed for JME-native behavior: osculating element invariants, derived longitude/radius fields, unsupported-body rejection, and null-output rejection are covered. |
| 76 | `reference_orbit_max_min_true_distance` | Covered | `jme_orbit_max_min_true_distance` | Closed for JME-native behavior: perihelion/aphelion time-distance output, expected Mercury distance ranges, unsupported-body rejection, and null-output rejection are covered. |
| 77 | `reference_deltat` | Covered | `jme_delta_t` | Delta T model remains a dedicated domain. |
| 78 | `reference_deltat_ex` | Covered | `jme_delta_t_ex` | Delta T model with flags remains a dedicated domain. |
| 79 | `reference_time_equ` | Covered | `jme_time_equ` | Equation of time remains a dedicated domain. |
| 80 | `reference_lmt_to_lat` | Covered | `jme_lmt_to_lat` | Local mean time to local apparent time remains a dedicated domain. |
| 81 | `reference_lat_to_lmt` | Covered | `jme_lat_to_lmt` | Local apparent time to local mean time remains a dedicated domain. |
| 82 | `reference_sidtime0` | Covered | `jme_sidereal_time0` | Closed for JME-native behavior with J2000 known-value validation. |
| 83 | `reference_sidtime` | Covered | `jme_sidereal_time` | Closed for JME-native behavior with default and IAU 2006 known-value validation. |
| 84 | `reference_set_interpolate_nut` | Covered | `jme_set_interpolate_nut` | Nutation interpolation selection remains a dedicated domain. |
| 85 | `reference_cotrans` | Covered | `jme_ecliptic_to_equatorial`, `jme_equatorial_to_ecliptic` | Coordinate conversion. |
| 86 | `reference_cotrans_sp` | Covered | `jme_ecliptic_to_equatorial_rectangular_state`, `jme_equatorial_to_ecliptic_rectangular_state` | State-vector coordinate conversion. |
| 87 | `reference_get_tid_acc` | Covered | `jme_get_tid_acc` | Tidal acceleration state query remains a dedicated domain. |
| 88 | `reference_set_tid_acc` | Covered | `jme_set_tid_acc` | Tidal acceleration state selection remains a dedicated domain. |
| 89 | `reference_set_delta_t_userdef` | Covered | `jme_set_delta_t_userdef` | User-defined Delta T state remains a dedicated domain. |
| 90 | `reference_degnorm` | Covered | `jme_degree_normalize` | Degree normalization. |
| 91 | `reference_radnorm` | Covered | `jme_radian_normalize` | Radian normalization. |
| 92 | `reference_rad_midp` | Covered | `jme_radian_midpoint` | Radian midpoint. |
| 93 | `reference_deg_midp` | Covered | `jme_degree_midpoint` | Degree midpoint. |
| 94 | `reference_split_deg` | Covered | `jme_split_degree` | Degree splitting. |
| 95 | `reference_csnorm` | Covered | `jme_centiseconds_normalize` | Centisecond normalization. |
| 96 | `reference_difcsn` | Covered | `jme_centiseconds_difference` | Centisecond difference. |
| 97 | `reference_difdegn` | Covered | `jme_degrees_difference` | Degree difference. |
| 98 | `reference_difcs2n` | Covered | `jme_centiseconds_difference_signed` | Signed centisecond difference. |
| 99 | `reference_difdeg2n` | Covered | `jme_degrees_difference_signed` | Signed degree difference. |
| 100 | `reference_difrad2n` | Covered | `jme_radians_difference_signed` | Signed radian difference. |
| 101 | `reference_csroundsec` | Covered | `jme_centiseconds_round_second` | Centisecond rounding to second. |
| 102 | `reference_d2l` | Covered | `jme_double_to_long` | Double-to-integer conversion. |
| 103 | `reference_day_of_week` | Covered | `jme_day_of_week` | Day-of-week conversion. |
| 104 | `reference_cs2timestr` | Covered | `jme_centiseconds_to_time_string` | Centisecond time string formatting remains a dedicated domain. |
| 105 | `reference_cs2lonlatstr` | Covered | `jme_centiseconds_to_lonlat_string` | Centisecond longitude/latitude string formatting remains a dedicated domain. |
| 106 | `reference_cs2degstr` | Covered | `jme_centiseconds_to_degree_string` | Centisecond degree string formatting remains a dedicated domain. |

---

## external reference engine Capability Comparison

This document tracks reference coverage against the project-owned JME API. Symbol existence is verified separately from full behavior completion.

### Current Inventory Status

| Metric | external reference engine (Reference) | **JPL Moshier Ephemeris (Current)** | **Parity %** |
| :--- | :--- | :--- | :--- |
| **Reference Function Rows Mapped** | 106 | **106** | **100% mapping, not 100% behavior** |
| **Total Functions (API Surface)** | - | **204** | 112 unique mapped `jme_*` functions + 92 extra `jme_*` functions |
| **Reference Constants** | 348 | **348** | **100% mapped, not 100% semantic parity** |
| **Total Constants (API Surface)** | - | **462** | (114 extra constants) |

---

### 1. Verification Summary

1. **Strict Constant Validation (462/462):**
    - **Test Suite:** `tests/test_all_constants.c`
    - **Result:** 100% Success. Verified existence and spelling for all 462 public `JME_*` constants.
2. **Function Symbol Validation (204/204):**
    - **Test Suite:** `tests/test_all_functions.c`
    - **Result:** 100% Success for declared/defined symbol coverage and smoke-level call coverage. This is not a full reference behavior-parity claim.
3. **Clean-Room Compliance:**
    - **Terminology:** 0 forbidden markers remain (no "to-do", "fix-me", or "fixed").
    - **Provenance:** Implemented logic remains traceable to independent permitted sources before it is marked behavior-complete.
    - **Contract:** Current tests enforce symbol coverage and reject false-success paths such as `JME_OK` with non-finite calculation output. This is not a full behavior-parity claim.

---

### 2. Full 106 Reference Function Mapping

All 106 reference behavior rows are mapped to native `jme_` equivalents. In the table below, `Verified` means the mapping/symbol is verified, not that reference behavior is complete. Some mapped rows are still partial or open at the algorithm level; the behavioral source of truth is `docs/VALIDATION_AND_COVERAGE.md`.

| # | Reference Function | JME Native Equivalent | Status |
|---|---|---|---|
| 1 | `reference_heliacal_ut` | `jme_heliacal_ut` | **Verified** |
| 2 | `reference_heliacal_pheno_ut` | `jme_heliacal_pheno_ut` | **Verified** |
| 3 | `reference_vis_limit_mag` | `jme_vis_limit_mag` | **Verified** |
| 4 | `reference_heliacal_angle` | `jme_heliacal_angle` | **Verified** |
| 5 | `reference_topo_arcus_visionis` | `jme_topo_arcus_visionis` | **Verified** |
| 6 | `reference_set_astro_models` | `jme_set_astro_models` | **Verified** |
| 7 | `reference_get_astro_models` | `jme_get_astro_models` | **Verified** |
| 8 | `reference_version` | `jme_version` | **Verified** |
| 9 | `reference_get_library_path` | `jme_library_path` | **Verified** |
| 10 | `reference_calc` | `jme_calc` | **Verified** |
| 11 | `reference_calc_ut` | `jme_calc_ut` | **Verified** |
| 12 | `reference_calc_pctr` | `jme_calc_pctr` | **Verified** |
| 13 | `reference_solcross` | `jme_solcross` | **Verified; refined root search** |
| 14 | `reference_solcross_ut` | `jme_solcross_ut` | **Verified; refined root search** |
| 15 | `reference_mooncross` | `jme_mooncross` | **Verified; refined root search** |
| 16 | `reference_mooncross_ut` | `jme_mooncross_ut` | **Verified; refined root search** |
| 17 | `reference_mooncross_node` | `jme_mooncross_node` | **Verified; refined root search** |
| 18 | `reference_mooncross_node_ut` | `jme_mooncross_node_ut` | **Verified; refined root search** |
| 19 | `reference_helio_cross` | `jme_helio_cross` | **Verified; refined root search** |
| 20 | `reference_helio_cross_ut` | `jme_helio_cross_ut` | **Verified; refined root search** |
| 21 | `reference_fixstar` | `jme_fixstar` | **Verified** |
| 22 | `reference_fixstar_ut` | `jme_fixstar_ut` | **Verified** |
| 23 | `reference_fixstar_mag` | `jme_fixstar_mag` | **Verified** |
| 24 | `reference_fixstar2` | `jme_fixstar2` | **Verified** |
| 25 | `reference_fixstar2_ut` | `jme_fixstar2_ut` | **Verified** |
| 26 | `reference_fixstar2_mag` | `jme_fixstar2_mag` | **Verified** |
| 27 | `reference_close` | `jme_close`, `jme_jpl_close` | **Verified** |
| 28 | `reference_set_ephe_path` | `jme_set_ephemeris_path` | **Verified** |
| 29 | `reference_set_jpl_file` | `jme_set_jpl_file` | **Verified** |
| 30 | `reference_get_planet_name` | `jme_body_name` | **Verified** |
| 31 | `reference_set_topo` | `jme_set_topo` | **Verified** |
| 32 | `reference_set_sid_mode` | `jme_set_sidereal_mode` | **Verified** |
| 33 | `reference_get_ayanamsa_ex` | `jme_get_ayanamsa_ex` | **Verified** |
| 34 | `reference_get_ayanamsa_ex_ut` | `jme_get_ayanamsa_ex_ut` | **Verified** |
| 35 | `reference_get_ayanamsa` | `jme_get_ayanamsa` | **Verified** |
| 36 | `reference_get_ayanamsa_ut` | `jme_get_ayanamsa_ut` | **Verified** |
| 37 | `reference_get_ayanamsa_name` | `jme_get_ayanamsa_name` | **Verified** |
| 38 | `reference_get_current_file_data` | `jme_jpl_current_file_data` | **Verified** |
| 39 | `reference_date_conversion` | `jme_date_is_valid`, `jme_julian_day` | **Verified** |
| 40 | `reference_julday` | `jme_julian_day` | **Verified** |
| 41 | `reference_revjul` | `jme_reverse_julian_day` | **Verified** |
| 42 | `reference_utc_to_jd` | `jme_utc_to_jd` | **Verified** |
| 43 | `reference_jdet_to_utc` | `jme_jd_to_utc` | **Verified** |
| 44 | `reference_jdut1_to_utc` | `jme_jd_to_utc` | **Verified** |
| 45 | `reference_utc_time_zone` | `jme_utc_time_zone` | **Verified** |
| 46 | `reference_houses` | `jme_houses` | **Mapped; Koch/Krusinski/APC/Sunshine and exact systems implemented** |
| 47 | `reference_houses_ex` | `jme_houses_ex` | **Mapped; Koch/Krusinski/APC/Sunshine and exact systems implemented** |
| 48 | `reference_houses_ex2` | `jme_houses_ex2` | **Mapped; finite speeds implemented** |
| 49 | `reference_houses_armc` | `jme_houses_armc` | **Mapped; Koch/Krusinski/APC/Sunshine and exact systems implemented** |
| 50 | `reference_houses_armc_ex2` | `jme_houses_armc_ex2` | **Mapped; finite speeds implemented** |
| 51 | `reference_house_pos` | `jme_house_pos` | **Verified** |
| 52 | `reference_house_name` | `jme_house_system_name` | **Verified** |
| 53 | `reference_gauquelin_sector` | `jme_gauquelin_sector` | **Mapped; methods 0/1 semiarc and 2/3 rise/set implemented; certification pending** |
| 54 | `reference_sol_eclipse_where` | `jme_sol_eclipse_where` | **Mapped; native geometry implemented; exact locality certification pending** |
| 55 | `reference_lun_occult_where` | `jme_lun_occult_where` | **Mapped; first-pass native geometry implemented; certification pending** |
| 56 | `reference_sol_eclipse_how` | `jme_sol_eclipse_how` | **Mapped; native local circumstances implemented; exact-contact certification pending** |
| 57 | `reference_sol_eclipse_when_loc` | `jme_sol_eclipse_when_loc` | **Mapped; native local search implemented; contact validation pending** |
| 58 | `reference_lun_occult_when_loc` | `jme_lun_occult_when_loc` | **Mapped; first-pass local topocentric search implemented; certification pending** |
| 59 | `reference_sol_eclipse_when_glob` | `jme_sol_eclipse_when_glob` | **Mapped; native global search and classification implemented; broader validation pending** |
| 60 | `reference_lun_occult_when_glob` | `jme_lun_occult_when_glob` | **Mapped; first-pass global search implemented; certification pending** |
| 61 | `reference_lun_eclipse_how` | `jme_lun_eclipse_how` | **Mapped; native lunar circumstances implemented; broader validation pending** |
| 62 | `reference_lun_eclipse_when` | `jme_lun_eclipse_when` | **Mapped; native lunar search implemented; broader validation pending** |
| 63 | `reference_lun_eclipse_when_loc` | `jme_lun_eclipse_when_loc` | **Mapped; local visibility/contact clipping implemented; certification pending** |
| 64 | `reference_pheno` | `jme_pheno` | **Verified; standard fields plus extended geometry exported** |
| 65 | `reference_pheno_ut` | `jme_pheno_ut` | **Verified; standard fields plus extended geometry exported** |
| 66 | `reference_refrac` | `jme_refract` | **Verified** |
| 67 | `reference_refrac_extended` | `jme_refract_extended` | **Verified** |
| 68 | `reference_set_lapse_rate` | `jme_set_lapse_rate` | **Verified** |
| 69 | `reference_azalt` | `jme_equatorial_to_horizontal` | **Verified** |
| 70 | `reference_azalt_rev` | `jme_horizontal_to_equatorial` | **Verified** |
| 71 | `reference_rise_trans_true_hor` | `jme_rise_trans_true_hor` | **Verified; true horizon honored** |
| 72 | `reference_rise_trans` | `jme_rise_trans` | **Verified; rise/set/transit/twilight flags tested** |
| 73 | `reference_nod_aps` | `jme_nod_aps` | **Mapped; lunar node/apside subset implemented** |
| 74 | `reference_nod_aps_ut` | `jme_nod_aps_ut` | **Mapped; lunar node/apside subset implemented** |
| 75 | `reference_get_orbital_elements` | `jme_get_orbital_elements` | **Verified** |
| 76 | `reference_orbit_max_min_true_distance` | `jme_orbit_max_min_true_distance` | **Verified** |
| 77 | `reference_deltat` | `jme_delta_t` | **Verified** |
| 78 | `reference_deltat_ex` | `jme_delta_t_ex` | **Verified** |
| 79 | `reference_time_equ` | `jme_time_equ` | **Verified** |
| 80 | `reference_lmt_to_lat` | `jme_lmt_to_lat` | **Verified** |
| 81 | `reference_lat_to_lmt` | `jme_lat_to_lmt` | **Verified** |
| 82 | `reference_sidtime0` | `jme_sidereal_time0` | **Verified** |
| 83 | `reference_sidtime` | `jme_sidereal_time` | **Verified** |
| 84 | `reference_set_interpolate_nut` | `jme_set_interpolate_nut` | **Verified** |
| 85 | `reference_cotrans` | `jme_ecliptic_to_equatorial` | **Verified** |
| 86 | `reference_cotrans_sp` | `jme_ecliptic_to_equatorial_rectangular_state` | **Verified** |
| 87 | `reference_get_tid_acc` | `jme_get_tid_acc` | **Verified** |
| 88 | `reference_set_tid_acc` | `jme_set_tid_acc` | **Verified** |
| 89 | `reference_set_delta_t_userdef` | `jme_set_delta_t_userdef` | **Verified** |
| 90 | `reference_degnorm` | `jme_degree_normalize` | **Verified** |
| 91 | `reference_radnorm` | `jme_radian_normalize` | **Verified** |
| 92 | `reference_rad_midp` | `jme_radian_midpoint` | **Verified** |
| 93 | `reference_deg_midp` | `jme_degree_midpoint` | **Verified** |
| 94 | `reference_split_deg` | `jme_split_degree` | **Verified** |
| 95 | `reference_csnorm` | `jme_centiseconds_normalize` | **Verified** |
| 96 | `reference_difcsn` | `jme_centiseconds_difference` | **Verified** |
| 97 | `reference_difdegn` | `jme_degrees_difference` | **Verified** |
| 98 | `reference_difcs2n` | `jme_centiseconds_difference_signed` | **Verified** |
| 99 | `reference_difdeg2n` | `jme_degrees_difference_signed` | **Verified** |
| 100 | `reference_difrad2n` | `jme_radians_difference_signed` | **Verified** |
| 101 | `reference_csroundsec` | `jme_centiseconds_round_second` | **Verified** |
| 102 | `reference_d2l` | `jme_double_to_long` | **Verified** |
| 103 | `reference_day_of_week` | `jme_day_of_week` | **Verified** |
| 104 | `reference_cs2timestr` | `jme_centiseconds_to_time_string` | **Verified** |
| 105 | `reference_cs2lonlatstr` | `jme_centiseconds_to_lonlat_string` | **Verified** |
| 106 | `reference_cs2degstr` | `jme_centiseconds_to_degree_string` | **Verified** |

---

### 3. Extended JME-Only Utilities

JME provides **92 distinct public functions outside the 106-row reference behavior mapping**. The arithmetic is `112` unique mapped `jme_*` functions plus `92` extra `jme_*` functions, for `204` total public `jme_*` functions. The `106` value is a behavior-row count, not a unique-function count, so it is not added directly to `92`.

- **Matrix Algebra:** `jme_matrix_multiply`, `jme_matrix_identity`, `jme_matrix_rotate_x/y/z`.
- **Native JPL State Lookups:** `jme_jpl_body_state_native_naif`, `jme_jpl_orientation_state_order_naif`.
- **Coordinate Vector Transforms:** `jme_rectangular_to_spherical_state`, `jme_spherical_to_rectangular_state`.
- **Modern Precession/Nutation:** `jme_get_nutation_matrix`, `jme_get_precession_matrix`.

**The JPL Moshier Ephemeris has full reference-row mapping and constant tracking, but full behavior parity is still in progress.**

---

## Value Verification Matrix

<!-- BEGIN GENERATED VALUE VERIFICATION MATRIX -->
This section tracks how each public `jme_*` function is currently validated.

It separates four different ideas that are often conflated:

- symbol existence
- direct test invocation
- internal contract/identity validation
- independent external numeric validation

Current source of truth for inventory:

- `204` public functions from `include/jme/jme.h` and `include/jme/jme_extended.h`

### Summary

| Verification mode | Count | Meaning |
|---|---:|---|
| `external_web_numeric` | 15 | Compared against authoritative external web/reference values. |
| `embedded_reference_numeric` | 6 | Compared against bundled reference datasets or known-value source material. |
| `independent_formula_or_identity` | 71 | Validated by deterministic mathematical identities, formulae, or round-trips. |
| `kernel_runtime_or_error_contract` | 52 | Validated by CALCEPH runtime success paths or explicit unavailable/error contracts. |
| `api_or_behavior_contract` | 60 | Called directly and contract-tested, but not yet broadly externally value-certified. |

### Interpretation

- `web-source numeric verified` is the strongest current category for public astronomical outputs.
- `contract verified only` does not mean untested; it means the function is exercised and hardened, but not yet certified against an independent external numeric oracle.
- Some functions are not meaningful candidates for web-value checks at all, such as setters, getters, serialization helpers, and unavailable-kernel boundary APIs.

### Per-Function Matrix

| Function | Verification mode | Current status | Note |
|---|---|---|---|
| `jme_body_id_from_name` | `api_or_behavior_contract` | contract verified only | Directly tested, but not yet broadly externally value-certified against an independent web oracle. |
| `jme_body_naif_id` | `api_or_behavior_contract` | contract verified only | Directly tested, but not yet broadly externally value-certified against an independent web oracle. |
| `jme_body_name` | `api_or_behavior_contract` | contract verified only | Directly tested, but not yet broadly externally value-certified against an independent web oracle. |
| `jme_calc` | `external_web_numeric` | web-source numeric verified | Checked against authoritative web/reference values in the current test suite. |
| `jme_calc_pctr` | `api_or_behavior_contract` | contract verified only | Directly tested, but not yet broadly externally value-certified against an independent web oracle. |
| `jme_calc_ut` | `external_web_numeric` | web-source numeric verified | Checked against authoritative web/reference values in the current test suite. |
| `jme_calendar_is_leap_year` | `independent_formula_or_identity` | formula/identity verified | Checked by closed-form formula, round-trip identity, or deterministic mathematical invariant. |
| `jme_centiseconds_difference` | `independent_formula_or_identity` | formula/identity verified | Checked by closed-form formula, round-trip identity, or deterministic mathematical invariant. |
| `jme_centiseconds_difference_signed` | `independent_formula_or_identity` | formula/identity verified | Checked by closed-form formula, round-trip identity, or deterministic mathematical invariant. |
| `jme_centiseconds_normalize` | `independent_formula_or_identity` | formula/identity verified | Checked by closed-form formula, round-trip identity, or deterministic mathematical invariant. |
| `jme_centiseconds_round_second` | `independent_formula_or_identity` | formula/identity verified | Checked by closed-form formula, round-trip identity, or deterministic mathematical invariant. |
| `jme_centiseconds_to_degree_string` | `api_or_behavior_contract` | contract verified only | Directly tested, but not yet broadly externally value-certified against an independent web oracle. |
| `jme_centiseconds_to_lonlat_string` | `api_or_behavior_contract` | contract verified only | Directly tested, but not yet broadly externally value-certified against an independent web oracle. |
| `jme_centiseconds_to_time_string` | `api_or_behavior_contract` | contract verified only | Directly tested, but not yet broadly externally value-certified against an independent web oracle. |
| `jme_close` | `api_or_behavior_contract` | contract verified only | Directly tested, but not yet broadly externally value-certified against an independent web oracle. |
| `jme_copy_body_name` | `api_or_behavior_contract` | contract verified only | Directly tested, but not yet broadly externally value-certified against an independent web oracle. |
| `jme_date_is_valid` | `independent_formula_or_identity` | formula/identity verified | Checked by closed-form formula, round-trip identity, or deterministic mathematical invariant. |
| `jme_day_of_week` | `independent_formula_or_identity` | formula/identity verified | Checked by closed-form formula, round-trip identity, or deterministic mathematical invariant. |
| `jme_day_of_year` | `independent_formula_or_identity` | formula/identity verified | Checked by closed-form formula, round-trip identity, or deterministic mathematical invariant. |
| `jme_days_in_month` | `independent_formula_or_identity` | formula/identity verified | Checked by closed-form formula, round-trip identity, or deterministic mathematical invariant. |
| `jme_decimal_hour` | `independent_formula_or_identity` | formula/identity verified | Checked by closed-form formula, round-trip identity, or deterministic mathematical invariant. |
| `jme_degree_midpoint` | `independent_formula_or_identity` | formula/identity verified | Checked by closed-form formula, round-trip identity, or deterministic mathematical invariant. |
| `jme_degree_normalize` | `independent_formula_or_identity` | formula/identity verified | Checked by closed-form formula, round-trip identity, or deterministic mathematical invariant. |
| `jme_degrees_difference` | `independent_formula_or_identity` | formula/identity verified | Checked by closed-form formula, round-trip identity, or deterministic mathematical invariant. |
| `jme_degrees_difference_signed` | `independent_formula_or_identity` | formula/identity verified | Checked by closed-form formula, round-trip identity, or deterministic mathematical invariant. |
| `jme_degrees_to_hours` | `independent_formula_or_identity` | formula/identity verified | Checked by closed-form formula, round-trip identity, or deterministic mathematical invariant. |
| `jme_degrees_to_radians` | `independent_formula_or_identity` | formula/identity verified | Checked by closed-form formula, round-trip identity, or deterministic mathematical invariant. |
| `jme_delta_t` | `independent_formula_or_identity` | formula/identity verified | Checked by closed-form formula, round-trip identity, or deterministic mathematical invariant. |
| `jme_delta_t_ex` | `independent_formula_or_identity` | formula/identity verified | Checked by closed-form formula, round-trip identity, or deterministic mathematical invariant. |
| `jme_double_to_long` | `independent_formula_or_identity` | formula/identity verified | Checked by closed-form formula, round-trip identity, or deterministic mathematical invariant. |
| `jme_ecliptic_to_equatorial` | `independent_formula_or_identity` | formula/identity verified | Checked by closed-form formula, round-trip identity, or deterministic mathematical invariant. |
| `jme_ecliptic_to_equatorial_rectangular_state` | `independent_formula_or_identity` | formula/identity verified | Checked by closed-form formula, round-trip identity, or deterministic mathematical invariant. |
| `jme_elp2000_moon_state` | `embedded_reference_numeric` | reference numeric verified | Checked against bundled known-value/reference datasets or derived reference cases. |
| `jme_ephemeris_path` | `api_or_behavior_contract` | contract verified only | Directly tested, but not yet broadly externally value-certified against an independent web oracle. |
| `jme_equatorial_to_ecliptic` | `independent_formula_or_identity` | formula/identity verified | Checked by closed-form formula, round-trip identity, or deterministic mathematical invariant. |
| `jme_equatorial_to_ecliptic_rectangular_state` | `independent_formula_or_identity` | formula/identity verified | Checked by closed-form formula, round-trip identity, or deterministic mathematical invariant. |
| `jme_equatorial_to_horizontal` | `independent_formula_or_identity` | formula/identity verified | Checked by closed-form formula, round-trip identity, or deterministic mathematical invariant. |
| `jme_fixstar` | `api_or_behavior_contract` | contract verified only | Directly tested, but not yet broadly externally value-certified against an independent web oracle. |
| `jme_fixstar2` | `api_or_behavior_contract` | contract verified only | Directly tested, but not yet broadly externally value-certified against an independent web oracle. |
| `jme_fixstar2_mag` | `api_or_behavior_contract` | contract verified only | Directly tested, but not yet broadly externally value-certified against an independent web oracle. |
| `jme_fixstar2_ut` | `api_or_behavior_contract` | contract verified only | Directly tested, but not yet broadly externally value-certified against an independent web oracle. |
| `jme_fixstar_mag` | `api_or_behavior_contract` | contract verified only | Directly tested, but not yet broadly externally value-certified against an independent web oracle. |
| `jme_fixstar_ut` | `api_or_behavior_contract` | contract verified only | Directly tested, but not yet broadly externally value-certified against an independent web oracle. |
| `jme_gauquelin_sector` | `api_or_behavior_contract` | contract verified only | Directly tested, but not yet broadly externally value-certified against an independent web oracle. |
| `jme_get_astro_models` | `api_or_behavior_contract` | contract verified only | Directly tested, but not yet broadly externally value-certified against an independent web oracle. |
| `jme_get_ayanamsa` | `independent_formula_or_identity` | formula/identity verified | Checked by closed-form formula, round-trip identity, or deterministic mathematical invariant. |
| `jme_get_ayanamsa_ex` | `independent_formula_or_identity` | formula/identity verified | Checked by closed-form formula, round-trip identity, or deterministic mathematical invariant. |
| `jme_get_ayanamsa_ex_ut` | `independent_formula_or_identity` | formula/identity verified | Checked by closed-form formula, round-trip identity, or deterministic mathematical invariant. |
| `jme_get_ayanamsa_name` | `api_or_behavior_contract` | contract verified only | Directly tested, but not yet broadly externally value-certified against an independent web oracle. |
| `jme_get_ayanamsa_ut` | `independent_formula_or_identity` | formula/identity verified | Checked by closed-form formula, round-trip identity, or deterministic mathematical invariant. |
| `jme_get_frame_bias_matrix` | `api_or_behavior_contract` | contract verified only | Directly tested, but not yet broadly externally value-certified against an independent web oracle. |
| `jme_get_nutation` | `independent_formula_or_identity` | formula/identity verified | Checked by closed-form formula, round-trip identity, or deterministic mathematical invariant. |
| `jme_get_nutation_matrix` | `independent_formula_or_identity` | formula/identity verified | Checked by closed-form formula, round-trip identity, or deterministic mathematical invariant. |
| `jme_get_obliquity` | `independent_formula_or_identity` | formula/identity verified | Checked by closed-form formula, round-trip identity, or deterministic mathematical invariant. |
| `jme_get_orbital_elements` | `external_web_numeric` | web-source numeric verified | Checked against authoritative web/reference values in the current test suite. |
| `jme_get_precession_matrix` | `independent_formula_or_identity` | formula/identity verified | Checked by closed-form formula, round-trip identity, or deterministic mathematical invariant. |
| `jme_get_sidereal_mode` | `api_or_behavior_contract` | contract verified only | Directly tested, but not yet broadly externally value-certified against an independent web oracle. |
| `jme_get_tid_acc` | `api_or_behavior_contract` | contract verified only | Directly tested, but not yet broadly externally value-certified against an independent web oracle. |
| `jme_get_topo_pos` | `independent_formula_or_identity` | formula/identity verified | Checked by closed-form formula, round-trip identity, or deterministic mathematical invariant. |
| `jme_heliacal_angle` | `api_or_behavior_contract` | contract verified only | Directly tested, but not yet broadly externally value-certified against an independent web oracle. |
| `jme_heliacal_pheno_ut` | `api_or_behavior_contract` | contract verified only | Directly tested, but not yet broadly externally value-certified against an independent web oracle. |
| `jme_heliacal_ut` | `api_or_behavior_contract` | contract verified only | Directly tested, but not yet broadly externally value-certified against an independent web oracle. |
| `jme_helio_cross` | `api_or_behavior_contract` | contract verified only | Directly tested, but not yet broadly externally value-certified against an independent web oracle. |
| `jme_helio_cross_ut` | `api_or_behavior_contract` | contract verified only | Directly tested, but not yet broadly externally value-certified against an independent web oracle. |
| `jme_horizontal_to_equatorial` | `independent_formula_or_identity` | formula/identity verified | Checked by closed-form formula, round-trip identity, or deterministic mathematical invariant. |
| `jme_hours_normalize` | `independent_formula_or_identity` | formula/identity verified | Checked by closed-form formula, round-trip identity, or deterministic mathematical invariant. |
| `jme_hours_to_degrees` | `independent_formula_or_identity` | formula/identity verified | Checked by closed-form formula, round-trip identity, or deterministic mathematical invariant. |
| `jme_house_pos` | `api_or_behavior_contract` | contract verified only | Directly tested, but not yet broadly externally value-certified against an independent web oracle. |
| `jme_house_system_name` | `api_or_behavior_contract` | contract verified only | Directly tested, but not yet broadly externally value-certified against an independent web oracle. |
| `jme_houses` | `api_or_behavior_contract` | contract verified only | Directly tested, but not yet broadly externally value-certified against an independent web oracle. |
| `jme_houses_armc` | `api_or_behavior_contract` | contract verified only | Directly tested, but not yet broadly externally value-certified against an independent web oracle. |
| `jme_houses_armc_ex2` | `api_or_behavior_contract` | contract verified only | Directly tested, but not yet broadly externally value-certified against an independent web oracle. |
| `jme_houses_ex` | `api_or_behavior_contract` | contract verified only | Directly tested, but not yet broadly externally value-certified against an independent web oracle. |
| `jme_houses_ex2` | `api_or_behavior_contract` | contract verified only | Directly tested, but not yet broadly externally value-certified against an independent web oracle. |
| `jme_jd_add_seconds` | `independent_formula_or_identity` | formula/identity verified | Checked by closed-form formula, round-trip identity, or deterministic mathematical invariant. |
| `jme_jd_difference_seconds` | `independent_formula_or_identity` | formula/identity verified | Checked by closed-form formula, round-trip identity, or deterministic mathematical invariant. |
| `jme_jd_to_utc` | `independent_formula_or_identity` | formula/identity verified | Checked by closed-form formula, round-trip identity, or deterministic mathematical invariant. |
| `jme_jpl_body_state` | `kernel_runtime_or_error_contract` | runtime/error contract verified | Checked through CALCEPH/runtime capability tests or explicit unavailable/error-path contracts. |
| `jme_jpl_body_state_naif` | `kernel_runtime_or_error_contract` | runtime/error contract verified | Checked through CALCEPH/runtime capability tests or explicit unavailable/error-path contracts. |
| `jme_jpl_body_state_native` | `kernel_runtime_or_error_contract` | runtime/error contract verified | Checked through CALCEPH/runtime capability tests or explicit unavailable/error-path contracts. |
| `jme_jpl_body_state_native_naif` | `kernel_runtime_or_error_contract` | runtime/error contract verified | Checked through CALCEPH/runtime capability tests or explicit unavailable/error-path contracts. |
| `jme_jpl_body_state_native_split` | `kernel_runtime_or_error_contract` | runtime/error contract verified | Checked through CALCEPH/runtime capability tests or explicit unavailable/error-path contracts. |
| `jme_jpl_body_state_native_split_naif` | `kernel_runtime_or_error_contract` | runtime/error contract verified | Checked through CALCEPH/runtime capability tests or explicit unavailable/error-path contracts. |
| `jme_jpl_body_state_order` | `kernel_runtime_or_error_contract` | runtime/error contract verified | Checked through CALCEPH/runtime capability tests or explicit unavailable/error-path contracts. |
| `jme_jpl_body_state_order_naif` | `kernel_runtime_or_error_contract` | runtime/error contract verified | Checked through CALCEPH/runtime capability tests or explicit unavailable/error-path contracts. |
| `jme_jpl_body_state_split` | `kernel_runtime_or_error_contract` | runtime/error contract verified | Checked through CALCEPH/runtime capability tests or explicit unavailable/error-path contracts. |
| `jme_jpl_body_state_split_naif` | `kernel_runtime_or_error_contract` | runtime/error contract verified | Checked through CALCEPH/runtime capability tests or explicit unavailable/error-path contracts. |
| `jme_jpl_body_state_utc` | `kernel_runtime_or_error_contract` | runtime/error contract verified | Checked through CALCEPH/runtime capability tests or explicit unavailable/error-path contracts. |
| `jme_jpl_body_state_utc_naif` | `kernel_runtime_or_error_contract` | runtime/error contract verified | Checked through CALCEPH/runtime capability tests or explicit unavailable/error-path contracts. |
| `jme_jpl_close` | `kernel_runtime_or_error_contract` | runtime/error contract verified | Checked through CALCEPH/runtime capability tests or explicit unavailable/error-path contracts. |
| `jme_jpl_constant` | `kernel_runtime_or_error_contract` | runtime/error contract verified | Checked through CALCEPH/runtime capability tests or explicit unavailable/error-path contracts. |
| `jme_jpl_constant_count` | `kernel_runtime_or_error_contract` | runtime/error contract verified | Checked through CALCEPH/runtime capability tests or explicit unavailable/error-path contracts. |
| `jme_jpl_constant_index` | `kernel_runtime_or_error_contract` | runtime/error contract verified | Checked through CALCEPH/runtime capability tests or explicit unavailable/error-path contracts. |
| `jme_jpl_constant_string` | `kernel_runtime_or_error_contract` | runtime/error contract verified | Checked through CALCEPH/runtime capability tests or explicit unavailable/error-path contracts. |
| `jme_jpl_constant_string_vector` | `kernel_runtime_or_error_contract` | runtime/error contract verified | Checked through CALCEPH/runtime capability tests or explicit unavailable/error-path contracts. |
| `jme_jpl_constant_vector` | `kernel_runtime_or_error_contract` | runtime/error contract verified | Checked through CALCEPH/runtime capability tests or explicit unavailable/error-path contracts. |
| `jme_jpl_coverage` | `kernel_runtime_or_error_contract` | runtime/error contract verified | Checked through CALCEPH/runtime capability tests or explicit unavailable/error-path contracts. |
| `jme_jpl_current_file_data` | `kernel_runtime_or_error_contract` | runtime/error contract verified | Checked through CALCEPH/runtime capability tests or explicit unavailable/error-path contracts. |
| `jme_jpl_ecliptic_state` | `kernel_runtime_or_error_contract` | runtime/error contract verified | Checked through CALCEPH/runtime capability tests or explicit unavailable/error-path contracts. |
| `jme_jpl_ecliptic_state_naif` | `kernel_runtime_or_error_contract` | runtime/error contract verified | Checked through CALCEPH/runtime capability tests or explicit unavailable/error-path contracts. |
| `jme_jpl_ecliptic_state_split` | `kernel_runtime_or_error_contract` | runtime/error contract verified | Checked through CALCEPH/runtime capability tests or explicit unavailable/error-path contracts. |
| `jme_jpl_ecliptic_state_split_naif` | `kernel_runtime_or_error_contract` | runtime/error contract verified | Checked through CALCEPH/runtime capability tests or explicit unavailable/error-path contracts. |
| `jme_jpl_ecliptic_state_utc` | `kernel_runtime_or_error_contract` | runtime/error contract verified | Checked through CALCEPH/runtime capability tests or explicit unavailable/error-path contracts. |
| `jme_jpl_ecliptic_state_utc_naif` | `kernel_runtime_or_error_contract` | runtime/error contract verified | Checked through CALCEPH/runtime capability tests or explicit unavailable/error-path contracts. |
| `jme_jpl_engine_version` | `kernel_runtime_or_error_contract` | runtime/error contract verified | Checked through CALCEPH/runtime capability tests or explicit unavailable/error-path contracts. |
| `jme_jpl_file` | `kernel_runtime_or_error_contract` | runtime/error contract verified | Checked through CALCEPH/runtime capability tests or explicit unavailable/error-path contracts. |
| `jme_jpl_file_version` | `kernel_runtime_or_error_contract` | runtime/error contract verified | Checked through CALCEPH/runtime capability tests or explicit unavailable/error-path contracts. |
| `jme_jpl_id_by_name` | `kernel_runtime_or_error_contract` | runtime/error contract verified | Checked through CALCEPH/runtime capability tests or explicit unavailable/error-path contracts. |
| `jme_jpl_is_available` | `kernel_runtime_or_error_contract` | runtime/error contract verified | Checked through CALCEPH/runtime capability tests or explicit unavailable/error-path contracts. |
| `jme_jpl_is_open` | `kernel_runtime_or_error_contract` | runtime/error contract verified | Checked through CALCEPH/runtime capability tests or explicit unavailable/error-path contracts. |
| `jme_jpl_is_thread_safe` | `kernel_runtime_or_error_contract` | runtime/error contract verified | Checked through CALCEPH/runtime capability tests or explicit unavailable/error-path contracts. |
| `jme_jpl_max_supported_order` | `kernel_runtime_or_error_contract` | runtime/error contract verified | Checked through CALCEPH/runtime capability tests or explicit unavailable/error-path contracts. |
| `jme_jpl_name_by_id` | `kernel_runtime_or_error_contract` | runtime/error contract verified | Checked through CALCEPH/runtime capability tests or explicit unavailable/error-path contracts. |
| `jme_jpl_open` | `kernel_runtime_or_error_contract` | runtime/error contract verified | Checked through CALCEPH/runtime capability tests or explicit unavailable/error-path contracts. |
| `jme_jpl_open_array` | `kernel_runtime_or_error_contract` | runtime/error contract verified | Checked through CALCEPH/runtime capability tests or explicit unavailable/error-path contracts. |
| `jme_jpl_orientation_record_count` | `kernel_runtime_or_error_contract` | runtime/error contract verified | Checked through CALCEPH/runtime capability tests or explicit unavailable/error-path contracts. |
| `jme_jpl_orientation_record_index` | `kernel_runtime_or_error_contract` | runtime/error contract verified | Checked through CALCEPH/runtime capability tests or explicit unavailable/error-path contracts. |
| `jme_jpl_orientation_state_naif` | `kernel_runtime_or_error_contract` | runtime/error contract verified | Checked through CALCEPH/runtime capability tests or explicit unavailable/error-path contracts. |
| `jme_jpl_orientation_state_order_naif` | `kernel_runtime_or_error_contract` | runtime/error contract verified | Checked through CALCEPH/runtime capability tests or explicit unavailable/error-path contracts. |
| `jme_jpl_orientation_state_split_naif` | `kernel_runtime_or_error_contract` | runtime/error contract verified | Checked through CALCEPH/runtime capability tests or explicit unavailable/error-path contracts. |
| `jme_jpl_orientation_state_utc_naif` | `kernel_runtime_or_error_contract` | runtime/error contract verified | Checked through CALCEPH/runtime capability tests or explicit unavailable/error-path contracts. |
| `jme_jpl_position_record_count` | `kernel_runtime_or_error_contract` | runtime/error contract verified | Checked through CALCEPH/runtime capability tests or explicit unavailable/error-path contracts. |
| `jme_jpl_position_record_index` | `kernel_runtime_or_error_contract` | runtime/error contract verified | Checked through CALCEPH/runtime capability tests or explicit unavailable/error-path contracts. |
| `jme_jpl_prefetch` | `kernel_runtime_or_error_contract` | runtime/error contract verified | Checked through CALCEPH/runtime capability tests or explicit unavailable/error-path contracts. |
| `jme_jpl_rotational_angular_momentum_state_naif` | `kernel_runtime_or_error_contract` | runtime/error contract verified | Checked through CALCEPH/runtime capability tests or explicit unavailable/error-path contracts. |
| `jme_jpl_rotational_angular_momentum_state_order_naif` | `kernel_runtime_or_error_contract` | runtime/error contract verified | Checked through CALCEPH/runtime capability tests or explicit unavailable/error-path contracts. |
| `jme_jpl_rotational_angular_momentum_state_split_naif` | `kernel_runtime_or_error_contract` | runtime/error contract verified | Checked through CALCEPH/runtime capability tests or explicit unavailable/error-path contracts. |
| `jme_jpl_rotational_angular_momentum_state_utc_naif` | `kernel_runtime_or_error_contract` | runtime/error contract verified | Checked through CALCEPH/runtime capability tests or explicit unavailable/error-path contracts. |
| `jme_jpl_timescale` | `kernel_runtime_or_error_contract` | runtime/error contract verified | Checked through CALCEPH/runtime capability tests or explicit unavailable/error-path contracts. |
| `jme_julian_day` | `independent_formula_or_identity` | formula/identity verified | Checked by closed-form formula, round-trip identity, or deterministic mathematical invariant. |
| `jme_lat_to_lmt` | `independent_formula_or_identity` | formula/identity verified | Checked by closed-form formula, round-trip identity, or deterministic mathematical invariant. |
| `jme_library_path` | `api_or_behavior_contract` | contract verified only | Directly tested, but not yet broadly externally value-certified against an independent web oracle. |
| `jme_lmt_to_lat` | `independent_formula_or_identity` | formula/identity verified | Checked by closed-form formula, round-trip identity, or deterministic mathematical invariant. |
| `jme_lun_eclipse_how` | `external_web_numeric` | web-source numeric verified | Checked against authoritative web/reference values in the current test suite. |
| `jme_lun_eclipse_when` | `external_web_numeric` | web-source numeric verified | Checked against authoritative web/reference values in the current test suite. |
| `jme_lun_eclipse_when_loc` | `external_web_numeric` | web-source numeric verified | Checked against authoritative web/reference values in the current test suite. |
| `jme_lun_occult_when_glob` | `external_web_numeric` | web-source numeric verified | Checked against authoritative web/reference values in the current test suite. |
| `jme_lun_occult_when_loc` | `external_web_numeric` | web-source numeric verified | Checked against authoritative web/reference values in the current test suite. |
| `jme_lun_occult_where` | `external_web_numeric` | web-source numeric verified | Checked against authoritative web/reference values in the current test suite. |
| `jme_matrix_identity` | `independent_formula_or_identity` | formula/identity verified | Checked by closed-form formula, round-trip identity, or deterministic mathematical invariant. |
| `jme_matrix_multiply` | `independent_formula_or_identity` | formula/identity verified | Checked by closed-form formula, round-trip identity, or deterministic mathematical invariant. |
| `jme_matrix_rotate_x` | `independent_formula_or_identity` | formula/identity verified | Checked by closed-form formula, round-trip identity, or deterministic mathematical invariant. |
| `jme_matrix_rotate_y` | `independent_formula_or_identity` | formula/identity verified | Checked by closed-form formula, round-trip identity, or deterministic mathematical invariant. |
| `jme_matrix_rotate_z` | `independent_formula_or_identity` | formula/identity verified | Checked by closed-form formula, round-trip identity, or deterministic mathematical invariant. |
| `jme_matrix_transform_state` | `independent_formula_or_identity` | formula/identity verified | Checked by closed-form formula, round-trip identity, or deterministic mathematical invariant. |
| `jme_meeus_moon_state` | `embedded_reference_numeric` | reference numeric verified | Checked against bundled known-value/reference datasets or derived reference cases. |
| `jme_meeus_planet_state` | `embedded_reference_numeric` | reference numeric verified | Checked against bundled known-value/reference datasets or derived reference cases. |
| `jme_meeus_sun_state` | `embedded_reference_numeric` | reference numeric verified | Checked against bundled known-value/reference datasets or derived reference cases. |
| `jme_mooncross` | `api_or_behavior_contract` | contract verified only | Directly tested, but not yet broadly externally value-certified against an independent web oracle. |
| `jme_mooncross_node` | `api_or_behavior_contract` | contract verified only | Directly tested, but not yet broadly externally value-certified against an independent web oracle. |
| `jme_mooncross_node_ut` | `api_or_behavior_contract` | contract verified only | Directly tested, but not yet broadly externally value-certified against an independent web oracle. |
| `jme_mooncross_ut` | `api_or_behavior_contract` | contract verified only | Directly tested, but not yet broadly externally value-certified against an independent web oracle. |
| `jme_moshier_planet_state` | `embedded_reference_numeric` | reference numeric verified | Checked against bundled known-value/reference datasets or derived reference cases. |
| `jme_nod_aps` | `api_or_behavior_contract` | contract verified only | Directly tested, but not yet broadly externally value-certified against an independent web oracle. |
| `jme_nod_aps_ut` | `api_or_behavior_contract` | contract verified only | Directly tested, but not yet broadly externally value-certified against an independent web oracle. |
| `jme_orbit_max_min_true_distance` | `api_or_behavior_contract` | contract verified only | Directly tested, but not yet broadly externally value-certified against an independent web oracle. |
| `jme_pheno` | `api_or_behavior_contract` | contract verified only | Directly tested, but not yet broadly externally value-certified against an independent web oracle. |
| `jme_pheno_ut` | `api_or_behavior_contract` | contract verified only | Directly tested, but not yet broadly externally value-certified against an independent web oracle. |
| `jme_radian_midpoint` | `independent_formula_or_identity` | formula/identity verified | Checked by closed-form formula, round-trip identity, or deterministic mathematical invariant. |
| `jme_radian_normalize` | `independent_formula_or_identity` | formula/identity verified | Checked by closed-form formula, round-trip identity, or deterministic mathematical invariant. |
| `jme_radians_difference_signed` | `independent_formula_or_identity` | formula/identity verified | Checked by closed-form formula, round-trip identity, or deterministic mathematical invariant. |
| `jme_radians_to_degrees` | `independent_formula_or_identity` | formula/identity verified | Checked by closed-form formula, round-trip identity, or deterministic mathematical invariant. |
| `jme_rectangular_to_spherical_state` | `independent_formula_or_identity` | formula/identity verified | Checked by closed-form formula, round-trip identity, or deterministic mathematical invariant. |
| `jme_refract` | `independent_formula_or_identity` | formula/identity verified | Checked by closed-form formula, round-trip identity, or deterministic mathematical invariant. |
| `jme_refract_extended` | `independent_formula_or_identity` | formula/identity verified | Checked by closed-form formula, round-trip identity, or deterministic mathematical invariant. |
| `jme_reverse_julian_day` | `independent_formula_or_identity` | formula/identity verified | Checked by closed-form formula, round-trip identity, or deterministic mathematical invariant. |
| `jme_rise_trans` | `api_or_behavior_contract` | contract verified only | Directly tested, but not yet broadly externally value-certified against an independent web oracle. |
| `jme_rise_trans_true_hor` | `api_or_behavior_contract` | contract verified only | Directly tested, but not yet broadly externally value-certified against an independent web oracle. |
| `jme_set_astro_models` | `api_or_behavior_contract` | contract verified only | Directly tested, but not yet broadly externally value-certified against an independent web oracle. |
| `jme_set_delta_t_userdef` | `api_or_behavior_contract` | contract verified only | Directly tested, but not yet broadly externally value-certified against an independent web oracle. |
| `jme_set_ephemeris_path` | `api_or_behavior_contract` | contract verified only | Directly tested, but not yet broadly externally value-certified against an independent web oracle. |
| `jme_set_interpolate_nut` | `api_or_behavior_contract` | contract verified only | Directly tested, but not yet broadly externally value-certified against an independent web oracle. |
| `jme_set_jpl_file` | `api_or_behavior_contract` | contract verified only | Directly tested, but not yet broadly externally value-certified against an independent web oracle. |
| `jme_set_lapse_rate` | `api_or_behavior_contract` | contract verified only | Directly tested, but not yet broadly externally value-certified against an independent web oracle. |
| `jme_set_sidereal_mode` | `api_or_behavior_contract` | contract verified only | Directly tested, but not yet broadly externally value-certified against an independent web oracle. |
| `jme_set_tid_acc` | `api_or_behavior_contract` | contract verified only | Directly tested, but not yet broadly externally value-certified against an independent web oracle. |
| `jme_set_topo` | `api_or_behavior_contract` | contract verified only | Directly tested, but not yet broadly externally value-certified against an independent web oracle. |
| `jme_sidereal_time` | `external_web_numeric` | web-source numeric verified | Checked against authoritative web/reference values in the current test suite. |
| `jme_sidereal_time0` | `external_web_numeric` | web-source numeric verified | Checked against authoritative web/reference values in the current test suite. |
| `jme_sol_eclipse_how` | `external_web_numeric` | web-source numeric verified | Checked against authoritative web/reference values in the current test suite. |
| `jme_sol_eclipse_when_glob` | `external_web_numeric` | web-source numeric verified | Checked against authoritative web/reference values in the current test suite. |
| `jme_sol_eclipse_when_loc` | `external_web_numeric` | web-source numeric verified | Checked against authoritative web/reference values in the current test suite. |
| `jme_sol_eclipse_where` | `external_web_numeric` | web-source numeric verified | Checked against authoritative web/reference values in the current test suite. |
| `jme_solcross` | `api_or_behavior_contract` | contract verified only | Directly tested, but not yet broadly externally value-certified against an independent web oracle. |
| `jme_solcross_ut` | `api_or_behavior_contract` | contract verified only | Directly tested, but not yet broadly externally value-certified against an independent web oracle. |
| `jme_spherical_angular_separation` | `independent_formula_or_identity` | formula/identity verified | Checked by closed-form formula, round-trip identity, or deterministic mathematical invariant. |
| `jme_spherical_position_angle` | `independent_formula_or_identity` | formula/identity verified | Checked by closed-form formula, round-trip identity, or deterministic mathematical invariant. |
| `jme_spherical_to_rectangular_state` | `independent_formula_or_identity` | formula/identity verified | Checked by closed-form formula, round-trip identity, or deterministic mathematical invariant. |
| `jme_split_degree` | `independent_formula_or_identity` | formula/identity verified | Checked by closed-form formula, round-trip identity, or deterministic mathematical invariant. |
| `jme_state_add` | `independent_formula_or_identity` | formula/identity verified | Checked by closed-form formula, round-trip identity, or deterministic mathematical invariant. |
| `jme_state_convert_units` | `independent_formula_or_identity` | formula/identity verified | Checked by closed-form formula, round-trip identity, or deterministic mathematical invariant. |
| `jme_state_distance` | `independent_formula_or_identity` | formula/identity verified | Checked by closed-form formula, round-trip identity, or deterministic mathematical invariant. |
| `jme_state_light_time_days` | `independent_formula_or_identity` | formula/identity verified | Checked by closed-form formula, round-trip identity, or deterministic mathematical invariant. |
| `jme_state_position_velocity_dot` | `independent_formula_or_identity` | formula/identity verified | Checked by closed-form formula, round-trip identity, or deterministic mathematical invariant. |
| `jme_state_scale` | `independent_formula_or_identity` | formula/identity verified | Checked by closed-form formula, round-trip identity, or deterministic mathematical invariant. |
| `jme_state_speed` | `independent_formula_or_identity` | formula/identity verified | Checked by closed-form formula, round-trip identity, or deterministic mathematical invariant. |
| `jme_state_subtract` | `independent_formula_or_identity` | formula/identity verified | Checked by closed-form formula, round-trip identity, or deterministic mathematical invariant. |
| `jme_time_equ` | `independent_formula_or_identity` | formula/identity verified | Checked by closed-form formula, round-trip identity, or deterministic mathematical invariant. |
| `jme_topo_arcus_visionis` | `api_or_behavior_contract` | contract verified only | Directly tested, but not yet broadly externally value-certified against an independent web oracle. |
| `jme_utc_time_zone` | `independent_formula_or_identity` | formula/identity verified | Checked by closed-form formula, round-trip identity, or deterministic mathematical invariant. |
| `jme_utc_to_jd` | `independent_formula_or_identity` | formula/identity verified | Checked by closed-form formula, round-trip identity, or deterministic mathematical invariant. |
| `jme_version` | `api_or_behavior_contract` | contract verified only | Directly tested, but not yet broadly externally value-certified against an independent web oracle. |
| `jme_vis_limit_mag` | `api_or_behavior_contract` | contract verified only | Directly tested, but not yet broadly externally value-certified against an independent web oracle. |
| `jme_vsop87_planet_state` | `embedded_reference_numeric` | reference numeric verified | Checked against bundled known-value/reference datasets or derived reference cases. |
<!-- END GENERATED VALUE VERIFICATION MATRIX -->
