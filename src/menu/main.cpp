#include "menu/main.h"
#include "menu/best_times.h"
#include "editor_dialog.h"
#include "EDITUJ.H"
#include "eol_settings.h"
#include "fs_utils.h"
#include "LEJATSZO.H"
#include "level.h"
#include "level_load.h"
#include "main.h"
#include "menu/intro.h"
#include "menu/nav.h"
#include "menu/options.h"
#include "menu/pic.h"
#include "menu/play.h"
#include "recorder.h"
#include "menu/replay.h"
#include "platform_impl.h"
#include "util/util.h"
#include <string>

static void menu_demo() {
    constexpr int DEMO_REPLAY_COUNT = 3;
    constexpr recname DEMO_NAMES[DEMO_REPLAY_COUNT] = {"demor1.rec", "demor2.rec", "demor3.rec"};

    int previous_demo = -1;

    while (true) {
        int demo = util::random::uint32() % DEMO_REPLAY_COUNT;
        while (demo == previous_demo) {
            demo = util::random::uint32() % DEMO_REPLAY_COUNT;
        }
        previous_demo = demo;

        loading_screen();
        int level_id = recorder::load_rec_file(DEMO_NAMES[demo], true);
        if (!level_file_exists(Rec1->level_filename)) {
            internal_error(std::string("menu_demo: cannot find level file for demo replay: ") +
                           Rec1->level_filename);
        }
        load_level_play(Rec1->level_filename);
        if (Ptop->level_id != level_id) {
            internal_error(
                std::string("menu_demo: level file changed since demo replay was made: ") +
                Rec1->level_filename);
        }
        Rec1->rewind();
        Rec2->rewind();
        if (lejatszo_r(Rec1->level_filename, 0)) {
            MenuPalette->set();
            return;
        }
        MenuPalette->set();
    }
}

static void menu_prompt_exit() {
    menu_nav nav("Do you want to quit?");
    nav.x_left = 300;
    nav.y_entries = 200;
    nav.dy = 40;
    nav.y_title = 50;
    nav.enable_esc = false;

    nav.add_row(
        "Yes", NAV_FUNC() {
            State->reload_toptens();

            State->save();
            State->write_stats();

            menu_exit();
        });

    nav.add_row("No");

    nav.navigate();
}

void menu_main() {
    MenuPalette->set();

    std::string prev_choice = "Play";
    while (true) {
        menu_nav nav("Main Menu");
        nav.x_left = 200;
        nav.y_entries = 100;
        nav.dy = 50;

        nav.add_row("Play", NAV_FUNC() { menu_play(); });
        nav.add_row("Replay", NAV_FUNC() { menu_replay_all(); });
        nav.add_row("Merge Replays", NAV_FUNC() { menu_merge_replays(); });
        if (EolSettings->show_demo_menu()) {
            nav.add_row("Demo", NAV_FUNC() { menu_demo(); });
        }
        nav.add_row("Options", NAV_FUNC() { menu_options(); });
        if (EolSettings->show_help_menu()) {
            nav.add_row("Help", NAV_FUNC() { menu_help(); });
        }
        if (EolSettings->show_best_times_menu()) {
            nav.add_row("Best Times", NAV_FUNC() { menu_best_times(); });
        }
        nav.add_row(
            "Editor", NAV_FUNC() {
                InEditor = true;
                hide_cursor();
                editor();
                show_cursor();
                InEditor = false;
                MenuPalette->set();
            });

        nav.select_row(prev_choice);

        int choice = nav.navigate();
        if (choice == -1) {
            menu_prompt_exit();
        } else {
            prev_choice = nav.entry_left(choice);
        }
    }
}
