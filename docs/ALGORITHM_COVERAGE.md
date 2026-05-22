# Algorithm Coverage Plan

This project target is broad astronomy and astrology coverage, with the JPL/CALCEPH backend as the high-precision primary path and independently sourced analytical modules as additional paths.

Coverage must mean implemented, tested, and documented. A name in a header is not considered completed coverage.

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
- sidereal ayanamsa models
- asteroid/comet catalog loading beyond direct kernel target IDs
- dense all-body/all-date certification for high-level houses, eclipses, rise/set/transit, fixed stars, heliacal visibility, and physical ephemerides beyond their current JME-native closure contracts

## Remaining Non-JPL Work

- broader independent known-value validation for Moshier
- broader variant/frame/unit validation for VSOP87
- broader date-range and contract validation for ELP2000
- formula-family provenance and completeness validation for Meeus-derived helpers
- optional Moshier lunar-table integration if the project requires a Moshier-native Moon fallback between ELP2000 and Meeus

These remaining non-JPL items are model-certification depth, not name-only public API functions. The six direct extra analytical state functions are closed under the public API closure checklist.
