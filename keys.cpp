#include "keys.h"
#include <directinput/scancodes.h>
#include "main.h"
#include "platform_impl.h"

constexpr int KeyBufferSize = 30;
static Keycode KeyBuffer[KeyBufferSize];
static int KeyBufferCount = 0;

static char KeyState1[MaxKeycode];
static char KeyState2[MaxKeycode];
static bool UseKeyState2 = true;
static Keycode DIKToAscii[MaxKeycode];

// Map DIK codes to ascii (+ a few extra codepoints for special keys)
void keys_init() {
    DIKToAscii[DIK_1] = '1';
    DIKToAscii[DIK_2] = '2';
    DIKToAscii[DIK_3] = '3';
    DIKToAscii[DIK_4] = '4';
    DIKToAscii[DIK_5] = '5';
    DIKToAscii[DIK_6] = '6';
    DIKToAscii[DIK_7] = '7';
    DIKToAscii[DIK_8] = '8';
    DIKToAscii[DIK_9] = '9';
    DIKToAscii[DIK_0] = '0';

    DIKToAscii[DIK_A] = 'a';
    DIKToAscii[DIK_B] = 'b';
    DIKToAscii[DIK_C] = 'c';
    DIKToAscii[DIK_D] = 'd';
    DIKToAscii[DIK_E] = 'e';
    DIKToAscii[DIK_F] = 'f';
    DIKToAscii[DIK_G] = 'g';
    DIKToAscii[DIK_H] = 'h';
    DIKToAscii[DIK_I] = 'i';
    DIKToAscii[DIK_J] = 'j';
    DIKToAscii[DIK_K] = 'k';
    DIKToAscii[DIK_L] = 'l';
    DIKToAscii[DIK_M] = 'm';
    DIKToAscii[DIK_N] = 'n';
    DIKToAscii[DIK_O] = 'o';
    DIKToAscii[DIK_P] = 'p';
    DIKToAscii[DIK_Q] = 'q';
    DIKToAscii[DIK_R] = 'r';
    DIKToAscii[DIK_S] = 's';
    DIKToAscii[DIK_T] = 't';
    DIKToAscii[DIK_U] = 'u';
    DIKToAscii[DIK_V] = 'v';
    DIKToAscii[DIK_W] = 'w';
    DIKToAscii[DIK_X] = 'x';
    DIKToAscii[DIK_Y] = 'y';
    DIKToAscii[DIK_Z] = 'z';

    DIKToAscii[DIK_SPACE] = ' ';
    DIKToAscii[DIK_PERIOD] = '.';

    DIKToAscii[DIK_ESCAPE] = KEY_ESC;
    DIKToAscii[DIK_RETURN] = KEY_ENTER;
    DIKToAscii[DIK_NUMPADENTER] = KEY_ENTER;

    DIKToAscii[DIK_UP] = KEY_UP;
    DIKToAscii[DIK_NUMPAD8] = KEY_UP;
    DIKToAscii[DIK_DOWN] = KEY_DOWN;
    DIKToAscii[DIK_NUMPAD2] = KEY_DOWN;
    DIKToAscii[DIK_LEFT] = KEY_LEFT;
    DIKToAscii[DIK_NUMPAD4] = KEY_LEFT;
    DIKToAscii[DIK_RIGHT] = KEY_RIGHT;
    DIKToAscii[DIK_NUMPAD6] = KEY_RIGHT;

    DIKToAscii[DIK_PRIOR] = KEY_PGUP;
    DIKToAscii[DIK_NUMPAD9] = KEY_PGUP;
    DIKToAscii[DIK_NEXT] = KEY_PGDOWN;
    DIKToAscii[DIK_NUMPAD3] = KEY_PGDOWN;

    DIKToAscii[DIK_DELETE] = KEY_DEL;
    DIKToAscii[DIK_DECIMAL] = KEY_DEL;
    DIKToAscii[DIK_BACK] = KEY_BACKSPACE;

    DIKToAscii[DIK_MINUS] = KEY_LEFT;
    DIKToAscii[DIK_SUBTRACT] = KEY_LEFT;
    DIKToAscii[DIK_EQUALS] = KEY_RIGHT;
    DIKToAscii[DIK_ADD] = KEY_RIGHT;
}

Keycode get_keypress() {
    while (true) {
        handle_events();
        if (KeyBufferCount > 0) {
            Keycode c = KeyBuffer[0];
            for (int i = 0; i < KeyBufferCount - 1; i++) {
                KeyBuffer[i] = KeyBuffer[i + 1];
            }
            KeyBufferCount--;
            return c;
        }
    }
}

void empty_keypress_buffer() {
    handle_events();
    KeyBufferCount = 0;
}

bool has_keypress() {
    handle_events();
    return KeyBufferCount > 0;
}

bool is_key_down(DikScancode code) {
    if (code < 0 || code >= MaxKeycode) {
        internal_error("code out of range in is_key_down()!");
        return false;
    }
    if (UseKeyState2) {
        return KeyState2[code];
    } else {
        return KeyState1[code];
    }
}

bool is_ctrl_alt_down() { return is_key_down(DIK_LMENU) && is_key_down(DIK_LCONTROL); }

// Update whether keys are pressed down or not
void update_key_state() {
    if (UseKeyState2) {
        fill_key_state(KeyState1);
    } else {
        fill_key_state(KeyState2);
    }
    UseKeyState2 = !UseKeyState2;
}

// Add keys to the keypress buffer
// Only DIK codes defined in init_dik_to_ascii will be added to the buffer
void update_keypress_buffer() {
    const char* prev = nullptr;
    const char* current = nullptr;
    if (UseKeyState2) {
        prev = KeyState1;
        current = KeyState2;
    } else {
        prev = KeyState2;
        current = KeyState1;
    }
    for (int i = 0; i < MaxKeycode; i++) {
        if (!prev[i] && current[i]) {
            if (DIKToAscii[i]) {
                if (KeyBufferCount < KeyBufferSize) {
                    if (DIKToAscii[i] >= 'a' && DIKToAscii[i] <= 'z' &&
                        (current[DIK_LSHIFT] || current[DIK_RSHIFT])) {
                        KeyBuffer[KeyBufferCount] = DIKToAscii[i] + 'A' - 'a';
                    } else {
                        KeyBuffer[KeyBufferCount] = DIKToAscii[i];
                    }
                    KeyBufferCount++;
                }
            }
        }
    }
}
