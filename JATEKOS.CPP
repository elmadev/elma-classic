#include "JATEKOS.H"
#include "abc8.h"
#include "keys.h"
#include "main.h"
#include "menu_intro.h"
#include "menu_main.h"
#include "menu_nav.h"
#include "menu_pic.h"
#include "platform_impl.h"
#include "platform_utils.h"
#include "state.h"
#include <cstring>
#include <directinput/scancodes.h>

static int Maxnevhossz = 8; // Ez kisebb kell hogy legyen MAX_PLAYERNAME_LENGTH-nal!

// Siker eseten igazzal ter vissza:
static int bevesznevet(char* nev, int visszater) {
    menu_pic szovlist(false);

    int i = 0;
    int valtozott = 1;
    nev[0] = 0;
    while (1) {
        handle_events();
        if (was_key_just_pressed(DIK_ESCAPE)) {
            if (visszater) {
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
                nev[i] = 0;
                valtozott = 1;
            }
        }
        while (has_text_input()) {
            char c = pop_text_input();
            if (MenuFont->has_char(c) && is_char_valid_for_filename(c)) {
                if (i >= Maxnevhossz) {
                    continue;
                }
                nev[i] = c;
                nev[i + 1] = 0;
                i++;
                valtozott = 1;
            }
        }
        if (valtozott) {
            valtozott = 0;

            szovlist.clear();

            nev[i] = '_';
            nev[i + 1] = 0;
            szovlist.add_line_centered(nev, 320, 240);
            nev[i] = 0;

            szovlist.add_line_centered("Please enter your name:", 320, 180);
        }
        szovlist.render();
    }
}

void newjatekos(int A, int visszater) {
    // Ellenorzes:
    if (State->player_count >= MAX_PLAYERS - 1) {
        external_error("Sorry, no more players can get onto the list!");
    }
    int maxrub = NavEntriesLeftMaxLength;
    if (MAX_PLAYERS > maxrub) {
        internal_error("MAX_PLAYERS > NavEntriesLeftMaxLength!");
    }

    if (bevesznevet(State->players[int(State->player_count)].name, visszater)) {
        if (A) {
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
        visszater = 1; // Ha elescapelte, akkor visszamegyunk menube
    }

    if (State->player_count == 0) {
        menu_exit();
    }

    if (visszater) {
        return;
    }

    menu_main();
}

void jatekosvalasztas(int A, int visszater) {
    // Valasztas:
    menu_nav_old* pval = NULL;
    pval = new menu_nav_old;
    char* eddigjatekos = NULL;
    if (A) {
        eddigjatekos = State->player1;
    } else {
        eddigjatekos = State->player2;
    }
    strcpy(pval->title, "Choose Player");

    strcpy(NavEntriesLeft[0], "Create New Player");
    pval->selected_index = 0;
    for (int i = 0; i < State->player_count; i++) {
        if (strlen(State->players[i].name) > NAV_ENTRY_TEXT_MAX_LENGTH) {
            internal_error("Tul hosszu nev jatekosvalasztas-ban!");
        }
        strcpy(NavEntriesLeft[i + 1], State->players[i].name);

        if (strcmp(eddigjatekos, State->players[i].name) == 0) {
            pval->selected_index = i + 1;
        }
    }

    pval->setup(State->player_count + 1);

    int eredmeny = pval->navigate();

    delete pval;
    pval = NULL;

    if (eredmeny < 0) {
        if (visszater) {
            return;
        }
        menu_exit();
    }
    if (eredmeny == 0) {
        newjatekos(A, visszater);
        return;
    }

    if (CtrlAltPressed && visszater) {
        // Ezt a jatekost akarja torolni (opciokbol hivva):
        if (State->player_count <= 1) {
            return;
        }

        int torlendo = eredmeny - 1;

        // Megjegyezzuk hogy A vagy B nem torolt jatekos-e veletlenul:
        int atorlodott = 0;
        if (strcmp(State->player1, State->players[torlendo].name) == 0) {
            atorlodott = 1;
        }
        int btorlodott = 0;
        if (strcmp(State->player2, State->players[torlendo].name) == 0) {
            btorlodott = 1;
        }

        // Toroljuk jatekost, tobbit elore hozzuk eggyel:
        for (int i = torlendo; i < State->player_count - 1; i++) {
            // i-edik helyre elore hozzuk i+1-edik helyrol:
            State->players[i] = State->players[i + 1];
        }
        State->player_count--;

        if (atorlodott) {
            strcpy(State->player1, State->players[0].name);
        }
        if (btorlodott) {
            strcpy(State->player2, State->players[0].name);
        }

        return;
    }

    if (A) {
        strcpy(State->player1, State->players[eredmeny - 1].name);
    } else {
        strcpy(State->player2, State->players[eredmeny - 1].name);
    }

    if (visszater) {
        return;
    }

    menu_main();
}
