#ifndef MAIN_H
#define MAIN_H

extern bool ErrorGraphicsLoaded;

void quit();

double stopwatch();
void stopwatch_reset();

void internal_error(const char* text1, const char* text2 = nullptr, const char* text3 = nullptr);
void external_error(const char* text1, const char* text2 = nullptr, const char* text3 = nullptr);

int random_range(int maximum);

#endif
