#ifndef GOLYUTK_H
#define GOLYUTK_H

enum { ALSO = 0, FELSO, JOBB, BAL };

double ggutkozesideje(ball*, ball*);
void ggutkozes(ball*, ball*, double t);

double goutkozesideje(ball*, int);
void goutkozes(ball*, int, double t);

void idoreigazit(ball* m, double t);

extern int Nincsfal; // Ha ez igaz, golyok kimehetnek falon kivulre

#endif
