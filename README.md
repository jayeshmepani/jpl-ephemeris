# JPL Moshier Ephemeris

Independent C ephemeris engine using JPL kernels through CALCEPH as the primary backend and independently verified Moshier-derived/public-domain algorithms as secondary components where appropriate.

This is not a wrapper around Astrodienst Swiss Ephemeris and does not use Swiss Ephemeris source code, headers, data files, comments, generated files, or implementation tables.

This library is developed and owned by Jayesh Mepani. It was built to replace a prior Swiss Ephemeris dependency with an independently implemented JPL/Moshier/VSOP+ELP+Meeus engine and a project-owned API surface.

## Goal

- Provide an independent ephemeris engine for astrology, astronomy, and calendrical calculations.
- JPL `.bsp` kernels through CALCEPH provide the high-precision numerical ephemeris path.
- Add verified analytical/public-domain components only when provenance is clear.
- Keep the primary public API under the project-owned `jme_*` prefix.
- Preserve lossless I/O contracts: no presentation rounding, no hidden output reshaping, no dropped status values, and no silent downgrade to a different calculation contract.

## Current Status

This repository now contains a substantial implemented ephemeris engine, but it is still not exact across every domain.

Current status summary:

- CMake build
- public C header at `include/jme/jme.h`
- `jme_*` primary API
- context/path/version API
- Julian day conversion API
- CALCEPH backend boundary
- analytical fallback engines for Moshier, VSOP87, ELP2000, and Meeus
- C-level engine selection for wrapper/config use: `ENGINE=JPL`, `ENGINE=MOSHIER`, `ENGINE=VSOP_ELP_MEEUS`, or `JME_ENGINE` environment selection
- eclipse, occultation, heliacal, house, and physical-phenomena coverage with remaining validation gaps documented in `docs/STATUS_AND_DESIGN.md` and `docs/VALIDATION_AND_COVERAGE.md`

No public calculation function may return approximate production output. A function is either exact for its documented contract or it returns `JME_ERR` with a clear error message.

Read `docs/POLICIES_AND_NOTICES.md`, `CONTRIBUTING.md`, and `docs/STATUS_AND_DESIGN.md` before adding implementation code.

## Build

```bash
cmake -S . -B build
cmake --build build
ctest --test-dir build --output-on-failure
```

With Visual Studio generators:

```bash
ctest --test-dir build -C Debug --output-on-failure
```

If CALCEPH is installed and discoverable, the library builds with CALCEPH support. Without CALCEPH, the API still builds and returns explicit backend errors for ephemeris calculations.

## Backend Data

Supported JPL kernel choices include `de440s.bsp`, `de440.bsp`, and `de441.bsp`.

Kernel files are not vendored in this repository. Download release-packaged kernels from:

```text
https://github.com/jayeshmepani/jpl-ephemeris/releases/tag/jpl-kernels
```

`de441.bsp` is larger than GitHub's single release-asset limit, so the release publishes it as split parts with checksums.

```c
jme_set_jpl_file("path/to/de440s.bsp");
```

## Engine Selection

Wrappers should not reimplement backend selection. Configure the C library directly:

```c
jme_set_astro_models("ENGINE=JPL", 0);             /* strict JPL/CALCEPH, no analytical fallback */
jme_set_astro_models("ENGINE=MOSHIER", 0);         /* Moshier-first analytical planets */
jme_set_astro_models("ENGINE=VSOP_ELP_MEEUS", 0);  /* VSOP87 + ELP2000 + Meeus analytical stack */
jme_set_astro_models("ENGINE=AUTO", 0);            /* JPL first, then analytical fallback */
```

If no explicit engine token is set, the library also reads `JME_ENGINE=JPL`, `JME_ENGINE=MOSHIER`, `JME_ENGINE=VSOP_ELP_MEEUS`, or `JME_ENGINE=ANALYTICAL`.

## License

The independent code in this repository is intended to be permissively licensed. Third-party backend libraries and data keep their own licenses/notices.
