#include "platform/scancode.h"
#include "main.h"
#include "platform/sdl/keyboard.h"
#include <sdl/scancodes_windows.h>

bool is_key_down(DikScancode code) {
    if (code < 0 || code >= MaxKeycode) {
        internal_error("code out of range in is_key_down()!");
    }

    SDL_Scancode sdl_code = windows_scancode_table[code];

    return keyboard::is_down(sdl_code);
}

bool was_key_just_pressed(DikScancode code) {
    if (code < 0 || code >= MaxKeycode) {
        internal_error("code out of range in was_key_just_pressed()!");
    }

    SDL_Scancode sdl_code = windows_scancode_table[code];
    return keyboard::was_just_pressed(sdl_code);
}

bool was_key_just_pressed(combo_scancode code) {
    if (!was_key_just_pressed(code.key)) {
        return false;
    }

    if (code.modifier != DIK_NONE) {
        return is_key_down(code.modifier);
    }

    for (DikScancode modifier : MODIFIERS) {
        if (is_key_down(modifier)) {
            return false;
        }
    }

    return true;
}

DikScancode get_any_key_just_pressed() {
    for (int i = 0; i < MaxKeycode; i++) {
        if (was_key_just_pressed(i)) {
            return i;
        }
    }

    return DIK_NONE;
}

bool was_key_down(DikScancode code) {
    SDL_Scancode sdl_code = windows_scancode_table[code];
    return keyboard::was_down(sdl_code);
}

bool is_shortcut_modifier_down() {
    SDL_Keymod mod = SDL_GetModState();
#ifdef __APPLE__
    return (mod & KMOD_GUI) != 0;
#else
    return (mod & KMOD_CTRL) != 0;
#endif
}
