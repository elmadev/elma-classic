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

unsigned char** lockbackbuffer();
void unlockbackbuffer(void); // Ki is teszi

unsigned char** lockfrontbuffer(void);
void unlockfrontbuffer(void);

extern bool Margrafikus;

#endif
