#include "ALL.H"

// Ez igaz, ha editorban van vezerles:
int Editorban_dialnak = 0;

unsigned char Dialogszin = 4;
unsigned char Dialogszelszin = 0;
unsigned char Buttonszin = 8;

void kirajzbox(pic8* ppic, int x1, int y1, int x2, int y2, unsigned char felulet,
               unsigned char szele) {
    ppic->fill_box(x1, y1, x2, y2, felulet);
    ppic->line(x1, y1, x2, y1, szele);
    ppic->line(x1, y2, x2, y2, szele);
    ppic->line(x1, y1, x1, y2, szele);
    ppic->line(x2, y1, x2, y2, szele);
}

void kirajzbox(pic8* ppic, box boxa, unsigned char felulet, unsigned char szele) {
    kirajzbox(ppic, boxa.x1, boxa.y1, boxa.x2, boxa.y2, felulet, szele);
}

int boxban(int x, int y, box boxa) {
    if (x >= boxa.x1 && x <= boxa.x2 && y >= boxa.y1 && y <= boxa.y2) {
        return 1;
    } else {
        return 0;
    }
}

int dialog(const char* text1, const char* text2, const char* text3, const char* text4,
           const char* text5, const char* text6, const char* text7, const char* text8,
           const char* text9, const char* text10, const char* text11, const char* text12,
           const char* text13, const char* text14, const char* text15, const char* text16,
           const char* text17, const char* text18, const char* text19, const char* text20,
           const char* text21, const char* text22, const char* text23, const char* text24) {
    const char* texttomb[30] = {text1,  text2,  text3,  text4,  text5,  text6,  text7,  text8,
                                text9,  text10, text11, text12, text13, text14, text15, text16,
                                text17, text18, text19, text20, text21, text22, text23, text24};
    int textszam = 24;
    for (int i = 0; i < 24; i++) {
        if (!texttomb[i]) {
            textszam = i;
            break;
        }
    }
    const char* gombtomb[30];
    for (int i = 0; i < 24; i++) {
        gombtomb[i] = NULL;
    }
    int gombszam = 0;
    for (int i = 0; i < textszam; i++) {
        if (strcmp(texttomb[i], "GOMBOK") == 0) {
            gombszam = textszam - i - 1;
            if (gombszam <= 0) {
                hiba("dialog-ban gombszam <= 0!");
            }
            textszam = i;
            for (int j = 0; j < gombszam; j++) {
                gombtomb[j] = texttomb[textszam + j + 1];
            }
            break;
        }
    }
    if (gombszam == 0) {
        gombszam = 1;
        gombtomb[0] = "OK";
    }

    int visszater = 0;
    if (gombszam == 1 && strcmpi(gombtomb[0], "VISSZATER") == 0) {
        gombszam = 0;
        visszater = 1;
    }

    if (textszam < 1) {
        hiba("iuofrf");
    }

    // Beallitottuk tomboket!!!

    if (Editorban_dialnak) {
        push(); // Leszedjuk egeret keprol
    } else {
        Moux = 320;
        Mouy = 240;
    }

    invalidateegesz();

    int xsize = 0;
    for (int i = 0; i < textszam; i++) {
        if (xsize < Pabc2->len(texttomb[i]) + 16) {
            xsize = Pabc2->len(texttomb[i]) + 16;
        }
    }

    int dy = 20;
    int ysize = textszam * dy + 50;
    int x1 = 320 - xsize / 2;
    int y1 = 240 - ysize / 2;
    int x2 = 320 + xsize / 2;
    int y2 = 240 + ysize / 2;
    kirajzbox(BufferMain, x1, y1, x2, y2, Dialogszin, Dialogszelszin);
    for (int i = 0; i < textszam; i++) {
        Pabc2->write_centered(BufferMain, 320, y1 + 22 + i * dy, texttomb[i]);
    }

    // Kirajzoljuk gombokat is:
    int gx1tomb[15], gx2tomb[15], gy1 = y2 - 30, gy2 = y2 - 10;
    int gdx = 80;
    if (gombszam == 1) {
        gx1tomb[0] = 320;
    }
    if (gombszam == 2) {
        gx1tomb[0] = 320 - gdx / 2;
        gx1tomb[1] = 320 + gdx / 2;
    }
    if (gombszam == 3) {
        gx1tomb[0] = 320 - gdx;
        gx1tomb[1] = 320;
        gx1tomb[2] = 320 + gdx;
    }
    if (gombszam > 3) {
        hiba("uofiufriufr");
    }
    for (int i = 0; i < gombszam; i++) {
        int xs = Pabc2->len(gombtomb[i]) + 10;
        gx2tomb[i] = gx1tomb[i] + xs / 2;
        gx1tomb[i] = gx1tomb[i] - xs / 2;
        kirajzbox(BufferMain, gx1tomb[i], gy1, gx2tomb[i], gy2, Buttonszin, Dialogszelszin);
        Pabc2->write_centered(BufferMain, (gx2tomb[i] + gx1tomb[i]) / 2, gy1 + 14, gombtomb[i]);
    }

    bltfront(BufferMain);
    pop();

    if (visszater) {
        if (!Editorban_dialnak) {
            push(); // Leszedjuk egeret keprol
        }
        return 0;
    }

    // Most figyeljuk user mit csinal:
    // Azert adunk egy erteket, hogy ha lenyomassal jutott
    // dialogba, egyszer fel kell engednie mielott ujra
    // lenyomasnak erzekelne:
    left_mouse_clicked();
    right_mouse_clicked();
    while (1) {
        while (mk_kbhit()) {
            int c = mk_getextchar();
            if (gombszam == 1) {
                if (c == MK_ESC || c == MK_ENTER) {
                    if (!Editorban_dialnak) {
                        push(); // Leszedjuk egeret keprol
                    }
                    return 0;
                }
            }
            for (int i = 0; i < gombszam; i++) {
                if (c == tolower(*(gombtomb[i]))) {
                    if (!Editorban_dialnak) {
                        push(); // Leszedjuk egeret keprol
                    }
                    return i;
                }
            }
        }
        // Megnezzuk eger gombok lemenetelet:
        if (right_mouse_clicked()) {
            if (gombszam == 1) {
                if (!Editorban_dialnak) {
                    push(); // Leszedjuk egeret keprol
                }
                return 0;
            }
        }

        if (left_mouse_clicked()) {
            int ujx = 0, ujy = 0;
            get_mouse_position(&ujx, &ujy);
            push();
            Moux = ujx;
            Mouy = ujy;
            pop();
            // Most nezzuk meg, hogy hova kattintott:
            for (int i = 0; i < gombszam; i++) {
                if (ujx >= gx1tomb[i] && ujx <= gx2tomb[i] && ujy >= gy1 && ujy <= gy2) {
                    if (!Editorban_dialnak) {
                        push(); // Leszedjuk egeret keprol
                    }
                    return i;
                }
            }
            continue;
        }
        int ujx = 0, ujy = 0;
        get_mouse_position(&ujx, &ujy);
        if (ujx == Moux && ujy == Mouy) {
            continue;
        }
        push();
        Moux = ujx;
        Mouy = ujy;
        pop();
    }
}
