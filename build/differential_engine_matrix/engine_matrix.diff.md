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

- `JPL` calls with timing: `204`, total ns: `54361937`, avg ns: `266480`, max ns: `18964424`
- `MOSHIER` calls with timing: `204`, total ns: `14429032842`, avg ns: `70730553`, max ns: `2893431893`
- `VSOP_ELP_MEEUS` calls with timing: `204`, total ns: `26584335550`, avg ns: `130315370`, max ns: `5206764543`

## Slowest Function+Engine Calls

- `jme_helio_cross` / `VSOP_ELP_MEEUS`: `5206764543` ns, status `OK`
- `jme_helio_cross_ut` / `VSOP_ELP_MEEUS`: `4783213270` ns, status `OK`
- `jme_mooncross` / `VSOP_ELP_MEEUS`: `3402567605` ns, status `OK`
- `jme_mooncross_ut` / `VSOP_ELP_MEEUS`: `3120408012` ns, status `OK`
- `jme_mooncross` / `MOSHIER`: `2893431893` ns, status `OK`
- `jme_mooncross_ut` / `MOSHIER`: `2784164367` ns, status `OK`
- `jme_lun_occult_when_loc` / `VSOP_ELP_MEEUS`: `1527923752` ns, status `ERROR`
- `jme_lun_occult_when_loc` / `MOSHIER`: `1349899598` ns, status `ERROR`
- `jme_mooncross_node` / `MOSHIER`: `1309040429` ns, status `OK`
- `jme_solcross_ut` / `VSOP_ELP_MEEUS`: `1186867085` ns, status `OK`
- `jme_sol_eclipse_when_glob` / `MOSHIER`: `1180636645` ns, status `ERROR`
- `jme_mooncross_node_ut` / `MOSHIER`: `1089899038` ns, status `OK`
- `jme_sol_eclipse_when_glob` / `VSOP_ELP_MEEUS`: `1082224087` ns, status `ERROR`
- `jme_solcross` / `VSOP_ELP_MEEUS`: `1002631295` ns, status `OK`
- `jme_lun_occult_when_glob` / `VSOP_ELP_MEEUS`: `974732139` ns, status `ERROR`
- `jme_lun_occult_when_glob` / `MOSHIER`: `858503479` ns, status `ERROR`
- `jme_sol_eclipse_when_loc` / `VSOP_ELP_MEEUS`: `819087121` ns, status `ERROR`
- `jme_mooncross_node` / `VSOP_ELP_MEEUS`: `798739533` ns, status `OK`
- `jme_mooncross_node_ut` / `VSOP_ELP_MEEUS`: `779688121` ns, status `OK`
- `jme_lun_eclipse_when` / `VSOP_ELP_MEEUS`: `751345503` ns, status `ERROR`

## Notes

- `ENGINE=JPL` is strict. In this build, JPL-mode success depends on CALCEPH availability plus a real kernel.
- Unadapted functions are still emitted as rows with `SKIPPED_WITH_REASON`.
