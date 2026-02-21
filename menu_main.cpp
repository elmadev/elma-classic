#include "menu_main.h"
#include "best_times.h"
#include "editor_dialog.h"
#include "EDITUJ.H"
#include "eol_settings.h"
#include "fs_utils.h"
#include "LEJATSZO.H"
#include "level.h"
#include "level_load.h"
#include "main.h"
#include "menu_dialog.h"
#include "menu_intro.h"
#include "menu_nav.h"
#include "menu_options.h"
#include "menu_pic.h"
#include "menu_play.h"
#include "platform_impl.h"
#include "platform_utils.h"
#include <algorithm>
#include <cstdlib>
#include <cstring>
#include <format>
#include <string>
#include <vector>

static unsigned int gen_rand_int() {
    unsigned int result = 0;
    for (int i = 0; i < 4; i++) {
        result = (result << 8) | (rand() & 0xFF);
    }
    return result;
}

static void replay_time(const std::string& filename) {
    MenuPalette->set();
    loading_screen();

    recorder::load_rec_file(filename.c_str(), false);

    int time = Rec1->frame_count();
    if (MultiplayerRec && Rec2->frame_count() > time) {
        time = Rec2->frame_count();
    }

    time = (int)(time * 3.3333333333333);
    time -= 2;
    time = std::max(time, 1);

    char time_str[25];
    centiseconds_to_string(time, time_str);
    strcat(time_str, "    +- 0.01 sec");
    menu_dialog(filename.c_str(), "The time of this replay file is:", time_str);
}

enum class LoadReplayResult { Success, Fail, Abort };

static LoadReplayResult load_replay(const std::string& filename) {
    MenuPalette->set();
    loading_screen();

    int level_id = recorder::load_rec_file(filename.c_str(), false);

    if (access_level_file(Rec1->level_filename) != 0) {
        DikScancode key =
            menu_dialog("Cannot find the lev file that corresponds", "to the record file!",
                        filename.c_str(), Rec1->level_filename);
        return key == DIK_ESCAPE ? LoadReplayResult::Abort : LoadReplayResult::Fail;
    }
    load_level_play(Rec1->level_filename);

    if (Ptop->level_id != level_id) {
        DikScancode key =
            menu_dialog("The level file has changed since the", "saving of the record file!",
                        filename.c_str(), Rec1->level_filename);
        return key == DIK_ESCAPE ? LoadReplayResult::Abort : LoadReplayResult::Fail;
    }

    return LoadReplayResult::Success;
}

static void replay_play(const std::string& filename) {
    if (load_replay(filename) == LoadReplayResult::Success) {
        replay_from_file(Rec1->level_filename);
    }
}

static void replay_render(const std::string& filename) {
    setup_render_directory(filename);
    std::string msg = std::format("Recording at {} FPS to {}", EolSettings->recording_fps(),
                                  VideoOutputDirectory);
    DikScancode c = menu_dialog("Render replay to video frames?", msg.c_str(),
                                "Press Enter to continue, ESC to cancel");
    if (c == DIK_RETURN) {
        if (load_replay(filename) == LoadReplayResult::Success) {
            Rec1->rewind();
            Rec2->rewind();
            render_replay(Rec1->level_filename);
        }
    }
}

static void replay_randomizer(std::vector<std::string>& filenames) {
    int count = filenames.end() - filenames.begin();
    int last_played = -1;
    int second_last_played = -1;
    while (true) {
        int index = gen_rand_int() % count;
        while ((index == last_played && count > 1) || (index == second_last_played && count > 2)) {
            index = gen_rand_int() % count;
        }
        second_last_played = last_played;
        last_played = index;

        LoadReplayResult loaded = load_replay(filenames[index]);
        if (loaded == LoadReplayResult::Success) {
            Rec1->rewind();
            Rec2->rewind();
            if (lejatszo_r(Rec1->level_filename, 0)) {
                return;
            }
        } else if (loaded == LoadReplayResult::Abort) {
            return;
        }
    }
}

static void menu_replay() {
    std::vector<std::string> replay_names;

    menu_nav nav("Select replay file!");
    nav.add_row("Randomizer", NAV_FUNC(&replay_names) { replay_randomizer(replay_names); });

    finame filename;
    bool done = find_first("rec/*.rec", filename);
    while (!done) {
        replay_names.emplace_back(filename);

        constexpr int EXT_LEN = 4;
        int len = strlen(filename);
        std::string short_name = std::string(filename, len - EXT_LEN);
        nav.add_row(
            short_name, NAV_FUNC(filename) {
                if (is_key_down(DIK_F1)) {
                    replay_render(filename);
                } else if (is_key_down(DIK_LCONTROL) && is_key_down(DIK_LMENU)) {
                    replay_time(filename);
                } else {
                    replay_play(filename);
                }
            });

        done = find_next(filename);
    }
    find_close();

    nav.search_pattern = SearchPattern::Sorted;
    nav.search_skip = 1;
    nav.sort_rows();

    if (nav.row_count() <= 1) {
        return;
    }

    while (true) {
        MenuPalette->set();
        int choice = nav.navigate();
        if (choice < 0) {
            return;
        }
    }
}

static void menu_demo() {
    constexpr int DEMO_REPLAY_COUNT = 3;
    char demo_names[DEMO_REPLAY_COUNT][MAX_FILENAME_LEN + 4] = {"demor1.rec", "demor2.rec",
                                                                "demor3.rec"};

    int previous_demo = -1;

    while (true) {
        int demo = gen_rand_int() % DEMO_REPLAY_COUNT;
        while (demo == previous_demo) {
            demo = gen_rand_int() % DEMO_REPLAY_COUNT;
        }
        previous_demo = demo;

        loading_screen();
        int level_id = recorder::load_rec_file(demo_names[demo], true);
        if (access_level_file(Rec1->level_filename) != 0) {
            internal_error("menu_demo: cannot find level file for demo replay: ",
                           Rec1->level_filename);
        }
        load_level_play(Rec1->level_filename);
        if (Ptop->level_id != level_id) {
            internal_error("menu_demo: level file changed since demo replay was made: ",
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

    menu_nav nav("Main Menu");
    nav.x_left = 200;
    nav.y_entries = 100;
    nav.dy = 50;

    nav.add_row("Play", NAV_FUNC() { menu_play(); });
    nav.add_row("Replay", NAV_FUNC() { menu_replay(); });
    nav.add_row("Demo", NAV_FUNC() { menu_demo(); });
    nav.add_row("Options", NAV_FUNC() { menu_options(); });
    nav.add_row("Help", NAV_FUNC() { menu_help(); });
    nav.add_row("Best Times", NAV_FUNC() { menu_best_times(); });
    nav.add_row(
        "Editor", NAV_FUNC() {
            InEditor = true;
            hide_cursor();
            editor();
            show_cursor();
            InEditor = false;
            MenuPalette->set();
        });

    while (true) {
        int choice = nav.navigate();
        if (choice == -1) {
            menu_prompt_exit();
        }
    }
}
