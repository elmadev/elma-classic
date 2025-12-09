#include "all.h"

#define MAXSOR32 (6)

// Visszaadja lenyomott billentyut:
int dialog320(char* text1, char* text2, char* text3, char* text4, char* text5, char* text6) {
    int szam;
    char* textek[MAXSOR32 + 1] = {text1, text2, text3, text4, text5, text6, NULL};
    for (szam = 0; szam < 6; szam++) {
        if (!textek[szam]) {
            break;
        }
    }

    int enter = 0;
    if (strcmp(textek[0], "ESC") == 0) {
        enter = 1;
        for (int i = 0; i < szam; i++) {
            textek[i] = textek[i + 1];
        }
        szam--;
    }

    szoveglista szl;
    int dy = 40;
    int y0 = 240 - (szam - 1) / 2 * dy;
    for (int i = 0; i < szam; i++) {
        szl.addszoveg_kozep(textek[i], 320, y0 + i * dy);
    }
    mk_emptychar();
    while (1) {
        if (mk_kbhit()) {
            int c = mk_getextchar();
            if (enter) {
                if (c == MK_ESC) {
                    return c;
                }
            } else {
                return c;
            }
        }
        szl.kirajzol();
    }
}
