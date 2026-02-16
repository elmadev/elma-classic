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
#include "pic8.h"
#include <directinput/scancodes.h>
#include <algorithm>
#include <SDL.h>
#include <sdl/scancodes_windows.h>

static SDL_Window* SDLWindow = nullptr;
static SDL_Surface* SDLSurfaceMain = nullptr;
static SDL_Surface* SDLSurfacePaletted = nullptr;
static palette* CurrentPalette = nullptr;

static bool LeftMouseDown = false;
static bool RightMouseDown = false;

// Per-frame mouse state for was_left/right_mouse_just_clicked()
static bool LeftMouseDownPrevFrame = false;
static bool RightMouseDownPrevFrame = false;

static int MouseWheelDelta = 0;
static bool CursorRequested = true;

void message_box(const char* text) {
    // As per docs, can be called even before SDL_Init
    // SDLWindow will either be a handle to the window, or nullptr if no parent
    SDL_ShowSimpleMessageBox(SDL_MESSAGEBOX_ERROR, "Message", text, SDLWindow);
}

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
        if (gl_init(SDLWindow, SCREEN_WIDTH, SCREEN_HEIGHT, SDLSurfacePaletted->pitch) != 0) {
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

static void apply_current_palette() {
    if (CurrentPalette) {
        CurrentPalette->set();
    }
}

void platform_apply_fullscreen_mode() {
    if (!SDLWindow) {
        return;
    }

    switch (EolSettings->fullscreen()) {
    case FullscreenMode::Windowed:
        SDL_SetWindowFullscreen(SDLWindow, 0);
        SDL_SetWindowSize(SDLWindow, SCREEN_WIDTH, SCREEN_HEIGHT);
        SDL_SetWindowPosition(SDLWindow, SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED);
        SDL_ShowCursor(CursorRequested ? SDL_ENABLE : SDL_DISABLE);
        break;
    case FullscreenMode::Fullscreen: {
        SDL_DisplayMode desired = {};
        desired.w = SCREEN_WIDTH;
        desired.h = SCREEN_HEIGHT;
        SDL_DisplayMode closest;
        if (!SDL_GetClosestDisplayMode(0, &desired, &closest)) {
            internal_error("No matching display mode found");
            break;
        }
        SDL_SetWindowDisplayMode(SDLWindow, &closest);
        // Snap to the closest supported display mode if the current resolution
        // doesn't match (e.g. stale settings from a different monitor, or an
        // arbitrary windowed resolution). This recurses through update_resolution
        // → platform_resize_window, but terminates because the second call finds
        // closest == SCREEN_WIDTH/HEIGHT.
        if (closest.w != SCREEN_WIDTH || closest.h != SCREEN_HEIGHT) {
            update_resolution(closest.w, closest.h);
        }
        SDL_SetWindowFullscreen(SDLWindow, SDL_WINDOW_FULLSCREEN);
        SDL_ShowCursor(SDL_DISABLE);
        break;
    }
    case FullscreenMode::FullscreenDesktop: {
        SDL_SetWindowFullscreen(SDLWindow, SDL_WINDOW_FULLSCREEN_DESKTOP);
        int w;
        int h;
        SDL_GetWindowSize(SDLWindow, &w, &h);
        if (w != SCREEN_WIDTH || h != SCREEN_HEIGHT) {
            update_resolution(w, h);
        }

        SDL_ShowCursor(SDL_DISABLE);
        break;
    }
    }

    if (EolSettings->renderer() == RendererType::Software) {
        SDLSurfaceMain = SDL_GetWindowSurface(SDLWindow);
    }
}

void platform_init() {
    if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_EVENTS)) {
        internal_error(SDL_GetError());
        return;
    }

    SDL_EventState(SDL_DROPFILE, SDL_DISABLE);
    SDL_EventState(SDL_DROPTEXT, SDL_DISABLE);

    create_window(SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, SCREEN_WIDTH, SCREEN_HEIGHT);
    create_palette_surface();
    initialize_renderer();
    apply_current_palette();
    keyboard::init();
    platform_apply_fullscreen_mode();
}

void platform_resize_window(int width, int height) {
    if (!SDLWindow) {
        internal_error("platform_resize_window no window!");
    }

    if (!is_fullscreen()) {
        // Keep window centered
        int x;
        int y;
        SDL_GetWindowPosition(SDLWindow, &x, &y);
        int dx = SCREEN_WIDTH - width;
        int dy = SCREEN_HEIGHT - height;
        SDL_SetWindowPosition(SDLWindow, x + dx / 2, y + dy / 2);
    }

    SCREEN_WIDTH = width;
    SCREEN_HEIGHT = height;

    if (EolSettings->fullscreen() == FullscreenMode::Fullscreen) {
        SDL_DisplayMode desired = {};
        desired.w = width;
        desired.h = height;
        SDL_DisplayMode closest;
        if (!SDL_GetClosestDisplayMode(0, &desired, &closest)) {
            internal_error("No matching display mode found");
            return;
        }
        if (closest.w != width || closest.h != height) {
            width = closest.w;
            height = closest.h;
            SCREEN_WIDTH = width;
            SCREEN_HEIGHT = height;
            EolSettings->set_screen_width(width);
            EolSettings->set_screen_height(height);
        }

        if (EolSettings->renderer() == RendererType::Software) {
            // Recreate the window in fullscreen software mode to avoid issues
            // with window surfaces not resizing after a display mode change.
            platform_recreate_window();
            return;
        }

        // Cycle fullscreen off and back on to force SDL2 to apply the new
        // display mode (SDL_SetWindowFullscreen is a no-op when already fullscreen).
        SDL_SetWindowFullscreen(SDLWindow, 0);
        SDL_SetWindowDisplayMode(SDLWindow, &closest);
        SDL_SetWindowFullscreen(SDLWindow, SDL_WINDOW_FULLSCREEN);
    } else {
        SDL_SetWindowSize(SDLWindow, width, height);
    }

    SDL_FreeSurface(SDLSurfacePaletted);
    SDLSurfacePaletted = nullptr;
    create_palette_surface();

    if (EolSettings->renderer() == RendererType::OpenGL) {
        if (gl_resize(width, height, SDLSurfacePaletted->pitch) != 0) {
            internal_error("Failed to resize OpenGL renderer");
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

    apply_current_palette();
}

void platform_recreate_window() {
    int x;
    int y;
    SDL_GetWindowPosition(SDLWindow, &x, &y);

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

    create_window(x, y, SCREEN_WIDTH, SCREEN_HEIGHT);
    create_palette_surface();
    initialize_renderer();
    apply_current_palette();
    platform_apply_fullscreen_mode();
}

long long get_milliseconds() { return SDL_GetTicks64(); }

bool has_window() { return SDLWindow != nullptr; }

static bool SurfaceLocked = false;

void lock_backbuffer(pic8& view, bool flipped) {
    if (SurfaceLocked) {
        internal_error("lock_backbuffer SurfaceLocked!");
    }
    SurfaceLocked = true;

    int pitch = SDLSurfacePaletted->pitch;
    int width = SDLSurfacePaletted->w;
    int height = SDLSurfacePaletted->h;
    unsigned char* pixels = (unsigned char*)SDLSurfacePaletted->pixels;
    view.subview(width, height, pixels, pitch, flipped);
}

void unlock_backbuffer() {
    if (!SurfaceLocked) {
        internal_error("unlock_backbuffer !SurfaceLocked!");
    }
    SurfaceLocked = false;

    if (EolSettings->renderer() == RendererType::OpenGL) {
        gl_upload_frame((unsigned char*)SDLSurfacePaletted->pixels, SDLSurfacePaletted->pitch);
        gl_present();
        SDL_GL_SwapWindow(SDLWindow);
    } else {
        SDL_BlitSurface(SDLSurfacePaletted, NULL, SDLSurfaceMain, NULL);
        SDL_UpdateWindowSurface(SDLWindow);
    }
}

void lock_frontbuffer(pic8& view, bool flipped) {
    if (SurfaceLocked) {
        internal_error("lock_frontbuffer SurfaceLocked!");
    }
    lock_backbuffer(view, flipped);
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
    CurrentPalette = this;
    if (EolSettings->renderer() == RendererType::OpenGL) {
        gl_update_palette(data);
    } else {
        SDL_SetPaletteColors(SDLSurfacePaletted->format->palette, (const SDL_Color*)data, 0, 256);
    }
}

void handle_events() {
    keyboard::begin_frame();
    MouseWheelDelta = 0;
    LeftMouseDownPrevFrame = LeftMouseDown;
    RightMouseDownPrevFrame = RightMouseDown;

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

#ifdef __APPLE__
            // macOS exclusive fullscreen captures the display and blocks
            // Cmd+Tab/M/H. Intercept them and minimize the window.
            if (EolSettings->fullscreen() == FullscreenMode::Fullscreen) {
                bool is_cmd = event.key.keysym.mod & KMOD_GUI;
                if (is_cmd && (scancode == SDL_SCANCODE_TAB || scancode == SDL_SCANCODE_M)) {
                    SDL_MinimizeWindow(SDLWindow);
                    break;
                }
                if (is_cmd && scancode == SDL_SCANCODE_H) {
                    SDL_HideWindow(SDLWindow);
                    break;
                }
            }
#endif

            // SDL doesn't generate text input events when Ctrl is held
            // Resolve layout-specific keycodes to support LCtrl search
            if (EolSettings->lctrl_search()) {
                bool is_lctrl_pressed = event.key.keysym.mod & KMOD_LCTRL;
                if (is_lctrl_pressed) {
                    SDL_Keycode sym = SDL_GetKeyFromScancode(event.key.keysym.scancode);
                    if (sym > 0 && sym < 128) {
                        add_char_to_buffer((char)sym);
                    }
                }
            }
            break;
        }
        case SDL_TEXTINPUT:
            add_text_to_buffer(event.text.text);
            break;
        case SDL_MOUSEWHEEL:
            MouseWheelDelta = event.wheel.y > 0 ? 1 : -1;
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

    keyboard::end_frame();
}

void hide_cursor() {
    CursorRequested = false;
    SDL_ShowCursor(SDL_DISABLE);
}

void show_cursor() {
    CursorRequested = true;
    if (!is_fullscreen()) {
        SDL_ShowCursor(SDL_ENABLE);
    }
}

void get_mouse_position(int* x, int* y) { SDL_GetMouseState(x, y); }
void set_mouse_position(int x, int y) { SDL_WarpMouseInWindow(NULL, x, y); }

bool was_left_mouse_just_clicked() { return LeftMouseDown && !LeftMouseDownPrevFrame; }

bool was_right_mouse_just_clicked() { return RightMouseDown && !RightMouseDownPrevFrame; }

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

std::string get_clipboard_text() {
    char* text = SDL_GetClipboardText();
    if (!text) {
        return {};
    }
    std::string result(text);
    SDL_free(text);
    return result;
}

bool is_shortcut_modifier_down() {
    SDL_Keymod mod = SDL_GetModState();
#ifdef __APPLE__
    return (mod & KMOD_GUI) != 0;
#else
    return (mod & KMOD_CTRL) != 0;
#endif
}

bool was_key_down(DikScancode code) {
    SDL_Scancode sdl_code = windows_scancode_table[code];
    return keyboard::was_down(sdl_code);
}

int get_mouse_wheel_delta() { return MouseWheelDelta; }

bool is_fullscreen() {
    Uint32 flags = SDL_GetWindowFlags(SDLWindow);
    return flags & (SDL_WINDOW_FULLSCREEN | SDL_WINDOW_FULLSCREEN_DESKTOP);
}

std::vector<std::pair<int, int>> platform_get_display_modes() {
    std::vector<std::pair<int, int>> modes;
    int count = SDL_GetNumDisplayModes(0);
    for (int i = 0; i < count; i++) {
        SDL_DisplayMode mode;
        if (SDL_GetDisplayMode(0, i, &mode) == 0) {
            std::pair<int, int> res = {mode.w, mode.h};
            if (std::find(modes.begin(), modes.end(), res) == modes.end()) {
                modes.push_back(res);
            }
        }
    }
    std::sort(modes.begin(), modes.end());
    return modes;
}

std::pair<int, int> platform_get_desktop_resolution() {
    SDL_DisplayMode mode;
    if (SDL_GetDesktopDisplayMode(0, &mode) == 0) {
        return {mode.w, mode.h};
    }
    return {SCREEN_WIDTH, SCREEN_HEIGHT};
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
        internal_error(std::string("Failed to initialize audio subsystem:\n") + SDL_GetError());
    }
    SDL_AudioSpec obtained_spec;
    SDLAudioDevice = SDL_OpenAudioDevice(NULL, 0, &desired_spec, &obtained_spec, 0);
    if (SDLAudioDevice == 0) {
        internal_error(std::string("Failed to open audio device:\n") + SDL_GetError());
    }
    if (obtained_spec.format != desired_spec.format) {
        internal_error("Failed to get correct audio format");
    }
    SDL_PauseAudioDevice(SDLAudioDevice, 0);
}
