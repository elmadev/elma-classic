#include "platform/scancode.h"
#include "main.h"
#include "platform/sdl/keyboard.h"
#include <SDL_keyboard.h>
#include <SDL_keycode.h>
#include <algorithm>
#include <format>
#include <vendor/sdl/scancodes_windows.h>

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

bool is_paste_modifier_down() {
    SDL_Keymod mod = SDL_GetModState();
#ifdef __APPLE__
    return (mod & KMOD_GUI) != 0;
#else
    return (mod & KMOD_CTRL) != 0 && (mod & KMOD_SHIFT) != 0;
#endif
}

std::string dik_to_string(DikScancode keycode) {
    switch (keycode) {
    case DIK_NONE:
        return "NONE";
    case DIK_RETURN:
        return "ENTER";
    case DIK_LCONTROL:
        return "L CTRL";
    case DIK_APOSTROPHE:
        return "\""; // The character ' does not exist in menu.abc so cannot currently be used
    case DIK_LSHIFT:
        return "L SHIFT";
    case DIK_RSHIFT:
        return "R SHIFT";
    case DIK_MULTIPLY:
        return "PAD *";
    case DIK_LMENU:
        return "L ALT";
    case DIK_SPACE:
        return "SPACEBAR";
    case DIK_CAPITAL:
        return "CAPS LOCK";
    case DIK_NUMLOCK:
        return "NUM LOCK";
    case DIK_SCROLL:
        return "SCROLL LOCK";
    case DIK_NUMPAD7:
        return "PAD HOME";
    case DIK_NUMPAD8:
        return "PAD UP";
    case DIK_NUMPAD9:
        return "PAD PGUP";
    case DIK_SUBTRACT:
        return "PAD -";
    case DIK_NUMPAD4:
        return "PAD LEFT";
    case DIK_NUMPAD5:
        return "PAD 5";
    case DIK_NUMPAD6:
        return "PAD RIGHT";
    case DIK_ADD:
        return "PAD +";
    case DIK_NUMPAD1:
        return "PAD END";
    case DIK_NUMPAD2:
        return "PAD DOWN";
    case DIK_NUMPAD3:
        return "PAD PGDOWN";
    case DIK_NUMPAD0:
        return "PAD INS";
    case DIK_DECIMAL:
        return "PAD DEL";
    case DIK_KANA:
        return "KANA";
    case DIK_CONVERT:
        return "CONVERT";
    case DIK_NOCONVERT:
        return "NOCONVERT";
    case DIK_YEN:
        return "YEN";
    case DIK_NUMPADEQUALS:
        return "PAD =";
    case DIK_PREVTRACK:
        return "CIRCUMFLEX";
    case DIK_AT:
        return "AT";
    case DIK_COLON:
        return "COLON";
    case DIK_UNDERLINE:
        return "UNDERLINE";
    case DIK_KANJI:
        return "KANJI";
    case DIK_STOP:
        return "STOP";
    case DIK_AX:
        return "AX";
    case DIK_UNLABELED:
        return "UNLABELED";
    case DIK_NUMPADENTER:
        return "PAD ENTER";
    case DIK_RCONTROL:
        return "R CTRL";
    case DIK_NUMPADCOMMA:
        return "COMMA";
    case DIK_DIVIDE:
        return "PAD /";
    case DIK_RMENU:
        return "R ALT";
    case DIK_UP:
        return "UP ARROW";
    case DIK_PRIOR:
        return "PAGE UP";
    case DIK_LEFT:
        return "LEFT ARROW";
    case DIK_RIGHT:
        return "RIGHT ARROW";
    case DIK_DOWN:
        return "DOWN ARROW";
    case DIK_NEXT:
        return "PAGE DOWN";
    case DIK_LWIN:
        return "L WIN";
    case DIK_RWIN:
        return "R WIN";
    case DIK_APPS:
        return "APPLICATION";
    }

    // Use SDL key names.
    //
    // SDL2 key-name table:
    // https://github.com/libsdl-org/SDL/blob/release-2.32.x/src/events/SDL_keyboard.c#L56
    //
    // In SDL3, the table has moved to:
    // https://github.com/libsdl-org/SDL/blob/main/src/events/SDL_keymap.c
    //
    // Platform-specific names can be identified by searching for
    // SDL_SetScancodeName. For example:
    //   - DIK_APPS maps to "Menu" on Windows vs. "Application" elsewhere.
    //   - DIK_LWIN maps to "Left Windows" on Windows vs. "Left GUI" elsewhere.
    std::string sdl_name = std::string(SDL_GetScancodeName(windows_scancode_table[keycode]));
    if (!sdl_name.empty()) {
        std::transform(sdl_name.begin(), sdl_name.end(), sdl_name.begin(), ::toupper);
        return sdl_name;
    }

    // Fallback
    return std::format("Key code: {}", keycode);
}

std::string dik_to_string(const combo_scancode& keycode) {
    std::string modifier = (keycode.modifier ? dik_to_string(keycode.modifier) + " + " : "");
    std::string key = dik_to_string(keycode.key);
    return modifier + key;
}
