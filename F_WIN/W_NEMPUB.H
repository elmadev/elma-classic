// w_main.h
#define SDL_MAIN_HANDLED
#include <SDL.h>

extern SDL_Window* SDLWindow;
int sdl_init(void);

// M_V.H-bol nem publikus reszek:
extern HWND GhWnd;           // Window handler
extern HINSTANCE ghInstance; // Instance handler

double mv_getsec(void); // Elvileg le fog allni, ha elkerul vezerles!

void szovegki(int y, char* szoveg); // torlendo:

// M_K.H-bol nem publikus reszek:
void mk_init(void);
// void mk_wmkeydown( int kod );
// void mk_wmkeyup( int kod );
// void mk_wmchar( int kod );

// Kovetkezo billentyu fuggvenyek nem publikusak:
void mkw_getDIstate(void);
// Ezt csak egyszer szabad meghivni minden mkw_getDIstate utan:
void mkw_setkeydown(void);

// Direct input:
void initDI(void);
void releaseDI(void);
void refreshDI(BYTE* buffer);
// int getDIkeystate( int key );
