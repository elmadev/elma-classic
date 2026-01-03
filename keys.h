#ifndef KEYS_H
#define KEYS_H

typedef int DIK_Code;

constexpr int MaxKeycode = 256;

void keys_init();
void update_keypress_buffer();
void update_key_state();

int get_keypress();
void empty_keypress_buffer();
bool has_keypress();

bool is_key_down(int code);
bool is_ctrl_alt_down();

#define KEY_ESC (1)
#define KEY_ENTER (400)
#define KEY_UP (401)
#define KEY_DOWN (402)
#define KEY_LEFT (403)
#define KEY_RIGHT (404)
#define KEY_PGUP (405)
#define KEY_PGDOWN (406)
#define KEY_DEL (407)
#define KEY_BACKSPACE (408)

#endif
