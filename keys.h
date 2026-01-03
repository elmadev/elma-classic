#ifndef KEYS_H
#define KEYS_H

// DIK_ Windows scancode
typedef int DikScancode;

// ASCII char defined in keys_init() or one of the special keycodes defined below.
typedef int Keycode;
constexpr Keycode KEY_ESC = 1;
constexpr Keycode KEY_ENTER = 400;
constexpr Keycode KEY_UP = 401;
constexpr Keycode KEY_DOWN = 402;
constexpr Keycode KEY_LEFT = 403;
constexpr Keycode KEY_RIGHT = 404;
constexpr Keycode KEY_PGUP = 405;
constexpr Keycode KEY_PGDOWN = 406;
constexpr Keycode KEY_DEL = 407;
constexpr Keycode KEY_BACKSPACE = 408;

constexpr int MaxKeycode = 256;

void keys_init();
void update_keypress_buffer();
void update_key_state();

Keycode get_keypress();
void empty_keypress_buffer();
bool has_keypress();

bool is_key_down(DikScancode code);
bool is_ctrl_alt_down();

#endif
