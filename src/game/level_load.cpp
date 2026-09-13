#include "game/level_load.h"
#include "debug/profiler.h"
#include "editor/dialog.h"
#include "editor/editor.h"
#include "level/level.h"
#include "level/segments.h"
#include "main.h"
#include "menu/dialog.h"
#include "physics/init.h"
#include "pic/lgr.h"
#include "platform/utils.h"
#include "renderer/canvas.h"
#include <cstring>

static bool ReloadLevel = false;
static finame CurrentLevelName = "";

void invalidate_level() { ReloadLevel = true; }

const char* current_level_filename() { return CurrentLevelName; }

// return true if Level is reloaded, false if Level is unchanged
static bool load_level(const char* levelname) {
    if (!levelname) {
        internal_error("load_level_play !levelname!");
    }
    size_t len = strlen(levelname);
    if (len > MAX_FILENAME_EXT_LEN || len <= 0) {
        internal_error(std::string("load_level_play levelname length invalid! ") + levelname);
    }

    if (!ReloadLevel && Level && strcmpi(levelname, CurrentLevelName) == 0) {
        return false;
    }

    ReloadLevel = false;
    strcpy(CurrentLevelName, levelname);

    delete Level;
    if (strcmpi(levelname, DEFAULT_LEVEL_FILENAME) == 0) {
        Level = new level;
        CurrentLevelName[0] = 0;
    } else {
        Level = new level(levelname);
    }

    return true;
}

LoadLevelResult load_level_play(const char* levelname) {
    if (!Segments) {
        invalidate_level();
    }
    if (load_level(levelname)) {
        if (Level->topology_errors) {
            delete Level;
            Level = nullptr;
            DikScancode key =
                menu_dialog("Level file has some topology errors!", "Use the editor to fix them!");
            return key == DIK_ESCAPE ? LoadLevelResult::Abort : LoadLevelResult::Fail;
        }

        lgrfile::load_lgr_file(Level->lgr_name, true);
        Level->load_sprite_wireframes(Lgr, false);

        START_TIME(segments_timer);
        delete Segments;
        Segments = new segments(Level);
        if (HeadRadius > Motor1->left_wheel.radius) {
            Segments->setup_collision_grid(HeadRadius);
        } else {
            Segments->setup_collision_grid(Motor1->left_wheel.radius);
        }
        END_TIME(segments_timer, std::format("{} Segments", levelname))

        canvas::create_canvases();
    }
    return LoadLevelResult::Success;
}

bool load_level_editor(const char* levelname) {
    if (load_level(levelname)) {
        lgrfile::load_lgr_file(Level->lgr_name, true);
        Level->load_sprite_wireframes(Lgr, true);
    }

    // Segments are not properly updated in the editor
    delete Segments;
    Segments = nullptr;

    if (Level->toptens.single.times_count > 0 || Level->toptens.multi.times_count > 0) {
        dialog("Warning!", "The level file you are opening has some best times.",
               "If you save this level file, these times will be erased!");
    }

    return true;
}
