#include "LOAD.H"
#include "ECSET.H"
#include "editor_dialog.h"
#include "EDITUJ.H"
#include "keys.h"
#include "level.h"
#include "lgr.h"
#include "main.h"
#include "menu_pic.h"
#include "physics_init.h"
#include "platform_utils.h"
#include "segments.h"
#include <cstring>

int ReloadLevel = 0;

static char CurrentLevelName[20] = "";

void invalidate_level() { ReloadLevel = 1; }

// Beolvas egy level-t ha kell (nev max 12 karakter).
// Torli es kitolti Ptop, Lgr, Segments, canvas mutatokat.
// Ha belso file-ban is megvan palya onnan veszi:
// Hamissal ter vissza, ha hibas topologiaju level:
int load_level_play(const char* levelname) {
    if (!levelname) {
        internal_error("load_level_play-ben !levelname!");
    }
    if (strlen(levelname) > 12 || !levelname[0]) {
        internal_error("load_level_play-ben strlen( levelname ) > 12 || !levelname[0]!: ",
                       levelname);
    }
    if (ReloadLevel || !Ptop || !Segments || strcmpi(levelname, CurrentLevelName) != 0) {
        ReloadLevel = 0;
        // Ujra be kell olvasni palyat:
        strcpy(CurrentLevelName, levelname);
        if (Ptop) {
            delete Ptop;
        }
        Ptop = new level(levelname);
        if (Ptop->topology_errors) {
            // Kiir hiba uzenetet:
            menu_pic menu;
            menu.add_line_centered("Level file has some topology errors!", 320, 190);
            menu.add_line_centered("Use the editor to fix them!", 320, 240);
            empty_keypress_buffer();
            while (1) {
                if (has_keypress()) {
                    Keycode c = get_keypress();
                    if (c == KEY_ESC || c == KEY_ENTER) {
                        break;
                    }
                }
                menu.render();
            }

            delete Ptop;
            Ptop = NULL;
            return 0;
        }
        lgrfile::load_lgr_file(Ptop->lgr_name);
        Ptop->discard_missing_lgr_assets(Lgr);

        if (Segments) {
            delete Segments;
        }
        Segments = new segments(Ptop);
        if (HeadRadius > Motor1->left_wheel.radius) {
            Segments->setup_collision_grid(HeadRadius);
        } else {
            Segments->setup_collision_grid(Motor1->left_wheel.radius);
        }

        create_canvases();
    }
    return 1;
}

// Torli es kitolti Ptop, Lgr, mutatokat.
// Segments, canvas mutatokat torli:
// Ha belso file-ban is megvan palya onnan veszi:
// Igaz ha file nem volt lezarva, kulonben new palya hamissal:
int load_level_editor(const char* levelname, int reload) {
    int locked = 0;

    if (!levelname) {
        internal_error("load_level_editor-ben !levelname!");
    }
    if (strlen(levelname) > 12 || !levelname[0]) {
        internal_error("load_level_editor-ben strlen( levelname ) > 12 || !levelname[0]!: ",
                       levelname);
    }
    if (reload || ReloadLevel || !Ptop || strcmpi(levelname, CurrentLevelName) != 0) {
        ReloadLevel = 0;
        // Ujra be kell olvasni palyat:
        strcpy(CurrentLevelName, levelname);
        if (Ptop) {
            delete Ptop;
        }
        if (strcmpi(levelname, "_uj_topol_") == 0) {
            Ptop = new level;
            CurrentLevelName[0] = 0;
        } else {
            Ptop = new level(levelname);
        }

        lgrfile::load_lgr_file(Ptop->lgr_name);
        if (Ptop->discard_missing_lgr_assets(Lgr)) {
            dialog_warn_lgr_assets_deleted();
        }
    }

    if (Segments) { // ervenytelenites
        delete Segments;
    }
    Segments = NULL;

    // Ha vannak benne idok, figyelmeztetes:
    if (Ptop->toptens.single.times_count > 0 || Ptop->toptens.multi.times_count > 0) {
        dialog("Warning!", "The level file you are opening has some best times.",
               "If you save this level file, these times will be erased!");
    }

    return !locked;
}

// Opciok minibike-ja hasznalja:
void invalidate_ptop(void) {
    if (Ptop) {
        delete Ptop;
        Ptop = 0;
    }
}

void dialog_warn_lgr_assets_deleted(void) {
    dialog("The LGR file has changed since the last edition of this level and",
           "some parts (pictures, textures or border polygons) of the level",
           "must have been deleted!", "", "!!!!IMPORTANT!!!!",
           "If you do not want to loose these parts, do not save this level",
           "on its original name!");
}
