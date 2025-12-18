#ifndef GOLYOK_H
#define GOLYOK_H

typedef struct {
    vekt2 r,
        // relozo,
        rmostani, v;
    double alfa, alfamostani, omega;
    double sug;
    double t;
} golyo;

void initgolyo(void);
void leptetgolyo(double stept);
// void ujraszamol( void );
// void idoreigazitmindet( void );

extern int Ngolyok;
extern golyo* Golyok;

extern double Hosszuido;

extern int Konstrualva;

extern double** Idok;
extern double* T;

#endif
