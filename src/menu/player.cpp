#include "menu/player.h"
#include "game/state.h"
#include "menu/dialog.h"
#include "menu/nav.h"
#include "menu/pic.h"
#include "pic/abc8.h"
#include "platform/implementation.h"
#include "platform/scancode.h"
#include "platform/text_input.h"
#include "util/util.h"
#include <cstring>

constexpr int MAX_PLAYERNAME_INPUT = 8;
static_assert(MAX_PLAYERNAME_INPUT < MAX_PLAYERNAME_LENGTH);

// Prompt for a new player name. Return true if name successfully selected
static bool prompt_new_name(char* player_name) {
    return prompt_text_input("Please enter your name:", player_name, MAX_PLAYERNAME_INPUT, nullptr);
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
