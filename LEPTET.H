#ifndef LEPTET_H
#define LEPTET_H

struct motorst;

void resetleptet(motorst* pmot);
void leptet(motorst* pmot, double most, double dt, int gaz, int fek, int ugrik1, int ugrik2);

// csak replay eseten kell meghivni, mivel leptet elvegzi:
void szamitfejr(motorst* pmot);

// 0-meghalt, 2-semmi kulonos:
int vizsgalat(motorst* pmot);

double kiszamolsurlodast(void);

#endif
