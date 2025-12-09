#include "all.h"

static const unsigned char Save_as_box_szin = 4, Save_as_szelszin = 5;

static unsigned char Openlistaszin = 6, Openlistainvszin = 7;

static int Maxnev = 1000; // Nem lehet nagyobb 3000-nel!
static char* Soknev = NULL;

static void abcberendez(int nevszam) {
    for (int i = 0; i < nevszam + 3; i++) {
        for (int j = 0; j < nevszam - 1; j++) {
            // Ha j es j+1 edik elem forditva van visszacsereljuk oket:
            char* nev1 = &Soknev[j * 10];
            char* nev2 = &Soknev[(j + 1) * 10];
            int abcbenelobb(char* text1, char* text2);
            if (abcbenelobb(nev2, nev1)) {
                // if( strcmp( nev1, nev2 ) > 0 ) {
                //  Csere:
                char tmp[20];
                strcpy(tmp, nev1);
                strcpy(nev1, nev2);
                strcpy(nev2, tmp);
            }
        }
    }
}

static void betolt(char* nev) {
    if (!Ptop) {
        hiba("uoiwfiufre");
    }
    if (!*nev) {
        hiba("oiufewguyxygxw");
    }
    char tmp[15];
    if (strlen(nev) > 8) {
        hiba("iovffiufi");
    }
    strcpy(tmp, nev);
    strcat(tmp, ".lev");
    if (strlen(tmp) > 12) {
        hiba("betolt-ben strlen( tmp ) > 12!");
    }
    strcpy(State->editfilenev, tmp);

    Valtozott = 0;
    if (!floadlevel_e(tmp, 1)) {
        State->editfilenev[0] = 0;
    }

    kitolt();
}

static void igazit(int* pkur, int* pfelso, int nevszam, int helyszam) {
    if (*pkur < 0) {
        *pkur = 0;
    }
    if (*pkur > nevszam - 1) {
        *pkur = nevszam - 1;
    }
    if (*pfelso > *pkur) {
        *pfelso = *pkur;
    }
    if (*pfelso < *pkur - helyszam + 1) {
        *pfelso = *pkur - helyszam + 1;
    }
}

static void boxbair(pic8* ppic, box abox, unsigned char szin, char* text, int alahuzas = 0) {
    ppic->fillbox(abox.x1 + 1, abox.y1 + 1, abox.x2 - 1, abox.y2 - 1, szin);
    if (alahuzas) {
        int hossz = Pabc2->len(text);
        int x = (abox.x1 + abox.x2) / 2 - hossz / 2 - 3;
        Pabc2->write(ppic, x, (abox.y1 + abox.y2) / 2 + 5, text);
        for (int i = 0; i < 6; i++) {
            ppic->ppixel(x + hossz + i + 1, (abox.y1 + abox.y2) / 2 + 4, 0);
        }
    } else {
        Pabc2->writekozep(ppic, (abox.x1 + abox.x2) / 2, (abox.y1 + abox.y2) / 2 + 5, text);
    }
}

static void boxbair_balra(pic8* ppic, box abox, unsigned char szin, char* text) {
    ppic->fillbox(abox.x1 + 1, abox.y1 + 1, abox.x2 - 1, abox.y2 - 1, szin);
    Pabc2->write(ppic, abox.x1 + 2, (abox.y1 + abox.y2) / 2 + 5, text);
}

static void boxbarajz(pic8* ppic, box abox, unsigned char szin, int fel) {
    int x1 = abox.x1;
    int y1 = abox.y1;
    int x2 = abox.x2;
    int y2 = abox.y2;
    int szeltav = 16;
    x1 += szeltav;
    x2 -= szeltav;
    y1 += 5;
    y2 -= 8;
    int xkozep = (x1 + x2) / 2;
    for (int xfut = x1; xfut <= xkozep; xfut++) {
        int x = xfut;
        int y = int(y1 + (y2 - y1) / double(xkozep - x1) * (x - x1));
        if (fel) {
            y = abox.y2 - (y - abox.y1);
            ppic->ppixel(x, y, szin);
            ppic->ppixel(x, y - 3, szin);
            x = xkozep + (xkozep - x);
            ppic->ppixel(x, y, szin);
            ppic->ppixel(x, y - 3, szin);
        } else {
            ppic->ppixel(x, y, szin);
            ppic->ppixel(x, y + 3, szin);
            x = xkozep + (xkozep - x);
            ppic->ppixel(x, y, szin);
            ppic->ppixel(x, y + 3, szin);
        }
    }
}

void tolt_open(void) {
    invalidateegesz();
    if (Valtozott) {
        if (dialog("There are unsaved changes in the level file.",
                   "If you open another file, you will loose these changes.",
                   "Do you still want to continue?", "GOMBOK", "Yes", "No") == 1) {
            return;
        }
    }
    if (!Soknev) {
        Soknev = new char[Maxnev * 10 + 100];
        if (!Soknev) {
            hiba("Nincs eleg memoria tolt_open-ben (hjfdf)!");
        }
    }
    for (int i = 0; i < Maxnev * 10 + 10; i++) {
        Soknev[i] = 0;
    }
    finame fname;
    int done = fifirst("lev\\*.lev", fname);
    int nevszam = 0;
    while (!done) {
        if (strlen(fname) > 14) {
            hiba("ijhfiyewpp");
        }
        char* nev = &Soknev[nevszam * 10];
        strcpy(nev, fname);
        // Leveszi kiterjesztest:
        int i = 0;
        while (nev[i] != '.') {
            if (!nev[i]) {
                hiba("Nincs pont nevben! (yewruyrewuy): ", nev);
            }
            i++;
        }
        nev[i] = 0;
        if (strlen(nev) > 8) {
            hiba("iuoriureiur");
        }

        done = finext(fname);
        nevszam++;
        if (nevszam >= Maxnev) {
            dialog("Number of files > 1000 in open dialog box!", "Could not load all the files!");
            done = 1;
        }
    }

    ficlose();

    if (nevszam < 1) {
        dialog("There is not any level file (*.lev) in this directory!");
        return;
    }

    abcberendez(nevszam);

    int kur = 0; // Ha megtalalja eddigit, akkor kur-t raallitja
    for (int i = 0; i < nevszam; i++) {
        char tmp[50];
        strcpy(tmp, &Soknev[i * 10]);
        strcat(tmp, ".lev");
        if (strcmpi(tmp, State->editfilenev) == 0) {
            kur = i;
        }
    }

    // State->editfilenev[0] = 0; Igy nem lesz unnamed CANCEL utan

    // Koordok megadasa:
    int helyszam = 10;
    int dy = 20;
    int x1 = 200;
    int y1 = 100;
    int x2 = 401;
    int y2 = 174 + helyszam * dy;
    int lx1 = x1 + 10;
    int ly1 = y1 + 37;
    int lx2 = lx1 + 100;
    int ly2 = ly1 + helyszam * dy;

    box boxfel = {x1 + 10, y1 + 11, x1 + 110, y1 + 31};
    box boxle = {x1 + 10, y2 - 30, x1 + 110, y2 - 10};
    box boxcancel = {x1 + 121, (y2 + y1) / 2 - 10, x1 + 121 + 70, (y2 + y1) / 2 + 10};

    int felso = 0;
    int eddigfelso = -1;
    int eddigkur = 0;
    mk_emptychar();
    // Azert 1, hogy ha lenyomott gombbal erkezett,
    // ne tekintse lenyomottnak:
    int egerballevoltnyomva = 1;
    while (1) {
        // Nezzuk gombokat:
        while (mk_kbhit()) {
            int c = mk_getextchar();
            if (c == MK_ESC) {
                return;
            }
            if (c == MK_ENTER) {
                igazit(&kur, &felso, nevszam, helyszam);
                betolt(&Soknev[kur * 10]);
                return;
            }
            if (c == MK_UP) {
                kur--;
            }
            if (c == MK_PGUP) {
                kur -= helyszam - 1;
            }
            if (c == MK_DOWN) {
                kur++;
            }
            if (c == MK_PGDOWN) {
                kur += helyszam - 1;
            }
        }
        // Eger lenyomas:
        int balnyomva = 0;
        int balallapot = getbutbmou();
        if (balallapot && !egerballevoltnyomva) {
            balnyomva = 1;
        }
        egerballevoltnyomva = balallapot;

        if (balnyomva) {
            int ujx = 0, ujy = 0;
            getmou(&ujx, &ujy);
            push();
            Moux = ujx;
            Mouy = ujy;
            pop();
            // Most nezzuk meg, hogy hova kattintott:
            if (boxban(ujx, ujy, boxle)) {
                kur += helyszam - 1;
            }
            if (boxban(ujx, ujy, boxfel)) {
                kur -= helyszam - 1;
            }
            if (boxban(ujx, ujy, boxcancel)) {
                return;
            }
            if (ujx > lx1 && ujx < lx2 && ujy > ly1 && ujy < ly2) {
                // Lista folott nyomta meg gombot:
                if (ujy < ly1 + dy * helyszam) {
                    // Egy nevet el is talalt:
                    int index = (ujy - ly1) / dy;
                    index += felso;
                    if (index < nevszam) {
                        betolt(&Soknev[index * 10]);
                        return;
                    }
                }
            }
        }
        igazit(&kur, &felso, nevszam, helyszam);
        if (felso != eddigfelso || kur != eddigkur) {
            // Kirajzoljuk, mivel valtozott kep:
            push();
            kirajzbox(Buffsima, x1, y1, x2, y2, Dialogszin, Dialogszelszin);
            kirajzbox(Buffsima, lx1, ly1, lx2, ly2, Openlistaszin, Dialogszelszin);
            for (int i = 0; i < helyszam && i < nevszam; i++) {
                if (i + felso == kur) {
                    // Ez most kurens nev:
                    Buffsima->fillbox(lx1 + 1, ly1 + i * dy + 1, lx2 - 1, ly1 + (i + 1) * dy - 1,
                                      Openlistainvszin);
                }
                Pabc2->write(Buffsima, lx1 + 3, ly1 + 15 + i * dy, &Soknev[(i + felso) * 10]);
            }
            // Most jonnek gombok:
            kirajzbox(Buffsima, boxfel, Buttonszin, Dialogszelszin);
            boxbarajz(Buffsima, boxfel, Dialogszelszin, 1);
            kirajzbox(Buffsima, boxle, Buttonszin, Dialogszelszin);
            boxbarajz(Buffsima, boxle, Dialogszelszin, 0);
            kirajzbox(Buffsima, boxcancel, Buttonszin, Dialogszelszin);
            Pabc2->writekozep(Buffsima, (boxcancel.x1 + boxcancel.x2) / 2, boxcancel.y1 + 15,
                              "CANCEL");

            bltfront(Buffsima, x1, y1, x2, y2);
            pop();

            eddigfelso = felso;
            eddigkur = kur;
        }
        int ujx = 0, ujy = 0;
        getmou(&ujx, &ujy);
        if (ujx != Moux && ujy != Mouy) {
            push();
            Moux = ujx;
            Mouy = ujy;
            pop();
        }
    }
}

// Editfilenev alapjan megprobal josolni, majd oda teszi eredmenyt:
void kivalasztkezdetit(void) {
    invalidateegesz();
    if (!Soknev) {
        Soknev = new char[Maxnev * 10 + 100];
        if (!Soknev) {
            hiba("Nincs eleg memoria tolt_open-ben (hjfdf)!");
        }
    }
    for (int i = 0; i < Maxnev * 10 + 10; i++) {
        Soknev[i] = 0;
    }
    finame fname;
    int done = fifirst("lev\\*.lev", fname);
    int nevszam = 0;
    while (!done) {
        if (strlen(fname) > 14) {
            hiba("ijhfiyewpp");
        }
        char* nev = &Soknev[nevszam * 10];
        strcpy(nev, fname);

        // Leveszi kiterjesztest:
        int i = 0;
        while (nev[i] != '.') {
            if (!nev[i]) {
                hiba("Nincs pont nevben! (yewruyrewuy): ", nev);
            }
            i++;
        }
        nev[i] = 0;
        if (strlen(nev) > 8) {
            hiba("iuoriureiur");
        }

        done = finext(fname);
        nevszam++;
        if (nevszam >= Maxnev) {
            dialog("Number of files > 1000 in open dialog box!", "Could not load all the files!");
            done = 1;
        }
    }

    ficlose();

    if (nevszam < 1) {
        State->editfilenev[0] = 0;
        return;
    }

    abcberendez(nevszam);

    int kur = 0; // Ha megtalalja eddigit, akkor kur-t raallitja
    for (int i = 0; i < nevszam; i++) {
        char tmp[50];
        strcpy(tmp, &Soknev[i * 10]);
        strcat(tmp, ".lev");
        if (strcmpi(tmp, State->editfilenev) == 0) {
            kur = i;
        }
    }

    State->editfilenev[0] = 0;

    // Koordok megadasa:
    int helyszam = 10;
    int dy = 20;
    int x1 = 200;
    int y1 = 100;
    int x2 = 401;
    int y2 = 174 + helyszam * dy;
    int lx1 = x1 + 10;
    int ly1 = y1 + 37;
    int lx2 = lx1 + 100;
    int ly2 = ly1 + helyszam * dy;

    box boxfel = {x1 + 10, y1 + 11, x1 + 110, y1 + 31};
    box boxle = {x1 + 10, y2 - 30, x1 + 110, y2 - 10};
    box boxcancel = {x1 + 121, (y2 + y1) / 2 - 10, x1 + 121 + 70, (y2 + y1) / 2 + 10};
    box boxnew = {x1 + 121, (y2 + y1) / 2 - 40, x1 + 121 + 70, (y2 + y1) / 2 - 20};

    int felso = 0;
    int eddigfelso = -1;

    int eddigkur = 0;
    mk_emptychar();
    int egerballevoltnyomva = 0;
    while (1) {
        // Nezzuk gombokat:
        while (mk_kbhit()) {
            int c = mk_getextchar();
            if (c == MK_ESC) {
                return;
            }
            if (c == MK_ENTER) {
                igazit(&kur, &felso, nevszam, helyszam);
                char* nev = &Soknev[kur * 10];
                if (strlen(nev) > 8) {
                    hiba("iovfgsdgffiufi");
                }
                strcpy(State->editfilenev, nev);
                strcat(State->editfilenev, ".lev");
                return;
            }
            if (c == MK_UP) {
                kur--;
            }
            if (c == MK_PGUP) {
                kur -= helyszam - 1;
            }
            if (c == MK_DOWN) {
                kur++;
            }
            if (c == MK_PGDOWN) {
                kur += helyszam - 1;
            }
        }
        // Eger lenyomas:
        int balnyomva = 0;
        if (getbutbmou() && !egerballevoltnyomva) {
            egerballevoltnyomva = 1;
            balnyomva = 1;
        } else {
            egerballevoltnyomva = 0;
        }
        if (balnyomva) {
            int ujx = 0, ujy = 0;
            getmou(&ujx, &ujy);
            Moux = ujx;
            Mouy = ujy;
            // Most nezzuk meg, hogy hova kattintott:
            if (boxban(ujx, ujy, boxle)) {
                kur += helyszam - 1;
            }
            if (boxban(ujx, ujy, boxfel)) {
                kur -= helyszam - 1;
            }
            if (boxban(ujx, ujy, boxcancel)) {
                return;
            }
            if (boxban(ujx, ujy, boxnew)) {
                State->editfilenev[0] = 0;
                return;
            }
            if (ujx > lx1 && ujx < lx2 && ujy > ly1 && ujy < ly2) {
                // Lista folott nyomta meg gombot:
                if (ujy < ly1 + dy * helyszam) {
                    // Egy nevet el is talalt:
                    int index = (ujy - ly1) / dy;
                    index += felso;
                    if (index < nevszam) {
                        char* nev = &Soknev[index * 10];
                        if (strlen(nev) > 8) {
                            hiba("iovfgsdgffiufi");
                        }
                        strcpy(State->editfilenev, nev);
                        strcat(State->editfilenev, ".lev");
                        return;
                    }
                }
            }
        }
        igazit(&kur, &felso, nevszam, helyszam);
        if (felso != eddigfelso || kur != eddigkur) {
            // Kirajzoljuk, mivel valtozott kep:
            push();
            kirajzbox(Buffsima, x1, y1, x2, y2, Dialogszin, Dialogszelszin);
            kirajzbox(Buffsima, lx1, ly1, lx2, ly2, Openlistaszin, Dialogszelszin);
            for (int i = 0; i < helyszam && i < nevszam; i++) {
                if (i + felso == kur) {
                    // Ez most kurens nev:
                    Buffsima->fillbox(lx1 + 1, ly1 + i * dy + 1, lx2 - 1, ly1 + (i + 1) * dy - 1,
                                      Openlistainvszin);
                }
                Pabc2->write(Buffsima, lx1 + 3, ly1 + 15 + i * dy, &Soknev[(i + felso) * 10]);
            }
            // Most jonnek gombok:
            kirajzbox(Buffsima, boxfel, Buttonszin, Dialogszelszin);
            boxbarajz(Buffsima, boxfel, Dialogszelszin, 1);
            kirajzbox(Buffsima, boxle, Buttonszin, Dialogszelszin);
            boxbarajz(Buffsima, boxle, Dialogszelszin, 0);
            kirajzbox(Buffsima, boxcancel, Buttonszin, Dialogszelszin);
            Pabc2->writekozep(Buffsima, (boxcancel.x1 + boxcancel.x2) / 2, boxcancel.y1 + 15,
                              "CANCEL");
            kirajzbox(Buffsima, boxnew, Buttonszin, Dialogszelszin);
            Pabc2->writekozep(Buffsima, (boxnew.x1 + boxnew.x2) / 2, boxnew.y1 + 15, "NEW");

            bltfront(Buffsima, x1, y1, x2, y2);
            pop();

            eddigfelso = felso;
            eddigkur = kur;
        }
        int ujx = 0, ujy = 0;
        getmou(&ujx, &ujy);
        if (ujx != Moux && ujy != Mouy) {
            push();
            Moux = ujx;
            Mouy = ujy;
            pop();
        }
    }
}

void tolt_new(void) {
    invalidateegesz();
    if (Valtozott) {
        if (dialog("There are unsaved changes in the level file.", "Do you still want to continue?",
                   "GOMBOK", "Yes", "No") == 1) {
            return;
        }
    }

    Valtozott = 0;
    floadlevel_e("_uj_topol_");
    State->editfilenev[0] = 0;
    kitolt();
}

// Ha sikeresen mentett, akkor igaz:
int tolt_save(int lokkolhato) {
    invalidateegesz();
    if (State->editfilenev[0] == 0) {
        // Ez a file meg nincsen elnevezve:
        return tolt_save_as(lokkolhato);
    }
    Ptop->save(State->editfilenev);
    Valtozott = 0;
    return 1;
}

// Ha sikeresen mentett, akkor igaz:
int tolt_save_as(int lokkolhato) {
    invalidateegesz();
    // Itt most be kell olvasnunk egy nevet:
    box boxnagy = {20, 200, 600, 380};
    if (!lokkolhato) {
        boxnagy.y2 = 300;
    }
    box boxsave = {320 - 30, 273, 320 + 30, 290};
    box boxsavelock = {320 - 45, 340, 320 + 45, 357};

    push();
    kirajzbox(Buffsima, boxnagy, Dialogszin, Dialogszelszin);
    kirajzbox(Buffsima, boxsave, Buttonszin, Dialogszelszin);
    if (lokkolhato) {
        kirajzbox(Buffsima, boxsavelock, Buttonszin, Dialogszelszin);
    }
    Pabc2->writekozep(
        Buffsima, (boxnagy.x2 + boxnagy.x1) / 2, 220,
        "Type in the file name and press ENTER or click on SAVE, or press ESC to cancel!");
    if (lokkolhato) {
        Pabc2->writekozep(
            Buffsima, (boxnagy.x2 + boxnagy.x1) / 2, 320,
            "If you click on the SAVE LOCKED button, the saved file won't be modifiable anymore. ");
    }
    Pabc2->writekozep(Buffsima, (boxsave.x2 + boxsave.x1) / 2, boxsave.y1 + 13, "SAVE");
    if (lokkolhato) {
        Pabc2->writekozep(Buffsima, (boxsavelock.x2 + boxsavelock.x1) / 2, boxsavelock.y1 + 13,
                          "SAVE LOCKED");
    }

    bltfront(Buffsima, boxnagy.x1, boxnagy.y1, boxnagy.x2, boxnagy.y2);
    pop();

    mk_emptychar();
    char nev[10] = "";
    int i = 0;
    int egerballevoltnyomva = 0;
    int egerjobblevoltnyomva = 0;
    char eddignev[10] = "$%...%^&";
    while (1) {
        if (strcmp(eddignev, nev) != 0) {
            strcpy(eddignev, nev);

            int ex1 = boxnagy.x1 + 100;
            int ey1 = boxnagy.y1 + 30;
            int ex2 = boxnagy.x2 - 100;
            int ey2 = boxnagy.y1 + 55;
            Buffsima->fillbox(ex1, ey1, ex2, ey2, Dialogszin);
            Pabc2->write(Buffsima, 290, 250, nev);
            char tmp[40];
            strcpy(tmp, nev);
            tmp[i] = 0;
            Pabc2->write(Buffsima, 290 + Pabc1->len(tmp), 255, "-");
            bltfront(Buffsima, ex1, ey1, ex2, ey2);
        }
        while (mk_kbhit()) {
            int c = mk_getextchar();
            if (c == MK_ESC) {
                return 0;
            }
            if (c == MK_ENTER) {
                if (nev[0] == 0) {
                    continue;
                }
                // Most mentunk:
                // Eloallitjuk nevet:
                strcat(nev, ".lev");
                char tmpnev[40];
                strcpy(tmpnev, "lev\\");
                strcat(tmpnev, nev);
                if (access(tmpnev, 0) == 0) {
                    if (dialog("File exists, overwrite?", nev, "GOMBOK", "Yes", "No") == 1) {
                        return 0;
                    }
                }
                Ptop->save(nev);
                strcpy(State->editfilenev, nev);
                Valtozott = 0;
                return 1;
            }
            if ((c >= 'A' && c <= 'Z') || (c >= 'a' && c <= 'z') || (c >= '0' && c <= '9')) {
                if (strlen(nev) >= 8) {
                    continue;
                }
                // c-t beillesztjuk i. helyre:
                int hossz = strlen(nev);
                for (int j = hossz; j >= i; j--) {
                    nev[j + 1] = nev[j];
                }
                nev[i] = (char)c;
                i++;
            }
            if (c == MK_BACKSPACE) {

                // Torles:
                int ikesebbnulla = 0;
                if (i <= 0) {
                    ikesebbnulla = 0;
                }
                if ((nev[0] == 0) || ikesebbnulla) {
                    continue;
                }
                // Kiszedjuk i-1-edik helyrol karaktert:
                int hossz = strlen(nev);
                for (int j = i - 1; j < hossz; j++) {
                    nev[j] = nev[j + 1];
                }
                i--;
            }
            if (c == MK_DEL) {
                // Torles:
                if ((nev[0] == 0) || (i >= (int)strlen(nev))) {
                    continue;
                }
                // Kiszedjuk i-1-edik helyrol karaktert:
                int hossz = strlen(nev);
                for (int j = i; j < hossz; j++) {
                    nev[j] = nev[j + 1];
                }
            }
            if (c == MK_LEFT && i > 0) {
                i--;
                eddignev[0] = 0;
            }
            if (c == MK_RIGHT && i < (int)strlen(nev)) {
                i++;
                eddignev[0] = 0;
            }
        }
        // Eger lenyomas:
        int balnyomva = 0;
        if (getbutbmou() && !egerballevoltnyomva) {
            egerballevoltnyomva = 1;
            balnyomva = 1;
        } else {
            egerballevoltnyomva = 0;
        }

        int jobbnyomva = 0;
        if (getbutjmou() && !egerjobblevoltnyomva) {
            egerjobblevoltnyomva = 1;
            jobbnyomva = 1;
        } else {
            egerjobblevoltnyomva = 0;
        }

        if (jobbnyomva) {
            return 0;
        }

        if (balnyomva) {
            int ujx = 0, ujy = 0;
            getmou(&ujx, &ujy);
            push();
            Moux = ujx;
            Mouy = ujy;
            pop();
            // Most nezzuk meg, hogy hova kattintott:
            if (boxban(ujx, ujy, boxsave)) {
                if (nev[0] == 0) {
                    continue;
                }
                // Most mentunk:
                // Eloallitjuk nevet:
                strcat(nev, ".lev");
                if (access(nev, 0) == 0) {
                    if (dialog("File exists, overwrite?", nev, "GOMBOK", "Yes", "No") == 1) {
                        continue;
                    }
                }
                Ptop->save(nev);
                strcpy(State->editfilenev, nev);
                Valtozott = 0;
                return 1;
            }
            if (lokkolhato && boxban(ujx, ujy, boxsavelock)) {
                // Lockoltan mentjuk el:
                if (nev[0] == 0) {
                    continue;
                }
                // Most mentunk:
                // Eloallitjuk nevet:
                strcat(nev, ".lev");
                if (strcmpi(nev, State->editfilenev) == 0) {
                    dialog(
                        "You must give a name that is different from the name of the edited file!");
                    return 0;
                }
                if (dialog(
                        "WARNING: The file you save will be locked. You won't be able to load it "
                        "into the editor!",
                        "You should save a locked file only if you want to give it to somebody",
                        "and don't want him/her to modify your level file.",
                        "It is recommended that you save and keep the original (not locked) file",
                        "in case you want to modify it.",
                        "After saving the file, the name of the content of the editor will remain "
                        "the original.",
                        "", "Do you want to continue?",

                        "GOMBOK", "Yes", "No") == 1) {
                    return 0;
                }
                if (access(nev, 0) == 0) {
                    if (dialog("File exists, overwrite?", nev, "GOMBOK", "Yes", "No") == 1) {
                        return 0;
                    }
                }
                Ptop->lezart = 1;
                Ptop->save(nev);
                Ptop->lezart = 0;
                // strcpy( State->editfilenev, nev );
                return 1;
            }
        }
        int ujx = 0, ujy = 0;
        getmou(&ujx, &ujy);
        if (ujx != Moux && ujy != Mouy) {
            push();
            Moux = ujx;
            Mouy = ujy;
            pop();
        }
    }
}

static unsigned char Szinkonvtomb[256];

static void makeszinkonvtomb(unsigned char* pal) {
    for (int i = 0; i < 256; i++) {
        int r = pal[i * 3];
        int g = pal[i * 3 + 1];
        int b = pal[i * 3 + 2];
        r >>= 4;
        g >>= 4;
        b >>= 4;
        int szin = r * 16 + g * 4 + b;
        if (szin < 0 || szin >= 64) {
            hiba("897 648u9");
        }
        Szinkonvtomb[i] = (unsigned char)(szin + 64);
        // Szinkonvtomb[i] = (unsigned char)(63 + 64);
    }
}

// Szimpla
static void vegyesblt(pic8* dest, kep* pkep, int x0, int y0) {
    if (x0 < 0 || y0 < 0 || x0 >= dest->getxsize() || y0 >= dest->getysize()) {
        hiba("7r79yffuew");
    }

    int xsize = pkep->xsize;
    int ysize = pkep->ysize;
    int xtullog = 0;
    if (x0 + xsize > dest->getxsize()) {
        xsize = dest->getxsize() - x0;
        xtullog = 1;
    }
    if (y0 + ysize > dest->getysize()) {
        ysize = dest->getysize() - y0;
    }
    // Most xsize es ysize mindenkeppen benne van teljesen kepben:

    int i = 0;
    unsigned char* tomb = pkep->adatok;
    for (int y = y0; y < y0 + ysize; y++) {
        // Egy sor elintezese:
        unsigned char* dsor = dest->getptr(y) + x0;

        int x = 0;
        while (1) {
            // Ures elintezes:
            int uresszam = tomb[i] * 256 + tomb[i + 1];
            i += 2;

            if (uresszam > 60000) {
                break;
            }

            x += uresszam;

            // Teli elintezes:
            int teliszam = tomb[i] * 256 + tomb[i + 1];
            i += 2;

            // Johet konverzio:
            if (xtullog) {
                for (int dx = 0; dx < teliszam; dx++) {
                    if (x + dx < xsize) {
                        dsor[x + dx] = Szinkonvtomb[tomb[i + dx]];
                    }
                }
            } else {
                for (int dx = 0; dx < teliszam; dx++) {
                    dsor[x + dx] = Szinkonvtomb[tomb[i + dx]];
                }
            }

            x += teliszam;
            i += teliszam;
        }
    }
}

// Maszkos:
static void vegyesblt(pic8* dest, textura* pt, maszk* pm, int x0, int y0) {
    if (x0 < 0 || y0 < 0 || x0 >= dest->getxsize() || y0 >= dest->getysize()) {
        hiba("7r79yffuew");
    }

    pic8* source = pt->ppic;

    int maxysize = pm->ysize;
    if (y0 + maxysize > dest->getysize()) {
        maxysize = dest->getysize() - y0;
    }
    // Most xsize es ysize mindenkeppen benne van teljesen kepben:
    int i = 0;
    int picxsize = source->getxsize();
    for (int y = y0; y < y0 + maxysize; y++) {
        unsigned char* dsor = dest->getptr(y) + x0;
        unsigned char* ssor = source->getptr((y - y0) % source->getysize());
        int sx = 0;
        int abszx = x0;
        // Maszkos eljaras:
        while (pm->adatok[i].tipus != ME_SOREMELES) {
            if (pm->adatok[i].tipus == ME_TELI) {
                for (int j = 0; j < pm->adatok[i].hossz; j++) {
                    if (abszx < 640) {
                        *dsor = Szinkonvtomb[ssor[sx]];
                    }
                    dsor++;
                    sx++;
                    sx %= picxsize;
                    abszx++;
                }
            } else {
                dsor += pm->adatok[i].hossz;
                sx += pm->adatok[i].hossz;
                sx %= picxsize;
                abszx += pm->adatok[i].hossz;
            }
            i++;
        }
        i++;
    }
}

// tipus 1-tol 4-ig (4. koveto file):
static void pickanev(char* kepnev, char* texturanev, char* maszknev, int tipus, int cleares = 0) {
    char* nev = NULL;
    int nevszam = 0;
    if (tipus == 1) {
        nev = kepnev;
        nevszam = Plgr->kepszam;
    }
    if (tipus == 2) {
        nev = texturanev;
        nevszam = Plgr->texturaszam;
        if (nevszam >= 1 && Plgr->opentextkimarad) {
            // qgrass-t nem jelenitjuk meg (lgr beolvaso lista vegere teszi):
            nevszam--;
        }
    }
    if (tipus == 3) {
        nev = maszknev;
        nevszam = Plgr->maszkszam;
    }

    if (!nev) {
        hiba("liytg");
    }

    if (nevszam < 1) {
        return;
    }

    char orignev[10];
    if (strlen(nev) > 8) {
        hiba("8guiojg");
    }
    strcpy(orignev, nev);

    int kur = 0; // Ha megtalalja eddigit, akkor kur-t raallitja
    for (int i = 0; i < nevszam; i++) {
        if (tipus == 1 && strcmpi(Plgr->kepek[i].nev, nev) == 0) {
            kur = i;
        }
        if (tipus == 2 && strcmpi(Plgr->texturak[i].nev, nev) == 0) {
            kur = i;
        }
        if (tipus == 3 && strcmpi(Plgr->maszkok[i].nev, nev) == 0) {
            kur = i;
        }
    }

    if (kur == 0) {
        if (tipus == 1) {
            strcpy(nev, Plgr->kepek[0].nev);
        }
        if (tipus == 2) {
            strcpy(nev, Plgr->texturak[0].nev);
        }
        if (tipus == 3) {
            strcpy(nev, Plgr->maszkok[0].nev);
        }
    }

    // Koordok megadasa:
    int helyszam = 10;
    int dy = 20;
    int x1 = 200;
    int y1 = 100;
    int x2 = 401;
    int y2 = 174 + helyszam * dy;
    int lx1 = x1 + 10;
    int ly1 = y1 + 37;
    int lx2 = lx1 + 100;
    int ly2 = ly1 + helyszam * dy;

    box boxfel = {x1 + 10, y1 + 11, x1 + 110, y1 + 31};
    box boxle = {x1 + 10, y2 - 30, x1 + 110, y2 - 10};
    box boxcancel = {x1 + 121, (y2 + y1) / 2 - 10, x1 + 121 + 70, (y2 + y1) / 2 + 10};
    box boxclear = {x1 + 121, y1 + 30, x1 + 121 + 70, y1 + 50};

    int felso = 0;
    int eddigfelso = -1;
    int eddigkur = 0;
    mk_emptychar();

    // Masodik bufferben mindig ilyenkor benne van eredeti kep:
    int buffervaltozottx = 0;
    int buffervaltozotty = 0;
    push();
    blt8(Buffgolyo, Buffsima);
    // Buffgolyo->save( "golyo.pcx" );
    // hiba( "golyo mentve!" );
    pop();

    // Azert hogy ha mar le volt nyomva, ne nyomodjon ujra:
    int egerballevoltnyomva = 1;
    while (1) {
        // Nezzuk gombokat:
        while (mk_kbhit()) {
            int c = mk_getextchar();
            if (c == MK_ESC) {
                strcpy(nev, orignev);
                return;
            }
            if (c == MK_ENTER) {
                igazit(&kur, &felso, nevszam, helyszam);
                if (tipus == 1) {
                    strcpy(nev, Plgr->kepek[kur].nev);
                }
                if (tipus == 2) {
                    strcpy(nev, Plgr->texturak[kur].nev);
                }
                if (tipus == 3) {
                    strcpy(nev, Plgr->maszkok[kur].nev);
                }
                return;
            }
            if (c == MK_UP) {
                kur--;
            }
            if (c == MK_PGUP) {
                kur -= helyszam - 1;
            }
            if (c == MK_DOWN) {
                kur++;
            }
            if (c == MK_PGDOWN) {
                kur += helyszam - 1;
            }
        }
        // Eger lenyomas:
        int balnyomva = 0;
        if (getbutbmou()) {
            if (!egerballevoltnyomva) {
                balnyomva = 1;
            }
            egerballevoltnyomva = 1;
        } else {
            egerballevoltnyomva = 0;
        }
        if (balnyomva) {
            int ujx = 0, ujy = 0;
            getmou(&ujx, &ujy);
            push();
            Moux = ujx;
            Mouy = ujy;
            pop();
            // Most nezzuk meg, hogy hova kattintott:
            if (boxban(ujx, ujy, boxle)) {
                kur += helyszam - 1;
            }
            if (boxban(ujx, ujy, boxfel)) {
                kur -= helyszam - 1;
            }
            if (boxban(ujx, ujy, boxcancel)) {
                strcpy(nev, orignev);
                return;
            }
            if (cleares && boxban(ujx, ujy, boxclear)) {
                nev[0] = 0;
                return;
            }
            if (ujx > lx1 && ujx < lx2 && ujy > ly1 && ujy < ly2) {
                // Lista folott nyomta meg gombot:
                if (ujy < ly1 + dy * helyszam) {
                    // Egy nevet el is talalt:
                    int index = (ujy - ly1) / dy;
                    index += felso;
                    if (index < nevszam) {
                        if (tipus == 1) {
                            strcpy(nev, Plgr->kepek[index].nev);
                        }
                        if (tipus == 2) {
                            strcpy(nev, Plgr->texturak[index].nev);
                        }
                        if (tipus == 3) {
                            strcpy(nev, Plgr->maszkok[index].nev);
                        }
                        return;
                    }
                }
            }
        }
        igazit(&kur, &felso, nevszam, helyszam);
        if (felso != eddigfelso || kur != eddigkur) {
            // Rajzolunk:

            // Nevet beallitjuk kur-nak megfeleloen:
            if (tipus == 1) {
                strcpy(nev, Plgr->kepek[kur].nev);
            }
            if (tipus == 2) {
                strcpy(nev, Plgr->texturak[kur].nev);
            }
            if (tipus == 3) {
                strcpy(nev, Plgr->maszkok[kur].nev);
            }

            push();

            blt8(Buffsima, Buffgolyo, 0, 0, 0, 0, buffervaltozottx, buffervaltozotty);

            if (kepnev[0] && (texturanev[0] || maszknev[0])) {
                hiba("iugriurg");
            }

            if (kepnev[0]) {
                // Ez egy kep:
                int index = Plgr->getkepindex(kepnev);
                if (index < 0) {
                    hiba("hjguyt");
                }
                kep* pkep = &Plgr->kepek[index];
                vegyesblt(Buffsima, pkep, 10, 10);
                buffervaltozottx = 10 + pkep->xsize - 1;
                buffervaltozotty = 10 + pkep->ysize - 1;
            }

            if (texturanev[0] && maszknev[0]) {
                // Ez egy texturas maszk:
                int index = Plgr->gettexturaindex(texturanev);
                if (index < 0) {
                    hiba(";lgrtijo");
                }
                textura* ptextura = &Plgr->texturak[index];

                index = Plgr->getmaszkindex(maszknev);
                if (index < 0) {
                    hiba("lkjefrg");
                }
                maszk* pmaszk = &Plgr->maszkok[index];

                vegyesblt(Buffsima, ptextura, pmaszk, 10, 10);
                buffervaltozottx = 10 + pmaszk->xsize - 1;
                buffervaltozotty = 10 + pmaszk->ysize - 1;
            }

            kirajzbox(Buffsima, x1, y1, x2, y2, Dialogszin, Dialogszelszin);
            kirajzbox(Buffsima, lx1, ly1, lx2, ly2, Openlistaszin, Dialogszelszin);
            for (int i = 0; i < helyszam && i + felso < nevszam; i++) {
                if (i + felso == kur) {
                    // Ez most kurens nev:
                    Buffsima->fillbox(lx1 + 1, ly1 + i * dy + 1, lx2 - 1, ly1 + (i + 1) * dy - 1,
                                      Openlistainvszin);
                }
                // Kiirjuk nevet:
                if (tipus == 1) {
                    Pabc2->write(Buffsima, lx1 + 3, ly1 + 15 + i * dy, Plgr->kepek[i + felso].nev);
                }
                if (tipus == 2) {
                    Pabc2->write(Buffsima, lx1 + 3, ly1 + 15 + i * dy,
                                 Plgr->texturak[i + felso].nev);
                }
                if (tipus == 3) {
                    Pabc2->write(Buffsima, lx1 + 3, ly1 + 15 + i * dy,
                                 Plgr->maszkok[i + felso].nev);
                }

                // Kiirjuk tavot:
                if (kepnev[0] || texturanev[0]) {
                    int tavolsag = 0;
                    if (kepnev[0]) {
                        tavolsag = Plgr->kepek[i + felso].tavolsag;
                    } else {
                        tavolsag = Plgr->texturak[i + felso].tavolsag;
                    }

                    char tavstr[20];
                    sprintf(tavstr, "%d", tavolsag);
                    Pabc2->write(Buffsima, lx1 + 67, ly1 + 15 + i * dy, tavstr);
                    // Kiirjuk hatarolokat:
                    int hatarol = 0;
                    if (kepnev[0]) {
                        hatarol = Plgr->kepek[i + felso].hatarol;
                    } else {
                        hatarol = Plgr->texturak[i + felso].hatarol;
                    }

                    strcpy(tavstr, "U");
                    if (hatarol == HATAROL_G) {
                        strcpy(tavstr, "G");
                    }
                    if (hatarol == HATAROL_S) {
                        strcpy(tavstr, "S");
                    }
                    Pabc2->write(Buffsima, lx1 + 92, ly1 + 15 + i * dy, tavstr);
                }
            }
            // Most jonnek gombok:
            kirajzbox(Buffsima, boxfel, Buttonszin, Dialogszelszin);
            boxbarajz(Buffsima, boxfel, Dialogszelszin, 1);
            kirajzbox(Buffsima, boxle, Buttonszin, Dialogszelszin);
            boxbarajz(Buffsima, boxle, Dialogszelszin, 0);
            kirajzbox(Buffsima, boxcancel, Buttonszin, Dialogszelszin);
            Pabc2->writekozep(Buffsima, (boxcancel.x1 + boxcancel.x2) / 2, boxcancel.y1 + 15,
                              "CANCEL");
            if (cleares) {
                kirajzbox(Buffsima, boxclear, Buttonszin, Dialogszelszin);
                Pabc2->writekozep(Buffsima, (boxclear.x1 + boxclear.x2) / 2, boxclear.y1 + 15,
                                  "CLEAR");
            }

            bltfront(Buffsima);
            pop();

            eddigfelso = felso;
            eddigkur = kur;
        }
        int ujx = 0, ujy = 0;
        getmou(&ujx, &ujy);
        if (ujx != Moux && ujy != Mouy) {
            push();
            Moux = ujx;
            Mouy = ujy;
            pop();
        }
    }
}

void tolt_pickasprite(void) {
    if (!Plgr) {
        hiba("tolt_pickasprite-ban !Plgr!");
    }

    makeszinkonvtomb(Plgr->paltomb);

    invalidateegesz();

    char kepnev[10], texturanev[10], maszknev[10], uresnev[10] = "";
    strcpy(kepnev, Plgr->aktiv_kepnev);
    strcpy(texturanev, Plgr->aktiv_texturanev);
    strcpy(maszknev, Plgr->aktiv_maszknev);

    // Koordok megadasa:
    int x1 = 160;
    int y1 = 100;
    int x2 = 390; // 340;
    int y2 = y1 + 185;

    int boxx1 = x1 + 120; // 70;
    box boxkepnev = {boxx1, y1 + 40, boxx1 + 100, y1 + 40 + 20};
    box boxtexturanev = {boxx1, y1 + 70, boxx1 + 100, y1 + 70 + 20};
    box boxmaszknev = {boxx1, y1 + 100, boxx1 + 100, y1 + 100 + 20};

    box boxok = {x1 + 50, y2 - 30, x1 + 100, y2 - 10};
    box boxcancel = {x1 + 130, y2 - 30, x1 + 180, y2 - 10};

    mk_emptychar();

    int egerballevoltnyomva = 1;
    int invalid = 1;
    while (1) {
        // Nezzuk gombokat:
        while (mk_kbhit()) {
            int c = mk_getextchar();
            if (c == MK_ESC) {
                return;
            }
            if (c == MK_ENTER) {
                strcpy(Plgr->aktiv_kepnev, kepnev);
                strcpy(Plgr->aktiv_texturanev, texturanev);
                strcpy(Plgr->aktiv_maszknev, maszknev);
                return;
            }
        }
        // Eger lenyomas:
        int balnyomva = 0;
        if (getbutbmou()) {
            if (!egerballevoltnyomva) {
                balnyomva = 1;
            }
            egerballevoltnyomva = 1;
        } else {
            egerballevoltnyomva = 0;
        }
        if (balnyomva) {
            int ujx = 0, ujy = 0;
            getmou(&ujx, &ujy);
            push();
            Moux = ujx;
            Mouy = ujy;
            pop();
            // Most nezzuk meg, hogy hova kattintott:
            if (boxban(ujx, ujy, boxok)) {
                strcpy(Plgr->aktiv_kepnev, kepnev);
                strcpy(Plgr->aktiv_texturanev, texturanev);
                strcpy(Plgr->aktiv_maszknev, maszknev);
                return;
            }
            if (boxban(ujx, ujy, boxcancel)) {
                return;
            }
            if (boxban(ujx, ujy, boxkepnev)) {
                // Kepnevet akarja valtoztatni:
                pickanev(kepnev, uresnev, uresnev, 1);
                push();
                blt8(Buffsima, Buffgolyo);
                bltfront(Buffsima);
                pop();
                if (kepnev[0]) {
                    texturanev[0] = maszknev[0] = 0;
                }
                invalid = 1;
            }
            if (boxban(ujx, ujy, boxtexturanev)) {
                // Texturanevet akarja valtoztatni:
                pickanev(uresnev, texturanev, maszknev, 2);
                push();
                blt8(Buffsima, Buffgolyo);
                bltfront(Buffsima);
                pop();
                if (texturanev[0]) {
                    kepnev[0] = 0;
                }
                invalid = 1;
            }
            if (boxban(ujx, ujy, boxmaszknev)) {
                // Maszknevet akarja valtoztatni:
                pickanev(uresnev, texturanev, maszknev, 3);
                push();
                blt8(Buffsima, Buffgolyo);
                bltfront(Buffsima);
                pop();
                if (maszknev[0]) {
                    kepnev[0] = 0;
                }
                invalid = 1;
            }
        }
        if (invalid) {
            // RAJZOLAS:
            invalid = 0;

            // Kirajzoljuk, mivel valtozott kep:
            push();

            // Dialogus nagy teglalapja:
            kirajzbox(Buffsima, x1, y1, x2, y2, Dialogszin, Dialogszelszin);

            Pabc2->writekozep(Buffsima, (x1 + x2) / 2, y1 + 20, "Choose picture");

            // Most jonnek gombok:
            kirajzbox(Buffsima, boxok, Buttonszin, Dialogszelszin);
            kirajzbox(Buffsima, boxcancel, Buttonszin, Dialogszelszin);
            Pabc2->writekozep(Buffsima, (boxok.x1 + boxok.x2) / 2, boxok.y1 + 15, "OK");
            Pabc2->writekozep(Buffsima, (boxcancel.x1 + boxcancel.x2) / 2, boxcancel.y1 + 15,
                              "CANCEL");

            // Most jonnek nevek:
            // Picture:
            int szovx1 = x1 + 8;
            Pabc2->write(Buffsima, szovx1, boxkepnev.y1 + 15, "Normal Picture:");
            kirajzbox(Buffsima, boxkepnev, Feherszin, Dialogszelszin);
            boxbair_balra(Buffsima, boxkepnev, Feherszin, kepnev);
            // Textura:
            Pabc2->write(Buffsima, szovx1, boxtexturanev.y1 + 15, "Texture:");
            kirajzbox(Buffsima, boxtexturanev, Feherszin, Dialogszelszin);
            boxbair_balra(Buffsima, boxtexturanev, Feherszin, texturanev);
            // Maszk:
            Pabc2->write(Buffsima, szovx1, boxmaszknev.y1 + 15, "Mask:");
            kirajzbox(Buffsima, boxmaszknev, Feherszin, Dialogszelszin);
            boxbair_balra(Buffsima, boxmaszknev, Feherszin, maszknev);

            bltfront(Buffsima);
            pop();
        }
        int ujx = 0, ujy = 0;
        getmou(&ujx, &ujy);
        if (ujx != Moux && ujy != Mouy) {
            push();
            Moux = ujx;
            Mouy = ujy;
            pop();
        }
    }
}

int tolt_picklgrfile(char* lgrnev) {
    invalidateegesz();
    blt8(Buffgolyo, Buffsima);
    if (!Soknev) {
        Soknev = new char[Maxnev * 10 + 100];
        if (!Soknev) {
            hiba("Nincs eleg memoria tolt_picklgrfile-ban!");
        }
    }
    for (int i = 0; i < Maxnev * 10 + 10; i++) {
        Soknev[i] = 0;
    }
    finame fname;
    int done = fifirst("lgr\\*.lgr", fname);
    int nevszam = 0;
    while (!done) {
        if (strlen(fname) > 14) {
            hiba("876759989");
        }
        char* nev = &Soknev[nevszam * 10];
        strcpy(nev, fname);
        // Leveszi kiterjesztest:
        int i = 0;
        while (nev[i] != '.') {
            if (!nev[i]) {
                hiba("Nincs pont nevben! (454534): ", nev);
            }
            i++;
        }
        nev[i] = 0;
        if (strlen(nev) > 8) {
            hiba("3875665");
        }

        done = finext(fname);
        nevszam++;
        if (nevszam >= Maxnev) {
            dialog("Number of files > 1000 in Pick LGR File dialog box!",
                   "Could not load all the files!");
            done = 1;
        }
    }

    ficlose();

    if (nevszam < 1) {
        uzenet("There is not any LGR files (*.lgr) in LGR directory!");
    }

    abcberendez(nevszam);

    int kur = 0; // Ha megtalalja eddigit, akkor kur-t raallitja
    for (int i = 0; i < nevszam; i++) {
        if (strcmpi(&Soknev[i * 10], lgrnev) == 0) {
            kur = i;
        }
    }

    // Koordok megadasa:
    int helyszam = 10;
    int dy = 20;
    int x1 = 200;
    int y1 = 100;
    int x2 = 401;
    int dyorigszov = 40;
    int y2 = 174 + helyszam * dy + dyorigszov;
    int lx1 = x1 + 10;
    int ly1 = y1 + 37;
    int lx2 = lx1 + 100;
    int ly2 = ly1 + helyszam * dy;

    box boxfel = {x1 + 10, y1 + 11, x1 + 110, y1 + 31};
    box boxle = {x1 + 10, y2 - 30 - dyorigszov, x1 + 110, y2 - 10 - dyorigszov};
    box boxcancel = {x1 + 121, (y2 + y1) / 2 - 10, x1 + 121 + 70, (y2 + y1) / 2 + 10};

    int felso = 0;
    int eddigfelso = -1;
    int eddigkur = 0;
    mk_emptychar();
    int egerballevoltnyomva = 1;
    while (1) {
        // Nezzuk gombokat:
        while (mk_kbhit()) {
            int c = mk_getextchar();
            if (c == MK_ESC) {
                return 0;
            }
            if (c == MK_ENTER) {
                igazit(&kur, &felso, nevszam, helyszam);
                strcpy(lgrnev, &Soknev[kur * 10]);
                return 1;
            }
            if (c == MK_UP) {
                kur--;
            }
            if (c == MK_PGUP) {
                kur -= helyszam - 1;
            }
            if (c == MK_DOWN) {
                kur++;
            }
            if (c == MK_PGDOWN) {
                kur += helyszam - 1;
            }
        }

        // Eger lenyomas:
        int balnyomva = 0;
        if (getbutbmou()) {
            if (!egerballevoltnyomva) {
                balnyomva = 1;
            }
            egerballevoltnyomva = 1;
        } else {
            egerballevoltnyomva = 0;
        }

        if (balnyomva) {
            int ujx = 0, ujy = 0;
            getmou(&ujx, &ujy);
            push();
            Moux = ujx;
            Mouy = ujy;
            pop();
            // Most nezzuk meg, hogy hova kattintott:
            if (boxban(ujx, ujy, boxle)) {
                kur += helyszam - 1;
            }
            if (boxban(ujx, ujy, boxfel)) {
                kur -= helyszam - 1;
            }
            if (boxban(ujx, ujy, boxcancel)) {
                return 0;
            }
            if (ujx > lx1 && ujx < lx2 && ujy > ly1 && ujy < ly2) {
                // Lista folott nyomta meg gombot:
                if (ujy < ly1 + dy * helyszam) {
                    // Egy nevet el is talalt:
                    int index = (ujy - ly1) / dy;
                    index += felso;
                    if (index < nevszam) {
                        strcpy(lgrnev, &Soknev[index * 10]);
                        return 1;
                    }
                }
            }
        }
        igazit(&kur, &felso, nevszam, helyszam);
        if (felso != eddigfelso || kur != eddigkur) {
            // Kirajzoljuk, mivel valtozott kep:
            push();
            kirajzbox(Buffsima, x1, y1, x2, y2, Dialogszin, Dialogszelszin);
            kirajzbox(Buffsima, lx1, ly1, lx2, ly2, Openlistaszin, Dialogszelszin);
            for (int i = 0; i < helyszam && i < nevszam; i++) {
                if (i + felso == kur) {
                    // Ez most kurens nev:
                    Buffsima->fillbox(lx1 + 1, ly1 + i * dy + 1, lx2 - 1, ly1 + (i + 1) * dy - 1,
                                      Openlistainvszin);
                }
                Pabc2->write(Buffsima, lx1 + 3, ly1 + 15 + i * dy, &Soknev[(i + felso) * 10]);
            }
            // Most jonnek gombok:
            kirajzbox(Buffsima, boxfel, Buttonszin, Dialogszelszin);
            boxbarajz(Buffsima, boxfel, Dialogszelszin, 1);
            kirajzbox(Buffsima, boxle, Buttonszin, Dialogszelszin);
            boxbarajz(Buffsima, boxle, Dialogszelszin, 0);
            kirajzbox(Buffsima, boxcancel, Buttonszin, Dialogszelszin);
            Pabc2->writekozep(Buffsima, (boxcancel.x1 + boxcancel.x2) / 2, boxcancel.y1 + 15,
                              "CANCEL");

            // Kiirjuk eredeti file nevet:
            char tttmp[50];
            strcpy(tttmp, lgrnev);
            // strcat( tttmp, ".LGR" );
            Pabc2->writekozep(Buffsima, (x1 + x2) / 2, y2 - 36, "Original LGR file:");
            Pabc2->writekozep(Buffsima, (x1 + x2) / 2, y2 - 18, tttmp);

            bltfront(Buffsima, x1, y1, x2, y2);
            pop();

            eddigfelso = felso;
            eddigkur = kur;
        }
        int ujx = 0, ujy = 0;
        getmou(&ujx, &ujy);
        if (ujx != Moux && ujy != Mouy) {
            push();
            Moux = ujx;
            Mouy = ujy;
            pop();
        }
    }
}

static int tavolsagbeir(pic8* ppic, box abox, int tavolsag) {
    push();

    mk_emptychar();
    int valtozott = 1;
    int oldtavolsag = tavolsag;
    while (1) {
        if (valtozott) {
            valtozott = 0;
            char tmp[10];
            sprintf(tmp, "%d", tavolsag);
            boxbair(ppic, abox, Feherszin, tmp, 1);
            bltfront(ppic, abox.x1, abox.y1, abox.x2, abox.y2);
        }
        int c = mk_getextchar();
        if (c == MK_ESC) {
            pop();
            return oldtavolsag;
        }
        if (c == MK_ENTER) {
            pop();
            return tavolsag;
        }
        if (c >= '0' && c <= '9') {
            if (tavolsag < 100) {
                tavolsag *= 10;
                tavolsag += c - '0';
                valtozott = 1;
            }
        }
        if (c == MK_BACKSPACE) {
            if (tavolsag > 0) {
                tavolsag /= 10;
                valtozott = 1;
            }
        }
    }
}

static int hatarolbeir(pic8* ppic, box abox, int hatarol) {
    push();
    boxbair(ppic, abox, Feherszin, "", 1);
    bltfront(ppic, abox.x1, abox.y1, abox.x2, abox.y2);
    mk_emptychar();
    while (1) {
        int c = mk_getextchar();
        if (c == MK_ESC) {
            pop();
            return hatarol;
        }
        if (c == 'u' || c == 'U') {
            pop();
            return 0;
        }
        if (c == 'g' || c == 'G') {
            pop();
            return HATAROL_G;
        }
        if (c == 's' || c == 'S') {
            pop();
            return HATAROL_S;
        }
    }
}

void setspritetav(sprite* psp) {
    invalidateegesz();

    // Koordok megadasa:
    int x1 = 200;
    int y1 = 100;
    int x2 = 442;
    int y2 = 300;

    box boxok = {x1 + 32, y2 - 30, x1 + 32 + 70, y2 - 10};
    box boxcancel = {x1 + 135, y2 - 30, x1 + 135 + 70, y2 - 10};
    box boxtav = {x1 + 102, y1 + 118, x1 + 102 + 34, y1 + 118 + 16};
    box boxclipp = {x1 + 181, y1 + 118, x1 + 181 + 34, y1 + 118 + 16};

    int kepvaltozott = 1;
    int egerballevoltnyomva = 0;
    int tavolsag = psp->tavolsag;
    int hatarol = psp->hatarol;
    while (1) {
        // Nezzuk gombokat:
        while (mk_kbhit()) {
            int c = mk_getextchar();
            if (c == MK_ESC) {
                return;
            }
            if (c == MK_ENTER) {
                if (psp->tavolsag != tavolsag || psp->hatarol != hatarol) {
                    Valtozott = 1;
                }
                psp->tavolsag = tavolsag;
                psp->hatarol = hatarol;
                return;
            }
        }
        // Eger lenyomas:
        int balnyomva = 0;
        if (getbutbmou() && !egerballevoltnyomva) {
            egerballevoltnyomva = 1;
            balnyomva = 1;
        } else {
            egerballevoltnyomva = 0;
        }
        if (balnyomva) {
            int ujx = 0, ujy = 0;
            getmou(&ujx, &ujy);
            push();
            Moux = ujx;
            Mouy = ujy;
            pop();
            // Most nezzuk meg, hogy hova kattintott:
            if (boxban(ujx, ujy, boxok)) {
                if (psp->tavolsag != tavolsag || psp->hatarol != hatarol) {
                    Valtozott = 1;
                }
                psp->tavolsag = tavolsag;
                psp->hatarol = hatarol;
                return;
            }
            if (boxban(ujx, ujy, boxcancel)) {
                return;
            }
            if (boxban(ujx, ujy, boxtav)) {
                tavolsag = tavolsagbeir(Buffsima, boxtav, tavolsag);
                kepvaltozott = 1;
            }
            if (boxban(ujx, ujy, boxclipp)) {
                hatarol = hatarolbeir(Buffsima, boxclipp, hatarol);
                kepvaltozott = 1;
            }
        }
        if (kepvaltozott) {
            kepvaltozott = 0;
            // Kirajzoljuk, mivel valtozott kep:
            push();
            kirajzbox(Buffsima, x1, y1, x2, y2, Dialogszin, Dialogszelszin);
            // Most jonnek gombok:
            kirajzbox(Buffsima, boxok, Buttonszin, Dialogszelszin);
            kirajzbox(Buffsima, boxcancel, Buttonszin, Dialogszelszin);
            Pabc2->writekozep(Buffsima, (boxok.x1 + boxok.x2) / 2, boxok.y1 + 15, "OK");
            Pabc2->writekozep(Buffsima, (boxcancel.x1 + boxcancel.x2) / 2, boxcancel.y1 + 15,
                              "CANCEL");

            Pabc2->writekozep(Buffsima, (x1 + x2) / 2, y1 + 15, "Set Picture Properties");
            if (psp->kepnev[0]) {
                Pabc2->writekozep(Buffsima, (x1 + x2) / 2, y1 + 45, psp->kepnev);
            } else {
                Pabc2->writekozep(Buffsima, (x1 + x2) / 2, y1 + 34, psp->texturanev);
                Pabc2->writekozep(Buffsima, (x1 + x2) / 2, y1 + 56, psp->maszknev);
            }
            Pabc2->write(Buffsima, x1 + 92, y1 + 76, "Distance");
            Pabc2->write(Buffsima, x1 + 172, y1 + 76, "Clipping");
            // Default-ok kiirasa:
            Pabc2->write(Buffsima, x1 + 12, y1 + 100, "Default:");
            int deftav = -1;
            int defhatarol = -1;
            if (psp->kepnev[0]) {
                int index = Plgr->getkepindex(psp->kepnev);
                if (index < 0) {
                    hiba("iohdiuyde");
                }
                deftav = Plgr->kepek[index].tavolsag;
                defhatarol = Plgr->kepek[index].hatarol;
            }
            if (psp->texturanev[0]) {
                int index = Plgr->gettexturaindex(psp->texturanev);
                if (index < 0) {
                    hiba("u6ythggf");
                }
                deftav = Plgr->texturak[index].tavolsag;
                defhatarol = Plgr->texturak[index].hatarol;
            }
            // Tavolsag:
            char tmp[10];
            if (deftav >= 0) {
                sprintf(tmp, "%d", deftav);
            } else {
                sprintf(tmp, "-");
            }
            Pabc2->writekozep(Buffsima, x1 + 119, y1 + 100, tmp);

            // Hatarolas:
            strcpy(tmp, "U");
            if (defhatarol < 0) {
                strcpy(tmp, "-");
            }
            if (defhatarol == HATAROL_G) {
                strcpy(tmp, "G");
            }
            if (defhatarol == HATAROL_S) {
                strcpy(tmp, "S");
            }
            Pabc2->writekozep(Buffsima, x1 + 198, y1 + 100, tmp);

            Pabc2->write(Buffsima, x1 + 12, y1 + 130, "Current:");
            // Tavolsag boxba:
            kirajzbox(Buffsima, boxtav, Feherszin, Dialogszelszin);
            sprintf(tmp, "%d", tavolsag);
            boxbair(Buffsima, boxtav, Feherszin, tmp);
            Pabc2->writekozep(Buffsima, x1 + 119, boxtav.y2 + 14, "(1-999)");
            // Clipping boxba:
            kirajzbox(Buffsima, boxclipp, Feherszin, Dialogszelszin);
            strcpy(tmp, "U");
            if (hatarol == HATAROL_G) {
                strcpy(tmp, "G");
            }
            if (hatarol == HATAROL_S) {
                strcpy(tmp, "S");
            }
            boxbair(Buffsima, boxclipp, Feherszin, tmp);
            Pabc2->writekozep(Buffsima, x1 + 198, boxclipp.y2 + 14, "(U, S, G)");

            bltfront(Buffsima, x1, y1, x2, y2);
            pop();
        }
        int ujx = 0, ujy = 0;
        getmou(&ujx, &ujy);
        if (ujx != Moux && ujy != Mouy) {
            push();
            Moux = ujx;
            Mouy = ujy;
            pop();
        }
    }
}

void set_gyuru_attributes(gyuru* pgy) {
    invalidateegesz();

    // Koordok megadasa:
    int x1 = 200;
    int y1 = 100;
    int x2 = 384;
    int y2 = 200;

    box boxok = {x1 + 12, y2 - 30, x1 + 12 + 70, y2 - 10};
    box boxcancel = {x1 + 103, y2 - 30, x1 + 103 + 70, y2 - 10};

    int boxx1 = x1 + 132;
    box boxkoveto = {boxx1, y1 + 35, boxx1 + 30, y1 + 55};

    int kepvaltozott = 1;
    int egerballevoltnyomva = 0;

    int koveto = pgy->koveto;

    while (1) {
        // Nezzuk gombokat:
        while (mk_kbhit()) {
            int c = mk_getextchar();
            if (c == MK_ESC) {
                return;
            }
            if (c == MK_ENTER) {
                // Meg van ismetelve OK-nal is lejjebb:
                if (pgy->koveto != koveto) {
                    Valtozott = 1;
                }
                pgy->koveto = koveto;
                return;
            }
        }
        // Eger lenyomas:
        int egermost = getbutbmou();
        int balnyomva = 0;
        if (egermost && !egerballevoltnyomva) {
            balnyomva = 1;
        }
        egerballevoltnyomva = egermost;

        if (balnyomva) {
            int ujx = 0, ujy = 0;
            getmou(&ujx, &ujy);
            push();
            Moux = ujx;
            Mouy = ujy;
            pop();
            // Most nezzuk meg, hogy hova kattintott:
            if (boxban(ujx, ujy, boxok)) {
                // Meg van ismetelve ENTER-nel is feljebb:
                if (pgy->koveto != koveto) {
                    Valtozott = 1;
                }
                pgy->koveto = koveto;
                return;
            }
            if (boxban(ujx, ujy, boxcancel)) {
                return;
            }
            if (boxban(ujx, ujy, boxkoveto)) {
                koveto = !koveto;
                kepvaltozott = 1;
            }
        }
        if (kepvaltozott) {
            kepvaltozott = 0;
            // Kirajzoljuk, mivel valtozott kep:
            push();
            kirajzbox(Buffsima, x1, y1, x2, y2, Dialogszin, Dialogszelszin);

            Pabc2->writekozep(Buffsima, (x1 + x2) / 2, y1 + 15, "Set Polygon Properties");

            // Most jonnek gombok:
            // Koveto vagy nem gomb:
            int szovegx1 = x1 + 18;
            Pabc2->write(Buffsima, szovegx1, boxkoveto.y1 + 15, "Grass polygon:");
            kirajzbox(Buffsima, boxkoveto, Feherszin, Dialogszelszin);
            if (koveto) {
                Pabc2->writekozep(Buffsima, (boxkoveto.x1 + boxkoveto.x2) / 2, boxkoveto.y1 + 15,
                                  "YES");
            } else {
                Pabc2->writekozep(Buffsima, (boxkoveto.x1 + boxkoveto.x2) / 2, boxkoveto.y1 + 15,
                                  "NO");
            }

            kirajzbox(Buffsima, boxok, Buttonszin, Dialogszelszin);
            kirajzbox(Buffsima, boxcancel, Buttonszin, Dialogszelszin);
            Pabc2->writekozep(Buffsima, (boxok.x1 + boxok.x2) / 2, boxok.y1 + 15, "OK");
            Pabc2->writekozep(Buffsima, (boxcancel.x1 + boxcancel.x2) / 2, boxcancel.y1 + 15,
                              "CANCEL");

            bltfront(Buffsima, x1, y1, x2, y2);
            pop();
        }
        int ujx = 0, ujy = 0;
        getmou(&ujx, &ujy);
        if (ujx != Moux && ujy != Mouy) {
            push();
            Moux = ujx;
            Mouy = ujy;
            pop();
        }
    }
}

void set_kerek_tipus(char* fejlec, int* pkajatipus, int* pfoodsorszam) {
    invalidateegesz();

    // Koordok megadasa:
    int listaszam = 5;
    int dy = 20;

    int x1 = 200;
    int y1 = 100;
    int x2 = 442;
    int y2 = y1 + listaszam * dy + 120;

    box boxcancel = {(x1 + x2) / 2 - 50, y2 - 30, (x1 + x2) / 2 + 50, y2 - 10};
    box boxanimszam = {x2 - 50, y2 - 56, x2 - 50 + 20, y2 - 56 + 20};
    int ly1 = y1 + 38;
    int ly2 = ly1 + dy * listaszam - 1;
    // Csak elso rubrika:
    box boxlista = {(x1 + x2) / 2 - 70, ly1, (x1 + x2) / 2 + 70, ly1 + dy};

    // Kirajzoljuk, dialogust:
    push();
    kirajzbox(Buffsima, x1, y1, x2, y2, Dialogszin, Dialogszelszin);
    Pabc2->writekozep(Buffsima, (x1 + x2) / 2, y1 + 20, fejlec);
    kirajzbox(Buffsima, boxcancel, Buttonszin, Dialogszelszin);
    Pabc2->writekozep(Buffsima, (boxcancel.x1 + boxcancel.x2) / 2, boxcancel.y1 + 15, "Cancel");
    // Anim szam boxa:
    Pabc2->write(Buffsima, boxanimszam.x1 - 162, boxanimszam.y1 + 15, "Food anim number (1-9):");
    char tmp[20];
    sprintf(tmp, "%d", (int)(*pfoodsorszam + 1));
    kirajzbox(Buffsima, boxanimszam, Feherszin, Dialogszelszin);
    Pabc2->writekozep(Buffsima, (boxanimszam.x1 + boxanimszam.x2) / 2, boxanimszam.y1 + 15, tmp);

    for (int i = 0; i < listaszam; i++) {
        kirajzbox(Buffsima, boxlista, Openlistaszin, Dialogszelszin);
        int lx = (boxlista.x1 + boxlista.x2) / 2;
        int ly = boxlista.y1 + 15;
        switch (i) {
        case 0:
            Pabc2->writekozep(Buffsima, lx, ly, "Normal Food");
            break;
        case 1:
            Pabc2->writekozep(Buffsima, lx, ly, "Gravity Up");
            break;
        case 2:
            Pabc2->writekozep(Buffsima, lx, ly, "Gravity Down");
            break;
        case 3:
            Pabc2->writekozep(Buffsima, lx, ly, "Gravity Left");
            break;
        case 4:
            Pabc2->writekozep(Buffsima, lx, ly, "Gravity Right");
            break;
        default:
            hiba("yuiwegfyjkweg");
        }
        boxlista.y1 += dy;
        boxlista.y2 += dy;
    }
    boxlista.y1 = ly1;
    boxlista.y2 = ly2;

    bltfront(Buffsima, x1, y1, x2, y2);
    pop();

    // Most jon ciklus:
    int egerballevoltnyomva = 0;
    while (1) {
        // Nezzuk gombokat:
        while (mk_kbhit()) {
            int c = mk_getextchar();
            if (c == MK_ESC) {
                return;
            }
        }
        // Eger lenyomas:
        int balnyomva = 0;
        if (getbutbmou() && !egerballevoltnyomva) {
            egerballevoltnyomva = 1;
            balnyomva = 1;
        } else {
            egerballevoltnyomva = 0;
        }
        if (balnyomva) {
            int ujx = 0, ujy = 0;
            getmou(&ujx, &ujy);
            push();
            Moux = ujx;
            Mouy = ujy;
            pop();
            // Most nezzuk meg, hogy hova kattintott:
            if (boxban(ujx, ujy, boxcancel)) {
                return;
            }
            if (boxban(ujx, ujy, boxanimszam)) {
                // Berajzolunk egy vizszintes kurzort:
                push(); // Leveszi egeret
                kirajzbox(Buffsima, boxanimszam, Feherszin, Dialogszelszin);
                Pabc2->writekozep(Buffsima, (boxanimszam.x1 + boxanimszam.x2) / 2,
                                  boxanimszam.y1 + 18, "-");
                bltfront(Buffsima, x1, y1, x2, y2);

                mk_emptychar();
                while (1) {
                    // Kirajzolta dobozba kurzort, most 1..9, vagy ESC-et var:
                    int c = mk_getextchar();
                    if (c == MK_ESC) {
                        break;
                    }
                    if (c >= '1' && c <= '9') {
                        *pfoodsorszam = c - '1';
                        break;
                    }
                }
                pop();
                return;
            }
            if (boxban(ujx, ujy, boxlista)) {
                // Benne van listaban:
                int index = (ujy - boxlista.y1) / dy;
                if (index >= listaszam) {
                    index = listaszam - 1;
                }
                Valtozott = 1;
                switch (index) {
                case 0:
                    *pkajatipus = KT_NORMAL;
                    break;
                case 1:
                    *pkajatipus = KT_UP;
                    break;
                case 2:
                    *pkajatipus = KT_DOWN;
                    break;
                case 3:
                    *pkajatipus = KT_LEFT;
                    break;
                case 4:
                    *pkajatipus = KT_RIGHT;
                    break;
                default:
                    hiba("0893ytpoerhg");
                }
                return;
            }
        }
        int ujx = 0, ujy = 0;
        getmou(&ujx, &ujy);
        if (ujx != Moux && ujy != Mouy) {
            push();
            Moux = ujx;
            Mouy = ujy;
            pop();
        }
    }
}

static void change_level_name(char* nev) {
    char orignev[LEVELNAMEHOSSZ + 10];
    strcpy(orignev, nev);

    push(); // Itt levesszuk egeret es nem is tesszuk vissza vegeig:

    blt8(Buffgolyo, Buffsima);

    invalidateegesz();
    // Itt most be kell olvasnunk egy nevet:
    int x1 = 50;
    int y1 = 200;
    int x2 = 600;
    int y2 = 280;

    mk_emptychar();
    int i = 0;
    while (1) {
        // Kiir:
        Buffsima->fillbox(x1, y1, x2, y2, Save_as_box_szin);
        Buffsima->line(x1, y1, x2, y1, Save_as_szelszin);
        Buffsima->line(x1, y2, x2, y2, Save_as_szelszin);
        Buffsima->line(x1, y1, x1, y2, Save_as_szelszin);
        Buffsima->line(x2, y1, x2, y2, Save_as_szelszin);
        Pabc2->writekozep(
            Buffsima, (x2 + x1) / 2, 220,
            "Type in the name of the level and the designer and press ENTER (ESC to cancel)!");
        int kezdes = 300 - Pabc1->len(nev) / 2;
        Pabc2->write(Buffsima, kezdes, 250, nev);
        char tmp[LEVELNAMEHOSSZ + 10];
        strcpy(tmp, nev);
        tmp[i] = 0;
        Pabc2->write(Buffsima, kezdes + Pabc1->len(tmp), 255, "-");
        bltfront(Buffsima, x1, y1, x2, y2);

        int c = mk_getextchar();
        if (c == MK_ESC) {
            strcpy(nev, orignev);
            pop();
            return;
        }
        if (c == MK_ENTER) {
            if (nev[0] == 0) {
                continue;
            }
            pop();
            return;
        }
        if ((c >= 'A' && c <= 'Z') || (c >= 'a' && c <= 'z') || (c >= '0' && c <= '9') ||
            c == ' ') {

            char tmpfejlec[LEVELNAMEHOSSZ + 10];
            sprintf(tmpfejlec, "Ext: %s", nev);

            if (strlen(nev) >= LEVELNAMEHOSSZ || Pmenuabc->len(tmpfejlec) > 625) {
                continue;
            }

            // c-t beillesztjuk i. helyre:
            int hossz = strlen(nev);
            for (int j = hossz; j >= i; j--) {
                nev[j + 1] = nev[j];
            }
            nev[i] = (char)c;
            i++;
        }
        if (c == MK_BACKSPACE) {
            // Torles:
            if (nev[0] == 0 || i <= 0) {
                continue;
            }
            // Kiszedjuk i-1-edik helyrol karaktert:
            int hossz = strlen(nev);
            for (int j = i - 1; j < hossz; j++) {
                nev[j] = nev[j + 1];
            }
            i--;
        }
        if (c == MK_DEL) {
            // Torles:
            if (nev[0] == 0 || i >= (int)strlen(nev)) {
                continue;
            }
            // Kiszedjuk i-1-edik helyrol karaktert:
            int hossz = strlen(nev);
            for (int j = i; j < hossz; j++) {
                nev[j] = nev[j + 1];
            }
        }
        if (c == MK_LEFT && i > 0) {
            i--;
        }
        if (c == MK_RIGHT && i < (int)strlen(nev)) {
            i++;
        }
    }
}

void change_level_properties(void) {
    invalidateegesz();

    char fgnev[10];
    char bgnev[10];
    char levelname[LEVELNAMEHOSSZ + 20];
    strcpy(fgnev, Ptop->fgnev);
    strcpy(bgnev, Ptop->bgnev);
    strcpy(levelname, Ptop->levelname);

    char lgrnev[20] = "";
    strcpy(lgrnev, Ptop->lgrnev);

    int x1 = 130, y1 = 100;
    int x2 = 570, y2 = 280;

    int bxtav = 10;
    int bxs = 70;
    int bxkoz = (x1 + x2) / 2;
    box boxok = {bxkoz - bxtav - bxs, y2 - 30, bxkoz - bxtav, y2 - 10};
    box boxcancel = {bxkoz + bxtav, y2 - 30, bxkoz + bxtav + bxs, y2 - 10};

    int boxx1 = x1 + 100;
    box boxfgnev = {boxx1, y1 + 38, boxx1 + 80, y1 + 58};
    box boxbgnev = {boxx1, y1 + 60, boxx1 + 80, y1 + 80};
    // box boxlevelname = { boxx1, y1+85, boxx1+180, y1+105 };
    box boxlevelname = {boxx1, y1 + 85, boxx1 + 320, y1 + 105};
    box boxlgrfile = {boxx1, y1 + 110, boxx1 + 80, y1 + 130};

    int egerballevoltnyomva = 1;
    int invalid = 1;

    // Ez a resz sebesseg tesztelesere szolgal:
    /*int ciklus = 0;
    while( 1 ) {
        // Nezzuk gombokat:
        ciklus++;
        if( ciklus > 40 ) {
            ciklus = 0;
            while( mk_kbhit() ) {
                int c = mk_getextchar();
                if( c == MK_ESC )
                    return;
            }
        }
        //getbutbmou();

        int ujx = 0, ujy = 0;
        //for( int i = 0; i < 10; i++ )
        getmou( &ujx, &ujy );
        if( ujx != Moux && ujy != Mouy ) {
            push();
            Moux = ujx;
            Mouy = ujy;
            pop();
        }
    } */

    while (1) {
        // Nezzuk gombokat:
        while (mk_kbhit()) {
            int c = mk_getextchar();
            if (c == MK_ESC) {
                return;
            }
            if (c == MK_ENTER) {
                if (strcmpi(Ptop->fgnev, fgnev) != 0 || strcmpi(Ptop->bgnev, bgnev) != 0 ||
                    strcmpi(Ptop->levelname, levelname) != 0) {
                    Valtozott = 1;
                }
                strcpy(Ptop->fgnev, fgnev);
                strcpy(Ptop->bgnev, bgnev);
                strcpy(Ptop->levelname, levelname);

                if (strcmpi(lgrnev, Ptop->lgrnev) != 0) {
                    Valtozott = 1;
                    strcpy(Ptop->lgrnev, lgrnev);
                    loadlgrfile(Ptop->lgrnev);
                    if (Ptop->pic_selejtez(Plgr)) {
                        dialogselejtez();
                    }
                }
                return;
            }
        }
        // Eger lenyomas:
        int balnyomva = 0;
        if (getbutbmou()) {
            if (!egerballevoltnyomva) {
                balnyomva = 1;
            }
            egerballevoltnyomva = 1;
        } else {
            egerballevoltnyomva = 0;
        }
        if (balnyomva) {
            int ujx = 0, ujy = 0;
            getmou(&ujx, &ujy);
            push();
            Moux = ujx;
            Mouy = ujy;
            pop();
            // Most nezzuk meg, hogy hova kattintott:
            if (boxban(ujx, ujy, boxok)) {
                if (strcmpi(Ptop->fgnev, fgnev) != 0 || strcmpi(Ptop->bgnev, bgnev) != 0 ||
                    strcmpi(Ptop->levelname, levelname) != 0) {
                    Valtozott = 1;
                }
                strcpy(Ptop->fgnev, fgnev);
                strcpy(Ptop->bgnev, bgnev);
                strcpy(Ptop->levelname, levelname);

                if (strcmpi(lgrnev, Ptop->lgrnev) != 0) {
                    Valtozott = 1;
                    strcpy(Ptop->lgrnev, lgrnev);
                    loadlgrfile(Ptop->lgrnev);
                    if (Ptop->pic_selejtez(Plgr)) {
                        dialogselejtez();
                    }
                }
                return;
            }
            if (boxban(ujx, ujy, boxcancel)) {
                return;
            }
            if (boxban(ujx, ujy, boxfgnev)) {
                // Foreground nevet akarja valtoztatni:
                char uresnev[10] = "";
                pickanev(uresnev, fgnev, uresnev, 2, 0);
                push();
                blt8(Buffsima, Buffgolyo);
                bltfront(Buffsima);
                pop();
                invalid = 1;
            }
            if (boxban(ujx, ujy, boxbgnev)) {
                // Background nevet akarja valtoztatni:
                char uresnev[10] = "";
                pickanev(uresnev, bgnev, uresnev, 2, 0);
                push();
                blt8(Buffsima, Buffgolyo);
                bltfront(Buffsima);
                pop();
                invalid = 1;
            }
            if (boxban(ujx, ujy, boxlevelname)) {
                // Palya nevet akarja valtoztatni:
                change_level_name(levelname);
                push();
                blt8(Buffsima, Buffgolyo);
                bltfront(Buffsima);
                pop();
                invalid = 1;
            }
            if (boxban(ujx, ujy, boxlgrfile)) {
                char belgrnev[20] = "";
                strcpy(belgrnev, lgrnev);
                if (!tolt_picklgrfile(lgrnev)) {
                    strcpy(lgrnev, belgrnev);
                }
                push();
                blt8(Buffsima, Buffgolyo);
                bltfront(Buffsima);
                pop();
                invalid = 1;
            }
        }
        if (invalid) {
            /*static int nyomasszam = 0;
            nyomasszam++;
            if( nyomasszam > 10 )
                uzenet( "nyomasszam >" );*/

            invalid = 0;

            push();
            // Kirajzoljuk, mivel valtozott kep:
            kirajzbox(Buffsima, x1, y1, x2, y2, Dialogszin, Dialogszelszin);

            Pabc2->writekozep(Buffsima, (x1 + x2) / 2, y1 + 22, "Level properties");

            // Most jonnek gombok:
            // OK:
            kirajzbox(Buffsima, boxok, Buttonszin, Dialogszelszin);
            Pabc2->writekozep(Buffsima, (boxok.x1 + boxok.x2) / 2, boxok.y1 + 15, "OK");
            // CANCEL:
            kirajzbox(Buffsima, boxcancel, Buttonszin, Dialogszelszin);
            Pabc2->writekozep(Buffsima, (boxcancel.x1 + boxcancel.x2) / 2, boxcancel.y1 + 15,
                              "CANCEL");

            // Foreground sor:
            int szovegx = x1 + 10;
            Pabc2->write(Buffsima, szovegx, boxfgnev.y1 + 15, "Foreground:");
            kirajzbox(Buffsima, boxfgnev, Feherszin, Dialogszelszin);
            boxbair_balra(Buffsima, boxfgnev, Feherszin, fgnev);
            // Background sor:
            Pabc2->write(Buffsima, szovegx, boxbgnev.y1 + 15, "Background:");
            kirajzbox(Buffsima, boxbgnev, Feherszin, Dialogszelszin);
            boxbair_balra(Buffsima, boxbgnev, Feherszin, bgnev);
            // Level name sor:
            Pabc2->write(Buffsima, szovegx, boxlevelname.y1 + 15, "Level name:");
            kirajzbox(Buffsima, boxlevelname, Feherszin, Dialogszelszin);
            boxbair_balra(Buffsima, boxlevelname, Feherszin, levelname);
            // LGR file nev sor:
            Pabc2->write(Buffsima, szovegx, boxlgrfile.y1 + 15, "LGR file:");
            kirajzbox(Buffsima, boxlgrfile, Feherszin, Dialogszelszin);
            boxbair_balra(Buffsima, boxlgrfile, Feherszin, lgrnev);

            bltfront(Buffsima, x1, y1, x2, y2);
            pop();
        }
        int ujx = 0, ujy = 0;
        getmou(&ujx, &ujy);
        if (ujx != Moux && ujy != Mouy) {
            push();
            Moux = ujx;
            Mouy = ujy;
            pop();
        }
    }
}

int Rajzolpoligon = 1;
int Rajzolkoveto = 1;
int Rajzolkepek = 1;

void setviewoptions(void) {
    invalidateegesz();

    int x1 = 200, y1 = 100;
    int x2 = 360, y2 = 230;

    int szovx = x1 + 14;

    box boxok = {(x1 + x2) / 2 - 20, y2 - 30, (x1 + x2) / 2 + 20, y2 - 10};

    box boxpoligon = {x1 + 110, y1 + 18, x1 + 140, y1 + 38};
    box boxkoveto = {x1 + 110, y1 + 40, x1 + 140, y1 + 60};
    box boxkepek = {x1 + 110, y1 + 65, x1 + 140, y1 + 85};

    int egerballevoltnyomva = 0;
    int invalid = 1;
    while (1) {
        // Nezzuk gombokat:
        while (mk_kbhit()) {
            int c = mk_getextchar();
            if (c == MK_ESC || c == MK_ENTER) {
                return;
            }
        }
        // Eger lenyomas:
        int egermost = getbutbmou();
        int balnyomva = 0;
        if (egermost && !egerballevoltnyomva) {
            balnyomva = 1;
        }
        egerballevoltnyomva = egermost;

        if (balnyomva) {
            int ujx = 0, ujy = 0;
            getmou(&ujx, &ujy);
            push();
            Moux = ujx;
            Mouy = ujy;
            pop();
            // Most nezzuk meg, hogy hova kattintott:
            if (boxban(ujx, ujy, boxok)) {
                return;
            }
            if (boxban(ujx, ujy, boxpoligon)) {
                Rajzolpoligon = !Rajzolpoligon;
                invalid = 1;
            }
            if (boxban(ujx, ujy, boxkoveto)) {
                Rajzolkoveto = !Rajzolkoveto;
                invalid = 1;
            }
            if (boxban(ujx, ujy, boxkepek)) {
                Rajzolkepek = !Rajzolkepek;
                invalid = 1;
            }
        }
        if (invalid) {
            invalid = 0;
            push();
            // Kirajzoljuk, mivel valtozott kep:
            kirajzbox(Buffsima, x1, y1, x2, y2, Dialogszin, Dialogszelszin);
            // Most jonnek gombok:
            // OK:
            kirajzbox(Buffsima, boxok, Buttonszin, Dialogszelszin);
            Pabc2->writekozep(Buffsima, (boxok.x1 + boxok.x2) / 2, boxok.y1 + 15, "OK");

            Pabc2->write(Buffsima, szovx, boxpoligon.y1 + 15, "View Polygons");
            kirajzbox(Buffsima, boxpoligon, Feherszin, Dialogszelszin);
            if (Rajzolpoligon) {
                boxbair(Buffsima, boxpoligon, Feherszin, "Yes");
            } else {
                boxbair(Buffsima, boxpoligon, Feherszin, "No");
            }

            Pabc2->write(Buffsima, szovx, boxkoveto.y1 + 15, "View Grass");
            kirajzbox(Buffsima, boxkoveto, Feherszin, Dialogszelszin);
            if (Rajzolkoveto) {
                boxbair(Buffsima, boxkoveto, Feherszin, "Yes");
            } else {
                boxbair(Buffsima, boxkoveto, Feherszin, "No");
            }

            Pabc2->write(Buffsima, szovx, boxkepek.y1 + 15, "View Pictures");
            kirajzbox(Buffsima, boxkepek, Feherszin, Dialogszelszin);
            if (Rajzolkepek) {
                boxbair(Buffsima, boxkepek, Feherszin, "Yes");
            } else {
                boxbair(Buffsima, boxkepek, Feherszin, "No");
            }

            bltfront(Buffsima, x1, y1, x2, y2);
            pop();
        }
        int ujx = 0, ujy = 0;
        getmou(&ujx, &ujy);
        if (ujx != Moux && ujy != Mouy) {
            push();
            Moux = ujx;
            Mouy = ujy;
            pop();
        }
    }
}
