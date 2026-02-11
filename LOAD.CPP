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
#include "fs_utils.h"

static bool ReloadLevel = false;
static char CurrentLevelName[20] = "";

void invalidate_level() { ReloadLevel = true; }

bool load_level_play(const char* levelname) {
    if (!levelname) {
        internal_error("load_level_play !levelname!");
    }
    if (strlen(levelname) > MAX_FILENAME_EXT_LEN || !levelname[0]) {
        internal_error(
            "load_level_play strlen( levelname ) > MAX_FILENAME_EXT_LEN || !levelname[0]!: ",
            levelname);
    }
    if (ReloadLevel || !Ptop || !Segments || strcmpi(levelname, CurrentLevelName) != 0) {
        ReloadLevel = false;
        strcpy(CurrentLevelName, levelname);
        delete Ptop;
        Ptop = new level(levelname);
        if (Ptop->topology_errors) {
            menu_pic menu;
            menu.add_line_centered("Level file has some topology errors!", 320, 190);
            menu.add_line_centered("Use the editor to fix them!", 320, 240);
            empty_keypress_buffer();
            while (true) {
                if (has_keypress()) {
                    Keycode c = get_keypress();
                    if (c == KEY_ESC || c == KEY_ENTER) {
                        break;
                    }
                }
                menu.render();
            }

            delete Ptop;
            Ptop = nullptr;
            return false;
        }
        lgrfile::load_lgr_file(Ptop->lgr_name);
        Ptop->discard_missing_lgr_assets(Lgr);

        delete Segments;
        Segments = new segments(Ptop);
        if (HeadRadius > Motor1->left_wheel.radius) {
            Segments->setup_collision_grid(HeadRadius);
        } else {
            Segments->setup_collision_grid(Motor1->left_wheel.radius);
        }

        create_canvases();
    }
    return true;
}

bool load_level_editor(const char* levelname, int reload) {
    if (!levelname) {
        internal_error("load_level_editor !levelname!");
    }
    if (strlen(levelname) > MAX_FILENAME_EXT_LEN || !levelname[0]) {
        internal_error(
            "load_level_editor strlen( levelname ) > MAX_FILENAME_EXT_LEN || !levelname[0]!: ",
            levelname);
    }
    if (reload || ReloadLevel || !Ptop || strcmpi(levelname, CurrentLevelName) != 0) {
        ReloadLevel = false;
        strcpy(CurrentLevelName, levelname);
        delete Ptop;
        if (strcmpi(levelname, DEFAULT_LEVEL_FILENAME) == 0) {
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

    // Forces a reload in load_level_play
    delete Segments;
    Segments = nullptr;

    if (Ptop->toptens.single.times_count > 0 || Ptop->toptens.multi.times_count > 0) {
        dialog("Warning!", "The level file you are opening has some best times.",
               "If you save this level file, these times will be erased!");
    }

    return true;
}

void dialog_warn_lgr_assets_deleted() {
    dialog("The LGR file has changed since the last edition of this level and",
           "some parts (pictures, textures or border polygons) of the level",
           "must have been deleted!", "", "!!!!IMPORTANT!!!!",
           "If you do not want to lose these parts, do not save this level",
           "on its original name!");
}
