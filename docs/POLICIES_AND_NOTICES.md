# Policies And Notices

## Project Boundary

The public C API is project-owned: public functions use `jme_*`, public constants use `JME_*`, and internal symbols use `jme_*`.

This C library is developed and owned by Jayesh Mepani. It is designed as an independently implemented ephemeris engine with three calculation paths: JPL/CALCEPH, Moshier, and VSOP87+ELP2000+Meeus.

Implementation work is based on independent and public sources such as JPL/NAIF data and documentation, CALCEPH API behavior, public-domain Moshier material, VSOP/ELP/Meeus references, peer-reviewed papers, public standards, and independently derived formulas.

The design goal is a broad project-owned API for astrology, astronomy, vector/state, metadata, validation, and calendrical workflows. The current surface is 204 public functions and 462 public constants.

## Independent Development Policy

This repository is intended to be an independent JPL/Moshier-based ephemeris engine.

This is an engineering policy, not legal advice. Its purpose is to document the source boundary for an independently developed ephemeris engine.

### Source Boundaries

| Area | Project position |
|---|---|
| Restricted third-party source code | Excluded from this repository's implementation sources |
| Restricted third-party headers | Excluded as repository source |
| Restricted comments, prose, generated files, tables, fixtures, and ephemeris data | Excluded from copied project material |
| Line-by-line translation of restricted implementation code | Outside the project boundary |
| Restricted internal algorithms as implementation source | Outside the project boundary |
| Vendored restricted ephemeris files | Outside the project boundary |

### Implementation Sources

Non-trivial algorithms are traced to independent sources. Accepted source categories include:

- peer-reviewed astronomy papers
- IAU/IERS/SOFA/ERFA style public standards, subject to their licenses
- NASA/JPL/NAIF documentation and kernels, subject to their terms
- CALCEPH documentation and source API behavior, subject to its terms
- Moshier public-domain code or documentation, only where provenance is confirmed
- independently derived formulas and tests

### Black-Box Verification

Behavioral comparison against other ephemeris engines is treated as black-box verification:

| Verification artifact | Description |
|---|---|
| Inputs | Recorded date, body, location, flags, and configuration |
| Outputs | Returned values, flags, status codes, and error strings |
| Comparison | Numeric and structural comparison against independently produced results |
| Source boundary | No restricted source code, comments, tables, or data copied into tests |

Tests in this repository contain project-owned cases, independent reference values, or black-box output comparisons.

### Public API

The primary public API uses project-owned names:

- functions use `jme_*`
- constants and macros use `JME_*`
- internal symbols use `jme_*`

Migration layers for other ecosystems are separate from the primary engine API.

### Acceptance Criteria

New implementations are considered complete when:

- The function has a documented independent source/provenance.
- The implementation does not copy restricted source/header/comment/table text.
- The public output layout matches the documented project contract.
- No rounding, hidden normalization, output reshaping, dropped flags, or silent fallback was added.
- Incomplete behavior returns `JME_ERR` instead of approximate output.
- Tests cover inputs, return code, output array layout, and error-buffer behavior where applicable.

---

## Third-Party Notices

### CALCEPH

CALCEPH is not vendored in this repository. When used, it is linked as an external library.

CALCEPH is governed by CeCILL-C, CeCILL-B, or CeCILL v2 terms according to the official CALCEPH documentation. If binaries are distributed with CALCEPH, include CALCEPH copyright/license notices and provide source for any CALCEPH modifications.

### JPL Ephemeris Kernels

JPL `.bsp` kernels are data files supplied separately by NASA/JPL/NAIF or related official sources. They are not vendored here by default.
