#ifndef QOPEN_H
#define QOPEN_H

#include <cstdio>

FILE* qopen(const char* filename, const char* mode);
void qclose(FILE* h);
int qseek(FILE* stream, int offset, int whence);

void init_qopen();

#endif
