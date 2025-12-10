// m_dd.h
#include <SDL_pixels.h>

class pic8;
class palyaleiro;
class recorder;

class ddpal {
    SDL_Color pal[256];

  public:
    ddpal(unsigned char* tomb);
    ~ddpal(void);
    void set(void);
};

// Ha igaz, alulrol indul y (kirajz320 allitja igazba):
extern int Locky0_alul;

unsigned char** lockbackbuffer(int fizikai);
void unlockbackbuffer(void); // Ki is teszi

unsigned char** lockfrontbuffer(void);
void unlockfrontbuffer(void);
