enum { ALSO = 0, FELSO, JOBB, BAL };

double ggutkozesideje(golyo*, golyo*);
void ggutkozes(golyo*, golyo*, double t);

double goutkozesideje(golyo*, int);
void goutkozes(golyo*, int, double t);

void idoreigazit(golyo* m, double t);

// Visszaadja osszenergiat:
double getenergia(void);
// Minden sebesseget es forgasit megszoroz ezzel:
void szorozmindet(double szorzo);
