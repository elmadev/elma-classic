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
#include <format>

// Format time as 00:00:00 (or 00:00:00:00 if hours are allowed)
// If `compact` is true, formats as: x:xx or x:xx:xx.
void centiseconds_to_string(int time, char* text, bool show_hours, bool compact) {
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
    if (time > 0) {
        if (show_hours) {
            hours = time;
        } else {
            // Cap to 59:59:99 if hours disallowed
            minutes = 59;
            seconds = 59;
            centiseconds = 99;
        }
    }

    text[0] = '\0';
    char* it = text;

    if (compact) {
        if (hours > 0) {
            it = std::format_to(it, "{}:{:02}:{:02}:{:02}", hours, minutes, seconds, centiseconds);
        } else if (minutes > 0) {
            it = std::format_to(it, "{}:{:02}:{:02}", minutes, seconds, centiseconds);
        } else {
            it = std::format_to(it, "{}:{:02}", seconds, centiseconds);
        }

        *it = '\0';
        return;
    }

    if (show_hours) {
        it = std::format_to(it, "{:02}:{:02}:{:02}:{:02}", hours, minutes, seconds, centiseconds);
    } else {
        it = std::format_to(it, "{:02}:{:02}:{:02}", minutes, seconds, centiseconds);
    }

    *it = '\0';
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

    std::string title = std::format("{} Player Best Times", single ? "Single" : "Multi");
    menu_nav nav(title);
    nav.x_left = 61;
    nav.x_right = 380;

    // Draw "1 Warm Up         bestplayer"
    for (int i = 0; i < visible_levels; i++) {
        topten* tten = &State->toptens[i].single;
        if (!single) {
            tten = &State->toptens[i].multi;
        }

        std::string level = std::format("{} {}", i + 1, get_internal_level_name(i));
        std::string best = "-";
        if (tten->times_count > 0) {
            if (single) {
                best = tten->names1[0];
            } else {
                best = std::format("{}  {}", tten->names1[0], tten->names2[0]);
            }
        }

        nav.add_row(level, best, NAV_FUNC(&single) { menu_internal_topten(choice, single); });
    }

    while (true) {
        int choice = nav.navigate();
        if (choice < 0) {
            return;
        }
    }
}

void menu_best_times() {
    menu_nav nav("Best Times");
    nav.x_left = 170;
    nav.y_entries = 190;
    nav.dy = 50;

    nav.add_row("Single Player Times", NAV_FUNC() { menu_best_times_choose_level(true); });
    nav.add_row("Multi Player Times", NAV_FUNC() { menu_best_times_choose_level(false); });

    nav.select_row(State->single ? 0 : 1);

    while (true) {
        int choice = nav.navigate();
        if (choice < 0) {
            return;
        }
    }
}
