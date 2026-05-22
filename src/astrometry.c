#include "jme/jme.h"
#include "context.h"
#include <math.h>

#define JME_ARCSEC_TO_RAD 4.848136811095359935899141023221522846734e-6
#define JME_DEG_TO_RAD 0.017453292519943295769236907684886127134428718885417

typedef struct jme_nutation_term {
    signed char l;
    signed char lp;
    signed char f;
    signed char d;
    signed char om;
    int psi_a;
    int psi_b;
    int eps_a;
    int eps_b;
} jme_nutation_term;

typedef struct jme_nutation_2000b_term {
    int l;
    int lp;
    int f;
    int d;
    int om;
    double ps;
    double pst;
    double pc;
    double ec;
    double ect;
    double es;
} jme_nutation_2000b_term;

/* IAU 1980 Nutation coefficients. Units: 0.0001 arcsec.
   Source: Seidelmann, 1982 (IAU 1980 Theory of Nutation).
   Fundamental arguments: l, l', F, D, Omega. */
static const jme_nutation_term nut80[] = {
    { 0,  0,  0,  0,  1, -171996, -1742,  92025,   89},
    { 0,  0,  0,  0,  2,    2062,     2,   -895,    5},
    {-2,  0,  2,  0,  1,      46,     0,    -24,    0},
    { 2,  0, -2,  0,  0,      11,     0,      0,    0},
    {-2,  0,  2,  0,  2,      -3,     0,      1,    0},
    { 1, -1,  0, -1,  0,      -3,     0,      0,    0},
    { 0, -2,  2, -2,  1,      -2,     0,      1,    0},
    { 2,  0, -2,  0,  1,       1,     0,      0,    0},
    { 0,  0,  2, -2,  2,  -13187,   -16,   5736,  -31},
    { 0,  1,  0,  0,  0,    1426,   -34,     54,   -1},
    { 0,  1,  2, -2,  2,    -517,    12,    224,   -6},
    { 0, -1,  2, -2,  2,     217,    -5,    -95,    3},
    { 0,  0,  2, -2,  1,     129,     1,    -70,    0},
    { 2,  0,  0, -2,  0,      48,     0,      1,    0},
    { 0,  0,  2, -2,  0,     -22,     0,      0,    0},
    { 0,  2,  0,  0,  0,      17,    -1,      0,    0},
    { 0,  1,  0,  0,  1,     -15,     0,      9,    0},
    { 0,  2,  2, -2,  2,     -16,     1,      7,    0},
    { 0, -1,  0,  0,  1,     -12,     0,      6,    0},
    {-2,  0,  0,  2,  1,      -6,     0,      3,    0},
    { 0, -1,  2, -2,  1,      -5,     0,      3,    0},
    { 2,  0,  0, -2,  1,       4,     0,     -2,    0},
    { 0,  1,  2, -2,  1,       4,     0,     -2,    0},
    { 1,  0,  0, -1,  0,      -4,     0,      0,    0},
    { 2,  1,  0, -2,  0,       1,     0,      0,    0},
    { 0,  0, -2,  2,  1,       1,     0,      0,    0},
    { 0,  1, -2,  2,  0,      -1,     0,      0,    0},
    { 0,  1,  0,  0,  2,       1,     0,      0,    0},
    {-1,  0,  0,  1,  1,       1,     0,      0,    0},
    { 0,  1,  2, -2,  0,      -1,     0,      0,    0},
    { 0,  0,  2,  0,  2,   -2274,    -2,    977,   -5},
    { 1,  0,  0,  0,  0,     712,     1,     -7,    0},
    { 0,  0,  2,  0,  1,    -386,    -4,    200,    0},
    { 1,  0,  2,  0,  2,    -301,     0,    129,   -1},
    { 1,  0,  0, -2,  0,    -158,     0,     -1,    0},
    {-1,  0,  2,  0,  2,     123,     0,    -53,    0},
    { 0,  0,  0,  2,  0,      63,     0,     -2,    0},
    { 1,  0,  0,  0,  1,      63,     1,    -33,    0},
    {-1,  0,  0,  0,  1,     -58,    -1,     32,    0},
    {-1,  0,  2,  2,  2,     -59,     0,     26,    0},
    { 1,  0,  2,  0,  1,     -51,     0,     27,    0},
    { 0,  0,  2,  2,  2,     -38,     0,     16,    0},
    { 2,  0,  0,  0,  0,      29,     0,     -1,    0},
    { 1,  0,  2, -2,  2,      29,     0,    -12,    0},
    { 2,  0,  2,  0,  2,     -31,     0,     13,    0},
    { 0,  0,  2,  0,  0,      26,     0,     -1,    0},
    {-1,  0,  2,  0,  1,      21,     0,    -10,    0},
    {-1,  0,  0,  2,  1,      16,     0,     -8,    0},
    { 1,  0,  0, -2,  1,     -13,     0,      7,    0},
    {-1,  0,  2,  2,  1,     -10,     0,      5,    0},
    { 1,  1,  0,  0,  0,      -7,     0,      0,    0},
    { 0,  1,  2,  0,  2,       7,     0,     -3,    0},
    { 0, -1,  2,  0,  2,      -7,     0,      3,    0},
    { 1,  0,  2,  2,  2,      -8,     0,      3,    0},
    { 1,  0,  0,  2,  0,       6,     0,      0,    0},
    { 2,  0,  2, -2,  2,       6,     0,     -3,    0},
    { 0,  0,  0,  2,  1,      -6,     0,      3,    0},
    { 0,  0,  2,  2,  1,      -6,     0,      3,    0},
    { 1,  0,  2,  0,  0,       5,     0,      0,    0},
    {-1,  0,  0,  0,  2,      -5,     0,      3,    0},
    { 1,  0,  0, -4,  0,      -5,     0,      0,    0},
    {-2,  0,  2,  2,  2,       5,     0,     -2,    0},
    {-1,  0,  2,  4,  2,      -4,     0,      2,    0},
    { 2,  0,  0, -4,  0,       4,     0,      0,    0},
    { 1,  1,  2,  0,  2,      -4,     0,      2,    0},
    { 1,  0,  2, -2,  1,      -4,     0,      2,    0},
    {-2,  0,  2,  4,  2,      -3,     0,      1,    0},
    {-1,  0,  4,  0,  2,      -3,     0,      1,    0},
    { 1, -1,  0,  0,  0,      -3,     0,      0,    0},
    { 1,  0,  2,  2,  1,      -3,     0,      1,    0},
    {-2,  0,  2,  4,  1,      -2,     0,      1,    0},
    {-1,  0,  4,  2,  2,      -2,     0,      1,    0},
    { 1, -1,  2,  0,  2,      -2,     0,      1,    0},
    { 1,  0,  0,  2,  1,      -2,     0,      1,    0},
    { 2,  0,  2,  0,  1,      -2,     0,      1,    0},
    { 0,  0,  4, -2,  2,       2,     0,     -1,    0},
    { 3,  0,  0,  0,  0,       2,     0,      0,    0},
    { 1,  1,  2, -2,  2,      -1,     0,      1,    0},
    { 0,  0,  2,  0, -2,       1,     0,      0,    0},
    {-1, -1,  2,  2,  2,       1,     0,     -1,    0},
    {-1,  0,  2, -2,  1,       1,     0,     -1,    0},
    {-2,  0,  0,  2,  0,       1,     0,      0,    0},
    { 3,  0,  2,  0,  2,      -1,     0,      1,    0},
    { 0,  1,  2,  2,  2,      -1,     0,      1,    0},
    { 1,  1,  0, -2,  0,      -1,     0,      0,    0},
    { 0,  1,  0,  2,  0,       1,     0,      0,    0},
    { 1, -1,  2,  2,  2,      -1,     0,      1,    0},
    {-1,  0,  0,  0,  3,       1,     0,     -1,    0},
    { 2, -1,  0, -2,  0,       1,     0,      0,    0},
    { 1,  0,  0, -2,  2,       1,     0,      0,    0},
    {-1,  0,  2,  2,  0,       1,     0,      0,    0},
    { 0,  0,  2,  4,  2,      -1,     0,      0,    0},
    { 0,  1,  0, -2,  0,       1,     0,      0,    0},
    { 1,  0, -2,  0,  0,       1,     0,      0,    0},
    { 2,  0,  2, -2,  1,       1,     0,     -1,    0},
    { 1,  0,  2,  2,  0,      -1,     0,      0,    0},
    { 1,  0,  0,  2,  2,      -1,     0,      0,    0},
    {-1,  0,  2,  4,  1,      -1,     0,      0,    0},
    { 0,  2,  2, -2,  1,      -1,     0,      0,    0},
    { 1,  0,  2, -2,  0,      -1,     0,      0,    0},
    {-2,  0,  2,  4,  0,       1,     0,      0,    0},
    {-1,  0,  4,  0,  1,       1,     0,      0,    0},
    { 1,  1,  2,  0,  1,      -1,     0,      0,    0},
    { 1,  0,  4,  0,  2,       1,     0,      0,    0},
    {-2,  1,  0,  2,  0,       1,     0,      0,    0},
    { 1, -1,  0, -2,  0,       1,     0,      0,    0}
};

/* IAU 2000B abridged nutation series, MHB_2000_SHORT/Luzum 2001.
   Coefficients are in 0.1 microarcseconds, matching the ERFA/SOFA
   implementation of the IAU 2000B model. */
static const jme_nutation_2000b_term nut00b[] = {
    { 0, 0, 0, 0,1,-172064161.0,-174666.0, 33386.0, 92052331.0, 9086.0, 15377.0},
    { 0, 0, 2,-2,2, -13170906.0,  -1675.0,-13696.0,  5730336.0,-3015.0, -4587.0},
    { 0, 0, 2, 0,2,  -2276413.0,   -234.0,  2796.0,   978459.0, -485.0,  1374.0},
    { 0, 0, 0, 0,2,   2074554.0,    207.0,  -698.0,  -897492.0,  470.0,  -291.0},
    { 0, 1, 0, 0,0,   1475877.0,  -3633.0, 11817.0,    73871.0, -184.0, -1924.0},
    { 0, 1, 2,-2,2,   -516821.0,   1226.0,  -524.0,   224386.0, -677.0,  -174.0},
    { 1, 0, 0, 0,0,    711159.0,     73.0,  -872.0,    -6750.0,    0.0,   358.0},
    { 0, 0, 2, 0,1,   -387298.0,   -367.0,   380.0,   200728.0,   18.0,   318.0},
    { 1, 0, 2, 0,2,   -301461.0,    -36.0,   816.0,   129025.0,  -63.0,   367.0},
    { 0,-1, 2,-2,2,    215829.0,   -494.0,   111.0,   -95929.0,  299.0,   132.0},
    { 0, 0, 2,-2,1,    128227.0,    137.0,   181.0,   -68982.0,   -9.0,    39.0},
    {-1, 0, 2, 0,2,    123457.0,     11.0,    19.0,   -53311.0,   32.0,    -4.0},
    {-1, 0, 0, 2,0,    156994.0,     10.0,  -168.0,    -1235.0,    0.0,    82.0},
    { 1, 0, 0, 0,1,     63110.0,     63.0,    27.0,   -33228.0,    0.0,    -9.0},
    {-1, 0, 0, 0,1,    -57976.0,    -63.0,  -189.0,    31429.0,    0.0,   -75.0},
    {-1, 0, 2, 2,2,    -59641.0,    -11.0,   149.0,    25543.0,  -11.0,    66.0},
    { 1, 0, 2, 0,1,    -51613.0,    -42.0,   129.0,    26366.0,    0.0,    78.0},
    {-2, 0, 2, 0,1,     45893.0,     50.0,    31.0,   -24236.0,  -10.0,    20.0},
    { 0, 0, 0, 2,0,     63384.0,     11.0,  -150.0,    -1220.0,    0.0,    29.0},
    { 0, 0, 2, 2,2,    -38571.0,     -1.0,   158.0,    16452.0,  -11.0,    68.0},
    { 0,-2, 2,-2,2,     32481.0,      0.0,     0.0,   -13870.0,    0.0,     0.0},
    {-2, 0, 0, 2,0,    -47722.0,      0.0,   -18.0,      477.0,    0.0,   -25.0},
    { 2, 0, 2, 0,2,    -31046.0,     -1.0,   131.0,    13238.0,  -11.0,    59.0},
    { 1, 0, 2,-2,2,     28593.0,      0.0,    -1.0,   -12338.0,   10.0,    -3.0},
    {-1, 0, 2, 0,1,     20441.0,     21.0,    10.0,   -10758.0,    0.0,    -3.0},
    { 2, 0, 0, 0,0,     29243.0,      0.0,   -74.0,     -609.0,    0.0,    13.0},
    { 0, 0, 2, 0,0,     25887.0,      0.0,   -66.0,     -550.0,    0.0,    11.0},
    { 0, 1, 0, 0,1,    -14053.0,    -25.0,    79.0,     8551.0,   -2.0,   -45.0},
    {-1, 0, 0, 2,1,     15164.0,     10.0,    11.0,    -8001.0,    0.0,    -1.0},
    { 0, 2, 2,-2,2,    -15794.0,     72.0,   -16.0,     6850.0,  -42.0,    -5.0},
    { 0, 0,-2, 2,0,     21783.0,      0.0,    13.0,     -167.0,    0.0,    13.0},
    { 1, 0, 0,-2,1,    -12873.0,    -10.0,   -37.0,     6953.0,    0.0,   -14.0},
    { 0,-1, 0, 0,1,    -12654.0,     11.0,    63.0,     6415.0,    0.0,    26.0},
    {-1, 0, 2, 2,1,    -10204.0,      0.0,    25.0,     5222.0,    0.0,    15.0},
    { 0, 2, 0, 0,0,     16707.0,    -85.0,   -10.0,      168.0,   -1.0,    10.0},
    { 1, 0, 2, 2,2,     -7691.0,      0.0,    44.0,     3268.0,    0.0,    19.0},
    {-2, 0, 2, 0,0,    -11024.0,      0.0,   -14.0,      104.0,    0.0,     2.0},
    { 0, 1, 2, 0,2,      7566.0,    -21.0,   -11.0,    -3250.0,    0.0,    -5.0},
    { 0, 0, 2, 2,1,     -6637.0,    -11.0,    25.0,     3353.0,    0.0,    14.0},
    { 0,-1, 2, 0,2,     -7141.0,     21.0,     8.0,     3070.0,    0.0,     4.0},
    { 0, 0, 0, 2,1,     -6302.0,    -11.0,     2.0,     3272.0,    0.0,     4.0},
    { 1, 0, 2,-2,1,      5800.0,     10.0,     2.0,    -3045.0,    0.0,    -1.0},
    { 2, 0, 2,-2,2,      6443.0,      0.0,    -7.0,    -2768.0,    0.0,    -4.0},
    {-2, 0, 0, 2,1,     -5774.0,    -11.0,   -15.0,     3041.0,    0.0,    -5.0},
    { 2, 0, 2, 0,1,     -5350.0,      0.0,    21.0,     2695.0,    0.0,    12.0},
    { 0,-1, 2,-2,1,     -4752.0,    -11.0,    -3.0,     2719.0,    0.0,    -3.0},
    { 0, 0, 0,-2,1,     -4940.0,    -11.0,   -21.0,     2720.0,    0.0,    -9.0},
    {-1,-1, 0, 2,0,      7350.0,      0.0,    -8.0,      -51.0,    0.0,     4.0},
    { 2, 0, 0,-2,1,      4065.0,      0.0,     6.0,    -2206.0,    0.0,     1.0},
    { 1, 0, 0, 2,0,      6579.0,      0.0,   -24.0,     -199.0,    0.0,     2.0},
    { 0, 1, 2,-2,1,      3579.0,      0.0,     5.0,    -1900.0,    0.0,     1.0},
    { 1,-1, 0, 0,0,      4725.0,      0.0,    -6.0,      -41.0,    0.0,     3.0},
    {-2, 0, 2, 0,2,     -3075.0,      0.0,    -2.0,     1313.0,    0.0,    -1.0},
    { 3, 0, 2, 0,2,     -2904.0,      0.0,    15.0,     1233.0,    0.0,     7.0},
    { 0,-1, 0, 2,0,      4348.0,      0.0,   -10.0,      -81.0,    0.0,     2.0},
    { 1,-1, 2, 0,2,     -2878.0,      0.0,     8.0,     1232.0,    0.0,     4.0},
    { 0, 0, 0, 1,0,     -4230.0,      0.0,     5.0,      -20.0,    0.0,    -2.0},
    {-1,-1, 2, 2,2,     -2819.0,      0.0,     7.0,     1207.0,    0.0,     3.0},
    {-1, 0, 2, 0,0,     -4056.0,      0.0,     5.0,       40.0,    0.0,    -2.0},
    { 0,-1, 2, 2,2,     -2647.0,      0.0,    11.0,     1129.0,    0.0,     5.0},
    {-2, 0, 0, 0,1,     -2294.0,      0.0,   -10.0,     1266.0,    0.0,    -4.0},
    { 1, 1, 2, 0,2,      2481.0,      0.0,    -7.0,    -1062.0,    0.0,    -3.0},
    { 2, 0, 0, 0,1,      2179.0,      0.0,    -2.0,    -1129.0,    0.0,    -2.0},
    {-1, 1, 0, 1,0,      3276.0,      0.0,     1.0,       -9.0,    0.0,     0.0},
    { 1, 1, 0, 0,0,     -3389.0,      0.0,     5.0,       35.0,    0.0,    -2.0},
    { 1, 0, 2, 0,0,      3339.0,      0.0,   -13.0,     -107.0,    0.0,     1.0},
    {-1, 0, 2,-2,1,     -1987.0,      0.0,    -6.0,     1073.0,    0.0,    -2.0},
    { 1, 0, 0, 0,2,     -1981.0,      0.0,     0.0,      854.0,    0.0,     0.0},
    {-1, 0, 0, 1,0,      4026.0,      0.0,  -353.0,     -553.0,    0.0,  -139.0},
    { 0, 0, 2, 1,2,      1660.0,      0.0,    -5.0,     -710.0,    0.0,    -2.0},
    {-1, 0, 2, 4,2,     -1521.0,      0.0,     9.0,      647.0,    0.0,     4.0},
    {-1, 1, 0, 1,1,      1314.0,      0.0,     0.0,     -700.0,    0.0,     0.0},
    { 0,-2, 2,-2,1,     -1283.0,      0.0,     0.0,      672.0,    0.0,     0.0},
    { 1, 0, 2, 2,1,     -1331.0,      0.0,     8.0,      663.0,    0.0,     4.0},
    {-2, 0, 2, 2,2,      1383.0,      0.0,    -2.0,     -594.0,    0.0,    -2.0},
    {-1, 0, 0, 0,2,      1405.0,      0.0,     4.0,     -610.0,    0.0,     2.0},
    { 1, 1, 2,-2,2,      1290.0,      0.0,     0.0,     -556.0,    0.0,     0.0}
};

static void jme_nutation_args(double t, double *args)
{
    /* Fundamental arguments (IAU 1980). Units: arcsec.
       T is Julian centuries from J2000.0. */
    double l, lp, f, d, om;

    l = 485866.733 + (1325.0 * 1296000.0 + 715922.633) * t + 31.310 * t * t + 0.064 * t * t * t;
    lp = 1287099.804 + (99.0 * 1296000.0 + 1292581.224) * t - 0.577 * t * t - 0.012 * t * t * t;
    f = 335277.910 + (1342.0 * 1296000.0 + 295263.137) * t - 13.257 * t * t + 0.011 * t * t * t;
    d = 1072261.307 + (1236.0 * 1296000.0 + 1105601.328) * t - 6.891 * t * t + 0.019 * t * t * t;
    om = 450160.280 - (5.0 * 1296000.0 + 482890.539) * t + 7.455 * t * t + 0.008 * t * t * t;

    args[0] = fmod(l, 1296000.0) * JME_ARCSEC_TO_RAD;
    args[1] = fmod(lp, 1296000.0) * JME_ARCSEC_TO_RAD;
    args[2] = fmod(f, 1296000.0) * JME_ARCSEC_TO_RAD;
    args[3] = fmod(d, 1296000.0) * JME_ARCSEC_TO_RAD;
    args[4] = fmod(om, 1296000.0) * JME_ARCSEC_TO_RAD;
}

int jme_get_nutation(double jd_et, int model, double *dpsi, double *deps, char *error)
{
    double effective_jd = jd_et;
    double t;
    double args[5];
    double arg;
    double dp = 0.0;
    double de = 0.0;
    size_t i;

    if (dpsi != 0) { *dpsi = 0.0; }
    if (deps != 0) { *deps = 0.0; }

    if (dpsi == 0 || deps == 0) {
        jme_set_error(error, "Nutation output pointers are required");
        return JME_ERR;
    }

    if (model == JME_MODEL_NUT_IAU_2000A) {
        jme_set_error(error, "IAU 2000A nutation is not available; use IAU 2000B or IAU 1980");
        return JME_ERR;
    }

    if (model != JME_MODEL_NUT_IAU_1980 && model != JME_MODEL_NUT_IAU_2000B) {
        jme_set_error(error, "Unsupported nutation model");
        return JME_ERR;
    }

    if (jme_context_interpolate_nut()) {
        double lower_jd = floor(jd_et);
        double upper_jd = lower_jd + 1.0;
        double weight = jd_et - lower_jd;
        double dp0, de0, dp1, de1;

        if (weight <= 1.0e-12 || weight >= 1.0 - 1.0e-12) {
            effective_jd = weight <= 0.5 ? lower_jd : upper_jd;
        } else {
            jme_get_context()->interpolate_nut = 0;
            if (jme_get_nutation(lower_jd, model, &dp0, &de0, error) != JME_OK
                || jme_get_nutation(upper_jd, model, &dp1, &de1, error) != JME_OK) {
                jme_get_context()->interpolate_nut = 1;
                return JME_ERR;
            }
            jme_get_context()->interpolate_nut = 1;
            if (dpsi != 0) { *dpsi = dp0 + (dp1 - dp0) * weight; }
            if (deps != 0) { *deps = de0 + (de1 - de0) * weight; }
            return JME_OK;
        }
    }

    t = (effective_jd - 2451545.0) / 36525.0;

    if (model == JME_MODEL_NUT_IAU_1980) {
        jme_nutation_args(t, args);
        for (i = 0; i < sizeof(nut80) / sizeof(nut80[0]); i++) {
            arg = (double)nut80[i].l * args[0] + (double)nut80[i].lp * args[1] + (double)nut80[i].f * args[2] + (double)nut80[i].d * args[3] + (double)nut80[i].om * args[4];
            dp += ((double)nut80[i].psi_a + (double)nut80[i].psi_b * 0.1 * t) * sin(arg);
            de += ((double)nut80[i].eps_a + (double)nut80[i].eps_b * 0.1 * t) * cos(arg);
        }

        if (dpsi != 0) { *dpsi = dp * 0.0001 / 3600.0; } /* degrees */
        if (deps != 0) { *deps = de * 0.0001 / 3600.0; } /* degrees */
        return JME_OK;
    }

    if (model == JME_MODEL_NUT_IAU_2000B) {
        double el;
        double elp;
        double f;
        double d;
        double om;
        double sarg;
        double carg;
        const double u2r = JME_ARCSEC_TO_RAD / 10000000.0;
        const double dpplan = -0.135 * JME_ARCSEC_TO_RAD / 1000.0;
        const double deplan = 0.388 * JME_ARCSEC_TO_RAD / 1000.0;

        el = fmod(485868.249036 + 1717915923.2178 * t, 1296000.0) * JME_ARCSEC_TO_RAD;
        elp = fmod(1287104.79305 + 129596581.0481 * t, 1296000.0) * JME_ARCSEC_TO_RAD;
        f = fmod(335779.526232 + 1739527262.8478 * t, 1296000.0) * JME_ARCSEC_TO_RAD;
        d = fmod(1072260.70369 + 1602961601.2090 * t, 1296000.0) * JME_ARCSEC_TO_RAD;
        om = fmod(450160.398036 - 6962890.5431 * t, 1296000.0) * JME_ARCSEC_TO_RAD;

        for (i = sizeof(nut00b) / sizeof(nut00b[0]); i-- > 0;) {
            arg = fmod((double)nut00b[i].l * el
                       + (double)nut00b[i].lp * elp
                       + (double)nut00b[i].f * f
                       + (double)nut00b[i].d * d
                       + (double)nut00b[i].om * om, 2.0 * acos(-1.0));
            sarg = sin(arg);
            carg = cos(arg);
            dp += (nut00b[i].ps + nut00b[i].pst * t) * sarg + nut00b[i].pc * carg;
            de += (nut00b[i].ec + nut00b[i].ect * t) * carg + nut00b[i].es * sarg;
        }

        if (dpsi != 0) { *dpsi = (dp * u2r + dpplan) / JME_DEG_TO_RAD; }
        if (deps != 0) { *deps = (de * u2r + deplan) / JME_DEG_TO_RAD; }
        return JME_OK;
    }

    jme_set_error(error, "Unsupported nutation model");
    return JME_ERR;
}

int jme_get_obliquity(double jd_et, int model, double *eps, char *error)
{
    double t = (jd_et - 2451545.0) / 36525.0;
    double eps0;

    if (eps != 0) { *eps = 0.0; }
    if (eps == 0) {
        jme_set_error(error, "Obliquity output is required");
        return JME_ERR;
    }

    switch (model) {
    case JME_MODEL_OBL_IAU_1980:
        /* IAU 1980 Mean Obliquity */
        eps0 = 84381.448 - 46.8150 * t - 0.00059 * t * t + 0.001813 * t * t * t;
        if (eps != 0) { *eps = eps0 / 3600.0; }
        return JME_OK;
    case JME_MODEL_OBL_IAU_2000:
    case JME_MODEL_OBL_IAU_2006:
        /* IAU 2006 (P03) Mean Obliquity */
        eps0 = 84381.406 - 46.836769 * t - 0.0001831 * t * t + 0.0020034 * t * t * t
               - 0.000000576 * t * t * t * t - 0.0000000434 * t * t * t * t * t;
        if (eps != 0) { *eps = eps0 / 3600.0; }
        return JME_OK;
    default:
        jme_set_error(error, "Unsupported obliquity model");
        return JME_ERR;
    }
}

int jme_get_precession_matrix(double jd_start, double jd_end, int model, double *m)
{
    double t = (jd_start - 2451545.0) / 36525.0;
    double dt = (jd_end - jd_start) / 36525.0;
    double zeta, z, theta;

    if (m == 0) {
        return JME_ERR;
    }

    jme_matrix_identity(m);

    if (model == JME_MODEL_PREC_IAU_1976) {
        /* IAU 1976 Precession (Lieske et al. 1977) */
        zeta = (2306.2181 + 1.39656 * t - 0.000139 * t * t) * dt
               + (0.30188 - 0.000344 * t) * dt * dt + 0.017998 * dt * dt * dt;
        z = zeta + (0.79280 + 0.000411 * t) * dt * dt + 0.000205 * dt * dt * dt;
        theta = (2004.3109 - 0.85330 * t - 0.000217 * t * t) * dt
                - (0.42665 + 0.000217 * t) * dt * dt - 0.041833 * dt * dt * dt;

        zeta *= JME_ARCSEC_TO_RAD;
        z *= JME_ARCSEC_TO_RAD;
        theta *= JME_ARCSEC_TO_RAD;

        jme_matrix_rotate_z(-zeta, m);
        jme_matrix_rotate_y(theta, m);
        jme_matrix_rotate_z(-z, m);

        return JME_OK;
    }

    if (model == JME_MODEL_PREC_IAU_2000
        || model == JME_MODEL_PREC_IAU_2006
        || model == JME_MODEL_PREC_LASKAR_1986
        || model == JME_MODEL_PREC_VONDRAK_2011) {
        double p_start[9];
        double p_end[9];
        double p_start_t[9];
        int i, j;

        {
            double ts = (jd_start - 2451545.0) / 36525.0;
            double gamma = (-0.052928
                + 10.556378 * ts
                + 0.4932044 * ts * ts
                - 0.00031238 * ts * ts * ts
                - 0.000002788 * ts * ts * ts * ts
                + 0.0000000260 * ts * ts * ts * ts * ts) * JME_ARCSEC_TO_RAD;
            double phi = (84381.412819
                - 46.811016 * ts
                + 0.0511268 * ts * ts
                + 0.00053289 * ts * ts * ts
                - 0.000000440 * ts * ts * ts * ts
                - 0.0000000176 * ts * ts * ts * ts * ts) * JME_ARCSEC_TO_RAD;
            double psi = (-0.041775
                + 5038.481484 * ts
                + 1.5584175 * ts * ts
                - 0.00018522 * ts * ts * ts
                - 0.000026452 * ts * ts * ts * ts
                - 0.0000000148 * ts * ts * ts * ts * ts) * JME_ARCSEC_TO_RAD;
            double eps = (84381.406
                - 46.836769 * ts
                - 0.0001831 * ts * ts
                + 0.00200340 * ts * ts * ts
                - 0.000000576 * ts * ts * ts * ts
                - 0.0000000434 * ts * ts * ts * ts * ts) * JME_ARCSEC_TO_RAD;

            jme_matrix_identity(p_start);
            jme_matrix_rotate_z(gamma, p_start);
            jme_matrix_rotate_x(phi, p_start);
            jme_matrix_rotate_z(-psi, p_start);
            jme_matrix_rotate_x(-eps, p_start);
        }

        {
            double te = (jd_end - 2451545.0) / 36525.0;
            double gamma = (-0.052928
                + 10.556378 * te
                + 0.4932044 * te * te
                - 0.00031238 * te * te * te
                - 0.000002788 * te * te * te * te
                + 0.0000000260 * te * te * te * te * te) * JME_ARCSEC_TO_RAD;
            double phi = (84381.412819
                - 46.811016 * te
                + 0.0511268 * te * te
                + 0.00053289 * te * te * te
                - 0.000000440 * te * te * te * te
                - 0.0000000176 * te * te * te * te * te) * JME_ARCSEC_TO_RAD;
            double psi = (-0.041775
                + 5038.481484 * te
                + 1.5584175 * te * te
                - 0.00018522 * te * te * te
                - 0.000026452 * te * te * te * te
                - 0.0000000148 * te * te * te * te * te) * JME_ARCSEC_TO_RAD;
            double eps = (84381.406
                - 46.836769 * te
                - 0.0001831 * te * te
                + 0.00200340 * te * te * te
                - 0.000000576 * te * te * te * te
                - 0.0000000434 * te * te * te * te * te) * JME_ARCSEC_TO_RAD;

            jme_matrix_identity(p_end);
            jme_matrix_rotate_z(gamma, p_end);
            jme_matrix_rotate_x(phi, p_end);
            jme_matrix_rotate_z(-psi, p_end);
            jme_matrix_rotate_x(-eps, p_end);
        }

        for (i = 0; i < 3; i++) {
            for (j = 0; j < 3; j++) {
                p_start_t[i * 3 + j] = p_start[j * 3 + i];
            }
        }
        jme_matrix_multiply(p_end, p_start_t, m);
        return JME_OK;
    }

    return JME_ERR;
}

int jme_get_frame_bias_matrix(int model, double *m)
{
    const double eps0 = 84381.448 * JME_ARCSEC_TO_RAD;
    const double dpsibi = -0.041775 * JME_ARCSEC_TO_RAD;
    const double depsbi = -0.0068192 * JME_ARCSEC_TO_RAD;
    const double dra0 = -0.0146 * JME_ARCSEC_TO_RAD;

    if (m == 0) {
        return JME_ERR;
    }

    jme_matrix_identity(m);

    if (model == JME_MODEL_BIAS_NONE) {
        return JME_OK;
    }

    if (model == JME_MODEL_BIAS_IAU2000 || model == JME_MODEL_BIAS_IAU2006) {
        jme_matrix_rotate_z(dra0, m);
        jme_matrix_rotate_y(dpsibi * sin(eps0), m);
        jme_matrix_rotate_x(-depsbi, m);
        return JME_OK;
    }

    return JME_ERR;
}

void jme_get_nutation_matrix(double dpsi_rad, double deps_rad, double eps_rad, double *m)
{
    jme_matrix_identity(m);
    jme_matrix_rotate_x(eps_rad, m);
    jme_matrix_rotate_z(-dpsi_rad, m);
    jme_matrix_rotate_x(-(eps_rad + deps_rad), m);
}

int jme_get_topo_pos(double jd_et, double *pos_au, char *error)
{
    jme_context *ctx = jme_get_context();
    double eps, dpsi, deps;
    double gst, lst_rad;
    double cos_phi, sin_phi;
    double a = 6378.137; /* WGS84 semi-major axis in km */
    double f = 1.0 / 298.257223563; /* WGS84 flattening */
    double c, s;
    double x_km, y_km, z_km;

    if (pos_au == 0) {
        jme_set_error(error, "Topocentric position output is required");
        return JME_ERR;
    }

    if (jme_get_obliquity(jd_et, jme_context_obliquity_model(), &eps, error) != JME_OK) { return JME_ERR; }
    if (jme_get_nutation(jd_et, jme_context_nutation_model(), &dpsi, &deps, error) != JME_OK) { return JME_ERR; }

    /* Greenwhich Sidereal Time (True) */
    gst = jme_sidereal_time0(jd_et, eps, dpsi); /* Note: jme_sidereal_time0 takes jd_ut, but jd_et is close enough for basic topo */
    /* Local Sidereal Time */
    lst_rad = (gst * 15.0 + ctx->topo_lon) * JME_DEG_TO_RAD;

    cos_phi = cos(ctx->topo_lat * JME_DEG_TO_RAD);
    sin_phi = sin(ctx->topo_lat * JME_DEG_TO_RAD);

    c = 1.0 / sqrt(cos_phi * cos_phi + (1.0 - f) * (1.0 - f) * sin_phi * sin_phi);
    s = (1.0 - f) * (1.0 - f) * c;

    x_km = (a * c + ctx->topo_alt * 0.001) * cos_phi * cos(lst_rad);
    y_km = (a * c + ctx->topo_alt * 0.001) * cos_phi * sin(lst_rad);
    z_km = (a * s + ctx->topo_alt * 0.001) * sin_phi;

    /* x_km, y_km, z_km are in True Equatorial system of date.
       Need to convert to Mean Equatorial J2000 for relative vector arithmetic. */
    {
        double prec_mat[9];
        double nut_mat[9];
        double state[6];

        state[0] = x_km;
        state[1] = y_km;
        state[2] = z_km;
        state[3] = 0.0;
        state[4] = 0.0;
        state[5] = 0.0;

        /* Invert Nutation and Precession to get back to J2000 */
        jme_get_nutation_matrix(dpsi * JME_DEG_TO_RAD, deps * JME_DEG_TO_RAD, eps * JME_DEG_TO_RAD, nut_mat);
        /* Simple matrix inversion for rotation matrices is transpose */
        {
            double inv_nut[9];
            int i, j;
            for (i = 0; i < 3; i++) for (j = 0; j < 3; j++) inv_nut[i * 3 + j] = nut_mat[j * 3 + i];
            jme_matrix_transform_state(inv_nut, state, state);
        }

        jme_get_precession_matrix(2451545.0, jd_et, jme_context_precession_model(), prec_mat);
        {
            double inv_prec[9];
            int i, j;
            for (i = 0; i < 3; i++) for (j = 0; j < 3; j++) inv_prec[i * 3 + j] = prec_mat[j * 3 + i];
            jme_matrix_transform_state(inv_prec, state, state);
        }

        pos_au[0] = state[0] / JME_AU_KM;
        pos_au[1] = state[1] / JME_AU_KM;
        pos_au[2] = state[2] / JME_AU_KM;
    }

    return JME_OK;
}
