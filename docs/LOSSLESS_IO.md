# Lossless I/O Contract

This project must not publish approximate production outputs.

## Rules

- Do not round numeric outputs for presentation inside the C API.
- Do not normalize angles unless the documented function contract explicitly requires that exact normalization.
- Do not silently switch algorithms when an exact backend is unavailable.
- Do not reshape arrays or hide return/status flags.
- Do not drop error buffers or status information.
- Do not convert raw backend vectors into public longitude/latitude output until the complete, audited reduction path is implemented.
- Do not claim completed behavior for a function until it has tests.
- Direct engine APIs must not silently fall back to a different theory for unsupported bodies; fallback selection belongs in the explicit high-level calculation pipeline.

## Incomplete Functions

Incomplete functions must return `JME_ERR` and set an error buffer where available.

This is intentional. An explicit error is better than a lossy or approximate result.

## Backend Boundary

CALCEPH may provide raw ephemeris vectors. Public functions must only expose derived values after the full required time-scale handling, frame reduction, apparent/geometric mode handling, sidereal handling, and flag behavior are implemented and tested.
