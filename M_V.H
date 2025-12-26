#ifndef M_V_H
#define M_V_H

// Ezt neha meg kell hivni!:
void mv_exit(); // Ezzel kell kilepni!

// 182*sec-et adja vissza idot tortresszel egyutt!
// d_stoppe.cpp-ben vannak megvalositva:
double mv_stopperido(void);
void mv_startstopper(void);

void hiba(const char* text1, const char* text2 = NULL, const char* text3 = NULL);
void uzenet(const char* text1, const char* text2 = NULL, const char* text3 = NULL);

int s_random(int); // Ugyanaz mint random, de az nincs WATCOM-ban

#endif
