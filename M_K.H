#ifndef M_K_H
#define M_K_H

typedef int DIK_Code;

const static int MaxKeycode = 256;

void fill_key_state(char* buffer);

void mkw_setkeydown();
void mkw_getDIstate();
void mk_init(void);
int mk_getextchar(void);
void mk_emptychar(void);
int mk_kbhit(void);
int mk_getstate(int kod);
int controlaltnyomva(void);

#define MK_ESC (1) // DIK_ESCAPE
#define MK_ENTER (400)
#define MK_UP (401)
#define MK_DOWN (402)
#define MK_LEFT (403)
#define MK_RIGHT (404)
#define MK_PGUP (405)
#define MK_PGDOWN (406)
#define MK_DEL (407)
#define MK_BACKSPACE (408)

#endif
