#!/usr/bin/env python3
"""Generate tests/fixtures/houses_reference.h: house cusps recomputed
independently of JME from the geometric definitions of each system, with
plain 3-D vector geometry in the true equator/equinox frame of date (no
spherical-trigonometry cusp formulas shared with src/houses.c), and the
sidereal-time / obliquity chain from pyerfa (ERFA, BSD-3).

Definitions (standard; e.g. R. Hand, "Astrological houses" literature and
the Swiss Ephemeris documentation sect. 6, consulted for conventions only):
  Asc  ecliptic intersection with the horizon, east side.
  MC   ecliptic intersection with the meridian, upper side (hour angle 0).
  Equal: Asc + 30(k-1).  Whole sign: sign of Asc + 30(k-1).
  Porphyry: trisection of the ecliptic arcs MC-Asc and Asc-IC.
  Campanus: great circles through the horizon north/south points dividing
            the prime vertical into 30 degree parts.
  Regiomontanus: same circles dividing the celestial equator into 30 degree
            parts from the east point.
  Placidus: points whose hour angle is 1/3, 2/3 of their diurnal (cusps 11,
            12) or nocturnal (2, 3) semi-arc from the meridian / horizon.
  Alcabitius: hour circles trisecting the Ascendant's diurnal and nocturnal
            semi-arcs on the equator.
  Polich-Page (topocentric): cusps 11, 12, 2, 3 have oblique ascensions
            ARMC+30, +60, +120, +150, i.e. they are ascendants for RAMC =
            ARMC-60, -30, +30, +60 at pole heights atan(tan(phi)/3), atan(2 tan(phi)/3),
            atan(2 tan(phi)/3), atan(tan(phi)/3) (Polich & Page 1964).
Cusps 4..9 are the opposite points of 10..3.

Second table: full time chain for jme_houses() under IAU2006A: ARMC =
ERFA gst06a(UT1, TT) + longitude, true obliquity = obl06 + deps(nut06a),
with a fixed Delta-T passed to JME; MC and Asc from those values.

Run: python3 tools/gen_houses_fixtures.py > tests/fixtures/houses_reference.h
"""
import math
import erfa

D = math.pi / 180.0

def ecl(lam, eps):
    return (math.cos(lam), math.sin(lam) * math.cos(eps), math.sin(lam) * math.sin(eps))

def dot(a, b): return a[0]*b[0] + a[1]*b[1] + a[2]*b[2]
def cross(a, b): return (a[1]*b[2]-a[2]*b[1], a[2]*b[0]-a[0]*b[2], a[0]*b[1]-a[1]*b[0])

def plane_ecl(normal, eps, side):
    """ecliptic longitude (rad) in the plane with given normal, with dot(e, side) > 0"""
    # e(l) . n = cos l * n0 + sin l * (n1 cos eps + n2 sin eps) = 0
    a = normal[0]; b = normal[1] * math.cos(eps) + normal[2] * math.sin(eps)
    l = math.atan2(-a, b)
    for cand in (l, l + math.pi):
        if dot(ecl(cand, eps), side) > 0:
            return cand % (2 * math.pi)
    raise ValueError('no side')

def frame(armc, phi):
    z = (math.cos(phi) * math.cos(armc), math.cos(phi) * math.sin(armc), math.sin(phi))
    east = (-math.sin(armc), math.cos(armc), 0.0)
    north = cross(z, east)
    return z, east, north

def asc_mc(armc, phi, eps):
    z, east, north = frame(armc, phi)
    asc = plane_ecl(z, eps, east)
    mc = plane_ecl(east, eps, (math.cos(armc), math.sin(armc), 0.0))
    return asc, mc

def ra_dec(lam, eps):
    v = ecl(lam, eps)
    return math.atan2(v[1], v[0]), math.asin(v[2])

def wrap(x):
    return (x + math.pi) % (2 * math.pi) - math.pi

def bisect(f, a, b):
    fa = f(a)
    for _ in range(200):
        m = 0.5 * (a + b)
        fm = f(m)
        if (fm < 0) == (fa < 0):
            a, fa = m, fm
        else:
            b = m
        if b - a < 1e-15:
            break
    return 0.5 * (a + b)

def root_in(f, lo, hi, n=2000):
    """first sign change of f on [lo, hi] (longitudes, hi may exceed 2pi)"""
    prev = lo; fp = f(lo)
    for i in range(1, n + 1):
        x = lo + (hi - lo) * i / n
        fx = f(x)
        if (fp < 0) != (fx < 0) and abs(fx - fp) < math.pi:
            return bisect(f, prev, x) % (2 * math.pi)
        prev, fp = x, fx
    raise ValueError('no root')

def cusps(system, armc, phi, eps):
    asc, mc = asc_mc(armc, phi, eps)
    c = [0.0] * 13
    c[1], c[10] = asc, mc
    if system == 'E':
        for k in range(1, 13): c[k] = asc + (k - 1) * math.pi / 6
    elif system == 'W':
        s = math.floor(asc / (math.pi / 6)) * math.pi / 6
        for k in range(1, 13): c[k] = s + (k - 1) * math.pi / 6
    else:
        z, east, north = frame(armc, phi)
        if system == 'O':
            q1 = (asc - mc) % (2 * math.pi); q2 = math.pi - q1
            c[11] = mc + q1 / 3; c[12] = mc + 2 * q1 / 3
            c[2] = asc + q2 / 3; c[3] = asc + 2 * q2 / 3
        elif system in 'CR':
            for k, ang in ((11, 60), (12, 30), (2, -30), (3, -60)):
                a = ang * D
                if system == 'C':
                    p = tuple(math.cos(a) * east[i] + math.sin(a) * z[i] for i in range(3))
                else:
                    ra = armc + (90 - ang) * D
                    p = (math.cos(ra), math.sin(ra), 0.0)
                c[k] = plane_ecl(cross(north, p), eps, p)
        elif system == 'P':
            def ha_sa(lam):
                ra, de = ra_dec(lam, eps)
                x = -math.tan(phi) * math.tan(de)
                dsa = math.acos(max(-1.0, min(1.0, x)))
                return wrap(armc - ra), dsa
            q1 = (asc - mc) % (2 * math.pi)
            for k, frac in ((11, 1 / 3), (12, 2 / 3)):
                c[k] = root_in(lambda l: wrap(ha_sa(l)[0] + frac * ha_sa(l)[1]), mc + 1e-9, mc + q1 - 1e-9)
            for k, frac in ((2, 1 / 3), (3, 2 / 3)):
                c[k] = root_in(lambda l: wrap(ha_sa(l)[0] + ha_sa(l)[1] + frac * (math.pi - ha_sa(l)[1])),
                               asc + 1e-9, asc + math.pi - q1 - 1e-9)
        elif system == 'A':
            ra_a, de_a = ra_dec(asc, eps)
            dsa = math.acos(-math.tan(phi) * math.tan(de_a)); nsa = math.pi - dsa
            for k, ra in ((11, armc + dsa / 3), (12, armc + 2 * dsa / 3),
                          (2, armc + dsa + nsa / 3), (3, armc + dsa + 2 * nsa / 3)):
                c[k] = math.atan2(math.sin(ra), math.cos(ra) * math.cos(eps))
        elif system == 'T':
            t = math.tan(phi)
            for k, da, f in ((11, 30, 1 / 3), (12, 60, 2 / 3), (2, 120, 2 / 3), (3, 150, 1 / 3)):
                c[k] = asc_mc(armc + (da - 90) * D, math.atan(f * t), eps)[0]
        for k in (4, 5, 6, 7, 8, 9):
            c[k] = c[k + 6 if k < 7 else k - 6] + math.pi
    return [(x % (2 * math.pi)) / D for x in c[1:]]

SYSTEMS = {'P': 'JME_HOUSE_PLACIDUS', 'R': 'JME_HOUSE_REGIOMONTANUS', 'C': 'JME_HOUSE_CAMPANUS',
           'A': 'JME_HOUSE_ALCABITIUS', 'T': 'JME_HOUSE_POLICH_PAGE', 'O': 'JME_HOUSE_PORPHYRIUS',
           'E': 'JME_HOUSE_EQUAL', 'W': 'JME_HOUSE_WHOLE_SIGN'}
ARMCS = [0.0, 1e-9, 37.25, 89.999, 90.0, 123.456789, 180.0, 241.5, 270.0, 333.333, 359.9999999]
LATS = [0.0, 1e-7, 23.242, -33.8688, 45.0, 51.5072, -55.0, 60.0, 65.5]
EPS = [23.4392911, 23.0, 24.2]

print('/* Generated by tools/gen_houses_fixtures.py (pyerfa %s). Do not edit. */' % erfa.__version__)
print('typedef struct { int hsys; double armc, lat, eps; double cusp[12]; } jme_house_fixture;')
print('static const jme_house_fixture house_fixtures[] = {')
for s, name in SYSTEMS.items():
    for armc in ARMCS:
        for lat in LATS:
            for eps in EPS:
                if s in 'PA' and abs(lat) > 90 - eps - 1:  # semi-arc systems undefined near/above polar circle
                    continue
                cs = cusps(s, armc * D, lat * D, eps * D)
                print('    {%s, %r, %r, %r, {%s}},' % (name, armc, lat, eps, ', '.join(repr(x) for x in cs)))
print('};')

# full time chain
print('typedef struct { double jd_ut, delta_t, lon, lat; double armc, eps, asc, mc; } jme_house_time_fixture;')
print('static const jme_house_time_fixture house_time_fixtures[] = {')
for jd_ut, dt in ((2451545.0, 63.8285), (2460000.25, 69.2), (2433282.42345905, 29.15), (2488070.0, 95.0)):
    for lon, lat in ((69.6669, 23.242), (-0.1276, 51.5072), (151.2093, -33.8688), (-179.999, 10.0)):
        tt = jd_ut + dt / 86400.0
        u1, u2 = math.floor(jd_ut), jd_ut - math.floor(jd_ut)
        t1, t2 = math.floor(tt), tt - math.floor(tt)
        gst = erfa.gst06a(u1, u2, t1, t2)
        dpsi, deps = erfa.nut06a(t1, t2)
        eps = erfa.obl06(t1, t2) + deps
        armc = (gst / D + lon) % 360.0
        asc, mc = asc_mc(armc * D, lat * D, eps)
        print('    {%r, %r, %r, %r, %r, %r, %r, %r},' % (jd_ut, dt, lon, lat, float(armc), float(eps / D), asc / D % 360, mc / D % 360))
print('};')
