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
        return key == KEY_ESC ? LoadReplayResult::Abort : LoadReplayResult::Fail;
    }
    load_level_play(Rec1->level_filename);

    if (Ptop->level_id != level_id) {
        DikScancode key =
            menu_dialog("The level file has changed since the", "saving of the record file!",
                        filename.c_str(), Rec1->level_filename);
        return key == KEY_ESC ? LoadReplayResult::Abort : LoadReplayResult::Fail;
    }

    return LoadReplayResult::Success;
}

static void menu_replay() {
    finame filename;
    std::vector<std::string> replay_names;
    bool done = find_first("rec/*.rec", filename);

    while (!done) {
        replay_names.emplace_back(filename);

        done = find_next(filename);
        if (replay_names.size() >= NavEntriesLeftMaxLength - 4) {
            done = true;
        }
    }
    find_close();

    if (replay_names.empty()) {
        return;
    }

    std::sort(replay_names.begin(), replay_names.end(),
              [](const std::string& a, const std::string& b) {
                  return strcmpi(a.c_str(), b.c_str()) < 0;
              });

    int count = 0;
    strcpy(NavEntriesLeft[count++], "Randomizer");

    for (const auto& full_name : replay_names) {
        finame display_name;
        strcpy(display_name, full_name.c_str());

        // Remove extension for display:
        int i = strlen(display_name) - 1;
        while (i >= 0 && display_name[i] != '.') {
            i--;
        }
        if (i < 0) {
            internal_error("menu_replay: no dot in name!: ", full_name.c_str());
        }
        display_name[i] = 0;

        strcpy(NavEntriesLeft[count++], display_name);
    }

    menu_nav_old nav;
    nav.search_pattern = SearchPattern::Sorted;
    nav.search_skip_one = true;
    nav.selected_index = 0;
    strcpy(nav.title, "Select replay file!");

    nav.setup(count);

    while (true) {
        MenuPalette->set();
        int choice = nav.navigate();

        if (choice < 0) {
            return;
        }

        if (choice == 0) {
            // Randomizer
            int last_played = -1;
            int second_last_played = -1;
            while (true) {
                int index = gen_rand_int() % (count - 1);
                while ((index == last_played && count > 2) ||
                       (index == second_last_played && count > 3)) {
                    index = gen_rand_int() % (count - 1);
                }
                second_last_played = last_played;
                last_played = index;

                LoadReplayResult loaded = load_replay(replay_names[index]);
                if (loaded == LoadReplayResult::Success) {
                    Rec1->rewind();
                    Rec2->rewind();
                    if (lejatszo_r(Rec1->level_filename, 0)) {
                        break;
                    }
                } else if (loaded == LoadReplayResult::Abort) {
                    break;
                }
            }
        } else {
            const std::string& replay_name = replay_names[choice - 1];
            // Play a rec file:
            if (F1Pressed) {
                F1Pressed = false;
                setup_render_directory(replay_name);
                std::string msg = std::format("Recording at {} FPS to {}",
                                              EolSettings->recording_fps(), VideoOutputDirectory);
                DikScancode c = menu_dialog("Render replay to video frames?", msg.c_str(),
                                            "Press Enter to continue, ESC to cancel");
                if (c == DIK_RETURN) {
                    if (load_replay(replay_name) == LoadReplayResult::Success) {
                        Rec1->rewind();
                        Rec2->rewind();
                        render_replay(Rec1->level_filename, replay_name.c_str());
                    }
                }
                continue;
            } else if (CtrlAltPressed) {
                replay_time(replay_name);
            } else {
                if (load_replay(replay_name) == LoadReplayResult::Success) {
                    replay_from_file(Rec1->level_filename);
                }
            }
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
