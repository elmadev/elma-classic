#include "SKIP.H"
#include "main.h"
#include "menu_pic.h"
#include "platform_utils.h"
#include "TOPOL.H"
#include <cstdlib>
#include <cstring>

int is_skippable(int index) {
    int MAX_SKIPS = 5;

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
    int is_skippable = used_skips < MAX_SKIPS;

    menu_pic menu;
    if (is_skippable) {
        // Lehet meg skippelni!
        int x0 = 320;
        int y0 = 150;
        int dy = 50;
        char value[10];
        itoa(MAX_SKIPS, value, 10);
        char tmp[100] = "Number of skips allowed: ";
        strcat(tmp, value);
        // Korny->pabc_afterplaymenu->writekozep( Korny->picbuffer, x0, y0, tmp );
        menu.add_line_centered(tmp, x0, y0);
        itoa(used_skips, value, 10);
        strcpy(tmp, "Number of skips so far:   ");
        strcat(tmp, value);
        // Korny->pabc_afterplaymenu->writekozep( Korny->picbuffer, x0, y0+dy, tmp );
        menu.add_line_centered(tmp, x0, y0 + dy);
        // Korny->pabc_afterplaymenu->writekozep( Korny->picbuffer, x0, y0+3*dy,
        //	"Press a key to continue!" );
        menu.add_line_centered("Press a key to continue!", x0, y0 + 3 * dy);
    } else {
        // Nem lehet mar skippelni!
        int x0 = 320;
        int y0 = 100;
        int dy = 50;
        // Korny->pabc_afterplaymenu->writekozep( Korny->picbuffer, x0, y0,
        //	"You already have reached" );
        menu.add_line_centered("You already have reached", x0, y0);
        // Korny->pabc_afterplaymenu->writekozep( Korny->picbuffer, x0, y0+dy,
        //	"the maximum number of" );
        menu.add_line_centered("the maximum number of", x0, y0 + dy);

        char value[10];
        itoa(MAX_SKIPS, value, 10);
        char tmp[100] = "skips allowed (";
        strcat(tmp, value);
        strcat(tmp, ")!");
        // Korny->pabc_afterplaymenu->writekozep( Korny->picbuffer, x0, y0+2*dy, tmp );
        menu.add_line_centered(tmp, x0, y0 + 2 * dy);
        // Korny->pabc_afterplaymenu->writekozep( Korny->picbuffer, x0, y0+3*dy,
        //	"Fullfill a previously skipped" );
        menu.add_line_centered("Fullfill a previously skipped", x0, y0 + 3 * dy);
        // Korny->pabc_afterplaymenu->writekozep( Korny->picbuffer, x0, y0+4*dy,
        //	"level to skip this level!" );
        menu.add_line_centered("level to skip this level!", x0, y0 + 4 * dy);
        // Korny->pabc_afterplaymenu->writekozep( Korny->picbuffer, x0, y0+5*dy,
        //	"Press a key to continue!" );
        menu.add_line_centered("Press a key to continue!", x0, y0 + 6 * dy);
    }

    // Kep kiteves:
    empty_keypress_buffer();
    while (1) {
        if (has_keypress()) {
            get_keypress();
            return is_skippable;
        }
        menu.render();
    }
}
