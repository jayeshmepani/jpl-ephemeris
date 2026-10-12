# C engine completion report (running status)

Branch: `fix/precision-time-and-aberration` (PR #3). No merge, no force push, no tag/release.
This file is updated after each milestone. Status keys: FIXED / CERTIFIED, plus the five final states: IMPLEMENTED & VERIFIED / IMPLEMENTED WITH MEASURED LIMITATION / UNSUPPORTED BY DESIGN / BLOCKED (external source) / UNRESOLVED.

## Audit checklist

| Area | Status | Evidence |
|---|---|---|
| Apparent place pipeline (ERFA light time, deflection, aberration, barycentric observer velocity) | FIXED | e6191a5, tests api_contract / analytical_validation |
| ERFA subset vendored (BSD-3, notice) | FIXED | 63dcdf1, src/erfa/vendor/LICENSE.ERFA |
| Engine policy: no silent model substitution; resolve API | FIXED | ae9ba2d (`jme_calc_resolve_engine`) |
| JPL centre policy (STRICT default / opt-in BARYCENTER); CALCEPH errors captured | FIXED | ae9ba2d |
| Thread safety of Moshier scratch arrays | FIXED | 4d83987 |
| Delta-T: IERS observed+predicted table (AUTOMATIC default), SMH-2016/2021 S15 splines, exact Espenak-Meeus 2006 | FIXED | ec72c21, tests/test_delta_t_reference.c |
| Delta-T SM1984/S1997 refused (explicit error, no aliasing, no bare parabola) | BLOCKED (external source) | test_precision_contract, test_delta_t_reference |
| Laskar 1986 precession | IMPLEMENTED & VERIFIED | 4122578: Tables 5 (Earth q+ip) and 8 (p_A, eps_A) checked against the scans; construction reproduces eraBp06 to 4e-12 rad; test_laskar1986_reference |
| Houses: time scales (UT1/TT), apparent GAST, true obliquity, input validation | FIXED | ba8b9e1 |
| Houses: Morinus / Meridian / Horizontal definitions | FIXED | ba8b9e1; definitions per R. Holden, The Elements of House Division (1977) |
| Houses: rates (5-point stencil, NaN when undefined) | FIXED | ba8b9e1 |
| Houses: sidereal flag | FIXED | ba8b9e1 |
| Gauquelin sectors 0/1 at GAST/TT | FIXED | ba8b9e1 |
| Gauquelin 36-sector cusps | FIXED / CERTIFIED | 5244784: jme_houses_gauquelin(_armc); independent sector check 3e-14, Placidus coincidence |
| Events do not mutate caller topocentre (thread-local scoped override) | FIXED | 9730e56 |
| Events use apparent places (was geometric: greatest eclipse off by 35-40 s) | FIXED | 9730e56 |
| Global solar eclipses (Besselian, WGS84 3-D; P1/P4, U1/U4, C1/C2, hybrid) | FIXED | 9730e56; NASA canon within 0.5 s / 0.1 deg (10 eclipses) |
| Local solar eclipses: next visible eclipse search, per-instant radii, obscuration | FIXED | 9730e56 |
| Lunar eclipses: Danjon shadow (0.998340, 1/85), axis-distance maximum, contacts | IMPLEMENTED & VERIFIED | 357b1f0: NASA canon, all 228 eclipses 2001-2100, greatest eclipse within 2.31 s (catalog rounds to 1 s), magnitudes within 0.0008 |
| VSOP87A: full official IMCCE series (42,520 terms), analytic velocity, compensated summation | FIXED | dd293e9, tests/test_vsop87_reference.c (90 vsop87.chk epochs, 5e-11 au) |
| Event root and extremum finding (Illinois, syzygy, Brent + stationary-point refinement) | FIXED | dd293e9 |
| Event evaluation cost (position-only apparent place) | FIXED | dd293e9; engine_matrix 368 s -> 118 s |
| Light-time retarded JPL states rounded to 2^-31 day (40 us; 0.3 mas Moon) | FIXED | ad94650 (offset carried in day fraction) |
| IAU 2006/2000A nutation with P03 adjustments (eraNut06a); IAU2006A preset uses it | FIXED | ad94650, JME_MODEL_NUT_IAU_2006A = 351 |
| Light deflection: Sun only -> Sun + Jupiter + Saturn systems (+ Earth topocentric) | FIXED | f08bfb6 |
| JPL apparent places vs Skyfield/ERFA (ICRS, J2000, of-date, ecliptic; 130 places) | CERTIFIED | tests/test_apparent_reference.c, max 0.009 mas |
| Rise/set/transit/crossings/occultations/heliacal: position-only evaluation | FIXED | f5591e4 |
| Occultations: Moon via complete topocentric apparent place (was geocentric minus observer vector) | FIXED | f5591e4 |
| Rise/set/transit refinement (was 1e-6 d bisection, 1e-4 deg hour-angle cutoff; up to 27 ms) | FIXED / CERTIFIED | 6d5dbcf; tests/test_riseset_reference.c vs Skyfield, 320 events, worst 0.16 ms |
| Tests used unsigned angle difference (wrap of -1e-14 to 360 on macOS libm) | FIXED | 1b13746 |
| CI matrix: GCC, Clang, GCC/Clang + CALCEPH 4.0.5 + DE440s (cached), AppleClang macOS, MSVC Windows | FIXED | c5a708c; green run https://github.com/jayeshmepani/jpl-ephemeris/actions/runs/38180594899 |
| Houses: Placidus cusps 2/3/8/9 were cusps 6/5 (antipodal observer) | FIXED / CERTIFIED | d4be836 |
| Houses: Campanus cusps (wrong prime-vertical elevation/side) | FIXED / CERTIFIED | d4be836 |
| Houses: Polich-Page applied a double pole reduction (Regiomontanus formula on topocentric pole) | FIXED / CERTIFIED | d4be836 |
| Houses reference vs independent vector geometry (P,R,C,A,T,O,E,W; 2354 sets) + UT->ARMC/eps chain vs ERFA | CERTIFIED | tests/test_houses_reference.c, 2.3e-13 deg cusps, 1.1e-10 deg time chain |
| Polar motion for topocentric observer / horizon frame / eclipse shadow frame | IMPLEMENTED WITH MEASURED LIMITATION | e241287, 22f6535: W = eraPom00(xp,yp,sp00), IERS EOP 4-point Lagrange, error without EOP; vs eraC2t06a 6 um; sub-daily tidal/libration EOP terms (<0.5 mas) not applied |
| Fixed stars without Hipparcos data (101 BSC5): FK5 -> ICRS | FIXED / CERTIFIED | 3a082b8 (eraFk52h; removes 24-29 mas); star_reference max 0.0004 mas |
| Exact memoisation of TDB-TT and nutation series | FIXED | 64213e1 (bit-identical; searches 4.3 s -> 0.29 s; engine_matrix 118 s -> ~60 s) |
| Lunar occultations of planets and stars (where/when_loc/when_glob) | FIXED / CERTIFIED | 6046e6b: generalised Besselian construction; occult_reference 27 local (0.08 ms) + 8 global (7.9 ms) vs Skyfield/DE440s |
| Eclipse/occultation contact geometry: astrometric (no aberration) | FIXED | 6046e6b (aberrated vectors shifted contacts up to ~0.6 s); solar eclipse NASA canon still within 0.5 s |
| Lunar-eclipse shadow: retarded shadow | IMPLEMENTED & VERIFIED | 357b1f0: derivation (axis displaced 2 tau v_E) equals apparent-Moon minus apparent-Sun to first order in v/c; explicit retardation agrees within 0.2 s; non-retarded variants rejected (78 s, 38 s); test_lunar_eclipse_canon |
| Docs: ACCURACY, TIME_SCALES, AYANAMSA_MODELS; API_REFERENCE regenerated | FIXED | e32fd0a, 5244784, 6046e6b |
| Ayanamsa provenance (Krishnamurti, J.N. Bhasin) | FLAGGED | docs/AYANAMSA_MODELS.md (KP formula zero 291.22 CE; 54.38" above table-fit KP) |
| Galactic Centre ayanamsa (Sgr A* at 0 Sag, mean/true) | IMPLEMENTED & VERIFIED | cf5affa: Reid & Brunthaler 2004 position, ICRS-fixed (Galactic frame defined in ICRS); vs ERFA 1e-12 deg; test_galcent_reference |
| Delta-T MS2004 | IMPLEMENTED & VERIFIED (named reconstruction) | 67d5764: MS2004_TABLE1_LINEAR, 58 rows checked against the scan, authors' "simple interpolation" (p. 331), parabola -20+32t^2 outside (their stated trend); the unpublished-spline model id 348 stays refused |
| Tidal acceleration: numeric/DE selectors and historical correction | IMPLEMENTED & VERIFIED | 0571dfd, 05c3e52, 78277fc: c = -0.91072 (n - n_model) u^2 before 1955 with each model's own reference (EM -26, SMH -25.82, MS2004 -26); never applied to IERS values or user Delta-T; selectors from primary sources |
| Tidal acceleration AUTOMATIC / DE441 | UNSUPPORTED BY DESIGN / UNRESOLVED | AUTOMATIC applies no correction (SPK files carry no DE number via CALCEPH); DE441 has no published n-dot: pre-1955 Delta-T returns an error naming Park et al. 2021 |

## Commits (this session, newest first)
- 4122578 Laskar 1986 precession (Tables 5 and 8, scan-verified; construction certified against ERFA)
- 1ef7f86 NOTICE.md; Delta-T and tidal sources credited; third-party documentation references replaced with primary sources
- 357b1f0 Lunar-eclipse retarded shadow certified against NASA canon (228 eclipses)
- 0e0415f KP provenance reconciled with secondary sources
- 67d5764 MS2004_TABLE1_LINEAR
- c045cfb / 78277fc SMH 2016 Table S15 bit-exact; 2021 revision split; SMH reference n-dot -25.82
- 22f6535 / e241287 Polar motion (EOP loader, pom00/sp00)
- 05c3e52 / 0571dfd Tidal selectors re-sourced; DE441 unresolved; calendar-year fix
- cf5affa Galactic Centre convention settled and certified
- 1897ce3 / cf408c4 / 4605344 Tidal acceleration regression tests, all DE selectors, model-reference baselines
- 11c0bd8 / 0812380 Report and checklist update for partial tidal-acceleration implementation
- f22334e Docs: tidal acceleration equations, selector provenance, and automatic-selection limitation
- 553908b / 8704a6c / 6bc6be6 Apply explicit lunar tidal acceleration to historical Delta-T; resolve DE selectors
- 85518ab / 073dda8 / 541aa77 / 67b6104 Tidal acceleration context state and initialization
- cf408c4 / 4605344 Tidal acceleration selector coverage and model-baseline regression tests
- f22334e Docs: tidal acceleration equations, selector provenance, and automatic-selection limitation
- 553908b / 8704a6c / 6bc6be6 Apply explicit lunar tidal acceleration to historical Delta-T; resolve DE selectors
- 85518ab / 073dda8 / 541aa77 / 67b6104 Tidal acceleration context state and initialization
- 3919eba / c281a76 Notices: test-generator dependencies, publications consulted
- b48050d Docs: VALIDATION certification table, STATUS superseded notice
- 3cf657e Report milestone
- 6046e6b Occultations: generalised Besselian construction (planets, stars); astrometric contact geometry; occult_reference CTest; ACCURACY.md
- 5244784 Houses: Gauquelin 36-sector cusps API; API_REFERENCE regenerated
- e32fd0a Docs: AYANAMSA_MODELS, TIME_SCALES
- 64213e1 Exact per-thread memos for TDB-TT and nutation series
- 3a082b8 Fixed stars: FK5->ICRS for BSC5 entries; star_reference CTest
- 04e7442 Report update
- d4be836 Houses: Placidus 2/3/8/9, Campanus, Polich-Page corrected; bracketed Placidus solver; houses_reference CTest
- 1b13746 Tests: signed angle differences (macOS)
- 6d5dbcf Rise/set/transit Illinois refinement to 1e-10 d; Skyfield certification
- f5591e4 Events: position-only evaluation; occultations with topocentric apparent places
- c5a708c CI matrix (CALCEPH, cached kernel, Clang, macOS, Windows)
- a5a521d Report milestone
- f08bfb6 Light deflection by Sun, Jupiter, Saturn (+ Earth topocentric)
- ad94650 JPL light time without date rounding; IAU 2006/2000A nutation; Skyfield/ERFA apparent-place certification
- dd293e9 VSOP87A from official IMCCE files; position-only event evaluation; Illinois/Brent root and extremum finding
- 8f9eed3 Lunar eclipses: Danjon shadow, angular greatest eclipse, per-instant contacts, type filter
- 9730e56 Events: scoped topocentre, apparent places, exact Besselian global solar eclipses
- ba8b9e1 Houses: TT/apparent GAST, true obliquity, sidereal flag, Morinus/Meridian/Horizontal, 5-point rates, Gauquelin at GAST
- ec72c21 Delta-T: IERS observed table, SMH-2016/2021 splines, exact Espenak-Meeus; AUTOMATIC default
- 2c4ac2b, ae9ba2d, 3e57adb, 4d83987, e6191a5, 2e5d04d, 63dcdf1 (earlier milestones)

## Semantic changes (for downstream wrappers)
- Default Delta-T is AUTOMATIC (IERS table, then SMH/EM).
- `jme_sol_eclipse_when_glob` tret[2..9] are now physical global contacts (P1, P4, U1, U4, C1, C2, hybrid transitions); previously geocentric disc contacts. `epheflag` is an eclipse-type filter (0 = any; unknown values are errors).
- `jme_sol_eclipse_where` returns the instantaneous type (total/annular at that instant), attr[1] is obscuration, attr[9] Moon/Sun diameter ratio.
- `jme_sol_eclipse_when_loc` searches forward/backward to the next eclipse visible at the place; attr[9] = maximum visible flag.
- Houses: Placidus cusps 2, 3, 8, 9 change (previously cusps 6/5 and their antipodes were returned); Campanus and Polich-Page (topocentric) cusps change at all non-zero latitudes; Placidus returns an error where a semi-arc is undefined.
- Occultations use the topocentric apparent Moon; event timings shift at the 0.1 ms - 27 ms level from the refinement change.
- Light deflection now includes Jupiter and Saturn (sub-mas changes); nutation preset IAU2006A uses eraNut06a (new constant 351, constant count 478).
- Houses: Morinus cusp 10 is not the MC; Horizontal cusp 1 is not the Ascendant; MERIDIAN and MORINUS are distinct.
- New functions jme_houses_gauquelin, jme_houses_gauquelin_armc (function count 217 -> 219).
- Solar eclipse and occultation contacts now use astrometric geometry: timings shift by ~0.1-0.6 s versus the previous apparent-vector construction.
- jme_lun_occult_when_glob: tret[2]/[3] first/last contact anywhere, [4]/[5] total phase, [6]/[7] central phase; invalid inputs return JME_ERR; searches span up to 2000 conjunctions (previously only the next conjunction).
- jme_lun_occult_where/when_loc: attr[0] fraction of target diameter covered, attr[1] obscuration, attr[2] astrometric separation; attr[9] maximum visible; any part of [C1,C4] with the target above the horizon counts.
- Fixed stars without Hipparcos astrometry move by 24-29 mas (FK5 -> ICRS).

## Unresolved / documented limits
- Default frame models remain IAU 1976/1980 without frame bias (historical default); they differ from IAU 2006/2000A by ~0.3"/century (e.g. 52 mas in 2023). Recommendation: select IAU2006A for modern work; changing the default is a product decision.
- SM1984/S1997 Delta-T and the MS2004 unpublished spline (id 348): explicit errors.
- Delta-T steps: 0.22 s at the 1973 IERS switch; 358 s at -720 (parabola/spline).
- Polar motion is opt-in (`jme_set_polar_motion`, EOP file required); sub-daily EOP terms (< 0.5 mas) not applied.
- Eclipse contact tangency evaluated on the axis silhouette (generator tilt 0.27 deg; < 0.2 s).

## CI
Green on 0e0415f (6 jobs): https://github.com/jayeshmepani/jpl-ephemeris/actions/runs/38206638910. Later heads: see the PR checks.

## Test count
25 CTest tests: all pass locally with GCC + CALCEPH 4.0.5 + DE440s (reference tests skip without CALCEPH).

## Pending
PR description draft (in the final report; not posted).

## Audit of the earlier tool's commits (85518ab..f093865) and completion

| Item | Earlier tool (commits, files) | Classification at audit | Completed here | Final state |
|---|---|---|---|---|
| Tidal acceleration (`jme_set_tid_acc`) | 85518ab, 6bc6be6, 8704a6c, 553908b, 6089381, 4605344, f22334e, cf408c4, 1897ce3, 1ce52da, d8308b0, f093865; src/api.c, src/time.c, src/context.*, tests/test_delta_t_reference.c, docs/TIME_SCALES.md | PARTIAL: correct formula and IERS/userdef exclusions, but the test failed on f093865 (calendar-year bug), selector values were not from primary sources, DE441 -25.936 unpublished, SMH reference -25.85 instead of -25.82 | 0571dfd, 05c3e52, 78277fc: calendar fix, primary-source selectors, DE441 unresolved, -25.82, AUTOMATIC kernel inference rejected with reasons | IMPLEMENTED & VERIFIED (DE441: UNRESOLVED) |
| Polar motion | none (report row OPEN) | NOT DONE | e241287, 22f6535: EOP loader, W matrix, topocentric/horizon/eclipse frames, error without EOP; vs eraC2t06a 6 um | IMPLEMENTED WITH MEASURED LIMITATION (sub-daily terms) |
| Lunar-eclipse retarded shadow | report text only ("terms of equal size cancel"), no code or test | WORKAROUND (unverified claim) | 357b1f0: derivation, 4 variants measured on 228 canon eclipses, regression test | IMPLEMENTED & VERIFIED |
| Galactic Centre / Sgr A* | none beyond the pre-existing model (report row OPEN) | PARTIAL (position used, convention and motion unsettled) | cf5affa: convention settled, ICRS-fixed justified, mean/true, ERFA certification | IMPLEMENTED & VERIFIED |

## Researcher sources (a)-(g)
- (a) SMH 2016 Table S15: verified bit for bit; rule S5.1 and the parabola outside (78277fc, c045cfb).
- (b) MS2004: Table 1 verified against the scan; implemented only as MS2004_TABLE1_LINEAR, using the authors' simple interpolation.
- (c) SM1984 / S1997: BLOCKED. The sources are not accessible. Requests to Researcher: Stephenson & Morrison 1984, Phil. Trans. R. Soc. A 313, 47 (tables), and Stephenson 1997, *Historical Eclipses and Earth's Rotation* (CUP), the ΔT table and its interpolation rule. No Stephenson-Houlden 1986 formulas and no bare parabola under these names.
- (d) Laskar 1986: Table 8 (p_A, eps_A) and Table 5 (Earth q+ip) verified against p-10.png and hi-06.png. Erratum A&A 164, 437 obtained from ADS and checked: it corrects only a Summary number and the page order, not the tables. Implemented in 4122578.
- (e) Tidal acceleration: DE430 -25.82 and DE431 -25.80 (Folkner et al. 2014). No published DE440/441 value, so DE441 is UNRESOLVED. SMH reference -25.82, EM -26.
- (f) KP: only secondary sources exist; provenance is flagged in docs/AYANAMSA_MODELS.md.
- (g) Galactic Centre: the defining convention (Sgr A* at 0 deg Sagittarius) is documented with the Reid & Brunthaler 2004 position. Under the clean-room rule no third-party software documentation is cited.

## Clean room
No AGPL source consulted. Third-party software documentation references were removed from code, docs and comments (1ef7f86). Residual risk: some public API names and output-array conventions (`tret`, `attr`, `geopos`, `iflag`, `*_tid_acc`) follow the widely used astrology-library calling style. Renaming them would break the API, so it needs a product decision.
