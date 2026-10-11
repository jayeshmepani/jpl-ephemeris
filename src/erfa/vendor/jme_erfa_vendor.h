#ifndef JME_ERFA_VENDOR_H
#define JME_ERFA_VENDOR_H

/*
**  - - - - - - -
**   e r f a . h
**  - - - - - - -
**
**  Prototype function declarations for ERFA library.
**
**  Copyright (C) 2013-2023, NumFOCUS Foundation.
**  Derived, with permission, from the SOFA library.  See notes at end of file.
*/

#include "math.h"

#ifdef __cplusplus
extern "C" {
#endif

/* Star-independent astrometry parameters */
typedef struct {
   double pmt;        /* PM time interval (SSB, Julian years) */
   double eb[3];      /* SSB to observer (vector, au) */
   double eh[3];      /* Sun to observer (unit vector) */
   double em;         /* distance from Sun to observer (au) */
   double v[3];       /* barycentric observer velocity (vector, c) */
   double bm1;        /* sqrt(1-|v|^2): reciprocal of Lorenz factor */
   double bpn[3][3];  /* bias-precession-nutation matrix */
   double along;      /* longitude + s' + dERA(DUT) (radians) */
   double phi;        /* geodetic latitude (radians) */
   double xpl;        /* polar motion xp wrt local meridian (radians) */
   double ypl;        /* polar motion yp wrt local meridian (radians) */
   double sphi;       /* sine of geodetic latitude */
   double cphi;       /* cosine of geodetic latitude */
   double diurab;     /* magnitude of diurnal aberration vector */
   double eral;       /* "local" Earth rotation angle (radians) */
   double refa;       /* refraction constant A (radians) */
   double refb;       /* refraction constant B (radians) */
} jme_eraASTROM;
/* (Vectors eb, eh, em and v are all with respect to BCRS axes.) */

/* Body parameters for light deflection */
typedef struct {
   double bm;         /* mass of the body (solar masses) */
   double dl;         /* deflection limiter (radians^2/2) */
   double pv[2][3];   /* barycentric PV of the body (au, au/day) */
} jme_eraLDBODY;

/* Astronomy/Calendars */
int jme_eraCal2jd(int iy, int im, int id, double *djm0, double *djm);
double jme_eraEpb(double dj1, double dj2);
void jme_eraEpb2jd(double epb, double *djm0, double *djm);
double jme_eraEpj(double dj1, double dj2);
void jme_eraEpj2jd(double epj, double *djm0, double *djm);
int jme_eraJd2cal(double dj1, double dj2,
                     int *iy, int *im, int *id, double *fd);
int jme_eraJdcalf(int ndp, double dj1, double dj2, int iymdf[4]);

/* Astronomy/Astrometry */
void jme_eraAb(double pnat[3], double v[3], double s, double bm1,
           double ppr[3]);
void jme_eraApcg(double date1, double date2,
             double ebpv[2][3], double ehp[3],
             jme_eraASTROM *astrom);
void jme_eraApcg13(double date1, double date2, jme_eraASTROM *astrom);
void jme_eraApci(double date1, double date2,
             double ebpv[2][3], double ehp[3],
             double x, double y, double s,
             jme_eraASTROM *astrom);
void jme_eraApci13(double date1, double date2,
               jme_eraASTROM *astrom, double *eo);
void jme_eraApco(double date1, double date2,
             double ebpv[2][3], double ehp[3],
             double x, double y, double s, double theta,
             double elong, double phi, double hm,
             double xp, double yp, double sp,
             double refa, double refb,
             jme_eraASTROM *astrom);
int jme_eraApco13(double utc1, double utc2, double dut1,
              double elong, double phi, double hm, double xp, double yp,
              double phpa, double tc, double rh, double wl,
              jme_eraASTROM *astrom, double *eo);
void jme_eraApcs(double date1, double date2, double pv[2][3],
             double ebpv[2][3], double ehp[3],
             jme_eraASTROM *astrom);
void jme_eraApcs13(double date1, double date2, double pv[2][3],
               jme_eraASTROM *astrom);
void jme_eraAper(double theta, jme_eraASTROM *astrom);
void jme_eraAper13(double ut11, double ut12, jme_eraASTROM *astrom);
void jme_eraApio(double sp, double theta,
             double elong, double phi, double hm, double xp, double yp,
             double refa, double refb,
             jme_eraASTROM *astrom);
int jme_eraApio13(double utc1, double utc2, double dut1,
              double elong, double phi, double hm, double xp, double yp,
              double phpa, double tc, double rh, double wl,
              jme_eraASTROM *astrom);
void jme_eraAtcc13(double rc, double dc,
               double pr, double pd, double px, double rv,
               double date1, double date2,
               double *ra, double *da);
void jme_eraAtccq(double rc, double dc,
              double pr, double pd, double px, double rv,
              jme_eraASTROM *astrom, double *ra, double *da);
void jme_eraAtci13(double rc, double dc,
               double pr, double pd, double px, double rv,
               double date1, double date2,
               double *ri, double *di, double *eo);
void jme_eraAtciq(double rc, double dc, double pr, double pd,
              double px, double rv, jme_eraASTROM *astrom,
              double *ri, double *di);
void jme_eraAtciqn(double rc, double dc, double pr, double pd,
               double px, double rv, jme_eraASTROM *astrom,
               int n, jme_eraLDBODY b[], double *ri, double *di);
void jme_eraAtciqz(double rc, double dc, jme_eraASTROM *astrom,
               double *ri, double *di);
int jme_eraAtco13(double rc, double dc,
              double pr, double pd, double px, double rv,
              double utc1, double utc2, double dut1,
              double elong, double phi, double hm, double xp, double yp,
              double phpa, double tc, double rh, double wl,
              double *aob, double *zob, double *hob,
              double *dob, double *rob, double *eo);
void jme_eraAtic13(double ri, double di,
               double date1, double date2,
               double *rc, double *dc, double *eo);
void jme_eraAticq(double ri, double di, jme_eraASTROM *astrom,
              double *rc, double *dc);
void jme_eraAticqn(double ri, double di, jme_eraASTROM *astrom,
               int n, jme_eraLDBODY b[], double *rc, double *dc);
int jme_eraAtio13(double ri, double di,
              double utc1, double utc2, double dut1,
              double elong, double phi, double hm, double xp, double yp,
              double phpa, double tc, double rh, double wl,
              double *aob, double *zob, double *hob,
              double *dob, double *rob);
void jme_eraAtioq(double ri, double di, jme_eraASTROM *astrom,
              double *aob, double *zob,
              double *hob, double *dob, double *rob);
int jme_eraAtoc13(const char *type, double ob1, double ob2,
              double utc1, double utc2, double dut1,
              double elong, double phi, double hm, double xp, double yp,
              double phpa, double tc, double rh, double wl,
              double *rc, double *dc);
int jme_eraAtoi13(const char *type, double ob1, double ob2,
              double utc1, double utc2, double dut1,
              double elong, double phi, double hm, double xp, double yp,
              double phpa, double tc, double rh, double wl,
              double *ri, double *di);
void jme_eraAtoiq(const char *type,
              double ob1, double ob2, jme_eraASTROM *astrom,
              double *ri, double *di);
void jme_eraLd(double bm, double p[3], double q[3], double e[3],
           double em, double dlim, double p1[3]);
void jme_eraLdn(int n, jme_eraLDBODY b[], double ob[3], double sc[3],
            double sn[3]);
void jme_eraLdsun(double p[3], double e[3], double em, double p1[3]);
void jme_eraPmpx(double rc, double dc, double pr, double pd,
             double px, double rv, double pmt, double pob[3],
             double pco[3]);
int jme_eraPmsafe(double ra1, double dec1, double pmr1, double pmd1,
              double px1, double rv1,
              double ep1a, double ep1b, double ep2a, double ep2b,
              double *ra2, double *dec2, double *pmr2, double *pmd2,
              double *px2, double *rv2);
void jme_eraPvtob(double elong, double phi, double height, double xp,
              double yp, double sp, double theta, double pv[2][3]);
void jme_eraRefco(double phpa, double tc, double rh, double wl,
              double *refa, double *refb);

/* Astronomy/Ephemerides */
int jme_eraEpv00(double date1, double date2,
             double pvh[2][3], double pvb[2][3]);
void jme_eraMoon98(double date1, double date2, double pv[2][3]);
int jme_eraPlan94(double date1, double date2, int np, double pv[2][3]);

/* Astronomy/FundamentalArgs */
double jme_eraFad03(double t);
double jme_eraFae03(double t);
double jme_eraFaf03(double t);
double jme_eraFaju03(double t);
double jme_eraFal03(double t);
double jme_eraFalp03(double t);
double jme_eraFama03(double t);
double jme_eraFame03(double t);
double jme_eraFane03(double t);
double jme_eraFaom03(double t);
double jme_eraFapa03(double t);
double jme_eraFasa03(double t);
double jme_eraFaur03(double t);
double jme_eraFave03(double t);

/* Astronomy/PrecNutPolar */
void jme_eraBi00(double *dpsibi, double *depsbi, double *dra);
void jme_eraBp00(double date1, double date2,
             double rb[3][3], double rp[3][3], double rbp[3][3]);
void jme_eraBp06(double date1, double date2,
             double rb[3][3], double rp[3][3], double rbp[3][3]);
void jme_eraBpn2xy(double rbpn[3][3], double *x, double *y);
void jme_eraC2i00a(double date1, double date2, double rc2i[3][3]);
void jme_eraC2i00b(double date1, double date2, double rc2i[3][3]);
void jme_eraC2i06a(double date1, double date2, double rc2i[3][3]);
void jme_eraC2ibpn(double date1, double date2, double rbpn[3][3],
               double rc2i[3][3]);
void jme_eraC2ixy(double date1, double date2, double x, double y,
              double rc2i[3][3]);
void jme_eraC2ixys(double x, double y, double s, double rc2i[3][3]);
void jme_eraC2t00a(double tta, double ttb, double uta, double utb,
               double xp, double yp, double rc2t[3][3]);
void jme_eraC2t00b(double tta, double ttb, double uta, double utb,
               double xp, double yp, double rc2t[3][3]);
void jme_eraC2t06a(double tta, double ttb, double uta, double utb,
               double xp, double yp, double rc2t[3][3]);
void jme_eraC2tcio(double rc2i[3][3], double era, double rpom[3][3],
               double rc2t[3][3]);
void jme_eraC2teqx(double rbpn[3][3], double gst, double rpom[3][3],
               double rc2t[3][3]);
void jme_eraC2tpe(double tta, double ttb, double uta, double utb,
              double dpsi, double deps, double xp, double yp,
              double rc2t[3][3]);
void jme_eraC2txy(double tta, double ttb, double uta, double utb,
              double x, double y, double xp, double yp,
              double rc2t[3][3]);
double jme_eraEo06a(double date1, double date2);
double jme_eraEors(double rnpb[3][3], double s);
void jme_eraFw2m(double gamb, double phib, double psi, double eps,
             double r[3][3]);
void jme_eraFw2xy(double gamb, double phib, double psi, double eps,
              double *x, double *y);
void jme_eraLtp(double epj, double rp[3][3]);
void jme_eraLtpb(double epj, double rpb[3][3]);
void jme_eraLtpecl(double epj, double vec[3]);
void jme_eraLtpequ(double epj, double veq[3]);
void jme_eraNum00a(double date1, double date2, double rmatn[3][3]);
void jme_eraNum00b(double date1, double date2, double rmatn[3][3]);
void jme_eraNum06a(double date1, double date2, double rmatn[3][3]);
void jme_eraNumat(double epsa, double dpsi, double deps, double rmatn[3][3]);
void jme_eraNut00a(double date1, double date2, double *dpsi, double *deps);
void jme_eraNut00b(double date1, double date2, double *dpsi, double *deps);
void jme_eraNut06a(double date1, double date2, double *dpsi, double *deps);
void jme_eraNut80(double date1, double date2, double *dpsi, double *deps);
void jme_eraNutm80(double date1, double date2, double rmatn[3][3]);
double jme_eraObl06(double date1, double date2);
double jme_eraObl80(double date1, double date2);
void jme_eraP06e(double date1, double date2,
             double *eps0, double *psia, double *oma, double *bpa,
             double *bqa, double *pia, double *bpia,
             double *epsa, double *chia, double *za, double *zetaa,
             double *thetaa, double *pa,
             double *gam, double *phi, double *psi);
void jme_eraPb06(double date1, double date2,
             double *bzeta, double *bz, double *btheta);
void jme_eraPfw06(double date1, double date2,
              double *gamb, double *phib, double *psib, double *epsa);
void jme_eraPmat00(double date1, double date2, double rbp[3][3]);
void jme_eraPmat06(double date1, double date2, double rbp[3][3]);
void jme_eraPmat76(double date1, double date2, double rmatp[3][3]);
void jme_eraPn00(double date1, double date2, double dpsi, double deps,
             double *epsa,
             double rb[3][3], double rp[3][3], double rbp[3][3],
             double rn[3][3], double rbpn[3][3]);
void jme_eraPn00a(double date1, double date2,
              double *dpsi, double *deps, double *epsa,
              double rb[3][3], double rp[3][3], double rbp[3][3],
              double rn[3][3], double rbpn[3][3]);
void jme_eraPn00b(double date1, double date2,
              double *dpsi, double *deps, double *epsa,
              double rb[3][3], double rp[3][3], double rbp[3][3],
              double rn[3][3], double rbpn[3][3]);
void jme_eraPn06(double date1, double date2, double dpsi, double deps,
             double *epsa,
             double rb[3][3], double rp[3][3], double rbp[3][3],
             double rn[3][3], double rbpn[3][3]);
void jme_eraPn06a(double date1, double date2,
              double *dpsi, double *deps, double *epsa,
              double rb[3][3], double rp[3][3], double rbp[3][3],
              double rn[3][3], double rbpn[3][3]);
void jme_eraPnm00a(double date1, double date2, double rbpn[3][3]);
void jme_eraPnm00b(double date1, double date2, double rbpn[3][3]);
void jme_eraPnm06a(double date1, double date2, double rnpb[3][3]);
void jme_eraPnm80(double date1, double date2, double rmatpn[3][3]);
void jme_eraPom00(double xp, double yp, double sp, double rpom[3][3]);
void jme_eraPr00(double date1, double date2,
             double *dpsipr, double *depspr);
void jme_eraPrec76(double date01, double date02,
               double date11, double date12,
               double *zeta, double *z, double *theta);
double jme_eraS00(double date1, double date2, double x, double y);
double jme_eraS00a(double date1, double date2);
double jme_eraS00b(double date1, double date2);
double jme_eraS06(double date1, double date2, double x, double y);
double jme_eraS06a(double date1, double date2);
double jme_eraSp00(double date1, double date2);
void jme_eraXy06(double date1, double date2, double *x, double *y);
void jme_eraXys00a(double date1, double date2,
               double *x, double *y, double *s);
void jme_eraXys00b(double date1, double date2,
               double *x, double *y, double *s);
void jme_eraXys06a(double date1, double date2,
               double *x, double *y, double *s);

/* Astronomy/RotationAndTime */
double jme_eraEe00(double date1, double date2, double epsa, double dpsi);
double jme_eraEe00a(double date1, double date2);
double jme_eraEe00b(double date1, double date2);
double jme_eraEe06a(double date1, double date2);
double jme_eraEect00(double date1, double date2);
double jme_eraEqeq94(double date1, double date2);
double jme_eraEra00(double dj1, double dj2);
double jme_eraGmst00(double uta, double utb, double tta, double ttb);
double jme_eraGmst06(double uta, double utb, double tta, double ttb);
double jme_eraGmst82(double dj1, double dj2);
double jme_eraGst00a(double uta, double utb, double tta, double ttb);
double jme_eraGst00b(double uta, double utb);
double jme_eraGst06(double uta, double utb, double tta, double ttb,
                double rnpb[3][3]);
double jme_eraGst06a(double uta, double utb, double tta, double ttb);
double jme_eraGst94(double uta, double utb);

/* Astronomy/SpaceMotion */
int jme_eraPvstar(double pv[2][3], double *ra, double *dec,
              double *pmr, double *pmd, double *px, double *rv);
int jme_eraStarpv(double ra, double dec,
              double pmr, double pmd, double px, double rv,
              double pv[2][3]);

/* Astronomy/StarCatalogs */
void jme_eraFk425(double r1950, double d1950,
              double dr1950, double dd1950,
              double p1950, double v1950,
              double *r2000, double *d2000,
              double *dr2000, double *dd2000,
              double *p2000, double *v2000);
void jme_eraFk45z(double r1950, double d1950, double bepoch,
              double *r2000, double *d2000);
void jme_eraFk524(double r2000, double d2000,
              double dr2000, double dd2000,
              double p2000, double v2000,
              double *r1950, double *d1950,
              double *dr1950, double *dd1950,
              double *p1950, double *v1950);
void jme_eraFk52h(double r5, double d5,
              double dr5, double dd5, double px5, double rv5,
              double *rh, double *dh,
              double *drh, double *ddh, double *pxh, double *rvh);
void jme_eraFk54z(double r2000, double d2000, double bepoch,
              double *r1950, double *d1950,
              double *dr1950, double *dd1950);
void jme_eraFk5hip(double r5h[3][3], double s5h[3]);
void jme_eraFk5hz(double r5, double d5, double date1, double date2,
              double *rh, double *dh);
void jme_eraH2fk5(double rh, double dh,
              double drh, double ddh, double pxh, double rvh,
              double *r5, double *d5,
              double *dr5, double *dd5, double *px5, double *rv5);
void jme_eraHfk5z(double rh, double dh, double date1, double date2,
              double *r5, double *d5, double *dr5, double *dd5);
int jme_eraStarpm(double ra1, double dec1,
              double pmr1, double pmd1, double px1, double rv1,
              double ep1a, double ep1b, double ep2a, double ep2b,
              double *ra2, double *dec2,
              double *pmr2, double *pmd2, double *px2, double *rv2);

/* Astronomy/EclipticCoordinates */
void jme_eraEceq06(double date1, double date2, double dl, double db,
               double *dr, double *dd);
void jme_eraEcm06(double date1, double date2, double rm[3][3]);
void jme_eraEqec06(double date1, double date2, double dr, double dd,
               double *dl, double *db);
void jme_eraLteceq(double epj, double dl, double db, double *dr, double *dd);
void jme_eraLtecm(double epj, double rm[3][3]);
void jme_eraLteqec(double epj, double dr, double dd, double *dl, double *db);

/* Astronomy/GalacticCoordinates */
void jme_eraG2icrs(double dl, double db, double *dr, double *dd);
void jme_eraIcrs2g(double dr, double dd, double *dl, double *db);

/* Astronomy/GeodeticGeocentric */
int jme_eraEform(int n, double *a, double *f);
int jme_eraGc2gd(int n, double xyz[3],
             double *elong, double *phi, double *height);
int jme_eraGc2gde(double a, double f, double xyz[3],
              double *elong, double *phi, double *height);
int jme_eraGd2gc(int n, double elong, double phi, double height,
             double xyz[3]);
int jme_eraGd2gce(double a, double f,
              double elong, double phi, double height, double xyz[3]);

/* Astronomy/Timescales */
int jme_eraD2dtf(const char *scale, int ndp, double d1, double d2,
             int *iy, int *im, int *id, int ihmsf[4]);
int jme_eraDat(int iy, int im, int id, double fd, double *deltat);
double jme_eraDtdb(double date1, double date2,
               double ut, double elong, double u, double v);
int jme_eraDtf2d(const char *scale, int iy, int im, int id,
             int ihr, int imn, double sec, double *d1, double *d2);
int jme_eraTaitt(double tai1, double tai2, double *tt1, double *tt2);
int jme_eraTaiut1(double tai1, double tai2, double dta,
              double *ut11, double *ut12);
int jme_eraTaiutc(double tai1, double tai2, double *utc1, double *utc2);
int jme_eraTcbtdb(double tcb1, double tcb2, double *tdb1, double *tdb2);
int jme_eraTcgtt(double tcg1, double tcg2, double *tt1, double *tt2);
int jme_eraTdbtcb(double tdb1, double tdb2, double *tcb1, double *tcb2);
int jme_eraTdbtt(double tdb1, double tdb2, double dtr,
             double *tt1, double *tt2);
int jme_eraTttai(double tt1, double tt2, double *tai1, double *tai2);
int jme_eraTttcg(double tt1, double tt2, double *tcg1, double *tcg2);
int jme_eraTttdb(double tt1, double tt2, double dtr,
             double *tdb1, double *tdb2);
int jme_eraTtut1(double tt1, double tt2, double dt,
             double *ut11, double *ut12);
int jme_eraUt1tai(double ut11, double ut12, double dta,
              double *tai1, double *tai2);
int jme_eraUt1tt(double ut11, double ut12, double dt,
             double *tt1, double *tt2);
int jme_eraUt1utc(double ut11, double ut12, double dut1,
              double *utc1, double *utc2);
int jme_eraUtctai(double utc1, double utc2, double *tai1, double *tai2);
int jme_eraUtcut1(double utc1, double utc2, double dut1,
              double *ut11, double *ut12);

/* Astronomy/HorizonEquatorial */
void jme_eraAe2hd(double az, double el, double phi,
              double *ha, double *dec);
void jme_eraHd2ae(double ha, double dec, double phi,
              double *az, double *el);
double jme_eraHd2pa(double ha, double dec, double phi);

/* Astronomy/Gnomonic */
int jme_eraTpors(double xi, double eta, double a, double b,
             double *a01, double *b01, double *a02, double *b02);
int jme_eraTporv(double xi, double eta, double v[3],
             double v01[3], double v02[3]);
void jme_eraTpsts(double xi, double eta, double a0, double b0,
              double *a, double *b);
void jme_eraTpstv(double xi, double eta, double v0[3], double v[3]);
int jme_eraTpxes(double a, double b, double a0, double b0,
             double *xi, double *eta);
int jme_eraTpxev(double v[3], double v0[3], double *xi, double *eta);

/* VectorMatrix/AngleOps */
void jme_eraA2af(int ndp, double angle, char *sign, int idmsf[4]);
void jme_eraA2tf(int ndp, double angle, char *sign, int ihmsf[4]);
int jme_eraAf2a(char s, int ideg, int iamin, double asec, double *rad);
double jme_eraAnp(double a);
double jme_eraAnpm(double a);
void jme_eraD2tf(int ndp, double days, char *sign, int ihmsf[4]);
int jme_eraTf2a(char s, int ihour, int imin, double sec, double *rad);
int jme_eraTf2d(char s, int ihour, int imin, double sec, double *days);

/* VectorMatrix/BuildRotations */
void jme_eraRx(double phi, double r[3][3]);
void jme_eraRy(double theta, double r[3][3]);
void jme_eraRz(double psi, double r[3][3]);

/* VectorMatrix/CopyExtendExtract */
void jme_eraCp(double p[3], double c[3]);
void jme_eraCpv(double pv[2][3], double c[2][3]);
void jme_eraCr(double r[3][3], double c[3][3]);
void jme_eraP2pv(double p[3], double pv[2][3]);
void jme_eraPv2p(double pv[2][3], double p[3]);

/* VectorMatrix/Initialization */
void jme_eraIr(double r[3][3]);
void jme_eraZp(double p[3]);
void jme_eraZpv(double pv[2][3]);
void jme_eraZr(double r[3][3]);

/* VectorMatrix/MatrixOps */
void jme_eraRxr(double a[3][3], double b[3][3], double atb[3][3]);
void jme_eraTr(double r[3][3], double rt[3][3]);

/* VectorMatrix/MatrixVectorProducts */
void jme_eraRxp(double r[3][3], double p[3], double rp[3]);
void jme_eraRxpv(double r[3][3], double pv[2][3], double rpv[2][3]);
void jme_eraTrxp(double r[3][3], double p[3], double trp[3]);
void jme_eraTrxpv(double r[3][3], double pv[2][3], double trpv[2][3]);

/* VectorMatrix/RotationVectors */
void jme_eraRm2v(double r[3][3], double w[3]);
void jme_eraRv2m(double w[3], double r[3][3]);

/* VectorMatrix/SeparationAndAngle */
double jme_eraPap(double a[3], double b[3]);
double jme_eraPas(double al, double ap, double bl, double bp);
double jme_eraSepp(double a[3], double b[3]);
double jme_eraSeps(double al, double ap, double bl, double bp);

/* VectorMatrix/SphericalCartesian */
void jme_eraC2s(double p[3], double *theta, double *phi);
void jme_eraP2s(double p[3], double *theta, double *phi, double *r);
void jme_eraPv2s(double pv[2][3],
             double *theta, double *phi, double *r,
             double *td, double *pd, double *rd);
void jme_eraS2c(double theta, double phi, double c[3]);
void jme_eraS2p(double theta, double phi, double r, double p[3]);
void jme_eraS2pv(double theta, double phi, double r,
             double td, double pd, double rd,
             double pv[2][3]);

/* VectorMatrix/VectorOps */
double jme_eraPdp(double a[3], double b[3]);
double jme_eraPm(double p[3]);
void jme_eraPmp(double a[3], double b[3], double amb[3]);
void jme_eraPn(double p[3], double *r, double u[3]);
void jme_eraPpp(double a[3], double b[3], double apb[3]);
void jme_eraPpsp(double a[3], double s, double b[3], double apsb[3]);
void jme_eraPvdpv(double a[2][3], double b[2][3], double adb[2]);
void jme_eraPvm(double pv[2][3], double *r, double *s);
void jme_eraPvmpv(double a[2][3], double b[2][3], double amb[2][3]);
void jme_eraPvppv(double a[2][3], double b[2][3], double apb[2][3]);
void jme_eraPvu(double dt, double pv[2][3], double upv[2][3]);
void jme_eraPvup(double dt, double pv[2][3], double p[3]);
void jme_eraPvxpv(double a[2][3], double b[2][3], double axb[2][3]);
void jme_eraPxp(double a[3], double b[3], double axb[3]);
void jme_eraS2xpv(double s1, double s2, double pv[2][3], double spv[2][3]);
void jme_eraSxp(double s, double p[3], double sp[3]);
void jme_eraSxpv(double s, double pv[2][3], double spv[2][3]);
void jme_eraFk52h(double r5, double d5,
              double dr5, double dd5, double px5, double rv5,
              double *rh, double *dh,
              double *drh, double *ddh, double *pxh, double *rvh);
void jme_eraFk5hip(double r5h[3][3], double s5h[3]);
void jme_eraRv2m(double w[3], double r[3][3]);
void jme_eraRxp(double r[3][3], double p[3], double rp[3]);

#ifdef __cplusplus
}
#endif

#endif


/*----------------------------------------------------------------------
**  
**  
**  Copyright (C) 2013-2023, NumFOCUS Foundation.
**  All rights reserved.
**  
**  This library is derived, with permission, from the International
**  Astronomical Union's "Standards of Fundamental Astronomy" library,
**  available from http://www.iausofa.org.
**  
**  The ERFA version is intended to retain identical functionality to
**  the SOFA library, but made distinct through different function and
**  file names, as set out in the SOFA license conditions.  The SOFA
**  original has a role as a reference standard for the IAU and IERS,
**  and consequently redistribution is permitted only in its unaltered
**  state.  The ERFA version is not subject to this restriction and
**  therefore can be included in distributions which do not support the
**  concept of "read only" software.
**  
**  Although the intent is to replicate the SOFA API (other than
**  replacement of prefix names) and results (with the exception of
**  bugs;  any that are discovered will be fixed), SOFA is not
**  responsible for any errors found in this version of the library.
**  
**  If you wish to acknowledge the SOFA heritage, please acknowledge
**  that you are using a library derived from SOFA, rather than SOFA
**  itself.
**  
**  
**  TERMS AND CONDITIONS
**  
**  Redistribution and use in source and binary forms, with or without
**  modification, are permitted provided that the following conditions
**  are met:
**  
**  1 Redistributions of source code must retain the above copyright
**    notice, this list of conditions and the following disclaimer.
**  
**  2 Redistributions in binary form must reproduce the above copyright
**    notice, this list of conditions and the following disclaimer in
**    the documentation and/or other materials provided with the
**    distribution.
**  
**  3 Neither the name of the Standards Of Fundamental Astronomy Board,
**    the International Astronomical Union nor the names of its
**    contributors may be used to endorse or promote products derived
**    from this software without specific prior written permission.
**  
**  THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
**  "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
**  LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS
**  FOR A PARTICULAR PURPOSE ARE DISCLAIMED.  IN NO EVENT SHALL THE
**  COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT,
**  INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING,
**  BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
**  LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
**  CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
**  LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN
**  ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
**  POSSIBILITY OF SUCH DAMAGE.
**  
*/
