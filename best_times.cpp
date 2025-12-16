#include "ALL.H"

void centiseconds_to_string(long time, char* text, int show_hours) {
    if (time < 0) {
        hiba("centiseconds_to_string time < 0!");
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
            centiseconds = 99;
            seconds = 59;
            minutes = 59;
        }
    }
    text[0] = 0;
    char tmp[30];
    if (hours) {
        // hours:
        itoa(hours, tmp, 10);
        if (!tmp[1]) {
            strcat(text, "0");
        }
        strcat(text, tmp);
        strcat(text, ":");
    }
    // minutes:
    itoa(minutes, tmp, 10);
    if (!tmp[1]) {
        strcat(text, "0");
    }
    strcat(text, tmp);
    strcat(text, ":");
    // seconds:
    itoa(seconds, tmp, 10);
    if (!tmp[1]) {
        strcat(text, "0");
    }
    strcat(text, tmp);
    strcat(text, ":");
    // centiseconds:
    itoa(centiseconds, tmp, 10);
    if (!tmp[1]) {
        strcat(text, "0");
    }
    strcat(text, tmp);
}

void render_topten(palyaegyfeleidok* tten, const char* header, int single) {
    if (tten->idokszama == 0) {
        return;
    }

    szoveglista men;

    men.addszoveg_kozep(header, 320, 37);

    int player_x, time_x;
    if (single) {
        player_x = 120;
        time_x = 360;
    } else {
        player_x = 80;
        time_x = 400;
    }
    for (int i = 0; i < tten->idokszama; i++) {
        char player_text[50];
        strcpy(player_text, tten->nevek1[i]);
        if (!single) {
            strcat(player_text, "  ");
            strcat(player_text, tten->nevek2[i]);
        }
        // Levagjuk hogy ne loghasson ki:
        while (Pmenuabc->len(player_text) > time_x - player_x - 4) {
            player_text[strlen(player_text) - 1] = 0;
        }

        men.addszoveg(player_text, player_x, 110 + i * (SM + 19));
        char time_text[30];
        centiseconds_to_string(tten->idok[i], time_text);
        men.addszoveg(time_text, time_x, 110 + i * (SM + 19));
    }

    mk_emptychar();
    while (1) {
        if (mk_kbhit()) {
            int c = mk_getextchar();
            if (c == MK_ESC || c == MK_ENTER) {
                return;
            }
        }
        men.kirajzol();
    }
}

void menu_internal_topten(int level, int single) {
    char header[100];
    itoa(level + 1, header, 10);
    strcat(header, ": ");
    strcat(header, getleveldescription(level));

    palyaegyfeleidok* tten = &State->palyakidejei[level].singleidok;
    if (!single) {
        tten = &State->palyakidejei[level].multiidok;
    }

    render_topten(tten, header, single);
}

void menu_external_topten(topol* top, int single) {
    if (single) {
        render_topten(&top->idok.singleidok, top->levelname, single);
    } else {
        render_topten(&top->idok.multiidok, top->levelname, single);
    }
}

void menu_best_times_choose_level(int single) {
    int visible_levels = 0;
    for (int i = 0; i < State->jatekosokszama; i++) {
        if (State->jatekosok[i].sikerespalyakszama > visible_levels) {
            visible_levels = int(State->jatekosok[i].sikerespalyakszama);
        }
    }
    /*if( visible_levels == 0 )
        return;
    if( visible_levels == 1 ) {
        menu_internal_topten( 0, single );
        return;
    } */

    visible_levels++; // Mivel utolso nem sikeres palya is szamit
    if (visible_levels >= Palyaszam) {
        visible_levels = Palyaszam - 1; // mivel utolso palya nem rendes palya
    }

    valaszt2 nav;
    nav.kur = 0;
    nav.egykepen = LISTegykepen;
    nav.x0 = 61;
    nav.x0_tab = 380;
    nav.y0 = LISTy0;
    nav.dy = LISTdy;
    nav.escelheto = 1;
    if (single) {
        strcpy(nav.cim, "Single Player Best Times");
    } else {
        strcpy(nav.cim, "Multi Player Best Times");
    }

    for (int i = 0; i < visible_levels; i++) {
        palyaegyfeleidok* tten = &State->palyakidejei[i].singleidok;
        if (!single) {
            tten = &State->palyakidejei[i].multiidok;
        }
        int has_time = 1;
        if (tten->idokszama == 0) {
            has_time = 0;
        }
        char player_text[50];
        strcpy(player_text, tten->nevek1[0]);
        if (!single) {
            strcat(player_text, "  ");
            strcat(player_text, tten->nevek2[0]);
        }
        // Sorszam 1-tol:
        itoa(i + 1, Rubrikak[i], 10);
        strcat(Rubrikak[i], " ");
        // Level neve:
        strcat(Rubrikak[i], getleveldescription(i));
        // strcat( Rubrikak[i], "Nincs meg nev." );
        strcat(Rubrikak[i], " ");
        while (Pmenuabc->len(Rubrikak[i]) < 184) {
            strcat(Rubrikak[i], " ");
        }
        // Jatekos neve:
        if (has_time) {
            strcpy(Rubrikak_tab[i], player_text);
        } else {
            strcpy(Rubrikak_tab[i], "-");
        }

        /*strcat( Korny->rubrikak[i], " " );
        // Legjobb idok kiirasa: Sajnos most nem fer ki:
        char tmp[30];
        centiseconds_to_string( tten->idok[0], tmp );
        strcat( Korny->rubrikak[i], tmp );
        */
    }

    nav.bead(visible_levels, 1);
    while (1) {
        int choice = nav.valassz();

        if (choice < 0) {
            return;
        }

        menu_internal_topten(choice, single);
    }
}

void menu_best_times(void) {
    valaszt2 nav;
    if (State->single) {
        nav.kur = 0;
    } else {
        nav.kur = 1;
    }
    nav.egykepen = 6;
    nav.x0 = 170;
    nav.y0 = 190;
    nav.dy = 50;
    nav.escelheto = 1;
    strcpy(nav.cim, "Best Times");

    strcpy(Rubrikak[0], "Single Player Times");
    strcpy(Rubrikak[1], "Multi Player Times");

    nav.bead(2);
    while (1) {
        int choice = nav.valassz();

        if (choice < 0) {
            return;
        }
        if (choice == 0) {
            menu_best_times_choose_level(1);
        } else {
            menu_best_times_choose_level(0);
        }
    }
}
