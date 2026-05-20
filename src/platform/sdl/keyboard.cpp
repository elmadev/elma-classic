#include "platform/sdl/keyboard.h"

#include <cstring>

// Map numpad keys to navigation equivalents when NumLock is off
static constexpr struct {
    SDL_Scancode numpad;
    SDL_Scancode nav;
} NUMPAD_NAV_MAP[] = {
    {SDL_SCANCODE_KP_ENTER, SDL_SCANCODE_RETURN}, {SDL_SCANCODE_KP_8, SDL_SCANCODE_UP},
    {SDL_SCANCODE_KP_2, SDL_SCANCODE_DOWN},       {SDL_SCANCODE_KP_4, SDL_SCANCODE_LEFT},
    {SDL_SCANCODE_KP_6, SDL_SCANCODE_RIGHT},      {SDL_SCANCODE_KP_9, SDL_SCANCODE_PAGEUP},
    {SDL_SCANCODE_KP_3, SDL_SCANCODE_PAGEDOWN},   {SDL_SCANCODE_KP_PERIOD, SDL_SCANCODE_DELETE},
};

namespace keyboard {

static const Uint8* SdlLive = nullptr;
static Uint8 Current[SDL_NUM_SCANCODES];
static Uint8 Previous[SDL_NUM_SCANCODES];
static bool WasDown[SDL_NUM_SCANCODES];
static bool NumpadNavEnabled = true;

void init() {
    SdlLive = SDL_GetKeyboardState(nullptr);

    memset(Previous, 0, sizeof(Previous));
}

void begin_frame() {
    memcpy(Previous, Current, sizeof(Previous));
    memset(WasDown, 0, sizeof(WasDown));
}

void end_frame() {
    memcpy(Current, SdlLive, sizeof(Current));

#ifdef __APPLE__
    // macos has no numlock key so we always force numpad navigation off
    bool numpad_nav = false;
#else
    // Map numpad keys to navigation equivalents when in navigation mode.
    // On Windows/Linux the behavior depends on the numlock state and global state.
    // We want gameplay inputs to always fire regardless of numlock state and
    // other menus fire the respective key. This is because when in the editor
    // pressing numpad 4 should either write 4 or navigate left, without this
    // we would fire both inputs
    bool numpad_nav = NumpadNavEnabled && !(SDL_GetModState() & KMOD_NUM);
#endif
    for (auto& [numpad, nav] : NUMPAD_NAV_MAP) {
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

void set_numpad_nav(bool enabled) { NumpadNavEnabled = enabled; }

bool is_down(SDL_Scancode sdl_code) { return Current[sdl_code] != 0; }

bool was_just_pressed(SDL_Scancode sdl_code) {
    return Current[sdl_code] != 0 && Previous[sdl_code] == 0;
}

bool was_down(SDL_Scancode sdl_code) { return WasDown[sdl_code]; }

} // namespace keyboard
