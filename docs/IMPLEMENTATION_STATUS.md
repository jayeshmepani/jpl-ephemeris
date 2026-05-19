# Implementation Status

This file is the continuation checkpoint for this repository.

It is intentionally written as engineering status, not release marketing. A symbol being declared and defined means it is callable. It does not automatically mean the full astronomical or reference-compatible behavior is complete.

## Canonical Inventories

The canonical generated inventory is `docs/API_TRACKING.md`.

Current verified counts from `tests/test_symbol_coverage.ps1`:

| Inventory | Count | Source of truth |
|---|---:|---|
| Public `jme_*` functions | 191 | `docs/API_TRACKING.md`, `include/jme/jme.h`, `include/jme/jme_extended.h` |
| Public `JME_*` constants | 460 | `docs/API_TRACKING.md`, `include/jme/jme.h`, `include/jme/jme_extended.h` |
| Reference constants tracked | 348 | `include/jme_compat/swephexp.h` |

Use `docs/API_TRACKING.md` for the full 191-function list and full 460-constant list. Do not maintain a second manually copied list here, because that would create two sources of truth.

Regenerate/check the inventory with:

```powershell
pwsh -NoProfile -ExecutionPolicy Bypass -File tests\test_symbol_coverage.ps1
```

Expected current output:

```text
jme_functions_total=191
jme_functions_defined=191
jme_constants_total=460
reference_constants_total=348
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

The complete 460-constant inventory is maintained in `docs/API_TRACKING.md`.

JPL-specific constants currently include:

| Constant | Meaning |
|---|---|
| `JME_JPL_TIMESCALE_UNKNOWN` | Unknown/unavailable kernel timescale |
| `JME_JPL_TIMESCALE_TDB` | Kernel timescale is TDB |
| `JME_JPL_TIMESCALE_TCB` | Kernel timescale is TCB |

JPL-related reference constants from `include/jme_compat/swephexp.h` are tracked in the reference-constant list, including `SEFLG_JPLEPH`, `SEFLG_JPLHOR`, `SEFLG_JPLHOR_APPROX`, `SE_TIDAL_JPLEPH`, `SE_MODEL_JPLHOR_MODE`, and related model constants.

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

## Moshier

Status: partially integrated.

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

Known remaining Moshier work:

- verify provenance and license notes for every imported Moshier table/source file
- normalize units and frame semantics into the project API contract
- add known-value tests per body
- add date-range/domain documentation
- integrate lunar Moshier latitude/radius tables if they are intended as a lunar fallback
- validate speed/derivative output, not only position output
- decide whether `src/moshier_full.c` is transitional source material or production source

## VSOP87

Status: partially integrated.

Implemented/callable:

| Function | File | Current behavior |
|---|---|---|
| `jme_vsop87_planet_state` | `src/vsop87.c` | Uses VSOP87D full-series functions for Mercury, Venus, Earth, Mars, Jupiter, Saturn, Uranus, and Neptune. Converts longitude/latitude from radians to degrees and returns radius. |

Source material currently present:

- `src/vsop87.c`
- `src/vsop87.h`
- `src/vsop87a_full.c`
- `src/vsop87d_full.c`

Known remaining VSOP87 work:

- confirm whether VSOP87A and VSOP87D are both required in production or only one is canonical
- add known-value tests for all supported planets
- document exact VSOP87 variant, units, frame, origin, and time argument
- implement or explicitly exclude Pluto, Moon, Earth-Moon barycenter, asteroids, and comets
- validate speed/derivative handling; current public state is not yet a fully validated six-component position/velocity state
- reconcile VSOP output with the JPL primary pipeline and fallback selection policy

## ELP2000

Status: source material present, public wrapper still returns `JME_ERR`.

Callable symbol:

| Function | File | Current behavior |
|---|---|---|
| `jme_elp2000_moon_state` | `src/elp2000.c` | Returns `JME_ERR`; the public wrapper is not wired to the ELP2000 source tree yet. |

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

Known remaining ELP2000 work:

- wire `jme_elp2000_moon_state` to the imported ELP2000-82B implementation
- define output contract: geocentric/ecliptic frame, units, epoch, and velocity behavior
- add known-value lunar tests
- validate all correction terms intended for production use
- document date range and expected numerical precision
- decide fallback ordering between JPL Moon, ELP2000 Moon, Moshier Moon, and Meeus Moon

## Meeus

Status: partially implemented as lightweight analytical fallback/utilities.

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
- basic rise/set stepping search in `src/events.c`

Known remaining Meeus work:

- add source-specific citations/provenance per formula family
- add independent known-value tests for Sun, Moon, planets, time, refraction, and rise/set helpers
- document expected precision and date ranges per formula
- decide which Meeus formulas are production fallbacks and which are only support utilities
- validate velocity outputs and frame semantics for every state-returning function
- complete eclipse, physical phenomena, and heliacal formulas only if they meet project accuracy requirements

## High-Level Public API Status

The project currently has all 191 public `jme_*` functions declared and defined. Some functions are complete utilities or complete JPL-boundary functions. Some are partial high-level algorithms. Some are explicit error-returning contracts.

Examples of explicit remaining or incomplete high-level areas:

- `jme_elp2000_moon_state`
- `jme_orbit_max_min_true_distance`
- `jme_sol_eclipse_where`
- `jme_sol_eclipse_how`
- `jme_lun_eclipse_how`
- `jme_lun_occult_where`
- `jme_lun_occult_when_loc`
- `jme_lun_occult_when_glob`
- `jme_heliacal_ut`
- `jme_heliacal_pheno_ut`
- `jme_heliacal_angle`
- `jme_topo_arcus_visionis`
- `jme_vis_limit_mag`
- `jme_nod_aps`
- `jme_nod_aps_ut`
- `jme_get_orbital_elements`
- `jme_gauquelin_sector`

Before marking any of these complete, require:

- real algorithm implementation
- independent source/provenance note
- known-value tests
- invalid-input tests
- documented output units and frame
- no false success path

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

- `build-local`: `7/7` tests passed
- `build-vcpkg` with real `de440s.bsp`: `7/7` tests passed
- symbol coverage: `191/191` functions defined, `460` constants tracked, `348` reference constants tracked

## Next Work Order

Recommended continuation order:

1. Finish ELP2000 wrapper because source material exists but public wrapper still returns `JME_ERR`.
2. Harden VSOP87 state output with known-value tests and velocity/frame documentation.
3. Harden Moshier fallback with provenance, known-value tests, and frame/unit documentation.
4. Decide exact fallback priority for `jme_calc`: JPL first, then ELP/Moshier/VSOP/Meeus according to body and requested precision.
5. Implement the explicit error-returning high-level functions one family at a time.
6. Update `docs/API_TRACKING.md`, `docs/REFERENCE_FUNCTION_COVERAGE.md`, and this file after each completed family.
