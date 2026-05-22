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

- `JPL` calls with timing: `204`, total ns: `46431254`, avg ns: `227604`, max ns: `9167963`
- `MOSHIER` calls with timing: `204`, total ns: `13700558771`, avg ns: `67159601`, max ns: `2765730258`
- `VSOP_ELP_MEEUS` calls with timing: `204`, total ns: `24480383173`, avg ns: `120001878`, max ns: `4659428254`

## Slowest Function+Engine Calls

- `jme_helio_cross_ut` / `VSOP_ELP_MEEUS`: `4659428254` ns, status `OK`
- `jme_helio_cross` / `VSOP_ELP_MEEUS`: `4511097110` ns, status `OK`
- `jme_mooncross_ut` / `VSOP_ELP_MEEUS`: `2834846328` ns, status `OK`
- `jme_mooncross_ut` / `MOSHIER`: `2765730258` ns, status `OK`
- `jme_mooncross` / `VSOP_ELP_MEEUS`: `2694543141` ns, status `OK`
- `jme_mooncross` / `MOSHIER`: `2643805744` ns, status `OK`
- `jme_lun_occult_when_loc` / `VSOP_ELP_MEEUS`: `1472829549` ns, status `ERROR`
- `jme_lun_occult_when_loc` / `MOSHIER`: `1407934126` ns, status `ERROR`
- `jme_mooncross_node_ut` / `MOSHIER`: `1116658560` ns, status `OK`
- `jme_mooncross_node` / `MOSHIER`: `1113592161` ns, status `OK`
- `jme_solcross` / `VSOP_ELP_MEEUS`: `1106441387` ns, status `OK`
- `jme_lun_occult_when_glob` / `VSOP_ELP_MEEUS`: `1053268929` ns, status `ERROR`
- `jme_solcross_ut` / `VSOP_ELP_MEEUS`: `1022026134` ns, status `OK`
- `jme_lun_eclipse_when_loc` / `VSOP_ELP_MEEUS`: `990830050` ns, status `ERROR`
- `jme_sol_eclipse_when_glob` / `MOSHIER`: `967742023` ns, status `ERROR`
- `jme_sol_eclipse_when_glob` / `VSOP_ELP_MEEUS`: `923415068` ns, status `ERROR`
- `jme_lun_occult_when_glob` / `MOSHIER`: `795146644` ns, status `ERROR`
- `jme_mooncross_node` / `VSOP_ELP_MEEUS`: `782750359` ns, status `OK`
- `jme_mooncross_node_ut` / `VSOP_ELP_MEEUS`: `763823517` ns, status `OK`
- `jme_lun_eclipse_when` / `MOSHIER`: `755001197` ns, status `ERROR`

## Notes

- `ENGINE=JPL` is strict. In this build, JPL-mode success depends on CALCEPH availability plus a real kernel.
- Unadapted functions are still emitted as rows with `SKIPPED_WITH_REASON`.
