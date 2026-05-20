# Implementation Status

This file is the continuation checkpoint for this repository.

It is intentionally written as engineering status, not release marketing. A symbol being declared and defined means it is callable. It does not automatically mean the full astronomical or reference-compatible behavior is complete.

## Canonical Inventories

The canonical generated inventory is `docs/API_TRACKING.md`.

Current verified counts from `tests/test_symbol_coverage.ps1`:

| Inventory | Count | Source of truth |
|---|---:|---|
| Public `jme_*` functions | 191 | `docs/API_TRACKING.md`, `include/jme/jme.h`, `include/jme/jme_extended.h` |
| Public `JME_*` constants | 462 | `docs/API_TRACKING.md`, `include/jme/jme.h`, `include/jme/jme_extended.h` |

Use `docs/API_TRACKING.md` for the full 191-function list and full 462-constant list. Do not maintain a second manually copied list here, because that would create two sources of truth.

Regenerate/check the inventory with:

```powershell
pwsh -NoProfile -ExecutionPolicy Bypass -File tests\test_symbol_coverage.ps1
```

Expected current output:

```text
jme_functions_total=191
jme_functions_defined=191
jme_constants_total=462
```

## JPL/CALCEPH Layer

Status: completed for the current JPL API surface and verified with a real kernel.

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

No remaining work is known inside the raw JPL/CALCEPH boundary listed above.

Remaining work is above the JPL boundary, not inside kernel access:

- full apparent geocentric reduction pipeline using JPL vectors
- full topocentric reduction pipeline using JPL vectors
- complete frame-bias, precession, nutation, aberration, light-deflection, and time-scale policy validation
- full physical ephemeris attributes built from JPL states
- full eclipse circumstances and locality algorithms built from JPL states
- full rise/set/transit and heliacal algorithms built from JPL states
- asteroid/comet catalog loading beyond direct kernel target IDs

## Compatibility Surface

Status: JME-native only.

The in-tree public surface is the project-owned `jme_*` / `JME_*` API. The former Swiss-style adapter layer is no longer part of this repository build.

Current native hardening:

- `jme_houses_ex2` and `jme_houses_armc_ex2` compute finite cusp and angle speeds by central difference instead of returning zero arrays.
- `jme_pheno` and `jme_pheno_ut` return phase angle, illuminated fraction, elongation, apparent diameter, magnitude, and internal distance fields.
- `jme_rise_trans` supports refined rise, set, meridian transit, anti-meridian transit, civil/nautical/astronomical twilight horizons, disc-center horizon handling, and no-refraction mode.
- Solar, lunar, and heliocentric longitude crossings plus lunar node crossings refine roots instead of returning coarse scan midpoints.
- Lunar node/apside helpers return bounded lunar mean node, true node, mean apogee, and focal-point longitude contracts.
- Orbital-element derivation now returns a fuller derived set from the current state vector, including eccentric anomaly, mean anomaly, mean motion, period, perihelion/aphelion distance, and related longitudes.
- Solar eclipse global/local search, geographic circumstance, and local circumstance entry points now return native geometric results. Global classification now uses Moon-shadow cone versus Earth-sphere geometry so hybrid and central/noncentral behavior are represented natively.
- Eclipse, occultation, and heliacal functions that are still exact-pending return `JME_ERR` with explicit error text instead of silent success.

Known limits:

- Solar and lunar eclipse outputs now return native search/circumstance results, including hybrid solar classification and central/noncentral shadow classification, but independent validation coverage for exact locality/contact precision is still narrower than the rest of the API.
- Lunar occultation locality/contact/search remain explicit `JME_ERR`.
- Sunshine is implemented for date-aware `jme_houses`/`jme_houses_ex`; ARMC-only Sunshine remains `ERR` because that API does not carry the required Sun declination input. Full Gauquelin house-system variants remain `ERR` unless an exact independent formula is implemented and validated.
- Gauquelin sector currently uses refined surrounding rise/set events and returns a bounded sector only when those events are available; it is not yet certified as full reference-method parity.
- Occultations and heliacal visibility remain explicit error-returning contracts until validated algorithms are added.
- Physical phenomena provide the standard first five phenomenon fields plus internal distance fields in extended slots; rotation-axis and central-meridian style physical ephemeris attributes remain open.

## Remaining Gaps

- `jme_lun_occult_where`, `jme_lun_occult_when_loc`, and `jme_lun_occult_when_glob` are still unimplemented and return explicit validated-algorithm errors.
- `jme_heliacal_ut`, `jme_heliacal_pheno_ut`, `jme_heliacal_angle`, and `jme_topo_arcus_visionis` are still unimplemented and return explicit validated-model errors.
- Eclipse coverage is materially stronger, but exact independent validation density for topocentric solar/lunar locality and contact precision is still thinner than the rest of the API.
- `jme_lun_eclipse_when_loc` still reuses global contact times and adds local visibility; it does not yet solve a separate local-contact geometry.
- Physical phenomena are still partial: the standard phase/elongation/diameter/magnitude fields are implemented, but full rotational physical ephemeris outputs remain open.
- Gauquelin support is still partial: `jme_gauquelin_sector` is bounded and refined, but not yet certified as full reference-method parity.
- Sunshine support is still partial: date-aware Sunshine is implemented, but ARMC-only Sunshine remains unsupported because that API does not provide Sun declination.
- Broader independent known-value validation is still incomplete across `jme_calc`, `jme_calc_ut`, `jme_calc_pctr`, fixed stars, ayanamsa, houses, rise/set/transit, physical phenomena, and analytical fallbacks.
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
| `jme_elp2000_moon_state` | `src/elp2000.c` | Calls `geocentric_moon_position_cartesian_of_J2000()`, converts kilometers to AU, and returns position components. Velocity components remain zero because this wrapper path does not yet derive velocity. |

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

- `tests/test_analytical_validation.c` checks the Moon state against a public ELP2000-82B reference case for JD `2451555.5`.
- The current source variant is kilometer-level against that external case, so ELP is validated as wired and bounded but not yet precision-certified.
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

Status: partially implemented as lightweight analytical fallback/utilities.

What is real today:

- Meeus-style analytical state helpers for Sun, Moon, and planets are implemented and callable.
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
| `jme_meeus_planet_state` | `src/meeus.c` | Computes analytical planetary fallback for supported bodies. |

Other Meeus-style utility areas currently implemented elsewhere:

- calendar and Julian day helpers in `src/julian.c`
- UTC/JD and Delta-T helpers in `src/time.c`
- coordinate conversion helpers in `src/coordinates.c`
- refraction helper in `src/coordinates.c`
- refined rise/set/transit and longitude-crossing searches in `src/events.c`

Current validation status:

- Meeus participates in the broader analytical/fallback validation story, but does not yet have a dedicated exhaustive known-value certification layer across all supported helper families.
- The repo currently proves that Meeus-style code exists and is callable; it does not yet prove that every Meeus-derived helper is production-certified at the exactness standard you want.

Known remaining Meeus work:

- add source-specific citations/provenance per formula family
- add independent known-value tests for Sun, Moon, planets, time, refraction, and rise/set helpers
- document expected precision and date ranges per formula
- decide which Meeus formulas are production fallbacks and which are only support utilities
- validate velocity outputs and frame semantics for every state-returning function
- complete eclipse, physical phenomena, and heliacal formulas only if they meet project accuracy requirements

Exact resume point for Meeus:

- keep the Sun/Moon/planet state helpers as implemented analytical support
- treat the broader Meeus presence as real but still partially certified
- next exact-closeout work is formula-family provenance, per-domain known-value validation, and clearer production-fallback policy

## High-Level Public API Status

The project currently has all 191 public `jme_*` functions declared and defined. Some functions are complete utilities or complete JPL-boundary functions. Some are partial high-level algorithms. Some are explicit error-returning contracts.

Examples of explicit remaining or incomplete high-level areas:

- `jme_elp2000_moon_state`
- `jme_sol_eclipse_where`
- `jme_sol_eclipse_how` exact circumstances
- `jme_lun_occult_where`
- `jme_lun_occult_when_loc`
- `jme_lun_occult_when_glob`
- `jme_heliacal_ut`
- `jme_heliacal_pheno_ut`
- `jme_heliacal_angle`
- `jme_topo_arcus_visionis`
- `jme_vis_limit_mag`
- `jme_nod_aps` complete reference method variants
- `jme_nod_aps_ut` complete reference method variants
- `jme_get_orbital_elements` full reference attribute layout and validation
- `jme_gauquelin_sector`

Before marking any of these complete, require:

- real algorithm implementation
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
- symbol coverage: `191/191` functions defined, `462` constants tracked

## Next Work Order

Recommended continuation order:

1. Harden ELP2000 output with known-value tests, velocity policy, frame/unit documentation, and precision/date-range limits.
2. Harden VSOP87 state output with known-value tests and velocity/frame documentation.
3. Harden Moshier fallback with provenance, known-value tests, and frame/unit documentation.
4. Decide exact fallback priority for `jme_calc`: JPL first, then ELP/Moshier/VSOP/Meeus according to body and requested precision.
5. Implement the explicit error-returning high-level functions one family at a time.
6. Update `docs/API_TRACKING.md`, `docs/REFERENCE_FUNCTION_COVERAGE.md`, and this file after each completed family.
