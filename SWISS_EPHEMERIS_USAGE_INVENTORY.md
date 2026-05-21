# Swiss Ephemeris Usage Inventory

This file lists the Swiss Ephemeris functions and constants used by these two projects:

- `vedic_astrology`
- `panchang-core`

Scope:

- scanned Python files in `vedic_astrology`
- scanned PHP files in `panchang-core`
- excluded generated output folders, virtual environments, bytecode caches, and Composer `vendor`
- excluded project-owned helper functions/constants unless they directly map to a Swiss Ephemeris symbol
- excluded comment-only hits

Important distinction:

- Python code uses `pyswisseph`-style names such as `swe.calc_ut`, `swe.SUN`, `swe.FLG_SWIEPH`.
- The Python compatibility layer also maps to `swisseph-ffi` / C-style names such as `SE_SUN`, `SEFLG_SWIEPH`, `swe_calc_ut`.
- PHP code uses the PHP FFI wrapper directly with C-style names such as `swe_calc_ut`, `SE_SUN`, `SEFLG_SWIEPH`.

## Combined Requirement

Across both projects, the Swiss Ephemeris replacement must cover these functions and constants.

### Combined Functions

Total unique Swiss Ephemeris functions required: `18`

| Swiss function | Short purpose in these projects |
|---|---|
| `swe_calc_ut` | Computes Sun, Moon, planet, and node positions for chart, panchang, transit, retrograde, combustion, yoga, and sankranti logic. |
| `swe_day_of_week` | Converts Julian day to weekday index for weekday-based strength logic. |
| `swe_get_ayanamsa_ut` | Gets sidereal offset for Lahiri/custom sidereal calculations and sidereal ascendant adjustment. |
| `swe_houses` | Computes house cusps and ascendant/midheaven values for horoscope, KP, bhava chalit, and lagna tables. |
| `swe_jdut1_to_utc` | Converts Julian day event times back to UTC calendar fields for eclipse and rise/set output. |
| `swe_julday` | Converts civil date/time to Julian day for all ephemeris calculations. |
| `swe_lun_eclipse_how` | Computes local lunar eclipse circumstances. |
| `swe_lun_eclipse_when` | Searches global lunar eclipse events. |
| `swe_lun_eclipse_when_loc` | Searches lunar eclipse events for a specific location. |
| `swe_revjul` | Converts Julian day back to calendar date/time. |
| `swe_rise_trans` | Computes rise, set, transit, twilight, and moon/sun visibility times. |
| `swe_set_ephe_path` | Configures ephemeris data-file path. |
| `swe_set_sid_mode` | Selects sidereal ayanamsa mode. |
| `swe_sidtime` | Computes sidereal time in debug/verification scripts. |
| `swe_sol_eclipse_how` | Computes local solar eclipse circumstances. |
| `swe_sol_eclipse_when_glob` | Searches global solar eclipse events. |
| `swe_sol_eclipse_when_loc` | Searches solar eclipse events for a specific location. |
| `swe_time_equ` | Computes equation of time for shadbala/time correction logic. |

### Combined Constants

Total unique Swiss Ephemeris constants required: `37`

| Swiss constant | Short purpose in these projects |
|---|---|
| `SEFLG_EQUATORIAL` | Requests equatorial coordinates for declination/right-ascension style strength calculations. |
| `SEFLG_HELCTR` | Requests heliocentric coordinates for shadbala/planetary strength calculations. |
| `SEFLG_SIDEREAL` | Requests sidereal zodiac positions. |
| `SEFLG_SPEED` | Requests speed values for retrograde/stationary/combustion logic. |
| `SEFLG_SWIEPH` | Selects Swiss Ephemeris calculation backend/data. |
| `SE_BIT_ASTRO_TWILIGHT` | Requests astronomical twilight event calculation. |
| `SE_BIT_CIVIL_TWILIGHT` | Requests civil twilight event calculation. |
| `SE_BIT_DISC_CENTER` | Uses body disc center for rise/set calculation mode. |
| `SE_BIT_NAUTIC_TWILIGHT` | Requests nautical twilight event calculation. |
| `SE_CALC_ITRANSIT` | Requests lower/anti-meridian transit event. |
| `SE_CALC_MTRANSIT` | Requests meridian transit event. |
| `SE_CALC_RISE` | Requests rise event. |
| `SE_CALC_SET` | Requests set event. |
| `SE_ECL_ALLTYPES_LUNAR` | Includes all lunar eclipse types in search. |
| `SE_ECL_ALLTYPES_SOLAR` | Includes all solar eclipse types in search. |
| `SE_ECL_ANNULAR` | Classifies annular solar eclipse. |
| `SE_ECL_ANNULAR_TOTAL` | Classifies hybrid annular-total solar eclipse. |
| `SE_ECL_PARTIAL` | Classifies partial eclipse. |
| `SE_ECL_TOTAL` | Classifies total eclipse. |
| `SE_ECL_VISIBLE` | Tests whether an eclipse is visible at a location. |
| `SE_GREG_CAL` | Uses Gregorian calendar conversion. |
| `SE_JUL_CAL` | Uses Julian calendar conversion. |
| `SE_JUPITER` | Jupiter body identifier. |
| `SE_MARS` | Mars body identifier. |
| `SE_MEAN_NODE` | Mean lunar node identifier, used as Rahu-style node. |
| `SE_MERCURY` | Mercury body identifier. |
| `SE_MOON` | Moon body identifier. |
| `SE_SATURN` | Saturn body identifier. |
| `SE_SIDM_FAGAN_BRADLEY` | Fagan-Bradley sidereal ayanamsa mode. |
| `SE_SIDM_KRISHNAMURTI` | Krishnamurti sidereal ayanamsa mode. |
| `SE_SIDM_LAHIRI` | Lahiri sidereal ayanamsa mode. |
| `SE_SIDM_RAMAN` | Raman sidereal ayanamsa mode. |
| `SE_SIDM_TRUE_CITRA` | True Chitra sidereal ayanamsa mode used in debug checks. |
| `SE_SIDM_USER` | Custom/user-defined sidereal ayanamsa mode. |
| `SE_SUN` | Sun body identifier. |
| `SE_TRUE_NODE` | True lunar node identifier. |
| `SE_VENUS` | Venus body identifier. |

## `vedic_astrology`

### Functions

Total Swiss Ephemeris functions used: `11`

| Python symbol used | Swiss C equivalent | Purpose | Used in |
|---|---|---|---|
| `swe.calc_ut` | `swe_calc_ut` | Computes sidereal/tropical body longitude, latitude, distance, and speed for chart, strength, panchanga, transit, and relationship logic. | `astrology/horoscope/points.py`, `astrology/kp/engine.py`, `astrology/nadi/timing.py`, `astrology/relationship/master_engine.py`, `astrology/strengths/shadbala.py`, `astrology/tajika/engine.py`, `astronomy/ephemeris.py`, `astronomy/retro_combust.py`, `main.py`, tests/debug scripts |
| `_swe.swe_day_of_week` | `swe_day_of_week` | Converts Julian day to weekday index for weekday-dependent strength rules. | `astronomy/swisseph_ffi_compat.py` |
| `swe.get_ayanamsa_ut` | `swe_get_ayanamsa_ut` | Reads ayanamsa offset for sidereal conversion and ascendant adjustment. | `astrology/horoscope/bhava_chalit.py`, `astrology/horoscope/points.py`, `astrology/kp/engine.py`, `astronomy/ephemeris.py`, `config.py`, `main.py`, tests/debug scripts |
| `swe.houses` | `swe_houses` | Computes tropical house cusps, ascendant, and MC values used by horoscope/KP/bhava logic. | `astrology/horoscope/bhava_chalit.py`, `astrology/horoscope/points.py`, `astrology/kp/engine.py`, `astronomy/ephemeris.py`, `main.py`, tests/debug scripts |
| `swe.julday` | `swe_julday` | Converts local/UTC calendar date-time into Julian day input for ephemeris calculations. | `astrology/horoscope/points.py`, `astrology/kp/engine.py`, `astrology/nadi/timing.py`, `astrology/relationship/master_engine.py`, `astrology/tajika/engine.py`, `astronomy/ephemeris.py`, `astronomy/sun_times.py`, `main.py`, tests/debug scripts |
| `swe.revjul` | `swe_revjul` | Converts Julian day event results back into date/time components. | `astrology/nadi/timing.py`, `astronomy/sun_times.py`, tests/debug scripts |
| `swe.rise_trans` | `swe_rise_trans` | Computes sunrise, sunset, moonrise, moonset, and related rise/set values. | `astrology/strengths/shadbala.py`, `astronomy/sun_times.py`, tests/debug scripts |
| `swe.set_ephe_path` | `swe_set_ephe_path` | Sets ephemeris data path in debug/verification scripts. | tests/debug scripts |
| `swe.set_sid_mode` | `swe_set_sid_mode` | Selects Lahiri, custom, or other sidereal mode before sidereal calculations. | `astrology/horoscope/points.py`, `astrology/kp/engine.py`, `astrology/nadi/timing.py`, `astrology/relationship/master_engine.py`, `astrology/tajika/engine.py`, `astronomy/ephemeris.py`, `config.py`, `main.py`, tests/debug scripts |
| `swe.sidtime` | `swe_sidtime` | Computes sidereal time in debug/verification scripts. | tests/debug scripts |
| `swe.time_equ` | `swe_time_equ` | Computes equation of time for shadbala/time correction logic. | `astrology/strengths/shadbala.py` |

Excluded:

- `get_sid_mode` is a project compatibility helper storing local state; it does not call a Swiss Ephemeris function in the current Python compatibility layer.

### Constants

Total Swiss Ephemeris constants used/mapped: `45`

| Python symbol used | Swiss C equivalent | Purpose | Used in |
|---|---|---|---|
| `swe.BIT_DISC_CENTER` | `SE_BIT_DISC_CENTER` | Uses body disc center mode for rise/set calculations. | `astrology/strengths/shadbala.py` |
| `swe.CALC_RISE` | `SE_CALC_RISE` | Requests rise event. | `astrology/strengths/shadbala.py`, `astronomy/sun_times.py`, tests/debug scripts |
| `swe.CALC_SET` | `SE_CALC_SET` | Requests set event. | `astrology/strengths/shadbala.py`, `astronomy/sun_times.py`, tests/debug scripts |
| `swe.FLG_EQUATORIAL` | `SEFLG_EQUATORIAL` | Requests equatorial coordinates for declination/RA calculations. | `astrology/strengths/shadbala.py`, tests/debug scripts |
| `swe.FLG_HELCTR` | `SEFLG_HELCTR` | Requests heliocentric coordinates for strength calculations. | `astrology/strengths/shadbala.py`, `astronomy/swisseph_ffi_compat.py` |
| `swe.FLG_SIDEREAL` | `SEFLG_SIDEREAL` | Requests sidereal zodiac output. | `astrology/horoscope/points.py`, `astrology/kp/engine.py`, `astrology/nadi/timing.py`, `astrology/relationship/master_engine.py`, `astrology/strengths/shadbala.py`, `astrology/tajika/engine.py`, `astronomy/ephemeris.py`, `astronomy/retro_combust.py`, `main.py`, tests/debug scripts |
| `swe.FLG_SPEED` | `SEFLG_SPEED` | Requests speed values for retrograde and combustion logic. | `astrology/strengths/shadbala.py`, `astronomy/retro_combust.py`, tests/debug scripts |
| `swe.FLG_SWIEPH` | `SEFLG_SWIEPH` | Selects Swiss Ephemeris backend/data. | `astrology/horoscope/points.py`, `astrology/kp/engine.py`, `astrology/nadi/timing.py`, `astrology/relationship/master_engine.py`, `astrology/strengths/shadbala.py`, `astrology/tajika/engine.py`, `astronomy/ephemeris.py`, `astronomy/retro_combust.py`, `astronomy/sun_times.py`, `main.py`, tests/debug scripts |
| `swe.GREG_CAL` | `SE_GREG_CAL` | Uses Gregorian calendar conversion. | `astronomy/sun_times.py`, tests/debug scripts |
| `swe.JUPITER` | `SE_JUPITER` | Jupiter body ID. | `astrology/constants.py`, `astrology/nadi/timing.py`, `astrology/relationship/master_engine.py`, tests/debug scripts |
| `swe.MARS` | `SE_MARS` | Mars body ID. | `astrology/constants.py`, `astrology/nadi/timing.py`, tests/debug scripts |
| `swe.MEAN_NODE` | `SE_MEAN_NODE` | Mean lunar node/Rahu-style body ID. | `astrology/constants.py`, `astrology/nadi/timing.py` |
| `swe.MERCURY` | `SE_MERCURY` | Mercury body ID. | `astrology/constants.py`, `astrology/nadi/timing.py`, tests/debug scripts |
| `swe.MOON` | `SE_MOON` | Moon body ID. | `astrology/constants.py`, `astrology/nadi/timing.py`, tests/debug scripts |
| `swe.SATURN` | `SE_SATURN` | Saturn body ID. | `astrology/constants.py`, `astrology/nadi/timing.py`, `astrology/relationship/master_engine.py`, tests/debug scripts |
| `swe.SIDM_FAGAN_BRADLEY` | `SE_SIDM_FAGAN_BRADLEY` | Fagan-Bradley ayanamsa mode. | `astrology/constants.py`, `astronomy/swisseph_ffi_compat.py` |
| `swe.SIDM_KRISHNAMURTI` | `SE_SIDM_KRISHNAMURTI` | Krishnamurti ayanamsa mode. | `astrology/constants.py`, `astronomy/swisseph_ffi_compat.py` |
| `swe.SIDM_LAHIRI` | `SE_SIDM_LAHIRI` | Lahiri ayanamsa mode. | `astrology/constants.py`, `astrology/horoscope/points.py`, `astrology/kp/engine.py`, `astrology/nadi/timing.py`, `astrology/relationship/master_engine.py`, `astrology/tajika/engine.py`, `astronomy/ephemeris.py`, `config.py`, `main.py`, tests/debug scripts |
| `swe.SIDM_RAMAN` | `SE_SIDM_RAMAN` | Raman ayanamsa mode. | `astrology/constants.py`, tests/debug scripts |
| `swe.SIDM_TRUE_CITRA` | `SE_SIDM_TRUE_CITRA` | True Chitra ayanamsa mode used in debug checks. | tests/debug scripts |
| `swe.SIDM_USER` | `SE_SIDM_USER` | Custom/user ayanamsa mode. | `astrology/kp/engine.py`, `astronomy/ephemeris.py`, `main.py`, tests/debug scripts |
| `swe.SUN` | `SE_SUN` | Sun body ID. | `astrology/constants.py`, `astrology/horoscope/points.py`, `astrology/nadi/timing.py`, `astrology/strengths/shadbala.py`, `astrology/tajika/engine.py`, `astronomy/retro_combust.py`, `astronomy/sun_times.py`, tests/debug scripts |
| `swe.VENUS` | `SE_VENUS` | Venus body ID. | `astrology/constants.py`, `astrology/nadi/timing.py`, `astrology/relationship/master_engine.py`, tests/debug scripts |
| `ffi.SEFLG_EQUATORIAL` | `SEFLG_EQUATORIAL` | `astronomy/swisseph_ffi_compat.py` |
| `ffi.SEFLG_HELCTR` | `SEFLG_HELCTR` | `astronomy/swisseph_ffi_compat.py` |
| `ffi.SEFLG_SIDEREAL` | `SEFLG_SIDEREAL` | `astronomy/swisseph_ffi_compat.py` |
| `ffi.SEFLG_SPEED` | `SEFLG_SPEED` | `astronomy/swisseph_ffi_compat.py` |
| `ffi.SEFLG_SWIEPH` | `SEFLG_SWIEPH` | `astronomy/swisseph_ffi_compat.py` |
| `ffi.SE_BIT_DISC_CENTER` | `SE_BIT_DISC_CENTER` | `astronomy/swisseph_ffi_compat.py` |
| `ffi.SE_CALC_RISE` | `SE_CALC_RISE` | `astronomy/swisseph_ffi_compat.py` |
| `ffi.SE_CALC_SET` | `SE_CALC_SET` | `astronomy/swisseph_ffi_compat.py` |
| `ffi.SE_GREG_CAL` | `SE_GREG_CAL` | `astronomy/swisseph_ffi_compat.py` |
| `ffi.SE_JUL_CAL` | `SE_JUL_CAL` | `astronomy/swisseph_ffi_compat.py` |
| `ffi.SE_JUPITER` | `SE_JUPITER` | `astronomy/swisseph_ffi_compat.py` |
| `ffi.SE_MARS` | `SE_MARS` | `astronomy/swisseph_ffi_compat.py` |
| `ffi.SE_MEAN_NODE` | `SE_MEAN_NODE` | `astronomy/swisseph_ffi_compat.py` |
| `ffi.SE_MERCURY` | `SE_MERCURY` | `astronomy/swisseph_ffi_compat.py` |
| `ffi.SE_MOON` | `SE_MOON` | `astronomy/swisseph_ffi_compat.py` |
| `ffi.SE_SATURN` | `SE_SATURN` | `astronomy/swisseph_ffi_compat.py` |
| `ffi.SE_SIDM_FAGAN_BRADLEY` | `SE_SIDM_FAGAN_BRADLEY` | `astronomy/swisseph_ffi_compat.py` |
| `ffi.SE_SIDM_KRISHNAMURTI` | `SE_SIDM_KRISHNAMURTI` | `astronomy/swisseph_ffi_compat.py` |
| `ffi.SE_SIDM_LAHIRI` | `SE_SIDM_LAHIRI` | `astronomy/swisseph_ffi_compat.py` |
| `ffi.SE_SIDM_RAMAN` | `SE_SIDM_RAMAN` | `astronomy/swisseph_ffi_compat.py` |
| `ffi.SE_SIDM_USER` | `SE_SIDM_USER` | `astronomy/swisseph_ffi_compat.py` |
| `ffi.SE_SUN` | `SE_SUN` | `astronomy/swisseph_ffi_compat.py` |
| `ffi.SE_VENUS` | `SE_VENUS` | `astronomy/swisseph_ffi_compat.py` |

Excluded:

- `HELIO` in `astronomy/swisseph_ffi_compat.py` is a project compatibility alias for `SEFLG_HELCTR`, so the real Swiss constant to cover is `SEFLG_HELCTR`.

## `panchang-core`

### Functions

Total Swiss Ephemeris functions used: `14`

| Swiss function | Purpose | Used in |
|---|---|---|
| `swe_calc_ut` | Computes Sun/Moon/planet/node longitudes and speeds for panchang, tithi, nakshatra, yoga, karana, transitions, and muhurta logic. | `src/Astronomy/AstronomyService.php`, `src/Astronomy/Math/TransitEngine.php`, `src/Panchanga/Traits/PanchangBirthMonthHelpersTrait.php`, `src/Panchanga/Traits/PanchangMuhurtaYogaDelegatesTrait.php` |
| `swe_get_ayanamsa_ut` | Gets Lahiri sidereal offset for sidereal longitude calculations. | `src/Astronomy/AstronomyService.php` |
| `swe_houses` | Computes ascendant/house cusps for lagna tables and astronomy output. | `src/Astronomy/AstronomyService.php`, `src/Muhurta/Lagna/LagnaTableCalculator.php` |
| `swe_jdut1_to_utc` | Converts Julian day event times into UTC/civil date fields for output. | `src/Astronomy/EclipseService.php`, `src/Astronomy/SunService.php` |
| `swe_julday` | Converts civil date/time into Julian day for all astronomy calculations. | `src/Astronomy/AstronomyService.php`, `src/Astronomy/EclipseService.php`, `src/Astronomy/SunService.php`, `src/Muhurta/Lagna/LagnaTableCalculator.php` |
| `swe_lun_eclipse_how` | Computes lunar eclipse circumstances at the configured location. | `src/Astronomy/EclipseService.php` |
| `swe_lun_eclipse_when` | Searches upcoming lunar eclipse events globally. | `src/Astronomy/EclipseService.php` |
| `swe_lun_eclipse_when_loc` | Searches lunar eclipse events for the configured location. | `src/Astronomy/EclipseService.php` |
| `swe_rise_trans` | Computes sunrise, sunset, moonrise, moonset, transit, and twilight event times. | `src/Astronomy/SunService.php` |
| `swe_set_ephe_path` | Configures Swiss ephemeris data-file path. | `src/Astronomy/Concerns/ConfiguresEphemeris.php`, `src/PanchangServiceProvider.php`, `src/Panchanga/PanchangService.php` |
| `swe_set_sid_mode` | Sets Lahiri sidereal mode for panchang calculations. | `src/Astronomy/AstronomyService.php`, `src/PanchangServiceProvider.php`, `src/Panchanga/PanchangService.php` |
| `swe_sol_eclipse_how` | Computes solar eclipse circumstances at the configured location. | `src/Astronomy/EclipseService.php` |
| `swe_sol_eclipse_when_glob` | Searches upcoming solar eclipse events globally. | `src/Astronomy/EclipseService.php` |
| `swe_sol_eclipse_when_loc` | Searches solar eclipse events for the configured location. | `src/Astronomy/EclipseService.php` |

### Constants

Total Swiss Ephemeris constants used: `28`

| Swiss constant | Purpose | Used in |
|---|---|---|
| `SEFLG_SIDEREAL` | Requests sidereal zodiac output. | `src/Astronomy/AstronomyService.php`, `src/Astronomy/Math/TransitEngine.php`, `src/Panchanga/Traits/PanchangAstronomyHelpersTrait.php`, `src/Panchanga/Traits/PanchangBirthMonthHelpersTrait.php` |
| `SEFLG_SPEED` | Requests speed values for retrograde/stationary data. | `src/Astronomy/AstronomyService.php` |
| `SEFLG_SWIEPH` | Selects Swiss Ephemeris backend/data. | `src/Astronomy/AstronomyService.php`, `src/Astronomy/EclipseService.php`, `src/Astronomy/Math/TransitEngine.php`, `src/Astronomy/SunService.php`, `src/Panchanga/Traits/PanchangAstronomyHelpersTrait.php`, `src/Panchanga/Traits/PanchangBirthMonthHelpersTrait.php` |
| `SE_BIT_ASTRO_TWILIGHT` | Requests astronomical twilight. | `src/Astronomy/SunService.php` |
| `SE_BIT_CIVIL_TWILIGHT` | Requests civil twilight. | `src/Astronomy/SunService.php` |
| `SE_BIT_NAUTIC_TWILIGHT` | Requests nautical twilight. | `src/Astronomy/SunService.php` |
| `SE_CALC_ITRANSIT` | Requests anti-meridian/lower transit. | `src/Astronomy/SunService.php` |
| `SE_CALC_MTRANSIT` | Requests meridian transit. | `src/Astronomy/SunService.php` |
| `SE_CALC_RISE` | Requests rise event. | `src/Astronomy/SunService.php` |
| `SE_CALC_SET` | Requests set event. | `src/Astronomy/SunService.php` |
| `SE_ECL_ALLTYPES_LUNAR` | Includes all lunar eclipse types in search. | `src/Astronomy/EclipseService.php` |
| `SE_ECL_ALLTYPES_SOLAR` | Includes all solar eclipse types in search. | `src/Astronomy/EclipseService.php` |
| `SE_ECL_ANNULAR` | Identifies annular solar eclipse. | `src/Astronomy/EclipseService.php` |
| `SE_ECL_ANNULAR_TOTAL` | Identifies hybrid annular-total solar eclipse. | `src/Astronomy/EclipseService.php` |
| `SE_ECL_PARTIAL` | Identifies partial eclipse. | `src/Astronomy/EclipseService.php` |
| `SE_ECL_TOTAL` | Identifies total eclipse. | `src/Astronomy/EclipseService.php` |
| `SE_ECL_VISIBLE` | Marks visibility at location. | `src/Astronomy/EclipseService.php` |
| `SE_GREG_CAL` | Uses Gregorian calendar conversion. | `src/Astronomy/AstronomyService.php`, `src/Astronomy/EclipseService.php`, `src/Astronomy/SunService.php`, `src/Muhurta/Lagna/LagnaTableCalculator.php` |
| `SE_JUPITER` | Jupiter body ID. | `src/Astronomy/AstronomyService.php` |
| `SE_MARS` | Mars body ID. | `src/Astronomy/AstronomyService.php` |
| `SE_MEAN_NODE` | Mean lunar node/Rahu-style body ID. | `src/Astronomy/AstronomyService.php` |
| `SE_MERCURY` | Mercury body ID. | `src/Astronomy/AstronomyService.php` |
| `SE_MOON` | Moon body ID. | `src/Astronomy/AstronomyService.php`, `src/Astronomy/Math/TransitEngine.php`, `src/Astronomy/SunService.php`, `src/Panchanga/Traits/PanchangAstronomyHelpersTrait.php`, `src/Panchanga/Traits/PanchangBirthMonthHelpersTrait.php` |
| `SE_SATURN` | Saturn body ID. | `src/Astronomy/AstronomyService.php` |
| `SE_SIDM_LAHIRI` | Lahiri ayanamsa mode. | `src/Astronomy/AstronomyService.php`, `src/PanchangServiceProvider.php`, `src/Panchanga/PanchangService.php` |
| `SE_SUN` | Sun body ID. | `src/Astronomy/AstronomyService.php`, `src/Astronomy/Math/TransitEngine.php`, `src/Astronomy/SunService.php`, `src/Panchanga/Traits/PanchangAstronomyHelpersTrait.php`, `src/Panchanga/Traits/PanchangBirthMonthHelpersTrait.php` |
| `SE_TRUE_NODE` | True lunar node body ID. | `src/Astronomy/AstronomyService.php` |
| `SE_VENUS` | Venus body ID. | `src/Astronomy/AstronomyService.php` |

## Practical Replacement Checklist

To replace Swiss Ephemeris in both projects without missing current usage, a new engine/wrapper must provide at least:

- planetary calculation: `swe_calc_ut`
- Julian date conversion: `swe_julday`
- reverse Julian conversion: `swe_revjul`
- UTC conversion from Julian day: `swe_jdut1_to_utc`
- ayanamsa: `swe_get_ayanamsa_ut`
- sidereal mode setting: `swe_set_sid_mode`
- ephemeris path setting: `swe_set_ephe_path`
- houses: `swe_houses`
- rise/set/transit: `swe_rise_trans`
- sidereal time: `swe_sidtime`
- equation of time: `swe_time_equ`
- day of week: `swe_day_of_week`
- solar eclipse search/detail APIs used by `panchang-core`
- lunar eclipse search/detail APIs used by `panchang-core`
- all listed planet, node, flag, calendar, sidereal-mode, rise/transit, twilight, and eclipse constants

## JME Replacement Readiness

This section tracks whether the current `JPL-Moshier-Ephemeris` project can replace each Swiss Ephemeris usage while keeping the caller I/O intact.

Definitions:

- **Ready:** equivalent exists and current behavior is low-risk for these projects.
- **Partial:** equivalent exists, but behavior is not yet strict enough to guarantee intact I/O for these projects.
- **Not ready:** no suitable equivalent exists yet, or the current equivalent cannot produce the required behavior for these projects.

### Function Readiness

| Swiss function needed | JME equivalent | Status | Reason / gap before intact I/O |
|---|---|---|---|
| `swe_julday` | `jme_julian_day` | Ready | Calendar-to-Julian conversion exists and is directly tested. A compatibility wrapper may still be needed if the caller expects exact Swiss argument/return shape. |
| `swe_revjul` | `jme_reverse_julian_day` | Ready | Julian-to-calendar conversion exists and is directly tested. A compatibility wrapper may still be needed for exact output argument shape. |
| `swe_jdut1_to_utc` | `jme_jd_to_utc` | Partial | Calendar-field conversion exists, but Swiss `swe_jdut1_to_utc` has a specific UT1/UTC-oriented signature and output convention. Add a compatibility wrapper and verify date/time fields against Swiss for eclipse/rise-set event outputs. |
| `swe_day_of_week` | `jme_day_of_week` | Ready | Weekday conversion exists and is tested. Verify numbering convention remains identical to Swiss where used. |
| `swe_set_ephe_path` | `jme_set_ephemeris_path` | Ready for state, partial for behavior | Path state exists. It does not by itself load Swiss `.se1` files; JME uses its own JPL/CALCEPH path model. For intact migration, define how this maps to JPL kernel/data paths. |
| `swe_set_sid_mode` | `jme_set_sidereal_mode` | Partial | Sidereal mode state exists, but all Swiss sidereal modes used here must be verified against Swiss values. Lahiri is the main production need; Fagan-Bradley, Raman, Krishnamurti, True Chitra, and user mode appear in Python/constants/debug usage. |
| `swe_get_ayanamsa_ut` | `jme_get_ayanamsa_ut` | Partial | Equivalent exists, but ayanamsa parity must be checked mode-by-mode. Lahiri must be verified first because both projects rely on it. |
| `swe_sidtime` | `jme_sidereal_time` | Ready for basic use, partial for exact parity | Sidereal time exists and is tested at J2000. Verify Swiss-compatible precision/date range before using as drop-in for debug/derived calculations. |
| `swe_time_equ` | `jme_time_equ` | Partial | Function exists, but depends on solar-position accuracy and time-scale policy. Needs Swiss comparison tests for shadbala use cases before claiming intact I/O. |
| `swe_calc_ut` | `jme_calc_ut` | Partial | Core equivalent exists, but full Swiss behavior parity is not complete. Gaps include exact flag handling, sidereal mode parity, apparent/true position policy, speed output, topocentric/geocentric behavior, and fallback/JPL reduction differences. This is the most important blocker for both projects. |
| `swe_houses` | `jme_houses` | Partial | Function exists, but not all Swiss house systems and edge cases are complete. For current usage, verify the default house system and ascendant/MC/cusp output shape against Swiss. |
| `swe_rise_trans` | `jme_rise_trans` | Partial | Function exists, but full Swiss-grade rise/set/transit behavior is not complete. Needs parity for Sun/Moon rise/set, disc-center flags, refraction policy, no-event handling, and date-boundary behavior. This is important for both projects. |
| `swe_sol_eclipse_how` | `jme_sol_eclipse_how` | Partial | Native local solar eclipse circumstances are implemented and contract-tested. Exact Swiss-compatible contact timing, output-array shape, and locality validation remain before intact migration. |
| `swe_lun_eclipse_how` | `jme_lun_eclipse_how` | Partial | Native lunar eclipse circumstances are implemented and contract-tested for total and penumbral cases. Broader Swiss-compatible output validation remains before intact migration. |
| `swe_sol_eclipse_when_glob` | `jme_sol_eclipse_when_glob` | Partial | Native global solar search/classification is implemented and tested for total, annular, and hybrid cases. Broader event catalog validation, search-direction behavior, and output shape parity remain. |
| `swe_sol_eclipse_when_loc` | `jme_sol_eclipse_when_loc` | Partial | Native local solar eclipse search is implemented and contract-tested. Exact location-aware contacts, visibility flags, and output-array parity remain before intact migration. |
| `swe_lun_eclipse_when` | `jme_lun_eclipse_when` | Partial | Native lunar eclipse search is implemented and tested for total and penumbral cases. Broader catalog/date-range validation and output-array parity remain. |
| `swe_lun_eclipse_when_loc` | `jme_lun_eclipse_when_loc` | Partial | Native local lunar eclipse visibility/contact clipping is implemented and tested. A fully independent topocentric shadow-contact model and output-array parity remain. |

### Constant Readiness

The in-tree C API is JME-native only. The old Swiss-style compatibility header is not present in this repository build, so required Swiss constants must be mapped through a separate migration wrapper or adapter.

Constant coverage status:

| Area | Status | Notes |
|---|---|---|
| Body IDs | Mapped in JME-native constants | Required bodies have `JME_BODY_*` equivalents. A migration adapter must expose any required `SE_*` names and confirm value semantics. |
| Calendar flags | Mapped in JME-native constants | Gregorian/Julian calendar behavior exists through `JME_CALENDAR_*`. A migration adapter must expose any required `SE_*` names. |
| Calculation flags | Mapped in JME-native constants, partial as behavior | Required flag concepts have `JME_CALC_*` equivalents, but behavior behind these flags must be verified in `jme_calc_ut`. |
| Rise/transit flags | Mapped in JME-native constants, partial as behavior | Rise/set/twilight/transit concepts exist in JME, but `jme_rise_trans` still needs full intact-I/O validation. |
| Eclipse flags | Mapped as JME constants, partial as behavior | JME eclipse constants exist, but they are not guaranteed Swiss-style flag values. Eclipse behavior is implemented but still needs intact I/O and broader parity validation for `panchang-core`. |
| Sidereal modes | Mapped in JME-native constants, partial as behavior | Required sidereal concepts have JME equivalents. Ayanamsa calculations need mode-by-mode parity verification and adapter-level name/value mapping. |

## Migration Blockers For Intact I/O

These are the blockers before replacing Swiss Ephemeris in `vedic_astrology` and `panchang-core` without output changes.

### `vedic_astrology`

Primary blockers:

1. `swe_calc_ut` parity through `jme_calc_ut`
   - verify Sun, Moon, Mercury, Venus, Mars, Jupiter, Saturn, mean node
   - verify longitude, latitude, distance, and speed tuple layout
   - verify `SEFLG_SWIEPH`, `SEFLG_SIDEREAL`, `SEFLG_SPEED`, `SEFLG_EQUATORIAL`, and `SEFLG_HELCTR`
   - verify return flag behavior

2. `swe_get_ayanamsa_ut` / `swe_set_sid_mode`
   - verify Lahiri first
   - verify user-defined ayanamsa mode
   - verify Fagan-Bradley, Raman, Krishnamurti, and True Chitra if tests/debug paths matter

3. `swe_houses`
   - verify cusp indexing and `ascmc` layout
   - verify default house system behavior
   - verify sidereal ascendant workflow currently used by the app

4. `swe_rise_trans`
   - verify sunrise/sunset and moonrise/moonset for the app's locations and date ranges
   - verify disc-center behavior
   - verify returned Julian day tuple layout

5. `swe_time_equ`
   - verify equation-of-time output for shadbala calculations

Lower-risk items:

- `swe_julday`
- `swe_revjul`
- `swe_day_of_week`
- `swe_sidtime`

### `panchang-core`

Primary blockers:

1. Eclipse APIs
   - `swe_sol_eclipse_how`
   - `swe_sol_eclipse_when_glob`
   - `swe_sol_eclipse_when_loc`
   - `swe_lun_eclipse_how`
   - `swe_lun_eclipse_when`
   - `swe_lun_eclipse_when_loc`

   Current JME equivalents are implemented but not yet sufficient for intact panchang-core eclipse JSON output. Local circumstances, visibility flags, type flags, contact times, and output arrays must be compared against existing Swiss-based output.

2. `swe_rise_trans`
   - must preserve sunrise, sunset, moonrise, moonset, transit, and twilight behavior
   - must preserve date-boundary behavior already handled in `panchang-core`

3. `swe_calc_ut`
   - must preserve sidereal Sun/Moon longitudes for tithi, nakshatra, yoga, karana, sankranti, month, and muhurta calculations
   - speed output must remain available where used

4. `swe_get_ayanamsa_ut` / `swe_set_sid_mode`
   - Lahiri output must match current Swiss-based panchang output

5. `swe_houses`
   - lagna table output must match current Swiss-based calculations

Lower-risk items:

- `swe_julday`
- `swe_jdut1_to_utc`, after wrapper shape is added and event time formatting is verified
- constants, after the migration layer maps required Swiss-style names and values onto the JME-native constants intentionally

## Required Compatibility Layer

To use JME in these two projects with minimal caller changes, add a compatibility layer that exposes Swiss-style names and signatures:

- C/PHP-facing compatibility names: `swe_*`
- Python-facing compatibility names: `calc_ut`, `julday`, `revjul`, `houses`, etc.
- constants: `SE_*`, `SEFLG_*`, and Python aliases such as `SUN`, `MOON`, `FLG_SWIEPH`, `SIDM_LAHIRI`

The compatibility layer must preserve:

- argument order
- pointer/output-array shape
- return flags
- tuple structure in Python
- cusp indexing
- `ascmc` indexing
- error string behavior where callers pass `serr`
- no rounding or output normalization beyond what Swiss-compatible behavior requires

Until this layer and the blockers above are complete, JME should not be treated as a drop-in replacement for the current Swiss-based `panchang-core` and `vedic_astrology` outputs.
