# Status And Design

## Current Design Snapshot

The current public C surface is defined by `include/jme/jme.h` and `include/jme/jme_extended.h`: 204 public `jme_*` functions and 462 public `JME_*` constants.

The current `.c` and `.h` source tree contains no Swiss Ephemeris, `swe_`, Astrodienst, or Swiss-author attribution references. Swiss Ephemeris remains only a black-box comparison target in the validation documentation, not an implementation source.

Project ownership and context: this C library is developed and owned by Jayesh Mepani. It is intended as a native ephemeris engine for FFI wrappers and application-level astrology, astronomy, and calendrical software.

Design motivation: the project exists to provide an independently implemented, non-Swiss ephemeris engine with three engine modes: JPL/CALCEPH, Moshier, and VSOP87+ELP2000+Meeus. The public API deliberately exceeds the older 106-row Swiss capability checklist, giving this library a larger project-owned surface for broader astrology and astronomy workflows.

| Area | Current source files |
|---|---|
| Engine/context selection | `src/api.c`, `src/context.c`, `src/context.h` |
| Main calculation pipeline | `src/calc.c` |
| JPL/CALCEPH kernel bridge | `src/jpl.c` |
| Moshier analytical backend | `src/moshier.c`, `src/moshier_full.c`, `src/moshier_moon.c`, `src/moshier/*` |
| VSOP87, ELP2000, Meeus analytical backends | `src/vsop87.c`, `src/vsop87a_full.c`, `src/vsop87d_full.c`, `src/elp2000.c`, `src/elp2000/*`, `src/meeus.c` |
| Eclipse, occultation, rise/set, heliacal, nodal/apsidal events | `src/events.c` |
| Astrometry, coordinates, houses, sidereal/time, fixed stars, metadata | `src/astrometry.c`, `src/coordinates.c`, `src/houses.c`, `src/sidereal.c`, `src/time.c`, `src/fixstar.c`, `src/metadata.c` |
| Calendar and angle utilities | `src/julian.c`, `src/angle.c` |

## Implementation Status

This file is the continuation checkpoint for this repository.

It is intentionally written as engineering status, not release marketing. A symbol being declared and defined means it is callable. It does not automatically mean the full astronomical or reference-compatible behavior is complete.

## Canonical Inventories

The canonical generated inventory is `docs/API_REFERENCE.md`.

Current verified counts from `tests/test_symbol_coverage.ps1`:

| Inventory | Count | Source of truth |
|---|---:|---|
| Public `jme_*` functions | 204 | `docs/API_REFERENCE.md`, `include/jme/jme.h`, `include/jme/jme_extended.h` |
| Public `JME_*` constants | 462 | `docs/API_REFERENCE.md`, `include/jme/jme.h`, `include/jme/jme_extended.h` |

`docs/API_REFERENCE.md` is the full 204-function and 462-constant inventory. This file avoids duplicating that complete list so there is only one source of truth.

`docs/VALIDATION_AND_COVERAGE.md` is the live behavior-closure checklist and records function-family closure status.

`docs/VALIDATION_AND_COVERAGE.md` also answers "how is this function currently validated?" It assigns every public function to one of the current verification modes: external web numeric, embedded reference numeric, independent formula/identity, kernel runtime/error contract, or API/behavior contract only.

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
$env:JME_TEST_JPL_KERNEL = "E:\jme-kernels\de440s.bsp"
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
JME_TEST_JPL_KERNEL=/tmp/jme-kernels/de440s.bsp \
JME_TEST_JPL_CONSTANT_KERNEL=/path/to/jpl-ephemeris/tests/fixtures/jpl_constants.tpc \
JME_TEST_JPL_ORIENTATION_KERNEL=/tmp/jme-kernels/moon_pa_de440_200625.bpc \
JME_TEST_JPL_VERSION_KERNEL=/tmp/jme-calceph-src/calceph-calceph_4_0_5/examples/example1.dat \
JME_TEST_JPL_ROTANGMOM_KERNEL=/tmp/jme-calceph-src/calceph-calceph_4_0_5/examples/example2_rotangmom.dat \
ctest --test-dir build-calceph-runtime -R 'jpl_runtime|extra_contract|analytical_validation' --output-on-failure
```

Result: `3/3` tests passed: `jpl_runtime`, `extra_contract`, and `analytical_validation`. This keeps all 49 extra raw JPL/CALCEPH functions closed in `docs/VALIDATION_AND_COVERAGE.md`.

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

The complete 462-constant inventory is maintained in `docs/API_REFERENCE.md`.

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
$env:JME_TEST_JPL_KERNEL = "E:\jme-kernels\de440s.bsp"
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
5. Update `docs/API_REFERENCE.md`, `docs/VALIDATION_AND_COVERAGE.md`, and this file after each completed family.

---

## Engine Differential Matrix

This harness generates a forensic comparison matrix for the public `jme_*` surface across exactly three engine selections:

- `ENGINE=JPL`
- `ENGINE=MOSHIER`
- `ENGINE=VSOP_ELP_MEEUS`

The purpose is not to force equality. The purpose is to preserve exact call I/O per engine and make differences auditable.

## How It Works

The matrix is produced by two pieces:

- `jme_call_probe`
  A C executable that performs one function call under one selected engine and emits a lossless JSON record.
- `tools/run_engine_matrix.sh`
  A runner that discovers the public function inventory from the same header surface used by symbol coverage, executes the probe across all three engines, stores raw records, hashes them, and writes summary artifacts.

The current output directory is:

- `build/differential_engine_matrix/`

Files produced:

- `engine_matrix.full.jsonl`
- `engine_matrix.summary.csv`
- `engine_matrix.diff.md`
- `engine_matrix.raw/<function>/<engine>.json`

Each raw per-engine record now also contains exact monotonic wall-clock timing metadata for that specific function call:

- `timing.start.sec`
- `timing.start.nsec`
- `timing.end.sec`
- `timing.end.nsec`
- `timing.elapsed_ns`
- `timing.elapsed_seconds_decimal_string`

## Why Hexfloat And Raw Bytes

Normal decimal formatting can hide real differences.

For floating-point values, the authoritative forms are:

- C99 hex-float text via `%a`
- raw IEEE-754 bytes as hexadecimal

Decimal text is convenience only. It is not the primary comparison form.

Buffers and strings are also captured losslessly:

- full buffer bytes
- visible text when meaningful
- null-terminator presence within capacity

Response time is captured losslessly too. The authoritative timing form is integer nanoseconds from `CLOCK_MONOTONIC`, with start/end timestamps preserved separately.

## Engine Selection

The probe sets the engine through the public selector:

```c
jme_set_astro_models("ENGINE=JPL", 0);
jme_set_astro_models("ENGINE=MOSHIER", 0);
jme_set_astro_models("ENGINE=VSOP_ELP_MEEUS", 0);
```

`ENGINE=JPL` is treated as strict. If CALCEPH or the required kernel path is unavailable, JPL-mode records remain explicit error records instead of silently falling back.

## Inventory Source

The runner derives the public function inventory directly from:

- `include/jme/jme.h`
- `include/jme/jme_extended.h`

The expected counts are:

- `204` public functions
- `462` constants

If the discovered counts differ, the runner stops as an infrastructure failure.

## Adapters

Not all public functions share a signature, so the probe uses per-function adapters.

Each adapter is responsible for:

- deterministic fixture choice
- engine reset/setup
- exact out-parameter capture
- buffer sentinel initialization
- status classification

When a function is not yet safely callable by the probe, the row is still emitted with:

- `SKIPPED_WITH_REASON`

This keeps the matrix inventory-complete even while adapter coverage expands.

## Running

Build and run the focused matrix:

```bash
cmake --build build --target jme_call_probe
./tools/run_engine_matrix.sh
```

Or through CTest:

```bash
ctest --test-dir build -R engine_matrix --output-on-failure
```

## Interpreting Differences

The summary CSV reports per-function engine status plus raw-output hashes and pairwise relations.

It also includes per-engine elapsed time columns:

- `jpl_elapsed_ns`
- `moshier_elapsed_ns`
- `vsop_elp_meeus_elapsed_ns`

Typical relations:

- `IDENTICAL_RAW`
- `ERROR_ONE_SIDE`
- `ERROR_ALL`
- `SKIPPED`
- `DIFFERENT_STRUCTURE`

The current runner preserves raw outputs first. Numeric-difference extraction can be extended later without changing the lossless per-engine records.

## Real JPL Kernels

The runner uses `$JME_TEST_JPL_KERNEL` when a real BSP kernel is needed. Kernel files are intentionally not vendored in the source tree; download `de440s.bsp`, `de440.bsp`, or `de441.bsp` from the project kernel release or from the upstream JPL source before running JPL/CALCEPH runtime tests.

Real JPL-mode success requires a CALCEPH-enabled build. A present kernel file alone is not enough if the library was built without CALCEPH.

---

## Ephemeris Engine Knowledge Base

This document captures the design knowledge discussed while shaping this project.

The core question was:

> If JPL/CALCEPH is the primary high-precision backend, what do Meeus, Moshier, VSOP87, ELP2000, SOFA/ERFA, IERS, and catalog data contribute, and how do they fit together?

This file is meant to help future contributors understand the architecture without rediscovering the same decisions.

## Short Answer

For this project, the clean architecture is:

```text
Primary precision layer:
JPL kernels through CALCEPH

Analytical fallback layer:
VSOP87 + ELP2000/ELP82/ELPMPP02 + Meeus + Moshier

Reference-frame/time layer:
SOFA/ERFA + IAU models + IERS/EOP data

Catalog layer:
fixed stars + asteroids + comets + satellites

Astrology layer:
ayanamsa + houses + rise/set + eclipses + calendar/ritual rules
```

The most important conclusion:

```text
Moshier < VSOP87 + ELP2000 + Meeus
VSOP87 + ELP2000 + Meeus < full astronomy/astrology engine
```

Moshier is useful, but it is not the whole universe. VSOP87, ELP, and Meeus together form a broader analytical fallback base than Moshier alone. Even that set is not enough for a complete astronomy/astrology engine.

## What Is Meeus?

Meeus means **Jean Meeus**, the author of *Astronomical Algorithms*.

In astronomy software, "Meeus" usually means formula-based algorithms from that book or that style of computational astronomy. These are compact, practical formulas for many common astronomical calculations.

Typical Meeus-style areas:

- Julian Day conversion
- Gregorian and Julian calendar conversion
- date validation
- day of week
- day of year
- Delta-T estimates
- mean sidereal time
- apparent sidereal time
- solar position
- lunar position
- approximate planetary positions
- nutation
- obliquity
- precession helpers
- rise, set, and transit
- lunar phases
- elongation
- illumination
- angular separation
- position angle
- eclipses
- atmospheric refraction

In this project, Meeus is best treated as:

```text
calendar/time/general astronomy formula layer
utility layer
lightweight fallback layer
```

Meeus is not the same as JPL. Meeus formulas are analytical approximations and utility algorithms. JPL kernels are numerical ephemeris data produced for high-precision planetary/lunar positions.

## JPL/CALCEPH vs Meeus

| Area | JPL/CALCEPH | Meeus |
|---|---|---|
| Nature | Numerical ephemeris kernel access | Formula-based algorithms |
| Data | `.bsp` kernel files such as DE440/DE441 | No large kernel required |
| Precision role | Primary high-precision backend | Utility/fallback/helper layer |
| Strength | Planetary/lunar state vectors from JPL kernels | Calendar, time, utility, approximate astronomy formulas |
| Weakness | Requires external kernel data and CALCEPH build | Not as precise as JPL for high-grade ephemeris positions |

Project decision:

```text
JPL/CALCEPH is the primary path.
Meeus is suitable for calendar/time/general utilities and fallback formulas.
Meeus is not treated as a replacement for JPL precision.
```

## What Is Moshier?

Moshier refers to Steve Moshier's public-domain astronomical code and tables.

In practical terms, Moshier is closer to a compact standalone ephemeris engine than a single formula family. It includes planetary tables and computational routines that can produce useful ephemeris results without JPL kernel files.

In this project, Moshier is useful as:

```text
compact analytical fallback engine
cross-check layer
non-JPL backup path
```

Moshier is not exactly equivalent to Meeus, VSOP87, or ELP2000. It overlaps with them, but its role is different.

## Moshier Compared To Meeus, VSOP87, And ELP

There is no single perfect equivalent to Moshier.

Best mapping:

| Moshier role | Closest equivalent |
|---|---|
| planetary positions | VSOP87 |
| lunar position | ELP2000 / ELP82 / other lunar theory |
| calendar/time/general astronomy formulas | Meeus |
| compact standalone ephemeris engine | Moshier itself |
| high-precision numerical source | JPL kernels through CALCEPH |

So the practical analogy is:

```text
Moshier ~= planetary theory + lunar theory + reduction formulas + utility astronomy code
```

But the more accurate project comparison is:

```text
Moshier < VSOP87 + ELP2000 + Meeus
```

Why?

- VSOP87 can be stronger and more complete for planetary analytical theory.
- ELP2000 can be stronger and more complete for lunar analytical theory.
- Meeus covers many utility formulas that Moshier may not expose in the same way.
- Together, VSOP87 + ELP2000 + Meeus can cover a broader analytical stack than Moshier alone.

But Moshier is more integrated than each one individually:

```text
Moshier > Meeus alone for ephemeris-engine usage
Moshier > VSOP87 alone for full practical-engine usage
Moshier > ELP alone for full practical-engine usage
```

Clean summary:

```text
Moshier < VSOP87 + ELP2000 + Meeus
Moshier > VSOP87 alone
Moshier > ELP alone
Moshier > Meeus alone
```

## VSOP87 Analytical Backend

VSOP87 is a planetary analytical theory.

It is mainly for planetary positions. It is not a complete astronomy engine by itself.

Typical coverage:

- Mercury
- Venus
- Earth
- Mars
- Jupiter
- Saturn
- Uranus
- Neptune

Depending on variant/source, VSOP may expose:

- heliocentric rectangular coordinates
- heliocentric spherical coordinates
- barycentric forms
- different epochs/frames

Project role:

```text
VSOP87 is the main analytical fallback for planets.
```

VSOP87 does not solve:

- Moon theory
- calendar/time systems
- Delta-T
- fixed stars
- asteroids/comets
- houses
- ayanamsa
- eclipse circumstances
- rise/set with observer conditions
- full coordinate-frame reduction pipeline

## ELP2000 / ELP82 / ELPMPP02

ELP is a lunar analytical theory family.

It is mainly for Moon position. It is not a complete astronomy engine by itself.

Project role:

```text
ELP is the main analytical fallback for the Moon.
```

ELP helps fill a gap that VSOP87 does not cover, because VSOP is planetary and does not provide a complete high-grade lunar theory.

Known ELP-related roles:

- lunar longitude
- lunar latitude
- lunar distance/radius
- lunar perturbation terms
- possible corrections depending on the selected ELP variant

ELP does not solve:

- planetary positions
- calendar/time systems
- fixed stars
- asteroids/comets
- houses
- ayanamsa
- calendar and ritual rules by itself

## Does VSOP87 + ELP + Meeus Cover All Astronomy?

No.

VSOP87 + ELP + Meeus covers a strong classical analytical core:

```text
VSOP87  = planets
ELP     = Moon
Meeus   = calendar/time/general formulas
```

But this is still not a complete astronomy engine and not a complete astrology engine.

It misses several important layers:

- high-precision numerical ephemeris data
- complete reference-frame handling
- high-precision Earth orientation
- fixed-star catalogs
- asteroid/comet catalogs
- artificial satellite propagation
- detailed topocentric observation modeling
- atmospheric refraction policy
- eclipse/occultation geometry
- rise/set/transit edge cases
- house-system algorithms
- sidereal ayanamsa models
- calendar and ritual rule layers

So:

```text
VSOP87 + ELP + Meeus < full astronomy engine
VSOP87 + ELP + Meeus < full astrology engine
```

## The Fourth Member And Beyond

To fill the set properly, the analytical stack needs more than VSOP87 + ELP + Meeus.

Better minimum serious set:

```text
JPL/CALCEPH + VSOP87 + ELP2000 + Meeus + Moshier + SOFA/ERFA + IERS
```

For astrology, add:

```text
fixed-star catalogs
small-body catalogs
house algorithms
ayanamsa models
rise/set/transit logic
eclipse/visibility logic
calendar and ritual rules
```

## SOFA / ERFA / IAU Layer

SOFA is the IAU Standards of Fundamental Astronomy library.

ERFA is an open-source derivative/reimplementation used by projects that need SOFA-style functionality with friendlier licensing.

This layer covers high-quality standards for:

- precession
- nutation
- frame bias
- Earth rotation angle
- sidereal time
- time-scale conversions
- coordinate transformations
- ICRS/FK5-style reference frames
- celestial-to-terrestrial transformation pieces

Project role:

```text
SOFA/ERFA-style logic is the preferred model for reference frames, time scales, and Earth orientation transformations.
```

This layer is critical because JPL gives positions in a kernel/reference-frame context, but astrology/observer output usually needs transformed apparent/topocentric/sidereal values.

JPL answers:

```text
Where is the body in the ephemeris frame?
```

SOFA/ERFA/IAU-style reductions help answer:

```text
What is the apparent/topocentric/ecliptic/equatorial position for an observer and date?
```

## IERS / EOP Layer

IERS means International Earth Rotation and Reference Systems Service.

EOP means Earth Orientation Parameters.

This layer covers:

- UT1-UTC
- polar motion
- Earth orientation corrections
- high-precision terrestrial/celestial frame linkage

Project role:

```text
IERS/EOP data is the high-precision path for observer-based calculations.
```

Without EOP, many applications can still work at ordinary astrology precision, but a serious astronomy engine should clearly document whether it uses EOP or a simplified policy.

## Catalog Layer

JPL kernels, VSOP, ELP, Meeus, and Moshier do not automatically cover every object users may ask for.

Additional catalog layers are needed.

### Fixed Stars

Needed sources may include:

- Gaia
- Hipparcos
- Yale Bright Star Catalog
- other curated star catalogs

Fixed-star support needs:

- star identifier/name lookup
- RA/Dec
- proper motion
- parallax
- radial velocity when available
- epoch handling
- magnitude
- coordinate-frame handling

### Asteroids, Comets, Minor Planets

Needed sources may include:

- JPL Horizons data
- MPC orbital elements
- SPK kernels for selected bodies

Small-body support needs:

- object database
- orbital elements or kernel target IDs
- epoch handling
- perturbation policy
- fallback policy when no SPK kernel exists

### Artificial Satellites

Needed model:

- SGP4

Needed data:

- TLE records

This is a separate problem from planetary ephemerides. JPL DE kernels do not replace SGP4/TLE workflows for Earth satellites.

## Observer / Topocentric Layer

For user-facing astronomy and astrology, geocentric positions are often not enough.

Needed pieces:

- observer longitude
- observer latitude
- observer altitude
- geodetic Earth model
- parallax correction
- topocentric coordinate conversion
- horizon coordinate conversion
- atmospheric refraction
- apparent disc radius policy for Sun/Moon/planets

This layer is necessary for:

- rise
- set
- transit
- twilight
- eclipse visibility
- occultations
- heliacal visibility
- local calendar calculations

## Refraction Layer

Atmospheric refraction affects apparent altitude, especially near the horizon.

Possible formula families:

- Bennett
- Saemundsson
- NOAA-style refraction formulas
- custom pressure/temperature-aware models

Project decision should be explicit:

```text
Refraction policy is explicit in public behavior.
Expose pressure, temperature, and true/apparent conversion mode where relevant.
```

## Eclipse And Occultation Geometry

JPL can provide accurate state vectors, but eclipse and occultation outputs require additional geometry.

Needed calculations:

- apparent Sun/Moon radius
- Earth/Moon shadow cones
- umbra/penumbra
- local horizon visibility
- contact times
- maximum eclipse
- central/non-central classification
- total/partial/annular/hybrid classification
- lunar penumbral/partial/total classification
- occultation object apparent size and relative motion

JPL gives the raw positions. Event geometry remains part of the engine.

## Rise, Set, Transit, And Visibility

Rise/set/transit is not just body longitude.

Needed inputs:

- apparent RA/Dec or altitude
- observer location
- horizon height
- refraction
- body disc radius
- topocentric parallax
- event search algorithm
- polar/no-event handling
- date-boundary handling

This is important for astronomy and calendrical applications.

## Astrology Layer

Astrology requires additional interpretation and calculation layers above raw astronomy.

Important astrology-specific pieces:

- tropical/sidereal zodiac policy
- ayanamsa models
- house systems
- cusps
- ascendant/midheaven
- planetary speed
- retrograde/stationary logic
- combustion logic if Vedic-style output is desired
- aspects if supported
- calendar-rule components
- muhurta periods
- festival/vrata rules if the engine includes calendar logic

These are not provided directly by VSOP, ELP, Meeus, Moshier, or JPL.

## Ayanamsa Layer

Sidereal astrology needs ayanamsa models.

Examples:

- Lahiri
- Raman
- Krishnamurti
- Fagan-Bradley
- Yukteshwar
- True Chitra
- True Revati
- user-defined ayanamsa

Project rule:

```text
Ayanamsa should be explicit and user-selectable.
Tropical and sidereal output are kept as explicit modes.
```

## House-System Layer

House systems are separate from ephemeris position calculation.

Examples:

- Whole Sign
- Equal
- Porphyry
- Placidus
- Koch
- Regiomontanus
- Campanus
- Alcabitius
- Morinus
- Meridian
- Horizontal
- Vehlow Equal
- Gauquelin sectors

JPL does not provide houses. VSOP does not provide houses. ELP does not provide houses. Meeus may provide some relevant astronomy formulas, but house systems are their own astrology/astronomical-coordinate problem.

## Calendar Rule Layer

Calendar rule logic is above ephemeris calculation.

Raw ephemeris provides Sun and Moon positions. Calendar-rule logic derives:

- tithi
- nakshatra
- yoga
- karana
- vara
- paksha
- lunar month
- solar month
- sankranti
- sunrise/sunset based day boundaries
- moonrise/moonset
- rahu kaal
- yama gandam
- gulika
- choghadiya
- hora
- muhurta
- festival rules
- vrata/parana rules

This project can provide astronomy primitives, while application-specific calendar rules belong in a separate rule engine if included.

## Recommended Engine Stack

The recommended architecture:

```text
1. JPL/CALCEPH
   Primary numerical ephemeris backend.

2. VSOP87
   Analytical planetary fallback.

3. ELP2000 / ELP82 / ELPMPP02
   Analytical lunar fallback.

4. Meeus
   Calendar, time, general astronomy, and lightweight fallback formulas.

5. Moshier
   Compact fallback engine and cross-check layer.

6. SOFA/ERFA + IAU models
   Reference frames, precession, nutation, sidereal time, time scales.

7. IERS/EOP
   Earth orientation when high-precision topocentric output is required.

8. Catalogs
   Fixed stars, asteroids, comets, satellites.

9. Observer and event geometry
   Rise/set/transit, eclipses, occultations, heliacal visibility.

10. Astrology layer
    Ayanamsa, houses, calendar rules, electional rules.
```

## Project-Specific Decision

For this project:

```text
JPL/CALCEPH is primary.
VSOP87 + ELP + Meeus form the analytical component fallback stack.
Moshier is an additional compact fallback/cross-check layer.
SOFA/ERFA and IERS-style logic are needed for a serious frame/time layer.
Catalogs and astrology-specific rules are separate layers above ephemeris math.
```

The project should avoid claiming that any one component covers everything.

Correct claims:

```text
JPL/CALCEPH covers primary numerical state vectors.
VSOP87 covers analytical planetary fallback.
ELP covers analytical lunar fallback.
Meeus covers utility and approximate formula fallback.
Moshier covers compact fallback engine behavior.
SOFA/ERFA covers standards-based frame/time transformations.
IERS covers high-precision Earth orientation.
Catalogs cover stars, asteroids, comets, and satellites.
Astrology layers cover houses, ayanamsa, calendar rules, and interpretation rules.
```

Incorrect claims:

```text
Meeus alone covers full astronomy.
Moshier alone covers full astronomy.
VSOP87 + ELP + Meeus covers full astrology.
JPL kernels alone provide houses, ayanamsa, calendar rules, or electional rules.
```

## Completion Philosophy

A component should be marked complete only when all of these are true:

- public API is implemented
- no false success path exists
- source/provenance is documented
- output units are documented
- coordinate frame is documented
- time scale is documented
- valid date/domain range is documented
- known-value tests exist
- invalid-input tests exist
- fallback order is explicit

This keeps the project honest and makes it possible for another contributor to continue from the current state without guessing.

---

## Product Contract

The public product surface is the project-owned `jme_*` API and `JME_*` constants.

This engine is not a wrapper and does not expose historical third-party function names as the primary API. External projects can be studied only as coverage references, not as source or naming authority.

## Product Surface

| Surface | Role |
| --- | --- |
| `jme_*` functions | Public C API |
| `JME_*` constants | Public C constants and enums |
| JPL/CALCEPH-backed calls | Primary numerical ephemeris path |
| Independent analytical modules | Future fallback and cross-check paths |

## Coverage Rule

The goal is an independent engine with feature parity and then feature superset coverage. Function names, constant names, argument shapes, and output structures should belong to this project unless there is a deliberate reason to expose a migration aid in a separate layer.

Every public function has:

- a compiled source definition
- tests for success and failure paths
- real backing calculation or real backend metadata
- no presentation rounding in the C API
- documented data requirements where a kernel or catalog is required

## Current Status

The current implemented callable surface is tracked in `docs/API_REFERENCE.md`.

The old 106-function list is not the public API target. It is a Swiss-reference coverage checklist used to make sure this product covers the same astronomy and astrology capability areas, with project-owned names and behavior.

Current Swiss-reference tracking:

- 106 reference behavior rows mapped to project-owned entry points
- 348 reference constants inventoried
- not all mapped rows are exact-complete yet
- not all inventoried constants are guaranteed Swiss-semantic parity yet

---

## Provenance

This document tracks allowed sources and disallowed sources for the independent implementation.

## Disallowed Implementation Sources

- Astrodienst Swiss Ephemeris C source code.
- Astrodienst Swiss Ephemeris header files as copied repository source.
- Astrodienst comments, documentation prose, generated files, tables, and ephemeris data files.
- Line-by-line translations of restricted implementation code.

## Current Repository Sources

| Area | Source | Status |
| --- | --- | --- |
| Build system | Independently written CMake scaffold | Active |
| Public API | Independently named `jme_*` / `JME_*` API | Active |
| Julian day conversion | Independently implemented standard calendar/JD formula | Active |
| Angle and centisecond utilities | Independently implemented modular arithmetic and sign-preserving rounding | Active |
| Coordinate/refraction utilities | Independently implemented spherical-coordinate transforms and standard refraction helper | Active |
| Metadata utilities | Independently maintained body and house-system name tables | Active |
| Fixed-star catalog | Yale Bright Star Catalog / NASA HEASARC BSC5P-derived public catalog data. Generated `src/fixstar_catalog.inc` contains HR, HD, SAO, catalog alternate name, J2000 RA/Dec, proper motion, and V magnitude fields for 9,096 entries. Common-name aliases in `src/fixstar.c` map to catalog HR identifiers. | Active |
| Ayanamsa source-definable modes | Project fixed-star catalog plus standard epoch and epoch-offset definitions. Numeric support exists for every declared sidereal mode: Lahiri, Fagan-Bradley, user-defined offsets, `J2000`, `J1900`, `B1950`, `Aryabhata`, Babylonian ETPSC/Huber/Kugler 1/2/3, `De Luce`, `Hipparchos`, `J.N. Bhasin`, `Sassanian`, fixed-star anchor modes (`Aldebaran 15 Tau`, `True/SS Citra`, `True/SS Revati`, `True Mula`, `True Pushya`, `Surya Siddhanta Revati 29°50' Pisces`, `Ushashashi`), `Galactic Center 0 Sagittarius`, Krishnamurti/Newcomb, Raman, and Yukteshwar. The fixed anchors use cataloged stars plus published Sgr A* coordinates; Surya Siddhanta/Ushashashi Revati uses the public Burgess/Whitney translation table that lists Revati at longitude `11s 29°50'`; Krishnamurti uses the published KP/Newcomb B1900 formula and zero-epoch/value checks; Raman and Yukteshwar use their published defining year/rate formulas with published examples. These modes are exact to their tracked defining contracts, not broader claims about historical/observational truth. | Active |
| Sidereal time | Independently implemented standard mean sidereal-time expression | Active |
| Backend boundary | CALCEPH API boundary, optional external link | Implemented for the current raw JPL API surface |
| Ephemeris data | NASA/JPL `.bsp` kernels | Not vendored |
| Main ephemeris calculations | Independent JME calculation pipeline with JPL and analytical fallback paths | Implemented; full Swiss/reference parity remains tracked separately from native JME closure |
| Analytical fallback | Moshier/public-domain source review completed enough to implement callable analytical paths; broader validation remains | Direct extra analytical APIs are implemented and contract-tested; model-wide dense precision certification remains separately tracked |
| Heliacal visibility | Independent published/secondary visibility references describing linear arcus-visionis relations for heliacal events, including Schaefer-style visibility literature and Alcyone visibility documentation. Current code uses the explicit relation `required_arcus = 10.50 + 1.40 * visual_magnitude` and its inverse limiting-magnitude expression, revalidates found events through the phenomenon function, and rejects unsupported body/null-output paths. | Active for current JME-native contract |

## Future Source Log Format

When adding a new algorithm, add an entry:

| Function/Area | Independent source | What was used | Notes |
| --- | --- | --- | --- |

The source log is part of the legal-risk control process. It should explain where the implementation logic came from without copying protected expression.

---

## Lossless I/O Contract

This project does not publish approximate production outputs.

### Contract Matrix

| Boundary | Contract |
|---|---|
| Presentation rounding | Numeric outputs are not rounded inside the C API |
| Angle normalization | Normalization occurs only when the documented function contract requires it |
| Backend availability | Missing exact backends produce explicit status/error behavior |
| Output arrays | Array shape, return status, and error buffers remain visible |
| Backend vectors | Raw vectors become public longitude/latitude output only after the complete audited reduction path exists |
| Completion status | Completed behavior is tied to tests |
| Engine fallback | Direct engine APIs keep unsupported-body behavior explicit; fallback selection belongs in the high-level calculation pipeline |

## Incomplete Functions

Incomplete functions return `JME_ERR` and set an error buffer where available.

This is intentional. An explicit error is better than a lossy or approximate result.

## Backend Boundary

CALCEPH may provide raw ephemeris vectors. Public functions expose derived values after the required time-scale handling, frame reduction, apparent/geometric mode handling, sidereal handling, and flag behavior are implemented and tested.

---

## CALCEPH Setup

This project can build without CALCEPH, but real JPL kernel computation requires CALCEPH at build time.

## Windows via vcpkg

```powershell
git clone https://github.com/microsoft/vcpkg.git E:\project\astrology\tools\vcpkg
E:\project\astrology\tools\vcpkg\bootstrap-vcpkg.bat
E:\project\astrology\tools\vcpkg\vcpkg.exe install calceph:x64-windows
```

Configure this project with CALCEPH required:

```powershell
cmake -S . -B build-vcpkg -DCMAKE_TOOLCHAIN_FILE=E:/project/astrology/tools/vcpkg/scripts/buildsystems/vcpkg.cmake -DJME_REQUIRE_CALCEPH=ON
cmake --build build-vcpkg
```

Run the integration test with a JPL SPK kernel:

```powershell
$env:JME_TEST_JPL_KERNEL = "E:\jme-kernels\de440s.bsp"
ctest --test-dir build-vcpkg -C Debug --output-on-failure
```

The local verified setup used CALCEPH `4.0.5` through vcpkg and NASA/JPL `de440s.bsp`.
