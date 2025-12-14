#ifndef QOPEN_H
#define QOPEN_H

FILE* qopen(const char* nev, const char* paramok);
void qclose(FILE* h);
int qseek(FILE* stream, long offset, int whence);

void initqopen(void);
void becsomagol(void);

#endif
