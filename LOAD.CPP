#include "LOAD.H"
#include "ALL.H"
#include "ECSET.H"
#include "LGRFILE.H"
#include "menu_pic.h"
#include "physics_init.h"
#include "platform_utils.h"

int Volttopsave = 0;

static char Bentlevelnev[20] = "";

void invalidate_level() { Volttopsave = 1; }

// Beolvas egy level-t ha kell (nev max 12 karakter).
// Torli es kitolti Ptop, Plgr, Pszak, ecset mutatokat.
// Ha belso file-ban is megvan palya onnan veszi:
// Hamissal ter vissza, ha hibas topologiaju level:
int floadlevel_p(const char* nev) {
    if (!nev) {
        internal_error("floadlevel_p-ben !nev!");
    }
    if (strlen(nev) > 12 || !nev[0]) {
        internal_error("floadlevel_p-ben strlen( nev ) > 12 || !nev[0]!: ", nev);
    }
    if (Volttopsave || !Ptop || !Pszak || strcmpi(nev, Bentlevelnev) != 0) {
        Volttopsave = 0;
        // Ujra be kell olvasni palyat:
        strcpy(Bentlevelnev, nev);
        if (Ptop) {
            delete Ptop;
        }
        Ptop = new topol(nev);
        if (Ptop->hibas) {
            // Kiir hiba uzenetet:
            menu_pic szl;
            szl.add_line_centered("Level file has some topology errors!", 320, 190);
            szl.add_line_centered("Use the editor to fix them!", 320, 240);
            empty_keypress_buffer();
            while (1) {
                if (has_keypress()) {
                    int c = get_keypress();
                    if (c == KEY_ESC || c == KEY_ENTER) {
                        break;
                    }
                }
                szl.render();
            }

            delete Ptop;
            Ptop = NULL;
            return 0;
        }
        loadlgrfile(Ptop->lgrnev);
        Ptop->pic_selejtez(Plgr);

        if (Pszak) {
            delete Pszak;
        }
        Pszak = new szakaszok(Ptop);
        if (HeadRadius > Motor1->left_wheel.radius) {
            Pszak->rendez(HeadRadius);
        } else {
            Pszak->rendez(Motor1->left_wheel.radius);
        }

        betoltecseteket();
    }
    return 1;
}

// Torli es kitolti Ptop, Plgr, mutatokat.
// Pszak, ecset mutatokat torli:
// Ha belso file-ban is megvan palya onnan veszi:
// Igaz ha file nem volt lezarva, kulonben new palya hamissal:
int floadlevel_e(const char* nev, int forceload) {
    int levoltzarva = 0;

    if (!nev) {
        internal_error("floadlevel_e-ben !nev!");
    }
    if (strlen(nev) > 12 || !nev[0]) {
        internal_error("floadlevel_e-ben strlen( nev ) > 12 || !nev[0]!: ", nev);
    }
    if (forceload || Volttopsave || !Ptop || Ptop->lezart || strcmpi(nev, Bentlevelnev) != 0) {
        Volttopsave = 0;
        // Ujra be kell olvasni palyat:
        strcpy(Bentlevelnev, nev);
        if (Ptop) {
            delete Ptop;
        }
        if (strcmpi(nev, "_uj_topol_") == 0) {
            Ptop = new topol;
            Bentlevelnev[0] = 0;
        } else {
            Ptop = new topol(nev);
        }

        if (Ptop->lezart) {
            levoltzarva = 1;

            char szoveg[80];
            sprintf(szoveg, "The file '%s' is locked, you cannot edit it!", State->editor_filename);

            dialog(szoveg);
            delete Ptop;
            Ptop = new topol;
        }

        loadlgrfile(Ptop->lgrnev);
        if (Ptop->pic_selejtez(Plgr)) {
            dialogselejtez();
        }
    }

    if (Pszak) { // ervenytelenites
        delete Pszak;
    }
    Pszak = NULL;

    // Ha vannak benne idok, figyelmeztetes:
    if (Ptop->idok.single.times_count > 0 || Ptop->idok.multi.times_count > 0) {
        dialog("Warning!", "The level file you are opening has some best times.",
               "If you save this level file, these times will be erased!");
    }

    return !levoltzarva;
}

// Opciok minibike-ja hasznalja:
void invalidate_ptop(void) {
    if (Ptop) {
        delete Ptop;
        Ptop = 0;
    }
}

void dialogselejtez(void) {
    dialog("The LGR file has changed since the last edition of this level and",
           "some parts (pictures, textures or border polygons) of the level",
           "must have been deleted!", "", "!!!!IMPORTANT!!!!",
           "If you do not want to loose these parts, do not save this level",
           "on its original name!");
}
