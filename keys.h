#ifndef KEYS_H
#define KEYS_H

// DIK_ Windows scancode
typedef int DikScancode;

// ASCII char defined in keys_init() or one of the special codepoints defined below
typedef int Codepoint;
constexpr Codepoint KEY_ESC = 1;
constexpr Codepoint KEY_ENTER = 400;
constexpr Codepoint KEY_UP = 401;
constexpr Codepoint KEY_DOWN = 402;
constexpr Codepoint KEY_LEFT = 403;
constexpr Codepoint KEY_RIGHT = 404;
constexpr Codepoint KEY_PGUP = 405;
constexpr Codepoint KEY_PGDOWN = 406;
constexpr Codepoint KEY_DEL = 407;
constexpr Codepoint KEY_BACKSPACE = 408;

constexpr int MaxKeycode = 256;

void keys_init();
void update_keypress_buffer();
void update_key_state();

Codepoint get_keypress();
void empty_keypress_buffer();
bool has_keypress();

bool is_key_down(DikScancode code);
bool is_ctrl_alt_down();

#endif
