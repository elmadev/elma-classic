#include "menu_main.h"
#include "best_times.h"
#include "editor_dialog.h"
#include "EDITUJ.H"
#include "eol_settings.h"
#include "fs_utils.h"
#include "keys.h"
#include "LEJATSZO.H"
#include "level.h"
#include "LOAD.H"
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
#include <string>
#include <vector>

static unsigned int gen_rand_int() {
    unsigned int result = 0;
    for (int i = 0; i < 4; i++) {
        result = (result << 8) | (rand() & 0xFF);
    }
    return result;
}

void menu_replay() {
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

    menu_nav nav;
    nav.search_pattern = SearchPattern::Sorted;
    nav.search_skip_one = true;
    nav.selected_index = 0;
    strcpy(nav.title, "Select replay file!");

    nav.setup(count);

    while (true) {
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

                MenuPalette->set();
                loading_screen();

                int level_id = recorder::load_rec_file(replay_names[index].c_str(), 0);
                if (access_level_file(Rec1->level_filename) != 0) {
                    int c = menu_dialog("Cannot find the lev file that corresponds",
                                        "to the record file!", replay_names[index].c_str(),
                                        Rec1->level_filename);
                    if (c == KEY_ESC) {
                        return;
                    }
                } else {
                    floadlevel_p(Rec1->level_filename);
                    if (Ptop->level_id != level_id) {
                        int c = menu_dialog("The level file has changed since the",
                                            "saving of the record file!",
                                            replay_names[index].c_str(), Rec1->level_filename);
                        if (c == KEY_ESC) {
                            return;
                        }
                    } else {
                        Rec1->rewind();
                        Rec2->rewind();
                        if (lejatszo_r(Rec1->level_filename, 0)) {
                            break;
                        }
                    }
                }
            }
        } else {
            const char* replay_name = replay_names[choice - 1].c_str();
            // Play a rec file:
            loading_screen();
            int level_id = recorder::load_rec_file(replay_name, 0);
            if (F1Pressed) {
                F1Pressed = false;
                char msg[128];
                snprintf(msg, sizeof(msg), "Recording at %d FPS to renders/%s",
                         EolSettings->recording_fps(), NavEntriesLeft[choice]);
                int c = menu_dialog("Render replay to video frames?", msg,
                                    "Press Enter to continue, ESC to cancel");
                if (c == KEY_ENTER) {
                    if (access_level_file(Rec1->level_filename) != 0) {
                        menu_dialog("Cannot find the lev file that corresponds",
                                    "to the record file!", replay_name, Rec1->level_filename);
                    } else {
                        floadlevel_p(Rec1->level_filename);
                        if (Ptop->level_id != level_id) {
                            menu_dialog("The level file has changed since the",
                                        "saving of the record file!", replay_name,
                                        Rec1->level_filename);
                        } else {
                            Rec1->rewind();
                            Rec2->rewind();
                            render_replay(Rec1->level_filename, replay_name);
                        }
                    }
                }
                MenuPalette->set();
                continue;
            }

            if (CtrlAltPressed) {
                int time = Rec1->frame_count;
                if (MultiplayerRec && Rec2->frame_count > time) {
                    time = Rec2->frame_count;
                }

                time = (int)(time * 3.3333333333333);
                time -= 2;
                time = std::max(time, 1);

                char time_str[25];
                centiseconds_to_string(time, time_str);
                strcat(time_str, "    +- 0.01 sec");
                menu_dialog(replay_name, "The time of this replay file is:", time_str);
                continue;
            }
            if (access_level_file(Rec1->level_filename) != 0) {
                menu_dialog("Cannot find the lev file that corresponds", "to the record file!",
                            replay_name, Rec1->level_filename);
            } else {
                floadlevel_p(Rec1->level_filename);
                if (Ptop->level_id != level_id) {
                    menu_dialog("The level file has changed since the",
                                "saving of the record file!", replay_name, Rec1->level_filename);
                } else {
                    replay_from_file(Rec1->level_filename);
                }
            }
        }
        MenuPalette->set();
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
        int level_id = recorder::load_rec_file(demo_names[demo], 1);
        if (access_level_file(Rec1->level_filename) != 0) {
            internal_error("menu_demo: cannot find level file for demo replay: ",
                           Rec1->level_filename);
        }
        floadlevel_p(Rec1->level_filename);
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
    menu_nav nav;
    nav.selected_index = 0;
    nav.x_left = 300;
    nav.y_entries = 200;
    nav.dy = 40;
    nav.y_title = 50;
    nav.enable_esc = false;
    strcpy(nav.title, "Do you want to quit?");

    strcpy(NavEntriesLeft[0], "Yes");
    strcpy(NavEntriesLeft[1], "No");

    nav.setup(2);

    int choice = nav.navigate();

    if (choice == 0) {
        State->reload_toptens();

        State->save();
        State->write_stats();
        menu_exit();
    }
}

void menu_main() {
    MenuPalette->set();

    menu_nav nav;
    nav.selected_index = 0;
    nav.x_left = 200;
    nav.y_entries = 100;
    nav.dy = 50;
    strcpy(nav.title, "Main Menu");

    strcpy(NavEntriesLeft[0], "Play");
    strcpy(NavEntriesLeft[1], "Replay");
    strcpy(NavEntriesLeft[2], "Demo");
    strcpy(NavEntriesLeft[3], "Options");
    strcpy(NavEntriesLeft[4], "Help");
    strcpy(NavEntriesLeft[5], "Best Times");
    strcpy(NavEntriesLeft[6], "Editor");

    nav.setup(7);

    while (true) {
        int choice = nav.navigate();

        if (choice == 0) {
            menu_play();
        }

        if (choice == 1) {
            menu_replay();
        }

        if (choice == 2) {
            menu_demo();
        }

        if (choice == 3) {
            menu_options();
        }

        if (choice == 4) {
            menu_help();
        }

        if (choice == 5) {
            menu_best_times();
        }

        if (choice == 6) {
            InEditor = true;
            hide_cursor();
            editor();
            show_cursor();
            InEditor = false;
            MenuPalette->set();
        }

        if (choice == -1) {
            menu_prompt_exit();
        }
    }
}
