# Engine Differential Matrix

- discovered function count: `204`
- tested function count: `204`
- skipped function rows: `0`
- rows with one or more engine errors: `81`
- identical raw across all three engines: `0`
- JPL kernel path present: `yes`
- full jsonl: `/home/shreesoftech/projects/test1/astro_packages/jpl-ephemeris-/build/differential_engine_matrix/engine_matrix.full.jsonl`
- summary csv: `/home/shreesoftech/projects/test1/astro_packages/jpl-ephemeris-/build/differential_engine_matrix/engine_matrix.summary.csv`
- raw directory: `/home/shreesoftech/projects/test1/astro_packages/jpl-ephemeris-/build/differential_engine_matrix/engine_matrix.raw`

## Per-Engine Timing

- `JPL` calls with timing: `204`, total ns: `48813142`, avg ns: `239280`, max ns: `7187334`
- `MOSHIER` calls with timing: `204`, total ns: `7897621049`, avg ns: `38713828`, max ns: `1337429695`
- `VSOP_ELP_MEEUS` calls with timing: `204`, total ns: `21477333481`, avg ns: `105281046`, max ns: `5796719840`

## Slowest Function+Engine Calls

- `jme_helio_cross` / `VSOP_ELP_MEEUS`: `5796719840` ns, status `OK`
- `jme_helio_cross_ut` / `VSOP_ELP_MEEUS`: `5294054499` ns, status `OK`
- `jme_mooncross_ut` / `VSOP_ELP_MEEUS`: `1616100345` ns, status `OK`
- `jme_mooncross` / `VSOP_ELP_MEEUS`: `1540475618` ns, status `OK`
- `jme_mooncross` / `MOSHIER`: `1337429695` ns, status `OK`
- `jme_mooncross_ut` / `MOSHIER`: `1278835734` ns, status `OK`
- `jme_solcross` / `VSOP_ELP_MEEUS`: `1241266241` ns, status `OK`
- `jme_solcross_ut` / `VSOP_ELP_MEEUS`: `1153324126` ns, status `OK`
- `jme_lun_occult_when_loc` / `VSOP_ELP_MEEUS`: `936471481` ns, status `ERROR`
- `jme_lun_occult_when_loc` / `MOSHIER`: `848468189` ns, status `ERROR`
- `jme_lun_occult_when_glob` / `VSOP_ELP_MEEUS`: `814867584` ns, status `ERROR`
- `jme_mooncross_node_ut` / `MOSHIER`: `716763992` ns, status `OK`
- `jme_sol_eclipse_when_glob` / `VSOP_ELP_MEEUS`: `670034984` ns, status `ERROR`
- `jme_sol_eclipse_when_loc` / `MOSHIER`: `668807957` ns, status `ERROR`
- `jme_sol_eclipse_when_glob` / `MOSHIER`: `607939968` ns, status `ERROR`
- `jme_mooncross_node` / `MOSHIER`: `574516836` ns, status `OK`
- `jme_lun_occult_when_glob` / `MOSHIER`: `543082829` ns, status `ERROR`
- `jme_lun_eclipse_when` / `MOSHIER`: `494211793` ns, status `ERROR`
- `jme_lun_eclipse_when` / `VSOP_ELP_MEEUS`: `439257704` ns, status `ERROR`
- `jme_mooncross_node` / `VSOP_ELP_MEEUS`: `438728267` ns, status `OK`

## Notes

- `ENGINE=JPL` is strict. In this build, JPL-mode success depends on CALCEPH availability plus a real kernel.
- Unadapted functions are still emitted as rows with `SKIPPED_WITH_REASON`.
