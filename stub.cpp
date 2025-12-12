#include "ALL.H"

void uzenet(const char* text1, const char* text2, const char* text3) {
    printf("%s %s %s\n", text1, text2, text3);
    mv_exit();
}

void hiba(const char* text1, const char* text2, const char* text3) {
    printf("%s %s %s\n", text1, text2, text3);
    mv_exit();
}

int s_random(int maximum) { return 0; }

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
