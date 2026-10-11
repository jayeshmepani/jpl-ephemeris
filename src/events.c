#include "jme/jme.h"
#include "context.h"
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#ifdef _WIN32
#include <windows.h>
#else
#include <sys/time.h>
#endif

#define JME_EVENTS_RAD_TO_DEG 57.295779513082320876798154814105170332405472466565
#define JME_EVENTS_DEG_TO_RAD 0.017453292519943295769236907684886127134428718885417
#define JME_EVENTS_PI 3.14159265358979323846264338327950288419716939937510
#define JME_EVENTS_EARTH_RADIUS_KM 6378.137
#define JME_EVENTS_MOON_RADIUS_KM 1737.4
#define JME_EVENTS_SUN_RADIUS_KM 695700.0

/*
 * Contact geometry of eclipses and occultations uses astrometric places
 * (light time and gravitational deflection, no aberration) compared with
 * geometric angular radii asin(R/d). This is the exact condition in the
 * barycentric frame: the ray from the target (or the Sun's limb) grazes the
 * Moon at its light-time position and reaches the observer. Aberration is
 * the same change of frame for every direction but does not preserve
 * angles (it scales them by about 1 - (v/c) cos(theta)), so comparing
 * apparent separations with unaberrated radii misplaces contacts by up to
 * ~0.1" (~0.2 s); and the aberrated geocentric Moon vector is a direction,
 * not a position, so using it as a position on the fundamental plane is
 * wrong by (R_earth / d_moon) x 20" (~0.3", ~0.6 s at the limb).
 * Altitudes (visibility) still use complete apparent places.
 */
#define JME_EVENTS_GEOMETRY_FLAGS ((int)JME_CALC_NO_ABERRATION)

/*
 * Greenwich sidereal time (hours) consistent with the coordinates being
 * compared: apparent (equation of the equinoxes included) for apparent
 * true-equator-of-date positions, mean when JME_CALC_NO_NUTATION is set.
 */
static double events_gst_hours(double jd_ut, int flags)
{
    double hours = NAN;
    if (jme_sidereal_time_ex(jd_ut, (flags & JME_CALC_NO_NUTATION) == 0, &hours, 0) != JME_OK) {
        return NAN;
    }
    return hours;
}


/*
 * Bracketed root refinement (Illinois variant of regula falsi) for event
 * searches. Invariant: f(lo) < 0 <= f(hi) (lo/hi may be in either time
 * order). Terminates with a bracket no wider than 1e-10 day (8.6 us); the
 * returned instant is the bracket midpoint. Superlinear convergence; each
 * step keeps a valid bracket, so the result is as reliable as bisection.
 */
typedef double (*events_root_fn)(double t, const void *ctx, char *error);

static int events_refine_root(events_root_fn f, const void *ctx, double lo, double flo, double hi, double fhi, double *root, char *error)
{
    const double tol = 1e-10;
    double m_prev = NAN;
    int side = 0, it;

    for (it = 0; it < 300 && fabs(hi - lo) > tol; it++) {
        double m = (lo * fhi - hi * flo) / (fhi - flo);
        double fm;
        if (!(isfinite(m)) || !((m > lo && m < hi) || (m < lo && m > hi))) m = 0.5 * (lo + hi);
        if (isfinite(m_prev) && fabs(m - m_prev) < 0.25 * tol) {
            /* interpolation has converged: close the bracket around m */
            double a = m - 0.5 * tol * (hi > lo ? 1.0 : -1.0);
            double b = m + 0.5 * tol * (hi > lo ? 1.0 : -1.0);
            double fa = f(a, ctx, error), fb = f(b, ctx, error);
            if (!isfinite(fa) || !isfinite(fb)) return JME_ERR;
            if (fa < 0.0 && fb >= 0.0) { lo = a; hi = b; break; }
            m = 0.5 * (lo + hi); /* fall back to a bisection step */
        }
        m_prev = m;
        fm = f(m, ctx, error);
        if (!isfinite(fm)) return JME_ERR;
        if (fm >= 0.0) {
            hi = m; fhi = fm;
            if (side == 1) flo *= 0.5;
            side = 1;
        } else {
            lo = m; flo = fm;
            if (side == -1) fhi *= 0.5;
            side = -1;
        }
    }
    *root = 0.5 * (lo + hi);
    return JME_OK;
}


/*
 * One-dimensional minimisation on [a, b]: Brent's method (Brent 1973,
 * "Algorithms for Minimization without Derivatives", ch. 5, procedure
 * localmin) - parabolic interpolation safeguarded by golden-section steps.
 * Absolute tolerance tol in days.
 */
static int events_minimize(events_root_fn f, const void *ctx, double a, double b, double tol, double *xmin, char *error)
{
    const double cg = 0.38196601125010515180; /* (3 - sqrt 5) / 2 */
    double x, w, v, fx, fw, fv, d = 0.0, e = 0.0, u, fu;
    int it;

    if (a > b) { double t = a; a = b; b = t; }
    x = w = v = a + cg * (b - a);
    fx = fw = fv = f(x, ctx, error);
    if (!isfinite(fx)) return JME_ERR;
    for (it = 0; it < 300; it++) {
        double m = 0.5 * (a + b);
        double tol1 = tol, tol2 = 2.0 * tol;
        int golden = 1;
        if (fabs(x - m) <= tol2 - 0.5 * (b - a)) break;
        if (fabs(e) > tol1) {
            double r = (x - w) * (fx - fv);
            double q = (x - v) * (fx - fw);
            double p = (x - v) * q - (x - w) * r;
            double etemp;
            q = 2.0 * (q - r);
            if (q > 0.0) p = -p; else q = -q;
            etemp = e;
            e = d;
            if (fabs(p) < fabs(0.5 * q * etemp) && p > q * (a - x) && p < q * (b - x)) {
                d = p / q;
                u = x + d;
                if (u - a < tol2 || b - u < tol2) d = (x < m) ? tol1 : -tol1;
                golden = 0;
            }
        }
        if (golden) {
            e = (x < m) ? b - x : a - x;
            d = cg * e;
        }
        u = (fabs(d) >= tol1) ? x + d : x + (d > 0.0 ? tol1 : -tol1);
        fu = f(u, ctx, error);
        if (!isfinite(fu)) return JME_ERR;
        if (fu <= fx) {
            if (u < x) b = x; else a = x;
            v = w; fv = fw; w = x; fw = fx; x = u; fx = fu;
        } else {
            if (u < x) a = u; else b = u;
            if (fu <= fw || w == x) {
                v = w; fv = fw; w = u; fw = fu;
            } else if (fu <= fv || v == x || v == w) {
                v = u; fv = fu;
            }
        }
    }
    *xmin = x;
    return JME_OK;
}

/*
 * Precise location of a smooth minimum found by events_minimize: the
 * minimum is flat (f - f_min ~ (t - t0)^2), so comparing values resolves its
 * abscissa only to ~sqrt(machine epsilon). It is refined as the root of the
 * symmetric difference g(t) = f(t + h) - f(t - h), h = 1e-5 day, whose zero
 * differs from that of the derivative by O(h^2 f3/f2) (about 1e-11 day).
 */
typedef struct events_stationary_ctx { events_root_fn f; const void *ctx; double h; } events_stationary_ctx;

static double events_stationary_fn(double t, const void *c, char *error)
{
    const events_stationary_ctx *s = (const events_stationary_ctx *)c;
    double fp = s->f(t + s->h, s->ctx, error);
    double fm = s->f(t - s->h, s->ctx, error);
    return fp - fm;
}

static int events_minimize_precise(events_root_fn f, const void *ctx, double a, double b, double *xmin, char *error)
{
    events_stationary_ctx sc;
    double x, delta, glo, ghi;
    int k;

    if (events_minimize(f, ctx, a, b, 1e-7, &x, error) != JME_OK) return JME_ERR;
    sc.f = f;
    sc.ctx = ctx;
    sc.h = 1e-5;
    for (k = 0, delta = 2e-6; k < 12; k++, delta *= 2.0) {
        glo = events_stationary_fn(x - delta, &sc, error);
        ghi = events_stationary_fn(x + delta, &sc, error);
        if (!isfinite(glo) || !isfinite(ghi)) return JME_ERR;
        if (glo < 0.0 && ghi >= 0.0) {
            return events_refine_root(events_stationary_fn, &sc, x - delta, glo, x + delta, ghi, xmin, error);
        }
    }
    /* no bracket (minimum at an interval end): keep the Brent result */
    *xmin = x;
    return JME_OK;
}

static unsigned long long g_profile_rise_trans_calls = 0ULL;
static double g_profile_rise_trans_seconds = 0.0;
static unsigned long long g_profile_altitude_calls = 0ULL;
static double g_profile_altitude_seconds = 0.0;
static int g_profile_events_enabled = 0;
static int g_profile_events_init = 0;

static double jme_profile_events_now_seconds(void)
{
#ifdef _WIN32
    LARGE_INTEGER frequency;
    LARGE_INTEGER counter;
    QueryPerformanceFrequency(&frequency);
    QueryPerformanceCounter(&counter);
    return (double)counter.QuadPart / (double)frequency.QuadPart;
#else
    struct timeval tv;
    gettimeofday(&tv, 0);
    return (double)tv.tv_sec + ((double)tv.tv_usec / 1000000.0);
#endif
}

static void jme_profile_events_report(void)
{
    fprintf(stderr, "[jme-profile] rise_trans calls=%llu total_s=%.6f per_call_s=%.9f\n",
        g_profile_rise_trans_calls,
        g_profile_rise_trans_seconds,
        g_profile_rise_trans_calls > 0ULL ? (g_profile_rise_trans_seconds / (double)g_profile_rise_trans_calls) : 0.0);
    fprintf(stderr, "[jme-profile] altitude calls=%llu total_s=%.6f per_call_s=%.9f\n",
        g_profile_altitude_calls,
        g_profile_altitude_seconds,
        g_profile_altitude_calls > 0ULL ? (g_profile_altitude_seconds / (double)g_profile_altitude_calls) : 0.0);
}

static void jme_profile_events_maybe_init(void)
{
    if (g_profile_events_init) {
        return;
    }
    g_profile_events_init = 1;
    g_profile_events_enabled = getenv("JME_PROFILE") != 0;
    if (g_profile_events_enabled) {
        atexit(jme_profile_events_report);
    }
}

static double vec_dot(const double *a, const double *b)
{
    return a[0] * b[0] + a[1] * b[1] + a[2] * b[2];
}

static void vec_cross(const double *a, const double *b, double *out)
{
    out[0] = a[1] * b[2] - a[2] * b[1];
    out[1] = a[2] * b[0] - a[0] * b[2];
    out[2] = a[0] * b[1] - a[1] * b[0];
}

static double vec_norm(const double *a)
{
    return sqrt(vec_dot(a, a));
}

static double orbital_radius_from_elements(double a, double e, double true_anomaly_deg)
{
    double denom = 1.0 + e * cos(true_anomaly_deg * JME_EVENTS_DEG_TO_RAD);

    if (!isfinite(a) || !isfinite(e) || !isfinite(true_anomaly_deg) || a <= 0.0 || denom == 0.0) {
        return NAN;
    }

    return a * (1.0 - e * e) / denom;
}

static double get_altitude(double jd_ut, int body, const char *starname, int flags, double *geopos, double atpress, double attemp, char *error);

static int heliacal_body_from_data(const double *dat_hel, int *body, char *error)
{
    int candidate;

    if (dat_hel == 0 || body == 0) {
        jme_set_error(error, "Heliacal body selection requires input data");
        return JME_ERR;
    }

    if (!isfinite(dat_hel[0])) {
        jme_set_error(error, "Heliacal body selection requires a finite body id");
        return JME_ERR;
    }

    candidate = (int)dat_hel[0];
    if ((double)candidate != dat_hel[0]) {
        jme_set_error(error, "Heliacal body selection requires an integer body id");
        return JME_ERR;
    }

    if (candidate < JME_BODY_MOON || candidate > JME_BODY_PLUTO) {
        jme_set_error(error, "Heliacal body selection supports Moon and planetary bodies only");
        return JME_ERR;
    }

    *body = candidate;
    return JME_OK;
}

static double heliacal_required_arcus(double magnitude)
{
    if (!isfinite(magnitude)) {
        return NAN;
    }

    /*
     * Stellar and planetary first/last visibility threshold.
     * Independent visibility references commonly use the linear relation
     * arcus visionis = 10.50 + 1.40 * visual magnitude for heliacal events.
     */
    return 10.50 + 1.40 * magnitude;
}

static double heliacal_limiting_magnitude(double arcus)
{
    if (!isfinite(arcus)) {
        return NAN;
    }

    return (arcus - 10.50) / 1.40;
}

static int heliacal_visibility_flag(double body_alt, double sun_alt, double arcus, double magnitude, double limiting_mag, double required_arcus)
{
    return magnitude <= limiting_mag
        && body_alt >= 0.0
        && sun_alt < 0.0
        && arcus >= required_arcus;
}

#define JME_SOLAR_VISIBLE_UPPER_LIMB_ALTITUDE_DEG (-0.8333)

/*
 * Lunar-eclipse shadow geometry, geocentric and angular (Danjon's
 * convention as used by the NASA lunar eclipse canon, Espenak & Meeus 2009,
 * and the Explanatory Supplement 1992 sect. 8.4):
 *   pi1   = asin(0.998340 a / D_moon)   (Earth radius at latitude 45 deg)
 *   s     = asin(R_sun / D_sun),  pi_s = asin(a / D_sun)
 *   umbra     rho_u = (1 + 1/85) pi1 - s + pi_s
 *   penumbra  rho_p = (1 + 1/85) pi1 + s + pi_s
 *   theta = angle between the Moon and the anti-Sun, s_m = asin(R_moon/D_moon)
 *   umbral magnitude    (rho_u + s_m - theta) / (2 s_m), penumbral likewise.
 * Apparent geocentric places. Lengths returned in km are the angles times
 * the Moon's distance (consistent units for the callers).
 */
typedef struct lunar_shadow {
    double theta, rho_u, rho_p, s_m, d_moon_km, along_km;
} lunar_shadow;

static int lunar_shadow_at(double jd_ut, lunar_shadow *ls, char *error)
{
    double sun[6], moon[6], ds, dm, c;
    const double a = JME_EVENTS_EARTH_RADIUS_KM;
    double pi1, s_sun, pi_sun;
    int i;

    if (internal_calc_position_ut(jd_ut, JME_BODY_SUN, 0, sun, error) != JME_OK
        || internal_calc_position_ut(jd_ut, JME_BODY_MOON, 0, moon, error) != JME_OK) {
        return JME_ERR;
    }
    for (i = 0; i < 3; i++) { sun[i] *= JME_AU_KM; moon[i] *= JME_AU_KM; }
    ds = vec_norm(sun);
    dm = vec_norm(moon);
    if (!isfinite(ds) || !isfinite(dm) || ds <= 0.0 || dm <= 0.0) {
        jme_set_error(error, "Lunar eclipse geometry requires valid Sun and Moon distances");
        return JME_ERR;
    }
    /* angle between Moon and anti-Sun, robust form atan2(|u x v|, u.v) */
    {
        double cr[3], anti[3] = {-sun[0], -sun[1], -sun[2]};
        cr[0] = moon[1] * anti[2] - moon[2] * anti[1];
        cr[1] = moon[2] * anti[0] - moon[0] * anti[2];
        cr[2] = moon[0] * anti[1] - moon[1] * anti[0];
        c = vec_dot(moon, anti);
        ls->theta = atan2(vec_norm(cr), c);
        ls->along_km = c / ds;
    }
    pi1 = asin(0.998340 * a / dm);
    s_sun = asin(JME_EVENTS_SUN_RADIUS_KM / ds);
    pi_sun = asin(a / ds);
    ls->rho_u = (1.0 + 1.0 / 85.0) * pi1 - s_sun + pi_sun;
    ls->rho_p = (1.0 + 1.0 / 85.0) * pi1 + s_sun + pi_sun;
    ls->s_m = asin(JME_EVENTS_MOON_RADIUS_KM / dm);
    ls->d_moon_km = dm;
    return JME_OK;
}

static int lunar_eclipse_geometry(double jd_ut, double *umbral_mag, double *penumbral_mag, double *umbral_radius_km, double *penumbral_radius_km, double *offset_km, double *axis_distance_km, char *error)
{
    lunar_shadow ls;

    if (lunar_shadow_at(jd_ut, &ls, error) != JME_OK) return JME_ERR;
    if (umbral_mag != 0) *umbral_mag = (ls.rho_u + ls.s_m - ls.theta) / (2.0 * ls.s_m);
    if (penumbral_mag != 0) *penumbral_mag = (ls.rho_p + ls.s_m - ls.theta) / (2.0 * ls.s_m);
    if (umbral_radius_km != 0) *umbral_radius_km = ls.rho_u * ls.d_moon_km;
    if (penumbral_radius_km != 0) *penumbral_radius_km = ls.rho_p * ls.d_moon_km;
    if (offset_km != 0) *offset_km = ls.theta * ls.d_moon_km;
    if (axis_distance_km != 0) *axis_distance_km = ls.along_km;
    return JME_OK;
}

/* Greatest lunar eclipse: minimum angular distance of the Moon's centre
 * from the shadow axis (NASA canon definition). */
static double lunar_eclipse_offset_metric(double jd_ut, char *error)
{
    lunar_shadow ls;

    if (lunar_shadow_at(jd_ut, &ls, error) != JME_OK) return NAN;
    if (ls.along_km <= 0.0) return JME_EVENTS_PI + ls.theta;
    return ls.theta;
}

/*
 * Syzygy search: instant at which the apparent geocentric ecliptic
 * longitude of the Moon minus that of the Sun equals target (0 = new moon,
 * 180 = full moon). Only genuine crossings are accepted (both bracket ends
 * within 90 deg of the target), so the +-180 deg wrap of the difference is
 * never mistaken for a root. Refined to 1e-10 day.
 */
typedef struct syzygy_ctx { double target; } syzygy_ctx;

static double syzygy_function(double t, const void *ctx, char *error)
{
    double sun[3], moon[3], d;
    if (internal_calc_position_ut(t, JME_BODY_SUN, 0, sun, error) != JME_OK
        || internal_calc_position_ut(t, JME_BODY_MOON, 0, moon, error) != JME_OK) {
        return NAN;
    }
    d = (atan2(moon[1], moon[0]) - atan2(sun[1], sun[0])) * JME_EVENTS_RAD_TO_DEG - ((const syzygy_ctx *)ctx)->target;
    return jme_degrees_difference_signed(jme_degree_normalize(d), 0.0);
}

static double syzygy_function_neg(double t, const void *ctx, char *error)
{
    return -syzygy_function(t, ctx, error);
}

static int find_syzygy(double jd_start, int backward, double target, double *tret, char *error)
{
    double direction = backward ? -1.0 : 1.0;
    double t0 = jd_start, f0, t1, f1;
    syzygy_ctx c;
    int i;

    if (!isfinite(jd_start)) {
        jme_set_error(error, "Julian date must be finite");
        return JME_ERR;
    }
    c.target = target;
    f0 = syzygy_function(t0, &c, error);
    if (!isfinite(f0)) return JME_ERR;
    for (i = 1; i <= 120; i++) {
        t1 = jd_start + direction * (0.5 * i);
        f1 = syzygy_function(t1, &c, error);
        if (!isfinite(f1)) return JME_ERR;
        if (fabs(f0) < 90.0 && fabs(f1) < 90.0 && ((f0 < 0.0) != (f1 < 0.0) || f1 == 0.0)) {
            if (f1 == 0.0) { *tret = t1; return JME_OK; }
            if (f0 < 0.0) return events_refine_root(syzygy_function, &c, t0, f0, t1, f1, tret, error);
            {
                syzygy_ctx n;
                n.target = target;
                /* make the lower end negative by evaluating -f */
                return events_refine_root(syzygy_function_neg, &n, t1, -f1, t0, -f0, tret, error);
            }
        }
        t0 = t1;
        f0 = f1;
    }
    jme_set_error(error, "Syzygy not found in search window");
    return JME_ERR;
}

static int find_full_moon_opposition(double jd_start, int backward, double *tret, char *error)
{
    return find_syzygy(jd_start, backward, 180.0, tret, error);
}

static int find_new_moon_conjunction(double jd_start, int backward, double *tret, char *error)
{
    return find_syzygy(jd_start, backward, 0.0, tret, error);
}


static double lunar_offset_min_fn(double t, const void *ctx, char *error)
{
    (void)ctx;
    return lunar_eclipse_offset_metric(t, error);
}

static int refine_lunar_eclipse_maximum(double guess, double *tmax, char *error)
{
    return events_minimize_precise(lunar_offset_min_fn, 0, guess - 0.75, guess + 0.75, tmax, error);
}

/* kind 0: penumbral outer contact (P1/P4), 1: umbral outer (U1/U4),
 * 2: umbral inner (U2/U3). Negative while the Moon is inside the limit;
 * shadow radii re-evaluated at each instant. */
static double lunar_contact_function(double t, int kind, char *error)
{
    lunar_shadow ls;
    if (lunar_shadow_at(t, &ls, error) != JME_OK) return NAN;
    if (kind == 0) return ls.theta - (ls.rho_p + ls.s_m);
    if (kind == 1) return ls.theta - (ls.rho_u + ls.s_m);
    return ls.theta - (ls.rho_u - ls.s_m);
}

typedef struct lunar_contact_ctx { int kind; } lunar_contact_ctx;
static double lunar_contact_root_fn(double t, const void *ctx, char *error)
{
    return lunar_contact_function(t, ((const lunar_contact_ctx *)ctx)->kind, error);
}

static int find_contact_time(double tmid, int kind, int left_side, double *tret, char *error)
{
    double step = 0.01, prev = tmid, f;
    int i;

    f = lunar_contact_function(tmid, kind, error);
    if (!isfinite(f) || f >= 0.0) return JME_ERR;
    for (i = 1; i <= 40; i++) {
        double t = tmid + (left_side ? -1.0 : 1.0) * step * i;
        double fprev = f;
        f = lunar_contact_function(t, kind, error);
        if (!isfinite(f)) return JME_ERR;
        if (f >= 0.0) {
            lunar_contact_ctx c;
            c.kind = kind;
            return events_refine_root(lunar_contact_root_fn, &c, prev, fprev, t, f, tret, error);
        }
        prev = t;
    }
    return JME_ERR;
}

static int classify_lunar_eclipse(double umbral_mag, double penumbral_mag)
{
    if (umbral_mag >= 1.0) {
        return JME_ECLIPSE_LUNAR_TOTAL;
    }
    if (umbral_mag > 0.0) {
        return JME_ECLIPSE_LUNAR_PARTIAL;
    }
    if (penumbral_mag > 0.0) {
        return JME_ECLIPSE_LUNAR_PENUMBRAL;
    }
    return 0;
}

static int moon_is_above_horizon(double jd_ut, double *geopos, char *error, double *altitude_out)
{
    double altitude = get_altitude(jd_ut, JME_BODY_MOON, 0, 0, geopos, 0.0, 0.0, error);

    if (altitude_out != 0) {
        *altitude_out = altitude;
    }

    return altitude > 0.0;
}

static double moon_altitude_metric(double jd_ut, double *geopos, char *error)
{
    double altitude = get_altitude(jd_ut, JME_BODY_MOON, 0, 0, geopos, 0.0, 0.0, error);
    (void)error;
    return altitude;
}

static int refine_moon_horizon_crossing(double t0, double t1, double *geopos, double *tret, char *error)
{
    double f0 = moon_altitude_metric(t0, geopos, error);
    double f1 = moon_altitude_metric(t1, geopos, error);
    int i;

    if (tret == 0 || !isfinite(f0) || !isfinite(f1) || f0 * f1 > 0.0) {
        return JME_ERR;
    }

    for (i = 0; i < 48; i++) {
        double m = 0.5 * (t0 + t1);
        double fm = moon_altitude_metric(m, geopos, error);

        if (!isfinite(fm)) {
            return JME_ERR;
        }
        if (fabs(fm) < 1.0e-7 || fabs(t1 - t0) < 1.0e-8) {
            *tret = m;
            return JME_OK;
        }
        if (f0 * fm <= 0.0) {
            t1 = m;
            f1 = fm;
        } else {
            t0 = m;
            f0 = fm;
        }
    }

    (void)f1;
    *tret = 0.5 * (t0 + t1);
    return JME_OK;
}

static int find_visible_moon_interval(double start, double end, double *geopos, double *visible_start, double *visible_end, char *error)
{
    double step;
    double prev_t;
    double prev_alt;
    int prev_above;
    int in_visible;
    int found = 0;
    int i;

    if (visible_start == 0 || visible_end == 0 || geopos == 0 || !isfinite(start) || !isfinite(end) || end <= start) {
        jme_set_error(error, "Local lunar eclipse visibility interval is invalid");
        return JME_ERR;
    }

    step = (end - start) / 96.0;
    if (step < 1.0 / 1440.0) {
        step = 1.0 / 1440.0;
    }

    prev_t = start;
    prev_alt = moon_altitude_metric(prev_t, geopos, error);
    if (!isfinite(prev_alt)) {
        return JME_ERR;
    }
    prev_above = prev_alt > 0.0;
    in_visible = prev_above;
    if (in_visible) {
        *visible_start = start;
        found = 1;
    }

    for (i = 1; i <= 192; i++) {
        double t = start + step * i;
        double alt;
        int above;

        if (t > end) {
            t = end;
        }

        alt = moon_altitude_metric(t, geopos, error);
        if (!isfinite(alt)) {
            return JME_ERR;
        }
        above = alt > 0.0;

        if (!prev_above && above) {
            double cross = t;
            refine_moon_horizon_crossing(prev_t, t, geopos, &cross, error);
            if (!found) {
                *visible_start = cross;
            }
            found = 1;
            in_visible = 1;
        } else if (prev_above && !above) {
            double cross = t;
            refine_moon_horizon_crossing(prev_t, t, geopos, &cross, error);
            if (in_visible) {
                *visible_end = cross;
                return JME_OK;
            }
        }

        if (t >= end) {
            break;
        }
        prev_t = t;
        prev_alt = alt;
        prev_above = above;
    }

    if (found && in_visible) {
        *visible_end = end;
        return JME_OK;
    }

    jme_set_error(error, "No locally visible lunar eclipse interval found");
    return JME_ERR;
}


static double angular_radius_deg(double radius_km, double distance_km)
{
    if (!isfinite(radius_km) || !isfinite(distance_km) || radius_km <= 0.0 || distance_km <= radius_km) {
        return NAN;
    }

    return asin(radius_km / distance_km) * JME_EVENTS_RAD_TO_DEG;
}

static double normalize_lon_180(double lon)
{
    lon = jme_degree_normalize(lon);
    if (lon > 180.0) {
        lon -= 360.0;
    }
    return lon;
}


static int solar_eclipse_local_geometry(double jd_ut, double *geopos, double *separation_deg, double *sun_radius_deg, double *moon_radius_deg, double *sun_alt_deg, double *moon_alt_deg, char *error)
{
    double sun_eq[6];
    double moon_eq[6];
    double sun_dist_km;
    double moon_dist_km;

    if (geopos == 0) {
        jme_set_error(error, "Solar eclipse local geometry requires geopos");
        return JME_ERR;
    }

    internal_topo_override_set(geopos[0], geopos[1], geopos[2]);

    if (internal_calc_position_ut(jd_ut, JME_BODY_SUN, JME_CALC_EQUATORIAL | JME_CALC_TOPOCENTRIC | JME_EVENTS_GEOMETRY_FLAGS, sun_eq, error) != JME_OK
        || internal_calc_position_ut(jd_ut, JME_BODY_MOON, JME_CALC_EQUATORIAL | JME_CALC_TOPOCENTRIC | JME_EVENTS_GEOMETRY_FLAGS, moon_eq, error) != JME_OK) {
        return JME_ERR;
    }

    sun_dist_km = vec_norm(sun_eq) * JME_AU_KM;
    moon_dist_km = vec_norm(moon_eq) * JME_AU_KM;

    if (separation_deg != 0) {
        double cr[3];
        cr[0] = sun_eq[1] * moon_eq[2] - sun_eq[2] * moon_eq[1];
        cr[1] = sun_eq[2] * moon_eq[0] - sun_eq[0] * moon_eq[2];
        cr[2] = sun_eq[0] * moon_eq[1] - sun_eq[1] * moon_eq[0];
        *separation_deg = atan2(vec_norm(cr), vec_dot(sun_eq, moon_eq)) * JME_EVENTS_RAD_TO_DEG;
    }
    if (sun_radius_deg != 0) {
        *sun_radius_deg = angular_radius_deg(JME_EVENTS_SUN_RADIUS_KM, sun_dist_km);
    }
    if (moon_radius_deg != 0) {
        *moon_radius_deg = angular_radius_deg(JME_EVENTS_MOON_RADIUS_KM, moon_dist_km);
    }
    if (sun_alt_deg != 0) {
        *sun_alt_deg = get_altitude(jd_ut, JME_BODY_SUN, 0, 0, geopos, 0.0, 0.0, error);
    }
    if (moon_alt_deg != 0) {
        *moon_alt_deg = get_altitude(jd_ut, JME_BODY_MOON, 0, 0, geopos, 0.0, 0.0, error);
    }

    return JME_OK;
}

static int classify_solar_eclipse(double sep_deg, double sun_r_deg, double moon_r_deg)
{
    if (!isfinite(sep_deg) || !isfinite(sun_r_deg) || !isfinite(moon_r_deg)) {
        return 0;
    }
    if (sep_deg > sun_r_deg + moon_r_deg) {
        return 0;
    }
    if (moon_r_deg >= sun_r_deg && sep_deg <= moon_r_deg - sun_r_deg) {
        return JME_ECLIPSE_SOLAR_TOTAL;
    }
    if (sun_r_deg > moon_r_deg && sep_deg <= sun_r_deg - moon_r_deg) {
        return JME_ECLIPSE_SOLAR_ANNULAR;
    }
    return JME_ECLIPSE_SOLAR_PARTIAL;
}



/*
 * Greatest eclipse (global): the instant at which the axis of the lunar
 * shadow passes closest to the Earth's centre (minimum gamma), the
 * definition used by the NASA eclipse catalogues (Espenak & Meeus 2006,
 * NASA TP-2006-214141, sect. 1) and the Explanatory Supplement. Apparent
 * geocentric places of Sun and Moon are used. This differs from the minimum
 * of the geocentric Sun-Moon separation by up to a few seconds for eclipses
 * of large gamma.
 */
static double solar_eclipse_global_metric(double jd_ut, char *error)
{
    double sun[6], moon[6], axis[3], closest[3], d, s;
    int i;

    if (internal_calc_position_ut(jd_ut, JME_BODY_SUN, JME_EVENTS_GEOMETRY_FLAGS, sun, error) != JME_OK
        || internal_calc_position_ut(jd_ut, JME_BODY_MOON, JME_EVENTS_GEOMETRY_FLAGS, moon, error) != JME_OK) {
        return NAN;
    }
    for (i = 0; i < 3; i++) axis[i] = moon[i] - sun[i];
    d = vec_norm(axis);
    if (!isfinite(d) || d <= 0.0) return NAN;
    for (i = 0; i < 3; i++) axis[i] /= d;
    s = -vec_dot(moon, axis);
    for (i = 0; i < 3; i++) closest[i] = moon[i] + s * axis[i];
    return vec_norm(closest) * JME_AU_KM;
}

/* Local maximum: minimum topocentric apparent Sun-Moon separation
 * (independent of the Sun's altitude; visibility is judged separately). */
static double solar_eclipse_local_metric(double jd_ut, double *geopos, char *error)
{
    double sep = NAN;

    if (solar_eclipse_local_geometry(jd_ut, geopos, &sep, 0, 0, 0, 0, error) != JME_OK) {
        return NAN;
    }
    return sep;
}

typedef struct solar_max_ctx { double *geopos; int local_mode; } solar_max_ctx;
static double solar_max_fn(double t, const void *ctx, char *error)
{
    const solar_max_ctx *c = (const solar_max_ctx *)ctx;
    return c->local_mode ? solar_eclipse_local_metric(t, c->geopos, error) : solar_eclipse_global_metric(t, error);
}

static int refine_solar_maximum(double guess, double *geopos, int local_mode, double *tmax, char *error)
{
    solar_max_ctx c;
    c.geopos = geopos;
    c.local_mode = local_mode;
    return events_minimize_precise(solar_max_fn, &c, guess - 0.75, guess + 0.75, tmax, error);
}

/*
 * Occultation / eclipse target: a body (JME body id) or a catalogue star
 * (star != NULL and non-empty). Body = JME_BODY_SUN with no star is the Sun.
 */
typedef struct events_target { int body; const char *star; } events_target;

static int events_target_is_star(const events_target *t)
{
    return t != 0 && t->star != 0 && t->star[0] != '\0';
}

static double event_body_radius_km(int body);

static double events_target_radius_km(const events_target *t)
{
    if (t == 0) return JME_EVENTS_SUN_RADIUS_KM;
    return events_target_is_star(t) ? 0.0 : event_body_radius_km(t->body);
}

/* Complete apparent place, position only, rectangular (au) in the frame
 * selected by flags. *dist_known is 0 for a star without parallax (pos is
 * then a unit vector, the star being at infinity). */
static int events_target_position(double jd_ut, const events_target *t, int flags, double *pos, int *dist_known, char *error)
{
    if (events_target_is_star(t)) {
        double dist = 0.0;
        if (internal_fixstar_position_ut(t->star, jd_ut, flags, pos, &dist, error) != JME_OK) return JME_ERR;
        if (dist_known != 0) *dist_known = dist > 0.0;
        return JME_OK;
    }
    if (dist_known != 0) *dist_known = 1;
    return internal_calc_position_ut(jd_ut, t == 0 ? JME_BODY_SUN : t->body, flags, pos, error);
}

static double event_body_radius_km(int body)
{
    switch (body) {
    case JME_BODY_SUN: return JME_EVENTS_SUN_RADIUS_KM;
    case JME_BODY_MOON: return JME_EVENTS_MOON_RADIUS_KM;
    case JME_BODY_MERCURY: return 2439.7;
    case JME_BODY_VENUS: return 6051.8;
    case JME_BODY_MARS: return 3396.2;
    case JME_BODY_JUPITER: return 71492.0;
    case JME_BODY_SATURN: return 60268.0;
    case JME_BODY_URANUS: return 25559.0;
    case JME_BODY_NEPTUNE: return 24764.0;
    case JME_BODY_PLUTO: return 1188.3;
    default: return 0.0;
    }
}

static double lunar_horizontal_parallax_deg(double moon_radius_deg)
{
    double moon_distance_km;

    if (!isfinite(moon_radius_deg) || moon_radius_deg <= 0.0) {
        return NAN;
    }

    moon_distance_km = JME_EVENTS_MOON_RADIUS_KM / sin(moon_radius_deg * JME_EVENTS_DEG_TO_RAD);
    return angular_radius_deg(JME_EVENTS_EARTH_RADIUS_KM, moon_distance_km);
}

/*
 * Local contact: root of sep(t) - (r_sun(t) + r_moon(t)) (outer, inner = 0)
 * or sep(t) - |r_sun(t) - r_moon(t)| (inner = 1), all topocentric apparent
 * and re-evaluated at every trial instant.
 */
static double solar_local_contact_function(double t, double *geopos, int inner, char *error)
{
    double sep, sr, mr;
    if (solar_eclipse_local_geometry(t, geopos, &sep, &sr, &mr, 0, 0, error) != JME_OK
        || !isfinite(sep) || !isfinite(sr) || !isfinite(mr)) {
        return NAN;
    }
    return inner ? sep - fabs(sr - mr) : sep - (sr + mr);
}

typedef struct solar_local_ctx { double *geopos; int inner; } solar_local_ctx;
static double solar_local_root_fn(double t, const void *ctx, char *error)
{
    const solar_local_ctx *c = (const solar_local_ctx *)ctx;
    return solar_local_contact_function(t, c->geopos, c->inner, error);
}

static int find_solar_contact_time(double tmid, double *geopos, int inner, int left_side, double *tret, char *error)
{
    double step = 0.01;
    double t_prev = tmid;
    double f_prev = solar_local_contact_function(t_prev, geopos, inner, error);
    int i;

    if (!isfinite(f_prev)) return JME_ERR;
    if (f_prev > 0.0) return JME_ERR; /* no overlap of this kind at tmid */
    for (i = 1; i <= 40; i++) {
        double t = tmid + (left_side ? -1.0 : 1.0) * step * i;
        double f = solar_local_contact_function(t, geopos, inner, error);
        if (!isfinite(f)) return JME_ERR;
        if (f >= 0.0) {
            solar_local_ctx c;
            c.geopos = geopos;
            c.inner = inner;
            return events_refine_root(solar_local_root_fn, &c, t_prev, f_prev, t, f, tret, error);
        }
        t_prev = t;
        f_prev = f;
    }
    return JME_ERR;
}

/* Fraction of the solar disc area covered by the lunar disc (radii and
 * separation in the same angular unit); exact circle-overlap area. */
static double solar_obscuration(double sep, double rs, double rm)
{
    double area;
    if (!isfinite(sep) || !isfinite(rs) || !isfinite(rm) || rs <= 0.0 || rm <= 0.0) return NAN;
    if (sep >= rs + rm) return 0.0;
    if (sep <= fabs(rs - rm)) return rm >= rs ? 1.0 : (rm * rm) / (rs * rs);
    area = rm * rm * acos((sep * sep + rm * rm - rs * rs) / (2.0 * sep * rm))
        + rs * rs * acos((sep * sep + rs * rs - rm * rm) / (2.0 * sep * rs))
        - 0.5 * sqrt((-sep + rm + rs) * (sep + rm - rs) * (sep - rm + rs) * (sep + rm + rs));
    return area / (JME_EVENTS_PI * rs * rs);
}

static double get_altitude_internal(double jd_ut, int body, const char *starname, int flags, double *geopos, double atpress, double attemp, int set_topo, char *error)
{
    double t0 = 0.0;
    double results[6];
    double ra, dec, hour_angle, azimuth, altitude;
    double gst;
    int equatorial_flag = (int)JME_CALC_EQUATORIAL;
    int res;

    if (set_topo) {
        internal_topo_override_set(geopos[0], geopos[1], geopos[2]);
    }

    jme_profile_events_maybe_init();
    if (g_profile_events_enabled) {
        t0 = jme_profile_events_now_seconds();
    }

    if (starname != 0 && starname[0] != '\0') {
        double xyz[6];
        flags = flags | (int)JME_CALC_TOPOCENTRIC | equatorial_flag;
        res = internal_fixstar_position_ut(starname, jd_ut, flags, xyz, 0, error);
        if (res == JME_OK) {
            xyz[3] = xyz[4] = xyz[5] = 0.0;
            if (jme_rectangular_to_spherical_state(xyz, results) != JME_OK) {
                jme_set_error(error, "Invalid star vector");
                res = JME_ERR;
            }
        }
    } else {
        flags = flags | (int)JME_CALC_TOPOCENTRIC | equatorial_flag;
        res = internal_calc_ut_positions(jd_ut, body, flags, results, error);
    }

    if (res != JME_OK) {
        if (g_profile_events_enabled) {
            g_profile_altitude_calls += 1ULL;
            g_profile_altitude_seconds += (jme_profile_events_now_seconds() - t0);
        }
        return -100.0;
    }

    ra = results[0];
    dec = results[1];

    gst = events_gst_hours(jd_ut, flags);
    hour_angle = jme_degree_normalize(gst * 15.0 + geopos[0] - ra);

    jme_equatorial_to_horizontal(hour_angle, dec, geopos[1], &azimuth, &altitude);

    if (atpress > 0.0) {
        altitude = jme_refract_extended(
            altitude,
            geopos != 0 ? geopos[2] : 0.0,
            atpress,
            attemp,
            jme_get_context()->lapse_rate,
            JME_COORD_TRUE_TO_APPARENT,
            0
        );
    }

    if (g_profile_events_enabled) {
        g_profile_altitude_calls += 1ULL;
        g_profile_altitude_seconds += (jme_profile_events_now_seconds() - t0);
    }
    return altitude;
}

static double get_altitude(double jd_ut, int body, const char *starname, int flags, double *geopos, double atpress, double attemp, char *error)
{
    return get_altitude_internal(jd_ut, body, starname, flags, geopos, atpress, attemp, 1, error);
}

typedef struct events_alt_ctx {
    int body; const char *starname; int flags; double *geopos; double atpress, attemp, h0;
} events_alt_ctx;

static double events_alt_fn(double t, const void *c, char *error)
{
    const events_alt_ctx *x = (const events_alt_ctx *)c;
    double a = get_altitude_internal(t, x->body, x->starname, x->flags, x->geopos, x->atpress, x->attemp, 0, error);
    return a == -100.0 ? NAN : a - x->h0;
}

/* Altitude crossing of h0 in [t0, t1], refined to 1e-10 day (events_refine_root). */
static int refine_altitude_crossing(
    double t0,
    double t1,
    double h0,
    int body,
    const char *starname,
    int flags,
    double *geopos,
    double atpress,
    double attemp,
    double *tret,
    char *error
)
{
    events_alt_ctx c;
    double a0, a1;

    internal_topo_override_set(geopos[0], geopos[1], geopos[2]);
    c.body = body; c.starname = starname; c.flags = flags; c.geopos = geopos;
    c.atpress = atpress; c.attemp = attemp; c.h0 = h0;
    a0 = events_alt_fn(t0, &c, error);
    a1 = events_alt_fn(t1, &c, error);
    if (!isfinite(a0) || !isfinite(a1) || a0 * a1 > 0.0) {
        return JME_ERR;
    }
    if (a0 == 0.0) { *tret = t0; return JME_OK; }
    if (a1 == 0.0) { *tret = t1; return JME_OK; }
    return a0 < 0.0
        ? events_refine_root(events_alt_fn, &c, t0, a0, t1, a1, tret, error)
        : events_refine_root(events_alt_fn, &c, t1, a1, t0, a0, tret, error);
}

typedef struct events_ha_ctx {
    int body; const char *starname; int flags; double *geopos; double target_hour_angle;
} events_ha_ctx;

static double events_ha_fn(double t, const void *c, char *error)
{
    const events_ha_ctx *x = (const events_ha_ctx *)c;
    double results[6];
    if (x->starname != 0 && x->starname[0] != '\0') {
        if (jme_fixstar_ut(x->starname, t, x->flags | JME_CALC_EQUATORIAL, results, error) != JME_OK) return NAN;
    } else if (internal_calc_ut_positions(t, x->body, x->flags | JME_CALC_EQUATORIAL, results, error) != JME_OK) {
        return NAN;
    }
    return jme_degrees_difference_signed(
        jme_degrees_difference_signed(events_gst_hours(t, x->flags) * 15.0 + x->geopos[0], results[0]),
        x->target_hour_angle);
}

/* Hour angle reaching target_hour_angle in [t0, t1], refined to 1e-10 day. */
static int refine_meridian_transit(
    double t0,
    double t1,
    int body,
    const char *starname,
    int flags,
    double *geopos,
    double target_hour_angle,
    double *tret,
    char *error
)
{
    events_ha_ctx c;
    double ha0, ha1;

    c.body = body; c.starname = starname; c.flags = flags; c.geopos = geopos;
    c.target_hour_angle = target_hour_angle;
    ha0 = events_ha_fn(t0, &c, error);
    ha1 = events_ha_fn(t1, &c, error);
    if (!isfinite(ha0) || !isfinite(ha1) || ha0 * ha1 > 0.0) {
        return JME_ERR;
    }
    /* reject the +-180 degree wrap: a true crossing has a small step */
    if (fabs(ha0 - ha1) > 180.0) return JME_ERR;
    if (ha0 == 0.0) { *tret = t0; return JME_OK; }
    if (ha1 == 0.0) { *tret = t1; return JME_OK; }
    return ha0 < 0.0
        ? events_refine_root(events_ha_fn, &c, t0, ha0, t1, ha1, tret, error)
        : events_refine_root(events_ha_fn, &c, t1, ha1, t0, ha0, tret, error);
}

static int rise_trans_search(double jd_ut, int body, const char *starname, int flags, int rsmi, double *geopos, double atpress, double attemp, int use_true_horizon, double true_horizon, double *tret, char *error)
{
    double search_t0 = 0.0;
    double t = jd_ut;
    double h0 = -0.5667; /* Standard for stars (refraction only) */
    double search_atpress = atpress;
    double alt, alt_next;
    int i;

    if (tret != 0) { *tret = 0.0; }
    jme_profile_events_maybe_init();
    if (g_profile_events_enabled) {
        search_t0 = jme_profile_events_now_seconds();
    }
    if (geopos == 0 || tret == 0) {
        jme_set_error(error, "Rise and transit output arguments are required");
        if (g_profile_events_enabled) {
            g_profile_rise_trans_calls += 1ULL;
            g_profile_rise_trans_seconds += (jme_profile_events_now_seconds() - search_t0);
        }
        return JME_ERR;
    }

    if (body == JME_BODY_SUN || body == JME_BODY_MOON) {
        h0 = -0.8333; /* Includes semi-diameter */
    }
    if (rsmi & JME_RISE_NO_REFRACTION) {
        search_atpress = 0.0;
        h0 = 0.0;
    }
    if (rsmi & JME_RISE_DISC_CENTER) {
        h0 = (rsmi & JME_RISE_NO_REFRACTION) ? 0.0 : -0.5667;
    }
    if (rsmi & JME_RISE_CIVIL_TWILIGHT) {
        h0 = -6.0;
    } else if (rsmi & JME_RISE_NAUTICAL_TWILIGHT) {
        h0 = -12.0;
    } else if (rsmi & JME_RISE_ASTRONOMICAL_TWILIGHT) {
        h0 = -18.0;
    }
    if (use_true_horizon) {
        h0 = true_horizon;
    }

    internal_topo_override_set(geopos[0], geopos[1], geopos[2]);

    if (rsmi & (JME_RISE_MERIDIAN_TRANSIT | JME_RISE_ANTI_MERIDIAN_TRANSIT)) {
        double target_hour_angle = (rsmi & JME_RISE_ANTI_MERIDIAN_TRANSIT) ? 180.0 : 0.0;
        double dt = 0.01;
        /* Search for transit from the supplied Julian day across one day. */
        double results[6];
        double ra, gst, ha, ha_next;
        if (starname != 0 && starname[0] != '\0') {
            if (jme_fixstar_ut(starname, t, flags | JME_CALC_EQUATORIAL, results, error) != JME_OK) {
                return JME_ERR;
            }
        } else {
            if (internal_calc_ut_positions(t, body, flags | JME_CALC_EQUATORIAL, results, error) != JME_OK) {
                return JME_ERR;
            }
        }
        ra = results[0];
        gst = events_gst_hours(t, flags);
        ha = jme_degrees_difference_signed(jme_degrees_difference_signed(gst * 15.0 + geopos[0], ra), target_hour_angle);

        for (i = 0; i < 240; i++) {
            t += dt; /* 14.4 mins */
            gst = events_gst_hours(t, flags);
            if (starname != 0 && starname[0] != '\0') {
                if (jme_fixstar_ut(starname, t, flags | JME_CALC_EQUATORIAL, results, error) != JME_OK) {
                    return JME_ERR;
                }
            } else {
                if (internal_calc_ut_positions(t, body, flags | JME_CALC_EQUATORIAL, results, error) != JME_OK) {
                    return JME_ERR;
                }
            }
            ra = results[0];
            ha_next = jme_degrees_difference_signed(jme_degrees_difference_signed(gst * 15.0 + geopos[0], ra), target_hour_angle);

            if (ha < 0 && ha_next >= 0) {
                {
                    int rc = refine_meridian_transit(t - dt, t, body, starname, flags, geopos, target_hour_angle, tret, error);
                    if (g_profile_events_enabled) {
                        g_profile_rise_trans_calls += 1ULL;
                        g_profile_rise_trans_seconds += (jme_profile_events_now_seconds() - search_t0);
                    }
                    return rc;
                }
            }
            ha = ha_next;
        }
    }

    /* Rise/set search across one day. */
    alt = get_altitude_internal(t, body, starname, flags, geopos, search_atpress, attemp, 0, error);
    for (i = 0; i < 24; i++) {
        t += 1.0 / 24.0;
        alt_next = get_altitude_internal(t, body, starname, flags, geopos, search_atpress, attemp, 0, error);

        if (rsmi & JME_RISE_RISE) {
            if (alt < h0 && alt_next >= h0) {
                {
                    int rc = refine_altitude_crossing(t - 1.0 / 24.0, t, h0, body, starname, flags, geopos, search_atpress, attemp, tret, error);
                    if (g_profile_events_enabled) {
                        g_profile_rise_trans_calls += 1ULL;
                        g_profile_rise_trans_seconds += (jme_profile_events_now_seconds() - search_t0);
                    }
                    return rc;
                }
            }
        } else if (rsmi & JME_RISE_SET) {
            if (alt > h0 && alt_next <= h0) {
                {
                    int rc = refine_altitude_crossing(t - 1.0 / 24.0, t, h0, body, starname, flags, geopos, search_atpress, attemp, tret, error);
                    if (g_profile_events_enabled) {
                        g_profile_rise_trans_calls += 1ULL;
                        g_profile_rise_trans_seconds += (jme_profile_events_now_seconds() - search_t0);
                    }
                    return rc;
                }
            }
        }
        alt = alt_next;
    }

    jme_set_error(error, "Event not found in 24h search window");
    if (g_profile_events_enabled) {
        g_profile_rise_trans_calls += 1ULL;
        g_profile_rise_trans_seconds += (jme_profile_events_now_seconds() - search_t0);
    }
    return JME_ERR;
}

static int jme_rise_trans_impl(double jd_ut, int body, const char *starname, int flags, int rsmi, double *geopos, double atpress, double attemp, double *tret, char *error)
{
    return rise_trans_search(jd_ut, body, starname, flags, rsmi, geopos, atpress, attemp, 0, 0.0, tret, error);
}

static int longitude_difference_at(double jd_ut, int body, int flags, int heliocentric, double x2cross, double *value, char *error)
{
    double results[6];

    if (value == 0) {
        jme_set_error(error, "Longitude crossing work value is required");
        return JME_ERR;
    }

    if (internal_calc_ut_positions(jd_ut, body, flags | (heliocentric ? JME_CALC_HELIOCENTRIC : 0), results, error) != JME_OK) {
        return JME_ERR;
    }

    *value = jme_degrees_difference_signed(results[0], x2cross);
    return JME_OK;
}

static int refine_longitude_crossing(double t0, double t1, int body, int flags, int heliocentric, double x2cross, double *tret, char *error)
{
    double v0;
    double v1;
    int i;

    if (longitude_difference_at(t0, body, flags, heliocentric, x2cross, &v0, error) != JME_OK
        || longitude_difference_at(t1, body, flags, heliocentric, x2cross, &v1, error) != JME_OK
        || v0 * v1 > 0.0) {
        return JME_ERR;
    }

    for (i = 0; i < 48; i++) {
        double tm = (t0 + t1) / 2.0;
        double vm;

        if (longitude_difference_at(tm, body, flags, heliocentric, x2cross, &vm, error) != JME_OK) {
            return JME_ERR;
        }
        if (fabs(vm) < 1e-10 || fabs(t1 - t0) < 1e-10) {
            *tret = tm;
            return JME_OK;
        }
        if (v0 * vm <= 0.0) {
            t1 = tm;
            v1 = vm;
        } else {
            t0 = tm;
            v0 = vm;
        }
    }

    (void)v1;
    *tret = (t0 + t1) / 2.0;
    return JME_OK;
}

static int solar_bessel_classify(double jd_ut, int *rc_out, int *class_out, double *lon, double *lat, double *distance_km, char *error);

static int jme_solcross_impl(double x2cross, double jd_ut, int flags, double *tret, char *error)
{
    double t = jd_ut;
    double lon, lon_next;
    double results[6];
    int i;

    if (tret == 0) {
        jme_set_error(error, "Solar crossing output is required");
        return JME_ERR;
    }

    x2cross = jme_degree_normalize(x2cross);
    for (i = 0; i < 365 * 2; i++) {
        if (internal_calc_ut_positions(t, JME_BODY_SUN, flags, results, error) != JME_OK) {
            return JME_ERR;
        }
        lon = results[0];
        t += 0.5;
        if (internal_calc_ut_positions(t, JME_BODY_SUN, flags, results, error) != JME_OK) {
            return JME_ERR;
        }
        lon_next = results[0];

        if (jme_degrees_difference_signed(lon_next, x2cross) >= 0 && jme_degrees_difference_signed(lon, x2cross) < 0) {
            return refine_longitude_crossing(t - 0.5, t, JME_BODY_SUN, flags, 0, x2cross, tret, error);
        }
    }

    jme_set_error(error, "Solar crossing not found in search window");
    return JME_ERR;
}

static int jme_mooncross_impl(double x2cross, double jd_ut, int flags, double *tret, char *error)
{
    double t = jd_ut;
    double lon, lon_next;
    double results[6];
    int i;

    if (tret == 0) {
        jme_set_error(error, "Lunar crossing output is required");
        return JME_ERR;
    }

    x2cross = jme_degree_normalize(x2cross);
    for (i = 0; i < 30 * 2; i++) {
        if (internal_calc_ut_positions(t, JME_BODY_MOON, flags, results, error) != JME_OK) {
            return JME_ERR;
        }
        lon = results[0];
        t += 0.5;
        if (internal_calc_ut_positions(t, JME_BODY_MOON, flags, results, error) != JME_OK) {
            return JME_ERR;
        }
        lon_next = results[0];

        if (jme_degrees_difference_signed(lon_next, x2cross) >= 0 && jme_degrees_difference_signed(lon, x2cross) < 0) {
            return refine_longitude_crossing(t - 0.5, t, JME_BODY_MOON, flags, 0, x2cross, tret, error);
        }
    }

    jme_set_error(error, "Lunar crossing not found in search window");
    return JME_ERR;
}

/*
 * Next (or previous) solar eclipse visible from geopos (lon, lat, height m).
 * Successive new moons are examined: lunations without a global eclipse are
 * skipped, then the local maximum (minimum topocentric separation) and the
 * local contacts are computed. An eclipse counts as visible if the Sun's
 * upper limb is above the apparent horizon (-0.8333 deg, standard refraction
 * plus semi-diameter) at some instant between first and last contact.
 *   tret[0] local maximum; tret[1] 0; tret[2]/tret[3] first/last contact;
 *   tret[4]/tret[5] second/third contact (total or annular phase), else 0.
 *   attr[0] magnitude, attr[1] obscuration, attr[2] separation (deg),
 *   attr[3]/attr[4] solar/lunar apparent diameters (arcsec), attr[5]/attr[6]
 *   Sun/Moon true altitude at maximum (deg), attr[7] 1 if the central phase
 *   is seen, attr[8] JME_ECLIPSE_VISIBLE, attr[9] 1 if the maximum itself
 *   is visible (Sun above the horizon at maximum), else 0.
 */
static int solar_local_sun_visible(double t, double *geopos, char *error)
{
    double alt = get_altitude(t, JME_BODY_SUN, 0, 0, geopos, 0.0, 0.0, error);
    return isfinite(alt) && alt > JME_SOLAR_VISIBLE_UPPER_LIMB_ALTITUDE_DEG;
}

static int jme_sol_eclipse_when_loc_impl(double jd_start, int flags, double *geopos, double *tret, double *attr, int backward, char *error)
{
    double guess = 0.0;
    double search_jd = jd_start;
    double direction = backward ? -1.0 : 1.0;
    int attempt;
    (void)flags;

    if (geopos == 0 || tret == 0 || attr == 0) {
        jme_set_error(error, "Solar eclipse local search requires geopos, tret, and attr outputs");
        return JME_ERR;
    }
    if (!isfinite(jd_start) || !isfinite(geopos[0]) || !isfinite(geopos[1]) || !isfinite(geopos[2])
        || geopos[1] < -90.0 || geopos[1] > 90.0) {
        jme_set_error(error, "Solar eclipse local search inputs are invalid");
        return JME_ERR;
    }

    for (attempt = 0; attempt < 3000; attempt++) {
        double gmax = 0.0, tmax = 0.0, sep, sun_r, moon_r, sun_alt, moon_alt;
        double c1 = 0.0, c4 = 0.0;
        int grc = 0, rc, visible, i;

        if (find_new_moon_conjunction(search_jd, backward, &guess, error) != JME_OK) {
            return JME_ERR;
        }
        search_jd = guess + direction;
        if (refine_solar_maximum(guess, 0, 0, &gmax, error) != JME_OK
            || solar_bessel_classify(gmax, &grc, 0, 0, 0, 0, error) != JME_OK) {
            return JME_ERR;
        }
        if (grc == 0) continue;
        if (refine_solar_maximum(guess, geopos, 1, &tmax, error) != JME_OK
            || solar_eclipse_local_geometry(tmax, geopos, &sep, &sun_r, &moon_r, &sun_alt, &moon_alt, error) != JME_OK) {
            return JME_ERR;
        }
        rc = classify_solar_eclipse(sep, sun_r, moon_r);
        if (rc == 0) continue;
        if (find_solar_contact_time(tmax, geopos, 0, 1, &c1, error) != JME_OK
            || find_solar_contact_time(tmax, geopos, 0, 0, &c4, error) != JME_OK) {
            return JME_ERR;
        }
        visible = solar_local_sun_visible(tmax, geopos, error)
            || solar_local_sun_visible(c1, geopos, error)
            || solar_local_sun_visible(c4, geopos, error);
        if (!visible) {
            /* sunrise or sunset strictly inside the eclipse interval */
            double t;
            for (t = c1; t < c4 && !visible; t += 1.0 / 1440.0) {
                visible = solar_local_sun_visible(t, geopos, error);
            }
        }
        if (!visible) continue;

        memset(tret, 0, sizeof(double) * 10);
        for (i = 0; i < 20; i++) attr[i] = 0.0;
        tret[0] = tmax;
        tret[2] = c1;
        tret[3] = c4;
        if (rc == JME_ECLIPSE_SOLAR_TOTAL || rc == JME_ECLIPSE_SOLAR_ANNULAR) {
            if (find_solar_contact_time(tmax, geopos, 1, 1, &tret[4], error) != JME_OK
                || find_solar_contact_time(tmax, geopos, 1, 0, &tret[5], error) != JME_OK) {
                return JME_ERR;
            }
        }
        attr[0] = (sun_r + moon_r - sep) / (2.0 * sun_r);
        attr[1] = solar_obscuration(sep, sun_r, moon_r);
        attr[2] = sep;
        attr[3] = 2.0 * sun_r * 3600.0;
        attr[4] = 2.0 * moon_r * 3600.0;
        attr[5] = sun_alt;
        attr[6] = moon_alt;
        attr[7] = sep <= fabs(sun_r - moon_r) ? 1.0 : 0.0;
        attr[8] = JME_ECLIPSE_VISIBLE;
        attr[9] = solar_local_sun_visible(tmax, geopos, error) ? 1.0 : 0.0;
        if (error != 0) error[0] = '\0';
        return rc;
    }

    jme_set_error(error, "No local solar eclipse found in search window");
    return JME_ERR;
}

/* ------------------------------------------------------------------------
 * Global solar-eclipse geometry on the Besselian fundamental plane.
 *
 * Construction (Explanatory Supplement to the Astronomical Almanac, 1992,
 * ch. 8; Meeus, Elements of Solar Eclipses 1951-2200, 1989): the z axis is
 * parallel to the shadow axis (Moon -> Sun direction), the fundamental plane
 * passes through the Earth's centre, unit = Earth equatorial radius. Sun and
 * Moon are apparent geocentric places on the true equator and equinox of
 * date; Earth-fixed coordinates follow from Greenwich apparent sidereal time
 * (polar motion not applied). The Earth is the WGS84 ellipsoid, treated
 * exactly in 3-D (no first-order ellipticity approximation):
 *   - the shadow axis meets the surface where the line Moon + s(-k)
 *     intersects the ellipsoid (quadratic in s);
 *   - penumbral/umbral contacts with the Earth are tangencies of the shadow
 *     cones with the ellipsoid's silhouette (the curve where the surface
 *     normal is perpendicular to the axis), evaluated with the cone radius
 *     at each silhouette point's own height zeta above the plane.
 * Residual: tangency is taken on the silhouette for the axis direction, not
 * for each cone generator (tilted by the cone half-angle, about 0.27 deg);
 * the effect on P1/P4/U1/U4 is below 0.2 s.
 * ---------------------------------------------------------------------- */
#define JME_BESSEL_EARTH_A_KM 6378.137
#define JME_BESSEL_EARTH_F (1.0 / 298.257223563)

typedef struct solar_bessel {
    double moon[3];   /* Moon, geocentric true-equator-of-date, Earth radii */
    double k[3];      /* unit vector Moon -> Sun (shadow axis, towards Sun) */
    double xhat[3], yhat[3];
    double x, y, z;   /* Moon in the fundamental frame, Earth radii */
    double tanf1, tanf2;
    double l1, l2;    /* penumbral/umbral radius on the plane; l2 < 0 total */
    double gast;      /* Greenwich apparent sidereal time, rad */
} solar_bessel;

/*
 * Fundamental-plane geometry for the Moon and a target (the Sun for solar
 * eclipses; a planet or star for lunar occultations). The shadow cones are
 * tangent to the Moon and the target: sin f1 = (R_t + R_m)/G,
 * sin f2 = (R_t - R_m)/G, G = |target - Moon|; the radii on the plane are
 * l1 = z tan f1 + k / cos f1 and l2 = z tan f2 - k / cos f2 (k = R_m in
 * Earth radii), which equal the classical (z + k/sin f1) tan f1 and
 * (z - k/sin f2) tan f2 and remain valid as f -> 0: a star without parallax
 * (at infinity) has a cylindrical shadow of radius k along its direction.
 */
static int bessel_at(double jd_ut, const events_target *tg, solar_bessel *b, char *error)
{
    double tgt[3], moon[3], g[3], gn, sinf1, sinf2, cosf1, cosf2, kk, d, a, rt;
    int i, known = 1;

    if (events_target_position(jd_ut, tg, JME_CALC_EQUATORIAL | JME_EVENTS_GEOMETRY_FLAGS, tgt, &known, error) != JME_OK
        || internal_calc_position_ut(jd_ut, JME_BODY_MOON, JME_CALC_EQUATORIAL | JME_EVENTS_GEOMETRY_FLAGS, moon, error) != JME_OK) {
        return JME_ERR;
    }
    for (i = 0; i < 3; i++) b->moon[i] = moon[i] * JME_AU_KM / JME_BESSEL_EARTH_A_KM;
    if (known) {
        for (i = 0; i < 3; i++) g[i] = tgt[i] * JME_AU_KM / JME_BESSEL_EARTH_A_KM - b->moon[i];
    } else {
        for (i = 0; i < 3; i++) g[i] = tgt[i];
    }
    gn = vec_norm(g);
    if (!isfinite(gn) || gn <= 0.0) {
        jme_set_error(error, "Shadow geometry requires a valid Moon-target vector");
        return JME_ERR;
    }
    for (i = 0; i < 3; i++) b->k[i] = g[i] / gn;
    d = asin(b->k[2]);
    a = atan2(b->k[1], b->k[0]);
    b->xhat[0] = -sin(a); b->xhat[1] = cos(a); b->xhat[2] = 0.0;
    b->yhat[0] = -sin(d) * cos(a); b->yhat[1] = -sin(d) * sin(a); b->yhat[2] = cos(d);
    b->x = vec_dot(b->moon, b->xhat);
    b->y = vec_dot(b->moon, b->yhat);
    b->z = vec_dot(b->moon, b->k);
    rt = events_target_radius_km(tg);
    if (known) {
        sinf1 = (rt + JME_EVENTS_MOON_RADIUS_KM) / (gn * JME_BESSEL_EARTH_A_KM);
        sinf2 = (rt - JME_EVENTS_MOON_RADIUS_KM) / (gn * JME_BESSEL_EARTH_A_KM);
    } else {
        sinf1 = 0.0;
        sinf2 = 0.0;
    }
    cosf1 = sqrt((1.0 - sinf1) * (1.0 + sinf1));
    cosf2 = sqrt((1.0 - sinf2) * (1.0 + sinf2));
    b->tanf1 = sinf1 / cosf1;
    b->tanf2 = sinf2 / cosf2;
    kk = JME_EVENTS_MOON_RADIUS_KM / JME_BESSEL_EARTH_A_KM;
    b->l1 = b->z * b->tanf1 + kk / cosf1;
    b->l2 = b->z * b->tanf2 - kk / cosf2;
    b->gast = events_gst_hours(jd_ut, 0) * 15.0 * JME_EVENTS_DEG_TO_RAD;
    if (!isfinite(b->gast)) {
        jme_set_error(error, "Sidereal time unavailable for shadow geometry");
        return JME_ERR;
    }
    return JME_OK;
}

/* Intersection of the shadow axis with the ellipsoid nearest the Moon.
 * Returns 1 and the point (Earth radii, true equator of date) if the axis
 * meets the Earth; 0 otherwise. *disc receives the scaled discriminant
 * (>= 0 iff the axis meets the ellipsoid). */
static int solar_bessel_axis_point(const solar_bessel *b, double *point, double *disc)
{
    double bb = 1.0 - JME_BESSEL_EARTH_F;
    double w = 1.0 / (bb * bb);
    double u[3] = {-b->k[0], -b->k[1], -b->k[2]};
    double qa = u[0] * u[0] + u[1] * u[1] + w * u[2] * u[2];
    double qb = 2.0 * (b->moon[0] * u[0] + b->moon[1] * u[1] + w * b->moon[2] * u[2]);
    double qc = b->moon[0] * b->moon[0] + b->moon[1] * b->moon[1] + w * b->moon[2] * b->moon[2] - 1.0;
    double dsc = qb * qb - 4.0 * qa * qc;
    double s;
    int i;

    if (disc != 0) *disc = dsc / (4.0 * qa);
    if (dsc < 0.0) return 0;
    /* nearer root, written to avoid cancellation */
    s = (2.0 * qc) / (-qb + sqrt(dsc));
    if (point != 0) for (i = 0; i < 3; i++) point[i] = b->moon[i] + s * u[i];
    return 1;
}

static void solar_bessel_geodetic(const solar_bessel *b, const double *p, double *lon_deg, double *lat_deg)
{
    double bb = 1.0 - JME_BESSEL_EARTH_F;
    double rho = sqrt(p[0] * p[0] + p[1] * p[1]);
    /* surface point: geodetic latitude from the normal (x, y, z / b^2) */
    *lat_deg = atan2(p[2], bb * bb * rho) * JME_EVENTS_RAD_TO_DEG;
    *lon_deg = normalize_lon_180((atan2(p[1], p[0]) - b->gast) * JME_EVENTS_RAD_TO_DEG);
}

/* Point of the silhouette at parameter t. */
static void solar_bessel_silhouette_point(const double *e1, const double *e2, double t, double *p)
{
    double bb = 1.0 - JME_BESSEL_EARTH_F;
    double c = cos(t), s = sin(t);
    p[0] = c * e1[0] + s * e2[0];
    p[1] = c * e1[1] + s * e2[1];
    p[2] = bb * (c * e1[2] + s * e2[2]);
}

/* kind 0: penumbra F = dist - L1'; kind 1: umbra F = dist - |L2'|;
 * kind 2: plain distance from the axis. */
static double solar_bessel_silhouette_f(const solar_bessel *b, const double *p, int kind)
{
    double xi = vec_dot(p, b->xhat) - b->x;
    double eta = vec_dot(p, b->yhat) - b->y;
    double zeta = vec_dot(p, b->k);
    double dist = sqrt(xi * xi + eta * eta);
    if (kind == 0) return dist - (b->l1 - zeta * b->tanf1);
    if (kind == 1) return dist - fabs(b->l2 - zeta * b->tanf2);
    return dist;
}

static double solar_bessel_silhouette_min(const solar_bessel *b, int kind, double *best_point)
{
    double bb = 1.0 - JME_BESSEL_EARTH_F;
    double kp[3] = {b->k[0], b->k[1], b->k[2] / bb};
    double e1[3], e2[3], n, p[3];
    double best_t = 0.0, best_f = INFINITY, lo, hi, gr = 0.6180339887498948482;
    double x1, x2, f1, f2;
    int i;
    const int samples = 720;

    n = vec_norm(kp);
    for (i = 0; i < 3; i++) kp[i] /= n;
    /* e1 = normalize(kp x z), or kp x x when kp is polar */
    if (fabs(kp[2]) < 0.9) {
        e1[0] = kp[1]; e1[1] = -kp[0]; e1[2] = 0.0;
    } else {
        e1[0] = 0.0; e1[1] = kp[2]; e1[2] = -kp[1];
    }
    n = vec_norm(e1);
    for (i = 0; i < 3; i++) e1[i] /= n;
    e2[0] = kp[1] * e1[2] - kp[2] * e1[1];
    e2[1] = kp[2] * e1[0] - kp[0] * e1[2];
    e2[2] = kp[0] * e1[1] - kp[1] * e1[0];

    for (i = 0; i < samples; i++) {
        double t = 2.0 * JME_EVENTS_PI * i / samples;
        double f;
        solar_bessel_silhouette_point(e1, e2, t, p);
        f = solar_bessel_silhouette_f(b, p, kind);
        if (f < best_f) { best_f = f; best_t = t; }
    }
    lo = best_t - 2.0 * JME_EVENTS_PI / samples;
    hi = best_t + 2.0 * JME_EVENTS_PI / samples;
    x1 = hi - gr * (hi - lo);
    x2 = lo + gr * (hi - lo);
    solar_bessel_silhouette_point(e1, e2, x1, p); f1 = solar_bessel_silhouette_f(b, p, kind);
    solar_bessel_silhouette_point(e1, e2, x2, p); f2 = solar_bessel_silhouette_f(b, p, kind);
    for (i = 0; i < 200 && (hi - lo) > 1e-14; i++) {
        if (f1 > f2) {
            lo = x1; x1 = x2; f1 = f2; x2 = lo + gr * (hi - lo);
            solar_bessel_silhouette_point(e1, e2, x2, p); f2 = solar_bessel_silhouette_f(b, p, kind);
        } else {
            hi = x2; x2 = x1; f2 = f1; x1 = hi - gr * (hi - lo);
            solar_bessel_silhouette_point(e1, e2, x1, p); f1 = solar_bessel_silhouette_f(b, p, kind);
        }
    }
    best_t = 0.5 * (lo + hi);
    solar_bessel_silhouette_point(e1, e2, best_t, p);
    if (best_point != 0) for (i = 0; i < 3; i++) best_point[i] = p[i];
    return solar_bessel_silhouette_f(b, p, kind);
}

/* Signed contact functions: negative while the condition holds.
 * kind 0: penumbra on Earth; 1: umbra/antumbra on Earth; 2: axis on Earth;
 * 3: L2' at the central point (sign changes = hybrid transitions). */
static double bessel_condition(double jd_ut, const events_target *tg, int kind, char *error)
{
    solar_bessel b;
    double disc, p[3];
    int central;

    if (bessel_at(jd_ut, tg, &b, error) != JME_OK) return NAN;
    central = solar_bessel_axis_point(&b, p, &disc);
    if (kind == 2) return -disc;
    if (kind == 3) {
        if (!central) return NAN;
        return b.l2 - vec_dot(p, b.k) * b.tanf2;
    }
    if (central) return -1.0;
    return solar_bessel_silhouette_min(&b, kind, 0);
}

static double solar_bessel_condition(double jd_ut, int kind, char *error)
{
    return bessel_condition(jd_ut, 0, kind, error);
}

/* Find the instant, on one side of t0 (where condition(t0) < 0), at which
 * the condition becomes >= 0. Illinois refinement to 1e-10 d (8.6 us). */
typedef struct bessel_ctx { const events_target *tg; int kind; } bessel_ctx;
static double bessel_root_fn(double t, const void *ctx, char *error)
{
    const bessel_ctx *c = (const bessel_ctx *)ctx;
    return bessel_condition(t, c->tg, c->kind, error);
}

static int bessel_contact(const events_target *tg, double t0, int kind, int left_side, double max_span, double *tret, char *error)
{
    double step = 0.005, prev = t0, f;
    int i;

    f = bessel_condition(t0, tg, kind, error);
    if (!isfinite(f) || f >= 0.0) return JME_ERR;
    for (i = 1; i * step <= max_span; i++) {
        double t = t0 + (left_side ? -1.0 : 1.0) * step * i;
        double fprev = f;
        f = bessel_condition(t, tg, kind, error);
        if (!isfinite(f)) return JME_ERR;
        if (f >= 0.0) {
            bessel_ctx c;
            c.tg = tg;
            c.kind = kind;
            return events_refine_root(bessel_root_fn, &c, prev, fprev, t, f, tret, error);
        }
        prev = t;
    }
    return JME_ERR;
}

static int solar_bessel_contact(double t0, int kind, int left_side, double max_span, double *tret, char *error)
{
    return bessel_contact(0, t0, kind, left_side, max_span, tret, error);
}

/* Root of L2'(central point) between a and b (sign change assumed). */
typedef struct solar_hybrid_ctx { double sign; } solar_hybrid_ctx;
static double solar_hybrid_root_fn(double t, const void *ctx, char *error)
{
    return ((const solar_hybrid_ctx *)ctx)->sign * solar_bessel_condition(t, 3, error);
}

static int solar_bessel_hybrid_transition(double a, double b, double *tret, char *error)
{
    double fa = solar_bessel_condition(a, 3, error);
    double fb = solar_bessel_condition(b, 3, error);
    solar_hybrid_ctx c;
    if (!isfinite(fa) || !isfinite(fb) || (fa < 0.0) == (fb < 0.0)) return JME_ERR;
    c.sign = fa < 0.0 ? 1.0 : -1.0; /* make f(a) negative */
    return events_refine_root(solar_hybrid_root_fn, &c, a, c.sign * fa, b, c.sign * fb, tret, error);
}

/* Instantaneous global classification and representative surface point.
 * rc: TOTAL/ANNULAR (central or not), PARTIAL or 0. class: CENTRAL,
 * NONCENTRAL or 0. distance_km: distance of the axis from the surface
 * silhouette (0 when central). */
static int bessel_classify(double jd_ut, const events_target *tg, int *rc_out, int *class_out, double *lon, double *lat, double *distance_km, char *error)
{
    solar_bessel b;
    double p[3], disc, f;
    int rc = 0, cls = 0;

    if (bessel_at(jd_ut, tg, &b, error) != JME_OK) return JME_ERR;
    if (solar_bessel_axis_point(&b, p, &disc)) {
        double l2p = b.l2 - vec_dot(p, b.k) * b.tanf2;
        rc = l2p < 0.0 ? JME_ECLIPSE_SOLAR_TOTAL : JME_ECLIPSE_SOLAR_ANNULAR;
        cls = JME_ECLIPSE_SOLAR_CENTRAL;
        if (distance_km != 0) *distance_km = 0.0;
    } else {
        double q[3];
        double dmin = solar_bessel_silhouette_min(&b, 2, q);
        if (distance_km != 0) *distance_km = dmin * JME_BESSEL_EARTH_A_KM;
        f = solar_bessel_silhouette_min(&b, 1, p);
        if (f < 0.0) {
            double l2p = b.l2 - vec_dot(p, b.k) * b.tanf2;
            rc = l2p < 0.0 ? JME_ECLIPSE_SOLAR_TOTAL : JME_ECLIPSE_SOLAR_ANNULAR;
            cls = JME_ECLIPSE_SOLAR_NONCENTRAL;
        } else if (solar_bessel_silhouette_min(&b, 0, p) < 0.0) {
            rc = JME_ECLIPSE_SOLAR_PARTIAL;
        }
        /* representative point: limb point nearest the axis */
        p[0] = q[0]; p[1] = q[1]; p[2] = q[2];
    }
    if (rc != 0 && lon != 0 && lat != 0) solar_bessel_geodetic(&b, p, lon, lat);
    if (rc_out != 0) *rc_out = rc;
    if (class_out != 0) *class_out = cls;
    return JME_OK;
}

static int solar_bessel_classify(double jd_ut, int *rc_out, int *class_out, double *lon, double *lat, double *distance_km, char *error)
{
    return bessel_classify(jd_ut, 0, rc_out, class_out, lon, lat, distance_km, error);
}

static int jme_sol_eclipse_where_impl(double jd_ut, int flags, double *geopos, double *attr, char *error)
{
    double sep = NAN, sun_r = NAN, moon_r = NAN;
    double sun_alt = NAN, moon_alt = NAN;
    double lon = 0.0, lat = 0.0, distance_km = NAN;
    int shadow_class = 0;
    int rc = 0;
    int i;
    (void)flags;

    if (geopos == 0 || attr == 0) {
        jme_set_error(error, "Solar eclipse geographic circumstance output is required");
        return JME_ERR;
    }
    if (!isfinite(jd_ut)) {
        jme_set_error(error, "Julian date must be finite");
        return JME_ERR;
    }
    if (solar_bessel_classify(jd_ut, &rc, &shadow_class, &lon, &lat, &distance_km, error) != JME_OK) {
        return JME_ERR;
    }
    if (rc == 0) {
        jme_set_error(error, "No solar eclipse is in progress at the supplied time");
        return JME_ERR;
    }

    /* Central point (axis on the surface) for central eclipses; otherwise
     * the point of the Earth's limb nearest the shadow axis. */
    geopos[0] = lon;
    geopos[1] = lat;
    geopos[2] = 0.0;

    if (solar_eclipse_local_geometry(jd_ut, geopos, &sep, &sun_r, &moon_r, &sun_alt, &moon_alt, error) != JME_OK) {
        return JME_ERR;
    }
    for (i = 0; i < 20; i++) attr[i] = 0.0;
    /* attr[0] magnitude: fraction of the solar diameter covered (Moon/Sun
     * diameter ratio for a central total/annular point);
     * attr[1] obscuration: fraction of the solar disc area covered. */
    attr[0] = (sun_r + moon_r - sep) / (2.0 * sun_r);
    if (attr[0] < 0.0) attr[0] = 0.0;
    attr[1] = solar_obscuration(sep, sun_r, moon_r);
    attr[2] = sep;
    attr[3] = 2.0 * sun_r * 3600.0;
    attr[4] = 2.0 * moon_r * 3600.0;
    attr[5] = sun_alt;
    attr[6] = moon_alt;
    attr[7] = isfinite(distance_km) ? distance_km : 0.0;
    attr[8] = (double)shadow_class;
    /* attr[9]: ratio of apparent lunar to solar diameter (the magnitude
     * quoted for central eclipses by the NASA catalogues). */
    attr[9] = moon_r / sun_r;
    return rc;
}

static int solar_type_filter_valid(int filter)
{
    return filter == 0 || filter == JME_ECLIPSE_SOLAR_TOTAL || filter == JME_ECLIPSE_SOLAR_ANNULAR
        || filter == JME_ECLIPSE_SOLAR_HYBRID || filter == JME_ECLIPSE_SOLAR_PARTIAL
        || filter == JME_ECLIPSE_SOLAR_CENTRAL || filter == JME_ECLIPSE_SOLAR_NONCENTRAL;
}

static int solar_type_filter_match(int filter, int rc, int cls)
{
    if (filter == 0) return 1;
    if (filter == JME_ECLIPSE_SOLAR_CENTRAL) return cls == JME_ECLIPSE_SOLAR_CENTRAL;
    if (filter == JME_ECLIPSE_SOLAR_NONCENTRAL) return cls == JME_ECLIPSE_SOLAR_NONCENTRAL;
    return rc == filter;
}

/*
 * Next (or previous) solar eclipse anywhere on Earth.
 *   epheflag: eclipse-type filter, 0 = any, or one of JME_ECLIPSE_SOLAR_TOTAL,
 *             _ANNULAR, _HYBRID, _PARTIAL, _CENTRAL, _NONCENTRAL; any other
 *             value is an error.
 *   tret[0] greatest eclipse (minimum distance of the shadow axis from the
 *           Earth's centre), UT;
 *   tret[1] same instant (kept for layout compatibility);
 *   tret[2] / tret[3] first / last contact of the penumbra with the Earth
 *           (P1 / P4);
 *   tret[4] / tret[5] first / last contact of the umbra or antumbra with
 *           the Earth (U1 / U4), 0 for partial eclipses;
 *   tret[6] / tret[7] central line begins / ends (axis meets the surface),
 *           0 for non-central eclipses;
 *   tret[8] / tret[9] for hybrid eclipses, the instants at which the
 *           central phase changes between annular and total (first and
 *           second change), 0 otherwise.
 * Return value: JME_ECLIPSE_SOLAR_TOTAL / _ANNULAR / _HYBRID / _PARTIAL.
 */
static int jme_sol_eclipse_when_glob_impl(double jd_start, int flags, int epheflag, double *tret, int backward, char *error)
{
    double guess = 0.0;
    double search_jd = jd_start;
    double direction = backward ? -1.0 : 1.0;
    double tmax = 0.0;
    int rc, cls;
    int attempt;
    (void)flags;

    if (tret == 0) {
        jme_set_error(error, "Solar eclipse time output is required");
        return JME_ERR;
    }
    if (!isfinite(jd_start)) {
        jme_set_error(error, "Julian date must be finite");
        return JME_ERR;
    }
    if (!solar_type_filter_valid(epheflag)) {
        jme_set_error(error, "Unknown solar eclipse type filter");
        return JME_ERR;
    }

    /* At most 12 eclipses of any type per 30 lunations; a filtered search
     * (e.g. hybrid) may need many years. */
    for (attempt = 0; attempt < 2000; attempt++) {
        if (find_new_moon_conjunction(search_jd, backward, &guess, error) != JME_OK) {
            return JME_ERR;
        }
        if (refine_solar_maximum(guess, 0, 0, &tmax, error) != JME_OK) {
            return JME_ERR;
        }
        if (solar_bessel_classify(tmax, &rc, &cls, 0, 0, 0, error) != JME_OK) {
            return JME_ERR;
        }
        if (rc != 0) {
            double c1 = 0.0, c2 = 0.0;
            memset(tret, 0, sizeof(double) * 10);
            tret[0] = tmax;
            tret[1] = tmax;
            if (cls == JME_ECLIPSE_SOLAR_CENTRAL) {
                double s0 = solar_bessel_condition(tmax, 3, error);
                if (solar_bessel_contact(tmax, 2, 1, 0.4, &c1, error) != JME_OK
                    || solar_bessel_contact(tmax, 2, 0, 0.4, &c2, error) != JME_OK) {
                    return JME_ERR;
                }
                tret[6] = c1;
                tret[7] = c2;
                {
                    /* the type at the ends of the central line: L2' just
                     * inside C1 / C2 */
                    double eps_t = 1e-7;
                    double sa = solar_bessel_condition(c1 + eps_t, 3, error);
                    double sb = solar_bessel_condition(c2 - eps_t, 3, error);
                    if (!isfinite(s0) || !isfinite(sa) || !isfinite(sb)) return JME_ERR;
                    if ((sa < 0.0) != (s0 < 0.0) || (sb < 0.0) != (s0 < 0.0)) {
                        rc = JME_ECLIPSE_SOLAR_HYBRID;
                        if ((sa < 0.0) != (s0 < 0.0)
                            && solar_bessel_hybrid_transition(c1 + eps_t, tmax, &tret[8], error) != JME_OK) {
                            return JME_ERR;
                        }
                        if ((sb < 0.0) != (s0 < 0.0)
                            && solar_bessel_hybrid_transition(tmax, c2 - eps_t, &tret[9], error) != JME_OK) {
                            return JME_ERR;
                        }
                        if (tret[8] == 0.0) { tret[8] = tret[9]; tret[9] = 0.0; }
                    }
                }
            }
            if (!solar_type_filter_match(epheflag, rc, cls)) {
                search_jd = guess + direction;
                continue;
            }
            if (solar_bessel_contact(tmax, 0, 1, 0.5, &tret[2], error) != JME_OK
                || solar_bessel_contact(tmax, 0, 0, 0.5, &tret[3], error) != JME_OK) {
                return JME_ERR;
            }
            if (rc != JME_ECLIPSE_SOLAR_PARTIAL) {
                if (solar_bessel_contact(tmax, 1, 1, 0.5, &tret[4], error) != JME_OK
                    || solar_bessel_contact(tmax, 1, 0, 0.5, &tret[5], error) != JME_OK) {
                    return JME_ERR;
                }
            }
            if (error != 0) error[0] = '\0';
            return rc;
        }
        search_jd = guess + direction;
    }

    jme_set_error(error, "No solar eclipse found in search window");
    return JME_ERR;
}

static int jme_lun_eclipse_when_impl(double jd_start, int flags, int iflag, double *tret, int backward, char *error)
{
    double guess = 0.0;
    double search_jd = jd_start;
    double direction = backward ? -1.0 : 1.0;
    double tmax = 0.0;
    double umbral_mag = 0.0;
    double penumbral_mag = 0.0;
    double umbra_km = 0.0;
    double penumbra_km = 0.0;
    double offset_km = 0.0;
    double axis_km = 0.0;
    int eclipse_type;
    int attempt;
    (void)flags;

    if (tret == 0) {
        jme_set_error(error, "Lunar eclipse time output is required");
        return JME_ERR;
    }
    if (!isfinite(jd_start)) {
        jme_set_error(error, "Julian date must be finite");
        return JME_ERR;
    }
    /* iflag: eclipse-type filter, 0 = any, or JME_ECLIPSE_LUNAR_TOTAL /
     * _PARTIAL / _PENUMBRAL; other values are errors. */
    if (iflag != 0 && iflag != JME_ECLIPSE_LUNAR_TOTAL && iflag != JME_ECLIPSE_LUNAR_PARTIAL
        && iflag != JME_ECLIPSE_LUNAR_PENUMBRAL) {
        jme_set_error(error, "Unknown lunar eclipse type filter");
        return JME_ERR;
    }

    for (attempt = 0; attempt < 2000; attempt++) {
        if (find_full_moon_opposition(search_jd, backward, &guess, error) != JME_OK) {
            return JME_ERR;
        }
        if (refine_lunar_eclipse_maximum(guess, &tmax, error) != JME_OK) {
            return JME_ERR;
        }
        if (lunar_eclipse_geometry(tmax, &umbral_mag, &penumbral_mag, &umbra_km, &penumbra_km, &offset_km, &axis_km, error) != JME_OK) {
            return JME_ERR;
        }

        eclipse_type = classify_lunar_eclipse(umbral_mag, penumbral_mag);
        if (eclipse_type != 0 && axis_km > 0.0 && (iflag == 0 || iflag == eclipse_type)) {
            memset(tret, 0, sizeof(double) * 10);
            tret[0] = tmax;
            tret[1] = tmax;

            if (penumbral_mag > 0.0) {
                find_contact_time(tmax, 0, 1, &tret[2], error);
                find_contact_time(tmax, 0, 0, &tret[3], error);
            }
            if (umbral_mag > 0.0) {
                find_contact_time(tmax, 1, 1, &tret[4], error);
                find_contact_time(tmax, 1, 0, &tret[5], error);
            }
            if (umbral_mag >= 1.0) {
                find_contact_time(tmax, 2, 1, &tret[6], error);
                find_contact_time(tmax, 2, 0, &tret[7], error);
            }

            if (error != 0) error[0] = '\0';
            return eclipse_type;
        }

        search_jd = guess + direction;
    }

    jme_set_error(error, "No lunar eclipse found in search window");
    return JME_ERR;
}

/* ------------------------------------------------------------------------
 * Lunar occultations of planets and stars.
 *
 * Global circumstances use the fundamental-plane (Besselian) construction
 * of the solar-eclipse code with the target in place of the Sun
 * (Explanatory Supplement to the Astronomical Almanac, 1992, ch. 8 and
 * 9.4): shadow cones tangent to the Moon and the target (a cylinder for a
 * star at infinity), WGS84 ellipsoid treated exactly in 3-D. Local
 * circumstances use complete topocentric apparent places (the observer's
 * own light time, deflection and aberration) with the angular radii
 * re-evaluated at each trial instant. Radii: Moon 1737.4 km (IAU mean
 * radius; the IOTA/IAU k = 0.2725076 Earth radii), planets IAU WGCCRE
 * mean equatorial radii (event_body_radius_km), stars 0 (point sources).
 * ---------------------------------------------------------------------- */

/* angle between two vectors, accurate at all separations */
static double events_vector_angle_deg(const double *a, const double *b)
{
    double c[3];
    vec_cross(a, b, c);
    return atan2(vec_norm(c), vec_dot(a, b)) * JME_EVENTS_RAD_TO_DEG;
}

/* Geocentric conjunction in apparent ecliptic longitude of the Moon and the
 * target (used only as the starting point of each search). Only genuine
 * crossings are accepted (both bracket ends within 90 deg), so the +-180 deg
 * wrap of the difference at opposition is never taken for a root. */
typedef struct occ_conj_ctx { const events_target *tg; double sign; } occ_conj_ctx;

static double occ_conj_fn(double t, const void *c, char *error)
{
    const occ_conj_ctx *x = (const occ_conj_ctx *)c;
    double moon[3], tgt[3], d;
    if (internal_calc_position_ut(t, JME_BODY_MOON, 0, moon, error) != JME_OK
        || events_target_position(t, x->tg, 0, tgt, 0, error) != JME_OK) {
        return NAN;
    }
    d = (atan2(moon[1], moon[0]) - atan2(tgt[1], tgt[0])) * JME_EVENTS_RAD_TO_DEG;
    return x->sign * jme_degrees_difference_signed(jme_degree_normalize(d), 0.0);
}

static int find_moon_target_conjunction(double jd_start, const events_target *tg, int backward, double *tret, char *error)
{
    double direction = backward ? -1.0 : 1.0;
    double t0 = jd_start, f0, t1, f1;
    occ_conj_ctx c;
    int i;

    c.tg = tg;
    c.sign = 1.0;
    f0 = occ_conj_fn(t0, &c, error);
    if (!isfinite(f0)) return JME_ERR;
    for (i = 1; i <= 120; i++) {
        t1 = jd_start + direction * (0.5 * i);
        f1 = occ_conj_fn(t1, &c, error);
        if (!isfinite(f1)) return JME_ERR;
        if (fabs(f0) < 90.0 && fabs(f1) < 90.0 && ((f0 < 0.0) != (f1 < 0.0) || f1 == 0.0)) {
            if (f1 == 0.0) { *tret = t1; return JME_OK; }
            if (f0 < 0.0) return events_refine_root(occ_conj_fn, &c, t0, f0, t1, f1, tret, error);
            c.sign = -1.0;
            return events_refine_root(occ_conj_fn, &c, t1, -f1, t0, -f0, tret, error);
        }
        t0 = t1;
        f0 = f1;
    }
    jme_set_error(error, "Moon-target conjunction not found in search window");
    return JME_ERR;
}

/* Geocentric separation (deg) and Moon/target angular radii (deg). */
static int occ_geocentric(double jd_ut, const events_target *tg, double *sep, double *moon_r, double *target_r, char *error)
{
    double moon[3], tgt[3];
    int known = 1;
    if (internal_calc_position_ut(jd_ut, JME_BODY_MOON, JME_CALC_EQUATORIAL | JME_EVENTS_GEOMETRY_FLAGS, moon, error) != JME_OK
        || events_target_position(jd_ut, tg, JME_CALC_EQUATORIAL | JME_EVENTS_GEOMETRY_FLAGS, tgt, &known, error) != JME_OK) {
        return JME_ERR;
    }
    if (sep != 0) *sep = events_vector_angle_deg(moon, tgt);
    if (moon_r != 0) *moon_r = angular_radius_deg(JME_EVENTS_MOON_RADIUS_KM, vec_norm(moon) * JME_AU_KM);
    if (target_r != 0) {
        double rt = events_target_radius_km(tg);
        *target_r = (rt > 0.0 && known) ? angular_radius_deg(rt, vec_norm(tgt) * JME_AU_KM) : 0.0;
    }
    return JME_OK;
}

/* Global maximum: instant at which the shadow axis passes closest to the
 * Earth's centre (minimum gamma), as for solar eclipses. */
static double occ_global_metric(double t, const void *c, char *error)
{
    const events_target *tg = (const events_target *)c;
    solar_bessel b;
    if (bessel_at(t, tg, &b, error) != JME_OK) return NAN;
    return hypot(b.x, b.y);
}

/* Local geometry: complete topocentric apparent places. */
typedef struct occ_local { double sep, moon_r, target_r; } occ_local;

static int occ_local_geometry(double jd_ut, const events_target *tg, double *geopos, occ_local *g, char *error)
{
    double moon[3], tgt[3];
    int known = 1;
    internal_topo_override_set(geopos[0], geopos[1], geopos[2]);
    if (internal_calc_position_ut(jd_ut, JME_BODY_MOON, JME_CALC_EQUATORIAL | JME_CALC_TOPOCENTRIC | JME_EVENTS_GEOMETRY_FLAGS, moon, error) != JME_OK
        || events_target_position(jd_ut, tg, JME_CALC_EQUATORIAL | JME_CALC_TOPOCENTRIC | JME_EVENTS_GEOMETRY_FLAGS, tgt, &known, error) != JME_OK) {
        return JME_ERR;
    }
    g->sep = events_vector_angle_deg(moon, tgt);
    g->moon_r = angular_radius_deg(JME_EVENTS_MOON_RADIUS_KM, vec_norm(moon) * JME_AU_KM);
    {
        double rt = events_target_radius_km(tg);
        g->target_r = (rt > 0.0 && known) ? angular_radius_deg(rt, vec_norm(tgt) * JME_AU_KM) : 0.0;
    }
    return JME_OK;
}

/* kind 0: local separation (minimised); 1: outer contact function
 * sep - (r_m + r_t); 2: inner contact function sep - |r_m - r_t|. */
typedef struct occ_local_ctx { const events_target *tg; double *geopos; int kind; } occ_local_ctx;

static double occ_local_fn(double t, const void *c, char *error)
{
    const occ_local_ctx *x = (const occ_local_ctx *)c;
    occ_local g;
    if (occ_local_geometry(t, x->tg, x->geopos, &g, error) != JME_OK) return NAN;
    if (x->kind == 1) return g.sep - (g.moon_r + g.target_r);
    if (x->kind == 2) return g.sep - fabs(g.moon_r - g.target_r);
    return g.sep;
}

static int occ_local_contact(const events_target *tg, double *geopos, double tmax, int kind, int left_side, double *tret, char *error)
{
    occ_local_ctx c;
    double step = 0.005, prev = tmax, f, fprev;
    int i;

    c.tg = tg;
    c.geopos = geopos;
    c.kind = kind;
    f = occ_local_fn(tmax, &c, error);
    if (!isfinite(f) || f >= 0.0) return JME_ERR;
    for (i = 1; i * step <= 0.5; i++) {
        double t = tmax + (left_side ? -1.0 : 1.0) * step * i;
        fprev = f;
        f = occ_local_fn(t, &c, error);
        if (!isfinite(f)) return JME_ERR;
        if (f >= 0.0) return events_refine_root(occ_local_fn, &c, prev, fprev, t, f, tret, error);
        prev = t;
    }
    jme_set_error(error, "Occultation contact not found");
    return JME_ERR;
}

/* Geometric altitude (deg) of the target's centre for the observer. */
typedef struct occ_alt_ctx { const events_target *tg; double *geopos; double sign; } occ_alt_ctx;

static double occ_alt_fn(double t, const void *c, char *error)
{
    const occ_alt_ctx *x = (const occ_alt_ctx *)c;
    double a = get_altitude(t, events_target_is_star(x->tg) ? 0 : x->tg->body,
        events_target_is_star(x->tg) ? x->tg->star : 0, 0, x->geopos, 0.0, 0.0, error);
    return x->sign * a;
}

/* Fraction of the target's disc area covered by the Moon (stars: 0 or 1). */
static double occ_obscuration(double sep, double rt, double rm)
{
    if (rt <= 0.0) return sep < rm ? 1.0 : 0.0;
    return solar_obscuration(sep, rt, rm);
}

static void occ_fill_attr(const occ_local *g, double moon_alt, double target_alt, double *attr)
{
    int i;
    for (i = 0; i < 20; i++) attr[i] = 0.0;
    /* magnitude: fraction of the target's diameter covered (stars: 1 when
     * occulted, 0 otherwise) */
    if (g->target_r > 0.0) {
        attr[0] = (g->moon_r + g->target_r - g->sep) / (2.0 * g->target_r);
        if (attr[0] < 0.0) attr[0] = 0.0;
        if (attr[0] > 1.0) attr[0] = 1.0;
    } else {
        attr[0] = g->sep < g->moon_r ? 1.0 : 0.0;
    }
    attr[1] = occ_obscuration(g->sep, g->target_r, g->moon_r);
    attr[2] = g->sep;
    attr[3] = 2.0 * g->moon_r * 3600.0;
    attr[4] = 2.0 * g->target_r * 3600.0;
    attr[5] = moon_alt;
    attr[6] = target_alt;
    attr[7] = g->sep <= g->moon_r - g->target_r ? 1.0 : 0.0;
}

static int occ_target_valid(int body, const char *starname, events_target *tg, char *error)
{
    tg->body = body;
    tg->star = starname;
    if (!events_target_is_star(tg) && (body == JME_BODY_MOON || body == JME_BODY_EARTH)) {
        jme_set_error(error, "Lunar occultation target must be a planet, the Sun or a star");
        return JME_ERR;
    }
    return JME_OK;
}

/* Conservative pre-filter (never rejects an occultation): at the
 * longitude conjunction the geocentric separation is within a factor
 * cos(10 deg) of its minimum (relative-path inclination to the ecliptic
 * < 7 deg), and an occultation anywhere on the Earth needs a geocentric
 * separation below asin((R_earth + R_moon)/d_moon) + r_target < 1.32 deg
 * + r_target. */
static int occ_conjunction_may_occult(double guess, const events_target *tg, int *may, char *error)
{
    double sep, rm, rt;
    if (occ_geocentric(guess, tg, &sep, &rm, &rt, error) != JME_OK) return JME_ERR;
    *may = sep * 0.98480775301220805936674302458952 <= 1.32 + rt + 0.05;
    return JME_OK;
}

static int jme_lun_occult_where_impl(double jd_ut, int body, const char *starname, int flags, double *geopos, double *attr, char *error)
{
    events_target tg;
    occ_local g;
    double lon = 0.0, lat = 0.0, sep, rm, rt;
    int rc = 0, cls = 0;
    (void)flags;

    if (geopos == 0 || attr == 0) {
        jme_set_error(error, "Lunar occultation geographic circumstance output is required");
        return JME_ERR;
    }
    if (!isfinite(jd_ut)) {
        jme_set_error(error, "Julian date must be finite");
        return JME_ERR;
    }
    if (occ_target_valid(body, starname, &tg, error) != JME_OK) return JME_ERR;
    if (bessel_classify(jd_ut, &tg, &rc, &cls, &lon, &lat, 0, error) != JME_OK) return JME_ERR;
    if (rc == 0) {
        jme_set_error(error, "No lunar occultation is in progress at the supplied time");
        return JME_ERR;
    }
    geopos[0] = lon;
    geopos[1] = lat;
    geopos[2] = 0.0;
    if (occ_local_geometry(jd_ut, &tg, geopos, &g, error) != JME_OK
        || occ_geocentric(jd_ut, &tg, &sep, &rm, &rt, error) != JME_OK) {
        return JME_ERR;
    }
    occ_fill_attr(&g,
        get_altitude(jd_ut, JME_BODY_MOON, 0, 0, geopos, 0.0, 0.0, error),
        get_altitude(jd_ut, events_target_is_star(&tg) ? 0 : body, events_target_is_star(&tg) ? starname : 0, 0, geopos, 0.0, 0.0, error),
        attr);
    attr[8] = cls == JME_ECLIPSE_SOLAR_CENTRAL ? 1.0 : 0.0;
    attr[9] = lunar_horizontal_parallax_deg(rm);
    if (error != 0) error[0] = '\0';
    return JME_OK;
}

static int jme_lun_occult_when_loc_impl(double jd_start, int body, const char *starname, int flags, double *geopos, double *tret, double *attr, int backward, char *error)
{
    events_target tg;
    double search_jd = jd_start;
    double direction = backward ? -1.0 : 1.0;
    int attempt;
    (void)flags;

    if (geopos == 0 || tret == 0 || attr == 0) {
        jme_set_error(error, "Lunar occultation local search requires geopos, tret, and attr outputs");
        return JME_ERR;
    }
    if (!isfinite(jd_start) || !isfinite(geopos[0]) || !isfinite(geopos[1]) || !isfinite(geopos[2])
        || geopos[1] < -90.0 || geopos[1] > 90.0) {
        jme_set_error(error, "Lunar occultation local search inputs are invalid");
        return JME_ERR;
    }
    if (occ_target_valid(body, starname, &tg, error) != JME_OK) return JME_ERR;

    /* about 13.4 conjunctions per year: 2000 covers ~150 years */
    for (attempt = 0; attempt < 2000; attempt++) {
        double guess = 0.0, gmax = 0.0, tmax = 0.0, c1 = 0.0, c4 = 0.0;
        double moon_alt, target_alt;
        int may = 0, grc = 0, visible;
        occ_local g;
        occ_local_ctx lc;
        occ_alt_ctx ac;

        if (find_moon_target_conjunction(search_jd, &tg, backward, &guess, error) != JME_OK) return JME_ERR;
        search_jd = guess + direction;
        if (occ_conjunction_may_occult(guess, &tg, &may, error) != JME_OK) return JME_ERR;
        if (!may) continue;
        if (events_minimize_precise(occ_global_metric, &tg, guess - 0.75, guess + 0.75, &gmax, error) != JME_OK
            || bessel_classify(gmax, &tg, &grc, 0, 0, 0, 0, error) != JME_OK) {
            return JME_ERR;
        }
        if (grc == 0) continue;
        lc.tg = &tg;
        lc.geopos = geopos;
        lc.kind = 0;
        if (events_minimize_precise(occ_local_fn, &lc, gmax - 0.3, gmax + 0.3, &tmax, error) != JME_OK
            || occ_local_geometry(tmax, &tg, geopos, &g, error) != JME_OK) {
            return JME_ERR;
        }
        if (!(g.sep < g.moon_r + g.target_r)) continue;
        if (occ_local_contact(&tg, geopos, tmax, 1, 1, &c1, error) != JME_OK
            || occ_local_contact(&tg, geopos, tmax, 1, 0, &c4, error) != JME_OK) {
            return JME_ERR;
        }
        /* observable if the target is above the horizon at some instant of
         * [c1, c4]: the altitude is smooth on this short interval, so its
         * maximum (Brent) decides exactly. */
        ac.tg = &tg;
        ac.geopos = geopos;
        ac.sign = 1.0;
        visible = occ_alt_fn(c1, &ac, error) > 0.0 || occ_alt_fn(c4, &ac, error) > 0.0 || occ_alt_fn(tmax, &ac, error) > 0.0;
        if (!visible) {
            double tbest;
            ac.sign = -1.0;
            if (events_minimize(occ_alt_fn, &ac, c1, c4, 1e-7, &tbest, error) != JME_OK) return JME_ERR;
            ac.sign = 1.0;
            visible = occ_alt_fn(tbest, &ac, error) > 0.0;
        }
        if (!visible) continue;

        memset(tret, 0, sizeof(double) * 10);
        tret[0] = tmax;
        tret[1] = tmax;
        tret[2] = c1;
        tret[3] = c4;
        if (g.sep < fabs(g.moon_r - g.target_r) || g.target_r == 0.0) {
            if (g.target_r == 0.0) {
                tret[4] = c1;
                tret[5] = c4;
            } else if (occ_local_contact(&tg, geopos, tmax, 2, 1, &tret[4], error) != JME_OK
                || occ_local_contact(&tg, geopos, tmax, 2, 0, &tret[5], error) != JME_OK) {
                return JME_ERR;
            }
        }
        moon_alt = get_altitude(tmax, JME_BODY_MOON, 0, 0, geopos, 0.0, 0.0, error);
        target_alt = occ_alt_fn(tmax, &ac, error);
        occ_fill_attr(&g, moon_alt, target_alt, attr);
        attr[8] = JME_ECLIPSE_VISIBLE;
        attr[9] = target_alt > 0.0 ? 1.0 : 0.0;
        if (error != 0) error[0] = '\0';
        return JME_OK;
    }
    jme_set_error(error, "No local lunar occultation found in search window");
    return JME_ERR;
}

static int jme_lun_occult_when_glob_impl(double jd_start, int body, const char *starname, int flags, int iflag, double *tret, int backward, char *error)
{
    events_target tg;
    double search_jd = jd_start;
    double direction = backward ? -1.0 : 1.0;
    int attempt;
    (void)flags;
    (void)iflag;

    if (tret == 0) {
        jme_set_error(error, "Lunar occultation time output is required");
        return JME_ERR;
    }
    if (!isfinite(jd_start)) {
        jme_set_error(error, "Julian date must be finite");
        return JME_ERR;
    }
    if (occ_target_valid(body, starname, &tg, error) != JME_OK) return JME_ERR;

    for (attempt = 0; attempt < 2000; attempt++) {
        double guess = 0.0, tmax = 0.0;
        int may = 0, rc = 0, cls = 0;

        if (find_moon_target_conjunction(search_jd, &tg, backward, &guess, error) != JME_OK) return JME_ERR;
        search_jd = guess + direction;
        if (occ_conjunction_may_occult(guess, &tg, &may, error) != JME_OK) return JME_ERR;
        if (!may) continue;
        if (events_minimize_precise(occ_global_metric, &tg, guess - 0.75, guess + 0.75, &tmax, error) != JME_OK
            || bessel_classify(tmax, &tg, &rc, &cls, 0, 0, 0, error) != JME_OK) {
            return JME_ERR;
        }
        if (rc == 0) continue;
        memset(tret, 0, sizeof(double) * 10);
        tret[0] = tmax;
        tret[1] = tmax;
        if (bessel_contact(&tg, tmax, 0, 1, 0.5, &tret[2], error) != JME_OK
            || bessel_contact(&tg, tmax, 0, 0, 0.5, &tret[3], error) != JME_OK) {
            return JME_ERR;
        }
        if (rc == JME_ECLIPSE_SOLAR_TOTAL
            && (bessel_contact(&tg, tmax, 1, 1, 0.5, &tret[4], error) != JME_OK
                || bessel_contact(&tg, tmax, 1, 0, 0.5, &tret[5], error) != JME_OK)) {
            return JME_ERR;
        }
        if (cls == JME_ECLIPSE_SOLAR_CENTRAL
            && (bessel_contact(&tg, tmax, 2, 1, 0.4, &tret[6], error) != JME_OK
                || bessel_contact(&tg, tmax, 2, 0, 0.4, &tret[7], error) != JME_OK)) {
            return JME_ERR;
        }
        if (error != 0) error[0] = '\0';
        return JME_OK;
    }
    jme_set_error(error, "No lunar occultation found in search window");
    return JME_ERR;
}

static int jme_rise_trans_true_hor_impl(double jd_ut, int body, const char *starname, int flags, int rsmi, double *geopos, double atpress, double attemp, double horhgt, double *tret, char *error)
{
    return rise_trans_search(jd_ut, body, starname, flags, rsmi, geopos, atpress, attemp, 1, horhgt, tret, error);
}

static int jme_solcross_ut_impl(double x2cross, double jd_ut, int flags, double *tret, char *error)
{
    return jme_solcross(x2cross, jd_ut, flags, tret, error);
}

static int jme_mooncross_ut_impl(double x2cross, double jd_ut, int flags, double *tret, char *error)
{
    return jme_mooncross(x2cross, jd_ut, flags, tret, error);
}

static int jme_mooncross_node_impl(double jd_ut, int flags, double *tret, char *error)
{
    double t = jd_ut;
    double pos[6];
    double lat;
    double next_lat;
    int i;

    if (tret == 0) {
        jme_set_error(error, "Lunar node crossing output is required");
        return JME_ERR;
    }

    if (internal_calc_ut_positions(t, JME_BODY_MOON, flags, pos, error) != JME_OK) {
        return JME_ERR;
    }
    lat = pos[1];

    for (i = 0; i < 60; i++) {
        t += 0.25;
        if (internal_calc_ut_positions(t, JME_BODY_MOON, flags, pos, error) != JME_OK) {
            return JME_ERR;
        }
        next_lat = pos[1];
        if ((lat <= 0.0 && next_lat >= 0.0) || (lat >= 0.0 && next_lat <= 0.0)) {
            double t0 = t - 0.25;
            double t1 = t;
            double l0 = lat;
            int j;

            for (j = 0; j < 48; j++) {
                double tm = (t0 + t1) / 2.0;
                double lm;
                if (internal_calc_ut_positions(tm, JME_BODY_MOON, flags, pos, error) != JME_OK) {
                    return JME_ERR;
                }
                lm = pos[1];
                if (fabs(lm) < 1e-10 || fabs(t1 - t0) < 1e-10) {
                    *tret = tm;
                    return JME_OK;
                }
                if ((l0 <= 0.0 && lm >= 0.0) || (l0 >= 0.0 && lm <= 0.0)) {
                    t1 = tm;
                } else {
                    t0 = tm;
                    l0 = lm;
                }
            }
            *tret = (t0 + t1) / 2.0;
            return JME_OK;
        }
        lat = next_lat;
    }

    jme_set_error(error, "Lunar node crossing not found in search window");
    return JME_ERR;
}

static int jme_mooncross_node_ut_impl(double jd_ut, int flags, double *tret, char *error)
{
    return jme_mooncross_node(jd_ut, flags, tret, error);
}

static int jme_helio_cross_impl(int body, double x2cross, double jd_ut, int flags, double *tret, char *error)
{
    double t = jd_ut;
    double pos[6];
    double lon;
    double lon_next;
    int i;

    if (tret == 0) {
        jme_set_error(error, "Heliocentric crossing output is required");
        return JME_ERR;
    }

    x2cross = jme_degree_normalize(x2cross);
    for (i = 0; i < 365 * 20; i++) {
        if (internal_calc_ut_positions(t, body, flags | JME_CALC_HELIOCENTRIC, pos, error) != JME_OK) {
            return JME_ERR;
        }
        lon = pos[0];
        t += 0.5;
        if (internal_calc_ut_positions(t, body, flags | JME_CALC_HELIOCENTRIC, pos, error) != JME_OK) {
            return JME_ERR;
        }
        lon_next = pos[0];
        if (jme_degrees_difference_signed(lon_next, x2cross) >= 0.0 && jme_degrees_difference_signed(lon, x2cross) < 0.0) {
            return refine_longitude_crossing(t - 0.5, t, body, flags, 1, x2cross, tret, error);
        }
    }

    jme_set_error(error, "Heliocentric crossing not found in search window");
    return JME_ERR;
}

static int jme_helio_cross_ut_impl(int body, double x2cross, double jd_ut, int flags, double *tret, char *error)
{
    return jme_helio_cross(body, x2cross, jd_ut, flags, tret, error);
}

static int jme_nod_aps_ut_impl(double jd_ut, int body, int flags, int method, double *tret, char *error)
{
    double jd_et;
    if (tret == 0) {
        jme_set_error(error, "Nodal/apsidal event output is required");
        return JME_ERR;
    }
    *tret = NAN;
    if (internal_ut_to_et(jd_ut, &jd_et, error) != JME_OK) return JME_ERR;
    return jme_nod_aps(jd_et, body, flags, method, tret, error);
}

static int jme_heliacal_pheno_ut_impl(double jd_ut, double *geopos, double *dat_hel, char *error)
{
    int body;
    double sun_alt;
    double body_alt;
    double sun_eq[6];
    double body_eq[6];
    double pheno[20];
    double arcus;
    double elongation;
    double limiting_mag;
    double required_arcus;
    int i;

    if (geopos == 0 || dat_hel == 0) {
        jme_set_error(error, "Heliacal phenomena require geopos and output data");
        return JME_ERR;
    }

    if (heliacal_body_from_data(dat_hel, &body, error) != JME_OK) {
        return JME_ERR;
    }

    sun_alt = get_altitude(jd_ut, JME_BODY_SUN, 0, 0, geopos, 0.0, 0.0, error);
    body_alt = get_altitude(jd_ut, body, 0, 0, geopos, 0.0, 0.0, error);
    if (sun_alt <= -99.0 || body_alt <= -99.0) {
        return JME_ERR;
    }

    if (internal_calc_ut_positions(jd_ut, JME_BODY_SUN, JME_CALC_EQUATORIAL, sun_eq, error) != JME_OK
        || internal_calc_ut_positions(jd_ut, body, JME_CALC_EQUATORIAL, body_eq, error) != JME_OK
        || jme_pheno_ut(jd_ut, body, 0, pheno, error) != JME_OK) {
        return JME_ERR;
    }

    arcus = body_alt - sun_alt;
    elongation = jme_spherical_angular_separation(sun_eq[0], sun_eq[1], body_eq[0], body_eq[1]);
    limiting_mag = heliacal_limiting_magnitude(arcus);
    required_arcus = heliacal_required_arcus(pheno[4]);
    if (!isfinite(limiting_mag) || !isfinite(required_arcus)) {
        jme_set_error(error, "Heliacal visibility model produced non-finite thresholds");
        return JME_ERR;
    }

    for (i = 0; i < 20; i++) {
        dat_hel[i] = 0.0;
    }
    dat_hel[0] = jd_ut;
    dat_hel[1] = (double)body;
    dat_hel[2] = sun_alt;
    dat_hel[3] = body_alt;
    dat_hel[4] = arcus;
    dat_hel[5] = limiting_mag;
    dat_hel[6] = pheno[4];
    dat_hel[7] = elongation;
    dat_hel[8] = pheno[3];
    dat_hel[9] = heliacal_visibility_flag(body_alt, sun_alt, arcus, pheno[4], limiting_mag, required_arcus) ? 1.0 : 0.0;
    dat_hel[10] = required_arcus;

    return JME_OK;
}

static double jme_heliacal_angle_impl(double jd_ut, double *geopos, double *dat_hel, char *error)
{
    double local[20] = {0.0};
    double *out = dat_hel != 0 ? dat_hel : local;

    if (dat_hel == 0) {
        local[0] = JME_BODY_VENUS;
    }

    if (jme_heliacal_pheno_ut(jd_ut, geopos, out, error) != JME_OK) {
        return NAN;
    }

    return out[7];
}

static double jme_topo_arcus_visionis_impl(double jd_ut, double *geopos, double *dat_hel, char *error)
{
    double local[20] = {0.0};
    double *out = dat_hel != 0 ? dat_hel : local;

    if (dat_hel == 0) {
        local[0] = JME_BODY_VENUS;
    }

    if (jme_heliacal_pheno_ut(jd_ut, geopos, out, error) != JME_OK) {
        return NAN;
    }

    return out[4];
}

static int jme_lun_eclipse_when_loc_impl(double jd_start, int flags, double *geopos, double *tret, double *attr, int backward, char *error)
{
    int rc;
    double altitude = -90.0;
    double global_tret[10];
    double local_tmax;
    double eclipse_start = 0.0;
    double eclipse_end = 0.0;
    double visible_start = 0.0;
    double visible_end = 0.0;
    int i;

    if (geopos == 0 || tret == 0 || attr == 0) {
        jme_set_error(error, "Lunar eclipse local search requires geopos, tret, and attr outputs");
        return JME_ERR;
    }

    rc = jme_lun_eclipse_when(jd_start, flags, 0, global_tret, backward, error);
    if (rc == JME_ERR) {
        return JME_ERR;
    }

    for (i = 2; i < 8; i += 2) {
        if (global_tret[i] != 0.0 && (eclipse_start == 0.0 || global_tret[i] < eclipse_start)) {
            eclipse_start = global_tret[i];
        }
    }
    for (i = 3; i < 8; i += 2) {
        if (global_tret[i] != 0.0 && global_tret[i] > eclipse_end) {
            eclipse_end = global_tret[i];
        }
    }

    if (eclipse_start == 0.0 || eclipse_end == 0.0 || eclipse_end <= eclipse_start) {
        eclipse_start = global_tret[0] - 0.25;
        eclipse_end = global_tret[0] + 0.25;
    }

    if (find_visible_moon_interval(eclipse_start, eclipse_end, geopos, &visible_start, &visible_end, error) != JME_OK) {
        return JME_ERR;
    }

    local_tmax = global_tret[0];
    if (local_tmax < visible_start) {
        local_tmax = visible_start;
    }
    if (local_tmax > visible_end) {
        local_tmax = visible_end;
    }

    if (jme_lun_eclipse_how(local_tmax, flags, geopos, attr, error) == JME_ERR) {
        return JME_ERR;
    }

    for (i = 0; i < 10; i++) {
        tret[i] = global_tret[i];
    }

    tret[0] = local_tmax;
    tret[1] = global_tret[0];
    for (i = 2; i < 8; i += 2) {
        if (tret[i] != 0.0 && tret[i] < visible_start) {
            tret[i] = visible_start;
        }
    }
    for (i = 3; i < 8; i += 2) {
        if (tret[i] != 0.0 && tret[i] > visible_end) {
            tret[i] = visible_end;
        }
    }
    tret[8] = visible_start;
    tret[9] = visible_end;

    attr[8] = moon_is_above_horizon(local_tmax, geopos, error, &altitude) ? JME_ECLIPSE_VISIBLE : 0.0;
    attr[7] = altitude;
    attr[9] = (visible_end - visible_start) * 24.0;

    return rc;
}

static int jme_sol_eclipse_how_impl(double jd_ut, int flags, double *geopos, double *attr, char *error)
{
    double sep = 0.0;
    double sun_r = 0.0;
    double moon_r = 0.0;
    double sun_alt = -90.0;
    double moon_alt = -90.0;
    int rc;
    int i;
    (void)flags;

    if (geopos == 0 || attr == 0) {
        jme_set_error(error, "Solar eclipse circumstance output requires geopos and attr");
        return JME_ERR;
    }

    if (solar_eclipse_local_geometry(jd_ut, geopos, &sep, &sun_r, &moon_r, &sun_alt, &moon_alt, error) != JME_OK) {
        return JME_ERR;
    }

    rc = classify_solar_eclipse(sep, sun_r, moon_r);
    if (rc == 0 || sun_alt <= JME_SOLAR_VISIBLE_UPPER_LIMB_ALTITUDE_DEG) {
        jme_set_error(error, "No local solar eclipse is in progress at the supplied time");
        return JME_ERR;
    }

    for (i = 0; i < 20; i++) {
        attr[i] = 0.0;
    }

    attr[0] = (sun_r + moon_r - sep) / (2.0 * sun_r);
    attr[1] = solar_obscuration(sep, sun_r, moon_r);
    attr[2] = sep;
    attr[3] = 2.0 * sun_r * 3600.0;
    attr[4] = 2.0 * moon_r * 3600.0;
    attr[5] = sun_alt;
    attr[6] = moon_alt;
    attr[7] = sep <= fabs(sun_r - moon_r) ? 1.0 : 0.0;
    attr[8] = JME_ECLIPSE_VISIBLE;

    return rc;
}

static int jme_lun_eclipse_how_impl(double jd_ut, int flags, double *geopos, double *attr, char *error)
{
    double umbral_mag = 0.0;
    double penumbral_mag = 0.0;
    double umbra_km = 0.0;
    double penumbra_km = 0.0;
    double offset_km = 0.0;
    double axis_km = 0.0;
    double altitude = -90.0;
    int rc;
    int i;
    (void)flags;

    if (attr == 0) {
        jme_set_error(error, "Lunar eclipse circumstance output is required");
        return JME_ERR;
    }

    for (i = 0; i < 20; i++) {
        attr[i] = 0.0;
    }

    if (lunar_eclipse_geometry(jd_ut, &umbral_mag, &penumbral_mag, &umbra_km, &penumbra_km, &offset_km, &axis_km, error) != JME_OK) {
        return JME_ERR;
    }

    rc = classify_lunar_eclipse(umbral_mag, penumbral_mag);
    if (rc == 0 || axis_km <= 0.0) {
        jme_set_error(error, "No lunar eclipse is in progress at the supplied time");
        return JME_ERR;
    }

    attr[0] = umbral_mag;
    attr[1] = penumbral_mag;
    attr[2] = offset_km;
    attr[3] = umbra_km;
    attr[4] = penumbra_km;
    attr[5] = axis_km;

    attr[8] = (geopos != 0 && moon_is_above_horizon(jd_ut, geopos, error, &altitude)) ? JME_ECLIPSE_VISIBLE : 0.0;
    attr[7] = altitude;

    return rc;
}

static int jme_nod_aps_impl(double jd_et, int body, int flags, int method, double *tret, char *error)
{
    double t;
    double mean_node;
    double true_node[6];
    double mean_apogee;
    double elem[20];
    double a;
    double e;
    double omega;
    double peri_lon;
    double node_radius;
    double apsis_radius;
    double true_anomaly_deg;
    double lon;
    int i;

    if (tret == 0) {
        jme_set_error(error, "Node/apside output is required");
        return JME_ERR;
    }

    for (i = 0; i < 4; i++) {
        tret[i] = 0.0;
    }
    if (!isfinite(jd_et)) {
        jme_set_error(error, "Terrestrial-time Julian date must be finite");
        return JME_ERR;
    }

    if (body == JME_BODY_MOON) {
        t = (jd_et - 2451545.0) / 36525.0;
        mean_node = 125.0445479
            - 1934.1362891 * t
            + 0.0020754 * t * t
            + (t * t * t) / 467441.0
            - (t * t * t * t) / 60616000.0;
        mean_apogee = 83.3532465
            + 4069.0137287 * t
            - 0.0103200 * t * t
            - (t * t * t) / 80053.0
            + (t * t * t * t) / 18999000.0;

        if (method & 1) {
            lon = mean_node;
        } else if (method & 2) {
            if (jme_calc(jd_et, JME_BODY_TRUE_NODE, flags | JME_CALC_TRUE_POSITION, true_node, error) != JME_OK) {
                return JME_ERR;
            }
            lon = true_node[0];
        } else if (method & 4) {
            lon = mean_apogee;
        } else if (method & 256) {
            lon = jme_degree_normalize(mean_apogee + 180.0);
        } else {
            lon = mean_node;
        }

        tret[0] = jme_degree_normalize(lon);
        tret[1] = 0.0;
        tret[2] = 1.0;
        tret[3] = jd_et;
        return JME_OK;
    }

    if (body == JME_BODY_SUN || body == JME_BODY_EARTH) {
        jme_set_error(error, "Node/apside calculation requires a non-central orbiting body");
        return JME_ERR;
    }

    if (jme_get_orbital_elements(jd_et, body, flags | JME_CALC_HELIOCENTRIC | JME_CALC_TRUE_POSITION, elem, error) != JME_OK) {
        return JME_ERR;
    }

    a = elem[0];
    e = elem[1];
    omega = elem[4];
    peri_lon = elem[17];

    if (!isfinite(a) || !isfinite(e) || !isfinite(omega) || !isfinite(peri_lon) || a <= 0.0 || e < 0.0 || e >= 1.0) {
        jme_set_error(error, "Node/apside calculation requires a bounded elliptical osculating orbit");
        return JME_ERR;
    }

    if (method & 4) {
        lon = jme_degree_normalize(peri_lon + 180.0);
        apsis_radius = a * (1.0 + e);
    } else if (method & 256) {
        lon = jme_degree_normalize(peri_lon);
        apsis_radius = a * (1.0 - e);
    } else {
        lon = elem[3];
        if (method & 2) {
            lon = jme_degree_normalize(lon);
        }
        true_anomaly_deg = jme_degree_normalize(-omega);
        node_radius = orbital_radius_from_elements(a, e, true_anomaly_deg);
        apsis_radius = isfinite(node_radius) ? node_radius : 0.0;
    }

    tret[0] = jme_degree_normalize(lon);
    tret[1] = apsis_radius;
    tret[2] = 0.0;
    tret[3] = jd_et;

    return JME_OK;
}

static int jme_get_orbital_elements_impl(double jd_et, int body, int flags, double *elem, char *error)
{
    static const double mu = 0.0002959122082855911025; /* Gaussian gravitational constant squared, AU^3/day^2 */
    double state[6];
    double r[3], v[3], h[3], n[3], evec[3];
    double z[3] = {0.0, 0.0, 1.0};
    double rmag, vmag, hmag, nmag, e, energy;
    double inc, node, argp, nu, a;
    double ecc_anomaly = 0.0;
    double mean_anomaly = 0.0;
    double mean_motion = 0.0;
    double period_days = 0.0;
    double perihelion_distance = 0.0;
    double aphelion_distance = 0.0;
    int i;

    if (elem == 0) {
        jme_set_error(error, "Orbital element output is required");
        return JME_ERR;
    }

    for (i = 0; i < 20; i++) {
        elem[i] = 0.0;
    }

    if (body == JME_BODY_SUN || body == JME_BODY_EARTH) {
        jme_set_error(error, "Heliocentric osculating elements require a non-central body");
        return JME_ERR;
    }

    if (jme_calc(jd_et, body, flags | JME_CALC_HELIOCENTRIC | JME_CALC_XYZ | JME_CALC_SPEED | JME_CALC_TRUE_POSITION, state, error) != JME_OK) {
        return JME_ERR;
    }

    for (i = 0; i < 3; i++) {
        r[i] = state[i];
        v[i] = state[i + 3];
    }

    rmag = vec_norm(r);
    vmag = vec_norm(v);
    vec_cross(r, v, h);
    hmag = vec_norm(h);
    vec_cross(z, h, n);
    nmag = vec_norm(n);

    if (!isfinite(rmag) || !isfinite(vmag) || !isfinite(hmag) || rmag <= 0.0 || vmag <= 0.0 || hmag <= 0.0) {
        jme_set_error(error, "Orbital element state lacks a valid position/velocity vector");
        return JME_ERR;
    }

    for (i = 0; i < 3; i++) {
        evec[i] = ((vmag * vmag - mu / rmag) * r[i] - vec_dot(r, v) * v[i]) / mu;
    }
    e = vec_norm(evec);
    energy = 0.5 * vmag * vmag - mu / rmag;
    a = fabs(energy) > 0.0 ? -mu / (2.0 * energy) : 0.0;

    inc = acos(h[2] / hmag) * JME_EVENTS_RAD_TO_DEG;
    node = nmag > 0.0 ? jme_degree_normalize(atan2(n[1], n[0]) * JME_EVENTS_RAD_TO_DEG) : 0.0;

    if (nmag > 0.0 && e > 0.0) {
        double c = vec_dot(n, evec) / (nmag * e);
        if (c > 1.0) { c = 1.0; }
        if (c < -1.0) { c = -1.0; }
        argp = acos(c) * JME_EVENTS_RAD_TO_DEG;
        if (evec[2] < 0.0) { argp = 360.0 - argp; }
    } else {
        argp = 0.0;
    }

    if (e > 0.0) {
        double c = vec_dot(evec, r) / (e * rmag);
        if (c > 1.0) { c = 1.0; }
        if (c < -1.0) { c = -1.0; }
        nu = acos(c) * JME_EVENTS_RAD_TO_DEG;
        if (vec_dot(r, v) < 0.0) { nu = 360.0 - nu; }
    } else {
        nu = 0.0;
    }

    if (e < 1.0 && a > 0.0) {
        ecc_anomaly = 2.0 * atan2(
            sqrt(1.0 - e) * sin(nu * JME_EVENTS_DEG_TO_RAD / 2.0),
            sqrt(1.0 + e) * cos(nu * JME_EVENTS_DEG_TO_RAD / 2.0)
        );
        if (ecc_anomaly < 0.0) {
            ecc_anomaly += 2.0 * JME_EVENTS_PI;
        }
        mean_anomaly = ecc_anomaly - e * sin(ecc_anomaly);
        mean_motion = sqrt(mu / (a * a * a));
        period_days = 2.0 * JME_EVENTS_PI / mean_motion;
        mean_motion *= JME_EVENTS_RAD_TO_DEG;
        mean_anomaly *= JME_EVENTS_RAD_TO_DEG;
        perihelion_distance = a * (1.0 - e);
        aphelion_distance = a * (1.0 + e);
    }

    elem[0] = a;                                        /* semi-major axis, AU */
    elem[1] = e;                                        /* eccentricity */
    elem[2] = inc;                                      /* inclination, deg */
    elem[3] = node;                                     /* ascending node, deg */
    elem[4] = argp;                                     /* argument of perihelion, deg */
    elem[5] = nu;                                       /* true anomaly, deg */
    elem[6] = hmag;                                     /* specific angular momentum, AU^2/day */
    elem[7] = rmag;                                     /* current radius, AU */
    elem[8] = vmag;                                     /* current speed, AU/day */
    elem[9] = ecc_anomaly * JME_EVENTS_RAD_TO_DEG;      /* eccentric anomaly, deg */
    elem[10] = mean_anomaly;                            /* mean anomaly, deg */
    elem[11] = mean_motion;                             /* mean motion, deg/day */
    elem[12] = period_days;                             /* sidereal period, days */
    elem[13] = perihelion_distance;                     /* perihelion distance, AU */
    elem[14] = aphelion_distance;                       /* aphelion distance, AU */
    elem[15] = jme_degree_normalize(argp + nu);         /* argument of latitude, deg */
    elem[16] = jme_degree_normalize(node + argp + nu);  /* true longitude, deg */
    elem[17] = jme_degree_normalize(node + argp);       /* longitude of perihelion, deg */

    return JME_OK;
}

static int jme_heliacal_ut_impl(double jd_ut, double *geopos, double *dat_hel, char *error)
{
    int body;
    double best_t = 0.0;
    double best_data[20] = {0.0};
    int i;

    if (geopos == 0 || dat_hel == 0) {
        jme_set_error(error, "Heliacal visibility requires geopos and output data");
        return JME_ERR;
    }

    if (heliacal_body_from_data(dat_hel, &body, error) != JME_OK) {
        return JME_ERR;
    }

    for (i = 0; i <= 370; i++) {
        double day = jd_ut + (double)i;
        double candidates[2] = {0.0, 0.0};
        double pheno[20];
        double required_arcus;
        int n = 0;
        double t;

        if (jme_pheno_ut(day + 0.5, body, 0, pheno, error) != JME_OK) {
            return JME_ERR;
        }
        required_arcus = heliacal_required_arcus(pheno[4]);
        if (!isfinite(required_arcus)) {
            jme_set_error(error, "Heliacal visibility search requires finite body magnitude");
            return JME_ERR;
        }

        if (jme_rise_trans_true_hor(day, JME_BODY_SUN, 0, 0, JME_RISE_SET, geopos, 0.0, 0.0, -required_arcus, &t, error) == JME_OK) {
            candidates[n++] = t;
        }
        if (jme_rise_trans_true_hor(day, JME_BODY_SUN, 0, 0, JME_RISE_RISE, geopos, 0.0, 0.0, -required_arcus, &t, error) == JME_OK) {
            candidates[n++] = t;
        }

        while (n-- > 0) {
            double tmp[20] = {0.0};
            tmp[0] = (double)body;
            if (candidates[n] < jd_ut) {
                continue;
            }
            if (jme_heliacal_pheno_ut(candidates[n], geopos, tmp, error) != JME_OK) {
                return JME_ERR;
            }
            if (tmp[9] > 0.0 && (best_t == 0.0 || candidates[n] < best_t)) {
                best_t = candidates[n];
                memcpy(best_data, tmp, sizeof(best_data));
            }
        }

        if (best_t != 0.0) {
            memcpy(dat_hel, best_data, sizeof(best_data));
            return JME_OK;
        }
    }

    jme_set_error(error, "No heliacal visibility event found in search window");
    return JME_ERR;
}

static int jme_vis_limit_mag_impl(double jd_ut, double *geopos, double *dat_hel, char *error)
{
    if (dat_hel == 0) {
        jme_set_error(error, "Visual limiting magnitude requires output data");
        return JME_ERR;
    }

    if (jme_heliacal_pheno_ut(jd_ut, geopos, dat_hel, error) != JME_OK) {
        return JME_ERR;
    }

    return JME_OK;
}

/*
 * Public entry points: each call runs with a scoped, thread-local observer
 * (geopos) and restores the caller's state, so these routines never modify
 * the user's jme_set_topo() configuration.
 */
int jme_rise_trans(double jd_ut, int body, const char *starname, int flags, int rsmi, double *geopos, double atpress, double attemp, double *tret, char *error)
{
    double saved_topo[4];
    int result;
    internal_topo_override_save(saved_topo);
    result = jme_rise_trans_impl(jd_ut, body, starname, flags, rsmi, geopos, atpress, attemp, tret, error);
    internal_topo_override_restore(saved_topo);
    return result;
}

int jme_solcross(double x2cross, double jd_ut, int flags, double *tret, char *error)
{
    double saved_topo[4];
    int result;
    internal_topo_override_save(saved_topo);
    result = jme_solcross_impl(x2cross, jd_ut, flags, tret, error);
    internal_topo_override_restore(saved_topo);
    return result;
}

int jme_mooncross(double x2cross, double jd_ut, int flags, double *tret, char *error)
{
    double saved_topo[4];
    int result;
    internal_topo_override_save(saved_topo);
    result = jme_mooncross_impl(x2cross, jd_ut, flags, tret, error);
    internal_topo_override_restore(saved_topo);
    return result;
}

int jme_sol_eclipse_when_loc(double jd_start, int flags, double *geopos, double *tret, double *attr, int backward, char *error)
{
    double saved_topo[4];
    int result;
    internal_topo_override_save(saved_topo);
    result = jme_sol_eclipse_when_loc_impl(jd_start, flags, geopos, tret, attr, backward, error);
    internal_topo_override_restore(saved_topo);
    return result;
}

int jme_sol_eclipse_where(double jd_ut, int flags, double *geopos, double *attr, char *error)
{
    double saved_topo[4];
    int result;
    internal_topo_override_save(saved_topo);
    result = jme_sol_eclipse_where_impl(jd_ut, flags, geopos, attr, error);
    internal_topo_override_restore(saved_topo);
    return result;
}

int jme_sol_eclipse_when_glob(double jd_start, int flags, int epheflag, double *tret, int backward, char *error)
{
    double saved_topo[4];
    int result;
    internal_topo_override_save(saved_topo);
    result = jme_sol_eclipse_when_glob_impl(jd_start, flags, epheflag, tret, backward, error);
    internal_topo_override_restore(saved_topo);
    return result;
}

int jme_lun_eclipse_when(double jd_start, int flags, int iflag, double *tret, int backward, char *error)
{
    double saved_topo[4];
    int result;
    internal_topo_override_save(saved_topo);
    result = jme_lun_eclipse_when_impl(jd_start, flags, iflag, tret, backward, error);
    internal_topo_override_restore(saved_topo);
    return result;
}

int jme_lun_occult_where(double jd_ut, int body, const char *starname, int flags, double *geopos, double *attr, char *error)
{
    double saved_topo[4];
    int result;
    internal_topo_override_save(saved_topo);
    result = jme_lun_occult_where_impl(jd_ut, body, starname, flags, geopos, attr, error);
    internal_topo_override_restore(saved_topo);
    return result;
}

int jme_lun_occult_when_loc(double jd_start, int body, const char *starname, int flags, double *geopos, double *tret, double *attr, int backward, char *error)
{
    double saved_topo[4];
    int result;
    internal_topo_override_save(saved_topo);
    result = jme_lun_occult_when_loc_impl(jd_start, body, starname, flags, geopos, tret, attr, backward, error);
    internal_topo_override_restore(saved_topo);
    return result;
}

int jme_lun_occult_when_glob(double jd_start, int body, const char *starname, int flags, int iflag, double *tret, int backward, char *error)
{
    double saved_topo[4];
    int result;
    internal_topo_override_save(saved_topo);
    result = jme_lun_occult_when_glob_impl(jd_start, body, starname, flags, iflag, tret, backward, error);
    internal_topo_override_restore(saved_topo);
    return result;
}

int jme_rise_trans_true_hor(double jd_ut, int body, const char *starname, int flags, int rsmi, double *geopos, double atpress, double attemp, double horhgt, double *tret, char *error)
{
    double saved_topo[4];
    int result;
    internal_topo_override_save(saved_topo);
    result = jme_rise_trans_true_hor_impl(jd_ut, body, starname, flags, rsmi, geopos, atpress, attemp, horhgt, tret, error);
    internal_topo_override_restore(saved_topo);
    return result;
}

int jme_solcross_ut(double x2cross, double jd_ut, int flags, double *tret, char *error)
{
    double saved_topo[4];
    int result;
    internal_topo_override_save(saved_topo);
    result = jme_solcross_ut_impl(x2cross, jd_ut, flags, tret, error);
    internal_topo_override_restore(saved_topo);
    return result;
}

int jme_mooncross_ut(double x2cross, double jd_ut, int flags, double *tret, char *error)
{
    double saved_topo[4];
    int result;
    internal_topo_override_save(saved_topo);
    result = jme_mooncross_ut_impl(x2cross, jd_ut, flags, tret, error);
    internal_topo_override_restore(saved_topo);
    return result;
}

int jme_mooncross_node(double jd_ut, int flags, double *tret, char *error)
{
    double saved_topo[4];
    int result;
    internal_topo_override_save(saved_topo);
    result = jme_mooncross_node_impl(jd_ut, flags, tret, error);
    internal_topo_override_restore(saved_topo);
    return result;
}

int jme_mooncross_node_ut(double jd_ut, int flags, double *tret, char *error)
{
    double saved_topo[4];
    int result;
    internal_topo_override_save(saved_topo);
    result = jme_mooncross_node_ut_impl(jd_ut, flags, tret, error);
    internal_topo_override_restore(saved_topo);
    return result;
}

int jme_helio_cross(int body, double x2cross, double jd_ut, int flags, double *tret, char *error)
{
    double saved_topo[4];
    int result;
    internal_topo_override_save(saved_topo);
    result = jme_helio_cross_impl(body, x2cross, jd_ut, flags, tret, error);
    internal_topo_override_restore(saved_topo);
    return result;
}

int jme_helio_cross_ut(int body, double x2cross, double jd_ut, int flags, double *tret, char *error)
{
    double saved_topo[4];
    int result;
    internal_topo_override_save(saved_topo);
    result = jme_helio_cross_ut_impl(body, x2cross, jd_ut, flags, tret, error);
    internal_topo_override_restore(saved_topo);
    return result;
}

int jme_nod_aps_ut(double jd_ut, int body, int flags, int method, double *tret, char *error)
{
    double saved_topo[4];
    int result;
    internal_topo_override_save(saved_topo);
    result = jme_nod_aps_ut_impl(jd_ut, body, flags, method, tret, error);
    internal_topo_override_restore(saved_topo);
    return result;
}

int jme_heliacal_pheno_ut(double jd_ut, double *geopos, double *dat_hel, char *error)
{
    double saved_topo[4];
    int result;
    internal_topo_override_save(saved_topo);
    result = jme_heliacal_pheno_ut_impl(jd_ut, geopos, dat_hel, error);
    internal_topo_override_restore(saved_topo);
    return result;
}

double jme_heliacal_angle(double jd_ut, double *geopos, double *dat_hel, char *error)
{
    double saved_topo[4];
    double result;
    internal_topo_override_save(saved_topo);
    result = jme_heliacal_angle_impl(jd_ut, geopos, dat_hel, error);
    internal_topo_override_restore(saved_topo);
    return result;
}

double jme_topo_arcus_visionis(double jd_ut, double *geopos, double *dat_hel, char *error)
{
    double saved_topo[4];
    double result;
    internal_topo_override_save(saved_topo);
    result = jme_topo_arcus_visionis_impl(jd_ut, geopos, dat_hel, error);
    internal_topo_override_restore(saved_topo);
    return result;
}

int jme_lun_eclipse_when_loc(double jd_start, int flags, double *geopos, double *tret, double *attr, int backward, char *error)
{
    double saved_topo[4];
    int result;
    internal_topo_override_save(saved_topo);
    result = jme_lun_eclipse_when_loc_impl(jd_start, flags, geopos, tret, attr, backward, error);
    internal_topo_override_restore(saved_topo);
    return result;
}

int jme_sol_eclipse_how(double jd_ut, int flags, double *geopos, double *attr, char *error)
{
    double saved_topo[4];
    int result;
    internal_topo_override_save(saved_topo);
    result = jme_sol_eclipse_how_impl(jd_ut, flags, geopos, attr, error);
    internal_topo_override_restore(saved_topo);
    return result;
}

int jme_lun_eclipse_how(double jd_ut, int flags, double *geopos, double *attr, char *error)
{
    double saved_topo[4];
    int result;
    internal_topo_override_save(saved_topo);
    result = jme_lun_eclipse_how_impl(jd_ut, flags, geopos, attr, error);
    internal_topo_override_restore(saved_topo);
    return result;
}

int jme_nod_aps(double jd_et, int body, int flags, int method, double *tret, char *error)
{
    double saved_topo[4];
    int result;
    internal_topo_override_save(saved_topo);
    result = jme_nod_aps_impl(jd_et, body, flags, method, tret, error);
    internal_topo_override_restore(saved_topo);
    return result;
}

int jme_get_orbital_elements(double jd_et, int body, int flags, double *elem, char *error)
{
    double saved_topo[4];
    int result;
    internal_topo_override_save(saved_topo);
    result = jme_get_orbital_elements_impl(jd_et, body, flags, elem, error);
    internal_topo_override_restore(saved_topo);
    return result;
}

int jme_heliacal_ut(double jd_ut, double *geopos, double *dat_hel, char *error)
{
    double saved_topo[4];
    int result;
    internal_topo_override_save(saved_topo);
    result = jme_heliacal_ut_impl(jd_ut, geopos, dat_hel, error);
    internal_topo_override_restore(saved_topo);
    return result;
}

int jme_vis_limit_mag(double jd_ut, double *geopos, double *dat_hel, char *error)
{
    double saved_topo[4];
    int result;
    internal_topo_override_save(saved_topo);
    result = jme_vis_limit_mag_impl(jd_ut, geopos, dat_hel, error);
    internal_topo_override_restore(saved_topo);
    return result;
}

