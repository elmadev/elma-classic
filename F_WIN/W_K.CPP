#include "w_all.h"

// static int Maxkod = 500;
// static int* Tomb = NULL;

static int Buffsize = 30;
static int* Buffer = NULL;
static BYTE* Buffer1 = NULL;
static BYTE* Buffer2 = NULL;
static int Elsojon = 1;
static int Charszam = 0;
static int* Konvtomb = NULL;

void mk_init(void) {
    if (Buffer) {
        hiba("mk_init-ben Buffer != NULL!");
    }

    // Lefoglaljuk buffereket:
    Buffer1 = new BYTE[256];
    Buffer2 = new BYTE[256];
    if (!Buffer1 || !Buffer2) {
        hiba("memory mk_init-ben!");
    }
    for (int i = 0; i < 256; i++) {
        Buffer1[i] = Buffer2[i] = 0;
    }

    Buffer = new int[Buffsize];
    if (!Buffer) {
        hiba("memory mk_init-ben!");
    }
    Charszam = 0;
    Elsojon = 1;

    void initwkonvtomb(void);
    initwkonvtomb();
}

int mk_getextchar(void) {
    while (1) {
        mv_check(/*"W_k mk_getext"*/);
        if (Charszam > 0) {
            int c = Buffer[0];
            for (int i = 0; i < Charszam - 1; i++) {
                Buffer[i] = Buffer[i + 1];
            }
            Charszam--;
            return c;
        }
    }
}

void mk_emptychar(void) {
    mv_check(/*"W_k mk_empty"*/);
    Charszam = 0;
}

int mk_kbhit(void) {
    mv_check(/*"W_k mk_kbh"*/);
    if (Charszam > 0) {
        return 1;
    } else {
        return 0;
    }
}

int mk_getstate(int kod) {
    if (kod < 0 || kod > 256) {
        hiba("mk_getstate-ben kod nem ismert!");
        return 0;
    }
    if (Elsojon) {
        return Buffer2[kod];
    } else {
        return Buffer1[kod];
    }
}

// Kovetkezo billentyu fuggvenyek nem publikusak:
void mkw_getDIstate(void) {
    if (!Buffer) {
        hiba("mkw_getDIstate-ben !Buffer!");
    }

    if (Elsojon) {
        refreshDI(Buffer1);
    } else {
        refreshDI(Buffer2);
    }
    Elsojon = !Elsojon;
}

// Ezt csak egyszer szabad meghivni minden mkw_getDIstate utan:
void mkw_setkeydown(void) {
    BYTE *buff1 = NULL, *buff2 = NULL; // buff2 az ujabb
    if (Elsojon) {
        buff1 = Buffer1;
        buff2 = Buffer2;
    } else {
        buff1 = Buffer2;
        buff2 = Buffer1;
    }
    for (int i = 0; i < 256; i++) {
        if (!buff1[i] && buff2[i]) {
            // Ez a billentyu lenyomodott:
            if (Konvtomb[i]) {
                if (Charszam < Buffsize) {
                    if (Konvtomb[i] >= 'a' && Konvtomb[i] <= 'z' &&
                        (buff2[DIK_LSHIFT] || buff2[DIK_RSHIFT])) {
                        // Nagybetu:
                        Buffer[Charszam] = Konvtomb[i] + 'A' - 'a';
                    } else {
                        // Kisbetu:
                        Buffer[Charszam] = Konvtomb[i];
                    }
                    Charszam++;
                }
            }
        }
    }
}

void initwkonvtomb(void) {
    Konvtomb = new int[260];
    for (int i = 0; i < 256; i++) {
        Konvtomb[i] = 0;
    }
    // Most beirjuk billentyuket:
    Konvtomb[DIK_1] = '1';
    Konvtomb[DIK_2] = '2';
    Konvtomb[DIK_3] = '3';
    Konvtomb[DIK_4] = '4';
    Konvtomb[DIK_5] = '5';
    Konvtomb[DIK_6] = '6';
    Konvtomb[DIK_7] = '7';
    Konvtomb[DIK_8] = '8';
    Konvtomb[DIK_9] = '9';
    Konvtomb[DIK_0] = '0';

    Konvtomb[DIK_A] = 'a';
    Konvtomb[DIK_B] = 'b';
    Konvtomb[DIK_C] = 'c';
    Konvtomb[DIK_D] = 'd';
    Konvtomb[DIK_E] = 'e';
    Konvtomb[DIK_F] = 'f';
    Konvtomb[DIK_G] = 'g';
    Konvtomb[DIK_H] = 'h';
    Konvtomb[DIK_I] = 'i';
    Konvtomb[DIK_J] = 'j';
    Konvtomb[DIK_K] = 'k';
    Konvtomb[DIK_L] = 'l';
    Konvtomb[DIK_M] = 'm';
    Konvtomb[DIK_N] = 'n';
    Konvtomb[DIK_O] = 'o';
    Konvtomb[DIK_P] = 'p';
    Konvtomb[DIK_Q] = 'q';
    Konvtomb[DIK_R] = 'r';
    Konvtomb[DIK_S] = 's';
    Konvtomb[DIK_T] = 't';
    Konvtomb[DIK_U] = 'u';
    Konvtomb[DIK_V] = 'v';
    Konvtomb[DIK_W] = 'w';
    Konvtomb[DIK_X] = 'x';
    Konvtomb[DIK_Y] = 'y';
    Konvtomb[DIK_Z] = 'z';

    Konvtomb[DIK_SPACE] = ' ';
    Konvtomb[DIK_MINUS] = '-';
    Konvtomb[DIK_SUBTRACT] = '-';
    Konvtomb[DIK_PERIOD] = '.';

    Konvtomb[DIK_ESCAPE] = MK_ESC;
    Konvtomb[DIK_RETURN] = MK_ENTER;
    Konvtomb[DIK_NUMPADENTER] = MK_ENTER;
    Konvtomb[DIK_UP] = MK_UP;
    Konvtomb[DIK_NUMPAD8] = MK_UP;
    Konvtomb[DIK_DOWN] = MK_DOWN;
    Konvtomb[DIK_NUMPAD2] = MK_DOWN;
    Konvtomb[DIK_LEFT] = MK_LEFT;
    Konvtomb[DIK_NUMPAD4] = MK_LEFT;
    Konvtomb[DIK_RIGHT] = MK_RIGHT;
    Konvtomb[DIK_NUMPAD6] = MK_RIGHT;
    Konvtomb[DIK_PRIOR] = MK_PGUP;
    Konvtomb[DIK_NUMPAD9] = MK_PGUP;
    Konvtomb[DIK_NEXT] = MK_PGDOWN;
    Konvtomb[DIK_NUMPAD3] = MK_PGDOWN;
    Konvtomb[DIK_DELETE] = MK_DEL;
    Konvtomb[DIK_DECIMAL] = MK_DEL;
    Konvtomb[DIK_BACK] = MK_BACKSPACE;

    // Sound Volume allitas miatt:
    Konvtomb[DIK_MINUS] = MK_LEFT;
    Konvtomb[DIK_SUBTRACT] = MK_LEFT;
    Konvtomb[DIK_EQUALS] = MK_RIGHT;
    Konvtomb[DIK_ADD] = MK_RIGHT;
}

// valaszt2-ben meg van irve DOS-os verziohoz:
int controlaltnyomva(void) {
    if (mk_getstate(DIK_LMENU) && mk_getstate(DIK_LCONTROL)) {
        return 1;
    } else {
        return 0;
    }
}
