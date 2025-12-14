#include "ALL.H"

#define MAXSOR32 (6)

// Visszaadja lenyomott billentyut:
int dialog320(const char* text1, const char* text2, const char* text3, const char* text4,
              const char* text5, const char* text6) {
    int szam;
    const char* textek[MAXSOR32 + 1] = {text1, text2, text3, text4, text5, text6, NULL};
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
