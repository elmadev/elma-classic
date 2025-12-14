#ifndef FOGOCSKA_H
#define FOGOCSKA_H

void fogocskareset(void);
void fogocska(double t);
// Aafogo alapjan csak idoket allitja:
void fogocska_r(double t);

extern int Aafogo, Kozelvannak;
extern int Aleszfogo; // Legkozelebbi jateknal
extern double Afogoido, Bfogoido;

#endif
