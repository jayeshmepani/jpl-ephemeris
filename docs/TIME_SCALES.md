# Time scales, ΔT and leap seconds

## 1. Scales and conversions (`src/timescale.c`, `jme_time_scale_convert`)

All conversions work on **two-part Julian dates** (d1 + d2), so the
sub-microsecond resolution of binary64 is kept over the whole range.

| Conversion | Definition | Implementation |
|---|---|---|
| UTC ↔ TAI | TAI − UTC from the IERS leap-second table (from 1972) and the 1960–1971 drift expressions | Adapted from ERFA `eraDat`/`eraUtctai`/`eraTaiutc` (BSD-3, see THIRD_PARTY_NOTICES.md). A UTC day containing a leap second has 86 401 s; its two-part fraction follows the SOFA "quasi-JD" convention. |
| TAI ↔ TT | TT = TAI + 32.184 s exactly | IAU 1991 Res. A4; IAU 2000 Res. B1.9 |
| TT ↔ TCG | dTCG/dTT = 1/(1 − L_G), L_G = 6.969290134e−10 | IAU 2000 Res. B1.9 (ERFA `eraTttcg`/`eraTcgtt`) |
| TT ↔ TDB | TDB − TT from the full Fairhead & Bretagnon (1990) series, geocentre | ERFA `eraDtdb`, all terms. The inverse uses fixed-point iteration to binary64 convergence. |
| TDB ↔ TCB | IAU 2006 Res. B3: L_B = 1.550519768e−8, TDB0 = −6.55e−5 s | ERFA `eraTdbtcb`/`eraTcbtdb` |
| UT1 ↔ TT | TT − UT1 = ΔT of the context model (§2) | The inverse (TT → UT1) iterates to a residual below 1e−9 s. |

Notes:

* The topocentric term of TDB − TT (at most 2 µs) is not applied. The
  engine's TDB is the geocentric TDB that the JPL ephemerides use as their
  argument. The term is a documented limit, not an approximation of the
  geocentric scale.
* `eraDtdb` is memoised per thread on the exact input pair. A cache hit
  returns the bit-identical value, so results do not depend on the memo.
* Failures never clamp or extrapolate silently: a date outside a table or
  model, or a non-finite input, returns `JME_ERR` with a message.

## 2. ΔT models (`src/time.c`, `jme_delta_t_ex`)

| Model | Coverage | Source |
|---|---|---|
| `JME_TIME_DELTAT_AUTOMATIC` (default) | IERS values inside the embedded table; the SMH 2021 revision before it; Espenak–Meeus 2006 after the last IERS prediction | see below |
| `JME_MODEL_DELTAT_STEPHENSON_ETC_2016` | Table S15 as published (54 rows, −720.0 … 2016.0), row rule S5.1 K_i ≤ Y < K_i+1 with equality only at Y = 2016.0; outside, the parabola −320 + 32.5((Y − 1825)/100)² (eq. 4.1) | Stephenson, Morrison & Hohenkerk (2016), Proc. R. Soc. A 472, 20160404, official ASCII Table-S15.txt (SHA-256 b972e922…, `tools/data/Table-S15.2016.txt`) and electronic supplement S5 |
| `JME_MODEL_DELTAT_STEPHENSON_ETC_2021` | 2020 revision of Table S15 (58 rows, −720 … 2019), K_i ≤ Y < K_i+1; same parabola outside | Morrison, Stephenson, Hohenkerk & Zawilski (2021), Proc. R. Soc. A 477, 20200776 (`tools/data/Table-S15.2020.csv`) |
| `JME_MODEL_DELTAT_ESPENAK_MEEUS_2006` | piecewise polynomials, −1999 … +3000 | NASA TP-2006-214141; https://eclipse.gsfc.nasa.gov/SEhelp/deltatpoly2004.html (all digits) |
| `JME_MODEL_DELTAT_MS2004_TABLE1_LINEAR` | linear interpolation, in the calendar decimal year, between the 58 rows of Table 1 (−1000 … +2000); outside it, the paper's long-term parabola −20 + 32((y − 1820)/100)² | Morrison & Stephenson (2004), J. Hist. Astron. 35, 327, Table 1 p. 332. Transcription verified row by row against the ADS scan (`tools/data/MS2004_Table1.txt`). The authors write (p. 331) that "intermediate figures for ΔT can be found with adequate precision by simple interpolation". This is a reconstruction from their rounded table; it is **not** their spline curve. Reference ṅ = −26.0″/cy² (p. 332). The parabola does not join the table: there is a 27.7 s step at −1000 and an 18.7 s step at +2000. |
| `JME_MODEL_DELTAT_STEPHENSON_MORRISON_2004` | **refused with `JME_ERR`** | Blocked by source: the cubic-spline curve behind Table 1 was never published in coefficient form. Use `MS2004_TABLE1_LINEAR`, which is honestly named, for the tabulated reconstruction. |
| `JME_MODEL_DELTAT_STEPHENSON_MORRISON_1984`, `_STEPHENSON_1997` | **refused with `JME_ERR`** | Blocked by an external dependency: the primary publications (Stephenson & Morrison 1984, Phil. Trans. R. Soc. A 313, 47; Stephenson 1997, *Historical Eclipses and Earth's Rotation*, CUP) were not accessible for transcription. Stephenson 1997 tabulates values; its −20 + 32t² long-term parabola alone is not the model, and no substitute is used. |
| `jme_set_delta_t_userdef(seconds)` | constant ΔT for every date | overrides the model; a non-finite argument (e.g. `NAN`) clears the override |

Details:

* **IERS segment.** ΔT = 32.184 s − (UT1 − TAI). UT1 − TAI is tabulated
  daily at 0h UTC from IERS `finals2000A.all`:
  * Bulletin B (IERS C04 final) values where present, otherwise Bulletin A;
  * Bulletin A predictions after the last observed day.
  UT1 − TAI is continuous across leap seconds. Values between days use
  4-point Lagrange interpolation in MJD, as in the IERS `interp.f` scheme.
  The sub-daily ocean-tide and libration terms are not included; they are
  below 0.1 ms. `jme_delta_t_iers_table_info()` reports the first day, the
  last observed day and the last predicted day.
* **Argument of Espenak–Meeus.** NASA's prescription
  y = year + (month − 0.5)/12 is the mid-month value of the continuous
  calendar decimal year. JME uses the continuous form, which removes the
  month-boundary steps. On the 15th of each month it agrees with the NASA
  values to the precision of their tables.
* **Model joins.** The SMHZ parabola does not join the spline at −720 or at
  2019. These are the published models; the jumps are kept, not smoothed.
* **Lunar tidal acceleration.** `jme_set_tid_acc()` accepts either a finite
  lunar mean-longitude tidal acceleration in ″/cy², or a named
  `JME_TIME_TIDAL_*` selector. Each DE selector resolves to the value
  published for that JPL lunar ephemeris:

  | Selector | ″/cy² | Primary source |
  |---|---|---|
  | DE200 | −23.8946 | ELP 2000-82 / LE200 (Chapront-Touzé & Chapront 1983, A&A 124, 50); −23.895 in Chapront, Chapront-Touzé & Francou 2002, A&A 387, 700, Table 7 |
  | DE403, DE404 | −25.580 | Chapront et al. 2002, Table 7 (DE404 is the long-span companion of DE403, Standish et al. 1995, JPL IOM 314.10-127) |
  | DE405, DE406 | −25.826 | Chapront et al. 2002, Table 7 (DE406 is the long-span companion of DE405, Standish 1998, JPL IOM 312.F-98-048) |
  | DE421 | −25.85 | Williams, Boggs & Folkner 2008, JPL IOM 335-JW,DB,WF-20080314-001 |
  | DE430 | −25.82 | Folkner et al. 2014, IPN Progress Report 42-196 |
  | DE431 | −25.80 | Folkner et al. 2014, IPN Progress Report 42-196 |
  | DE441 | none published | Park et al. 2021 (AJ 161, 105) say only that the tidal acceleration is implicit in the time-delay tide model |

  The DE441 selector is **unresolved**. Pre-1955 Espenak–Meeus or SMH ΔT
  (including AUTOMATIC's SMH branch) returns NaN with an explicit error.
  It does not use an unpublished number. A figure of −25.936 circulates,
  attributed to private correspondence, but JME does not adopt it. To
  use it, or any other value, pass it numerically. `jme_get_tid_acc()`
  returns the selector unchanged.
* **Historical correction.** When an acceleration (a) is selected
  explicitly, ΔT before 1955 is corrected relative to the model's own
  baseline (a_0): −26″/cy² for Espenak–Meeus 2006 and −25.82″/cy² for
  SMH 2016/2021 (the DE430 value used to reduce their data, Stephenson et
  al. 2016 §2(a)). The 2021 revision keeps that reduction. The correction is
  c = −0.91072 (a − a_0) ((y − 1955)/100)² s
  (NASA [secular-acceleration correction](https://eclipse.gsfc.nasa.gov/LEcat5/secular.html)).
  Here y is the model's own decimal year: the calendar year for EM-2006
  (Julian calendar before 1582 Oct 15), the Julian epoch year for SMH.
  Nothing is corrected from 1955 onward, in the embedded IERS
  observational/prediction table, or when the user has fixed ΔT
  (`jme_set_delta_t_userdef`). Each exclusion is covered in
  `tests/test_delta_t_reference.c`.
* **AUTOMATIC selector.** `JME_TIME_TIDAL_AUTOMATIC` applies no correction:
  the published base models are used unchanged. Inferring a value from the
  loaded JPL kernel was evaluated and rejected, for two reasons. First, SPK
  kernels carry no machine-readable DE identity: CALCEPH reports no file
  version and no `DENUM` for `de440s.bsp`, so the inference would have to
  parse free-text comments. Second, the current kernels (DE440/DE441) have
  no published value (see above). Selecting the matching DE selector, or a
  number, is the user's explicit choice.


## 3. Leap-second table update

The TAI − UTC table (`jme_utc_offsets[]` in `src/time.c`, mirrored in
`LEAPS` in `tools/gen_deltat.py`) is valid until the expiry date in
`src/time.c`. That expiry is currently 2027-06-30 23:59:59 UTC, following
IERS Bulletin C 72 (6 July 2026: no leap second at the end of December
2026). After each new Bulletin C:

1. Add any announced leap second to `jme_utc_offsets[]` and to `LEAPS`.
2. Move `JME_UTC_VALID_UNTIL_YEAR`/`_MONTH` in `src/time.c` (and its comment) to the last day the new bulletin guarantees.
3. Regenerate the time fixtures against pyerfa (`tools/gen_time_fixtures.py`)
   and run `ctest` (`time_scale_contract`, `delta_t_reference`).

Dates after the expiry return `JME_ERR` from UTC conversions; they are never
extrapolated.

## 4. IERS ΔT table update

1. Download `https://datacenter.iers.org/data/9/finals2000A.all` into
   `tools/data/`.
2. Review the file, then update `FINALS_SHA` in `tools/gen_deltat.py`. The
   generator refuses to run while the digest does not match.
3. `python3 tools/gen_deltat.py > src/deltat_data.inc`. When pyerfa is
   installed, the generator cross-checks its leap-second list.
4. Rebuild and run `ctest`. The ΔT fixtures (`tools/gen_deltat_fixtures.py`)
   compare the IERS segment with astropy/ERFA.

## 5. Polar motion (`src/eop.c`, `jme_eop_*`, `jme_set_polar_motion`)

* **Data.** `jme_eop_load_file(path)` loads one of two IERS series and
  replaces any series already loaded. The format is detected line by line.
  * finals2000A (`finals2000A.all` / `.data` / `.daily`, from
    https://datacenter.iers.org/): fixed columns per `readme.finals2000A`.
    These are MJD (8–15), the polar-motion flag (17, `I` or `P`), and
    Bulletin A PM-x (19–27) and PM-y (38–46) in arcseconds. Reading stops at
    the first row without polar-motion values. Prediction rows (`P`) are
    used and reported through the `predicted` output of
    `jme_eop_polar_motion`.
  * EOP 20 C04 (`eopc04.1962-now`) and the earlier 14 C04 layout: '#' or
    text header lines are skipped. MJD is the first field with a decimal
    point, followed by x and y.
* **Interpolation.** 4-point (cubic) Lagrange interpolation in MJD over
  four consecutive daily rows, two on each side of the requested instant.
  This is the IERS `interp.f` (LAGINT) scheme. A date without two rows on
  each side, or with a gap in the series, is an error. The series is never
  extrapolated.
* **Time argument.** The UT1 date is used as the MJD argument. The series
  is tabulated at 0h UTC, and |UT1 − UTC| < 0.9 s changes xp and yp by
  less than 0.1 µas.
* **Application.** With `jme_set_polar_motion(1)`, the observer's ITRS
  (WGS84) vector becomes r_true = R3(−GAST) Wᵀ r_itrs, where
  W = `eraPom00(xp, yp, s′)` and s′ = `eraSp00(TT)`. This is the
  equinox-based form of IERS Conventions (2010) eq. 5.1. The same W is used
  in three places:
  * every topocentric place (`JME_CALC_TOPOCENTRIC`, `jme_get_topo_pos*`);
  * the local horizon used by rise/set/transit and eclipse altitudes;
  * eclipse and occultation geographic mappings. Their shadow geometry is
    evaluated in the frame R3(−GAST) W R3(GAST), whose z axis is the ITRS
    pole. Contact times are rotation invariant.
* **No silent default.** The default is off (`jme_set_polar_motion(0)`).
  Off means no polar motion, and no s′ either; results are unchanged from
  earlier releases. When the flag is on and no loaded series covers the
  date, every affected computation returns an error naming the missing EOP
  data. Zero polar motion is never assumed.
* **Certification.** `tests/test_polar_motion_reference.c` checks:
  * the observer GCRS vector against pyerfa `eraC2t06a` ×
    `eraGd2gc(WGS84)`, using IERS finals2000A data for 2020, 2022 and 2024
    (four sites, three times of day, model set `IAU2006A`, ΔT fixed). The
    maximum difference is 0.006 mm; without polar motion it is 0.31 mm,
    which is the s′ term. The polar-motion effect is at least 5.6 m.
  * that interpolation of both file formats matches an independent numpy
    implementation to 1e-12″;
  * the error paths;
  * the horizon path: zero xp/yp reproduces the hour-angle result, and real
    EOP moves a Delhi sunrise by −0.015 s.
* **Measured limitation.** The sub-daily ocean-tide and libration
  corrections to polar motion (IERS Conventions 2010 §5.5.1, Tables
  5.1a/5.1b) are not added. Their amplitude is below 0.5 mas, which is
  1.5 cm at the Earth's surface.
