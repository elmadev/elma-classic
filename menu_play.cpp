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
#include "directinput/scancodes.h"
#include <cstdlib>
#include <cstring>
#include <directinput/scancodes.h>
#include <format>
#include <optional>

// Prompt for replay filename and return true if enter, false if esc
static bool menu_prompt_replay_name(char* filename) {
    menu_pic menu;
    int i = 0;
    empty_keypress_buffer();
    bool rerender = true;
    filename[0] = 0;
    while (true) {
        handle_events();
        if (was_key_just_pressed(DIK_ESCAPE)) {
            return false;
        }
        if (was_key_just_pressed(DIK_RETURN)) {
            if (i > 0) {
                return true;
            }
        }
        if (was_key_down(DIK_BACK)) {
            if (i > 0) {
                i--;
                filename[i] = 0;
                rerender = true;
            }
        }
        while (has_text_input()) {
            char c = pop_text_input();
            if (MenuFont->has_char(c) && is_char_valid_for_filename(c)) {
                if (i >= 8) {
                    continue;
                }
                filename[i] = c;
                filename[i + 1] = 0;
                i++;
                rerender = true;
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
    recorder::save_rec_file(tmp, level_id);
}

void update_top_ten(int time, char* time_message, int internal_index,
                    const char* external_filename) {
    // Check if internal or external
    bool external_level = external_filename ? true : false;

    time_message[0] = 0;

    // Dead
    if (time <= 0) {
        strcpy(time_message, "You Failed to Finish!");
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
        sprintf(time_message, "%s", tmp);
    } else {
        if (Aerintetteviragot) {
            sprintf(time_message, "A:   %s", tmp);
        } else {
            sprintf(time_message, "B:   %s", tmp);
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
        std::string title;
        if (external_level) {
            if (strlen(Ptop->level_name) > LEVEL_NAME_LENGTH) {
                internal_error("menu_level level_name too long!");
            }
            title = std::format("External: {}", Ptop->level_name);
            if (MenuFont->len(title.c_str()) > 630) {
                title = std::format("Ext: {}", Ptop->level_name);
            }
        } else {
            title = std::format("Level {}: {}", internal_index + 1,
                                get_internal_level_name(internal_index));
        }

        menu_nav nav(title);

        // Show either the time_message from update_top_ten or FlagTag info
        std::string overlay_text;
        if (!Single && Tag) {
            std::string letter = FlagTagAStarts ? "A" : "B";
            overlay_text = letter + " start with the flag next.";
        } else {
            overlay_text = std::string(time_message);
        }
        nav.add_overlay(overlay_text, 320, Single ? 370 : 314, OverlayAlignment::Centered);

        if (!Single) {
            // Show extra multiplayer information
            bool is_flagtag = Tag;

            int dx = 0;
            if (!is_flagtag) {
                dx = 100;
            }

            int y1 = 380;
            int y2 = 415;

            // Adjust horizontal spacing if player names are too long
            bool long_name =
                MenuFont->len(player1->name) > 160 || MenuFont->len(player2->name) > 160;
            std::string padding = long_name ? "" : "    ";

            // Name
            nav.add_overlay(std::format("Player A: {}{}", padding, player1->name), 10 + dx, y1);
            nav.add_overlay(std::format("Player B: {}{}", padding, player2->name), 10 + dx, y2);

            // Adjust horizontal spacing if player names are too long
            if (long_name) {
                dx += 40;
            }

            // Apple count
            nav.add_overlay(std::to_string(Motor1->apple_count), 380 + dx, y1);
            nav.add_overlay(std::to_string(Motor2->apple_count), 380 + dx, y2);

            // Flag time
            if (is_flagtag) {
                int time1 = FlagTimeA * TimeToCentiseconds;
                char time_text1[20];
                centiseconds_to_string(time1, time_text1);
                nav.add_overlay(std::string(time_text1), 440 + dx, y1);
                int time2 = FlagTimeB * TimeToCentiseconds;
                char time_text2[20];
                centiseconds_to_string(time2, time_text2);
                nav.add_overlay(std::string(time_text2), 440 + dx, y2);
            }
        }

        nav.x_left = 230;
        nav.y_entries = 110;
        nav.dy = 42;

        std::optional<MenuLevel> ret;

        nav.add_row("Play again", NAV_FUNC(&ret) { ret = MenuLevel::PlayAgain; });

        if (show_play_next) {
            nav.add_row("Play next", NAV_FUNC(&ret) { ret = MenuLevel::PlayNext; });
        }

        if (show_skip_level) {
            nav.add_row(
                "Skip level", NAV_FUNC(&ret, &internal_index) {
                    if (is_skippable(internal_index)) {
                        ret = MenuLevel::Skip;
                    }
                });
        }

        nav.add_row(
            "Replay", NAV_FUNC() {
                replay_previous_run();
                MenuPalette->set();
            });

        nav.add_row("Save play", NAV_FUNC() { menu_save_play(Ptop->level_id); });

        nav.add_row(
            "Best times", NAV_FUNC(&external_level, &internal_index) {
                if (external_level) {
                    menu_external_topten(Ptop, Single);
                } else {
                    menu_internal_topten(internal_index, Single);
                }
            });

        nav.select_row(default_choice);

        int choice = nav.navigate();

        if (choice < 0) {
            return MenuLevel::Esc;
        }
        if (ret) {
            return *ret;
        }
    }
}

void loading_screen() {
    menu_pic menu;
    menu.clear();
    menu.add_line_centered("Loading", 320, 230);
    menu.render(true);
}

static void play_internal(int internal_index, bool map_viewer) {
    player* cur_player = State->get_player(State->player1);
    while (true) {
        char filename[20];
        sprintf(filename, "QWQUU%03d.LEV", internal_index + 1);

        loading_screen();

        load_level_play(filename);
        Rec1->erase(filename);
        Rec2->erase(filename);

        int time = lejatszo(filename, map_viewer ? CameraMode::MapViewer : CameraMode::Normal);

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

        menu_nav nav("Select Level!");
        nav.search_pattern = SearchPattern::Internals;
        nav.select_row(player1->selected_level + 1);

        nav.add_row(
            "External File", NAV_FUNC(&player1) {
                player1->selected_level = -1;
                menu_external_levels();
            });

        for (int i = 0; i < levels_completed; i++) {
            std::string level_name = std::format(
                "{} {}", i + 1, player1->skipped[i] ? "SKIPPED!" : get_internal_level_name(i));
            nav.add_row(level_name, NAV_FUNC() { play_internal(choice - 1, is_key_down(DIK_F1)); });
        }

        int choice = nav.navigate();
        if (choice < 0) {
            return;
        }
    }
}
