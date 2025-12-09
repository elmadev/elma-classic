// m_dd.h
#if defined(_WIN64) || !defined(_WIN32)
#include <SDL_pixels.h>
#endif

class pic8;
class palyaleiro;
class recorder;

class ddpal {
    friend void lassufizre(pic8* ppic, ddpal* ppal);
#if defined(_WIN64) || !defined(_WIN32)
    SDL_Color pal[256];
#endif

  public:
#if defined(_WIN32) && !defined(_WIN64)
    void* pddpal;
#endif

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
