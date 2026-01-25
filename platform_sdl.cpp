#include "platform_impl.h"
#include "editor_dialog.h"
#include "eol_settings.h"
#include "EDITUJ.H"
#include "sound_engine.h"
#include "keys.h"
#include "gl_renderer.h"
#include "main.h"
#include "M_PIC.H"
#include <SDL.h>
#include <sdl/scancodes_windows.h>

static SDL_Window* SDLWindow = nullptr;
static SDL_Surface* SDLSurfaceMain = nullptr;
static SDL_Surface* SDLSurfacePaletted = nullptr;

static RendererType CurrentRenderer;

static bool LeftMouseDownPrev = false;
static bool RightMouseDownPrev = false;
static bool LeftMouseDown = false;
static bool RightMouseDown = false;

// SDL keyboard state and mappings
static const Uint8* SDLKeyState = nullptr;
static Keycode SDLToKeycode[SDL_NUM_SCANCODES];

void message_box(const char* text) {
    // As per docs, can be called even before SDL_Init
    // SDLWindow will either be a handle to the window, or nullptr if no parent
    SDL_ShowSimpleMessageBox(SDL_MESSAGEBOX_ERROR, "Message", text, SDLWindow);
}

static unsigned char** SurfaceBuffer;

static void create_window(int window_pos_x, int window_pos_y, int width, int height) {
    if (CurrentRenderer == RendererType::OpenGL) {
        SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_CORE);
        SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 4);
        SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 1);
    }

    int window_flags = CurrentRenderer == RendererType::OpenGL ? SDL_WINDOW_OPENGL : 0;

    SDLWindow =
        SDL_CreateWindow("Elasto Mania", window_pos_x, window_pos_y, width, height, window_flags);
    if (!SDLWindow) {
        internal_error(SDL_GetError());
        return;
    }
}

static void initialize_renderer() {
    if (CurrentRenderer == RendererType::OpenGL) {
        if (gl_init(SDLWindow, SCREEN_WIDTH, SCREEN_HEIGHT) != 0) {
            internal_error("Failed to initialize OpenGL renderer");
            return;
        }

        SDLSurfaceMain = nullptr; // Not used in GL mode
    } else {
        SDLSurfaceMain = SDL_GetWindowSurface(SDLWindow);
        if (!SDLSurfaceMain) {
            internal_error(SDL_GetError());
            return;
        }
    }
}

static void create_palette_surface() {
    SDLSurfacePaletted =
        SDL_CreateRGBSurfaceWithFormat(0, SCREEN_WIDTH, SCREEN_HEIGHT, 0, SDL_PIXELFORMAT_INDEX8);
    if (!SDLSurfacePaletted) {
        internal_error(SDL_GetError());
        return;
    }
}

static void initialize_keyboard_mappings() {
    // Initialize keyboard state and mappings
    SDLKeyState = SDL_GetKeyboardState(nullptr);

    // Map SDL scancodes to Keycodes
    for (int i = 0; i < SDL_NUM_SCANCODES; i++) {
        SDLToKeycode[i] = SDL_SCANCODE_UNKNOWN;
    }

    SDLToKeycode[SDL_SCANCODE_1] = '1';
    SDLToKeycode[SDL_SCANCODE_2] = '2';
    SDLToKeycode[SDL_SCANCODE_3] = '3';
    SDLToKeycode[SDL_SCANCODE_4] = '4';
    SDLToKeycode[SDL_SCANCODE_5] = '5';
    SDLToKeycode[SDL_SCANCODE_6] = '6';
    SDLToKeycode[SDL_SCANCODE_7] = '7';
    SDLToKeycode[SDL_SCANCODE_8] = '8';
    SDLToKeycode[SDL_SCANCODE_9] = '9';
    SDLToKeycode[SDL_SCANCODE_0] = '0';

    SDLToKeycode[SDL_SCANCODE_A] = 'a';
    SDLToKeycode[SDL_SCANCODE_B] = 'b';
    SDLToKeycode[SDL_SCANCODE_C] = 'c';
    SDLToKeycode[SDL_SCANCODE_D] = 'd';
    SDLToKeycode[SDL_SCANCODE_E] = 'e';
    SDLToKeycode[SDL_SCANCODE_F] = 'f';
    SDLToKeycode[SDL_SCANCODE_G] = 'g';
    SDLToKeycode[SDL_SCANCODE_H] = 'h';
    SDLToKeycode[SDL_SCANCODE_I] = 'i';
    SDLToKeycode[SDL_SCANCODE_J] = 'j';
    SDLToKeycode[SDL_SCANCODE_K] = 'k';
    SDLToKeycode[SDL_SCANCODE_L] = 'l';
    SDLToKeycode[SDL_SCANCODE_M] = 'm';
    SDLToKeycode[SDL_SCANCODE_N] = 'n';
    SDLToKeycode[SDL_SCANCODE_O] = 'o';
    SDLToKeycode[SDL_SCANCODE_P] = 'p';
    SDLToKeycode[SDL_SCANCODE_Q] = 'q';
    SDLToKeycode[SDL_SCANCODE_R] = 'r';
    SDLToKeycode[SDL_SCANCODE_S] = 's';
    SDLToKeycode[SDL_SCANCODE_T] = 't';
    SDLToKeycode[SDL_SCANCODE_U] = 'u';
    SDLToKeycode[SDL_SCANCODE_V] = 'v';
    SDLToKeycode[SDL_SCANCODE_W] = 'w';
    SDLToKeycode[SDL_SCANCODE_X] = 'x';
    SDLToKeycode[SDL_SCANCODE_Y] = 'y';
    SDLToKeycode[SDL_SCANCODE_Z] = 'z';

    SDLToKeycode[SDL_SCANCODE_SPACE] = ' ';
    SDLToKeycode[SDL_SCANCODE_PERIOD] = '.';

    SDLToKeycode[SDL_SCANCODE_ESCAPE] = KEY_ESC;
    SDLToKeycode[SDL_SCANCODE_RETURN] = KEY_ENTER;
    SDLToKeycode[SDL_SCANCODE_KP_ENTER] = KEY_ENTER; // KP = Keypad

    SDLToKeycode[SDL_SCANCODE_UP] = KEY_UP;
    SDLToKeycode[SDL_SCANCODE_KP_8] = KEY_UP;
    SDLToKeycode[SDL_SCANCODE_DOWN] = KEY_DOWN;
    SDLToKeycode[SDL_SCANCODE_KP_2] = KEY_DOWN;
    SDLToKeycode[SDL_SCANCODE_LEFT] = KEY_LEFT;
    SDLToKeycode[SDL_SCANCODE_KP_4] = KEY_LEFT;
    SDLToKeycode[SDL_SCANCODE_RIGHT] = KEY_RIGHT;
    SDLToKeycode[SDL_SCANCODE_KP_6] = KEY_RIGHT;

    SDLToKeycode[SDL_SCANCODE_PAGEUP] = KEY_PGUP;
    SDLToKeycode[SDL_SCANCODE_KP_9] = KEY_PGUP;
    SDLToKeycode[SDL_SCANCODE_PAGEDOWN] = KEY_PGDOWN;
    SDLToKeycode[SDL_SCANCODE_KP_3] = KEY_PGDOWN;

    SDLToKeycode[SDL_SCANCODE_DELETE] = KEY_DEL;
    SDLToKeycode[SDL_SCANCODE_KP_PERIOD] = KEY_DEL;
    SDLToKeycode[SDL_SCANCODE_BACKSPACE] = KEY_BACKSPACE;

    SDLToKeycode[SDL_SCANCODE_MINUS] = KEY_LEFT;
    SDLToKeycode[SDL_SCANCODE_KP_MINUS] = KEY_LEFT;
    SDLToKeycode[SDL_SCANCODE_EQUALS] = KEY_RIGHT;
    SDLToKeycode[SDL_SCANCODE_KP_PLUS] = KEY_RIGHT;
}

void platform_init() {
    CurrentRenderer = EolSettings->renderer();

    if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_EVENTS)) {
        internal_error(SDL_GetError());
        return;
    }

    SDL_EventState(SDL_DROPFILE, SDL_DISABLE);
    SDL_EventState(SDL_DROPTEXT, SDL_DISABLE);

    SurfaceBuffer = new unsigned char*[SCREEN_HEIGHT];

    create_window(SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, SCREEN_WIDTH, SCREEN_HEIGHT);
    initialize_renderer();
    create_palette_surface();
    initialize_keyboard_mappings();
}

void platform_recreate_window() {
    int x;
    int y;
    SDL_GetWindowPosition(SDLWindow, &x, &y);

    int width;
    int height;
    SDL_GetWindowSize(SDLWindow, &width, &height);

    gl_cleanup();

    if (SDLSurfacePaletted) {
        SDL_FreeSurface(SDLSurfacePaletted);
        SDLSurfacePaletted = nullptr;
    }

    if (SDLSurfaceMain) {
        SDL_DestroyWindowSurface(SDLWindow);
        SDLSurfaceMain = nullptr;
    }

    SDL_DestroyWindow(SDLWindow);
    SDLWindow = nullptr;

    create_window(x, y, width, height);
    initialize_renderer();
    create_palette_surface();
}

long long get_milliseconds() { return SDL_GetTicks64(); }

bool has_window() { return SDLWindow != nullptr; }

static bool SurfaceLocked = false;

unsigned char** lock_backbuffer(bool flipped) {
    if (SurfaceLocked) {
        internal_error("lock_backbuffer SurfaceLocked!");
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
        internal_error("unlock_backbuffer !SurfaceLocked!");
    }
    SurfaceLocked = false;

    if (CurrentRenderer == RendererType::OpenGL) {
        gl_upload_frame((unsigned char*)SDLSurfacePaletted->pixels);
        gl_present();
        SDL_GL_SwapWindow(SDLWindow);
    } else {
        SDL_BlitSurface(SDLSurfacePaletted, NULL, SDLSurfaceMain, NULL);
        SDL_UpdateWindowSurface(SDLWindow);
    }
}

unsigned char** lock_frontbuffer(bool flipped) {
    if (SurfaceLocked) {
        internal_error("lock_frontbuffer SurfaceLocked!");
    }

    return lock_backbuffer(flipped);
}

void unlock_frontbuffer() {
    if (!SurfaceLocked) {
        internal_error("unlock_frontbuffer !SurfaceLocked!");
    }

    unlock_backbuffer();
}

palette::palette(unsigned char* palette_data) {
    SDL_Color* pal = new SDL_Color[256];
    for (int i = 0; i < 256; i++) {
        pal[i].r = palette_data[3 * i];
        pal[i].g = palette_data[3 * i + 1];
        pal[i].b = palette_data[3 * i + 2];
        pal[i].a = 0xFF;
    }
    data = (void*)pal;
}

palette::~palette() { delete[] (SDL_Color*)data; }

void palette::set() {
    if (CurrentRenderer == RendererType::OpenGL) {
        gl_update_palette(data);
    } else {
        SDL_SetPaletteColors(SDLSurfacePaletted->format->palette, (const SDL_Color*)data, 0, 256);
    }
}

void handle_events() {
    SDL_Event event;
    while (SDL_PollEvent(&event)) {
        switch (event.type) {
        case SDL_QUIT:
            // Exit request probably sent by user to terminate program
            if (InEditor && Valtozott) {
                // Disallow exiting if unsaved changes in editor
                break;
            }
            quit();
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
            break;
        case SDL_KEYDOWN: {
            SDL_Scancode scancode = event.key.keysym.scancode;
            Keycode keycode = SDLToKeycode[scancode];
            if (keycode == SDL_SCANCODE_UNKNOWN) {
                break; // Unmapped key
            }

            // Handle shift for letters
            if (keycode >= 'a' && keycode <= 'z') {
                if (SDLKeyState[SDL_SCANCODE_LSHIFT] || SDLKeyState[SDL_SCANCODE_RSHIFT]) {
                    keycode = keycode + 'A' - 'a';
                }
            }

            if (event.key.repeat) {
                bool allow_repeat = (keycode != KEY_ESC && keycode != KEY_ENTER);
                if (!allow_repeat) {
                    break;
                }
            }

            add_key_to_buffer(keycode);
            break;
        }
        case SDL_MOUSEWHEEL:
            if (event.wheel.y > 0) {
                add_key_to_buffer(KEY_UP);
            } else if (event.wheel.y < 0) {
                add_key_to_buffer(KEY_DOWN);
            }
            break;
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

bool is_key_down(DikScancode code) {
    if (code < 0 || code >= MaxKeycode) {
        internal_error("code out of range in is_key_down()!");
        return false;
    }

    SDL_Scancode sdl_code = windows_scancode_table[code];

    return SDLKeyState[sdl_code] != 0;
}

bool is_ctrl_alt_down() {
    return (SDLKeyState[SDL_SCANCODE_LCTRL] || SDLKeyState[SDL_SCANCODE_RCTRL]) &&
           (SDLKeyState[SDL_SCANCODE_LALT] || SDLKeyState[SDL_SCANCODE_RALT]);
}

bool is_fullscreen() {
    Uint32 flags = SDL_GetWindowFlags(SDLWindow);
    return flags & SDL_WINDOW_FULLSCREEN;
}

static SDL_AudioDeviceID SDLAudioDevice;
static bool SDLSoundInitialized = false;

static void audio_callback(void* udata, Uint8* stream, int len) {
    sound_mixer((short*)stream, len / 2);
}

void init_sound() {
    if (SDLSoundInitialized) {
        internal_error("Sound already initialized!");
    }
    SDLSoundInitialized = true;

    SDL_AudioSpec desired_spec;
    memset(&desired_spec, 0, sizeof(desired_spec));
    desired_spec.callback = audio_callback;
    desired_spec.freq = 11025;
    desired_spec.channels = 1;
    desired_spec.samples = 512;
    desired_spec.format = AUDIO_S16LSB;

    if (SDL_InitSubSystem(SDL_INIT_AUDIO) < 0) {
        internal_error("Failed to initialize audio subsystem", SDL_GetError());
    }
    SDL_AudioSpec obtained_spec;
    SDLAudioDevice = SDL_OpenAudioDevice(NULL, 0, &desired_spec, &obtained_spec, 0);
    if (SDLAudioDevice == 0) {
        internal_error("Failed to open audio device", SDL_GetError());
    }
    if (obtained_spec.format != desired_spec.format) {
        internal_error("Failed to get correct audio format");
    }
    SDL_PauseAudioDevice(SDLAudioDevice, 0);
}
