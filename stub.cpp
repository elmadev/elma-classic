#include "ALL.H"

int sdl_init(void);
int main() { return sdl_init(); }

void uzenet(char* text1, char* text2, char* text3) { printf("%s %s %s\n", text1, text2, text3); }

void hiba(char* text1, char* text2, char* text3) { printf("%s %s %s\n", text1, text2, text3); }

int s_random(int maximum) { return 0; }

void mv_check(int unused) {}

void setmou(int x, int y) {}
void getmou(int* x, int* y) {}

double mv_stopperido(void) { return 0; }

void mv_startstopper(void) {}

int getbutbmou(void) { return 0; }

int getbutjmou(void) { return 0; }

int Billint = 0;

int mk_kbhit(void) { return 0; }
int mk_getstate(int unused) { return 0; }
int mk_getextchar(void) { return 0; }

void mk_emptychar(void) {}

void initdsound(int secondaryeloir) {}
int controlaltnyomva(void) { return 0; }
void mv_exit(char* text) { exit(0); }
