#include "platform_impl.h"
#include "editor_dialog.h"
#include "eol_settings.h"
#include "EDITUJ.H"
#include "sound_engine.h"
#include "keys.h"
#include "platform_sdl_keyboard.h"
#include "gl_renderer.h"
#include "main.h"
#include "M_PIC.H"
#include <directinput/scancodes.h>
#include <SDL.h>
#include <sdl/scancodes_windows.h>

static SDL_Window* SDLWindow = nullptr;
static SDL_Surface* SDLSurfaceMain = nullptr;
static SDL_Surface* SDLSurfacePaletted = nullptr;

static bool LeftMouseDownPrev = false;
static bool RightMouseDownPrev = false;
static bool LeftMouseDown = false;
static bool RightMouseDown = false;

void message_box(const char* text) {
    // As per docs, can be called even before SDL_Init
    // SDLWindow will either be a handle to the window, or nullptr if no parent
    SDL_ShowSimpleMessageBox(SDL_MESSAGEBOX_ERROR, "Message", text, SDLWindow);
}

static unsigned char** SurfaceBuffer;

static void create_window(int window_pos_x, int window_pos_y, int width, int height) {
    if (EolSettings->renderer() == RendererType::OpenGL) {
        SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_CORE);
        SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 4);
        SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 1);
    }

    int window_flags = EolSettings->renderer() == RendererType::OpenGL ? SDL_WINDOW_OPENGL : 0;

    SDLWindow =
        SDL_CreateWindow("Elasto Mania", window_pos_x, window_pos_y, width, height, window_flags);
    if (!SDLWindow) {
        internal_error(SDL_GetError());
        return;
    }
}

static void initialize_renderer() {
    if (EolSettings->renderer() == RendererType::OpenGL) {
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

void platform_init() {
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
    keyboard::init();
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

    if (EolSettings->renderer() == RendererType::OpenGL) {
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
    if (EolSettings->renderer() == RendererType::OpenGL) {
        gl_update_palette(data);
    } else {
        SDL_SetPaletteColors(SDLSurfacePaletted->format->palette, (const SDL_Color*)data, 0, 256);
    }
}

void handle_events() {
    keyboard::begin_frame();

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
            keyboard::record_key_down(scancode);
            Keycode keycode = keyboard::keycode_for(scancode);

            // SDL doesn't generate text input events when Ctrl is held
            // Resolve layout-specific keycodes for unmapped keys to support LCtrl search
            if (EolSettings->lctrl_search() && keycode == SDL_SCANCODE_UNKNOWN) {
                bool is_lctrl_pressed = event.key.keysym.mod & KMOD_LCTRL;
                if (is_lctrl_pressed) {
                    SDL_Keycode sym = SDL_GetKeyFromScancode(scancode);
                    if (sym > 0 && sym < 128) {
                        keycode = (Keycode)sym;
                    }
                }
            }

            if (keycode == SDL_SCANCODE_UNKNOWN) {
                break; // Not a control mapping - delivered through text input events.
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
        case SDL_TEXTINPUT:
            add_text_to_buffer(event.text.text);
            break;
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

    return keyboard::is_down(sdl_code);
}

bool was_key_just_pressed(DikScancode code) {
    SDL_Scancode sdl_code = windows_scancode_table[code];
    return keyboard::was_just_pressed(sdl_code);
}

DikScancode get_any_key_just_pressed() {
    for (int i = 0; i < MaxKeycode; i++) {
        if (was_key_just_pressed(i)) {
            return i;
        }
    }

    return DIK_UNKNOWN;
}

bool was_key_down(DikScancode code) {
    SDL_Scancode sdl_code = windows_scancode_table[code];
    return keyboard::was_down(sdl_code);
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
