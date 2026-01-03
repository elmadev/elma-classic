#ifndef M_K_H
#define M_K_H

typedef int DIK_Code;

const static int MaxKeycode = 256;

void update_keypress_buffer();
void update_key_state();
void keys_init(void);
int get_keypress(void);
void empty_keypress_buffer(void);
int has_keypress(void);
int is_key_down(int kod);
int is_ctrl_alt_down(void);

#define KEY_ESC (1) // DIK_ESCAPE
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
