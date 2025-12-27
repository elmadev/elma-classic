#ifndef MAIN_H
#define MAIN_H

extern bool Margrafikus;

void mv_exit();

double mv_stopperido(void);
void mv_startstopper(void);

void hiba(const char* text1, const char* text2 = NULL, const char* text3 = NULL);
void uzenet(const char* text1, const char* text2 = NULL, const char* text3 = NULL);

int s_random(int);

#endif
