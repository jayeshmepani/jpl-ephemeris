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
* **Lunar tidal acceleration.** `jme_set_tid_acc()` accepts a finite
  lunar mean-longitude secular acceleration in arcseconds per century squared,
  or a named `JME_TIME_TIDAL_*` selector. The DE selectors resolve to:
  DE200 −23.8946; DE403/DE404 −25.580; DE405/DE406 −25.826; DE421 −25.85;
  DE430 −25.82; DE431 −25.80; DE441 −25.936. The DE441 figure is an
  unpublished value attributed to an email from J. Giorgini in the Swiss
  Ephemeris public header, not a value independently published by JPL.
* **Historical correction.** For explicitly selected acceleration (a), the
  Espenak–Meeus 2006 model is corrected before 1955 relative to its published
  −26″/cy² baseline, and the SMH-2016/2021 model relative to its −25.85″/cy²
  baseline. The correction is
  (c=-0.91072(a-a_0)((y-1955)/100)^2) seconds, where (a_0) is the
  respective model's baseline and (y) is its continuous calendar decimal
  year. No correction is applied to dates from 1955 onward, to the embedded
  IERS observational/prediction table, or to a user-defined fixed ΔT.
* **Automatic selector limitation.** `JME_TIME_TIDAL_AUTOMATIC` clears the
  explicit correction and restores the base published model values. JME does
  not yet infer a kernel-specific value from the currently loaded JPL file or
  automatically select a different acceleration by analytical engine. The
  configured input/selector is preserved by `jme_get_tid_acc()`; non-finite
  setter inputs are ignored. Selector values follow the documented
  DE-specific values in the
  [Swiss Ephemeris public header](https://sources.debian.org/src/libswe/2.10.03-3.1/swephexp.h/)
  and the NASA [secular-acceleration correction](https://eclipse.gsfc.nasa.gov/LEcat5/secular.html).

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
