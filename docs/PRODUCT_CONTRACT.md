# Product Contract

The public product surface is the project-owned `jme_*` API and `JME_*` constants.

This engine is not a wrapper and does not expose historical third-party function names as the primary API. External projects can be studied only as coverage references, not as source or naming authority.

## Product Surface

| Surface | Role |
| --- | --- |
| `jme_*` functions | Public C API |
| `JME_*` constants | Public C constants and enums |
| JPL/CALCEPH-backed calls | Primary numerical ephemeris path |
| Independent analytical modules | Future fallback and cross-check paths |

## Coverage Rule

The goal is an independent engine with feature parity and then feature superset coverage. Function names, constant names, argument shapes, and output structures should belong to this project unless there is a deliberate reason to expose a migration aid in a separate layer.

Every public function must have:

- a compiled source definition
- tests for success and failure paths
- real backing calculation or real backend metadata
- no presentation rounding in the C API
- documented data requirements where a kernel or catalog is required

## Current Status

The current implemented callable surface is tracked in `docs/API_TRACKING.md`.

The old 106-function list is not the public API target. It is only a coverage checklist used to make sure this product eventually covers the same astronomy and astrology capability areas, with project-owned names and behavior.
