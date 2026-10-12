# Ayanamsa models

This document defines every sidereal model in `src/sidereal.c` (`JME_SIDEREAL_*`).
For each model it gives the reference epoch, the reference value, the source,
and why the model is mapped the way it is.

**Sources.** Model definitions (epochs and values) are conventions of the
astrological traditions concerned. Where the original publication is
available, it is cited. Many of these conventions are catalogued in the
*Swiss Ephemeris General Documentation*, §2.8 "Sidereal ephemerides"
(Astrodienst, https://www.astro.com/swisseph/swisseph.htm). That
**documentation** was read for the *definitions of the conventions only*.
No Swiss Ephemeris source code was consulted, and none of the code here is
derived from it. All numerical procedures (precession, nutation, frame
rotations, star propagation) are implemented from IAU/IERS/ERFA and the
publications cited below.

## 1. Conventions

### 1.1 Mean and true ayanamsa

* `jme_get_ayanamsa_ex()`, `jme_get_ayanamsa_ex2(..., JME_AYANAMSA_MEAN, ...)`
  return the **mean** ayanamsa. This is the longitude of the sidereal origin,
  measured from the **mean equinox of date** along the mean ecliptic of date.
* `JME_AYANAMSA_TRUE` returns **mean + Δψ**. Δψ is the nutation in longitude
  from the context nutation model (`jme_get_nutation`). The result is
  measured from the **true equinox of date**.
* Sidereal positions (`JME_CALC_SIDEREAL`) subtract the ayanamsa that matches
  the equinox of the tropical output:
  * the true ayanamsa is used for true-of-date (nutated) output;
  * the mean ayanamsa is used for `JME_CALC_NO_NUTATION` output.
  A sidereal longitude is therefore independent of whether nutation was
  applied.
* `rate`, when requested, is the time derivative of the **mean** ayanamsa
  (degrees per TT day):
  * it is analytic for epoch models;
  * it is `NAN` for star-anchored models, whose rate the caller can obtain by
    finite differences of the anchor position.

### 1.2 Epoch-defined models

A model defined by a value A(t0) at epoch t0 is propagated as

    A(t) = A(t0) + P(t0 -> t)

P is the IAU 1976 general precession in longitude between two arbitrary
epochs (J. H. Lieske, T. Lederle, W. Fricke, B. Morando, A&A 58, 1–16, 1977).
It is used in the two-epoch form published in *The Indian Astronomical
Ephemeris 1989*, p. 525:

    P = (5029.0966 + 2.22226 T - 0.000042 T^2) t
        + (1.11161 - 0.000127 T) t^2 - 0.000113 t^3      [arcsec]
    T = (t0 - J2000.0)/36525,  t = (t - t0)/36525  (TT, Julian centuries)

This is the complete published expression; no term is dropped. IAU 1976 is
used deliberately, even though the rest of the engine uses IAU 2006, because:

* the reference values of the Indian models (Lahiri, IAE) were **defined**
  with this precession;
* the 20th-century Western models were defined with Newcomb's precession,
  which IAU 1976 reproduces to well below 0.1″ over the 20th century.

Propagating such a value with a different precession model would change the
convention.

### 1.3 Star-anchored ("true") models

For a model that keeps star S at sidereal longitude λs:

    A(t) = λ(S, t) - λs

λ(S, t) is the longitude of the star's **geocentric true position** on the
mean ecliptic and equinox of date. It is computed with the full catalogue
propagation in `src/fixstar.c`:

* Hipparcos (ESA 1997) astrometry, with van Leeuwen 2007 where available;
* proper motion, parallax and radial velocity rigorously propagated
  (ERFA `pmsafe`);
* the BSC5 FK5 fallback rotated to ICRS (ERFA `fk52h`);
* IAU 2006 precession and frame bias to the mean equator and equinox of date;
* the mean obliquity of the context model to the ecliptic.

Annual aberration and light deflection are **not** applied. The anchor is a
direction in space, not an apparent place. Including them would make the
sidereal origin oscillate by about ±20″ over the year.

### 1.4 Historical epochs

* Epochs stated in UT are converted to TT with the context ΔT model
  (`internal_ut_to_et`).
* Dates before 1582-10-15 are Julian-calendar dates.
* Year numbers are astronomical: −100 = 101 BCE.

## 2. Models

| Constant | Definition implemented | Source / justification |
|---|---|---|
| `FAGAN_BRADLEY` | 24°02′31.36″ at 1950.0, taken as B1950.0 = JD 2433282.42345905 TT; §1.2 | C. Fagan & R. C. Firebrace, *A Primer of Sidereal Astrology* (1971): the "synetic vernal point", with Spica at 29 Vir 06′05″ without aberration. The epoch is stated as "1950", which the Swiss documentation gives as 1 Jan 1950. B1950.0 (1949 Dec 31 22:09 UT) is the Besselian epoch the 1950s ephemerides used; the difference from 1950 Jan 1 0h is 1.8 h, or 0.01″ of precession. |
| `LAHIRI` | **True** ayanamsa 23°15′00.658″ at 1956-03-21 0h ET (= TT, JD 2435553.5). Mean value at t0 = true − Δψ(IAU 1980, t0); §1.2. | *Indian Astronomical Ephemeris 1989*, p. 556 footnote: the ICRC value revised with effect from 1985. The value is a true ayanamsa, as the yearly IAE true-ayanamsa tables show (also argued in the Swiss documentation §2.8.5). IAE uses IAU 1976 precession and IAU 1980 nutation, so the IAU 1980 Δψ removes nutation exactly as the definition requires. |
| `LAHIRI_ICRC` | As `LAHIRI`, with 23°15′00″ | M. N. Saha & N. C. Lahiri, *Report of the Calendar Reform Committee*, CSIR, New Delhi, 1955. Also used by the IAE/IENA before 1985. |
| `KRISHNAMURTI` | A = 80564.38104″ + 50.2564″ t + 0.000111″ t², where t is in tropical (Besselian) years of 365.242198781 d from B1900.0 = JD 2415020.31352 | **Provenance flagged: no primary source.** K. S. Krishnamurti's *KP Reader 1* (pp. 55–59) could not be obtained. Secondary sources (logicastro.com, "ayanamsa" page; V. Subramanian, *Ayanamsa*, logicastro.com PDF) report three things: Krishnamurti followed Newcomb's precession; he took 291 CE as the year of coincidence; and he tabulated values for 1840–2001, adding 50.2388475″ per year after 2001. The engine's formula integrates Newcomb's general precession rate 50.2564″ + 0.000222″ t per year, which is zero in 291.22 CE. It is a reconstruction consistent with that description, not a transcription of Krishnamurti's table. It is **54.38″ larger** at 1900 than the table-fitted value 22.363889° at 1 Jan 1900, as defined in the Swiss Ephemeris documentation §2.8.6, model 5 (used for the definition only). Two other definitions exist and are **not** provided under this name: M. C. Khare's 1978 "KP straight line" (Y − 291) × 50.2388475″, and S. P. Khullar's formula. For the table-fitted variant, use `JME_SIDEREAL_USER` with t0 = JD 2415020.5 and A0 = 22.363889. |
| `RAMAN` | (Gregorian year − 397) × 50⅓″, constant within a calendar year | B. V. Raman, *Hindu Predictive Astrology*, pp. 378–379 (whole-year rule as used by Raman's tables). The Swiss documentation mentions 389 CE as Raman's zero year in one place. JME keeps its documented whole-year rule (zero year 397). |
| `YUKTESHWAR` | (Gregorian year − 499) × 54″, whole-year rule | Sri Yukteswar, *The Holy Science*: zero in 499 CE with 54″/yr (360°/24 000 yr). His stated 20°54′36″ for 1894 follows from this rule (it actually applies to 1893). |
| `ARYABHATA` | 0 at 499-03-21 06:56:55.57 UT (Julian); §1.2 | Kali 3600 counted with the Aryabhata year from sunrise of the Kaliyuga epoch: the mean Sun at the mean equinox (Swiss documentation §2.8.4, model 23; Simon et al. 1994 mean elements). |
| `SURYASIDDHANTA` | 0 at 499-03-21 07:30:31.57 UT; §1.2 | Kali 3600 with the Surya Siddhanta year from midnight (Burgess, *Surya Siddhanta*; Swiss documentation §2.8.4, model 21). |
| `SS_REVATI` | −0.79167046° at the SS epoch above | ζ Psc (Revati) at **polar** longitude 359°50′ in 499 CE (Surya Siddhanta star list; Swiss documentation model 25). Defined as an epoch value; the polar projection is already in the value. |
| `SS_CITRA` | +2.11070444° at the SS epoch above | Spica (Citra) at polar longitude 180° in 499 CE (model 26). |
| `USHASHASHI` | 18°39′39.46″ at 1900-01-01 0h UT | Ushā-Shashī value, Revati-based zodiac (Mercier 1976/77; Swiss documentation model 4). |
| `BABYL_KUGLER1/2/3` | −5°40′, −4°16′, −3°25′ at −100 Jan 1 (Julian) | F. X. Kugler's three groups of Babylonian positions, as summarised by R. Mercier, *Arch. Int. Hist. Sci.* 26 (1976) 197 and 27 (1977) 33. |
| `BABYL_HUBER` | −4°28′ at −100 | P. Huber, *Centaurus* 5 (1958) 192–208. The corrected value; the −4°34′ found in some literature is a transcription error, as noted in the Swiss documentation. |
| `BABYL_ETPSC` | −5°04′46″ at −129 | η Psc culminating with the zero point (Mercier 1977). |
| `DELUCE` | 26°24′47″ at 1900-01-01 0h TT | R. DeLuce, *Constellational Astrology According to the Hindu System* (1963), p. 5: the value for 1900 (zero about 4 June 1 BCE). |
| `HIPPARCHOS` | −9°20′ at JD 1674484.0 (27 June −128) | Mercier's Hipparchan zero point (ζ Psc at 29 Psc 33′49″). |
| `SASSANIAN` | 0 at JD 1927135.8747793 TT (18 Mar 564, 7:53:23 UT) | The Zīj al-Shāh reform (Mercier 1977; Swiss documentation model 16). |
| `JN_BHASIN` | 0 at JD 1854239.3 | **Provenance flagged:** the epoch of J. N. Bhasin's ayanamsa as historically carried in the Nova/Solar Fire tradition. No primary publication located. |
| `J2000`, `J1900`, `B1950` | 0 at JD 2451545.0, 2415020.0 and 2433282.42345905 (TT) | Sidereal frame aligned to the mean equinox of the standard epoch. |
| `ALDEBARAN_15TAU` | Aldebaran (α Tau) at 45°00′ (15 Tau); §1.3 | Bradley's proposed fiducial (Fagan; Swiss documentation model 14). |
| `TRUE_CITRA` | Spica (α Vir) at 180°; §1.3 | "True Chitrapaksha": Spica always at 0° Libra in ecliptic longitude. |
| `TRUE_REVATI` | ζ Psc at 359°50′; §1.3 | Surya Siddhanta longitude of Revati (non-polar form). |
| `TRUE_MULA` | Shaula (λ Sco) at 240° | Mula nakshatra start at 0 Sag (Chandra Hari). |
| `TRUE_PUSHYA` | δ Cnc at 106°; §1.3 | P. V. R. Narasimha Rao, "Introducing Pushya-paksha Ayanamsha" (2013). |
| `GALCENT_0SAG` | Sgr A* at 240° (0 Sag) on the mean ecliptic of date | Position 17h45m40.0409s, −29°00′28.118″ (J2000/ICRS frame; M. J. Reid & A. Brunthaler, ApJ 616, 872, 2004), rotated by IAU 2006 bias-precession to the mean equator of date, then to the mean ecliptic. The model was introduced in 1999 at D. Koch's suggestion (Swiss documentation §2.8.7). See §3 for the proper motion. |
| `USER` | `jme_set_sidereal_mode(JME_SIDEREAL_USER, t0, ayan_t0)` | §1.2 with the caller's t0 and A(t0). t0 = 0 gives a constant ayanamsa (documented legacy behaviour). |

## 3. Known limits and open points

* **Galactic Centre convention (settled).** `GALCENT_0SAG` places Sgr A\* at
  240° (0° Sagittarius) on the mean ecliptic of date. This is the Swiss
  documentation's model 17, introduced in 1999 at D. Koch's suggestion. The
  documentation was consulted for the definition only. Rafael Gil Brand's
  model (Galactic Centre at the golden section between 0° Scorpio and
  0° Aquarius), Cochrane's (0° Capricorn) and Wilhelm's Dhruva/mid-Mula
  model are different definitions. JME does not provide them; no
  `GALCENT_0SAG` value is reused for them.
* **Reference position and epoch.** RA 17h45m40.0409s, Dec −29°00′28.118″,
  the J2000 phase-reference position of Reid & Brunthaler (2004, ApJ 616,
  872, Table 1 note). It is a nominal position, not a fitted one. A weighted
  linear fit to the paper's Table 1 J1745−283 east offsets
  (`3.151 mas/yr`, which reproduces Table 2) gives zero offset at
  **1996.08**. In right ascension, therefore, the nominal position is
  Sgr A\*'s position at about 1996.1. At J2000 the source lies 12.4 mas east
  of it. That is a 0.01″-level effect on the ayanamsa. The text of the
  north-offset column could not be extracted, so no northerly epoch is
  claimed.
* **Proper motion: deliberately not applied.** The apparent motion
  (μα cos δ = −3.151, μδ = −5.547 mas/yr; ℓ: −6.379, b: −0.202 mas/yr,
  Table 2) is the Sun's Galactic orbit reflected onto Sgr A\*. The
  Galactic frame (IAU ℓ = b = 0) is non-rotating with respect to the ICRS.
  A "Galactic Centre" anchor therefore stays fixed in ICRS, and the model
  uses a constant ICRS direction. Following the moving radio source instead
  would change the ayanamsa by at most 6.4 mas/yr. That is 0.64″ per
  century, or 32″ at ±5000 years from J2000. This is a definition choice,
  documented here, not an omitted term.
* **Certification.** `tests/test_galcent_reference.c` compares the engine
  with independent pyerfa values (`tools/gen_galcent_fixtures.py`) at 11
  epochs from −3000 to +3000. Two model sets are checked: the default
  (no bias, `eraPmat76`, `eraObl80`) and `IAU2006` (`eraEcm06`).
  Tolerance is 1 µas; the measured agreement is ~1e-12°.
* **Models whose sources are flagged** (`KRISHNAMURTI`, `JN_BHASIN`) are
  kept bit-compatible with earlier releases until a primary source is found.
* The precession of §1.2 is accurate to the IAU 1976 model's validity
  (several centuries). Epoch models far in the past (Babylonian,
  Hipparchan) inherit the conventions of their definitions, not modern
  long-term precession.
