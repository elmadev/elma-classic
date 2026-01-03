#include "ALL.H"
#include "menu_pic.h"

int skippelheto(int level) {
    int maxskippelheto = 5;

    player* pjatekos = State->get_player(State->player1);
    if (pjatekos->levels_completed != level) {
        internal_error("skippelheto-ben pjatekos->sikerespalyakszam != level!");
    }
    int eddigskippelt = 0;
    for (int i = 0; i < level; i++) {
        if (pjatekos->skipped[i]) {
            eddigskippelt++;
        }
    }
    if (eddigskippelt > maxskippelheto) {
        internal_error("eddigskippelt > maxskippelheto!");
    }
    int skippelheto = eddigskippelt < maxskippelheto;

    menu_pic szl;
    if (skippelheto) {
        // Lehet meg skippelni!
        int x0 = 320;
        int y0 = 150;
        int dy = 50;
        char szam[10];
        itoa(maxskippelheto, szam, 10);
        char tmp[100] = "Number of skips allowed: ";
        strcat(tmp, szam);
        // Korny->pabc_afterplaymenu->writekozep( Korny->picbuffer, x0, y0, tmp );
        szl.add_line_centered(tmp, x0, y0);
        itoa(eddigskippelt, szam, 10);
        strcpy(tmp, "Number of skips so far:   ");
        strcat(tmp, szam);
        // Korny->pabc_afterplaymenu->writekozep( Korny->picbuffer, x0, y0+dy, tmp );
        szl.add_line_centered(tmp, x0, y0 + dy);
        // Korny->pabc_afterplaymenu->writekozep( Korny->picbuffer, x0, y0+3*dy,
        //	"Press a key to continue!" );
        szl.add_line_centered("Press a key to continue!", x0, y0 + 3 * dy);
    } else {
        // Nem lehet mar skippelni!
        int x0 = 320;
        int y0 = 100;
        int dy = 50;
        // Korny->pabc_afterplaymenu->writekozep( Korny->picbuffer, x0, y0,
        //	"You already have reached" );
        szl.add_line_centered("You already have reached", x0, y0);
        // Korny->pabc_afterplaymenu->writekozep( Korny->picbuffer, x0, y0+dy,
        //	"the maximum number of" );
        szl.add_line_centered("the maximum number of", x0, y0 + dy);

        char szam[10];
        itoa(maxskippelheto, szam, 10);
        char tmp[100] = "skips allowed (";
        strcat(tmp, szam);
        strcat(tmp, ")!");
        // Korny->pabc_afterplaymenu->writekozep( Korny->picbuffer, x0, y0+2*dy, tmp );
        szl.add_line_centered(tmp, x0, y0 + 2 * dy);
        // Korny->pabc_afterplaymenu->writekozep( Korny->picbuffer, x0, y0+3*dy,
        //	"Fullfill a previously skipped" );
        szl.add_line_centered("Fullfill a previously skipped", x0, y0 + 3 * dy);
        // Korny->pabc_afterplaymenu->writekozep( Korny->picbuffer, x0, y0+4*dy,
        //	"level to skip this level!" );
        szl.add_line_centered("level to skip this level!", x0, y0 + 4 * dy);
        // Korny->pabc_afterplaymenu->writekozep( Korny->picbuffer, x0, y0+5*dy,
        //	"Press a key to continue!" );
        szl.add_line_centered("Press a key to continue!", x0, y0 + 6 * dy);
    }

    // Kep kiteves:
    mk_emptychar();
    while (1) {
        if (mk_kbhit()) {
            mk_getextchar();
            return skippelheto;
        }
        szl.render();
    }
}
