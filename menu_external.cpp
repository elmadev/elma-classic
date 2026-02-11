#include "menu_external.h"
#include "menu_nav.h"
#include "fs_utils.h"
#include "LEJATSZO.H"
#include "level_load.h"
#include "main.h"
#include "menu_nav.h"
#include "menu_pic.h"
#include "menu_play.h"
#include "platform_impl.h"
#include "state.h"
#include "directinput/scancodes.h"
#include <cstring>
#include <string>

static void play_external(const std::string& filename, bool map_viewer) {
    if (filename.length() > MAX_FILENAME_LEN + 4) {
        internal_error("menu_external_levels() entry too long!");
    }
    const char* filename_str = filename.c_str();
    strcpy(State->external_filename, filename_str);

    while (true) {
        loading_screen();
        if (!load_level_play(filename_str)) {
            return;
        }
        Rec1->erase(filename_str);
        Rec2->erase(filename_str);
        int time = lejatszo(filename_str, map_viewer ? CameraMode::MapViewer : CameraMode::Normal);
        MenuPalette->set();
        char finish_msg[100] = "";
        update_top_ten(time, finish_msg, 0, filename_str);
        if (menu_level(0, 0, finish_msg, filename_str) == MenuLevel::Esc) {
            Rec1->erase(filename_str);
            Rec2->erase(filename_str);
            return;
        }
    }
}

void menu_external_levels() {
    menu_nav nav("Select External File!");
    nav.search_pattern = SearchPattern::Sorted;

    finame filename;
    bool done = find_first("lev/*.lev", filename);
    while (!done) {
        nav.add_row(filename, NAV_FUNC() { play_external(left, is_key_down(DIK_F1)); });
        done = find_next(filename);
    }
    find_close();

    if (nav.row_count() <= 0) {
        return;
    }

    nav.sort_rows();
    nav.select_row(State->external_filename);

    while (true) {
        int choice = nav.navigate();
        if (choice < 0) {
            return;
        }
    }
}
