# Time scales, ΔT and leap seconds

## 1. Scales and conversions (`src/timescale.c`, `jme_time_convert`)

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
| `JME_TIME_DELTAT_AUTOMATIC` (default) | IERS values inside the embedded table; SMHZ 2016/2021 before it; Espenak–Meeus 2006 after the last IERS prediction | see below |
| `JME_MODEL_DELTAT_STEPHENSON_ETC_2016` | splines for −720 ≤ y < 2019 and the long-term parabola −320 + 32.5((y − 1825)/100)² outside | Stephenson, Morrison & Hohenkerk (2016), Proc. R. Soc. A 472, 20160404, with the 2020 spline revision of Morrison, Stephenson, Hohenkerk & Zawilski (2021), Proc. R. Soc. A 477, 20200776, Table S15 |
| `JME_MODEL_DELTAT_ESPENAK_MEEUS_2006` | piecewise polynomials, −1999 … +3000 | NASA TP-2006-214141; https://eclipse.gsfc.nasa.gov/SEhelp/deltatpoly2004.html (all digits) |
| `JME_MODEL_DELTAT_STEPHENSON_MORRISON_1984`, `_STEPHENSON_1997`, `_STEPHENSON_MORRISON_2004` | **refused with `JME_ERR`** | The defining tables and their interpolation rules have not yet been transcribed from the primary publications and verified, so these models are not substituted with another one. |
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
  baseline (a_0): −26″/cy² for Espenak–Meeus 2006, −25.85″/cy² for
  SMH-2016/2021. The correction is
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
