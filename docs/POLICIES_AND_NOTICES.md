# Policies And Notices

## Current Swiss Ephemeris Boundary

This repository is not a fork, clone, or source-derived copy of Astrodienst Swiss Ephemeris. The public C API is project-owned (`jme_*` functions and `JME_*` constants). Implementation work is based on independent/public sources such as JPL/NAIF data and documentation, CALCEPH API behavior, public-domain Moshier material, VSOP/ELP/Meeus references, peer-reviewed papers, public standards, and independently derived formulas.

This C library is developed and owned by Jayesh Mepani. The motivation was to replace a prior AGPL Swiss Ephemeris dependency with an independently implemented ephemeris library suitable for non-AGPL application distribution. Swiss Ephemeris was used as an inspiration and broad capability benchmark because it exposes a mature astrology/astronomy surface, but this project is intended to be implemented from independent public sources rather than Swiss Ephemeris source, headers, comments, tables, generated files, or ephemeris data.

The design goal is a superset-style project-owned API: cover the broad capability areas represented by the Swiss Ephemeris public surface, then add extra JME-specific functions and constants for JPL/CALCEPH, Moshier, VSOP87, ELP2000, Meeus, vector/state, metadata, and validation workflows. The current surface is 204 public functions and 462 public constants.

Source audit performed on 2026-05-24:

```powershell
rg -n -i "swiss|swisseph|swe_|swiss ephemeris|astrodienst|dieter koch|alois|treindl" -g "*.c" -g "*.h"
```

Result: no matches in current `.c` or `.h` files. Swiss Ephemeris is mentioned only in policy/validation documents as a black-box capability comparison target and as a prohibited source-code/data source.

## Independent Development Policy

This repository is intended to be an independent JPL/Moshier-based ephemeris engine.

This is an engineering policy, not legal advice. Its purpose is to document the source boundary for an independently developed ephemeris engine.

### Source Boundaries

| Area | Project position |
|---|---|
| Swiss Ephemeris source code | Excluded from this repository's implementation sources |
| Swiss Ephemeris headers | Excluded as repository source |
| Swiss comments, prose, generated files, tables, fixtures, and ephemeris data | Excluded from copied project material |
| Line-by-line translation of Swiss implementation code | Outside the project boundary |
| Swiss internal algorithms as implementation source | Outside the project boundary |
| Vendored Swiss ephemeris files | Outside the project boundary |
| AGPL Swiss Ephemeris code | Outside this repository unless the licensing strategy is deliberately changed and reviewed first |

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
