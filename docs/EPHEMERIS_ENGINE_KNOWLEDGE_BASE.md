# Ephemeris Engine Knowledge Base

This document captures the design knowledge discussed while shaping this project.

The core question was:

> If JPL/CALCEPH is the primary high-precision backend, what do Meeus, Moshier, VSOP87, ELP2000, SOFA/ERFA, IERS, and catalog data contribute, and how do they fit together?

This file is meant to help future contributors understand the architecture without rediscovering the same decisions.

## Short Answer

For this project, the clean architecture is:

```text
Primary precision layer:
JPL kernels through CALCEPH

Analytical fallback layer:
VSOP87 + ELP2000/ELP82/ELPMPP02 + Meeus + Moshier

Reference-frame/time layer:
SOFA/ERFA + IAU models + IERS/EOP data

Catalog layer:
fixed stars + asteroids + comets + satellites

Astrology layer:
ayanamsa + houses + rise/set + eclipses + panchang/muhurta rules
```

The most important conclusion:

```text
Moshier < VSOP87 + ELP2000 + Meeus
VSOP87 + ELP2000 + Meeus < full astronomy/astrology engine
```

Moshier is useful, but it is not the whole universe. VSOP87, ELP, and Meeus together form a broader analytical fallback base than Moshier alone. Even that set is not enough for a complete astronomy/astrology engine.

## What Is Meeus?

Meeus means **Jean Meeus**, the author of *Astronomical Algorithms*.

In astronomy software, "Meeus" usually means formula-based algorithms from that book or that style of computational astronomy. These are compact, practical formulas for many common astronomical calculations.

Typical Meeus-style areas:

- Julian Day conversion
- Gregorian and Julian calendar conversion
- date validation
- day of week
- day of year
- Delta-T estimates
- mean sidereal time
- apparent sidereal time
- solar position
- lunar position
- approximate planetary positions
- nutation
- obliquity
- precession helpers
- rise, set, and transit
- lunar phases
- elongation
- illumination
- angular separation
- position angle
- eclipses
- atmospheric refraction

In this project, Meeus is best treated as:

```text
calendar/time/general astronomy formula layer
utility layer
lightweight fallback layer
```

Meeus is not the same as JPL. Meeus formulas are analytical approximations and utility algorithms. JPL kernels are numerical ephemeris data produced for high-precision planetary/lunar positions.

## JPL/CALCEPH vs Meeus

| Area | JPL/CALCEPH | Meeus |
|---|---|---|
| Nature | Numerical ephemeris kernel access | Formula-based algorithms |
| Data | `.bsp` kernel files such as DE440/DE441 | No large kernel required |
| Precision role | Primary high-precision backend | Utility/fallback/helper layer |
| Strength | Planetary/lunar state vectors from JPL kernels | Calendar, time, utility, approximate astronomy formulas |
| Weakness | Requires external kernel data and CALCEPH build | Not as precise as JPL for high-grade ephemeris positions |

Project decision:

```text
Use JPL/CALCEPH as primary.
Use Meeus for calendar/time/general utilities and fallback formulas.
Do not pretend Meeus replaces JPL precision.
```

## What Is Moshier?

Moshier refers to Steve Moshier's public-domain astronomical code and tables.

In practical terms, Moshier is closer to a compact standalone ephemeris engine than a single formula family. It includes planetary tables and computational routines that can produce useful ephemeris results without JPL kernel files.

In this project, Moshier is useful as:

```text
compact analytical fallback engine
cross-check layer
non-JPL backup path
```

Moshier is not exactly equivalent to Meeus, VSOP87, or ELP2000. It overlaps with them, but its role is different.

## Moshier Compared To Meeus, VSOP87, And ELP

There is no single perfect equivalent to Moshier.

Best mapping:

| Moshier role | Closest equivalent |
|---|---|
| planetary positions | VSOP87 |
| lunar position | ELP2000 / ELP82 / other lunar theory |
| calendar/time/general astronomy formulas | Meeus |
| compact standalone ephemeris engine | Moshier itself |
| high-precision numerical source | JPL kernels through CALCEPH |

So the practical analogy is:

```text
Moshier ~= planetary theory + lunar theory + reduction formulas + utility astronomy code
```

But the more accurate project comparison is:

```text
Moshier < VSOP87 + ELP2000 + Meeus
```

Why?

- VSOP87 can be stronger and more complete for planetary analytical theory.
- ELP2000 can be stronger and more complete for lunar analytical theory.
- Meeus covers many utility formulas that Moshier may not expose in the same way.
- Together, VSOP87 + ELP2000 + Meeus can cover a broader analytical stack than Moshier alone.

But Moshier is more integrated than each one individually:

```text
Moshier > Meeus alone for ephemeris-engine usage
Moshier > VSOP87 alone for full practical-engine usage
Moshier > ELP alone for full practical-engine usage
```

Clean summary:

```text
Moshier < VSOP87 + ELP2000 + Meeus
Moshier > VSOP87 alone
Moshier > ELP alone
Moshier > Meeus alone
```

## VSOP87

VSOP87 is a planetary analytical theory.

It is mainly for planetary positions. It is not a complete astronomy engine by itself.

Typical coverage:

- Mercury
- Venus
- Earth
- Mars
- Jupiter
- Saturn
- Uranus
- Neptune

Depending on variant/source, VSOP may expose:

- heliocentric rectangular coordinates
- heliocentric spherical coordinates
- barycentric forms
- different epochs/frames

Project role:

```text
Use VSOP87 as the main analytical fallback for planets.
```

VSOP87 does not solve:

- Moon theory
- calendar/time systems
- Delta-T
- fixed stars
- asteroids/comets
- houses
- ayanamsa
- eclipse circumstances
- rise/set with observer conditions
- full coordinate-frame reduction pipeline

## ELP2000 / ELP82 / ELPMPP02

ELP is a lunar analytical theory family.

It is mainly for Moon position. It is not a complete astronomy engine by itself.

Project role:

```text
Use ELP as the main analytical fallback for the Moon.
```

ELP helps fill a gap that VSOP87 does not cover, because VSOP is planetary and does not provide a complete high-grade lunar theory.

Known ELP-related roles:

- lunar longitude
- lunar latitude
- lunar distance/radius
- lunar perturbation terms
- possible corrections depending on the selected ELP variant

ELP does not solve:

- planetary positions
- calendar/time systems
- fixed stars
- asteroids/comets
- houses
- ayanamsa
- panchang rules by itself

## Does VSOP87 + ELP + Meeus Cover All Astronomy?

No.

VSOP87 + ELP + Meeus covers a strong classical analytical core:

```text
VSOP87  = planets
ELP     = Moon
Meeus   = calendar/time/general formulas
```

But this is still not a complete astronomy engine and not a complete astrology engine.

It misses several important layers:

- high-precision numerical ephemeris data
- complete reference-frame handling
- high-precision Earth orientation
- fixed-star catalogs
- asteroid/comet catalogs
- artificial satellite propagation
- detailed topocentric observation modeling
- atmospheric refraction policy
- eclipse/occultation geometry
- rise/set/transit edge cases
- house-system algorithms
- sidereal ayanamsa models
- panchang/muhurta rule layers

So:

```text
VSOP87 + ELP + Meeus < full astronomy engine
VSOP87 + ELP + Meeus < full astrology engine
```

## The Fourth Member And Beyond

To fill the set properly, the analytical stack needs more than VSOP87 + ELP + Meeus.

Better minimum serious set:

```text
JPL/CALCEPH + VSOP87 + ELP2000 + Meeus + Moshier + SOFA/ERFA + IERS
```

For astrology, add:

```text
fixed-star catalogs
small-body catalogs
house algorithms
ayanamsa models
rise/set/transit logic
eclipse/visibility logic
panchang/muhurta rules
```

## SOFA / ERFA / IAU Layer

SOFA is the IAU Standards of Fundamental Astronomy library.

ERFA is an open-source derivative/reimplementation used by projects that need SOFA-style functionality with friendlier licensing.

This layer covers high-quality standards for:

- precession
- nutation
- frame bias
- Earth rotation angle
- sidereal time
- time-scale conversions
- coordinate transformations
- ICRS/FK5-style reference frames
- celestial-to-terrestrial transformation pieces

Project role:

```text
Use SOFA/ERFA-style logic for reference frames, time scales, and Earth orientation transformations.
```

This layer is critical because JPL gives positions in a kernel/reference-frame context, but astrology/observer output usually needs transformed apparent/topocentric/sidereal values.

JPL answers:

```text
Where is the body in the ephemeris frame?
```

SOFA/ERFA/IAU-style reductions help answer:

```text
What is the apparent/topocentric/ecliptic/equatorial position for an observer and date?
```

## IERS / EOP Layer

IERS means International Earth Rotation and Reference Systems Service.

EOP means Earth Orientation Parameters.

This layer covers:

- UT1-UTC
- polar motion
- Earth orientation corrections
- high-precision terrestrial/celestial frame linkage

Project role:

```text
Use IERS/EOP when high-precision observer-based calculations are required.
```

Without EOP, many applications can still work at ordinary astrology precision, but a serious astronomy engine should clearly document whether it uses EOP or a simplified policy.

## Catalog Layer

JPL kernels, VSOP, ELP, Meeus, and Moshier do not automatically cover every object users may ask for.

Additional catalog layers are needed.

### Fixed Stars

Needed sources may include:

- Gaia
- Hipparcos
- Yale Bright Star Catalog
- other curated star catalogs

Fixed-star support needs:

- star identifier/name lookup
- RA/Dec
- proper motion
- parallax
- radial velocity when available
- epoch handling
- magnitude
- coordinate-frame handling

### Asteroids, Comets, Minor Planets

Needed sources may include:

- JPL Horizons data
- MPC orbital elements
- SPK kernels for selected bodies

Small-body support needs:

- object database
- orbital elements or kernel target IDs
- epoch handling
- perturbation policy
- fallback policy when no SPK kernel exists

### Artificial Satellites

Needed model:

- SGP4

Needed data:

- TLE records

This is a separate problem from planetary ephemerides. JPL DE kernels do not replace SGP4/TLE workflows for Earth satellites.

## Observer / Topocentric Layer

For user-facing astronomy and astrology, geocentric positions are often not enough.

Needed pieces:

- observer longitude
- observer latitude
- observer altitude
- geodetic Earth model
- parallax correction
- topocentric coordinate conversion
- horizon coordinate conversion
- atmospheric refraction
- apparent disc radius policy for Sun/Moon/planets

This layer is necessary for:

- rise
- set
- transit
- twilight
- eclipse visibility
- occultations
- heliacal visibility
- local panchang calculations

## Refraction Layer

Atmospheric refraction affects apparent altitude, especially near the horizon.

Possible formula families:

- Bennett
- Saemundsson
- NOAA-style refraction formulas
- custom pressure/temperature-aware models

Project decision should be explicit:

```text
Do not hide refraction policy.
Expose pressure, temperature, and true/apparent conversion mode where relevant.
```

## Eclipse And Occultation Geometry

JPL can provide accurate state vectors, but eclipse and occultation outputs require additional geometry.

Needed calculations:

- apparent Sun/Moon radius
- Earth/Moon shadow cones
- umbra/penumbra
- local horizon visibility
- contact times
- maximum eclipse
- central/non-central classification
- total/partial/annular/hybrid classification
- lunar penumbral/partial/total classification
- occultation object apparent size and relative motion

JPL gives the raw positions. The engine must still implement the event geometry.

## Rise, Set, Transit, And Visibility

Rise/set/transit is not just body longitude.

Needed inputs:

- apparent RA/Dec or altitude
- observer location
- horizon height
- refraction
- body disc radius
- topocentric parallax
- event search algorithm
- polar/no-event handling
- date-boundary handling

This is important for astronomy and panchang.

## Astrology Layer

Astrology requires additional interpretation and calculation layers above raw astronomy.

Important astrology-specific pieces:

- tropical/sidereal zodiac policy
- ayanamsa models
- house systems
- cusps
- ascendant/midheaven
- planetary speed
- retrograde/stationary logic
- combustion logic if Vedic-style output is desired
- aspects if supported
- panchang components
- muhurta periods
- festival/vrata rules if the engine includes calendar logic

These are not provided directly by VSOP, ELP, Meeus, Moshier, or JPL.

## Ayanamsa Layer

Sidereal astrology needs ayanamsa models.

Examples:

- Lahiri
- Raman
- Krishnamurti
- Fagan-Bradley
- Yukteshwar
- True Chitra
- True Revati
- user-defined ayanamsa

Project rule:

```text
Ayanamsa should be explicit and user-selectable.
Do not silently mix tropical and sidereal output.
```

## House-System Layer

House systems are separate from ephemeris position calculation.

Examples:

- Whole Sign
- Equal
- Porphyry
- Placidus
- Koch
- Regiomontanus
- Campanus
- Alcabitius
- Morinus
- Meridian
- Horizontal
- Vehlow Equal
- Gauquelin sectors

JPL does not provide houses. VSOP does not provide houses. ELP does not provide houses. Meeus may provide some relevant astronomy formulas, but house systems are their own astrology/astronomical-coordinate problem.

## Panchang / Vedic Calendar Layer

Panchang is above ephemeris calculation.

Raw ephemeris provides Sun and Moon positions. Panchang logic derives:

- tithi
- nakshatra
- yoga
- karana
- vara
- paksha
- lunar month
- solar month
- sankranti
- sunrise/sunset based day boundaries
- moonrise/moonset
- rahu kaal
- yama gandam
- gulika
- choghadiya
- hora
- muhurta
- festival rules
- vrata/parana rules

This project can provide astronomy primitives, but panchang needs a separate rule engine if included.

## Recommended Engine Stack

The recommended architecture:

```text
1. JPL/CALCEPH
   Primary numerical ephemeris backend.

2. VSOP87
   Analytical planetary fallback.

3. ELP2000 / ELP82 / ELPMPP02
   Analytical lunar fallback.

4. Meeus
   Calendar, time, general astronomy, and lightweight fallback formulas.

5. Moshier
   Compact fallback engine and cross-check layer.

6. SOFA/ERFA + IAU models
   Reference frames, precession, nutation, sidereal time, time scales.

7. IERS/EOP
   Earth orientation when high-precision topocentric output is required.

8. Catalogs
   Fixed stars, asteroids, comets, satellites.

9. Observer and event geometry
   Rise/set/transit, eclipses, occultations, heliacal visibility.

10. Astrology layer
    Ayanamsa, houses, panchang, muhurta, rules.
```

## Project-Specific Decision

For this project:

```text
JPL/CALCEPH is primary.
VSOP87 + ELP + Meeus form the analytical component fallback stack.
Moshier is an additional compact fallback/cross-check layer.
SOFA/ERFA and IERS-style logic are needed for a serious frame/time layer.
Catalogs and astrology-specific rules are separate layers above ephemeris math.
```

The project should avoid claiming that any one component covers everything.

Correct claims:

```text
JPL/CALCEPH covers primary numerical state vectors.
VSOP87 covers analytical planetary fallback.
ELP covers analytical lunar fallback.
Meeus covers utility and approximate formula fallback.
Moshier covers compact fallback engine behavior.
SOFA/ERFA covers standards-based frame/time transformations.
IERS covers high-precision Earth orientation.
Catalogs cover stars, asteroids, comets, and satellites.
Astrology layers cover houses, ayanamsa, panchang, and rules.
```

Incorrect claims:

```text
Meeus alone covers full astronomy.
Moshier alone covers full astronomy.
VSOP87 + ELP + Meeus covers full astrology.
JPL kernels alone provide houses, ayanamsa, panchang, or muhurta.
```

## Completion Philosophy

A component should be marked complete only when all of these are true:

- public API is implemented
- no false success path exists
- source/provenance is documented
- output units are documented
- coordinate frame is documented
- time scale is documented
- valid date/domain range is documented
- known-value tests exist
- invalid-input tests exist
- fallback order is explicit

This keeps the project honest and makes it possible for another contributor to continue from the current state without guessing.
