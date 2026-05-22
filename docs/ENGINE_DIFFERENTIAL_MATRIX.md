# Engine Differential Matrix

This harness generates a forensic comparison matrix for the public `jme_*` surface across exactly three engine selections:

- `ENGINE=JPL`
- `ENGINE=MOSHIER`
- `ENGINE=VSOP_ELP_MEEUS`

The purpose is not to force equality. The purpose is to preserve exact call I/O per engine and make differences auditable.

## How It Works

The matrix is produced by two pieces:

- `jme_call_probe`
  A C executable that performs one function call under one selected engine and emits a lossless JSON record.
- `tools/run_engine_matrix.sh`
  A runner that discovers the public function inventory from the same header surface used by symbol coverage, executes the probe across all three engines, stores raw records, hashes them, and writes summary artifacts.

The current output directory is:

- `build/differential_engine_matrix/`

Files produced:

- `engine_matrix.full.jsonl`
- `engine_matrix.summary.csv`
- `engine_matrix.diff.md`
- `engine_matrix.raw/<function>/<engine>.json`

Each raw per-engine record now also contains exact monotonic wall-clock timing metadata for that specific function call:

- `timing.start.sec`
- `timing.start.nsec`
- `timing.end.sec`
- `timing.end.nsec`
- `timing.elapsed_ns`
- `timing.elapsed_seconds_decimal_string`

## Why Hexfloat And Raw Bytes

Normal decimal formatting can hide real differences.

For floating-point values, the authoritative forms are:

- C99 hex-float text via `%a`
- raw IEEE-754 bytes as hexadecimal

Decimal text is convenience only. It is not the primary comparison form.

Buffers and strings are also captured losslessly:

- full buffer bytes
- visible text when meaningful
- null-terminator presence within capacity

Response time is captured losslessly too. The authoritative timing form is integer nanoseconds from `CLOCK_MONOTONIC`, with start/end timestamps preserved separately.

## Engine Selection

The probe sets the engine through the public selector:

```c
jme_set_astro_models("ENGINE=JPL", 0);
jme_set_astro_models("ENGINE=MOSHIER", 0);
jme_set_astro_models("ENGINE=VSOP_ELP_MEEUS", 0);
```

`ENGINE=JPL` is treated as strict. If CALCEPH or the required kernel path is unavailable, JPL-mode records must remain explicit error records instead of silently falling back.

## Inventory Source

The runner derives the public function inventory directly from:

- `include/jme/jme.h`
- `include/jme/jme_extended.h`

The expected counts are:

- `204` public functions
- `462` constants

If the discovered counts differ, the runner stops as an infrastructure failure.

## Adapters

Not all public functions share a signature, so the probe uses per-function adapters.

Each adapter is responsible for:

- deterministic fixture choice
- engine reset/setup
- exact out-parameter capture
- buffer sentinel initialization
- status classification

When a function is not yet safely callable by the probe, the row is still emitted with:

- `SKIPPED_WITH_REASON`

This keeps the matrix inventory-complete even while adapter coverage expands.

## Running

Build and run the focused matrix:

```bash
cmake --build build --target jme_call_probe
./tools/run_engine_matrix.sh
```

Or through CTest:

```bash
ctest --test-dir build -R engine_matrix --output-on-failure
```

## Interpreting Differences

The summary CSV reports per-function engine status plus raw-output hashes and pairwise relations.

It also includes per-engine elapsed time columns:

- `jpl_elapsed_ns`
- `moshier_elapsed_ns`
- `vsop_elp_meeus_elapsed_ns`

Typical relations:

- `IDENTICAL_RAW`
- `ERROR_ONE_SIDE`
- `ERROR_ALL`
- `SKIPPED`
- `DIFFERENT_STRUCTURE`

The current runner preserves raw outputs first. Numeric-difference extraction can be extended later without changing the lossless per-engine records.

## Real JPL Kernels

The runner uses:

- `$JME_TEST_JPL_KERNEL` when set
- otherwise `data/jpl/de440s.bsp`

For real JPL-mode success, the build must also have CALCEPH enabled. A present kernel file alone is not enough if the library was built without CALCEPH.
