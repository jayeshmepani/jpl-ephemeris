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
