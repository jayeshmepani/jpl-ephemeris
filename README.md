# JPL Moshier Ephemeris

Independent C ephemeris engine using JPL kernels through CALCEPH as the primary backend, with analytical engines based on Moshier's DE404-fit theories and on VSOP87 + ELP2000 + Meeus as secondary paths.

This library is developed and maintained by Jayesh Mepani (Jayesh Patel), who owns the original JME code and the project-owned `jme_*` API. It combines that code with credited third-party theories, data, and code (see [Credits](#credits)) to provide JPL, Moshier, and VSOP+ELP+Meeus engines.

The API reference for this C library and the PHP, Python, and Dart bindings is <https://jayeshmepani.github.io/jpl-ephemeris/>.

## Goal

- Provide an independent ephemeris engine for astrology, astronomy, and calendrical calculations.
- JPL `.bsp` kernels through CALCEPH provide the high-precision numerical ephemeris path.
- Add analytical components only when their provenance and license terms are clear.
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

## Credits

JME builds on the work of others:

- **JPL DE440 / DE441** planetary and lunar ephemerides (NASA/JPL; Park et al. 2021), redistributed unmodified per the NAIF rules.
- **CALCEPH** (CNRS, Observatoire de Paris, Observatoire de la Côte d'Azur), used under CeCILL-B to read JPL kernels.
- **Stephen L. Moshier**: plan404 and Astronomical Almanac 5.6 analytical theories (Moshier engine). MIT License by permission of the author (12 October 2026; see [docs/permissions/MOSHIER_PERMISSION.md](docs/permissions/MOSHIER_PERMISSION.md)). Portions derived from the work of Stephen L. Moshier, incorporated herein by permission of the author.
- **ELP2000-82B** lunar theory by M. Chapront-Touzé and J. Chapront (CDS VI/79 series data, attribution); the evaluator (`src/elp82b.c`) is JME's own implementation (MIT). The earlier third-party C implementation was removed.
- **VSOP87** by P. Bretagnon and G. Francou (official IMCCE files / CDS VI/81 series data); the VSOP87A evaluator (`src/vsop87.c`) is JME's own implementation (MIT).
- **Jean Meeus**, *Astronomical Algorithms* (formulas cited).
- **ERFA** (BSD 3-Clause, NumFOCUS Foundation): vendored subset for precession, nutation, frame bias, sidereal time, light deflection, aberration, star space motion and time scales.
- **F. Espenak and J. Meeus** (NASA): ΔT polynomials.
- **Yale Bright Star Catalogue**, 5th Rev. Ed. (Hoffleit and Warren): fixed stars.

Full credits, terms, and required notice texts: [THIRD_PARTY_NOTICES.md](THIRD_PARTY_NOTICES.md).

## License

The original code in this repository is licensed under the [MIT License](LICENSE), Copyright (c) 2026 Jayesh Mepani (Jayesh Patel).

**Scope:** MIT applies to the original code in this repository; third-party components keep their own terms, see [THIRD_PARTY_NOTICES.md](THIRD_PARTY_NOTICES.md). The Moshier material (`src/moshier_full.c`, `src/moshier_moon.c`, `src/moshier/`) is distributed under the MIT License by permission of Stephen L. Moshier (12 October 2026): Portions derived from the work of Stephen L. Moshier, incorporated herein by permission of the author. The ELP2000-82B evaluator is JME's own code; its series data come from CDS VI/79.
