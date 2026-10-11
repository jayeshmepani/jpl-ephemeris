# C engine completion report (running status)

Branch: `fix/precision-time-and-aberration` (PR #3). No merge, no force push, no tag/release.
This file is updated after each milestone. Status keys: FIXED / PARTIAL / OPEN.

## Audit checklist

| Area | Status | Evidence |
|---|---|---|
| Apparent place pipeline (ERFA light time, deflection, aberration, barycentric observer velocity) | FIXED | e6191a5, tests api_contract / analytical_validation |
| ERFA subset vendored (BSD-3, notice) | FIXED | 63dcdf1, src/erfa/vendor/LICENSE.ERFA |
| Engine policy: no silent model substitution; resolve API | FIXED | ae9ba2d (`jme_calc_resolve_engine`) |
| JPL centre policy (STRICT default / opt-in BARYCENTER); CALCEPH errors captured | FIXED | ae9ba2d |
| Thread safety of Moshier scratch arrays | FIXED | 4d83987 |
| Delta-T: IERS observed+predicted table (AUTOMATIC default), SMH-2016/2021 S15 splines, exact Espenak-Meeus 2006 | FIXED | ec72c21, tests/test_delta_t_reference.c |
| Refused Delta-T models SM1984/S1997/SM2004 (explicit error, no aliasing) | FIXED (explicit) | test_precision_contract |
| Laskar 1986 precession | OPEN (explicit error) | coefficients not transcribed/verified |
| Houses: time scales (UT1/TT), apparent GAST, true obliquity, input validation | FIXED | ba8b9e1 |
| Houses: Morinus / Meridian / Horizontal definitions | FIXED | ba8b9e1 (Swiss Ephemeris documentation 6.2.7-6.2.9, conventions only) |
| Houses: rates (5-point stencil, NaN when undefined) | FIXED | ba8b9e1 |
| Houses: sidereal flag | FIXED | ba8b9e1 |
| Gauquelin sectors 0/1 at GAST/TT | FIXED | ba8b9e1 |
| Gauquelin as a 36-sector house system in jme_houses* | OPEN | cusp arrays are 13 long in the public API; needs a new API |
| Events do not mutate caller topocentre (thread-local scoped override) | FIXED | 9730e56 |
| Events use apparent places (was geometric: greatest eclipse off by 35-40 s) | FIXED | 9730e56 |
| Global solar eclipses (Besselian, WGS84 3-D; P1/P4, U1/U4, C1/C2, hybrid) | FIXED | 9730e56; NASA canon within 0.5 s / 0.1 deg (10 eclipses) |
| Local solar eclipses: next visible eclipse search, per-instant radii, obscuration | FIXED | 9730e56 |
| Lunar eclipses: Danjon shadow (0.998340, 1/85), axis-distance maximum, contacts | FIXED | NASA canon: 8 eclipses within 1 s, magnitude within 0.0006 |
| VSOP87A: full official IMCCE series (42,520 terms), analytic velocity, compensated summation | FIXED | dd293e9, tests/test_vsop87_reference.c (90 vsop87.chk epochs, 5e-11 au) |
| Event root and extremum finding (Illinois, syzygy, Brent + stationary-point refinement) | FIXED | dd293e9 |
| Event evaluation cost (position-only apparent place) | FIXED | dd293e9; engine_matrix 368 s -> 118 s |
| Light-time retarded JPL states rounded to 2^-31 day (40 us; 0.3 mas Moon) | FIXED | ad94650 (offset carried in day fraction) |
| IAU 2006/2000A nutation with P03 adjustments (eraNut06a); IAU2006A preset uses it | FIXED | ad94650, JME_MODEL_NUT_IAU_2006A = 351 |
| Light deflection: Sun only -> Sun + Jupiter + Saturn systems (+ Earth topocentric) | FIXED | f08bfb6 |
| JPL apparent places vs Skyfield/ERFA (ICRS, J2000, of-date, ecliptic; 130 places) | CERTIFIED | tests/test_apparent_reference.c, max 0.009 mas |
| Polar motion for topocentric observer / horizon frame | OPEN | finals2000A has x_p,y_p; needs pom00/sp00 vendoring |

## Commits (this session, newest first)
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
- Houses: Morinus cusp 10 is not the MC; Horizontal cusp 1 is not the Ascendant; MERIDIAN and MORINUS are distinct.

## Unresolved / documented limits
- Default frame models remain IAU 1976/1980 without frame bias (historical default); they differ from IAU 2006/2000A by ~0.3"/century (e.g. 52 mas in 2023). Recommendation: select IAU2006A for modern work; changing the default is a product decision.
- Laskar 1986 precession, SM1984/S1997/SM2004 Delta-T: explicit errors.
- Delta-T steps: 0.22 s at the 1973 IERS switch; 358 s at -720 (parabola/spline).
- Polar motion not applied to topocentric positions (< 15 m).
- Eclipse contact tangency evaluated on the axis silhouette (generator tilt 0.27 deg; < 0.2 s).

## Pending (in progress)
Position-only calls in rise/set and occultation paths, occultations, rise/set vs Skyfield, houses reference fixtures, docs (ACCURACY, TIME_SCALES, AYANAMSA_MODELS), CI matrix (CALCEPH, kernel cache, clang, macOS/Windows), PR description draft.
