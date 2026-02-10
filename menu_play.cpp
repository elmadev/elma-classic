#include "menu_play.h"
#include "abc8.h"
#include "best_times.h"
#include "EDITUJ.H"
#include "flagtag.h"
#include "keys.h"
#include "LEJATSZO.H"
#include "level.h"
#include "level_load.h"
#include "main.h"
#include "menu_external.h"
#include "menu_nav.h"
#include "menu_pic.h"
#include "platform_impl.h"
#include "platform_utils.h"
#include "physics_init.h"
#include "skip.h"
#include "timer.h"
#include <cstdlib>
#include <cstring>
#include <format>

// Prompt for replay filename and return true if enter, false if esc
static bool menu_prompt_replay_name(char* filename) {
    menu_pic menu;
    int i = 0;
    empty_keypress_buffer();
    bool rerender = true;
    filename[0] = 0;
    while (true) {
        while (has_keypress()) {
            Keycode c = get_keypress();
            if (c == KEY_ESC) {
                return false;
            }
            if (c == KEY_ENTER) {
                if (i > 0) {
                    return true;
                }
            }
            if (MenuFont->has_char(c) && is_char_valid_for_filename(c)) {
                if (i >= 8) {
                    continue;
                }
                filename[i] = (char)c;
                filename[i + 1] = 0;
                i++;
                rerender = true;
            }
            if (c == KEY_BACKSPACE) {
                if (i > 0) {
                    i--;
                    filename[i] = 0;
                    rerender = true;
                }
            }
        }
        if (rerender) {
            rerender = false;
            menu.clear();

            filename[i] = '_';
            filename[i + 1] = 0;
            menu.add_line_centered(filename, 320, 240);
            filename[i] = 0;

            menu.add_line_centered("Please enter the filename:", 320, 180);
        }
        menu.render();
    }
}

static void menu_save_play(int level_id) {
    char tmp[20] = "";
    if (!menu_prompt_replay_name(tmp)) {
        return;
    }
    strcat(tmp, ".rec");
    recorder::save_rec_file(tmp, level_id, State->flag_tag);
}

void update_top_ten(int time, char* time_message, int internal_index,
                    const char* external_filename) {
    // Check if internal or external
    bool external_level = external_filename ? true : false;

    time_message[0] = 0;

    // Dead
    if (time <= 0) {
        strcpy(time_message, MENU_CENTER_TEXT "You Failed to Finish!");
        if (MeghalteloszorAB == 1) {
            strcat(time_message, "    (A died first)");
        }
        if (MeghalteloszorAB == 2) {
            strcat(time_message, "    (B died first)");
        }
        return;
    }

    if (!external_level) {
        State->reload_toptens();
    }

    // Display finish time
    char tmp[10];
    centiseconds_to_string(time, tmp);
    if (Single) {
        sprintf(time_message, MENU_CENTER_TEXT "%s", tmp);
    } else {
        if (Aerintetteviragot) {
            sprintf(time_message, MENU_CENTER_TEXT "A:   %s", tmp);
        } else {
            sprintf(time_message, MENU_CENTER_TEXT "B:   %s", tmp);
        }
    }

    // Grab top ten table
    topten_set* tten_set;
    if (external_level) {
        tten_set = &Ptop->toptens;
    } else {
        tten_set = &State->toptens[internal_index];
    }
    topten* tten = &tten_set->single;
    if (!Single) {
        tten = &tten_set->multi;
    }

    // Not in top ten?
    if (tten->times_count == MAX_TIMES && tten->times[MAX_TIMES - 1] < time) {
        return;
    }

    // First and only time?
    if (tten->times_count == 0) {
        tten->times_count = 1;
        tten->times[0] = time;
        strcpy(tten->names1[0], State->player1);
        strcpy(tten->names2[0], State->player2);
        strcat(time_message, "     Best Time!");
        if (external_level) {
            Ptop->save_topten(external_filename);
        }
        return;
    }

    // Check Best Time, middle time, or last time
    bool not_last = tten->times[tten->times_count - 1] > time;
    if (tten->times[0] > time) {
        strcat(time_message, "     Best Time!");
    } else {
        if (not_last) {
            strcat(time_message, "     You Made the Top Ten");
        }
    }

    // Add your time to the end of the best time list
    if (tten->times_count == MAX_TIMES) {
        tten->times[MAX_TIMES - 1] = time;
        strcpy(tten->names1[MAX_TIMES - 1], State->player1);
        strcpy(tten->names2[MAX_TIMES - 1], State->player2);
    } else {
        tten->times[int(tten->times_count)] = time;
        strcpy(tten->names1[int(tten->times_count)], State->player1);
        strcpy(tten->names2[int(tten->times_count)], State->player2);
        tten->times_count++;
    }

    // Bubble sort your time to the right position
    for (int i = 0; i < MAX_TIMES + 1; i++) {
        for (int j = 0; j < tten->times_count - 1; j++) {
            if (tten->times[j] > tten->times[j + 1]) {
                int tmp = tten->times[j];
                tten->times[j] = tten->times[j + 1];
                tten->times[j + 1] = tmp;

                player_name tmp_name;
                strcpy(tmp_name, tten->names1[j]);
                strcpy(tten->names1[j], tten->names1[j + 1]);
                strcpy(tten->names1[j + 1], tmp_name);

                strcpy(tmp_name, tten->names2[j]);
                strcpy(tten->names2[j], tten->names2[j + 1]);
                strcpy(tten->names2[j + 1], tmp_name);
            }
        }
    }

    // Save external levels (state.dat is not saved)
    if (external_level) {
        Ptop->save_topten(external_filename);
    }
}

void replay_previous_run() {
    load_level_play(Rec1->level_filename);
    bool reset_player_visibility = true;
    while (true) {
        Rec1->rewind();
        Rec2->rewind();
        if (lejatszo_r(Rec1->level_filename, !reset_player_visibility)) {
            if (Ptop->objects_flipped) {
                internal_error("replay_previous_run flipped!");
            }
            return;
        }
        reset_player_visibility = false;
    }
}

void replay_from_file(const char* filename) {
    bool reset_play_visibility = true;
    while (true) {
        Rec1->rewind();
        Rec2->rewind();
        if (lejatszo_r(filename, !reset_play_visibility)) {
            if (Ptop->objects_flipped) {
                internal_error("replay_from_file flipped!");
            }
            return;
        }
        if (Ptop->objects_flipped) {
            internal_error("replay_from_file flipped!");
        }
        reset_play_visibility = false;
    }
}

static text_line ExtraTimeText[14];

MenuLevel menu_level(int internal_index, bool nav_on_play_next, const char* time_message,
                     const char* external_filename) {
    bool external_level = external_filename != nullptr;

    player* player1 = State->get_player(State->player1);
    player* player2 = State->get_player(State->player2);

    // Determine whether to show Play Next, Skip Level, or neither
    int show_play_next = 0;
    int show_skip_level = 0;
    int default_choice = 0;
    if (Single && !external_level) {
        if (player1->levels_completed > internal_index &&
            internal_index < INTERNAL_LEVEL_COUNT - 1) {
            show_play_next = 1;
        }
        if (!show_play_next && internal_index < INTERNAL_LEVEL_COUNT - 1) {
            show_skip_level = 1;
        }
        if (nav_on_play_next) {
            default_choice = 1;
        }
        if (nav_on_play_next && !show_play_next) {
            internal_error("nav_on_play_next && !show_play_next!");
        }
        if (show_skip_level && show_play_next) {
            internal_error("show_skip_level && show_play_next!");
        }
    }

    while (true) {
        // Title: Level 1: Warm Up or External: Unnamed
        char title[100] = "";
        if (external_level) {
            if (strlen(Ptop->level_name) > LEVEL_NAME_LENGTH) {
                internal_error("menu_level level_name too long!");
            }
            sprintf(title, "External: %s", Ptop->level_name);
            if (MenuFont->len(title) > 630) {
                sprintf(title, "Ext: %s", Ptop->level_name);
            }
        } else {
            strcpy(title, "Level ");
            char tmp[10];
            itoa(internal_index + 1, tmp, 10);
            strcat(title, tmp);
            strcat(title, ": ");
            strcat(title, get_internal_level_name(internal_index));
        }

        // Show either the time_message from update_top_ten or FlagTag info
        if (!Single && Tag) {
            if (FlagTagAStarts) {
                ExtraTimeText[0].text = MENU_CENTER_TEXT "A start with the flag next.";
            } else {
                ExtraTimeText[0].text = MENU_CENTER_TEXT "B start with the flag next.";
            }
        } else {
            ExtraTimeText[0].text = time_message;
        }
        ExtraTimeText[0].x = 320;
        if (Single) {
            ExtraTimeText[0].y = 370;
        } else {
            ExtraTimeText[0].y = 314;
        }
        int extra_time_text_length = 1;

        if (!Single) {
            // Show extra multiplayer information
            bool is_flagtag = Tag;

            int dx = 0;
            if (!is_flagtag) {
                dx = 100;
            }

            int y1 = 380;
            int y2 = 415;

            // Shorten info if player names are too long
            bool long_name =
                MenuFont->len(player1->name) > 160 || MenuFont->len(player2->name) > 160;

            // Player A name
            if (long_name) {
                ExtraTimeText[1].text = std::format("Player A: {}", player1->name);
            } else {
                ExtraTimeText[1].text = std::format("Player A:     {}", player1->name);
            }
            ExtraTimeText[1].x = 10 + dx;
            ExtraTimeText[1].y = y1;

            // Player A apple count
            ExtraTimeText[2].text = std::to_string(Motor1->apple_count);
            ExtraTimeText[2].x = 380 + dx;
            ExtraTimeText[2].y = y1;

            // Player A flag time
            if (is_flagtag) {
                int time = FlagTimeA * TimeToCentiseconds;
                char time_text[20];
                centiseconds_to_string(time, time_text);
                ExtraTimeText[3].text = std::string(time_text);
                ExtraTimeText[3].x = 440 + dx;
                ExtraTimeText[3].y = y1;
            } else {
                ExtraTimeText[3].text = " ";
                ExtraTimeText[3].x = 100 + dx;
                ExtraTimeText[3].y = 100;
            }

            // Player B name
            if (long_name) {
                ExtraTimeText[4].text = std::format("Player B: {}", player2->name);
            } else {
                ExtraTimeText[4].text = std::format("Player B:     {}", player2->name);
            }
            ExtraTimeText[4].x = 10 + dx;
            ExtraTimeText[4].y = y2;

            // Player B apple count
            ExtraTimeText[5].text = std::to_string(Motor2->apple_count);
            ExtraTimeText[5].x = 380 + dx;
            ExtraTimeText[5].y = y2;

            // Player B flag time
            if (is_flagtag) {
                int time = FlagTimeB * TimeToCentiseconds;
                char time_text[20];
                centiseconds_to_string(time, time_text);
                ExtraTimeText[6].text = std::string(time_text);
                ExtraTimeText[6].x = 440 + dx;
                ExtraTimeText[6].y = y2;
            }
            if (is_flagtag) {
                extra_time_text_length += 6;
            } else {
                extra_time_text_length += 5;
            }

            // Shift apple count and flag time right if names too long
            if (long_name) {
                ExtraTimeText[2].x += 40;
                ExtraTimeText[3].x += 40;
                ExtraTimeText[5].x += 40;
                ExtraTimeText[6].x += 40;
            }
        }

        menu_nav nav;
        nav.selected_index = default_choice;
        nav.x_left = 230;
        nav.y_entries = 110;
        nav.dy = 42;
        strcpy(nav.title, title);

        if (show_play_next || show_skip_level) {
            strcpy(NavEntriesLeft[0], "Play again");
            if (show_play_next) {
                strcpy(NavEntriesLeft[1], "Play next");
            } else {
                strcpy(NavEntriesLeft[1], "Skip level");
            }
            strcpy(NavEntriesLeft[2], "Replay");
            strcpy(NavEntriesLeft[3], "Save play");
            strcpy(NavEntriesLeft[4], "Best times");
        } else {
            strcpy(NavEntriesLeft[0], "Play again");
            strcpy(NavEntriesLeft[1], "Replay");
            strcpy(NavEntriesLeft[2], "Save play");
            strcpy(NavEntriesLeft[3], "Best times");
        }

        nav.setup(4 + show_play_next + show_skip_level);

        int choice = nav.navigate(ExtraTimeText, extra_time_text_length);
        default_choice = choice;

        if (choice > 0 && !show_play_next && !show_skip_level) {
            choice++;
        }
        if (choice < 0) {
            return MenuLevel::Esc;
        }
        if (choice == 0) {
            return MenuLevel::PlayAgain;
        }
        if (choice == 1) {
            // Play next / Skip level
            if (!show_play_next && !show_skip_level) {
                internal_error("choice == 1 && (!show_play_next && !show_skip_level)!");
            }
            if (show_play_next) {
                return MenuLevel::PlayNext;
            }
            if (is_skippable(internal_index)) {
                return MenuLevel::Skip;
            }
        }
        if (choice == 2) {
            // Replay
            replay_previous_run();
            MenuPalette->set();
        }
        if (choice == 3) {
            // Save play
            menu_save_play(Ptop->level_id);
        }
        if (choice == 4) {
            // Best times
            if (external_level) {
                menu_external_topten(Ptop, Single);
            } else {
                menu_internal_topten(internal_index, Single);
            }
        }
    }
}

void loading_screen() {
    menu_pic menu;
    menu.clear();
    menu.add_line_centered("Loading", 320, 230);
    menu.render(true);
}

static void play_internal(int internal_index) {
    player* cur_player = State->get_player(State->player1);
    while (true) {
        char filename[20];
        sprintf(filename, "QWQUU%03d.LEV", internal_index + 1);

        loading_screen();

        load_level_play(filename);
        Rec1->erase(filename);
        Rec2->erase(filename);

        int time = lejatszo(filename, F1Pressed ? CameraMode::MapViewer : CameraMode::Normal);

        MenuPalette->set();
        if (Ptop->objects_flipped) {
            internal_error("play_internal objects_flipped!");
        }

        char time_message[100] = "";
        update_top_ten(time, time_message, internal_index, nullptr);

        // Check to see if we unlock a new internal level (only available in singleplayer)
        int unlocked_new_level = 0;
        if (time > 0) {
            cur_player->skipped[internal_index] = 0;
            if (cur_player->levels_completed == internal_index && Single) {
                cur_player->levels_completed++;
                if (cur_player->levels_completed < INTERNAL_LEVEL_COUNT) {
                    unlocked_new_level = 1;
                }
            }
            // Update state.dat after every finish
            State->save();
        }

        MenuLevel choice = menu_level(internal_index, unlocked_new_level, time_message, nullptr);
        Rec1->erase(filename);
        Rec2->erase(filename);

        if (choice == MenuLevel::Esc) {
            cur_player->selected_level = internal_index + unlocked_new_level;
            return;
        }
        if (choice == MenuLevel::PlayAgain) {
            continue;
        }
        if (choice == MenuLevel::PlayNext) {
            internal_index++;
            cur_player->selected_level = internal_index;
        }
        if (choice == MenuLevel::Skip) {
            if (internal_index != cur_player->levels_completed) {
                internal_error("internal_index != cur_player->levels_completed!");
            }
            cur_player->skipped[internal_index] = 1;
            internal_index++;
            cur_player->selected_level = internal_index;
            cur_player->levels_completed++;
            State->reload_toptens();
            State->save();
        }
    }
}

void menu_play() {
    while (true) {
        // Get the total number of levels completed
        player* player1 = State->get_player(State->player1);
        int levels_completed = player1->levels_completed + 1;
        if (!State->single) {
            player* player2 = State->get_player(State->player2);
            if (levels_completed < player2->levels_completed + 1) {
                levels_completed = player2->levels_completed + 1;
            }
        }
        if (levels_completed > INTERNAL_LEVEL_COUNT) {
            levels_completed = INTERNAL_LEVEL_COUNT;
        }

        menu_nav nav;
        nav.search_pattern = SearchPattern::Internals;
        nav.selected_index = player1->selected_level + 1;
        strcpy(nav.title, "Select Level!");

        strcpy(NavEntriesLeft[0], "External File");
        for (int i = 0; i < levels_completed; i++) {
            char level_name[NAV_ENTRY_TEXT_MAX_LENGTH];
            itoa(i + 1, level_name, 10);
            strcat(level_name, " ");
            if (player1->skipped[i]) {
                strcat(level_name, "SKIPPED!");
            } else {
                strcat(level_name, get_internal_level_name(i));
            }
            if (strlen(level_name) > NAV_ENTRY_TEXT_MAX_LENGTH - 5) {
                internal_error("menu_play strlen( level_name ) > NAV_ENTRY_TEXT_MAX_LENGTH-5!");
            }
            strcpy(NavEntriesLeft[i + 1], level_name);
        }

        nav.setup(levels_completed + 1);

        int choice = nav.navigate();
        if (choice < 0) {
            return;
        }
        if (choice == 0) {
            player1->selected_level = -1;
            menu_external_levels();
        } else {
            play_internal(choice - 1);
        }
    }
}
