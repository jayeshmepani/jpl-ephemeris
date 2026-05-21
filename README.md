# JPL Moshier Ephemeris

Independent C ephemeris engine using JPL kernels through CALCEPH as the primary backend and independently verified Moshier-derived/public-domain algorithms as secondary components where appropriate.

This is not a wrapper around Astrodienst Swiss Ephemeris and does not use Swiss Ephemeris source code, headers, data files, comments, generated files, or implementation tables.

## Goal

- Provide an independent ephemeris engine for astrology, panchang, astronomy, and calendrical calculations.
- Use JPL `.bsp` kernels through CALCEPH for high-precision numerical ephemerides.
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
- eclipse, occultation, heliacal, house, and physical-phenomena coverage with remaining validation gaps documented in `docs/IMPLEMENTATION_STATUS.md` and `docs/REFERENCE_FUNCTION_COVERAGE.md`

No public calculation function may return approximate production output. A function is either exact for its documented contract or it returns `JME_ERR` with a clear error message.

Read `INDEPENDENT_DEVELOPMENT_POLICY.md`, `CONTRIBUTING.md`, `docs/PROVENANCE.md`, and `docs/LOSSLESS_IO.md` before adding implementation code.

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

Use JPL `.bsp` kernels such as `de440s.bsp`, `de440.bsp`, or `de441.bsp`.

```c
jme_set_jpl_file("path/to/de440s.bsp");
```

## License

The independent code in this repository is intended to be permissively licensed. Third-party backend libraries and data keep their own licenses/notices.
