#include "JATEKOS.H"
#include "abc8.h"
#include "keys.h"
#include "main.h"
#include "menu_dialog.h"
#include "menu_intro.h"
#include "menu_main.h"
#include "menu_nav.h"
#include "menu_pic.h"
#include "platform_impl.h"
#include "state.h"
#include "util/util.h"
#include <directinput/scancodes.h>
#include <cstring>

constexpr int MAX_PLAYERNAME_INPUT = 8;
static_assert(MAX_PLAYERNAME_INPUT < MAX_PLAYERNAME_LENGTH);

// Prompt for a new player name. Return true if name successfully selected
static bool prompt_new_name(char* player_name) {
    menu_pic menu(false);

    int i = 0;
    empty_keypress_buffer();
    bool needs_update = true;
    player_name[0] = 0;
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
                player_name[i] = 0;
                needs_update = true;
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
                needs_update = true;
            }
        }
        if (needs_update) {
            needs_update = false;

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

bool menu_player_create(bool change_player1) {
    if (State->player_count >= MAX_PLAYERS - 1) {
        menu_dialog("Sorry, no more players can get onto the list!",
                    "Delete a player to be able to create a new one!");
        return false;
    }

    char* player_name = State->players[int(State->player_count)].name;
    if (prompt_new_name(player_name)) {
        for (int i = 0; i < State->player_count; i++) {
            if (strcmp(State->players[i].name, player_name) == 0) {
                memset(player_name, 0, MAX_PLAYERNAME_LENGTH);
                menu_dialog("This player name already exists!");
                return false;
            }
        }

        if (change_player1) {
            strcpy(State->player1, player_name);
            State->player_count++;
            if (State->player_count == 1) {
                strcpy(State->player2, State->player1);
            }
        } else {
            strcpy(State->player2, player_name);
            State->player_count++;
            if (State->player_count == 1) {
                strcpy(State->player1, State->player2);
            }
        }
        return true;
    }
    return false;
}

bool menu_player_choose(bool change_player1, bool allow_delete) {
    menu_nav nav("Choose Player");

    bool success = true;
    nav.add_row(
        "Create New Player",
        NAV_FUNC(&change_player1, &success) { success = menu_player_create(change_player1); });

    for (int i = 0; i < State->player_count; i++) {
        nav.add_row(
            State->players[i].name, NAV_FUNC(&change_player1, &allow_delete) {
                bool delete_player =
                    allow_delete && is_key_down(DIK_LCONTROL) && is_key_down(DIK_LMENU);
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
                    for (int i = choice - 1; i < State->player_count - 1; i++) {
                        State->players[i] = State->players[i + 1];
                    }
                    State->player_count--;

                    // If deleted player is selected, wipe the name
                    if (strcmp(State->player1, left.c_str()) == 0) {
                        strcpy(State->player1, State->players[0].name);
                    }
                    if (strcmp(State->player2, left.c_str()) == 0) {
                        strcpy(State->player2, State->players[0].name);
                    }
                }
            });
    }

    char* player_name = change_player1 ? State->player1 : State->player2;
    nav.select_row(player_name);

    int choice = nav.navigate();
    return success && choice >= 0;
}
