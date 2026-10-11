# Accuracy

This document states, for each part of the engine:

* what the engine computes, and to which model;
* how it was verified (the CTest that certifies it, against which
  independent reference);
* where binary64 or model limits set the floor.

Figures are maxima over the fixtures unless stated otherwise.

## 1. Principles

* **Complete published models.** Series are evaluated with all of their
  published terms. This covers:
  * VSOP87A (42 520 terms);
  * ELP 2000-82B;
  * IAU 2000A/2006 nutation;
  * Fairhead–Bretagnon TDB;
  * the Hipparcos catalogue.

  Nothing is rounded or truncated. Where a value can only be represented to
  binary64 precision, the limit is stated here.
* **No silent substitution.** A model, body or centre that is not available
  is an error (`JME_ERR` with a message), not a fallback. The single opt-in
  exception is the JPL centre policy (§3).
* **Memoisation is exact.** The TDB−TT and nutation series are memoised per
  thread on the exact input doubles. A hit returns the bit-identical value,
  so caching never changes a result.

## 2. Apparent places (JPL engine)

The pipeline (ERFA-based, `src/calc.c`, `src/astrometry.c`) applies:

* light time iterated to convergence, with the retarded epoch carried as a
  two-part date, not rounded;
* gravitational deflection by the Sun, Jupiter and Saturn systems, plus the
  Earth for topocentric observers;
* special-relativistic annual aberration with the barycentric observer
  velocity (diurnal aberration included for topocentric observers);
* frame bias, precession and nutation of the selected models (IAU 2006 /
  IAU 2000A with the P03 adjustments for the `IAU2006A` preset).

| Check | Reference | Result |
|---|---|---|
| 130 apparent places: ICRS, J2000, of date, ecliptic (`apparent_reference`) | Skyfield + ERFA, DE440s | max 0.0087 mas |
| Fixed stars, 5 frames incl. FK5-only stars (`star_reference`) | ERFA `pmsafe`/`atci13`/`fk52h` | max 0.0004 mas |
| VSOP87A, 90 official check epochs (`vsop87_reference`) | IMCCE `vsop87.chk` | 5e−11 au |

**Rates** come from a five-point central difference of the complete
reduction (step 0.01 d). The truncation error is about 1e−11 relative,
below the ~1e−8 relative noise floor of binary64 series evaluation.

**Default frame models.** For compatibility, the defaults remain IAU 1976
precession and IAU 1980 nutation without frame bias. They differ from IAU
2006/2000A by about 0.3″ per century (52 mas in 2023). For modern work,
select the `IAU2006A` model token (`jme_set_astro_models("IAU2006A", 0)`).

## 3. JPL centres

The DE4xx planetary kernels hold system **barycentres** for Mars to Pluto;
the planet centres are in the satellite kernels (`mar097`, `jup365`, …).

* The default policy, **STRICT**, refuses a planet-centre request that the
  loaded kernels cannot answer.
* `jme_set_jpl_center_policy(JME_JPL_CENTER_BARYCENTER)` (model token
  `JPL_CENTER=BARYCENTER`) opts in to the system barycentre. This is never
  silent: `jme_calc_resolve_engine` reports the substitution.

The barycentre-to-centre offset as seen from the Earth is approximately:

| System | Offset |
|---|---|
| Mars | < 0.001 mas |
| Jupiter | ≤ 0.08″ (Galilean moons, ~230 km) |
| Saturn | ≤ 0.05″ (Titan) |
| Uranus | ≤ 0.004″ |
| Neptune | ≤ 0.004″ (Triton) |
| Pluto | ≤ 0.11″ (Charon, ~2 350 km) |

## 4. Time

See `TIME_SCALES.md`.

* TT↔TDB uses the full Fairhead–Bretagnon series. The topocentric term
  (≤ 2 µs) is not applied.
* UTC conversions refuse dates beyond the leap-second table.
* ΔT model joins are published discontinuities, kept as published:
  * 0.22 s at the 1973 IERS start;
  * 358 s at −720 (SMHZ parabola/spline).
* ΔT itself is the dominant uncertainty for historical UT. The SMHZ
  uncertainty is about 1 h at −1000.

## 5. Events

| Event | Construction | Verification |
|---|---|---|
| Rise, set, transit | Illinois refinement to 1e−10 d on full topocentric apparent places with refraction (`riseset_reference`) | Skyfield, 320 events: worst 0.16 ms |
| Solar eclipses, global | Generalised Besselian elements: WGS84 3-D observer, P1/P4, U1/U4, C1/C2, hybrid transitions | NASA canon (10 eclipses): within 0.5 s and 0.1° |
| Solar eclipses, local | Per-instant topocentric radii; contacts by root finding | (with the occultation fixture, §5.1) |
| Lunar eclipses | Danjon shadow: Earth radius at latitude 45° (0.998340 a), shadow radii enlarged by 1/85 | NASA canon (8 eclipses): 1 s, magnitude 0.0006 |
| Occultations of planets and stars by the Moon (`occult_reference`) | Generalised Besselian construction (§5.1). Stars use a cylinder shadow; local contacts by Illinois root finding | Skyfield + DE440s reference generator: see §5.1 |
| Houses (`houses_reference`) | Closed forms from the house definitions | Independent vector geometry, 2354 sets: 2.3e−13° |

### 5.1 Contact geometry for eclipses and occultations

A contact is the instant at which the limbs of two bodies are tangent as
seen by the observer. The exact condition can be stated in the barycentric
frame: the angle between the **astrometric** directions (light time and
deflection, **no aberration**) equals the sum or difference of the
geometric angular radii asin(R/d). Aberration is a direction-dependent
transformation of the sky, so it does not preserve angles between two
directions:

* differential aberration of the pair is κ·sep ≈ 0.09″, about 0.15 s;
* using the aberrated geocentric Moon vector as a *position* in the
  Besselian frame shifts the shadow by (R⊕/d)·20″ ≈ 0.3″, about 0.6 s.

The solar-eclipse and occultation contact code therefore uses astrometric
places (`JME_EVENTS_GEOMETRY_FLAGS` in `src/events.c`). Altitudes used for
visibility keep full apparent places with refraction.

Lunar-eclipse contacts keep their construction because two terms of the
same size cancel:

* the Earth's shadow lags by the light time of the Sun–Earth–Moon path
  (about 41″);
* the aberration terms are equal in size.

The resulting contacts match the NASA canon to 1 s. A rigorous retarded
shadow model is listed as a verification item.

### 5.2 Floors

Contact times are roots of a smooth function evaluated with ~1e−12 relative
noise. Near grazing contacts, the root becomes ill-conditioned: the time
error grows as the inverse of the relative velocity across the limb, and at
exact grazing the contact pair merges and disappears.
