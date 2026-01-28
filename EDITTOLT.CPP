#include "EDITTOLT.H"
#include "abc8.h"
#include "editor_canvas.h"
#include "editor_dialog.h"
#include "EDITUJ.H"
#include "fs_utils.h"
#include "keys.h"
#include "level.h"
#include "LOAD.H"
#include "lgr.h"
#include "M_PIC.H"
#include "main.h"
#include "menu_pic.h"
#include "pic8.h"
#include "platform_impl.h"
#include "platform_utils.h"
#include "polygon.h"
#include "sprite.h"
#include <cstring>

constexpr unsigned char Save_as_box_szin = 4, Save_as_szelszin = 5;

static unsigned char Openlistaszin = 6, Openlistainvszin = 7;

static int Maxnev = 1000; // Nem lehet nagyobb 3000-nel!
static char* Soknev = NULL;

static void abcberendez(int nevszam) {
    for (int i = 0; i < nevszam + 3; i++) {
        for (int j = 0; j < nevszam - 1; j++) {
            // Ha j es j+1 edik elem forditva van visszacsereljuk oket:
            char* nev1 = &Soknev[j * 10];
            char* nev2 = &Soknev[(j + 1) * 10];
            int abcbenelobb(const char* text1, const char* text2);
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

static void betolt(const char* nev) {
    if (!Ptop) {
        internal_error("uoiwfiufre");
    }
    if (!*nev) {
        internal_error("oiufewguyxygxw");
    }
    char tmp[15];
    if (strlen(nev) > 8) {
        internal_error("iovffiufi");
    }
    strcpy(tmp, nev);
    strcat(tmp, ".lev");
    if (strlen(tmp) > 12) {
        internal_error("betolt-ben strlen( tmp ) > 12!");
    }
    strcpy(State->editor_filename, tmp);

    Valtozott = 0;
    if (!floadlevel_e(tmp, 1)) {
        State->editor_filename[0] = 0;
    }

    zoom_fill();
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

static void boxbair(pic8* ppic, box abox, unsigned char szin, const char* text, int alahuzas = 0) {
    ppic->fill_box(abox.x1 + 1, abox.y1 + 1, abox.x2 - 1, abox.y2 - 1, szin);
    if (alahuzas) {
        int hossz = Pabc2->len(text);
        int x = (abox.x1 + abox.x2) / 2 - hossz / 2 - 3;
        Pabc2->write(ppic, x, (abox.y1 + abox.y2) / 2 + 5, text);
        for (int i = 0; i < 6; i++) {
            ppic->ppixel(x + hossz + i + 1, (abox.y1 + abox.y2) / 2 + 4, 0);
        }
    } else {
        Pabc2->write_centered(ppic, (abox.x1 + abox.x2) / 2, (abox.y1 + abox.y2) / 2 + 5, text);
    }
}

static void boxbair_balra(pic8* ppic, box abox, unsigned char szin, const char* text) {
    ppic->fill_box(abox.x1 + 1, abox.y1 + 1, abox.x2 - 1, abox.y2 - 1, szin);
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
                   "Do you still want to continue?", DIALOG_BUTTONS, "Yes", "No") == 1) {
            return;
        }
    }
    if (!Soknev) {
        Soknev = new char[Maxnev * 10 + 100];
        if (!Soknev) {
            internal_error("Nincs eleg memoria tolt_open-ben (hjfdf)!");
        }
    }
    for (int i = 0; i < Maxnev * 10 + 10; i++) {
        Soknev[i] = 0;
    }
    finame fname;
    bool done = find_first("lev/*.lev", fname);
    int nevszam = 0;
    while (!done) {
        if (strlen(fname) > 14) {
            internal_error("ijhfiyewpp");
        }
        char* nev = &Soknev[nevszam * 10];
        strcpy(nev, fname);
        // Leveszi kiterjesztest:
        int i = 0;
        while (nev[i] != '.') {
            if (!nev[i]) {
                internal_error("Nincs pont nevben! (yewruyrewuy): ", nev);
            }
            i++;
        }
        nev[i] = 0;
        if (strlen(nev) > 8) {
            internal_error("iuoriureiur");
        }

        done = find_next(fname);
        nevszam++;
        if (nevszam >= Maxnev) {
            dialog("Number of files > 1000 in open dialog box!", "Could not load all the files!");
            done = 1;
        }
    }

    find_close();

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
        if (strcmpi(tmp, State->editor_filename) == 0) {
            kur = i;
        }
    }

    // State->editor_filename[0] = 0; Igy nem lesz unnamed CANCEL utan

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
    empty_keypress_buffer();
    // Azert 1, hogy ha lenyomott gombbal erkezett,
    // ne tekintse lenyomottnak:
    left_mouse_clicked();
    while (1) {
        // Nezzuk gombokat:
        while (has_keypress()) {
            Keycode c = get_keypress();
            if (c == KEY_ESC) {
                return;
            }
            if (c == KEY_ENTER) {
                igazit(&kur, &felso, nevszam, helyszam);
                betolt(&Soknev[kur * 10]);
                return;
            }
            if (c == KEY_UP) {
                kur--;
            }
            if (c == KEY_PGUP) {
                kur -= helyszam - 1;
            }
            if (c == KEY_DOWN) {
                kur++;
            }
            if (c == KEY_PGDOWN) {
                kur += helyszam - 1;
            }
        }
        // Eger lenyomas:
        if (left_mouse_clicked()) {
            update_and_draw_cursor();
            // Most nezzuk meg, hogy hova kattintott:
            if (is_in_box(Moux, Mouy, boxle)) {
                kur += helyszam - 1;
            }
            if (is_in_box(Moux, Mouy, boxfel)) {
                kur -= helyszam - 1;
            }
            if (is_in_box(Moux, Mouy, boxcancel)) {
                return;
            }
            if (Moux > lx1 && Moux < lx2 && Mouy > ly1 && Mouy < ly2) {
                // Lista folott nyomta meg gombot:
                if (Mouy < ly1 + dy * helyszam) {
                    // Egy nevet el is talalt:
                    int index = (Mouy - ly1) / dy;
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
            render_box(BufferMain, x1, y1, x2, y2, DIALOG_PALETTE_ID, DIALOG_BORDER_PALETTE_ID);
            render_box(BufferMain, lx1, ly1, lx2, ly2, Openlistaszin, DIALOG_BORDER_PALETTE_ID);
            for (int i = 0; i < helyszam && i < nevszam; i++) {
                if (i + felso == kur) {
                    // Ez most kurens nev:
                    BufferMain->fill_box(lx1 + 1, ly1 + i * dy + 1, lx2 - 1, ly1 + (i + 1) * dy - 1,
                                         Openlistainvszin);
                }
                Pabc2->write(BufferMain, lx1 + 3, ly1 + 15 + i * dy, &Soknev[(i + felso) * 10]);
            }
            // Most jonnek gombok:
            render_box(BufferMain, boxfel, BUTTON_PALETTE_ID, DIALOG_BORDER_PALETTE_ID);
            boxbarajz(BufferMain, boxfel, DIALOG_BORDER_PALETTE_ID, 1);
            render_box(BufferMain, boxle, BUTTON_PALETTE_ID, DIALOG_BORDER_PALETTE_ID);
            boxbarajz(BufferMain, boxle, DIALOG_BORDER_PALETTE_ID, 0);
            render_box(BufferMain, boxcancel, BUTTON_PALETTE_ID, DIALOG_BORDER_PALETTE_ID);
            Pabc2->write_centered(BufferMain, (boxcancel.x1 + boxcancel.x2) / 2, boxcancel.y1 + 15,
                                  "CANCEL");

            bltfront(BufferMain, x1, y1, x2, y2);
            pop();

            eddigfelso = felso;
            eddigkur = kur;
        }
        update_and_draw_cursor();
    }
}

// Editfilenev alapjan megprobal josolni, majd oda teszi eredmenyt:
void kivalasztkezdetit(void) {
    invalidateegesz();
    if (!Soknev) {
        Soknev = new char[Maxnev * 10 + 100];
        if (!Soknev) {
            internal_error("Nincs eleg memoria tolt_open-ben (hjfdf)!");
        }
    }
    for (int i = 0; i < Maxnev * 10 + 10; i++) {
        Soknev[i] = 0;
    }
    finame fname;
    bool done = find_first("lev/*.lev", fname);
    int nevszam = 0;
    while (!done) {
        if (strlen(fname) > 14) {
            internal_error("ijhfiyewpp");
        }
        char* nev = &Soknev[nevszam * 10];
        strcpy(nev, fname);

        // Leveszi kiterjesztest:
        int i = 0;
        while (nev[i] != '.') {
            if (!nev[i]) {
                internal_error("Nincs pont nevben! (yewruyrewuy): ", nev);
            }
            i++;
        }
        nev[i] = 0;
        if (strlen(nev) > 8) {
            internal_error("iuoriureiur");
        }

        done = find_next(fname);
        nevszam++;
        if (nevszam >= Maxnev) {
            dialog("Number of files > 1000 in open dialog box!", "Could not load all the files!");
            done = 1;
        }
    }

    find_close();

    if (nevszam < 1) {
        State->editor_filename[0] = 0;
        return;
    }

    abcberendez(nevszam);

    int kur = 0; // Ha megtalalja eddigit, akkor kur-t raallitja
    for (int i = 0; i < nevszam; i++) {
        char tmp[50];
        strcpy(tmp, &Soknev[i * 10]);
        strcat(tmp, ".lev");
        if (strcmpi(tmp, State->editor_filename) == 0) {
            kur = i;
        }
    }

    State->editor_filename[0] = 0;

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
    empty_keypress_buffer();
    left_mouse_clicked();
    while (1) {
        // Nezzuk gombokat:
        while (has_keypress()) {
            Keycode c = get_keypress();
            if (c == KEY_ESC) {
                return;
            }
            if (c == KEY_ENTER) {
                igazit(&kur, &felso, nevszam, helyszam);
                char* nev = &Soknev[kur * 10];
                if (strlen(nev) > 8) {
                    internal_error("iovfgsdgffiufi");
                }
                strcpy(State->editor_filename, nev);
                strcat(State->editor_filename, ".lev");
                return;
            }
            if (c == KEY_UP) {
                kur--;
            }
            if (c == KEY_PGUP) {
                kur -= helyszam - 1;
            }
            if (c == KEY_DOWN) {
                kur++;
            }
            if (c == KEY_PGDOWN) {
                kur += helyszam - 1;
            }
        }
        // Eger lenyomas:
        if (left_mouse_clicked()) {
            update_and_draw_cursor();
            // Most nezzuk meg, hogy hova kattintott:
            if (is_in_box(Moux, Mouy, boxle)) {
                kur += helyszam - 1;
            }
            if (is_in_box(Moux, Mouy, boxfel)) {
                kur -= helyszam - 1;
            }
            if (is_in_box(Moux, Mouy, boxcancel)) {
                return;
            }
            if (is_in_box(Moux, Mouy, boxnew)) {
                State->editor_filename[0] = 0;
                return;
            }
            if (Moux > lx1 && Moux < lx2 && Mouy > ly1 && Mouy < ly2) {
                // Lista folott nyomta meg gombot:
                if (Mouy < ly1 + dy * helyszam) {
                    // Egy nevet el is talalt:
                    int index = (Mouy - ly1) / dy;
                    index += felso;
                    if (index < nevszam) {
                        char* nev = &Soknev[index * 10];
                        if (strlen(nev) > 8) {
                            internal_error("iovfgsdgffiufi");
                        }
                        strcpy(State->editor_filename, nev);
                        strcat(State->editor_filename, ".lev");
                        return;
                    }
                }
            }
        }
        igazit(&kur, &felso, nevszam, helyszam);
        if (felso != eddigfelso || kur != eddigkur) {
            // Kirajzoljuk, mivel valtozott kep:
            push();
            render_box(BufferMain, x1, y1, x2, y2, DIALOG_PALETTE_ID, DIALOG_BORDER_PALETTE_ID);
            render_box(BufferMain, lx1, ly1, lx2, ly2, Openlistaszin, DIALOG_BORDER_PALETTE_ID);
            for (int i = 0; i < helyszam && i < nevszam; i++) {
                if (i + felso == kur) {
                    // Ez most kurens nev:
                    BufferMain->fill_box(lx1 + 1, ly1 + i * dy + 1, lx2 - 1, ly1 + (i + 1) * dy - 1,
                                         Openlistainvszin);
                }
                Pabc2->write(BufferMain, lx1 + 3, ly1 + 15 + i * dy, &Soknev[(i + felso) * 10]);
            }
            // Most jonnek gombok:
            render_box(BufferMain, boxfel, BUTTON_PALETTE_ID, DIALOG_BORDER_PALETTE_ID);
            boxbarajz(BufferMain, boxfel, DIALOG_BORDER_PALETTE_ID, 1);
            render_box(BufferMain, boxle, BUTTON_PALETTE_ID, DIALOG_BORDER_PALETTE_ID);
            boxbarajz(BufferMain, boxle, DIALOG_BORDER_PALETTE_ID, 0);
            render_box(BufferMain, boxcancel, BUTTON_PALETTE_ID, DIALOG_BORDER_PALETTE_ID);
            Pabc2->write_centered(BufferMain, (boxcancel.x1 + boxcancel.x2) / 2, boxcancel.y1 + 15,
                                  "CANCEL");
            render_box(BufferMain, boxnew, BUTTON_PALETTE_ID, DIALOG_BORDER_PALETTE_ID);
            Pabc2->write_centered(BufferMain, (boxnew.x1 + boxnew.x2) / 2, boxnew.y1 + 15, "NEW");

            bltfront(BufferMain, x1, y1, x2, y2);
            pop();

            eddigfelso = felso;
            eddigkur = kur;
        }
        update_and_draw_cursor();
    }
}

void tolt_new(void) {
    invalidateegesz();
    if (Valtozott) {
        if (dialog("There are unsaved changes in the level file.", "Do you still want to continue?",
                   DIALOG_BUTTONS, "Yes", "No") == 1) {
            return;
        }
    }

    Valtozott = 0;
    floadlevel_e("_uj_topol_");
    State->editor_filename[0] = 0;
    zoom_fill();
}

// Ha sikeresen mentett, akkor igaz:
int tolt_save() {
    invalidateegesz();
    if (State->editor_filename[0] == 0) {
        // Ez a file meg nincsen elnevezve:
        return tolt_save_as();
    }
    Ptop->save(State->editor_filename);
    Valtozott = 0;
    return 1;
}

// Ha sikeresen mentett, akkor igaz:
int tolt_save_as() {
    invalidateegesz();
    // Itt most be kell olvasnunk egy nevet:
    box boxnagy = {20, 200, 600, 300};
    box boxsave = {320 - 30, 273, 320 + 30, 290};

    push();
    render_box(BufferMain, boxnagy, DIALOG_PALETTE_ID, DIALOG_BORDER_PALETTE_ID);
    render_box(BufferMain, boxsave, BUTTON_PALETTE_ID, DIALOG_BORDER_PALETTE_ID);
    Pabc2->write_centered(
        BufferMain, (boxnagy.x2 + boxnagy.x1) / 2, 220,
        "Type in the file name and press ENTER or click on SAVE, or press ESC to cancel!");
    Pabc2->write_centered(BufferMain, (boxsave.x2 + boxsave.x1) / 2, boxsave.y1 + 13, "SAVE");

    bltfront(BufferMain, boxnagy.x1, boxnagy.y1, boxnagy.x2, boxnagy.y2);
    pop();

    empty_keypress_buffer();
    char nev[20] = "";
    int i = 0;
    left_mouse_clicked();
    right_mouse_clicked();
    char eddignev[10] = "$%...%^&";
    while (1) {
        if (strcmp(eddignev, nev) != 0) {
            strcpy(eddignev, nev);

            int ex1 = boxnagy.x1 + 100;
            int ey1 = boxnagy.y1 + 30;
            int ex2 = boxnagy.x2 - 100;
            int ey2 = boxnagy.y1 + 55;
            BufferMain->fill_box(ex1, ey1, ex2, ey2, DIALOG_PALETTE_ID);
            Pabc2->write(BufferMain, 290, 250, nev);
            char tmp[40];
            strcpy(tmp, nev);
            tmp[i] = 0;
            Pabc2->write(BufferMain, 290 + Pabc1->len(tmp), 255, "-");
            bltfront(BufferMain, ex1, ey1, ex2, ey2);
        }
        while (has_keypress()) {
            Keycode c = get_keypress();
            if (c == KEY_ESC) {
                return 0;
            }
            if (c == KEY_ENTER) {
                if (nev[0] == 0) {
                    continue;
                }
                // Most mentunk:
                // Eloallitjuk nevet:
                strcat(nev, ".lev");
                char tmpnev[40];
                strcpy(tmpnev, "lev/");
                strcat(tmpnev, nev);
                if (access(tmpnev, 0) == 0) {
                    if (dialog("File exists, overwrite?", nev, DIALOG_BUTTONS, "Yes", "No") == 1) {
                        return 0;
                    }
                }
                Ptop->save(nev);
                strcpy(State->editor_filename, nev);
                Valtozott = 0;
                return 1;
            }
            if (Pabc2->has_char(c) && is_char_valid_for_filename(c)) {
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
            if (c == KEY_BACKSPACE) {

                // Torles:
                int ikesebbnulla = 0;
                if (i <= 0) {
                    ikesebbnulla = 1;
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
            if (c == KEY_DEL) {
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
            if (c == KEY_LEFT && i > 0) {
                i--;
                eddignev[0] = 0;
            }
            if (c == KEY_RIGHT && i < (int)strlen(nev)) {
                i++;
                eddignev[0] = 0;
            }
        }
        if (right_mouse_clicked()) {
            return 0;
        }
        if (left_mouse_clicked()) {
            update_and_draw_cursor();
            // Most nezzuk meg, hogy hova kattintott:
            if (is_in_box(Moux, Mouy, boxsave)) {
                if (nev[0] == 0) {
                    continue;
                }
                // Most mentunk:
                // Eloallitjuk nevet:
                strcat(nev, ".lev");
                char tmpnev[40];
                strcpy(tmpnev, "lev/");
                strcat(tmpnev, nev);
                if (access(tmpnev, 0) == 0) {
                    if (dialog("File exists, overwrite?", nev, DIALOG_BUTTONS, "Yes", "No") == 1) {
                        return 0;
                    }
                }
                Ptop->save(nev);
                strcpy(State->editor_filename, nev);
                Valtozott = 0;
                return 1;
            }
        }
        update_and_draw_cursor();
    }
}

static unsigned char Szinkonvtomb[256];

static void makeszinkonvtomb(unsigned char* pal) {
    for (int i = 0; i < 256; i++) {
        int r = pal[i * 3];
        int g = pal[i * 3 + 1];
        int b = pal[i * 3 + 2];
        r >>= 6;
        g >>= 6;
        b >>= 6;
        int szin = r * 16 + g * 4 + b;
        if (szin < 0 || szin >= 64) {
            internal_error("897 648u9");
        }
        Szinkonvtomb[i] = (unsigned char)(szin + 64);
        // Szinkonvtomb[i] = (unsigned char)(63 + 64);
    }
}

// Szimpla
static void vegyesblt(pic8* dest, picture* pkep, int x0, int y0) {
    if (x0 < 0 || y0 < 0 || x0 >= dest->get_width() || y0 >= dest->get_height()) {
        internal_error("7r79yffuew");
    }

    int xsize = pkep->width;
    int ysize = pkep->height;
    int xtullog = 0;
    if (x0 + xsize > dest->get_width()) {
        xsize = dest->get_width() - x0;
        xtullog = 1;
    }
    if (y0 + ysize > dest->get_height()) {
        ysize = dest->get_height() - y0;
    }
    // Most xsize es ysize mindenkeppen benne van teljesen kepben:

    int i = 0;
    unsigned char* tomb = pkep->data;
    for (int y = y0; y < y0 + ysize; y++) {
        // Egy sor elintezese:
        unsigned char* dsor = dest->get_row(y) + x0;

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
static void vegyesblt(pic8* dest, texture* pt, mask* pm, int x0, int y0) {
    if (x0 < 0 || y0 < 0 || x0 >= dest->get_width() || y0 >= dest->get_height()) {
        internal_error("7r79yffuew");
    }

    pic8* source = pt->pic;

    int maxysize = pm->height;
    if (y0 + maxysize > dest->get_height()) {
        maxysize = dest->get_height() - y0;
    }
    // Most xsize es ysize mindenkeppen benne van teljesen kepben:
    int i = 0;
    int picxsize = source->get_width();
    for (int y = y0; y < y0 + maxysize; y++) {
        unsigned char* dsor = dest->get_row(y) + x0;
        unsigned char* ssor = source->get_row((y - y0) % source->get_height());
        int sx = 0;
        int abszx = x0;
        // Maszkos eljaras:
        while (pm->data[i].type != MaskEncoding::EndOfLine) {
            if (pm->data[i].type == MaskEncoding::Solid) {
                for (int j = 0; j < pm->data[i].length; j++) {
                    if (abszx < 640) {
                        *dsor = Szinkonvtomb[ssor[sx]];
                    }
                    dsor++;
                    sx++;
                    sx %= picxsize;
                    abszx++;
                }
            } else {
                dsor += pm->data[i].length;
                sx += pm->data[i].length;
                sx %= picxsize;
                abszx += pm->data[i].length;
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
        nevszam = Lgr->picture_count;
    }
    if (tipus == 2) {
        nev = texturanev;
        nevszam = Lgr->texture_count;
    }
    if (tipus == 3) {
        nev = maszknev;
        nevszam = Lgr->mask_count;
    }

    if (!nev) {
        internal_error("liytg");
    }

    if (nevszam < 1) {
        return;
    }

    char orignev[10];
    if (strlen(nev) > 8) {
        internal_error("8guiojg");
    }
    strcpy(orignev, nev);

    int kur = 0; // Ha megtalalja eddigit, akkor kur-t raallitja
    for (int i = 0; i < nevszam; i++) {
        if (tipus == 1 && strcmpi(Lgr->pictures[i].name, nev) == 0) {
            kur = i;
        }
        if (tipus == 2 && strcmpi(Lgr->textures[i].name, nev) == 0) {
            kur = i;
        }
        if (tipus == 3 && strcmpi(Lgr->masks[i].name, nev) == 0) {
            kur = i;
        }
    }

    if (kur == 0) {
        if (tipus == 1) {
            strcpy(nev, Lgr->pictures[0].name);
        }
        if (tipus == 2) {
            strcpy(nev, Lgr->textures[0].name);
        }
        if (tipus == 3) {
            strcpy(nev, Lgr->masks[0].name);
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
    empty_keypress_buffer();

    // Masodik bufferben mindig ilyenkor benne van eredeti kep:
    int buffervaltozottx = 0;
    int buffervaltozotty = 0;
    push();
    blit8(BufferBall, BufferMain);
    // BufferBall->save( "golyo.pcx" );
    // internal_error( "golyo mentve!" );
    pop();

    // Azert hogy ha mar le volt nyomva, ne nyomodjon ujra:
    left_mouse_clicked();
    while (1) {
        // Nezzuk gombokat:
        while (has_keypress()) {
            Keycode c = get_keypress();
            if (c == KEY_ESC) {
                strcpy(nev, orignev);
                return;
            }
            if (c == KEY_ENTER) {
                igazit(&kur, &felso, nevszam, helyszam);
                if (tipus == 1) {
                    strcpy(nev, Lgr->pictures[kur].name);
                }
                if (tipus == 2) {
                    strcpy(nev, Lgr->textures[kur].name);
                }
                if (tipus == 3) {
                    strcpy(nev, Lgr->masks[kur].name);
                }
                return;
            }
            if (c == KEY_UP) {
                kur--;
            }
            if (c == KEY_PGUP) {
                kur -= helyszam - 1;
            }
            if (c == KEY_DOWN) {
                kur++;
            }
            if (c == KEY_PGDOWN) {
                kur += helyszam - 1;
            }
        }
        // Eger lenyomas:
        if (left_mouse_clicked()) {
            update_and_draw_cursor();
            // Most nezzuk meg, hogy hova kattintott:
            if (is_in_box(Moux, Mouy, boxle)) {
                kur += helyszam - 1;
            }
            if (is_in_box(Moux, Mouy, boxfel)) {
                kur -= helyszam - 1;
            }
            if (is_in_box(Moux, Mouy, boxcancel)) {
                strcpy(nev, orignev);
                return;
            }
            if (cleares && is_in_box(Moux, Mouy, boxclear)) {
                nev[0] = 0;
                return;
            }
            if (Moux > lx1 && Moux < lx2 && Mouy > ly1 && Mouy < ly2) {
                // Lista folott nyomta meg gombot:
                if (Mouy < ly1 + dy * helyszam) {
                    // Egy nevet el is talalt:
                    int index = (Mouy - ly1) / dy;
                    index += felso;
                    if (index < nevszam) {
                        if (tipus == 1) {
                            strcpy(nev, Lgr->pictures[index].name);
                        }
                        if (tipus == 2) {
                            strcpy(nev, Lgr->textures[index].name);
                        }
                        if (tipus == 3) {
                            strcpy(nev, Lgr->masks[index].name);
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
                strcpy(nev, Lgr->pictures[kur].name);
            }
            if (tipus == 2) {
                strcpy(nev, Lgr->textures[kur].name);
            }
            if (tipus == 3) {
                strcpy(nev, Lgr->masks[kur].name);
            }

            push();

            blit8(BufferMain, BufferBall, 0, 0, 0, 0, buffervaltozottx, buffervaltozotty);

            if (kepnev[0] && (texturanev[0] || maszknev[0])) {
                internal_error("iugriurg");
            }

            if (kepnev[0]) {
                // Ez egy kep:
                int index = Lgr->get_picture_index(kepnev);
                if (index < 0) {
                    internal_error("hjguyt");
                }
                picture* pkep = &Lgr->pictures[index];
                vegyesblt(BufferMain, pkep, 10, 10);
                buffervaltozottx = 10 + pkep->width - 1;
                buffervaltozotty = 10 + pkep->height - 1;
            }

            if (texturanev[0] && maszknev[0]) {
                // Ez egy texturas maszk:
                int index = Lgr->get_texture_index(texturanev);
                if (index < 0) {
                    internal_error(";lgrtijo");
                }
                texture* ptextura = &Lgr->textures[index];

                index = Lgr->get_mask_index(maszknev);
                if (index < 0) {
                    internal_error("lkjefrg");
                }
                mask* pmaszk = &Lgr->masks[index];

                vegyesblt(BufferMain, ptextura, pmaszk, 10, 10);
                buffervaltozottx = 10 + pmaszk->width - 1;
                buffervaltozotty = 10 + pmaszk->height - 1;
            }

            render_box(BufferMain, x1, y1, x2, y2, DIALOG_PALETTE_ID, DIALOG_BORDER_PALETTE_ID);
            render_box(BufferMain, lx1, ly1, lx2, ly2, Openlistaszin, DIALOG_BORDER_PALETTE_ID);
            for (int i = 0; i < helyszam && i + felso < nevszam; i++) {
                if (i + felso == kur) {
                    // Ez most kurens nev:
                    BufferMain->fill_box(lx1 + 1, ly1 + i * dy + 1, lx2 - 1, ly1 + (i + 1) * dy - 1,
                                         Openlistainvszin);
                }
                // Kiirjuk nevet:
                if (tipus == 1) {
                    Pabc2->write(BufferMain, lx1 + 3, ly1 + 15 + i * dy,
                                 Lgr->pictures[i + felso].name);
                }
                if (tipus == 2) {
                    Pabc2->write(BufferMain, lx1 + 3, ly1 + 15 + i * dy,
                                 Lgr->textures[i + felso].name);
                }
                if (tipus == 3) {
                    Pabc2->write(BufferMain, lx1 + 3, ly1 + 15 + i * dy,
                                 Lgr->masks[i + felso].name);
                }

                // Kiirjuk tavot:
                if (kepnev[0] || texturanev[0]) {
                    int tavolsag = 0;
                    if (kepnev[0]) {
                        tavolsag = Lgr->pictures[i + felso].default_distance;
                    } else {
                        tavolsag = Lgr->textures[i + felso].default_distance;
                    }

                    char tavstr[20];
                    sprintf(tavstr, "%d", tavolsag);
                    Pabc2->write(BufferMain, lx1 + 67, ly1 + 15 + i * dy, tavstr);
                    // Kiirjuk hatarolokat:
                    Clipping hatarol = Clipping::Unclipped;
                    if (kepnev[0]) {
                        hatarol = Lgr->pictures[i + felso].default_clipping;
                    } else {
                        hatarol = Lgr->textures[i + felso].default_clipping;
                    }

                    strcpy(tavstr, "U");
                    if (hatarol == Clipping::Ground) {
                        strcpy(tavstr, "G");
                    }
                    if (hatarol == Clipping::Sky) {
                        strcpy(tavstr, "S");
                    }
                    Pabc2->write(BufferMain, lx1 + 92, ly1 + 15 + i * dy, tavstr);
                }
            }
            // Most jonnek gombok:
            render_box(BufferMain, boxfel, BUTTON_PALETTE_ID, DIALOG_BORDER_PALETTE_ID);
            boxbarajz(BufferMain, boxfel, DIALOG_BORDER_PALETTE_ID, 1);
            render_box(BufferMain, boxle, BUTTON_PALETTE_ID, DIALOG_BORDER_PALETTE_ID);
            boxbarajz(BufferMain, boxle, DIALOG_BORDER_PALETTE_ID, 0);
            render_box(BufferMain, boxcancel, BUTTON_PALETTE_ID, DIALOG_BORDER_PALETTE_ID);
            Pabc2->write_centered(BufferMain, (boxcancel.x1 + boxcancel.x2) / 2, boxcancel.y1 + 15,
                                  "CANCEL");
            if (cleares) {
                render_box(BufferMain, boxclear, BUTTON_PALETTE_ID, DIALOG_BORDER_PALETTE_ID);
                Pabc2->write_centered(BufferMain, (boxclear.x1 + boxclear.x2) / 2, boxclear.y1 + 15,
                                      "CLEAR");
            }

            bltfront(BufferMain);
            pop();

            eddigfelso = felso;
            eddigkur = kur;
        }
        update_and_draw_cursor();
    }
}

void tolt_pickasprite(void) {
    if (!Lgr) {
        internal_error("tolt_pickasprite-ban !Lgr!");
    }

    makeszinkonvtomb(Lgr->palette_data);

    invalidateegesz();

    char kepnev[10], texturanev[10], maszknev[10], uresnev[10] = "";
    strcpy(kepnev, Lgr->editor_picture_name);
    strcpy(texturanev, Lgr->editor_texture_name);
    strcpy(maszknev, Lgr->editor_mask_name);

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

    empty_keypress_buffer();

    left_mouse_clicked();
    int invalid = 1;
    while (1) {
        // Nezzuk gombokat:
        while (has_keypress()) {
            Keycode c = get_keypress();
            if (c == KEY_ESC) {
                return;
            }
            if (c == KEY_ENTER) {
                strcpy(Lgr->editor_picture_name, kepnev);
                strcpy(Lgr->editor_texture_name, texturanev);
                strcpy(Lgr->editor_mask_name, maszknev);
                return;
            }
        }
        // Eger lenyomas:
        if (left_mouse_clicked()) {
            update_and_draw_cursor();
            // Most nezzuk meg, hogy hova kattintott:
            if (is_in_box(Moux, Mouy, boxok)) {
                strcpy(Lgr->editor_picture_name, kepnev);
                strcpy(Lgr->editor_texture_name, texturanev);
                strcpy(Lgr->editor_mask_name, maszknev);
                return;
            }
            if (is_in_box(Moux, Mouy, boxcancel)) {
                return;
            }
            if (is_in_box(Moux, Mouy, boxkepnev)) {
                // Kepnevet akarja valtoztatni:
                pickanev(kepnev, uresnev, uresnev, 1);
                push();
                blit8(BufferMain, BufferBall);
                bltfront(BufferMain);
                pop();
                if (kepnev[0]) {
                    texturanev[0] = maszknev[0] = 0;
                }
                invalid = 1;
            }
            if (is_in_box(Moux, Mouy, boxtexturanev)) {
                // Texturanevet akarja valtoztatni:
                pickanev(uresnev, texturanev, maszknev, 2);
                push();
                blit8(BufferMain, BufferBall);
                bltfront(BufferMain);
                pop();
                if (texturanev[0]) {
                    kepnev[0] = 0;
                }
                invalid = 1;
            }
            if (is_in_box(Moux, Mouy, boxmaszknev)) {
                // Maszknevet akarja valtoztatni:
                pickanev(uresnev, texturanev, maszknev, 3);
                push();
                blit8(BufferMain, BufferBall);
                bltfront(BufferMain);
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
            render_box(BufferMain, x1, y1, x2, y2, DIALOG_PALETTE_ID, DIALOG_BORDER_PALETTE_ID);

            Pabc2->write_centered(BufferMain, (x1 + x2) / 2, y1 + 20, "Choose picture");

            // Most jonnek gombok:
            render_box(BufferMain, boxok, BUTTON_PALETTE_ID, DIALOG_BORDER_PALETTE_ID);
            render_box(BufferMain, boxcancel, BUTTON_PALETTE_ID, DIALOG_BORDER_PALETTE_ID);
            Pabc2->write_centered(BufferMain, (boxok.x1 + boxok.x2) / 2, boxok.y1 + 15, "OK");
            Pabc2->write_centered(BufferMain, (boxcancel.x1 + boxcancel.x2) / 2, boxcancel.y1 + 15,
                                  "CANCEL");

            // Most jonnek nevek:
            // Picture:
            int szovx1 = x1 + 8;
            Pabc2->write(BufferMain, szovx1, boxkepnev.y1 + 15, "Normal Picture:");
            render_box(BufferMain, boxkepnev, Feherszin, DIALOG_BORDER_PALETTE_ID);
            boxbair_balra(BufferMain, boxkepnev, Feherszin, kepnev);
            // Textura:
            Pabc2->write(BufferMain, szovx1, boxtexturanev.y1 + 15, "Texture:");
            render_box(BufferMain, boxtexturanev, Feherszin, DIALOG_BORDER_PALETTE_ID);
            boxbair_balra(BufferMain, boxtexturanev, Feherszin, texturanev);
            // Maszk:
            Pabc2->write(BufferMain, szovx1, boxmaszknev.y1 + 15, "Mask:");
            render_box(BufferMain, boxmaszknev, Feherszin, DIALOG_BORDER_PALETTE_ID);
            boxbair_balra(BufferMain, boxmaszknev, Feherszin, maszknev);

            bltfront(BufferMain);
            pop();
        }
        update_and_draw_cursor();
    }
}

int tolt_picklgrfile(char* lgrnev) {
    invalidateegesz();
    blit8(BufferBall, BufferMain);
    if (!Soknev) {
        Soknev = new char[Maxnev * 10 + 100];
        if (!Soknev) {
            internal_error("Nincs eleg memoria tolt_picklgrfile-ban!");
        }
    }
    for (int i = 0; i < Maxnev * 10 + 10; i++) {
        Soknev[i] = 0;
    }
    finame fname;
    bool done = find_first("lgr/*.lgr", fname);
    int nevszam = 0;
    while (!done) {
        if (strlen(fname) > 14) {
            internal_error("876759989");
        }
        char* nev = &Soknev[nevszam * 10];
        strcpy(nev, fname);
        // Leveszi kiterjesztest:
        int i = 0;
        while (nev[i] != '.') {
            if (!nev[i]) {
                internal_error("Nincs pont nevben! (454534): ", nev);
            }
            i++;
        }
        nev[i] = 0;
        if (strlen(nev) > 8) {
            internal_error("3875665");
        }

        done = find_next(fname);
        nevszam++;
        if (nevszam >= Maxnev) {
            dialog("Number of files > 1000 in Pick LGR File dialog box!",
                   "Could not load all the files!");
            done = 1;
        }
    }

    find_close();

    if (nevszam < 1) {
        external_error("There is not any LGR files (*.lgr) in LGR directory!");
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
    empty_keypress_buffer();
    left_mouse_clicked();
    while (1) {
        // Nezzuk gombokat:
        while (has_keypress()) {
            Keycode c = get_keypress();
            if (c == KEY_ESC) {
                return 0;
            }
            if (c == KEY_ENTER) {
                igazit(&kur, &felso, nevszam, helyszam);
                strcpy(lgrnev, &Soknev[kur * 10]);
                return 1;
            }
            if (c == KEY_UP) {
                kur--;
            }
            if (c == KEY_PGUP) {
                kur -= helyszam - 1;
            }
            if (c == KEY_DOWN) {
                kur++;
            }
            if (c == KEY_PGDOWN) {
                kur += helyszam - 1;
            }
        }

        // Eger lenyomas:
        if (left_mouse_clicked()) {
            update_and_draw_cursor();
            // Most nezzuk meg, hogy hova kattintott:
            if (is_in_box(Moux, Mouy, boxle)) {
                kur += helyszam - 1;
            }
            if (is_in_box(Moux, Mouy, boxfel)) {
                kur -= helyszam - 1;
            }
            if (is_in_box(Moux, Mouy, boxcancel)) {
                return 0;
            }
            if (Moux > lx1 && Moux < lx2 && Mouy > ly1 && Mouy < ly2) {
                // Lista folott nyomta meg gombot:
                if (Mouy < ly1 + dy * helyszam) {
                    // Egy nevet el is talalt:
                    int index = (Mouy - ly1) / dy;
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
            render_box(BufferMain, x1, y1, x2, y2, DIALOG_PALETTE_ID, DIALOG_BORDER_PALETTE_ID);
            render_box(BufferMain, lx1, ly1, lx2, ly2, Openlistaszin, DIALOG_BORDER_PALETTE_ID);
            for (int i = 0; i < helyszam && i < nevszam; i++) {
                if (i + felso == kur) {
                    // Ez most kurens nev:
                    BufferMain->fill_box(lx1 + 1, ly1 + i * dy + 1, lx2 - 1, ly1 + (i + 1) * dy - 1,
                                         Openlistainvszin);
                }
                Pabc2->write(BufferMain, lx1 + 3, ly1 + 15 + i * dy, &Soknev[(i + felso) * 10]);
            }
            // Most jonnek gombok:
            render_box(BufferMain, boxfel, BUTTON_PALETTE_ID, DIALOG_BORDER_PALETTE_ID);
            boxbarajz(BufferMain, boxfel, DIALOG_BORDER_PALETTE_ID, 1);
            render_box(BufferMain, boxle, BUTTON_PALETTE_ID, DIALOG_BORDER_PALETTE_ID);
            boxbarajz(BufferMain, boxle, DIALOG_BORDER_PALETTE_ID, 0);
            render_box(BufferMain, boxcancel, BUTTON_PALETTE_ID, DIALOG_BORDER_PALETTE_ID);
            Pabc2->write_centered(BufferMain, (boxcancel.x1 + boxcancel.x2) / 2, boxcancel.y1 + 15,
                                  "CANCEL");

            // Kiirjuk eredeti file nevet:
            char tttmp[50];
            strcpy(tttmp, lgrnev);
            // strcat( tttmp, ".LGR" );
            Pabc2->write_centered(BufferMain, (x1 + x2) / 2, y2 - 36, "Original LGR file:");
            Pabc2->write_centered(BufferMain, (x1 + x2) / 2, y2 - 18, tttmp);

            bltfront(BufferMain, x1, y1, x2, y2);
            pop();

            eddigfelso = felso;
            eddigkur = kur;
        }
        update_and_draw_cursor();
    }
}

static int tavolsagbeir(pic8* ppic, box abox, int tavolsag) {
    push();

    empty_keypress_buffer();
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
        Keycode c = get_keypress();
        if (c == KEY_ESC) {
            pop();
            return oldtavolsag;
        }
        if (c == KEY_ENTER) {
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
        if (c == KEY_BACKSPACE) {
            if (tavolsag > 0) {
                tavolsag /= 10;
                valtozott = 1;
            }
        }
    }
}

static Clipping hatarolbeir(pic8* ppic, box abox, Clipping hatarol) {
    push();
    boxbair(ppic, abox, Feherszin, "", 1);
    bltfront(ppic, abox.x1, abox.y1, abox.x2, abox.y2);
    empty_keypress_buffer();
    while (1) {
        Keycode c = get_keypress();
        if (c == KEY_ESC) {
            pop();
            return hatarol;
        }
        if (c == 'u' || c == 'U') {
            pop();
            return Clipping::Unclipped;
        }
        if (c == 'g' || c == 'G') {
            pop();
            return Clipping::Ground;
        }
        if (c == 's' || c == 'S') {
            pop();
            return Clipping::Sky;
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
    left_mouse_clicked();
    int tavolsag = psp->distance;
    Clipping hatarol = psp->clipping;
    while (1) {
        // Nezzuk gombokat:
        while (has_keypress()) {
            Keycode c = get_keypress();
            if (c == KEY_ESC) {
                return;
            }
            if (c == KEY_ENTER) {
                if (psp->distance != tavolsag || psp->clipping != hatarol) {
                    Valtozott = 1;
                }
                psp->distance = tavolsag;
                psp->clipping = hatarol;
                return;
            }
        }
        // Eger lenyomas:
        if (left_mouse_clicked()) {
            update_and_draw_cursor();
            // Most nezzuk meg, hogy hova kattintott:
            if (is_in_box(Moux, Mouy, boxok)) {
                if (psp->distance != tavolsag || psp->clipping != hatarol) {
                    Valtozott = 1;
                }
                psp->distance = tavolsag;
                psp->clipping = hatarol;
                return;
            }
            if (is_in_box(Moux, Mouy, boxcancel)) {
                return;
            }
            if (is_in_box(Moux, Mouy, boxtav)) {
                tavolsag = tavolsagbeir(BufferMain, boxtav, tavolsag);
                kepvaltozott = 1;
            }
            if (is_in_box(Moux, Mouy, boxclipp)) {
                hatarol = hatarolbeir(BufferMain, boxclipp, hatarol);
                kepvaltozott = 1;
            }
        }
        if (kepvaltozott) {
            kepvaltozott = 0;
            // Kirajzoljuk, mivel valtozott kep:
            push();
            render_box(BufferMain, x1, y1, x2, y2, DIALOG_PALETTE_ID, DIALOG_BORDER_PALETTE_ID);
            // Most jonnek gombok:
            render_box(BufferMain, boxok, BUTTON_PALETTE_ID, DIALOG_BORDER_PALETTE_ID);
            render_box(BufferMain, boxcancel, BUTTON_PALETTE_ID, DIALOG_BORDER_PALETTE_ID);
            Pabc2->write_centered(BufferMain, (boxok.x1 + boxok.x2) / 2, boxok.y1 + 15, "OK");
            Pabc2->write_centered(BufferMain, (boxcancel.x1 + boxcancel.x2) / 2, boxcancel.y1 + 15,
                                  "CANCEL");

            Pabc2->write_centered(BufferMain, (x1 + x2) / 2, y1 + 15, "Set Picture Properties");
            if (psp->picture_name[0]) {
                Pabc2->write_centered(BufferMain, (x1 + x2) / 2, y1 + 45, psp->picture_name);
            } else {
                Pabc2->write_centered(BufferMain, (x1 + x2) / 2, y1 + 34, psp->texture_name);
                Pabc2->write_centered(BufferMain, (x1 + x2) / 2, y1 + 56, psp->mask_name);
            }
            Pabc2->write(BufferMain, x1 + 92, y1 + 76, "Distance");
            Pabc2->write(BufferMain, x1 + 172, y1 + 76, "Clipping");
            // Default-ok kiirasa:
            Pabc2->write(BufferMain, x1 + 12, y1 + 100, "Default:");
            int deftav = -1;
            Clipping defhatarol = Clipping::Unknown;
            if (psp->picture_name[0]) {
                int index = Lgr->get_picture_index(psp->picture_name);
                if (index < 0) {
                    internal_error("iohdiuyde");
                }
                deftav = Lgr->pictures[index].default_distance;
                defhatarol = Lgr->pictures[index].default_clipping;
            }
            if (psp->texture_name[0]) {
                int index = Lgr->get_texture_index(psp->texture_name);
                if (index < 0) {
                    internal_error("u6ythggf");
                }
                deftav = Lgr->textures[index].default_distance;
                defhatarol = Lgr->textures[index].default_clipping;
            }
            // Tavolsag:
            char tmp[10];
            if (deftav >= 0) {
                sprintf(tmp, "%d", deftav);
            } else {
                sprintf(tmp, "-");
            }
            Pabc2->write_centered(BufferMain, x1 + 119, y1 + 100, tmp);

            // Hatarolas:
            if (defhatarol == Clipping::Unknown) {
                strcpy(tmp, "-");
            }
            if (defhatarol == Clipping::Unclipped) {
                strcpy(tmp, "U");
            }
            if (defhatarol == Clipping::Ground) {
                strcpy(tmp, "G");
            }
            if (defhatarol == Clipping::Sky) {
                strcpy(tmp, "S");
            }
            Pabc2->write_centered(BufferMain, x1 + 198, y1 + 100, tmp);

            Pabc2->write(BufferMain, x1 + 12, y1 + 130, "Current:");
            // Tavolsag boxba:
            render_box(BufferMain, boxtav, Feherszin, DIALOG_BORDER_PALETTE_ID);
            sprintf(tmp, "%d", tavolsag);
            boxbair(BufferMain, boxtav, Feherszin, tmp);
            Pabc2->write_centered(BufferMain, x1 + 119, boxtav.y2 + 14, "(1-999)");
            // Clipping boxba:
            render_box(BufferMain, boxclipp, Feherszin, DIALOG_BORDER_PALETTE_ID);
            if (hatarol == Clipping::Unclipped) {
                strcpy(tmp, "U");
            }
            if (hatarol == Clipping::Ground) {
                strcpy(tmp, "G");
            }
            if (hatarol == Clipping::Sky) {
                strcpy(tmp, "S");
            }
            boxbair(BufferMain, boxclipp, Feherszin, tmp);
            Pabc2->write_centered(BufferMain, x1 + 198, boxclipp.y2 + 14, "(U, S, G)");

            bltfront(BufferMain, x1, y1, x2, y2);
            pop();
        }
        update_and_draw_cursor();
    }
}

void set_gyuru_attributes(polygon* pgy) {
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
    left_mouse_clicked();

    int koveto = pgy->is_grass;

    while (1) {
        // Nezzuk gombokat:
        while (has_keypress()) {
            Keycode c = get_keypress();
            if (c == KEY_ESC) {
                return;
            }
            if (c == KEY_ENTER) {
                // Meg van ismetelve OK-nal is lejjebb:
                if (pgy->is_grass != koveto) {
                    Valtozott = 1;
                }
                pgy->is_grass = koveto;
                return;
            }
        }
        // Eger lenyomas:
        if (left_mouse_clicked()) {
            update_and_draw_cursor();
            // Most nezzuk meg, hogy hova kattintott:
            if (is_in_box(Moux, Mouy, boxok)) {
                // Meg van ismetelve ENTER-nel is feljebb:
                if (pgy->is_grass != koveto) {
                    Valtozott = 1;
                }
                pgy->is_grass = koveto;
                return;
            }
            if (is_in_box(Moux, Mouy, boxcancel)) {
                return;
            }
            if (is_in_box(Moux, Mouy, boxkoveto)) {
                koveto = !koveto;
                kepvaltozott = 1;
            }
        }
        if (kepvaltozott) {
            kepvaltozott = 0;
            // Kirajzoljuk, mivel valtozott kep:
            push();
            render_box(BufferMain, x1, y1, x2, y2, DIALOG_PALETTE_ID, DIALOG_BORDER_PALETTE_ID);

            Pabc2->write_centered(BufferMain, (x1 + x2) / 2, y1 + 15, "Set Polygon Properties");

            // Most jonnek gombok:
            // Koveto vagy nem gomb:
            int szovegx1 = x1 + 18;
            Pabc2->write(BufferMain, szovegx1, boxkoveto.y1 + 15, "Grass polygon:");
            render_box(BufferMain, boxkoveto, Feherszin, DIALOG_BORDER_PALETTE_ID);
            if (koveto) {
                Pabc2->write_centered(BufferMain, (boxkoveto.x1 + boxkoveto.x2) / 2,
                                      boxkoveto.y1 + 15, "YES");
            } else {
                Pabc2->write_centered(BufferMain, (boxkoveto.x1 + boxkoveto.x2) / 2,
                                      boxkoveto.y1 + 15, "NO");
            }

            render_box(BufferMain, boxok, BUTTON_PALETTE_ID, DIALOG_BORDER_PALETTE_ID);
            render_box(BufferMain, boxcancel, BUTTON_PALETTE_ID, DIALOG_BORDER_PALETTE_ID);
            Pabc2->write_centered(BufferMain, (boxok.x1 + boxok.x2) / 2, boxok.y1 + 15, "OK");
            Pabc2->write_centered(BufferMain, (boxcancel.x1 + boxcancel.x2) / 2, boxcancel.y1 + 15,
                                  "CANCEL");

            bltfront(BufferMain, x1, y1, x2, y2);
            pop();
        }
        update_and_draw_cursor();
    }
}

void set_kerek_tipus(const char* fejlec, object::Property* pkajatipus, int* pfoodsorszam) {
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
    render_box(BufferMain, x1, y1, x2, y2, DIALOG_PALETTE_ID, DIALOG_BORDER_PALETTE_ID);
    Pabc2->write_centered(BufferMain, (x1 + x2) / 2, y1 + 20, fejlec);
    render_box(BufferMain, boxcancel, BUTTON_PALETTE_ID, DIALOG_BORDER_PALETTE_ID);
    Pabc2->write_centered(BufferMain, (boxcancel.x1 + boxcancel.x2) / 2, boxcancel.y1 + 15,
                          "Cancel");
    // Anim szam boxa:
    Pabc2->write(BufferMain, boxanimszam.x1 - 162, boxanimszam.y1 + 15, "Food anim number (1-9):");
    char tmp[20];
    sprintf(tmp, "%d", (int)(*pfoodsorszam + 1));
    render_box(BufferMain, boxanimszam, Feherszin, DIALOG_BORDER_PALETTE_ID);
    Pabc2->write_centered(BufferMain, (boxanimszam.x1 + boxanimszam.x2) / 2, boxanimszam.y1 + 15,
                          tmp);

    for (int i = 0; i < listaszam; i++) {
        render_box(BufferMain, boxlista, Openlistaszin, DIALOG_BORDER_PALETTE_ID);
        int lx = (boxlista.x1 + boxlista.x2) / 2;
        int ly = boxlista.y1 + 15;
        switch (i) {
        case 0:
            Pabc2->write_centered(BufferMain, lx, ly, "Normal Food");
            break;
        case 1:
            Pabc2->write_centered(BufferMain, lx, ly, "Gravity Up");
            break;
        case 2:
            Pabc2->write_centered(BufferMain, lx, ly, "Gravity Down");
            break;
        case 3:
            Pabc2->write_centered(BufferMain, lx, ly, "Gravity Left");
            break;
        case 4:
            Pabc2->write_centered(BufferMain, lx, ly, "Gravity Right");
            break;
        default:
            internal_error("yuiwegfyjkweg");
        }
        boxlista.y1 += dy;
        boxlista.y2 += dy;
    }
    boxlista.y1 = ly1;
    boxlista.y2 = ly2;

    bltfront(BufferMain, x1, y1, x2, y2);
    pop();

    // Most jon ciklus:
    left_mouse_clicked();
    while (1) {
        // Nezzuk gombokat:
        while (has_keypress()) {
            Keycode c = get_keypress();
            if (c == KEY_ESC) {
                return;
            }
        }
        // Eger lenyomas:
        if (left_mouse_clicked()) {
            update_and_draw_cursor();
            // Most nezzuk meg, hogy hova kattintott:
            if (is_in_box(Moux, Mouy, boxcancel)) {
                return;
            }
            if (is_in_box(Moux, Mouy, boxanimszam)) {
                // Berajzolunk egy vizszintes kurzort:
                push(); // Leveszi egeret
                render_box(BufferMain, boxanimszam, Feherszin, DIALOG_BORDER_PALETTE_ID);
                Pabc2->write_centered(BufferMain, (boxanimszam.x1 + boxanimszam.x2) / 2,
                                      boxanimszam.y1 + 18, "-");
                bltfront(BufferMain, x1, y1, x2, y2);

                empty_keypress_buffer();
                while (1) {
                    // Kirajzolta dobozba kurzort, most 1..9, vagy ESC-et var:
                    Keycode c = get_keypress();
                    if (c == KEY_ESC) {
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
            if (is_in_box(Moux, Mouy, boxlista)) {
                // Benne van listaban:
                int index = (Mouy - boxlista.y1) / dy;
                if (index >= listaszam) {
                    index = listaszam - 1;
                }
                Valtozott = 1;
                switch (index) {
                case 0:
                    *pkajatipus = object::Property::None;
                    break;
                case 1:
                    *pkajatipus = object::Property::GravityUp;
                    break;
                case 2:
                    *pkajatipus = object::Property::GravityDown;
                    break;
                case 3:
                    *pkajatipus = object::Property::GravityLeft;
                    break;
                case 4:
                    *pkajatipus = object::Property::GravityRight;
                    break;
                default:
                    internal_error("0893ytpoerhg");
                }
                return;
            }
        }
        update_and_draw_cursor();
    }
}

static void change_level_name(char* nev) {
    char orignev[LEVEL_NAME_LENGTH + 10];
    strcpy(orignev, nev);

    push(); // Itt levesszuk egeret es nem is tesszuk vissza vegeig:

    blit8(BufferBall, BufferMain);

    invalidateegesz();
    // Itt most be kell olvasnunk egy nevet:
    int x1 = 50;
    int y1 = 200;
    int x2 = 600;
    int y2 = 280;

    empty_keypress_buffer();
    int i = 0;
    while (1) {
        // Kiir:
        BufferMain->fill_box(x1, y1, x2, y2, Save_as_box_szin);
        BufferMain->line(x1, y1, x2, y1, Save_as_szelszin);
        BufferMain->line(x1, y2, x2, y2, Save_as_szelszin);
        BufferMain->line(x1, y1, x1, y2, Save_as_szelszin);
        BufferMain->line(x2, y1, x2, y2, Save_as_szelszin);
        Pabc2->write_centered(
            BufferMain, (x2 + x1) / 2, 220,
            "Type in the name of the level and the designer and press ENTER (ESC to cancel)!");
        int kezdes = (x2 + x1) / 2 - Pabc1->len(nev) / 2;
        Pabc2->write(BufferMain, kezdes, 250, nev);
        char tmp[LEVEL_NAME_LENGTH + 10];
        strcpy(tmp, nev);
        tmp[i] = 0;
        Pabc2->write(BufferMain, kezdes + Pabc1->len(tmp), 255, "-");
        bltfront(BufferMain, x1, y1, x2, y2);

        Keycode c = get_keypress();
        if (c == KEY_ESC) {
            strcpy(nev, orignev);
            pop();
            return;
        }
        if (c == KEY_ENTER) {
            if (nev[0] == 0) {
                continue;
            }
            pop();
            return;
        }
        if (Pabc2->has_char(c) && is_char_valid_for_filename(c)) {
            char tmpfejlec[LEVEL_NAME_LENGTH + 10];
            sprintf(tmpfejlec, "Ext: %s", nev);

            if (strlen(nev) >= LEVEL_NAME_LENGTH || MenuFont->len(tmpfejlec) > 625) {
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
        if (c == KEY_BACKSPACE) {
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
        if (c == KEY_DEL) {
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
        if (c == KEY_LEFT && i > 0) {
            i--;
        }
        if (c == KEY_RIGHT && i < (int)strlen(nev)) {
            i++;
        }
    }
}

void change_level_properties(void) {
    invalidateegesz();

    char fgnev[10];
    char bgnev[10];
    char levelname[LEVEL_NAME_LENGTH + 20];
    strcpy(fgnev, Ptop->foreground_name);
    strcpy(bgnev, Ptop->background_name);
    strcpy(levelname, Ptop->level_name);

    char lgrnev[20] = "";
    strcpy(lgrnev, Ptop->lgr_name);

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

    left_mouse_clicked();
    int invalid = 1;
    while (1) {
        // Nezzuk gombokat:
        while (has_keypress()) {
            Keycode c = get_keypress();
            if (c == KEY_ESC) {
                return;
            }
            if (c == KEY_ENTER) {
                if (strcmpi(Ptop->foreground_name, fgnev) != 0 ||
                    strcmpi(Ptop->background_name, bgnev) != 0 ||
                    strcmp(Ptop->level_name, levelname) != 0) {
                    Valtozott = 1;
                }
                strcpy(Ptop->foreground_name, fgnev);
                strcpy(Ptop->background_name, bgnev);
                strcpy(Ptop->level_name, levelname);

                if (strcmpi(lgrnev, Ptop->lgr_name) != 0) {
                    Valtozott = 1;
                    strcpy(Ptop->lgr_name, lgrnev);
                    lgrfile::load_lgr_file(Ptop->lgr_name);
                    if (Ptop->discard_missing_lgr_assets(Lgr)) {
                        dialogselejtez();
                    }
                }
                return;
            }
        }
        // Eger lenyomas:
        if (left_mouse_clicked()) {
            update_and_draw_cursor();
            // Most nezzuk meg, hogy hova kattintott:
            if (is_in_box(Moux, Mouy, boxok)) {
                if (strcmpi(Ptop->foreground_name, fgnev) != 0 ||
                    strcmpi(Ptop->background_name, bgnev) != 0 ||
                    strcmp(Ptop->level_name, levelname) != 0) {
                    Valtozott = 1;
                }
                strcpy(Ptop->foreground_name, fgnev);
                strcpy(Ptop->background_name, bgnev);
                strcpy(Ptop->level_name, levelname);

                if (strcmpi(lgrnev, Ptop->lgr_name) != 0) {
                    Valtozott = 1;
                    strcpy(Ptop->lgr_name, lgrnev);
                    lgrfile::load_lgr_file(Ptop->lgr_name);
                    if (Ptop->discard_missing_lgr_assets(Lgr)) {
                        dialogselejtez();
                    }
                }
                return;
            }
            if (is_in_box(Moux, Mouy, boxcancel)) {
                return;
            }
            if (is_in_box(Moux, Mouy, boxfgnev)) {
                // Foreground nevet akarja valtoztatni:
                char uresnev[10] = "";
                pickanev(uresnev, fgnev, uresnev, 2, 0);
                push();
                blit8(BufferMain, BufferBall);
                bltfront(BufferMain);
                pop();
                invalid = 1;
            }
            if (is_in_box(Moux, Mouy, boxbgnev)) {
                // Background nevet akarja valtoztatni:
                char uresnev[10] = "";
                pickanev(uresnev, bgnev, uresnev, 2, 0);
                push();
                blit8(BufferMain, BufferBall);
                bltfront(BufferMain);
                pop();
                invalid = 1;
            }
            if (is_in_box(Moux, Mouy, boxlevelname)) {
                // Palya nevet akarja valtoztatni:
                change_level_name(levelname);
                push();
                blit8(BufferMain, BufferBall);
                bltfront(BufferMain);
                pop();
                invalid = 1;
            }
            if (is_in_box(Moux, Mouy, boxlgrfile)) {
                char belgrnev[20] = "";
                strcpy(belgrnev, lgrnev);
                if (!tolt_picklgrfile(lgrnev)) {
                    strcpy(lgrnev, belgrnev);
                }
                push();
                blit8(BufferMain, BufferBall);
                bltfront(BufferMain);
                pop();
                invalid = 1;
            }
        }
        if (invalid) {
            /*static int nyomasszam = 0;
            nyomasszam++;
            if( nyomasszam > 10 )
                external_error( "nyomasszam >" );*/

            invalid = 0;

            push();
            // Kirajzoljuk, mivel valtozott kep:
            render_box(BufferMain, x1, y1, x2, y2, DIALOG_PALETTE_ID, DIALOG_BORDER_PALETTE_ID);

            Pabc2->write_centered(BufferMain, (x1 + x2) / 2, y1 + 22, "Level properties");

            // Most jonnek gombok:
            // OK:
            render_box(BufferMain, boxok, BUTTON_PALETTE_ID, DIALOG_BORDER_PALETTE_ID);
            Pabc2->write_centered(BufferMain, (boxok.x1 + boxok.x2) / 2, boxok.y1 + 15, "OK");
            // CANCEL:
            render_box(BufferMain, boxcancel, BUTTON_PALETTE_ID, DIALOG_BORDER_PALETTE_ID);
            Pabc2->write_centered(BufferMain, (boxcancel.x1 + boxcancel.x2) / 2, boxcancel.y1 + 15,
                                  "CANCEL");

            // Foreground sor:
            int szovegx = x1 + 10;
            Pabc2->write(BufferMain, szovegx, boxfgnev.y1 + 15, "Foreground:");
            render_box(BufferMain, boxfgnev, Feherszin, DIALOG_BORDER_PALETTE_ID);
            boxbair_balra(BufferMain, boxfgnev, Feherszin, fgnev);
            // Background sor:
            Pabc2->write(BufferMain, szovegx, boxbgnev.y1 + 15, "Background:");
            render_box(BufferMain, boxbgnev, Feherszin, DIALOG_BORDER_PALETTE_ID);
            boxbair_balra(BufferMain, boxbgnev, Feherszin, bgnev);
            // Level name sor:
            Pabc2->write(BufferMain, szovegx, boxlevelname.y1 + 15, "Level name:");
            render_box(BufferMain, boxlevelname, Feherszin, DIALOG_BORDER_PALETTE_ID);
            boxbair_balra(BufferMain, boxlevelname, Feherszin, levelname);
            // LGR file nev sor:
            Pabc2->write(BufferMain, szovegx, boxlgrfile.y1 + 15, "LGR file:");
            render_box(BufferMain, boxlgrfile, Feherszin, DIALOG_BORDER_PALETTE_ID);
            boxbair_balra(BufferMain, boxlgrfile, Feherszin, lgrnev);

            bltfront(BufferMain, x1, y1, x2, y2);
            pop();
        }
        update_and_draw_cursor();
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

    left_mouse_clicked();
    int invalid = 1;
    while (1) {
        // Nezzuk gombokat:
        while (has_keypress()) {
            Keycode c = get_keypress();
            if (c == KEY_ESC || c == KEY_ENTER) {
                return;
            }
        }
        // Eger lenyomas:
        if (left_mouse_clicked()) {
            update_and_draw_cursor();
            // Most nezzuk meg, hogy hova kattintott:
            if (is_in_box(Moux, Mouy, boxok)) {
                return;
            }
            if (is_in_box(Moux, Mouy, boxpoligon)) {
                Rajzolpoligon = !Rajzolpoligon;
                invalid = 1;
            }
            if (is_in_box(Moux, Mouy, boxkoveto)) {
                Rajzolkoveto = !Rajzolkoveto;
                invalid = 1;
            }
            if (is_in_box(Moux, Mouy, boxkepek)) {
                Rajzolkepek = !Rajzolkepek;
                invalid = 1;
            }
        }
        if (invalid) {
            invalid = 0;
            push();
            // Kirajzoljuk, mivel valtozott kep:
            render_box(BufferMain, x1, y1, x2, y2, DIALOG_PALETTE_ID, DIALOG_BORDER_PALETTE_ID);
            // Most jonnek gombok:
            // OK:
            render_box(BufferMain, boxok, BUTTON_PALETTE_ID, DIALOG_BORDER_PALETTE_ID);
            Pabc2->write_centered(BufferMain, (boxok.x1 + boxok.x2) / 2, boxok.y1 + 15, "OK");

            Pabc2->write(BufferMain, szovx, boxpoligon.y1 + 15, "View Polygons");
            render_box(BufferMain, boxpoligon, Feherszin, DIALOG_BORDER_PALETTE_ID);
            if (Rajzolpoligon) {
                boxbair(BufferMain, boxpoligon, Feherszin, "Yes");
            } else {
                boxbair(BufferMain, boxpoligon, Feherszin, "No");
            }

            Pabc2->write(BufferMain, szovx, boxkoveto.y1 + 15, "View Grass");
            render_box(BufferMain, boxkoveto, Feherszin, DIALOG_BORDER_PALETTE_ID);
            if (Rajzolkoveto) {
                boxbair(BufferMain, boxkoveto, Feherszin, "Yes");
            } else {
                boxbair(BufferMain, boxkoveto, Feherszin, "No");
            }

            Pabc2->write(BufferMain, szovx, boxkepek.y1 + 15, "View Pictures");
            render_box(BufferMain, boxkepek, Feherszin, DIALOG_BORDER_PALETTE_ID);
            if (Rajzolkepek) {
                boxbair(BufferMain, boxkepek, Feherszin, "Yes");
            } else {
                boxbair(BufferMain, boxkepek, Feherszin, "No");
            }

            bltfront(BufferMain, x1, y1, x2, y2);
            pop();
        }
        update_and_draw_cursor();
    }
}
