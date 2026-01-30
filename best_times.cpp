#include "abc8.h"
#include "best_times.h"
#include "keys.h"
#include "level.h"
#include "main.h"
#include "menu_nav.h"
#include "menu_pic.h"
#include "platform_utils.h"
#include "state.h"
#include <cstdlib>
#include <cstring>

// Format time as 00:00:00 (or 00:00:00:00 if hours are allowed)
void centiseconds_to_string(long time, char* text, bool show_hours) {
    // Calculate time
    if (time < 0) {
        internal_error("centiseconds_to_string time < 0!");
    }
    int centiseconds = int(time % 100);
    time /= 100;
    int seconds = int(time % 60);
    time /= 60;
    int minutes = int(time % 60);
    time /= 60;
    int hours = 0;
    if (time) {
        if (show_hours) {
            hours = time;
        } else {
            // Cap to 59:59:99 if hours disallowed
            centiseconds = 99;
            seconds = 59;
            minutes = 59;
        }
    }
    // Write time
    text[0] = 0;
    char tmp[30];
    // hours
    if (hours) {
        itoa(hours, tmp, 10);
        if (!tmp[1]) {
            strcat(text, "0");
        }
        strcat(text, tmp);
        strcat(text, ":");
    }
    // minutes
    itoa(minutes, tmp, 10);
    if (!tmp[1]) {
        strcat(text, "0");
    }
    strcat(text, tmp);
    strcat(text, ":");
    // seconds
    itoa(seconds, tmp, 10);
    if (!tmp[1]) {
        strcat(text, "0");
    }
    strcat(text, tmp);
    strcat(text, ":");
    // centiseconds
    itoa(centiseconds, tmp, 10);
    if (!tmp[1]) {
        strcat(text, "0");
    }
    strcat(text, tmp);
}

// Render the best times list (both internal and external)
// Some examples:
// PlayerAName               00:14:00 (Single)
// PlayerAName  PlayerBName  00:14:00 (Multi)
// LongPlayerAName LongPlaye 00:14:00 (truncated)
static void render_topten(topten* tten, const char* header, int single) {
    if (tten->times_count == 0) {
        return;
    }

    menu_pic men(false);
    men.add_line_centered(header, 320, 37);

    int player_x;
    int time_x;
    if (single) {
        player_x = 120;
        time_x = 360;
    } else {
        player_x = 80;
        time_x = 400;
    }
    for (int i = 0; i < tten->times_count; i++) {
        char player_text[50];
        strcpy(player_text, tten->names1[i]);
        if (!single) {
            strcat(player_text, "  ");
            strcat(player_text, tten->names2[i]);
        }
        // Truncate the player names so it doesn't overlap with the times
        while (MenuFont->len(player_text) > time_x - player_x - 4) {
            player_text[strlen(player_text) - 1] = 0;
        }

        constexpr int ROW_HEIGHT = 34;
        men.add_line(player_text, player_x, 110 + i * ROW_HEIGHT);
        char time_text[30];
        centiseconds_to_string(tten->times[i], time_text);
        men.add_line(time_text, time_x, 110 + i * ROW_HEIGHT);
    }

    empty_keypress_buffer();
    while (true) {
        if (has_keypress()) {
            Keycode c = get_keypress();
            if (c == KEY_ESC || c == KEY_ENTER) {
                return;
            }
        }
        men.render();
    }
}

// Render the internal best times list
void menu_internal_topten(int level, bool single) {
    char header[100];
    itoa(level + 1, header, 10);
    strcat(header, ": ");
    strcat(header, get_internal_level_name(level));

    topten* tten = &State->toptens[level].single;
    if (!single) {
        tten = &State->toptens[level].multi;
    }

    render_topten(tten, header, single);
}

// Render the external best times list
void menu_external_topten(level* top, bool single) {
    if (single) {
        render_topten(&top->toptens.single, top->level_name, single);
    } else {
        render_topten(&top->toptens.multi, top->level_name, single);
    }
}

// Main Menu Best Times
// Show a list of unlocked internals so that can you view the best times
void menu_best_times_choose_level(bool single) {
    // Find the last level anyone has unlocked
    int visible_levels = 0;
    for (int i = 0; i < State->player_count; i++) {
        if (State->players[i].levels_completed > visible_levels) {
            visible_levels = int(State->players[i].levels_completed);
        }
    }
    // Also show the last uncompleted level
    visible_levels++;
    // Disallow "More Levels"
    if (visible_levels >= INTERNAL_LEVEL_COUNT) {
        visible_levels = INTERNAL_LEVEL_COUNT - 1;
    }

    menu_nav nav;
    nav.selected_index = 0;
    nav.x_left = 61;
    nav.x_right = 380;
    if (single) {
        strcpy(nav.title, "Single Player Best Times");
    } else {
        strcpy(nav.title, "Multi Player Best Times");
    }

    // Draw "1 Warm Up         bestplayer"
    for (int i = 0; i < visible_levels; i++) {
        topten* tten = &State->toptens[i].single;
        if (!single) {
            tten = &State->toptens[i].multi;
        }

        // "1 Warm Up"
        itoa(i + 1, NavEntriesLeft[i], 10);
        strcat(NavEntriesLeft[i], " ");
        strcat(NavEntriesLeft[i], get_internal_level_name(i));

        // Best player, if exists
        strcpy(NavEntriesRight[i], "-");
        if (tten->times_count > 0) {
            strcpy(NavEntriesRight[i], tten->names1[0]);
            if (!single) {
                strcat(NavEntriesRight[i], "  ");
                strcat(NavEntriesRight[i], tten->names2[0]);
            }
        }
    }

    nav.setup(visible_levels, true);
    while (true) {
        int choice = nav.navigate();
        if (choice < 0) {
            return;
        }
        menu_internal_topten(choice, single);
    }
}

void menu_best_times() {
    menu_nav nav;
    if (State->single) {
        nav.selected_index = 0;
    } else {
        nav.selected_index = 1;
    }
    nav.x_left = 170;
    nav.y_entries = 190;
    nav.dy = 50;
    strcpy(nav.title, "Best Times");

    strcpy(NavEntriesLeft[0], "Single Player Times");
    strcpy(NavEntriesLeft[1], "Multi Player Times");

    nav.setup(2);
    while (true) {
        int choice = nav.navigate();
        if (choice < 0) {
            return;
        }
        if (choice == 0) {
            menu_best_times_choose_level(true);
        } else {
            menu_best_times_choose_level(false);
        }
    }
}
