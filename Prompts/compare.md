You are working in this repository:

/home/shreesoftech/projects/test1/astro_packages/jpl-ephemeris-

Goal:
Build a complete differential test harness that executes ALL exported public wrapper functions currently counted by symbol coverage as 204 functions, across exactly 3 engine modes:

1. ENGINE=JPL
2. ENGINE=MOSHIER
3. ENGINE=VSOP_ELP_MEEUS

For each function, use the SAME deterministic input fixture across all 3 engines, capture the FULL raw output losslessly, and generate a comparison matrix with:

- one row per public function
- three primary output columns:
  - JPL output
  - MOSHIER output
  - VSOP_ELP_MEEUS output
- plus diff/status columns that show whether outputs are identical, numerically different, structurally different, errored, skipped, or unsupported.

Important:
Do NOT silently round, truncate, pretty-format, normalize, or simplify any output. The purpose is to preserve exact I/O for forensic comparison.

============================================================
STRICT LOSSLESS OUTPUT REQUIREMENTS
============================================================

1. Do not use normal printf("%.6f") or rounded decimal formatting.

2. For every floating-point value, capture BOTH:
   - C99 hex-float representation using %a
   - raw IEEE-754 bytes as hexadecimal

Example:
double d;
record:
{
  "type": "double",
  "decimal": generated with enough precision if available,
  "hexfloat": "0x1.921fb54442d18p+1",
  "raw_le_hex": "182d4454fb210940"
}

3. For decimal text representation, use maximum round-trip precision:
   - double: DBL_DECIMAL_DIG or at least 17 significant digits
   - float: FLT_DECIMAL_DIG or at least 9 significant digits
   - long double: LDBL_DECIMAL_DIG
But decimal is secondary. The authoritative representation is:
   - hexfloat
   - raw bytes

4. If any function returns structs, arrays, pointer outputs, or out-parameters:
   - capture every field
   - preserve order
   - preserve names if known
   - include raw bytes where possible
   - include output buffer length
   - include null terminator status for strings

5. For strings:
   - capture exact bytes as hex
   - capture UTF-8/text version only as convenience
   - do not trim whitespace
   - do not normalize newlines
   - preserve NUL bytes if present

6. For integers:
   - capture signed decimal
   - unsigned decimal
   - hex
   - raw bytes

7. For return codes:
   - capture exact return value
   - capture errno if changed
   - capture any library error/status output if available

8. If a function writes to a caller-provided buffer:
   - initialize buffer with a deterministic sentinel pattern before call, e.g. 0xA5
   - after call, capture the full buffer bytes, not only strlen()
   - also capture visible string if applicable

9. If a function returns a pointer:
   - do not compare raw process addresses as semantic output
   - capture whether null/non-null
   - if pointer points to documented static/string data, capture pointed data bytes safely
   - mark pointer address as non-deterministic metadata only

10. Do not use JSON serialization that loses precision.
    JSON output must store numbers as strings when needed.
    Prefer:
    {
      "hexfloat": "...",
      "raw_le_hex": "...",
      "decimal_string": "..."
    }

============================================================
INPUT FIXTURE REQUIREMENTS
============================================================

Use the exact same input set for each engine mode.

Create deterministic fixtures covering at least these cases:

A. Modern standard date:
   UTC: 2026-05-22 12:00:00
   JD_UT equivalent calculated by library or known helper
   Location: Bhuj, Gujarat, India
   Latitude: 23.241999
   Longitude: 69.666931
   Altitude: 110.0 meters
   Timezone metadata: Asia/Kolkata, but core calculations should use UTC/JD as needed.

B. J2000 baseline:
   UTC: 2000-01-01 12:00:00 TT/UTC distinction must be explicit
   Location same as above.

C. Ancient/edge test:
   UTC/JD equivalent for 1000-01-01 00:00:00 UTC
   Same location.

D. Future test:
   UTC: 2050-01-01 00:00:00 UTC
   Same location.

E. High latitude test for rise/set/house edge cases:
   Latitude: 69.6492
   Longitude: 18.9553
   Location: Tromsø, Norway
   Date: 2026-06-21 12:00:00 UTC

Do not invent function-specific random inputs.
Every input must be deterministic and documented.

For each function, choose the minimum safe fixture required by its signature.
If a function requires a body/planet id, test at least:
   SUN, MOON, MERCURY, VENUS, MARS, JUPITER, SATURN, URANUS, NEPTUNE, PLUTO, MEAN_NODE, TRUE_NODE
where supported by the function.

If a function requires flags/options, test canonical/default flags first.
Optionally add secondary fixtures for sidereal/tropical/apparent/equatorial if the function’s purpose requires it.

============================================================
ENGINE SELECTION REQUIREMENTS
============================================================

For each function test, run it under each engine mode in isolation:

1. ENGINE=JPL
2. ENGINE=MOSHIER
3. ENGINE=VSOP_ELP_MEEUS

Use the public C-level selector:

jme_set_astro_models("ENGINE=JPL", 0);
jme_set_astro_models("ENGINE=MOSHIER", 0);
jme_set_astro_models("ENGINE=VSOP_ELP_MEEUS", 0);

Do not rely only on environment variables for the main test, but add a small separate env-selection test to verify:
   JME_ENGINE=JPL
   JME_ENGINE=MOSHIER
   JME_ENGINE=VSOP_ELP_MEEUS
   JME_ENGINE=ANALYTICAL

For ENGINE=JPL:
- It must be strict.
- If kernel is missing or CALCEPH/JPL cannot run, mark the JPL column as ERROR_JPL_UNAVAILABLE.
- Do NOT silently fallback to analytical.

For ENGINE=MOSHIER:
- Ensure JPL is skipped.
- Expected behavior: Moshier-first planets, ELP2000/Meeus Moon if that is current implementation policy.

For ENGINE=VSOP_ELP_MEEUS:
- Ensure both JPL and Moshier are skipped for planets.
- Expected behavior: VSOP87 first, then Meeus; Moon via ELP2000/Meeus.

============================================================
FUNCTION DISCOVERY
============================================================

Use the existing symbol coverage mechanism as the source of truth.

Current expected public surface:
- 204 functions
- 462 constants

Do not manually hardcode an incomplete function list if the project already has generated symbol coverage metadata.

Implement one of these approaches:

Preferred:
- Load/parse the same symbol list used by tests/test_symbol_coverage.ps1 or equivalent generated API manifest.
- Generate test rows from that manifest.

Acceptable:
- Use nm/readelf/dumpbin to list exported public symbols, then filter public wrapper API symbols.
- Cross-check count equals 204.

Fail the test if discovered function count is not 204, unless the repository intentionally changed and symbol coverage test also changed.

============================================================
FUNCTION CALL STRATEGY
============================================================

Because all 204 functions may have different signatures, implement a registry of function test adapters.

Each adapter must define:
- function_name
- signature description
- input fixture used
- call implementation
- output capture schema
- whether comparable across engines
- expected unsupported behavior if any

Example adapter shape:

{
  "function": "jme_calc_ut",
  "signature": "...",
  "fixture_id": "modern_bhuj_standard",
  "engines": ["JPL", "MOSHIER", "VSOP_ELP_MEEUS"],
  "call": "...",
  "outputs": {
    "return_code": "...",
    "out_array": "...",
    "serr_buffer": "..."
  }
}

For functions that are not calculation-sensitive, such as version/info/constants helpers:
- still run them under all 3 modes
- mark category as "engine_invariant"
- compare exact output

For setter/config functions:
- test carefully in isolated process or reset state after each call
- do not let one test contaminate the next

For functions that cannot safely be called without complex setup:
- mark as SKIPPED_WITH_REASON
- but the row must still exist in the final 204-function matrix
- skipped functions should be minimized and justified.

============================================================
PROCESS ISOLATION
============================================================

Prefer process isolation per function+engine to avoid global state contamination.

Recommended layout:
- a test runner enumerates function adapters
- for each function and engine, spawn a small helper executable:
  ./jme_call_probe --function <name> --engine <engine> --fixture <fixture_id>
- helper prints a lossless JSON record to stdout
- runner combines records into the matrix

If process isolation is too heavy, at minimum:
- reset engine state before every call
- reset ephemeris paths/kernels
- reset sidereal mode
- reset global flags
- reset errno
- clear buffers

============================================================
OUTPUT FILES
============================================================

Generate these files under:

build/differential_engine_matrix/

1. engine_matrix.full.jsonl
   One JSON line per function.
   Must include all engine outputs losslessly.

2. engine_matrix.summary.csv
   One row per function.
   Columns:
   - function_name
   - category
   - fixture_id
   - jpl_status
   - moshier_status
   - vsop_elp_meeus_status
   - jpl_output_sha256
   - moshier_output_sha256
   - vsop_elp_meeus_output_sha256
   - jpl_vs_moshier_relation
   - jpl_vs_vsop_relation
   - moshier_vs_vsop_relation
   - max_abs_diff_if_numeric
   - max_rel_diff_if_numeric
   - notes

3. engine_matrix.diff.md
   Human-readable report:
   - total functions tested
   - total identical across all 3
   - total numerically different
   - total structurally different
   - total errored
   - total skipped
   - top differences by absolute/relative numeric difference
   - functions where JPL strict mode failed
   - functions where analytical engines diverged heavily

4. engine_matrix.raw/
   Optional per-call raw JSON files:
   <function>/<engine>.json

============================================================
COMPARISON RULES
============================================================

Do not compare rounded display text.

Comparison should be based on canonical captured structure.

For exact comparison:
- hash canonical JSON containing raw representations
- preserve field order deterministically

For numeric comparison:
- parse hexfloat back to high precision if needed
- compute absolute difference
- compute relative difference
- compute ULP difference for doubles when possible
- never replace raw values with rounded values

Relationships:
- IDENTICAL_RAW
- IDENTICAL_NUMERIC
- DIFFERENT_NUMERIC
- DIFFERENT_STRUCTURE
- ERROR_ONE_SIDE
- ERROR_ALL
- SKIPPED
- NOT_COMPARABLE

============================================================
NO SILENT TRADE-OFFS
============================================================

Do not say “close enough” without storing exact raw differences.

Do not round to 6 decimals.

Do not truncate long arrays.

Do not omit buffers.

Do not omit error strings.

Do not normalize engine outputs.

Do not allow AUTO mode inside this matrix.

This test is not about user-friendly display.
This test is about forensic engine comparison.

============================================================
TEST INTEGRATION
============================================================

Add CTest integration:

ctest -R engine_matrix --output-on-failure

Add a focused script:

./tools/run_engine_matrix.sh

or:

pwsh ./tests/test_engine_matrix.ps1

The test should:
1. build required probe executable
2. verify symbol count is 204
3. verify constants count is 462 if current manifest supports constants
4. verify required JPL kernel path exists for JPL-mode tests
5. run all function adapters
6. generate all output files
7. fail only for infrastructure problems, not because engines differ numerically

Engine differences are expected and should be reported, not automatically failed, unless:
- a function crashes
- a function corrupts output
- strict JPL silently falls back
- selected engine does not match actual engine used
- lossless capture fails
- function inventory is incomplete

============================================================
EXPECTED FINAL RESULT
============================================================

At the end, print:

- discovered function count
- tested function count
- skipped function count with reasons
- per-engine success/error counts
- output directory
- command to open summary CSV/report

Also update documentation:

docs/ENGINE_DIFFERENTIAL_MATRIX.md

Document:
- purpose
- how engine modes are selected
- why output is captured as hexfloat/raw bytes
- how to interpret differences
- how to add adapters for new exported functions
- how to run with real JPL/CALCEPH kernels

============================================================
QUALITY BAR
============================================================

Before final answer, run:

ctest --test-dir build --output-on-failure
ctest --test-dir build -R engine_matrix --output-on-failure
tests/test_symbol_coverage.ps1 or equivalent
git diff --check

Do not leave incomplete-implementation marker text in source/docs, except in third-party/build artifacts if unavoidable.

Important:
If some functions cannot be called safely because the ABI/signature metadata is insufficient, do not invent calls. Create rows for them with SKIPPED_WITH_REASON and document what adapter metadata is missing.
