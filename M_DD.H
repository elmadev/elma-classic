#ifndef M_DD_H
#define M_DD_H

#include <SDL_pixels.h>

class pic8;
class palyaleiro;
class recorder;

class ddpal {
    SDL_Color pal[256];

  public:
    ddpal(unsigned char* tomb);
    void set(void);
};

// Ha igaz, alulrol indul y (kirajz320 allitja igazba):
extern int Locky0_alul;

unsigned char** lock_backbuffer();
void unlock_backbuffer(); // Ki is teszi

unsigned char** lock_frontbuffer();
void unlock_frontbuffer();

extern bool Margrafikus;

#endif
