#include "platform/scancode.h"
#include "main.h"
#include "platform/sdl/keyboard.h"
#include <format>
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
    case DIK_1:
        return "1";
    case DIK_2:
        return "2";
    case DIK_3:
        return "3";
    case DIK_4:
        return "4";
    case DIK_5:
        return "5";
    case DIK_6:
        return "6";
    case DIK_7:
        return "7";
    case DIK_8:
        return "8";
    case DIK_9:
        return "9";
    case DIK_0:
        return "0";
    case DIK_MINUS:
        return "-";
    case DIK_EQUALS:
        return "=";
    case DIK_BACK:
        return "<-";
    case DIK_TAB:
        return "TAB";
    case DIK_Q:
        return "Q";
    case DIK_W:
        return "W";
    case DIK_E:
        return "E";
    case DIK_R:
        return "R";
    case DIK_T:
        return "T";
    case DIK_Y:
        return "Y";
    case DIK_U:
        return "U";
    case DIK_I:
        return "I";
    case DIK_O:
        return "O";
    case DIK_P:
        return "P";
    case DIK_LBRACKET:
        return "[";
    case DIK_RBRACKET:
        return "]";
    case DIK_RETURN:
        return "ENTER";
    case DIK_LCONTROL:
        return "L CTRL";
    case DIK_A:
        return "A";
    case DIK_S:
        return "S";
    case DIK_D:
        return "D";
    case DIK_F:
        return "F";
    case DIK_G:
        return "G";
    case DIK_H:
        return "H";
    case DIK_J:
        return "J";
    case DIK_K:
        return "K";
    case DIK_L:
        return "L";
    case DIK_SEMICOLON:
        return ";";
    case DIK_APOSTROPHE:
        return "\"";
    case DIK_GRAVE:
        return "`";
    case DIK_LSHIFT:
        return "L SHIFT";
    case DIK_BACKSLASH:
        return "\\";
    case DIK_Z:
        return "Z";
    case DIK_X:
        return "X";
    case DIK_C:
        return "C";
    case DIK_V:
        return "V";
    case DIK_B:
        return "B";
    case DIK_N:
        return "N";
    case DIK_M:
        return "M";
    case DIK_COMMA:
        return ",";
    case DIK_PERIOD:
        return ".";
    case DIK_SLASH:
        return "SLASH";
    case DIK_RSHIFT:
        return "R SHIFT";
    case DIK_MULTIPLY:
        return "PAD_*";
    case DIK_LMENU:
        return "L ALT";
    case DIK_SPACE:
        return "SPACEBAR";
    case DIK_CAPITAL:
        return "CAPS LOCK";
    case DIK_F1:
        return "F1";
    case DIK_F2:
        return "F2";
    case DIK_F3:
        return "F3";
    case DIK_F4:
        return "F4";
    case DIK_F5:
        return "F5";
    case DIK_F6:
        return "F6";
    case DIK_F7:
        return "F7";
    case DIK_F8:
        return "F8";
    case DIK_F9:
        return "F9";
    case DIK_F10:
        return "F10";
    case DIK_NUMLOCK:
        return "NUM LOCK";
    case DIK_SCROLL:
        return "SCROLL LOCK";
    case DIK_NUMPAD7:
        return "PAD_HOME";
    case DIK_NUMPAD8:
        return "PAD_UP";
    case DIK_NUMPAD9:
        return "PAD_PGUP";
    case DIK_SUBTRACT:
        return "PAD_-";
    case DIK_NUMPAD4:
        return "PAD_LEFT";
    case DIK_NUMPAD5:
        return "PAD_5";
    case DIK_NUMPAD6:
        return "PAD_RIGHT";
    case DIK_ADD:
        return "PAD_+";
    case DIK_NUMPAD1:
        return "PAD_END";
    case DIK_NUMPAD2:
        return "PAD_DOWN";
    case DIK_NUMPAD3:
        return "PAD_PGDOWN";
    case DIK_NUMPAD0:
        return "PAD_INS";
    case DIK_DECIMAL:
        return "PAD_DEL";
    case DIK_F11:
        return "F11";
    case DIK_F12:
        return "F12";
    case DIK_F13:
        return "F13";
    case DIK_F14:
        return "F14";
    case DIK_F15:
        return "F15";
    case DIK_KANA:
        return "KANA";
    case DIK_CONVERT:
        return "CONVERT";
    case DIK_NOCONVERT:
        return "NOCONVERT";
    case DIK_YEN:
        return "YEN";
    case DIK_NUMPADEQUALS:
        return "PAD_=";
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
        return "PAD_ENTER";
    case DIK_RCONTROL:
        return "R CTRL";
    case DIK_NUMPADCOMMA:
        return "COMMA";
    case DIK_DIVIDE:
        return "PAD_/";
    case DIK_SYSRQ:
        return "SYSRQ";
    case DIK_RMENU:
        return "R ALT";
    case DIK_HOME:
        return "HOME";
    case DIK_UP:
        return "UP ARROW";
    case DIK_PRIOR:
        return "PAGEUP";
    case DIK_LEFT:
        return "LEFT ARROW";
    case DIK_RIGHT:
        return "RIGHT ARROW";
    case DIK_END:
        return "END";
    case DIK_DOWN:
        return "DOWN ARROW";
    case DIK_NEXT:
        return "PAGE DOWN";
    case DIK_INSERT:
        return "INS";
    case DIK_DELETE:
        return "DEL";
    case DIK_LWIN:
        return "L WIN";
    case DIK_RWIN:
        return "R WIN";
    case DIK_APPS:
        return "APPLICATION";
    }
    return std::format("Key code: {}", keycode);
}

std::string dik_to_string(const combo_scancode& keycode) {
    std::string modifier = (keycode.modifier ? dik_to_string(keycode.modifier) + " + " : "");
    std::string key = dik_to_string(keycode.key);
    return modifier + key;
}
