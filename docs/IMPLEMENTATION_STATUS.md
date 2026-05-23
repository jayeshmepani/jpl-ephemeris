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

Use `docs/VALUE_VERIFICATION_MATRIX.md` when the question is not "is this callable?" but "how is this function currently validated?" That matrix assigns every public function to one of the current verification modes: external web numeric, embedded reference numeric, independent formula/identity, kernel runtime/error contract, or API/behavior contract only.

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

Additional Linux verification for the extra-function closure checklist in this workspace:

```bash
cmake -S . -B build-calceph-runtime -DJME_REQUIRE_CALCEPH=ON -Dcalceph_DIR=/tmp/jme-calceph-deb/root/usr/lib/x86_64-linux-gnu/cmake/calceph -DCMAKE_BUILD_TYPE=Debug
cmake --build build-calceph-runtime
LD_LIBRARY_PATH=/tmp/jme-calceph-deb/root/usr/lib/x86_64-linux-gnu \
JME_TEST_JPL_KERNEL=/home/shreesoftech/projects/test1/astro_packages/jpl-ephemeris-/data/jpl/de440s.bsp \
JME_TEST_JPL_CONSTANT_KERNEL=/home/shreesoftech/projects/test1/astro_packages/jpl-ephemeris-/tests/fixtures/jpl_constants.tpc \
JME_TEST_JPL_ORIENTATION_KERNEL=/tmp/jme-kernels/moon_pa_de440_200625.bpc \
JME_TEST_JPL_VERSION_KERNEL=/tmp/jme-calceph-src/calceph-calceph_4_0_5/examples/example1.dat \
JME_TEST_JPL_ROTANGMOM_KERNEL=/tmp/jme-calceph-src/calceph-calceph_4_0_5/examples/example2_rotangmom.dat \
ctest --test-dir build-calceph-runtime -R 'jpl_runtime|extra_contract|analytical_validation' --output-on-failure
```

Result: `3/3` tests passed: `jpl_runtime`, `extra_contract`, and `analytical_validation`. This keeps all 49 extra raw JPL/CALCEPH functions closed in `docs/FUNCTION_CLOSURE_CHECKLIST.md`.

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
- full rise/set/transit model breadth built from JPL states
- asteroid/comet catalog loading beyond direct kernel target IDs

## Compatibility Surface

Status: JME-native only.

The in-tree public surface is the project-owned `jme_*` / `JME_*` API. The former Swiss-style adapter layer is no longer part of this repository build.

Current native hardening:

- `jme_set_astro_models` and `jme_get_astro_models` now maintain canonical model state for bias, nutation, obliquity, precession, sidereal time, and Delta-T profiles instead of only storing a raw string.
- `jme_get_nutation` is closed for the project-owned public contract: IAU 1980 and IAU 2000B abridged nutation are supported and tested; IAU 2000B uses the 77-term MHB_2000_SHORT/Luzum series and fixed planetary-bias offsets used by ERFA/SOFA; the declared IAU 2000A constant is intentionally rejected until the full 1365-term model is implemented; unknown model IDs and null output pointers reject instead of silently falling back.
- `jme_sidereal_time` now consumes the selected sidereal-time model: default/IAU 1976 keeps the historical GMST expression, while IAU 2006 uses the Earth-rotation-angle plus Capitaine-compatible GMST06 polynomial with TT from the active Delta-T model.
- Frame-bias model state is now consumed by the equatorial reduction pipeline. IAU 2000/2006 bias uses the fixed MHB2000/Chapront frame-bias angles before precession in planet, fixed-star, and topocentric event reductions; `JME_MODEL_BIAS_NONE` remains an identity transform.
- Direct public contracts now assert that `jme_moshier_planet_state` succeeds for supported planet bodies and rejects unsupported ones, and that `jme_meeus_planet_state` does the same for its supported-body subset. That is a contract check, not a precision certification.
- Engine selection is now a C-library feature. Wrappers can select the calculation engine by calling `jme_set_astro_models("ENGINE=JPL", 0)`, `jme_set_astro_models("ENGINE=MOSHIER", 0)`, or `jme_set_astro_models("ENGINE=VSOP_ELP_MEEUS", 0)`. If no explicit engine token is set, the library reads `JME_ENGINE` values `JPL`, `MOSHIER`, `VSOP_ELP_MEEUS`, or `ANALYTICAL`; otherwise it defaults to `AUTO`.
- `ENGINE=JPL` is strict: if no CALCEPH kernel is open or the requested JPL state fails, `jme_calc` returns `JME_ERR` instead of silently using analytical fallback. `ENGINE=MOSHIER` and `ENGINE=VSOP_ELP_MEEUS` skip JPL even if a kernel is available.
- The public calculation pipeline now also has a direct regression that exercises a non-JPL analytic fallback body path (`JME_BODY_PLUTO`) and requires finite output, which helps guard the VSOP/Moshier/Meeus selection stack without overstating numerical certification.
- The same regression now checks rectangular and distance-unit flag behavior for the fallback Pluto path, so the public contract is not only "finite" but also flag-consistent.
- The public calculation regression also checks velocity-per-second conversion for a representative analytic body, so rectangular velocity scaling is covered too.
- The no-kernel analytical fallback order in `jme_calc` is now explicit and tested: `AUTO` tries JPL first and then analytical fallback; `MOSHIER` uses Moshier-first planets; `VSOP_ELP_MEEUS` uses VSOP87 before Meeus for planets. The Moon uses ELP2000 first, then Meeus for the geocentric lunar state, and combines that with the selected analytical Earth heliocentric state. `tests/test_analytical_validation.c` checks that Moshier mode matches direct Moshier planet output for Mercury, Earth, and Pluto across multiple dates, that Moshier-mode Moon output matches direct ELP2000 Moon plus Moshier Earth, and that VSOP/ELP/Meeus mode selects VSOP87 for Mercury and Meeus for Pluto.
- NASA/JPL Horizons observer ecliptic regression is now included for 2026-05-22 00:00 UTC geocentric Sun and Moon apparent ecliptic-of-date longitude/latitude. This external check found that no-kernel analytical fallback output was being returned in a J2000 ecliptic frame while user-facing `jme_calc` output was documented as date-frame apparent ecliptic. `src/calc.c` now converts analytical J2000 ecliptic vectors into the public reduction pipeline before final output. Current deltas against Horizons quantity 31 are about `0.0071°` for the Sun longitude and `0.0076°` for the Moon longitude in no-kernel Moshier/ELP fallback mode.
- The external Horizons screen is now broader than the original Sun/Moon regression. `tests/test_analytical_validation.c` also checks geocentric apparent ecliptic-of-date longitude/latitude for Sun, Moon, Mercury, Venus, Mars, Jupiter, Saturn, Uranus, Neptune, and Pluto at `2000-01-01 00:00 UTC`, `2026-05-22 00:00 UTC`, and `2050-01-01 00:00 UTC` under both `ENGINE=MOSHIER` and `ENGINE=VSOP_ELP_MEEUS`. Current observed deltas stay within the declared tolerances of `0.02°/0.02°` for Moshier mode and `0.03°/0.02°` for VSOP/ELP/Meeus mode.
- The same contract block now checks `jme_calc_pctr()` for the Mercury/Sun difference contract using the same ET timescale, and also checks spherical conversion, radians, sidereal longitude, null-output rejection, unsupported body/center rejection, and distance-unit scaling against the direct heliocentric Mercury vector norm.
- `jme_jpl_current_file_data` is now closed for the Swiss mapped metadata row: the CALCEPH runtime suite validates a real `de440s.bsp` success path, output coverage span, closed/unavailable error paths, reset behavior, and null metadata output rejection.
- `jme_get_ayanamsa_ex` no longer silently maps unknown sidereal model IDs to Lahiri. Lahiri, Fagan-Bradley, user-defined mode, epoch-zero modes (`J2000`, `J1900`, `B1950`, `Aryabhata`, `J.N. Bhasin`, `Sassanian`), epoch-offset modes (`Babylonian ETPSC`, `Babylonian Huber`, `Babylonian Kugler 1/2/3`, `De Luce`, `Hipparchos`), fixed-star anchor modes (`Aldebaran 15 Tau`, `True/SS Citra`, `True/SS Revati`, `True Mula`, `True Pushya`, `Surya Siddhanta`, `Ushashashi`), `Galactic Center 0 Sagittarius`, Krishnamurti/Newcomb, Raman, Yukteshwar, UT wrapper behavior, null-output rejection, and unknown-model rejection are covered.
- The stricter adversarial validation pass surfaced and fixed two concrete API-contract bugs: `jme_set_astro_models()` now accepts canonical key/value tokens for bias/nutation/obliquity/precession/sidereal-time/Delta-T model selection instead of only shorthand tokens for several families, and `jme_house_pos()` now returns `NaN` on null input / unavailable house-system error instead of a silent numeric zero.
- The extra public surface beyond the Swiss mapping now has a dedicated adversarial suite (`test_extra_adversarial`) that stress-tests the 92 extra functions with randomized helpers, malformed/null input rejection, repeated determinism checks, production-like analytical sweeps, raw JPL no-kernel error-path checks, and multithreaded read-only execution.
- The 112 unique mapped `jme_*` functions behind the 106 Swiss-reference rows now have an additional adversarial validation target. The pass adds randomized angle/coordinate round-trips, boundary calendar checks, malformed fixed-star strings including UTF-8 input, null/error contract checks, production-like date/location/body workflows, deterministic repeated calculations, model-state serialization/deserialization checks, and read-only multithreaded calls.
- The full public `jme_*` surface now has a 204-function direct-test gate in `tests/test_symbol_coverage.ps1`, and the non-Swiss public surface is covered by `test_extra_adversarial` with calendar fuzzing, matrix/state round-trips, astrometry model checks, analytical state-provider sweeps, raw JPL no-kernel contracts, and read-only multithreaded calls.
- The adversarial pass found and fixed three API robustness issues: key/value model tokens such as `NUT=IAU2000B` now parse consistently with serialized `jme_get_astro_models` output; unknown body names now return deterministic `"Unknown"` instead of a null string; invalid `jme_house_pos` input now returns `NaN` instead of ambiguous zero.
- `jme_fixstar*` is closed for the JME-native fixed-star contract: a generated 9,096-entry Bright Star/Yale-derived catalog is used for broad coverage; common-name aliases plus HR/HD/SAO/catalog-name lookup are supported; J2000 true-position coordinates, proper-motion speed output, radians and sidereal behavior, UT wrappers, alternate entry points, magnitude lookup, catalog endpoints, and invalid-input paths are covered. Exact Swiss catalog-name and black-box parity are still not claimed.
- The analytical validation suite also checks direct public state production for representative Moshier, VSOP87, and Meeus bodies at J2000 so the stack remains callable as state providers, not just through derived-element paths.
- `jme_houses_ex2` and `jme_houses_armc_ex2` compute finite cusp and angle speeds by central difference instead of returning zero arrays.
- `jme_pheno` and `jme_pheno_ut` return phase angle, illuminated fraction, elongation, apparent diameter, magnitude, internal distance fields, light-time, apparent radius, phase defect, and bright-limb position angle.
- `jme_rise_trans` supports refined rise, set, meridian transit, anti-meridian transit, civil/nautical/astronomical twilight horizons, disc-center horizon handling, and no-refraction mode. The rise/transit altitude path now uses the public topocentric equatorial calculation pipeline directly, avoiding mixed-frame rectangular corrections. Contract tests independently recompute returned altitude and hour-angle roots for Sun, Moon, Mars, and Sirius plus invalid/no-event paths.
- Solar, lunar, and heliocentric longitude crossings plus lunar node crossings refine roots instead of returning coarse scan midpoints.
- Lunar node/apside helpers return bounded lunar mean node, true node, mean apogee, and focal-point longitude contracts.
- Orbital-element derivation now returns a fuller derived set from the current state vector, including eccentric anomaly, mean anomaly, mean motion, period, perihelion/aphelion distance, and related longitudes.
- Solar eclipse global/local search, geographic circumstance, and local circumstance entry points now return native geometric results. Global classification now uses Moon-shadow cone versus Earth-sphere geometry so hybrid and central/noncentral behavior are represented natively.
- Eclipse, occultation, and heliacal functions now have native geometry/visibility paths. Heliacal support is closed for the documented JME-native linear arcus-visionis contract; it does not claim real-atmosphere observer photometry beyond that contract.

Known limits:

- Astro model selection has real canonical state and is consumed by reduction paths. Frame bias now has none plus IAU 2000/2006 paths, obliquity has IAU 1980 plus corrected IAU 2006/P03 polynomial coverage, precession has IAU 1976 and P03/IAU 2006-family matrix paths, nutation has closed IAU 1980 plus IAU 2000B paths with explicit IAU 2000A rejection, and sidereal time has IAU 1976/default plus IAU 2006 paths. Remaining model-specific formula gaps are future-extension items rather than silent approximations in the current public contract.
- Solar eclipse search/circumstance behavior is closed for the current JME-native contract. The suite covers total, annular, hybrid, and partial global known-value regressions, visible local circumstance/search cases, backward global search, ordered outer contacts, centrality classification, non-eclipse rejection, and null-output/geopos rejection. The current API intentionally does not overclaim second/third contacts where the solver returns zero.
- Solar partial-only global circumstances no longer report negative magnitude from geocentric apparent separation; `jme_sol_eclipse_where()` now derives the circumstance fields at the returned shadow-surface location, and 2022-10-25 is covered as a partial solar known-value regression.
- Global lunar eclipse search/circumstance behavior is closed for JME-native behavior: total, partial, and penumbral eclipse regressions are covered against external known-value maxima/magnitudes, backward search works, non-eclipse instants reject, and null-output paths are tested.
- Lunar occultation locality/contact/search are closed for the current JME-native Mars occultation contract: the 2022-12-08 Mars occultation is covered globally and locally for Los Angeles, with positive circumstances, ordered outer contacts, and null required-output/geopos rejection.
- Solar eclipse, lunar eclipse, and lunar occultation public entry points now have direct null required-output/geopos rejection tests. This is input-contract hardening, not exact contact/locality closure.
- Sunshine is implemented for date-aware `jme_houses`/`jme_houses_ex`; ARMC Sunshine is also available when the caller supplies Sun declination in `ascmc[9]`.
- Gauquelin sector now branches by method: ecliptic longitude/latitude semiarc, longitude-only semiarc, disk-center rise/set, and refracted disk-center rise/set. It returns fractional sectors, but it is not yet certified as full reference-method parity.
- Heliacal visibility is closed for the current JME-native contract using local twilight, Sun/body altitude, arcus visionis, elongation, apparent magnitude, and an explicit arcus-visionis threshold model. The contract uses `required_arcus = 10.50 + 1.40 * visual_magnitude` and `limiting_magnitude = (arcus - 10.50) / 1.40`; event search evaluates twilight candidates at the required solar depression, revalidates the returned event through `jme_heliacal_pheno_ut`, and rejects unsupported body IDs and null required outputs.
- Heliacal body selection is now explicit for array-output APIs: unsupported IDs, fractional IDs, node bodies, and null required outputs return errors instead of silently changing the request to Venus. Scalar helpers may still use Venus only when the caller omits `dat_hel` because they return one scalar value and use an internal default.
- Physical phenomena provide the standard first five phenomenon fields plus extended geometric fields; rotation-axis and central-meridian style physical ephemeris attributes remain open.

## Remaining Gaps

- `jme_lun_occult_where`, `jme_lun_occult_when_loc`, and `jme_lun_occult_when_glob` are closed for the current Mars occultation contract. Additional star/planet catalog breadth can still be added, but these mapped rows are no longer open under the current JME-native closure rule.
- `jme_heliacal_ut`, `jme_heliacal_pheno_ut`, `jme_heliacal_angle`, `jme_topo_arcus_visionis`, and `jme_vis_limit_mag` are closed for the JME-native visibility contract with explicit required-arcus and limiting-magnitude semantics, body sweeps, event revalidation, scalar-helper consistency, and invalid-input coverage.
- Eclipse coverage is materially stronger: solar eclipse global/local behavior, lunar occultation, global lunar eclipse search/circumstance behavior, and local lunar eclipse visibility behavior are closed for the current JME-native contract.
- `jme_lun_eclipse_when_loc` now intersects the global lunar eclipse phase window with the local Moon-above-horizon interval, clips local-visible contacts, reports visible duration, supports forward/backward search, and rejects hidden observer cases. It still intentionally uses global geocentric shadow contacts clipped to local visibility rather than claiming a separate topocentric lunar-shadow model.
- Physical phenomena are still partial: phase/elongation/diameter/magnitude, distance, light-time, apparent-radius, phase-defect, and bright-limb position-angle fields are implemented, but full rotational physical ephemeris outputs remain open.
- Gauquelin support is stronger but still partial: `jme_gauquelin_sector` supports methods 0 through 3 with fractional sectors; polar-region semiarc edge behavior and independent known-value certification remain open.
- Sunshine support is stronger but still partial: date-aware Sunshine is implemented, and ARMC Sunshine works with caller-supplied Sun declination in `ascmc[9]`; independent reference-value validation remains open.
- Broader independent known-value validation remains a model-certification task for dense analytical fallback edge cases.
- Mixed fallback path validation now covers the selected high-level no-kernel stack for representative planets and Moon across multiple dates. Broader all-body/all-date certification across VSOP87, Moshier, ELP2000, and Meeus remains model-certification work.

## Moshier

Status: direct public API contract closed; broader model certification remains separately tracked.

What is real today:

- Native source material for the Moshier planetary tables and driver is present in the repo.
- A public callable wrapper exists for planetary state generation.
- The engine is the first no-kernel analytical fallback for supported planet bodies in `jme_calc`.

Model-wide claims not made by this direct API closure:

- full exact-parity certification of Moshier output against an independently defined project contract
- fully documented frame/unit/date-range semantics for every returned component
- closed-out provenance/licensing and production-source decisions for all imported table files

Implemented/callable:

| Function | File | Current behavior |
|---|---|---|
| `jme_moshier_planet_state` | `src/moshier.c` | Calls Moshier `gplan()` tables for Mercury, Venus, Earth, Mars, Jupiter, Saturn, Uranus, Neptune, and Pluto. Converts the Moshier longitude/latitude/radius output to heliocentric ecliptic rectangular AU/AU-day state. |

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
- The public contract tests supported-body success, unsupported-body rejection, null-output rejection, finite AU/day state output, and non-zero velocity shape.
- The high-level fallback regression verifies that `jme_calc` uses direct Moshier planet output first when no JPL kernel is open, and keeps the selected engine's velocity components instead of recomputing them from a different fallback engine.
- The broader analytical validation suite checks derived orbital-element plausibility against JPL SSD approximate-element tables for supported analytical engines. This is model-scope validation, not a zero-error physical truth claim.

Known remaining Moshier work:

- verify provenance and license notes for every imported Moshier table/source file
- expand known-value tests per body beyond the current representative element and fallback-order tests
- add date-range/domain documentation
- integrate lunar Moshier latitude/radius tables if they are intended as a lunar fallback
- decide whether `src/moshier_full.c` is transitional source material or production source

Exact resume point for broader Moshier certification:

- keep `jme_moshier_planet_state` closed under the public API contract
- do not call the whole Moshier theory “zero-error precision-certified”
- next exact-closeout work is dense per-body known-value validation, date-range documentation, optional lunar-table integration, and clear production-source decisions

## VSOP87

Status: direct public API contract closed; broader VSOP variant certification remains separately tracked.

What is real today:

- The repo contains VSOP87 source material and a public callable wrapper.
- `jme_vsop87_planet_state` is implemented for the major supported planets.
- The analytical validation suite already includes direct VSOP87A J2000 known-value checks for the supported planets.

Model-wide claims not made by this direct API closure:

- full exact-parity closure for all VSOP87 variants, frames, derivatives, and production-policy decisions
- closed-out semantic documentation for variant selection and public output contract

Implemented/callable:

| Function | File | Current behavior |
|---|---|---|
| `jme_vsop87_planet_state` | `src/vsop87.c` | Uses VSOP87A full-series rectangular functions for Mercury, Venus, Earth, Mars, Jupiter, Saturn, Uranus, and Neptune, and returns J2000 heliocentric ecliptic rectangular AU/AU-day state. Unsupported bodies return `JME_ERR`; there is no hidden Meeus fallback inside the direct VSOP87 API. |

Source material currently present:

- `src/vsop87.c`
- `src/vsop87.h`
- `src/vsop87a_full.c`
- `src/vsop87d_full.c`

Current validation status:

- `tests/test_analytical_validation.c` validates a VSOP87A J2000 reference set for Mercury, Venus, Earth, Mars, Jupiter, Saturn, Uranus, and Neptune.
- The same suite also checks non-zero finite derivatives for the public output.
- `tests/test_extra_contract.c` adds multiple-date finite-state coverage and invalid-input rejection.
- `jme_calc` uses VSOP87 only after the Moshier planet path rejects or fails; direct VSOP87 remains independently callable and rejects unsupported public bodies instead of silently switching to Meeus.
- This closes the direct public API contract. It does not claim that every possible VSOP variant, date range, or comparison against every JPL kernel is zero-error.

Known remaining VSOP87 work:

- confirm whether VSOP87A and VSOP87D are both required in production or only one is canonical
- expand known-value tests beyond the current VSOP87A/J2000 validation case
- document exact VSOP87 variant, units, frame, origin, and time argument
- Pluto, Moon, asteroids, and comets are explicitly excluded from the direct VSOP87 public contract and return `JME_ERR`; Earth-Moon barycenter and Moon remain internal helper paths, not public supported direct-engine bodies.
- expand velocity validation beyond central-difference shape and representative finite/non-zero checks
- expand high-level fallback tests to force VSOP87-selected branches if a future body/policy path exposes one; the current public planet path selects Moshier first for all bodies supported by Moshier

Exact resume point for broader VSOP87 certification:

- keep `jme_vsop87_planet_state` closed under the public API contract
- treat the current state as materially validated for supported bodies, J2000 known values, multiple-date finite output, and invalid-input rejection
- do not claim all VSOP variants or all-date zero-error certification

## ELP2000

Status: direct public API contract closed; broader lunar model certification remains separately tracked.

What is real today:

- The ELP2000-82B source tree is present in the repo.
- `jme_elp2000_moon_state` is callable and returns a Moon state from the imported source.
- The wrapper converts position units into the project API surface and exposes a non-zero derived velocity.
- The high-level no-kernel Moon fallback uses ELP2000 as the first lunar state source and combines it with the selected analytical Earth heliocentric state.

Model-wide claims not made by this direct API closure:

- full precision certification across date ranges
- final output-contract closure for frame, epoch, units, and derivative semantics
- optional lunar Moshier integration policy, if the project decides Moshier Moon tables should become a production lunar fallback

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
- The current source variant is validated against external reference cases and the wrapper is contract-closed for finite AU/AU-day output plus null rejection.
- The high-level fallback regression checks that `jme_calc` Moon heliocentric output equals direct ELP2000 geocentric Moon plus direct Moshier Earth state across multiple dates when no JPL kernel is open.
- This does not claim zero-error lunar truth across all dates and all ELP source variants.

Known remaining ELP2000 work:

- define output contract: geocentric/ecliptic frame, units, epoch, and velocity behavior
- expand known-value lunar tests across date ranges and source variants
- validate all correction terms intended for production use
- document date range and expected numerical precision
- decide whether to add Moshier Moon tables as a production fallback between ELP2000 and Meeus

Exact resume point for broader ELP2000 certification:

- keep `jme_elp2000_moon_state` closed under the public API contract
- do not mark the whole ELP2000 theory as all-date zero-error certified
- next exact-closeout work is broader lunar known-value validation, explicit frame/unit/velocity contract definition, and optional Moshier Moon production-policy decisions

## Meeus

Status: implemented as bounded analytical fallback/utilities, with certification work still required.

What is real today:

- Meeus-style analytical state helpers for Sun, Moon, and planets are implemented and callable.
- `jme_meeus_planet_state` now uses time-varying heliocentric mean elements plus a Newton-solved Kepler equation for Mercury, Venus, Earth, Mars, Jupiter, Saturn, Uranus, Neptune, and Pluto; unsupported bodies return `JME_ERR`.
- The Moon helper now consumes the auxiliary longitude/latitude terms already present in the routine instead of leaving those terms unused.
- Meeus-style utility behavior also exists in adjacent time, coordinate, refraction, and search helpers across the codebase.
- Meeus is therefore not just conceptual; it is part of the actual low-weight analytical toolbox in the repo.

Model-wide claims not made by this direct API closure:

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
- `jme_calc` reaches Meeus only after the higher-priority analytical engine for that body fails: after Moshier and VSOP87 for planets, and after ELP2000 for the Moon.
- The repo currently proves that Meeus-style code exists, is callable, and returns finite state vectors for representative cases; it does not yet prove that every Meeus-derived helper is production-certified at the exactness standard you want.

Known remaining Meeus work:

- add source-specific citations/provenance per formula family
- add independent known-value tests for Sun, Moon, planets, time, refraction, and rise/set helpers
- document expected precision and date ranges per formula
- document which Meeus formulas are production fallbacks and which are only support utilities
- validate velocity outputs and frame semantics for every state-returning function
- complete eclipse and physical phenomena formulas only if they meet project accuracy requirements

Exact resume point for Meeus:

- keep the Sun/Moon/planet state helpers as implemented analytical support
- treat the broader Meeus presence as real but still requiring dedicated certification
- next exact-closeout work is formula-family provenance, per-domain known-value validation, and clearer support-utility documentation

## High-Level Public API Status

The project currently has all 204 public `jme_*` functions declared and defined. Some functions are complete utilities or complete JPL-boundary functions. Some are partial high-level algorithms. Some are explicit error-returning contracts.

Examples of implemented-but-not-yet-complete high-level areas:

- `jme_elp2000_moon_state`
- `jme_sol_eclipse_where`
- `jme_sol_eclipse_how` exact circumstances
- `jme_lun_occult_where`
- `jme_lun_occult_when_loc`
- `jme_lun_occult_when_glob`
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

- normal Linux build: `9/9` tests passed
- CALCEPH-enabled Linux runtime subset with real `de440s.bsp`, text constants, binary PCK orientation, version, and angular-momentum fixture files: `3/3` tests passed for `jpl_runtime`, `analytical_validation`, and `extra_contract`
- symbol coverage: `204/204` functions defined, `462` constants tracked

## Next Work Order

Recommended continuation order:

1. Expand dense known-value validation for Moshier, VSOP87, ELP2000, and Meeus beyond the current representative cases.
2. Decide whether Moshier Moon tables should become a production lunar fallback, then integrate and test them if yes.
3. Add explicit frame/unit/date-range precision documentation for each analytical family.
4. Continue closing the remaining Swiss-reference rows: full numeric ayanamsa model breadth.
5. Update `docs/API_TRACKING.md`, `docs/REFERENCE_FUNCTION_COVERAGE.md`, and this file after each completed family.
