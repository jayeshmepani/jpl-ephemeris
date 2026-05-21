# Provenance

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
| Sidereal time | Independently implemented standard mean sidereal-time expression | Active |
| Backend boundary | CALCEPH API boundary, optional external link | Implemented for the current raw JPL API surface |
| Ephemeris data | NASA/JPL `.bsp` kernels | Not vendored |
| Main ephemeris calculations | Independent JME calculation pipeline with JPL and analytical fallback paths | Implemented, but full reference-parity validation remains partial |
| Analytical fallback | Moshier/public-domain source review completed enough to implement callable analytical paths; broader validation remains | Implemented, but not precision-certified |

## Future Source Log Format

When adding a new algorithm, add an entry:

| Function/Area | Independent source | What was used | Notes |
| --- | --- | --- | --- |

The source log is part of the legal-risk control process. It should explain where the implementation logic came from without copying protected expression.
