#include "platform_sdl_keyboard.h"

#include <cstring>

namespace keyboard {

static const Uint8* SdlLive = nullptr;
static Uint8 Previous[SDL_NUM_SCANCODES];
static Keycode SdlToKeycode[SDL_NUM_SCANCODES];
static bool WasDown[SDL_NUM_SCANCODES];

void init() {
    SdlLive = SDL_GetKeyboardState(nullptr);

    memset(Previous, 0, sizeof(Previous));

    SdlToKeycode[SDL_SCANCODE_ESCAPE] = KEY_ESC;
    SdlToKeycode[SDL_SCANCODE_RETURN] = KEY_ENTER;
    SdlToKeycode[SDL_SCANCODE_KP_ENTER] = KEY_ENTER;

    SdlToKeycode[SDL_SCANCODE_UP] = KEY_UP;
    SdlToKeycode[SDL_SCANCODE_KP_8] = KEY_UP;
    SdlToKeycode[SDL_SCANCODE_DOWN] = KEY_DOWN;
    SdlToKeycode[SDL_SCANCODE_KP_2] = KEY_DOWN;
    SdlToKeycode[SDL_SCANCODE_LEFT] = KEY_LEFT;
    SdlToKeycode[SDL_SCANCODE_KP_4] = KEY_LEFT;
    SdlToKeycode[SDL_SCANCODE_RIGHT] = KEY_RIGHT;
    SdlToKeycode[SDL_SCANCODE_KP_6] = KEY_RIGHT;

    SdlToKeycode[SDL_SCANCODE_PAGEUP] = KEY_PGUP;
    SdlToKeycode[SDL_SCANCODE_KP_9] = KEY_PGUP;
    SdlToKeycode[SDL_SCANCODE_PAGEDOWN] = KEY_PGDOWN;
    SdlToKeycode[SDL_SCANCODE_KP_3] = KEY_PGDOWN;

    SdlToKeycode[SDL_SCANCODE_DELETE] = KEY_DEL;
    SdlToKeycode[SDL_SCANCODE_KP_PERIOD] = KEY_DEL;
    SdlToKeycode[SDL_SCANCODE_BACKSPACE] = KEY_BACKSPACE;

    SdlToKeycode[SDL_SCANCODE_MINUS] = KEY_LEFT;
    SdlToKeycode[SDL_SCANCODE_KP_MINUS] = KEY_LEFT;
    SdlToKeycode[SDL_SCANCODE_EQUALS] = KEY_RIGHT;
    SdlToKeycode[SDL_SCANCODE_KP_PLUS] = KEY_RIGHT;
}

void begin_frame() {
    memcpy(Previous, SdlLive, sizeof(Uint8) * SDL_NUM_SCANCODES);
    memset(WasDown, 0, sizeof(WasDown));
}

void record_key_down(SDL_Scancode scancode) { WasDown[scancode] = true; }

bool is_down(SDL_Scancode sdl_code) { return SdlLive[sdl_code] != 0; }

bool was_just_pressed(SDL_Scancode sdl_code) {
    return SdlLive[sdl_code] != 0 && Previous[sdl_code] == 0;
}

bool was_down(SDL_Scancode sdl_code) { return WasDown[sdl_code]; }

Keycode keycode_for(SDL_Scancode scancode) { return SdlToKeycode[scancode]; }

} // namespace keyboard
