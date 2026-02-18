#include "platform_sdl_keyboard.h"

#include <cstring>

// Map numpad keys to navigation equivalents when NumLock is off
static constexpr struct {
    SDL_Scancode numpad;
    SDL_Scancode nav;
    Keycode keycode;
} NUMPAD_NAV_MAP[] = {
    {SDL_SCANCODE_KP_ENTER, SDL_SCANCODE_RETURN, KEY_ENTER},
    {SDL_SCANCODE_KP_8, SDL_SCANCODE_UP, KEY_UP},
    {SDL_SCANCODE_KP_2, SDL_SCANCODE_DOWN, KEY_DOWN},
    {SDL_SCANCODE_KP_4, SDL_SCANCODE_LEFT, KEY_LEFT},
    {SDL_SCANCODE_KP_6, SDL_SCANCODE_RIGHT, KEY_RIGHT},
    {SDL_SCANCODE_KP_9, SDL_SCANCODE_PAGEUP, KEY_PGUP},
    {SDL_SCANCODE_KP_3, SDL_SCANCODE_PAGEDOWN, KEY_PGDOWN},
    {SDL_SCANCODE_KP_PERIOD, SDL_SCANCODE_DELETE, KEY_DEL},
};

namespace keyboard {

static const Uint8* SdlLive = nullptr;
static Uint8 Current[SDL_NUM_SCANCODES];
static Uint8 Previous[SDL_NUM_SCANCODES];
static Keycode SdlToKeycode[SDL_NUM_SCANCODES];
static bool WasDown[SDL_NUM_SCANCODES];

void init() {
    SdlLive = SDL_GetKeyboardState(nullptr);

    memset(Previous, 0, sizeof(Previous));

    SdlToKeycode[SDL_SCANCODE_ESCAPE] = KEY_ESC;
    SdlToKeycode[SDL_SCANCODE_RETURN] = KEY_ENTER;

    SdlToKeycode[SDL_SCANCODE_UP] = KEY_UP;
    SdlToKeycode[SDL_SCANCODE_DOWN] = KEY_DOWN;
    SdlToKeycode[SDL_SCANCODE_LEFT] = KEY_LEFT;
    SdlToKeycode[SDL_SCANCODE_RIGHT] = KEY_RIGHT;

    SdlToKeycode[SDL_SCANCODE_PAGEUP] = KEY_PGUP;
    SdlToKeycode[SDL_SCANCODE_PAGEDOWN] = KEY_PGDOWN;

    SdlToKeycode[SDL_SCANCODE_DELETE] = KEY_DEL;
    SdlToKeycode[SDL_SCANCODE_BACKSPACE] = KEY_BACKSPACE;

    SdlToKeycode[SDL_SCANCODE_MINUS] = KEY_LEFT;
    SdlToKeycode[SDL_SCANCODE_KP_MINUS] = KEY_LEFT;
    SdlToKeycode[SDL_SCANCODE_EQUALS] = KEY_RIGHT;
    SdlToKeycode[SDL_SCANCODE_KP_PLUS] = KEY_RIGHT;

    for (auto& [numpad, nav, keycode] : NUMPAD_NAV_MAP) {
        SdlToKeycode[numpad] = keycode;
    }
}

void begin_frame() {
    memcpy(Previous, Current, sizeof(Previous));
    memset(WasDown, 0, sizeof(WasDown));
}

void end_frame() {
    memcpy(Current, SdlLive, sizeof(Current));

    // Map numpad keys to navigation equivalents when in navigation mode.
    // On macOS, KMOD_NUM is unreliable (SDL never initializes it from OS state),
    // so we always map numpad keys to text input — macOS has no NumLock key anyway.
#ifdef __APPLE__
    bool numpad_nav = false;
#else
    bool numpad_nav = !(SDL_GetModState() & KMOD_NUM);
#endif
    for (auto& [numpad, nav, keycode] : NUMPAD_NAV_MAP) {
        // KP_ENTER always maps to Return regardless of NumLock
        if (numpad == SDL_SCANCODE_KP_ENTER || numpad_nav) {
            if (Current[numpad]) {
                Current[nav] = 1;
            }
            if (WasDown[numpad]) {
                WasDown[nav] = true;
            }
        }
    }
}

void record_key_down(SDL_Scancode scancode) { WasDown[scancode] = true; }

bool is_down(SDL_Scancode sdl_code) { return Current[sdl_code] != 0; }

bool was_just_pressed(SDL_Scancode sdl_code) {
    return Current[sdl_code] != 0 && Previous[sdl_code] == 0;
}

bool was_down(SDL_Scancode sdl_code) { return WasDown[sdl_code]; }

Keycode keycode_for(SDL_Scancode scancode) { return SdlToKeycode[scancode]; }

} // namespace keyboard
