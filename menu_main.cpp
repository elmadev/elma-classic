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
#include <algorithm>
#include <cstdlib>
#include <cstring>
#include <ctime>

// Returns true if text1 is earlier in the alphabet than text2
int case_insensitive_lexicographical_compare(const char* text1, const char* text2) {
    // If they differ only by case, we track which one had the precedence
    int earlier_case1 = 0;
    int earlier_case2 = 0;
    while (true) {
        char char1 = *text1;
        char char2 = *text2;

        if (!char1 && !char2) {
            // Letters matched, maybe case difference decides:
            if (earlier_case2) {
                return 0;
            }
            return 1;
        }

        if (!char1) {
            return 1;
        }
        if (!char2) {
            return 0;
        }

        if (char1 >= 'a') {
            char1 -= 'a' - 'A';
        }
        if (char2 >= 'a') {
            char2 -= 'a' - 'A';
        }
        if (char1 == char2) {
            if (!earlier_case1 && !earlier_case2) {
                if (*text1 < *text2) {
                    earlier_case1 = 1;
                }
                if (*text2 < *text1) {
                    earlier_case2 = 1;
                }
            }
        } else {
            if (char1 < char2) {
                return 1;
            }
            if (char2 < char1) {
                return 0;
            }
        }

        text1++;
        text2++;
    }
}

static int SrandNeeded = 1;

static unsigned int gen_rand_int() {
    if (SrandNeeded) {
        SrandNeeded = 0;
        srand((unsigned)clock());
    }

    unsigned int result = 0;
    for (int i = 0; i < 4; i++) {
        result = (result << 8) | (rand() & 0xFF);
    }
    return result;
}

void menu_replay() {
    strcpy(NavEntriesLeft[0], "Randomizer");

    finame filename;
    bool done = find_first("rec/*.rec", filename);
    // This is always one more than reality, since the first entry is always 'Randomizer':
    int count = 1;
    while (!done && count < NavEntriesLeftMaxLength) {
        char* name = NavEntriesLeft[count];
        strcpy(name, filename);
        // Remove extension:
        int i = strlen(name) - 1;
        while (i >= 0 && name[i] != '.') {
            i--;
        }
        if (i < 0) {
            internal_error("menu_replay: no dot in name!: ", name);
        }
        name[i] = 0;

        done = find_next(filename);
        count++;
        if (count >= NavEntriesLeftMaxLength - 4) {
            done = true;
        }
    }
    find_close();

    if (count < 2) {
        return;
    }

    // Sort alphabetically:
    for (int i = 0; i < count + 1; i++) {
        for (int j = 1; j < count - 1; j++) {
            if (case_insensitive_lexicographical_compare(NavEntriesLeft[j + 1],
                                                         NavEntriesLeft[j])) {
                // Swap:
                char tmp[100];
                strcpy(tmp, NavEntriesLeft[j]);
                strcpy(NavEntriesLeft[j], NavEntriesLeft[j + 1]);
                strcpy(NavEntriesLeft[j + 1], tmp);
            }
        }
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
                // Generate name:
                char tmp[30];
                strcpy(tmp, NavEntriesLeft[index + 1]);
                strcat(tmp, ".REC");
                MenuPalette->set();
                loading_screen();
                int level_id = recorder::load_rec_file(tmp, 0);
                if (access_level_file(Rec1->level_filename) != 0) {
                    int c = menu_dialog("Cannot find the lev file that corresponds",
                                        "to the record file!", tmp, Rec1->level_filename);
                    if (c == KEY_ESC) {
                        return;
                    }
                } else {
                    floadlevel_p(Rec1->level_filename);
                    if (Ptop->level_id != level_id) {
                        int c =
                            menu_dialog("The level file has changed since the",
                                        "saving of the record file!", tmp, Rec1->level_filename);
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
            // Play a rec file:
            char tmp[30];
            strcpy(tmp, NavEntriesLeft[choice]);
            strcat(tmp, ".REC");
            loading_screen();
            int level_id = recorder::load_rec_file(tmp, 0);
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
                                    "to the record file!", tmp, Rec1->level_filename);
                    } else {
                        floadlevel_p(Rec1->level_filename);
                        if (Ptop->level_id != level_id) {
                            menu_dialog("The level file has changed since the",
                                        "saving of the record file!", tmp, Rec1->level_filename);
                        } else {
                            Rec1->rewind();
                            Rec2->rewind();
                            render_replay(Rec1->level_filename, tmp);
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
                menu_dialog(tmp, "The time of this replay file is:", time_str);
                continue;
            }
            if (access_level_file(Rec1->level_filename) != 0) {
                menu_dialog("Cannot find the lev file that corresponds", "to the record file!", tmp,
                            Rec1->level_filename);
            } else {
                floadlevel_p(Rec1->level_filename);
                if (Ptop->level_id != level_id) {
                    menu_dialog("The level file has changed since the",
                                "saving of the record file!", tmp, Rec1->level_filename);
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
