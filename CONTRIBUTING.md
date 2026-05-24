# Contributing

This project has a strict independent-development rule.

By contributing, you confirm that your contribution:

- was not copied from Astrodienst Swiss Ephemeris source code, headers, comments, documentation, generated files, tables, or ephemeris data files
- was not produced by line-by-line translation of restricted implementation code
- uses independent/public sources for algorithms and behavior
- preserves the lossless I/O contract
- returns explicit `JME_ERR` for incomplete behavior instead of approximate values

## Required Notes For Algorithmic Changes

For every non-trivial calculation change, include the independent source used to implement it in the commit/PR notes or the relevant source documentation.

Examples:

- public standard
- NASA/JPL/NAIF source
- CALCEPH source/API documentation
- Moshier/public-domain source with confirmed provenance
- independently derived formula
- black-box behavior test result

Code copied from Astrodienst Swiss Ephemeris is outside this repository's contribution boundary.
