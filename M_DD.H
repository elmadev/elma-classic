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

void lockbackbuffer(pic8* view);
void unlockbackbuffer(void); // Ki is teszi

void lockfrontbuffer(pic8* view);
void unlockfrontbuffer(void);

#endif
