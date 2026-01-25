#include "keys.h"
#include "platform_impl.h"

constexpr int KeyBufferSize = 30;
static Keycode KeyBuffer[KeyBufferSize];
static int KeyBufferCount = 0;

void add_key_to_buffer(Keycode keycode) {
    if (KeyBufferCount >= KeyBufferSize) {
        return;
    }
    KeyBuffer[KeyBufferCount++] = keycode;
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
