# Algorithm Coverage Plan

This project target is broad astronomy and astrology coverage, with the JPL/CALCEPH backend as the high-precision primary path and independently sourced analytical modules as additional paths.

Coverage must mean implemented, tested, and documented. A name in a header is not considered completed coverage.

## Required Families

| Family | Intended role | Current status |
| --- | --- | --- |
| JPL/CALCEPH | Primary numerical ephemeris backend | Raw kernel access, multi-kernel open, rectangular body-state vectors, UTC wrappers, ecliptic state helpers, kernel metadata, derivative-order vectors, name/id lookup, constants access, and orientation/rotation state calls are implemented through CALCEPH when CALCEPH is present at build time |
| Moshier | Lightweight analytical/public-domain fallback components after provenance review | Implemented and callable, but not precision-certified |
| VSOP87 | Planetary analytical theory support after source/license review | Implemented and callable, with J2000 known-value checks for supported planets |
| ELP2000 | Lunar analytical theory support after source/license review | Implemented and callable, with external reference checks, but not full date-range certified |
| Meeus | Calendar, time, coordinate, rise/set, eclipse, and astronomy utility formulas where suitable | Implemented in multiple helper families and analytical fallbacks; planet state now uses Kepler-solved mean elements for Mercury through Pluto, but full helper-family certification is still pending |

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

- light-time, aberration, deflection, precession, nutation, frame bias pipeline
- apparent geocentric and topocentric reduction pipeline
- UTC/TT/TDB time-scale conversion pipeline for direct civil-time calls
- horizontal coordinate conversions
- sidereal ayanamsa models
- topocentric pipeline
- houses
- eclipses
- rise/set/transit
- fixed stars
- heliacal visibility
- physical ephemerides
- asteroid/comet catalog loading

## Remaining Non-JPL Work

- broader independent known-value validation for Moshier
- broader variant/frame/unit validation for VSOP87
- broader date-range and contract validation for ELP2000
- formula-family provenance and completeness validation for Meeus-derived helpers
