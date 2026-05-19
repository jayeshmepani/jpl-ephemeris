# API Inventory

This inventory tracks callable API that is currently backed by source definitions.

## Current Policy

- Public declarations must have compiled source definitions.
- Future target coverage is documented as planned work until the corresponding exact implementation and tests are added.
- The public product API is jme_* plus JME_*.
- Historical third-party names are reference material only unless a separate migration layer is deliberately created.

## Current Verified Counts

Run:

``powershell
pwsh -NoProfile -ExecutionPolicy Bypass -File tests\test_symbol_coverage.ps1
``

Current expected shape:

- jme_functions_total: callable JME functions declared in public headers
- jme_functions_defined: same count; every declared JME function must have a source definition
- jme_constants_total: at least 348
- eference_constants_total: exactly 348

Function count is not treated as proof of original-reference behavior coverage. Reference behavior coverage is tracked separately in docs/REFERENCE_FUNCTION_COVERAGE.md.

Current verified output:

``text
jme_functions_total=191
jme_functions_defined=191
jme_constants_total=460
reference_constants_total=348
``