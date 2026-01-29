#include "keys.h"
#include "platform_impl.h"
#include "platform_utils.h"

constexpr int KEY_BUFFER_SIZE = 64;
static Keycode KeyBuffer[KEY_BUFFER_SIZE];
static int KeyBufferCount = 0;

void add_key_to_buffer(Keycode keycode) {
    if (KeyBufferCount >= KEY_BUFFER_SIZE) {
        return;
    }
    KeyBuffer[KeyBufferCount++] = keycode;
}

void add_text_to_buffer(const char* text) {
    while (*text && KeyBufferCount < KEY_BUFFER_SIZE) {
        unsigned char c = *text;

        if (!is_ascii_character(c)) {
            text++;
            continue;
        }

        KeyBuffer[KeyBufferCount++] = c;
        text++;
    }
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
