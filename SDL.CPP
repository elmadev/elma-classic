#include "ALL.H"
#include <SDL.h>
#include "scancodes_windows.h"

SDL_Window* SDLWindow;
SDL_Surface* SDLSurfaceMain;
SDL_Surface* SDLSurfacePaletted;

static bool LeftMouseDownPrev = false;
static bool RightMouseDownPrev = false;
static bool LeftMouseDown = false;
static bool RightMouseDown = false;

void message_box(const char* text) {
    // As per docs, can be called even before SDL_Init
    // SDLWindow will either be a handle to the window, or nullptr if no parent
    SDL_ShowSimpleMessageBox(SDL_MESSAGEBOX_ERROR, "Message", text, SDLWindow);
}

void platform_init() {
    if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_EVENTS)) {
        hiba(SDL_GetError());
        return;
    }

    SDLWindow = SDL_CreateWindow("Elasto Mania", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED,
                                 SCREEN_WIDTH, SCREEN_HEIGHT, 0);
    if (!SDLWindow) {
        hiba(SDL_GetError());
        return;
    }

    SDL_EventState(SDL_DROPFILE, SDL_DISABLE);
    SDL_EventState(SDL_DROPTEXT, SDL_DISABLE);

    SDLSurfaceMain = SDL_GetWindowSurface(SDLWindow);
    if (!SDLSurfaceMain) {
        hiba(SDL_GetError());
        return;
    }
    SDLSurfacePaletted = SDL_CreateRGBSurfaceWithFormat(0, SDLSurfaceMain->w, SDLSurfaceMain->h, 0,
                                                        SDL_PIXELFORMAT_INDEX8);
    if (!SDLSurfacePaletted) {
        hiba(SDL_GetError());
        return;
    }
}

static unsigned char* SurfaceBuffer[SCREEN_HEIGHT];
static bool SurfaceLocked = false;

unsigned char** lock_backbuffer(bool flipped) {
    if (SurfaceLocked) {
        hiba("lock_backbuffer SurfaceLocked!");
    }
    SurfaceLocked = true;

    unsigned char* row = (unsigned char*)SDLSurfacePaletted->pixels;
    if (flipped) {
        // Set the row buffer bottom-down
        for (int y = 0; y < SCREEN_HEIGHT; y++) {
            SurfaceBuffer[SCREEN_HEIGHT - 1 - y] = row;
            row += SDLSurfacePaletted->w;
        }
    } else {
        // Set the row buffer top-down
        for (int y = 0; y < SCREEN_HEIGHT; y++) {
            SurfaceBuffer[y] = row;
            row += SDLSurfacePaletted->w;
        }
    }

    return SurfaceBuffer;
}

void unlock_backbuffer() {
    if (!SurfaceLocked) {
        hiba("unlock_backbuffer !SurfaceLocked!");
    }
    SurfaceLocked = false;

    SDL_BlitSurface(SDLSurfacePaletted, NULL, SDLSurfaceMain, NULL);
    SDL_UpdateWindowSurface(SDLWindow);
}

unsigned char** lock_frontbuffer(bool flipped) {
    if (SurfaceLocked) {
        hiba("lock_frontbuffer SurfaceLocked!");
    }

    return lock_backbuffer(flipped);
}

void unlock_frontbuffer() {
    if (!SurfaceLocked) {
        hiba("unlock_frontbuffer !SurfaceLocked!");
    }

    unlock_backbuffer();
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

void handle_events() {
    SDL_Event event;
    while (SDL_PollEvent(&event)) {
        switch (event.type) {
        case SDL_QUIT:
            // Exit request probably sent by user to terminate program
            if (Editorban_dialnak && Valtozott) {
                // Disallow exiting if unsaved changes in editor
                break;
            }
            exit(0);
            break;
        case SDL_WINDOWEVENT:
            // Force editor redraw if focus gained/lost to fix editor sometimes blanking
            switch (event.window.event) {
            case SDL_WINDOWEVENT_FOCUS_GAINED:
                invalidateegesz();
                break;
            case SDL_WINDOWEVENT_FOCUS_LOST:
                invalidateegesz();
                break;
            }
        case SDL_MOUSEBUTTONDOWN:
            if (event.button.button == SDL_BUTTON_LEFT) {
                LeftMouseDown = true;
            }
            if (event.button.button == SDL_BUTTON_RIGHT) {
                RightMouseDown = true;
            }
            break;
        case SDL_MOUSEBUTTONUP:
            if (event.button.button == SDL_BUTTON_LEFT) {
                LeftMouseDown = false;
            }
            if (event.button.button == SDL_BUTTON_RIGHT) {
                RightMouseDown = false;
            }
            break;
        }
    }

    mkw_getDIstate();
    mkw_setkeydown();
}

void fill_key_state(char* buffer) {
    const unsigned char* state = SDL_GetKeyboardState(NULL);
    for (int i = 0; i < MaxKeycode; i++) {
        buffer[i] = state[windows_scancode_table[i]];
    }
}

void hide_cursor() { SDL_ShowCursor(SDL_DISABLE); }
void show_cursor() { SDL_ShowCursor(SDL_ENABLE); }

void get_mouse_position(int* x, int* y) { SDL_GetMouseState(x, y); }
void set_mouse_position(int x, int y) { SDL_WarpMouseInWindow(NULL, x, y); }

bool left_mouse_clicked() {
    handle_events();
    bool click = !LeftMouseDownPrev && LeftMouseDown;
    LeftMouseDownPrev = LeftMouseDown;
    return click;
}

bool right_mouse_clicked() {
    handle_events();
    bool click = !RightMouseDownPrev && RightMouseDown;
    RightMouseDownPrev = RightMouseDown;
    return click;
}

static SDL_AudioDeviceID SDLAudioDevice;
static bool SoundInitialized = false;

static void audio_callback(void* udata, Uint8* stream, int len) {
    callbackhang((short*)stream, len / 2);
}

void init_sound() {
    if (SoundInitialized) {
        hiba("Sound already initialized!");
    }
    SoundInitialized = true;

    SDL_AudioSpec desired_spec;
    memset(&desired_spec, 0, sizeof(desired_spec));
    desired_spec.callback = audio_callback;
    desired_spec.freq = 11025;
    desired_spec.channels = 1;
    desired_spec.samples = 512;
    desired_spec.format = AUDIO_S16LSB;

    if (SDL_InitSubSystem(SDL_INIT_AUDIO) < 0) {
        hiba("Failed to initialize audio subsystem", SDL_GetError());
    }
    SDL_AudioSpec obtained_spec;
    SDLAudioDevice = SDL_OpenAudioDevice(NULL, 0, &desired_spec, &obtained_spec, 0);
    if (SDLAudioDevice == 0) {
        hiba("Failed to open audio device", SDL_GetError());
    }
    if (obtained_spec.format != desired_spec.format) {
        hiba("Failed to get correct audio format");
    }
    Hangenabled = 1;
    SDL_PauseAudioDevice(SDLAudioDevice, 0);
}
