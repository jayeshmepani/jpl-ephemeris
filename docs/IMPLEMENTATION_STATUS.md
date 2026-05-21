# Implementation Status

This file is the continuation checkpoint for this repository.

It is intentionally written as engineering status, not release marketing. A symbol being declared and defined means it is callable. It does not automatically mean the full astronomical or reference-compatible behavior is complete.

## Canonical Inventories

The canonical generated inventory is `docs/API_TRACKING.md`.

Current verified counts from `tests/test_symbol_coverage.ps1`:

| Inventory | Count | Source of truth |
|---|---:|---|
| Public `jme_*` functions | 204 | `docs/API_TRACKING.md`, `include/jme/jme.h`, `include/jme/jme_extended.h` |
| Public `JME_*` constants | 462 | `docs/API_TRACKING.md`, `include/jme/jme.h`, `include/jme/jme_extended.h` |

Use `docs/API_TRACKING.md` for the full 204-function list and full 462-constant list. Do not maintain a second manually copied list here, because that would create two sources of truth.

Use `docs/FUNCTION_CLOSURE_CHECKLIST.md` as the live behavior-closure checklist. Update that checklist whenever a function family moves from not-closed to closed.

Regenerate/check the inventory with:

```powershell
pwsh -NoProfile -ExecutionPolicy Bypass -File tests\test_symbol_coverage.ps1
```

Expected current output:

```text
jme_functions_total=204
jme_functions_defined=204
jme_constants_total=462
```

## JPL/CALCEPH Layer

Status: completed for the current raw JPL/CALCEPH API surface and verified with capability-specific real kernels/files.

Verification already performed:

```powershell
$env:JME_TEST_JPL_KERNEL = "E:\project\astrology\JPL-Moshier-Ephemeris\data\jpl\de440s.bsp"
ctest --test-dir build-vcpkg -C Debug --output-on-failure
$env:JME_TEST_JPL_KERNEL = $null
```

Result: `7/7` tests passed in the CALCEPH-enabled build.

The non-CALCEPH build also passes:

```powershell
ctest --test-dir build-local -C Debug --output-on-failure
```

Result: `7/7` tests passed.

Additional Linux verification for the extra-function closure checklist:

```bash
cmake -S . -B build-calceph -DJME_REQUIRE_CALCEPH=ON -Dcalceph_DIR=/tmp/jme-calceph-cmake/calceph -DCMAKE_BUILD_TYPE=Debug
cmake --build build-calceph
LD_LIBRARY_PATH=/tmp/jme-calceph-root/usr/lib/x86_64-linux-gnu \
JME_TEST_JPL_KERNEL=/home/shreesoftech/projects/test1/astro_packages/jpl-ephemeris-/data/jpl/de440s.bsp \
JME_TEST_JPL_CONSTANT_KERNEL=/home/shreesoftech/projects/test1/astro_packages/jpl-ephemeris-/tests/fixtures/jpl_constants.tpc \
JME_TEST_JPL_ORIENTATION_KERNEL=/tmp/jme-kernels/moon_pa_de440_200625.bpc \
JME_TEST_JPL_VERSION_KERNEL=/tmp/calceph-4.0.5/examples/example1.dat \
JME_TEST_JPL_ROTANGMOM_KERNEL=/tmp/calceph-4.0.5/examples/example2_rotangmom.dat \
ctest --test-dir build-calceph -R jpl_runtime --output-on-failure
```

Result: `1/1` `jpl_runtime` passed. This closes all 49 extra raw JPL/CALCEPH functions in `docs/FUNCTION_CLOSURE_CHECKLIST.md`.

### JPL Functions Currently Implemented

All functions below are implemented in `src/jpl.c`. When `JME_HAVE_CALCEPH` is enabled, they call CALCEPH or return a real error. When CALCEPH is not present, they return explicit unavailable/closed-kernel errors instead of pretending success.

| Area | Functions |
|---|---|
| Engine state | `jme_jpl_engine_version`, `jme_jpl_is_available`, `jme_jpl_is_open`, `jme_jpl_close` |
| Kernel open/load | `jme_jpl_open`, `jme_jpl_open_array`, `jme_jpl_prefetch` |
| Kernel capability metadata | `jme_jpl_is_thread_safe`, `jme_jpl_timescale`, `jme_jpl_coverage`, `jme_jpl_file_version`, `jme_jpl_current_file_data` |
| Kernel names and identifiers | `jme_jpl_id_by_name`, `jme_jpl_name_by_id` |
| Kernel constants | `jme_jpl_constant`, `jme_jpl_constant_count`, `jme_jpl_constant_index`, `jme_jpl_constant_vector`, `jme_jpl_constant_string`, `jme_jpl_constant_string_vector` |
| Kernel records | `jme_jpl_position_record_count`, `jme_jpl_position_record_index`, `jme_jpl_orientation_record_count`, `jme_jpl_orientation_record_index`, `jme_jpl_max_supported_order` |
| Body rectangular state, NAIF IDs | `jme_jpl_body_state_naif`, `jme_jpl_body_state_split_naif`, `jme_jpl_body_state_native_naif`, `jme_jpl_body_state_native_split_naif`, `jme_jpl_body_state_order_naif`, `jme_jpl_body_state_utc_naif` |
| Body rectangular state, project body IDs | `jme_jpl_body_state`, `jme_jpl_body_state_split`, `jme_jpl_body_state_native`, `jme_jpl_body_state_native_split`, `jme_jpl_body_state_order`, `jme_jpl_body_state_utc` |
| Ecliptic spherical state, NAIF IDs | `jme_jpl_ecliptic_state_naif`, `jme_jpl_ecliptic_state_split_naif`, `jme_jpl_ecliptic_state_utc_naif` |
| Ecliptic spherical state, project body IDs | `jme_jpl_ecliptic_state`, `jme_jpl_ecliptic_state_split`, `jme_jpl_ecliptic_state_utc` |
| Orientation state | `jme_jpl_orientation_state_naif`, `jme_jpl_orientation_state_split_naif`, `jme_jpl_orientation_state_order_naif`, `jme_jpl_orientation_state_utc_naif` |
| Rotational angular momentum state | `jme_jpl_rotational_angular_momentum_state_naif`, `jme_jpl_rotational_angular_momentum_state_split_naif`, `jme_jpl_rotational_angular_momentum_state_order_naif`, `jme_jpl_rotational_angular_momentum_state_utc_naif` |

### JPL Constants Currently Tracked

The complete 462-constant inventory is maintained in `docs/API_TRACKING.md`.

JPL-specific constants currently include:

| Constant | Meaning |
|---|---|
| `JME_JPL_TIMESCALE_UNKNOWN` | Unknown/unavailable kernel timescale |
| `JME_JPL_TIMESCALE_TDB` | Kernel timescale is TDB |
| `JME_JPL_TIMESCALE_TCB` | Kernel timescale is TCB |

### JPL Work Remaining

No remaining work is known inside the current raw JPL/CALCEPH boundary listed above.

Remaining work is above the JPL boundary, not inside kernel access:

- full apparent geocentric reduction pipeline using JPL vectors
- full topocentric reduction pipeline using JPL vectors
- remaining nutation variants, aberration, light-deflection, and time-scale policy validation
- full physical ephemeris attributes built from JPL states
- full eclipse circumstances and locality algorithms built from JPL states
- full rise/set/transit and fully validated heliacal algorithms built from JPL states
- asteroid/comet catalog loading beyond direct kernel target IDs

## Compatibility Surface

Status: JME-native only.

The in-tree public surface is the project-owned `jme_*` / `JME_*` API. The former Swiss-style adapter layer is no longer part of this repository build.

Current native hardening:

- `jme_set_astro_models` and `jme_get_astro_models` now maintain canonical model state for bias, nutation, obliquity, precession, sidereal time, and Delta-T profiles instead of only storing a raw string.
- `jme_get_nutation` implements IAU 1980 and IAU 2000B abridged nutation. The IAU 2000B path uses the 77-term MHB_2000_SHORT/Luzum series and fixed planetary-bias offsets used by ERFA/SOFA. The declared IAU 2000A constant is intentionally rejected until the full 1365-term model is implemented, avoiding silent approximation.
- `jme_sidereal_time` now consumes the selected sidereal-time model: default/IAU 1976 keeps the historical GMST expression, while IAU 2006 uses the Earth-rotation-angle plus Capitaine-compatible GMST06 polynomial with TT from the active Delta-T model.
- Frame-bias model state is now consumed by the equatorial reduction pipeline. IAU 2000/2006 bias uses the fixed MHB2000/Chapront frame-bias angles before precession in planet, fixed-star, and topocentric event reductions; `JME_MODEL_BIAS_NONE` remains an identity transform.
- Direct public contracts now assert that `jme_moshier_planet_state` succeeds for supported planet bodies and rejects unsupported ones, and that `jme_meeus_planet_state` does the same for its supported-body subset. That is a contract check, not a precision certification.
- The public calculation pipeline now also has a direct regression that exercises a non-JPL analytic fallback body path (`JME_BODY_PLUTO`) and requires finite output, which helps guard the VSOP/Moshier/Meeus selection stack without overstating numerical certification.
- The same regression now checks rectangular and distance-unit flag behavior for the fallback Pluto path, so the public contract is not only "finite" but also flag-consistent.
- The public calculation regression also checks velocity-per-second conversion for a representative analytic body, so rectangular velocity scaling is covered too.
- The same contract block now checks `jme_calc_pctr()` for the Mercury/Sun difference contract using the same ET timescale, and also checks spherical conversion, radians, sidereal longitude, null-output rejection, unsupported body/center rejection, and distance-unit scaling against the direct heliocentric Mercury vector norm.
- `jme_jpl_current_file_data` is now closed for the Swiss mapped metadata row: the CALCEPH runtime suite validates a real `de440s.bsp` success path, output coverage span, closed/unavailable error paths, reset behavior, and null metadata output rejection.
- `jme_get_ayanamsa_ex` no longer silently maps unsupported sidereal model IDs to Lahiri. Lahiri, Fagan-Bradley, user-defined mode, UT wrapper behavior, null-output rejection, and unsupported-model rejection are covered. The numeric ayanamsa rows remain open until all declared sidereal models have source-backed formula/epoch data and known-value validation.
- `jme_fixstar*` now has stronger safety and flag handling: null/empty star names reject cleanly, spherical output honors radians and sidereal longitude flags, nutation conversion uses the full-precision degree-to-radian constant, and every built-in catalog entry is contract-tested for finite position and magnitude. The fixed-star rows remain open until broad catalog support and reference-grade reduction validation are implemented.
- The analytical validation suite also checks direct public state production for representative Moshier, VSOP87, and Meeus bodies at J2000 so the stack remains callable as state providers, not just through derived-element paths.
- `jme_houses_ex2` and `jme_houses_armc_ex2` compute finite cusp and angle speeds by central difference instead of returning zero arrays.
- `jme_pheno` and `jme_pheno_ut` return phase angle, illuminated fraction, elongation, apparent diameter, magnitude, internal distance fields, light-time, apparent radius, phase defect, and bright-limb position angle.
- `jme_rise_trans` supports refined rise, set, meridian transit, anti-meridian transit, civil/nautical/astronomical twilight horizons, disc-center horizon handling, and no-refraction mode. The rise/transit altitude path now uses the public topocentric equatorial calculation pipeline directly, avoiding mixed-frame rectangular corrections. Contract tests independently recompute returned altitude and hour-angle roots for Sun, Moon, Mars, and Sirius plus invalid/no-event paths.
- Solar, lunar, and heliocentric longitude crossings plus lunar node crossings refine roots instead of returning coarse scan midpoints.
- Lunar node/apside helpers return bounded lunar mean node, true node, mean apogee, and focal-point longitude contracts.
- Orbital-element derivation now returns a fuller derived set from the current state vector, including eccentric anomaly, mean anomaly, mean motion, period, perihelion/aphelion distance, and related longitudes.
- Solar eclipse global/local search, geographic circumstance, and local circumstance entry points now return native geometric results. Global classification now uses Moon-shadow cone versus Earth-sphere geometry so hybrid and central/noncentral behavior are represented natively.
- Eclipse, occultation, and heliacal functions now have native geometry paths. Heliacal support is a first-pass visibility model, not final reference-method parity.

Known limits:

- Astro model selection has real canonical state and is consumed by reduction paths. Frame bias now has none plus IAU 2000/2006 paths, obliquity has IAU 1980 plus corrected IAU 2006/P03 polynomial coverage, precession has IAU 1976 and P03/IAU 2006-family matrix paths, nutation has IAU 1980 plus IAU 2000B paths, and sidereal time has IAU 1976/default plus IAU 2006 paths. Remaining model-specific formula gaps are mainly IAU 2000A nutation and broader cross-validation.
- Solar and lunar eclipse outputs now return native search/circumstance results, including hybrid solar classification and central/noncentral shadow classification, but independent validation coverage for exact locality/contact precision is still narrower than the rest of the API.
- Solar partial-only global circumstances no longer report negative magnitude from geocentric apparent separation; `jme_sol_eclipse_where()` now derives the circumstance fields at the returned shadow-surface location, and 2022-10-25 is covered as a partial solar known-value regression.
- Global lunar eclipse search/circumstance behavior is closed for JME-native behavior: total, partial, and penumbral eclipse regressions are covered against external known-value maxima/magnitudes, backward search works, non-eclipse instants reject, and null-output paths are tested.
- Lunar occultation locality/contact/search now return first-pass native geometry results, with validation limits documented below.
- Solar eclipse, lunar eclipse, and lunar occultation public entry points now have direct null required-output/geopos rejection tests. This is input-contract hardening, not exact contact/locality closure.
- Sunshine is implemented for date-aware `jme_houses`/`jme_houses_ex`; ARMC Sunshine is also available when the caller supplies Sun declination in `ascmc[9]`.
- Gauquelin sector now branches by method: ecliptic longitude/latitude semiarc, longitude-only semiarc, disk-center rise/set, and refracted disk-center rise/set. It returns fractional sectors, but it is not yet certified as full reference-method parity.
- Heliacal visibility now returns first-pass native results using local twilight, Sun/body altitude, arcus visionis, elongation, apparent magnitude, and a simple limiting-magnitude rule. It still needs a source-backed visual-observer model and external known-value validation before it can be marked complete.
- Heliacal body selection is now explicit for array-output APIs: unsupported IDs, fractional IDs, node bodies, and null required outputs return errors instead of silently changing the request to Venus. Scalar helpers may still use Venus only when the caller omits `dat_hel` because they return one scalar value and use an internal default.
- Physical phenomena provide the standard first five phenomenon fields plus extended geometric fields; rotation-axis and central-meridian style physical ephemeris attributes remain open.

## Remaining Gaps

- `jme_lun_occult_where`, `jme_lun_occult_when_loc`, and `jme_lun_occult_when_glob` now have first-pass native Moon-target geometry. Global search uses lunar horizontal parallax as a possibility threshold; local search uses topocentric disk overlap. Broader external validation and exact geographic-contact modeling remain open.
- `jme_heliacal_ut`, `jme_heliacal_pheno_ut`, `jme_heliacal_angle`, `jme_topo_arcus_visionis`, and `jme_vis_limit_mag` now return first-pass native visibility fields. Remaining work is source-backed visual-model parity, atmospheric/observer parameter handling, and independent known-value validation.
- Eclipse coverage is materially stronger, and global lunar eclipse search/circumstance behavior is closed for the current JME-native contract. Exact independent validation density for topocentric solar locality, local lunar visibility/contact clipping, and occultation locality is still thinner than the rest of the API.
- `jme_lun_eclipse_when_loc` now intersects the global lunar eclipse phase window with the local Moon-above-horizon interval, clips local-visible contacts, and reports visible duration. It still uses geocentric shadow geometry rather than a fully independent topocentric lunar-shadow contact model.
- Physical phenomena are still partial: phase/elongation/diameter/magnitude, distance, light-time, apparent-radius, phase-defect, and bright-limb position-angle fields are implemented, but full rotational physical ephemeris outputs remain open.
- Gauquelin support is stronger but still partial: `jme_gauquelin_sector` supports methods 0 through 3 with fractional sectors; polar-region semiarc edge behavior and independent known-value certification remain open.
- Sunshine support is stronger but still partial: date-aware Sunshine is implemented, and ARMC Sunshine works with caller-supplied Sun declination in `ascmc[9]`; independent reference-value validation remains open.
- Broader independent known-value validation is still incomplete across fixed stars, full numeric ayanamsa model breadth, eclipse/occultation, heliacal visibility, and analytical fallback edge cases.
- Full ELP2000 lunar integration and broader cross-validation across VSOP87, Moshier, Meeus, and mixed fallback paths are still not closed out as complete.

## Moshier

Status: partially integrated.

What is real today:

- Native source material for the Moshier planetary tables and driver is present in the repo.
- A public callable wrapper exists for planetary state generation.
- The engine is part of the analytical fallback conversation for non-JPL operation.

What is not yet safe to claim:

- full exact-parity certification of Moshier output against an independently defined project contract
- fully documented frame/unit/date-range semantics for every returned component
- closed-out provenance/licensing and production-source decisions for all imported table files

Implemented/callable:

| Function | File | Current behavior |
|---|---|---|
| `jme_moshier_planet_state` | `src/moshier.c` | Calls Moshier `gplan()` tables for Mercury, Venus, Earth, Mars, Jupiter, Saturn, Uranus, Neptune, and Pluto. Returns heliocentric ecliptic spherical-style state values currently populated from `gplan()` output. |

Source material currently present:

- `src/moshier/gplan.c`
- `src/moshier/plantbl.h`
- `src/moshier/mer404.c`
- `src/moshier/ven404.c`
- `src/moshier/ear404.c`
- `src/moshier/mar404.c`
- `src/moshier/jup404.c`
- `src/moshier/sat404.c`
- `src/moshier/ura404.c`
- `src/moshier/nep404.c`
- `src/moshier/plu404.c`
- `src/moshier/mlr404.c`
- `src/moshier/mlat404.c`
- `src/moshier_full.c`

Current validation status:

- The engine is compiled in and callable through `jme_moshier_planet_state`.
- The broader analytical validation suite checks derived orbital-element plausibility against JPL SSD approximate-element tables for supported analytical engines, but that is not the same as per-body, per-date, exact Moshier certification.
- There is not yet a dense body-by-body Moshier known-value suite that closes position, velocity, frame, and domain behavior as exact.

Known remaining Moshier work:

- verify provenance and license notes for every imported Moshier table/source file
- normalize units and frame semantics into the project API contract
- add known-value tests per body
- add date-range/domain documentation
- integrate lunar Moshier latitude/radius tables if they are intended as a lunar fallback
- validate speed/derivative output, not only position output
- decide whether `src/moshier_full.c` is transitional source material or production source

Exact resume point for Moshier:

- keep `jme_moshier_planet_state` as implemented
- do not call Moshier “precision-certified” yet
- next exact-closeout work is per-body known-value validation, output-contract documentation, and clear production-source decisions

## VSOP87

Status: partially integrated.

What is real today:

- The repo contains VSOP87 source material and a public callable wrapper.
- `jme_vsop87_planet_state` is implemented for the major supported planets.
- The analytical validation suite already includes direct VSOP87A J2000 known-value checks for the supported planets.

What is not yet safe to claim:

- full exact-parity closure for all VSOP87 variants, frames, derivatives, and production-policy decisions
- closed-out semantic documentation for variant selection and public output contract

Implemented/callable:

| Function | File | Current behavior |
|---|---|---|
| `jme_vsop87_planet_state` | `src/vsop87.c` | Uses VSOP87D full-series functions for Mercury, Venus, Earth, Mars, Jupiter, Saturn, Uranus, and Neptune. Converts longitude/latitude from radians to degrees and returns radius. |

Source material currently present:

- `src/vsop87.c`
- `src/vsop87.h`
- `src/vsop87a_full.c`
- `src/vsop87d_full.c`

Current validation status:

- `tests/test_analytical_validation.c` validates a VSOP87A J2000 reference set for Mercury, Venus, Earth, Mars, Jupiter, Saturn, Uranus, and Neptune.
- The same suite also checks non-zero finite derivatives for the public output.
- This is meaningful validation, but it still does not close out all variant semantics, production-policy decisions, or broader date-range behavior.

Known remaining VSOP87 work:

- confirm whether VSOP87A and VSOP87D are both required in production or only one is canonical
- expand known-value tests beyond the current VSOP87A/J2000 validation case
- document exact VSOP87 variant, units, frame, origin, and time argument
- implement or explicitly exclude Pluto, Moon, Earth-Moon barycenter, asteroids, and comets
- validate speed/derivative handling; current public state is not yet a fully validated six-component position/velocity state
- reconcile VSOP output with the JPL primary pipeline and fallback selection policy

Exact resume point for VSOP87:

- keep `jme_vsop87_planet_state` as a real implemented analytical engine
- treat the current state as materially validated for a narrow high-value reference slice
- do not mark variant/frame/derivative/fallback semantics as fully closed out yet

## ELP2000

Status: partially wired. The public wrapper now calls the imported ELP2000-82B source and returns a geocentric Moon position vector, but the implementation is not yet precision-certified.

What is real today:

- The ELP2000-82B source tree is present in the repo.
- `jme_elp2000_moon_state` is callable and returns a Moon state from the imported source.
- The wrapper converts position units into the project API surface and exposes a non-zero derived velocity.

What is not yet safe to claim:

- full precision certification across date ranges
- final output-contract closure for frame, epoch, units, and derivative semantics
- final production fallback ordering relative to JPL, Meeus, and any lunar Moshier path

Callable symbol:

| Function | File | Current behavior |
|---|---|---|
| `jme_elp2000_moon_state` | `src/elp2000.c` | Calls `geocentric_moon_position_cartesian_of_J2000()`, converts kilometers to AU, and returns position plus central-difference velocity in AU/day. |

Source material currently present:

- `src/elp2000/elp2000-82b.c`
- `src/elp2000/elp2000-82b.h`
- `src/elp2000/arguments.h`
- `src/elp2000/mainprob.h`
- `src/elp2000/planetary1.h`
- `src/elp2000/planetary2.h`
- `src/elp2000/earthfig.h`
- `src/elp2000/relativistic.h`
- `src/elp2000/tidal.h`
- `src/elp2000/moon_nutation.h`

Current ELP2000 validation:

- `tests/test_analytical_validation.c` checks the Moon state against multiple public ELP2000-82B reference cases generated from the original Fortran code.
- The current source variant is tens-of-kilometers level against the modern external cases, so ELP is validated as wired and bounded but not yet precision-certified.
- The current wrapper path is therefore better described as “implemented and bounded” than “fully closed”.

Known remaining ELP2000 work:

- define output contract: geocentric/ecliptic frame, units, epoch, and velocity behavior
- expand known-value lunar tests across date ranges and source variants
- validate all correction terms intended for production use
- document date range and expected numerical precision
- decide fallback ordering between JPL Moon, ELP2000 Moon, Moshier Moon, and Meeus Moon

Exact resume point for ELP2000:

- keep `jme_elp2000_moon_state` as a real implemented lunar analytical path
- do not mark ELP2000 as exact-complete yet
- next exact-closeout work is broader lunar known-value validation, explicit frame/unit/velocity contract definition, and final fallback-policy decisions

## Meeus

Status: implemented as bounded analytical fallback/utilities, with certification work still required.

What is real today:

- Meeus-style analytical state helpers for Sun, Moon, and planets are implemented and callable.
- `jme_meeus_planet_state` now uses time-varying heliocentric mean elements plus a Newton-solved Kepler equation for Mercury, Venus, Earth, Mars, Jupiter, Saturn, Uranus, Neptune, and Pluto; unsupported bodies return `JME_ERR`.
- The Moon helper now consumes the auxiliary longitude/latitude terms already present in the routine instead of leaving those terms unused.
- Meeus-style utility behavior also exists in adjacent time, coordinate, refraction, and search helpers across the codebase.
- Meeus is therefore not just conceptual; it is part of the actual low-weight analytical toolbox in the repo.

What is not yet safe to claim:

- exact per-formula provenance/coverage signoff for all Meeus-derived utilities
- complete precision/date-range documentation
- closed-out semantic decisions about which Meeus formulas are production fallbacks and which are only support helpers

Implemented/callable:

| Function | File | Current behavior |
|---|---|---|
| `jme_meeus_sun_state` | `src/meeus.c` | Computes a solar analytical state used by fallback paths. |
| `jme_meeus_moon_state` | `src/meeus.c` | Computes a lunar analytical state used by fallback paths. |
| `jme_meeus_planet_state` | `src/meeus.c` | Computes J2000 ecliptic heliocentric rectangular analytical fallback for Mercury through Pluto and rejects unsupported bodies. |

Other Meeus-style utility areas currently implemented elsewhere:

- calendar and Julian day helpers in `src/julian.c`
- UTC/JD and Delta-T helpers in `src/time.c`
- coordinate conversion helpers in `src/coordinates.c`
- refraction helper in `src/coordinates.c`
- refined rise/set/transit and longitude-crossing searches in `src/events.c`

Current validation status:

- Meeus participates in the broader analytical/fallback validation story and has direct contract coverage for supported-body success and unsupported-body rejection.
- The repo currently proves that Meeus-style code exists, is callable, and returns finite state vectors for representative cases; it does not yet prove that every Meeus-derived helper is production-certified at the exactness standard you want.

Known remaining Meeus work:

- add source-specific citations/provenance per formula family
- add independent known-value tests for Sun, Moon, planets, time, refraction, and rise/set helpers
- document expected precision and date ranges per formula
- decide which Meeus formulas are production fallbacks and which are only support utilities
- validate velocity outputs and frame semantics for every state-returning function
- complete eclipse, physical phenomena, and heliacal formulas only if they meet project accuracy requirements

Exact resume point for Meeus:

- keep the Sun/Moon/planet state helpers as implemented analytical support
- treat the broader Meeus presence as real but still requiring dedicated certification
- next exact-closeout work is formula-family provenance, per-domain known-value validation, and clearer production-fallback policy

## High-Level Public API Status

The project currently has all 204 public `jme_*` functions declared and defined. Some functions are complete utilities or complete JPL-boundary functions. Some are partial high-level algorithms. Some are explicit error-returning contracts.

Examples of implemented-but-not-yet-complete high-level areas:

- `jme_elp2000_moon_state`
- `jme_sol_eclipse_where`
- `jme_sol_eclipse_how` exact circumstances
- `jme_lun_occult_where`
- `jme_lun_occult_when_loc`
- `jme_lun_occult_when_glob`
- `jme_heliacal_ut` source-backed visual-model parity
- `jme_heliacal_pheno_ut` source-backed visual-model parity
- `jme_heliacal_angle` output semantics validation
- `jme_topo_arcus_visionis` output semantics validation
- `jme_vis_limit_mag` observer/atmosphere model validation
- `jme_nod_aps` complete reference method variants
- `jme_nod_aps_ut` complete reference method variants
- `jme_get_orbital_elements` full reference attribute layout and validation
- `jme_gauquelin_sector` polar semiarc validation and independent known-value certification

Before marking any of these exact-complete, require:

- real algorithm implementation where the function is not already implemented
- independent source/provenance note
- known-value tests
- invalid-input tests
- documented output units and frame
- no false success path
- no `JME_OK` return with non-finite output values

## Current Test Gate

Current test commands:

```powershell
cmake --build build-local --config Debug
ctest --test-dir build-local -C Debug --output-on-failure

cmake --build build-vcpkg --config Debug
$env:JME_TEST_JPL_KERNEL = "E:\project\astrology\JPL-Moshier-Ephemeris\data\jpl\de440s.bsp"
ctest --test-dir build-vcpkg -C Debug --output-on-failure
$env:JME_TEST_JPL_KERNEL = $null

pwsh -NoProfile -ExecutionPolicy Bypass -File tests\test_symbol_coverage.ps1
```

Current verified results:

- `build-local`: `8/8` tests passed
- `build-vcpkg` with real `de440s.bsp`: `7/7` tests passed
- symbol coverage: `204/204` functions defined, `462` constants tracked

## Next Work Order

Recommended continuation order:

1. Harden ELP2000 output with known-value tests, velocity policy, frame/unit documentation, and precision/date-range limits.
2. Harden VSOP87 state output with known-value tests and velocity/frame documentation.
3. Harden Moshier fallback with provenance, known-value tests, and frame/unit documentation.
4. Decide exact fallback priority for `jme_calc`: JPL first, then ELP/Moshier/VSOP/Meeus according to body and requested precision.
5. Implement the explicit error-returning high-level functions one family at a time.
6. Update `docs/API_TRACKING.md`, `docs/REFERENCE_FUNCTION_COVERAGE.md`, and this file after each completed family.
