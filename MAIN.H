#ifndef MAIN_H
#define MAIN_H

extern bool ErrorGraphicsLoaded;

void quit();

double stopwatch(void);
void stopwatch_reset(void);

void internal_error(const char* text1, const char* text2 = NULL, const char* text3 = NULL);
void external_error(const char* text1, const char* text2 = NULL, const char* text3 = NULL);

int random_range(int);

#endif
