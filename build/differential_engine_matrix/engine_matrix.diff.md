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

- `JPL` calls with timing: `204`, total ns: `58116106`, avg ns: `284882`, max ns: `18061497`
- `MOSHIER` calls with timing: `204`, total ns: `453052901`, avg ns: `2220847`, max ns: `159815688`
- `VSOP_ELP_MEEUS` calls with timing: `204`, total ns: `15710191141`, avg ns: `77010740`, max ns: `4963549677`

## Slowest Function+Engine Calls

- `jme_helio_cross_ut` / `VSOP_ELP_MEEUS`: `4963549677` ns, status `OK`
- `jme_helio_cross` / `VSOP_ELP_MEEUS`: `4701347229` ns, status `OK`
- `jme_solcross` / `VSOP_ELP_MEEUS`: `1101990137` ns, status `OK`
- `jme_solcross_ut` / `VSOP_ELP_MEEUS`: `966359842` ns, status `OK`
- `jme_mooncross` / `VSOP_ELP_MEEUS`: `784424045` ns, status `OK`
- `jme_mooncross_ut` / `VSOP_ELP_MEEUS`: `671155847` ns, status `OK`
- `jme_lun_occult_when_loc` / `VSOP_ELP_MEEUS`: `412118850` ns, status `ERROR`
- `jme_sol_eclipse_when_glob` / `VSOP_ELP_MEEUS`: `382072815` ns, status `ERROR`
- `jme_mooncross_node_ut` / `VSOP_ELP_MEEUS`: `346512216` ns, status `OK`
- `jme_lun_occult_when_glob` / `VSOP_ELP_MEEUS`: `305050638` ns, status `ERROR`
- `jme_lun_eclipse_when` / `VSOP_ELP_MEEUS`: `292253344` ns, status `ERROR`
- `jme_sol_eclipse_when_loc` / `VSOP_ELP_MEEUS`: `194628898` ns, status `ERROR`
- `jme_helio_cross` / `MOSHIER`: `159815688` ns, status `OK`
- `jme_mooncross_node` / `VSOP_ELP_MEEUS`: `156453673` ns, status `OK`
- `jme_lun_eclipse_when_loc` / `VSOP_ELP_MEEUS`: `121962511` ns, status `ERROR`
- `jme_helio_cross_ut` / `MOSHIER`: `94037941` ns, status `OK`
- `jme_rise_trans_true_hor` / `VSOP_ELP_MEEUS`: `93642692` ns, status `ERROR`
- `jme_rise_trans` / `VSOP_ELP_MEEUS`: `75868949` ns, status `ERROR`
- `jme_solcross` / `MOSHIER`: `34252026` ns, status `OK`
- `jme_solcross_ut` / `MOSHIER`: `29648822` ns, status `OK`

## Notes

- `ENGINE=JPL` is strict. In this build, JPL-mode success depends on CALCEPH availability plus a real kernel.
- Unadapted functions are still emitted as rows with `SKIPPED_WITH_REASON`.
