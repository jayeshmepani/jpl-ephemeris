#!/usr/bin/env python3
"""Generate tests/fixtures/occult_reference.h: lunar occultations of planets
and stars computed independently with Skyfield (DE440s), used to certify
jme_lun_occult_when_loc() and jme_lun_occult_when_glob() on the JPL path.

Reference software: Skyfield 1.55 (MIT), pyerfa 2.0.1.5 (BSD-3, star space
motion with eraStarpm), SciPy (BSD-3, Nelder-Mead minimisation).

Definitions shared with JME (and stated in its documentation):
  - contact geometry from astrometric topocentric places (light time and
    deflection by Sun, Jupiter, Saturn and the Earth; no aberration) and
    geometric radii: the exact barycentric-frame condition (aberration does
    not preserve angles); Skyfield's apparent() is used with its aberration
    step disabled for these evaluations; altitudes use complete apparent
    places; WGS84 observer, no refraction, no polar motion; fixed Delta-T (69.2 s) passed to JME with
    jme_set_delta_t_userdef;
  - angular radii asin(R / light-time distance): Moon 1737.4 km, Venus
    6051.8 km, Mars 3396.2 km, Jupiter 71492 km, Saturn 60268 km (Mars,
    Jupiter, Saturn at their system barycentres as in de440s: JME is run
    with JPL_CENTER=BARYCENTER); stars are points;
  - local: disappearance/reappearance c1/c4 are the roots of
    sep - (r_moon + r_target), inner contacts (planets fully hidden) of
    sep - (r_moon - r_target), maximum = minimum topocentric separation;
    the event returned is the first after jd_start during which the
    target's geometric altitude is > 0 at some instant of [c1, c4];
  - global: first/last contact anywhere on the Earth = roots of
    G(t) = min over the WGS84 surface of [sep - (r_moon + r_target)],
    evaluated here by direct minimisation over (lat, lon) of the
    topocentric function (an independent method; JME uses the
    fundamental-plane construction).
Run:
    python3 tools/gen_occult_fixtures.py /path/to/de440s.bsp > tests/fixtures/occult_reference.h
"""
import hashlib, math, re, sys, warnings
import numpy as np
import erfa, skyfield, scipy
from scipy.optimize import minimize
from skyfield.api import load, wgs84, Star
from skyfield.units import Angle
import skyfield.positionlib as _positionlib

_add_aberration = _positionlib.add_aberration
_ABERRATION = [True]
def _aberration_switch(*args):
    if _ABERRATION[0]:
        _add_aberration(*args)
_positionlib.add_aberration = _aberration_switch

def astrometric_deflected(observer_at, target):
    _ABERRATION[0] = False
    try:
        return observer_at.observe(target).apparent()
    finally:
        _ABERRATION[0] = True

path = sys.argv[1]
digest = hashlib.sha256(open(path, 'rb').read()).hexdigest()
DELTA_T = 69.2
ts = load.timescale(delta_t=DELTA_T)
eph = load(path)
earth, moon = eph['earth'], eph['moon']
R_MOON = 1737.4
AU_KM = 149597870.700

rows = {}
for line in open(__file__.rsplit('/', 2)[0] + '/src/fixstar_hipparcos.inc'):
    m = re.match(r'\s*\{(\d+), (\d+), ([-\d.]+), ([-\d.]+), ([-\d.]+), ([-\d.]+), ([-\d.]+), ([-\d.]+), (\d)\}', line)
    if m:
        rows[int(m.group(1))] = [float(x) for x in m.groups()[2:8]]
STAR_HR = {'Spica': 5056, 'Antares': 6134, 'Regulus': 3982}
MAS = math.pi / 180.0 / 3.6e6

def star_at(name, jd_tt):
    ra, dec, pma, pmd, plx, rv = rows[STAR_HR[name]]
    r1, d1 = math.radians(ra), math.radians(dec)
    t = ts.tt_jd(jd_tt)
    tdb = t.tdb
    with warnings.catch_warnings():
        warnings.simplefilter('error', erfa.ErfaWarning)
        r2, d2, pr2, pd2, px2, rv2 = erfa.starpm(r1, d1, pma * MAS / math.cos(d1), pmd * MAS, plx / 1000.0, rv,
                                                 2448349.0625, 0.0, math.floor(tdb), tdb - math.floor(tdb))
    return Star(ra=Angle(radians=r2), dec=Angle(radians=d2), ra_mas_per_year=pr2 * math.cos(d2) / MAS,
                dec_mas_per_year=pd2 / MAS, parallax_mas=px2 * 1000.0, radial_km_per_s=rv2, epoch=t)

# (JME body, star name, Skyfield target, radius km, start UT1 JD)
CASES = [
    (4, None, 'mars barycenter', 3396.2, ts.ut1(2022, 12, 1).ut1),
    (3, None, 'venus', 6051.8, ts.ut1(2023, 11, 1).ut1),
    (5, None, 'jupiter barycenter', 71492.0, ts.ut1(2023, 3, 15).ut1),
    (6, None, 'saturn barycenter', 60268.0, ts.ut1(2024, 7, 15).ut1),
    (6, None, 'saturn barycenter', 60268.0, ts.ut1(2024, 10, 10).ut1),
    (-1, 'Spica', None, 0.0, ts.ut1(2024, 12, 20).ut1),
    (-1, 'Antares', None, 0.0, ts.ut1(2024, 3, 25).ut1),
    (-1, 'Regulus', None, 0.0, ts.ut1(2026, 5, 15).ut1),
]
SITES = [(-118.2437, 34.0522, 89.0), (-3.7038, 40.4168, 667.0), (69.6669, 23.2420, 110.0),
         (151.2093, -33.8688, 58.0), (139.6917, 35.6895, 40.0), (18.4241, -33.9249, 10.0),
         (-70.6693, -33.4489, 570.0), (-43.1729, -22.9068, 10.0)]

def target_obj(case, jd):
    body, star, name, rad, _ = case
    return star_at(star, jd) if star else eph[name]

def geo_sep(case, t, tg):
    e = earth.at(t)
    return e.observe(moon).apparent().separation_from(e.observe(tg).apparent()).degrees

def local_f(case, obs, tg, jd, kind=1):
    t = ts.ut1_jd(jd)
    o = obs.at(t)
    m = astrometric_deflected(o, moon)
    p = astrometric_deflected(o, tg)
    sep = m.separation_from(p).degrees
    rm = np.degrees(np.arcsin(R_MOON / (m.distance().au * AU_KM)))
    rt = np.degrees(np.arcsin(case[3] / (p.distance().au * AU_KM))) if case[3] > 0 else 0.0 * sep
    if kind == 0: return sep
    if kind == 1: return sep - (rm + rt)
    return sep - (rm - rt)

def bisect(f, a, b, tol=1e-11):
    fa = f(a)
    fb = f(b)
    assert (fa < 0) != (fb < 0), (fa, fb)
    for _ in range(200):
        if abs(b - a) < tol: break
        m = 0.5 * (a + b); fm = f(m)
        if (fm < 0) == (fa < 0): a, fa = m, fm
        else: b, fb = m, fm
    return 0.5 * (a + b)

def argmin_precise(f, a, b):
    # golden section, then root of the symmetric difference (h = 1e-5 d)
    g = (math.sqrt(5) - 1) / 2
    x1, x2 = b - g * (b - a), a + g * (b - a); f1, f2 = f(x1), f(x2)
    while b - a > 1e-7:
        if f1 > f2: a, x1, f1 = x1, x2, f2; x2 = a + g * (b - a); f2 = f(x2)
        else: b, x2, f2 = x2, x1, f1; x1 = b - g * (b - a); f1 = f(x1)
    x = 0.5 * (a + b)
    d = lambda t: f(t + 1e-5) - f(t - 1e-5)
    e = 2e-6
    while not (d(x - e) < 0 <= d(x + e)): e *= 2
    return bisect(d, x - e, x + e)

def conjunctions(case, start, days):
    jd = np.arange(start, start + days, 0.05)
    t = ts.ut1_jd(jd)
    tg = target_obj(case, start + days / 2)
    s = geo_sep(case, t, tg)
    return [jd[i] for i in range(1, len(s) - 1) if s[i] <= s[i - 1] and s[i] <= s[i + 1] and s[i] < 1.5]

def local_event(case, site, start):
    lon, lat, h = site
    obs = earth + wgs84.latlon(lat, lon, h)
    for jc in conjunctions(case, start, 420.0):
        tg = target_obj(case, jc)
        jd = np.arange(jc - 0.4, jc + 0.4, 10.0 / 86400.0)
        f = local_f(case, obs, tg, jd)
        neg = f < 0
        if not neg.any(): continue
        i0 = int(np.argmax(neg)); i1 = len(neg) - 1 - int(np.argmax(neg[::-1]))
        if i0 == 0 or i1 == len(neg) - 1: raise SystemExit('grid too short')
        if not neg[i0:i1 + 1].all(): raise SystemExit('two intervals')
        fs = lambda x: float(local_f(case, obs, tg, x))
        c1 = bisect(lambda x: -fs(x), jd[i0 - 1], jd[i0])
        c4 = bisect(fs, jd[i1], jd[i1 + 1])
        if c1 < start: continue
        tt = np.linspace(c1, c4, 400)
        alt = obs.at(ts.ut1_jd(tt)).observe(tg).apparent().altaz()[0].degrees
        if not (alt > 0).any():
            continue
        if (alt > 0).all() is False and min(abs(alt)) < 1e-3:
            raise SystemExit('ambiguous visibility')
        tmax = argmin_precise(lambda x: float(local_f(case, obs, tg, x, 0)), c1, c4)
        i1r = i4r = 0.0
        if case[3] > 0 and float(local_f(case, obs, tg, tmax, 2)) < 0:
            f2 = lambda x: float(local_f(case, obs, tg, x, 2))
            i1r = bisect(lambda x: -f2(x), c1, tmax)
            i4r = bisect(f2, tmax, c4)
        elif case[3] == 0:
            i1r, i4r = c1, c4
        return tmax, c1, c4, i1r, i4r
    return None

def surface_min(case, tg, jd, guess):
    t = ts.ut1_jd(jd)
    e = earth.at(t)
    def f(v):
        lat, lon = v
        lat = max(-90.0, min(90.0, lat))
        o = (earth + wgs84.latlon(lat, lon, 0.0)).at(t)
        m = astrometric_deflected(o, moon); p = astrometric_deflected(o, tg)
        sep = m.separation_from(p).degrees
        rm = math.degrees(math.asin(R_MOON / (m.distance().au * AU_KM)))
        rt = math.degrees(math.asin(case[3] / (p.distance().au * AU_KM))) if case[3] > 0 else 0.0
        return sep - (rm + rt)
    if guess is None:
        best = None
        for lat in range(-87, 90, 6):
            for lon in range(-180, 180, 6):
                v = f((lat, lon))
                if best is None or v < best[0]: best = (v, (lat, lon))
        guess = best[1]
    r = minimize(f, guess, method='Nelder-Mead', options={'xatol': 1e-10, 'fatol': 1e-14, 'maxiter': 4000})
    return r.fun, tuple(r.x)

def global_event(case, start):
    for jc in conjunctions(case, start, 420.0):
        tg = target_obj(case, jc)
        g0, x0 = surface_min(case, tg, jc, None)
        if g0 >= 0:
            # check around the conjunction (the global minimum is within hours)
            ok = False
            for d in (-0.1, 0.1):
                g, _ = surface_min(case, tg, jc + d, None)
                if g < 0: ok = True; jc += d; x0 = _; break
            if not ok: continue
        guess = [x0]
        def G(jd):
            # warm-started minimisation; a non-negative result (the only kind
            # that can end a walk or move a bisection bracket wrongly) is
            # confirmed by a fresh global grid search, since the warm start
            # can follow a local minimum
            v, x = surface_min(case, tg, jd, guess[0])
            if v >= 0:
                v2, x2 = surface_min(case, tg, jd, None)
                if v2 < v: v, x = v2, x2
            guess[0] = x
            return v
        # walk outwards to bracket each contact
        a = jc
        while G(a) < 0: a -= 0.02
        p1 = bisect(lambda x: -G(x), a, a + 0.02, 1e-9)
        guess[0] = x0
        b = jc
        while G(b) < 0: b += 0.02
        p4 = bisect(G, b - 0.02, b, 1e-9)
        if p1 < start: continue
        return p1, p4
    return None

print('/* Generated by tools/gen_occult_fixtures.py with Skyfield %s, pyerfa %s, SciPy %s' % (skyfield.__version__, erfa.__version__, scipy.__version__))
print(' * from de440s.bsp (sha256 %s). Delta-T fixed at %.1f s. Do not edit. */' % (digest, DELTA_T))
print('typedef struct { int body; const char *star; double lon, lat, height, jd_start, delta_t, tmax, c1, c4, i1, i4; } jme_occult_local_fixture;')
print('typedef struct { int body; const char *star; double jd_start, delta_t, p1, p4; } jme_occult_glob_fixture;')
def dt_at(jd):
    t = ts.ut1_jd(jd); return float((t.tt - t.ut1) * 86400.0)
def cstr(s): return '"%s"' % s if s else '0'
print('static const jme_occult_local_fixture occult_local_fixtures[] = {')
for case in CASES:
    for site in SITES:
        ev = local_event(case, site, case[4])
        if ev is None: continue
        print('    {%d, %s, %r, %r, %r, %r, %r, %r, %r, %r, %r, %r},' % ((case[0], cstr(case[1])) + site + (float(case[4]), dt_at(ev[0])) + tuple(float(x) for x in ev)))
        sys.stdout.flush()
print('};')
print('static const jme_occult_glob_fixture occult_glob_fixtures[] = {')
for case in CASES:
    ev = global_event(case, case[4])
    if ev is None: continue
    print('    {%d, %s, %r, %r, %r, %r},' % (case[0], cstr(case[1]), float(case[4]), dt_at(ev[0]), float(ev[0]), float(ev[1])))
    sys.stdout.flush()
print('};')
