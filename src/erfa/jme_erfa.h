#ifndef JME_ERFA_TIME_H
#define JME_ERFA_TIME_H

double jme_erfa_dtdb(
    double date1,
    double date2,
    double ut,
    double elong,
    double u,
    double v
);

int jme_erfa_tdbtcb(
    double tdb1,
    double tdb2,
    double *tcb1,
    double *tcb2
);

#endif
