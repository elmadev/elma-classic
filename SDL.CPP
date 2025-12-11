#include "ALL.H"
#include <SDL.h>

SDL_Window* SDLWindow;
SDL_Surface* SDLSurfaceMain;
SDL_Surface* SDLSurfacePaletted;

//if true, y starts from below (set in kirajz320)
int Locky0_alul = 0;

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

    SDLSurfacePaletted = SDL_CreateRGBSurfaceWithFormat(0, SDLSurfaceMain->w, SDLSurfaceMain->h, 0,
                                                        SDL_PIXELFORMAT_INDEX8);
    if (!SDLSurfacePaletted) {
        hiba((char*)SDL_GetError());
        return 1;
    }

    return 0;
}

static unsigned char* Mutbuffer[480];
static int Bufflocked = 0;

unsigned char** lockbackbuffer() {
    if (Bufflocked) {
        hiba(" Bufflocked in lockbackbuffer!");
    }
    Bufflocked = 1;

    unsigned char* mut = (unsigned char*)SDLSurfacePaletted->pixels;
    if (Locky0_alul) {
        // Set the row buffer bottom-down
        for (int y = 0; y < 480; y++) {
            Mutbuffer[480 - 1 - y] = mut;
            mut += SDLSurfacePaletted->w;
        }
    } else {
        // Set the row buffer top-down
        for (int y = 0; y < 480; y++) {
            Mutbuffer[y] = mut;
            mut += SDLSurfacePaletted->w;
        }
    }

    return Mutbuffer;
}

void unlockbackbuffer(void) {
    if (!Bufflocked) {
        hiba("!Bufflocked in unlockbackbuffer!");
    }
    Bufflocked = 0;

    SDL_BlitSurface(SDLSurfacePaletted, NULL, SDLSurfaceMain, NULL);
    SDL_UpdateWindowSurface(SDLWindow);
}

unsigned char** lockfrontbuffer(void) {
    if (Bufflocked) {
        hiba("Bufflocked in lockfrontbuffer!");
    }

    return lockbackbuffer();
}

void unlockfrontbuffer(void) {
    if (!Bufflocked) {
        hiba("!Bufflocked in unlockbackbuffer!");
    }

    unlockbackbuffer();
}

ddpal::ddpal(unsigned char* tomb) {
    for (int i = 0; i < 256; i++) {
        pal[i].r = tomb[3 * i] << 2;
        pal[i].g = tomb[3 * i + 1] << 2;
        pal[i].b = tomb[3 * i + 2] << 2;
        pal[i].a = 0xFF;
    }
}

void ddpal::set() { SDL_SetPaletteColors(SDLSurfacePaletted->format->palette, pal, 0, 256); }
