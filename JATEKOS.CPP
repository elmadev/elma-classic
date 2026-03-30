#include "JATEKOS.H"
#include "abc8.h"
#include "keys.h"
#include "main.h"
#include "menu_intro.h"
#include "menu_main.h"
#include "menu_nav.h"
#include "menu_pic.h"
#include "platform_impl.h"
#include "state.h"
#include "util/util.h"
#include <directinput/scancodes.h>
#include <cstring>
#include <directinput/scancodes.h>

static int MAX_PLAYERNAME_INPUT = 8;

// Prompt for a new player name. Return true if name successfully selected
static int prompt_new_name(char* player_name, int should_return) {
    menu_pic menu(false);

    int i = 0;
    empty_keypress_buffer();
    int needs_update = 1;
    player_name[0] = 0;
    while (1) {
        handle_events();
        if (was_key_just_pressed(DIK_ESCAPE)) {
            if (should_return) {
                return 0;
            }
            menu_exit();
        }
        if (was_key_just_pressed(DIK_RETURN)) {
            if (i > 0) {
                return 1;
            }
        }
        if (was_key_down(DIK_BACK)) {
            if (i > 0) {
                i--;
                player_name[i] = 0;
                needs_update = 1;
            }
        }
        while (has_text_input()) {
            char c = pop_text_input();
            if (MenuFont->has_char(c) && util::text::is_filename_char(c)) {
                if (i >= MAX_PLAYERNAME_INPUT) {
                    continue;
                }
                player_name[i] = c;
                player_name[i + 1] = 0;
                i++;
                needs_update = 1;
            }
        }
        if (needs_update) {
            needs_update = 0;

            menu.clear();

            player_name[i] = '_';
            player_name[i + 1] = 0;
            menu.add_line_centered(player_name, 320, 240);
            player_name[i] = 0;

            menu.add_line_centered("Please enter your name:", 320, 180);
        }
        menu.render();
    }
}

void menu_player_create(int change_player1, int should_return) {
    if (State->player_count >= MAX_PLAYERS - 1) {
        external_error("Sorry, no more players can get onto the list!");
    }

    if (prompt_new_name(State->players[int(State->player_count)].name, should_return)) {
        if (change_player1) {
            strcpy(State->player1, State->players[int(State->player_count)].name);
            State->player_count++;
            if (State->player_count == 1) {
                strcpy(State->player2, State->player1);
            }
        } else {
            strcpy(State->player2, State->players[int(State->player_count)].name);
            State->player_count++;
            if (State->player_count == 1) {
                strcpy(State->player1, State->player2);
            }
        }
    } else {
        should_return = 1;
    }

    if (State->player_count == 0) {
        menu_exit();
    }

    if (should_return) {
        return;
    }

    menu_main();
}

void menu_player_choose(int change_player1, int should_return) {
    menu_nav nav("Choose Player");

    char* player_name = nullptr;
    if (change_player1) {
        player_name = State->player1;
    } else {
        player_name = State->player2;
    }

    nav.add_row(
        "Create New Player", NAV_FUNC(&change_player1, &should_return) {
            menu_player_create(change_player1, should_return);
        });

    for (int i = 0; i < State->player_count; i++) {
        nav.add_row(
            State->players[i].name, NAV_FUNC(&change_player1, &should_return) {
                bool delete_player =
                    should_return && is_key_down(DIK_LCONTROL) && is_key_down(DIK_LMENU);
                if (!delete_player) {
                    // Normal behaviour - select player
                    if (change_player1) {
                        strcpy(State->player1, left.c_str());
                    } else {
                        strcpy(State->player2, left.c_str());
                    }
                } else {
                    // Try to delete player
                    if (State->player_count <= 1) {
                        return;
                    }

                    // Use index in case we have two duplicate player names
                    // There should be a create player check to prevent this because it's buggy
                    // but at the moment there's no check
                    int delete_index = choice - 1;

                    // If deleted player is selected, wipe the name
                    int match1 = 0;
                    if (strcmp(State->player1, left.c_str()) == 0) {
                        match1 = 1;
                    }
                    int match2 = 0;
                    if (strcmp(State->player2, left.c_str()) == 0) {
                        match2 = 1;
                    }

                    for (int i = delete_index; i < State->player_count - 1; i++) {
                        State->players[i] = State->players[i + 1];
                    }
                    State->player_count--;

                    if (match1) {
                        strcpy(State->player1, State->players[0].name);
                    }
                    if (match2) {
                        strcpy(State->player2, State->players[0].name);
                    }
                }
            });
    }

    nav.select_row(player_name);

    int choice = nav.navigate();

    if (choice < 0) {
        if (!should_return) {
            menu_exit();
        }
    }

    if (should_return) {
        return;
    }

    menu_main();
}
