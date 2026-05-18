# Independent Development Policy

This repository is intended to be an independent JPL/Moshier-based ephemeris engine.

This file is an engineering policy, not legal advice. The goal is to reduce legal and licensing risk by preventing source-code, data, documentation, table, and implementation copying from restricted projects.

## Mandatory Rules

- Do not copy Astrodienst Swiss Ephemeris source code.
- Do not copy Astrodienst Swiss Ephemeris header files as repository source.
- Do not copy Astrodienst comments, documentation prose, generated files, tables, test fixtures, or ephemeris data files.
- Do not translate Astrodienst implementation code line-by-line into new code.
- Do not use Astrodienst internal algorithms as the implementation source.
- Do not vendor Astrodienst ephemeris files.
- Do not use AGPL Swiss Ephemeris code in this repository unless the whole licensing strategy is deliberately changed and reviewed before the code is added.

## Allowed Implementation Sources

Every non-trivial algorithm must be implemented from independent sources.

Acceptable implementation sources include:

- peer-reviewed astronomy papers
- IAU/IERS/SOFA/ERFA style public standards, subject to their licenses
- NASA/JPL/NAIF documentation and kernels, subject to their terms
- CALCEPH documentation and source API behavior, subject to its terms
- Moshier public-domain code or documentation, only where provenance is confirmed
- independently derived formulas and tests

## Testing Rule

Behavioral comparison against other ephemeris engines is allowed only as black-box verification:

- record inputs
- record returned values, flags, and error strings
- compare behavior numerically and structurally
- do not inspect or copy restricted source used to produce those outputs

Tests must never embed copied restricted source code, comments, tables, or data.

## Public API Rule

The primary public API uses project-owned names:

- functions use `jme_*`
- constants and macros use `JME_*`
- internal symbols use `jme_*`

Any migration layer for another ecosystem must be separate from the primary engine API.

## Review Checklist

Before accepting a new implementation:

- The function has a documented independent source/provenance.
- The implementation does not copy restricted source/header/comment/table text.
- The public output layout matches the documented project contract.
- No rounding, hidden normalization, output reshaping, dropped flags, or silent fallback was added.
- Incomplete behavior returns `JME_ERR` instead of approximate output.
- Tests cover inputs, return code, output array layout, and error-buffer behavior where applicable.
