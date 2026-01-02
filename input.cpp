#include "ALL.H"
#include "directinput_scancodes.h"

const static int KeyBufferSize = 30;
static int* KeyBuffer = NULL;
static int KeyBufferCount = 0;

static char* KeyState1 = NULL;
static char* KeyState2 = NULL;
static bool UseKeyState2 = true;
static int* DIKToAscii = NULL;

static void init_dik_to_ascii(void);

void mk_init(void) {
    if (KeyBuffer) {
        internal_error("mk_init() called twice!");
    }

    KeyState1 = new char[MaxKeycode];
    KeyState2 = new char[MaxKeycode];
    if (!KeyState1 || !KeyState2) {
        internal_error("KeyState allocation failed in mk_init!()");
    }
    for (int i = 0; i < MaxKeycode; i++) {
        KeyState1[i] = KeyState2[i] = 0;
    }

    KeyBuffer = new int[KeyBufferSize];
    if (!KeyBuffer) {
        internal_error("KeyBuffer allocation failed in mk_init!()");
    }
    KeyBufferCount = 0;
    UseKeyState2 = true;

    init_dik_to_ascii();
}

int mk_getextchar(void) {
    while (1) {
        handle_events();
        if (KeyBufferCount > 0) {
            int c = KeyBuffer[0];
            for (int i = 0; i < KeyBufferCount - 1; i++) {
                KeyBuffer[i] = KeyBuffer[i + 1];
            }
            KeyBufferCount--;
            return c;
        }
    }
}

void mk_emptychar(void) {
    handle_events();
    KeyBufferCount = 0;
}

int mk_kbhit(void) {
    handle_events();
    return KeyBufferCount > 0;
}

int mk_getstate(int code) {
    if (code < 0 || code > MaxKeycode) {
        internal_error("code out of range in mk_getstate()!");
        return 0;
    }
    if (UseKeyState2) {
        return KeyState2[code];
    } else {
        return KeyState1[code];
    }
}

void mkw_getDIstate(void) {
    if (!KeyBuffer) {
        internal_error("Buffer is NULL in mkw_getDIstate()");
    }

    if (UseKeyState2) {
        fill_key_state(KeyState1);
    } else {
        fill_key_state(KeyState2);
    }
    UseKeyState2 = !UseKeyState2;
}

void mkw_setkeydown(void) {
    const char* prev = NULL;
    const char* current = NULL;
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

static void init_dik_to_ascii(void) {
    DIKToAscii = new int[260];
    for (int i = 0; i < MaxKeycode; i++) {
        DIKToAscii[i] = 0;
    }
    // Most beirjuk billentyuket:
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
    DIKToAscii[DIK_MINUS] = '-';
    DIKToAscii[DIK_SUBTRACT] = '-';
    DIKToAscii[DIK_PERIOD] = '.';

    DIKToAscii[DIK_ESCAPE] = MK_ESC;
    DIKToAscii[DIK_RETURN] = MK_ENTER;
    DIKToAscii[DIK_NUMPADENTER] = MK_ENTER;
    DIKToAscii[DIK_UP] = MK_UP;
    DIKToAscii[DIK_NUMPAD8] = MK_UP;
    DIKToAscii[DIK_DOWN] = MK_DOWN;
    DIKToAscii[DIK_NUMPAD2] = MK_DOWN;
    DIKToAscii[DIK_LEFT] = MK_LEFT;
    DIKToAscii[DIK_NUMPAD4] = MK_LEFT;
    DIKToAscii[DIK_RIGHT] = MK_RIGHT;
    DIKToAscii[DIK_NUMPAD6] = MK_RIGHT;
    DIKToAscii[DIK_PRIOR] = MK_PGUP;
    DIKToAscii[DIK_NUMPAD9] = MK_PGUP;
    DIKToAscii[DIK_NEXT] = MK_PGDOWN;
    DIKToAscii[DIK_NUMPAD3] = MK_PGDOWN;
    DIKToAscii[DIK_DELETE] = MK_DEL;
    DIKToAscii[DIK_DECIMAL] = MK_DEL;
    DIKToAscii[DIK_BACK] = MK_BACKSPACE;

    DIKToAscii[DIK_MINUS] = MK_LEFT;
    DIKToAscii[DIK_SUBTRACT] = MK_LEFT;
    DIKToAscii[DIK_EQUALS] = MK_RIGHT;
    DIKToAscii[DIK_ADD] = MK_RIGHT;
}

// menu_nav-ben meg van irve DOS-os verziohoz:
int controlaltnyomva(void) {
    if (mk_getstate(DIK_LMENU) && mk_getstate(DIK_LCONTROL)) {
        return 1;
    } else {
        return 0;
    }
}
