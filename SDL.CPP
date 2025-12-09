#include "ALL.H"
#include <SDL.h>

SDL_Window* SDLWindow;
SDL_Surface* SDLSurfaceMain;

int sdl_init() {
    if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_EVENTS)) {
        hiba((char*)SDL_GetError());
        return 1;
    }

    SDLWindow = SDL_CreateWindow("Elasto Mania", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED,
                                 640, 480, 0);
    if (!SDLWindow) {
        hiba((char*)SDL_GetError());
        return 1;
    }

    SDL_EventState(SDL_DROPFILE, SDL_DISABLE);
    SDL_EventState(SDL_DROPTEXT, SDL_DISABLE);

    SDLSurfaceMain = SDL_GetWindowSurface(SDLWindow);
    if (!SDLSurfaceMain) {
        hiba((char*)SDL_GetError());
        return 1;
    }

    SDL_LockSurface(SDLSurfaceMain);
    for (int i = 0; i < (SDLSurfaceMain->w * SDLSurfaceMain->h * 4); i++) {
        ((unsigned char*)SDLSurfaceMain->pixels)[i] = 0xFF;
    }
    SDL_UnlockSurface(SDLSurfaceMain);
    SDL_UpdateWindowSurface(SDLWindow);

    return 0;
}
