#include "skip.h"
#include "keys.h"
#include "main.h"
#include "menu_pic.h"
#include "platform_utils.h"
#include "state.h"
#include <cstdlib>
#include <cstring>

bool is_skippable(int index) {
    constexpr int MAX_SKIPS = 5;

    player* cur_player = State->get_player(State->player1);
    if (cur_player->levels_completed != index) {
        internal_error("Can only try to skip last level!");
    }
    int used_skips = 0;
    for (int i = 0; i < index; i++) {
        if (cur_player->skipped[i]) {
            used_skips++;
        }
    }
    if (used_skips > MAX_SKIPS) {
        internal_error("used_skips > MAX_SKIPS!");
    }
    bool is_skippable = used_skips < MAX_SKIPS;

    menu_pic menu;
    if (is_skippable) {
        int x0 = 320;
        int y0 = 150;
        int dy = 50;
        char value[10];
        itoa(MAX_SKIPS, value, 10);
        char tmp[100] = "Number of skips allowed: ";
        strcat(tmp, value);
        menu.add_line_centered(tmp, x0, y0);
        itoa(used_skips, value, 10);
        strcpy(tmp, "Number of skips so far:   ");
        strcat(tmp, value);
        menu.add_line_centered(tmp, x0, y0 + dy);
        menu.add_line_centered("Press a key to continue!", x0, y0 + 3 * dy);
    } else {
        int x0 = 320;
        int y0 = 100;
        int dy = 50;
        menu.add_line_centered("You already have reached", x0, y0);
        menu.add_line_centered("the maximum number of", x0, y0 + dy);

        char value[10];
        itoa(MAX_SKIPS, value, 10);
        char tmp[100] = "skips allowed (";
        strcat(tmp, value);
        strcat(tmp, ")!");
        menu.add_line_centered(tmp, x0, y0 + 2 * dy);
        menu.add_line_centered("Fullfill a previously skipped", x0, y0 + 3 * dy);
        menu.add_line_centered("level to skip this level!", x0, y0 + 4 * dy);
        menu.add_line_centered("Press a key to continue!", x0, y0 + 6 * dy);
    }

    empty_keypress_buffer();
    while (true) {
        if (has_keypress()) {
            get_keypress();
            return is_skippable;
        }
        menu.render();
    }
}
