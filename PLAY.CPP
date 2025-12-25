#include "ALL.H"

/*static void betuki( int oszlop, char c ) {
    int x = 50+oszlop*19;
    int y = 6*SM + 8;
    blit8( Korny->picbuffer, Korny->ppic_saveplay, x, y, x, y, x+19, y+30 );
    if( c == '_' ) {
        c = '-';
        y += 11;
    }
    char tmp[2] = "Q";
    tmp[0] = c;
    Korny->pabc_newjatekos->write( Korny->picbuffer, x, y, tmp );
    lassufizre( Korny->picbuffer, Korny->pal_newjatekos );
} */

/*static int bevesznevet( char* nev ) {
    int i = 0;
    betuki( 0, '_' );
    while( 1 ) {
        mk_emptychar();
        int c = mk_getextchar();
        if( c == MK_ESC )
            return 0;
        if( c == MK_ENTER ) {
            // ENTER
            if( i > 0 ) {
                nev[i] = 0;
                return 1;
            }
        }
        if( ( c >= 'A' && c <= 'Z' ) ||
            ( c >= 'a' && c <= 'z' ) ||
            ( c >= '0' && c <= '9' ) ) {
            if( i >= 8 )
                continue;
            betuki( i, c );
            nev[i] = c;
            i++;
            betuki( i, '_' );
        }
        if( c == MK_BACKSPACE ) {
            // <- (torles):
            if( i > 0 ) {
                betuki( i, ' ' );
                i--;
                betuki( i, '_' );
            }
        }
    }
}*/

// Siker eseten igazzal ter vissza:
static int bevesznevet(char* nev) {
    menu_pic szovlist;

    int i = 0;
    mk_emptychar();
    int valtozott = 1;
    nev[0] = 0;
    while (1) {
        while (mk_kbhit()) {
            int c = mk_getextchar();
            if (c == MK_ESC) {
                return 0;
            }
            if (c == MK_ENTER) {
                // ENTER
                if (i > 0) {
                    return 1;
                }
            }
            if ((c >= 'A' && c <= 'Z') || (c >= 'a' && c <= 'z') || (c >= '0' && c <= '9')) {
                if (i >= 8) {
                    continue;
                }
                nev[i] = (char)c;
                nev[i + 1] = 0;
                i++;
                valtozott = 1;
            }
            if (c == MK_BACKSPACE) {
                // <- (torles):
                if (i > 0) {
                    i--;
                    nev[i] = 0;
                    valtozott = 1;
                }
            }
        }
        if (valtozott) {
            valtozott = 0;

            szovlist.clear();

            nev[i] = '_';
            nev[i + 1] = 0;
            szovlist.add_line_centered(nev, 320, 240);
            nev[i] = 0;

            szovlist.add_line_centered("Please enter the filename:", 320, 180);
            // szovlist.add_line_centered( "Press ESC to exit", 320, 300 );
        }
        szovlist.render();
    }
}

static int Eltolas = 15;

static void konvoda(char* text) {
    int betuszam = 'z' - 'A' + 1;
    while (*text) {
        if (*text != 0x20) {
            int sorszam = *text - 'A';        // sorszam legkisebb erteke 0
            sorszam = betuszam - 1 - sorszam; // Megforditjuk sorrendet
            sorszam += Eltolas;
            sorszam %= betuszam;
            if (sorszam < 0 || sorszam >= betuszam) {
                hiba("B508976!");
            }
            *text = (char)('A' + sorszam);
        }
        text++;
    }
}

static void konvback(char* text) {
    int betuszam = 'z' - 'A' + 1;
    while (*text) {
        if (*text != 0x20) {
            int sorszam = *text - 'A'; // sorszam legkisebb erteke 0
            sorszam -= Eltolas;
            sorszam += betuszam * 2;
            sorszam %= betuszam;
            sorszam = betuszam - 1 - sorszam; // Megforditjuk sorrendet
            if (sorszam < 0 || sorszam >= betuszam) {
                hiba("B508976!");
            }
            *text = (char)('A' + sorszam);
        }
        text++;
    }
}

/*static void gratulacio( void ) {
    blit8( Korny->picbuffer, Korny->ppic_fomenu );

    int x1 = 160;
    int yo = 4;
    int dy = 24;

    Korny->pabc_fomenu->writekozep( Korny->picbuffer, x1, yo,      "Congratulations!");
    Korny->pabc_fomenu->writekozep( Korny->picbuffer, x1, yo+dy,   "You have just completed" );
    Korny->pabc_fomenu->writekozep( Korny->picbuffer, x1, yo+dy*2, "the final level." );
    Korny->pabc_fomenu->writekozep( Korny->picbuffer, x1, yo+dy*3, "" );
    Korny->pabc_fomenu->writekozep( Korny->picbuffer, x1, yo+dy*4, "Check out readme.txt on" );
    Korny->pabc_fomenu->writekozep( Korny->picbuffer, x1, yo+dy*5, "how you can get some" );
    Korny->pabc_fomenu->writekozep( Korny->picbuffer, x1, yo+dy*6, "more levels for free!" );
    Korny->pabc_fomenu->writekozep( Korny->picbuffer, x1, yo+dy*7, "Press 'c' to continue!" );

    lassufizre( Korny->picbuffer, Korny->pal_fomenu );
    while( 1 ) {
        mk_emptychar();
        int c = mk_getextchar();
        if( c == 'c' || c == 'C' )
            return;
    }
} */

void saveplay(long belyeg) {
    // Korny->pabc_saveplay->write( Korny->picbuffer, 20, 2*SM, "To exit press ESC!" );
    // Korny->pabc_saveplay->write( Korny->picbuffer, 20, 5*SM, "Please enter filename:" );
    // lassufizre( Korny->picbuffer, Korny->pal_saveplay );
    char tmp[20] = "";
    if (!bevesznevet(tmp)) {
        return;
    }
    if (strcmp(tmp, "memtocpy") == 0) {
        // Itt most kiirja specialis uzenetet:

        // char* sor1 = "This is the executable code";
        // char* sor2 = "of    ______ __________"; ket kisbetuvel
        // char* sor3 = "copyrighted in 1997"
        // char* sor4 = "by    Balazs Rozsa"
        // char* sor1 = "vbaW aW Vbe eRegUVih^e g[fe";
        // char* sor2 = "[d    OgVa[3 wUZeXMX[WW";
        // char* sor3 = "g[ZQXacbVef a3 3333";
        // char* sor4 = "hQ    Ni^iPW x[PWi";

        // vbaW aW Vbe eRegUVih^e g[fe|
        // vbaW|
        //  aW |
        // Vbe |
        // eReg|
        // UVih|
        //^e g|
        //[fe|
        char sor1[30];
        sor1[0] = 'v';
        sor1[1] = 'b';
        sor1[2] = 'a';
        sor1[3] = 'W';
        sor1[4] = ' ';
        sor1[5] = 'a';
        sor1[6] = 'W';
        sor1[7] = ' ';
        sor1[8] = 'V';
        sor1[9] = 'b';
        sor1[10] = 'e';
        sor1[11] = ' ';
        sor1[12] = 'e';
        sor1[13] = 'R';
        sor1[14] = 'e';
        sor1[15] = 'g';
        sor1[16] = 'U';
        sor1[17] = 'V';
        sor1[18] = 'i';
        sor1[19] = 'h';
        sor1[20] = '^';
        sor1[21] = 'e';
        sor1[22] = ' ';
        sor1[23] = 'g';
        sor1[24] = '[';
        sor1[25] = 'f';
        sor1[26] = 'e';
        sor1[27] = 0;

        //[d    OgVa[3 wUZeXMX[WW|
        //[d  |
        //  Og|
        // Va[3|
        // wUZ|
        // eXMX|
        //[WW|
        char sor2[30];
        sor2[0] = '[';
        sor2[1] = 'd';
        sor2[2] = ' ';
        sor2[3] = ' ';
        sor2[4] = ' ';
        sor2[5] = ' ';
        sor2[6] = 'O';
        sor2[7] = 'g';
        sor2[8] = 'V';
        sor2[9] = 'a';
        sor2[10] = '[';
        sor2[11] = '3';
        sor2[12] = ' ';
        sor2[13] = 'w';
        sor2[14] = 'U';
        sor2[15] = 'Z';
        sor2[16] = 'e';
        sor2[17] = 'X';
        sor2[18] = 'M';
        sor2[19] = 'X';
        sor2[20] = '[';
        sor2[21] = 'W';
        sor2[22] = 'W';
        sor2[23] = 0;

        // g[ZQXacbVef a3 3333|
        // g[ZQ|
        // Xacb|
        // Vef |
        // a3 3|
        // 333|
        char sor3[30];
        sor3[0] = 'g';
        sor3[1] = '[';
        sor3[2] = 'Z';
        sor3[3] = 'Q';
        sor3[4] = 'X';
        sor3[5] = 'a';
        sor3[6] = 'c';
        sor3[7] = 'b';
        sor3[8] = 'V';
        sor3[9] = 'e';
        sor3[10] = 'f';
        sor3[11] = ' ';
        sor3[12] = 'a';
        sor3[13] = '3';
        sor3[14] = ' ';
        sor3[15] = '3';
        sor3[16] = '3';
        sor3[17] = '3';
        sor3[18] = '3';
        sor3[19] = 0;

        // hQ    Ni^iPW x[PWi|
        // hQ  |
        //   Ni|
        //^iPW|
        //  x[P|
        // Wi|
        char sor4[30];
        sor4[0] = 'h';
        sor4[1] = 'Q';
        sor4[2] = ' ';
        sor4[3] = ' ';
        sor4[4] = ' ';
        sor4[5] = ' ';
        sor4[6] = 'N';
        sor4[7] = 'i';
        sor4[8] = '^';
        sor4[9] = 'i';
        sor4[10] = 'P';
        sor4[11] = 'W';
        sor4[12] = ' ';
        sor4[13] = 'x';
        sor4[14] = '[';
        sor4[15] = 'P';
        sor4[16] = 'W';
        sor4[17] = 'i';
        sor4[18] = 0;

        konvoda(sor1);
        konvoda(sor2);
        konvoda(sor3);
        konvoda(sor4);
        sor3[15] = '1';
        sor2[11] = 'n';
        sor3[16] = '9';
        sor3[17] = '9';
        sor3[13] = 'n';
        sor3[18] = '7';

        menu_pic szl;
        szl.add_line(sor1, 160, 3 * SM);
        szl.add_line(sor2, 160, 5 * SM);
        szl.add_line(sor3, 160, 7 * SM);
        szl.add_line(sor4, 160, 9 * SM);

        // Korny->pabc_saveplay->writekozep( Korny->picbuffer, 160, 3*SM, sor1 );
        // Korny->pabc_saveplay->writekozep( Korny->picbuffer, 160, 5*SM, sor2 );
        // Korny->pabc_saveplay->writekozep( Korny->picbuffer, 160, 7*SM, sor3 );
        // Korny->pabc_saveplay->writekozep( Korny->picbuffer, 160, 9*SM, sor4 );

        konvback(sor1);
        konvback(sor2);
        konvback(sor3);
        konvback(sor4);

        mk_emptychar();
        while (1) {
            if (mk_kbhit()) {
                int c = mk_getextchar();
                if (c == MK_ESC) {
                    return;
                }
            }
            szl.render();
        }
    }
    strcat(tmp, ".rec");
    saverecek(tmp, belyeg, State->flag_tag);
}

// Idot beirja tablaba, ha kell:
void idoelintezes(long ido, char* valasz, int level, const char* filenev) {
    int extlev = 0;
    if (filenev) {
        extlev = 1;
    }

    valasz[0] = 0;
    if (ido <= 0) {
        // Nem fejeztek be:
        // Azt jelzi menu_nav::navigate-nak, hogy kozepre tegye:
        strcpy(valasz, "*$$^&|@You Failed to Finish!");
        if (MeghalteloszorAB == 1) {
            strcat(valasz, "    (A died first)");
        }
        if (MeghalteloszorAB == 2) {
            strcat(valasz, "    (B died first)");
        }
        return;
    }

    // Befejeztek, kiirjuk idot:

    if (!extlev) {
        State->reload_toptens();
    }

    char tmp[10];
    centiseconds_to_string(ido, tmp);
    if (Single) {
        sprintf(valasz, "*$$^&|@%s", tmp);
    } else {
        if (Aerintetteviragot) {
            sprintf(valasz, "*$$^&|@A:   %s", tmp);
        } else {
            sprintf(valasz, "*$$^&|@B:   %s", tmp);
        }
    }

    // Kivalasztjuk hogy single, vagy multiplayer idok-e:
    topten_set* pketfeleido;
    if (extlev) {
        pketfeleido = &Ptop->idok;
    } else {
        pketfeleido = &State->toptens[level];
    }

    topten* pidok = &pketfeleido->single;
    if (!Single) {
        pidok = &pketfeleido->multi;
    }

    if (pidok->times_count == MAX_TIMES && pidok->times[MAX_TIMES - 1] < ido) {
        return;
    }

    // Felkerult tablara!
    if (pidok->times_count == 0) {
        // Elso ido:
        pidok->times_count = 1;
        pidok->times[0] = ido;
        strcpy(pidok->names1[0], State->player1);
        strcpy(pidok->names2[0], State->player2);
        strcat(valasz, "     Best Time!");
        if (extlev) {
            Ptop->saveidok(filenev);
        }
        return;
    }
    // Tablan van mar mas ido is:
    // Uzenetek eldontese:
    int elozott = 0;
    if (pidok->times[unsigned(pidok->times_count - 1)] > ido) {
        // Vkit leelozott:
        elozott = 1;
    }
    if (pidok->times[0] > ido) {
        // Legjobb ido!:
        strcat(valasz, "     Best Time!");
    } else {
        if (elozott) {
            strcat(valasz, "     You Made the Top Ten");
        }
    }
    // Beillesztjuk uj idot vegere!:
    if (pidok->times_count == MAX_TIMES) {
        pidok->times[MAX_TIMES - 1] = ido;
        strcpy(pidok->names1[MAX_TIMES - 1], State->player1);
        strcpy(pidok->names2[MAX_TIMES - 1], State->player2);
    } else {
        pidok->times[int(pidok->times_count)] = ido;
        strcpy(pidok->names1[int(pidok->times_count)], State->player1);
        strcpy(pidok->names2[int(pidok->times_count)], State->player2);
        pidok->times_count++;
    }
    // Sorrendbe allitjuk idoket:
    for (int i = 0; i < MAX_TIMES + 1; i++) {
        for (int j = 0; j < pidok->times_count - 1; j++) {
            if (pidok->times[j] > pidok->times[j + 1]) {
                // Csere:
                long tmp = pidok->times[j];
                pidok->times[j] = pidok->times[j + 1];
                pidok->times[j + 1] = tmp;

                player_name tmpnev;
                strcpy(tmpnev, pidok->names1[j]);
                strcpy(pidok->names1[j], pidok->names1[j + 1]);
                strcpy(pidok->names1[j + 1], tmpnev);

                strcpy(tmpnev, pidok->names2[j]);
                strcpy(pidok->names2[j], pidok->names2[j + 1]);
                strcpy(pidok->names2[j + 1], tmpnev);
            }
        }
    }

    if (extlev) {
        Ptop->saveidok(filenev);
    }
}

void replayprec(void) {
    floadlevel_p(Prec1->palyanev);
    int elso = 1;
    while (1) {
        Prec1->rewind();
        Prec2->rewind();
        if (lejatszo_r(Prec1->palyanev, !elso)) {
            if (Ptop->kerekekfejjellefele) {
                hiba("replayprec-ben Ptop->kerekekfejjellefel!");
            }
            return;
        }
        elso = 0;
    }
}

// A jelenleg beolvasott palyan jatszik vissza:
// Azt a nevet kerik, amit topol::topol kap:
// Ezenlegjobbido beallitasahoz kell neki:
void replayjelenlegi(const char* filenev) {
    int elso = 1;
    while (1) {
        Prec1->rewind();
        Prec2->rewind();
        if (lejatszo_r(filenev, !elso)) {
            if (Ptop->kerekekfejjellefele) {
                hiba("replayjelenlegi-ben Ptop->kerekekfejjellefel!");
            }
            return;
        }
        if (Ptop->kerekekfejjellefele) {
            hiba("replayjelenlegi-ben Ptop->kerekekfejjellefel!");
        }
        elso = 0;
    }
}

static text_line Idoszovegek[14];

// 0-ESC, 1-this level, 2-next level, 3-skip level
// Ha filenev nem NULL, akkor external file:
int afterplay(int level, int nextdefault, const char* idoeredmeny, const char* filenev) {
    int extlev = 0;
    if (filenev) {
        extlev = 1;
    }

    player* pjatekosa = State->get_player(State->player1);
    player* pjatekosb = State->get_player(State->player2);

    // next, skip, kurrens beallitasa:
    int nextisvan = 0;
    int skipisvan = 0;
    int kurrens = 0;
    if (Single && !extlev) {
        if (pjatekosa->levels_completed > level && level < INTERNAL_LEVEL_COUNT - 1) {
            nextisvan = 1;
        }
        if (!nextisvan && level < INTERNAL_LEVEL_COUNT - 1) {
            skipisvan = 1;
        }
        kurrens = nextdefault;
        // Ellenorzes:
        if (nextdefault && !nextisvan) {
            hiba("nextdefault && !nextisvan!");
        }
        if (skipisvan && nextisvan) {
            hiba("skipisvan && nextisvan!");
        }
    }

    while (1) {
        char fejlec[100] = "";
        if (extlev) {
            if (strlen(Ptop->levelname) > LEVELNAMEHOSSZ) {
                hiba("67834767");
            }
            sprintf(fejlec, "External: %s", Ptop->levelname);
            if (MenuFont->len(fejlec) > 630) {
                sprintf(fejlec, "Ext: %s", Ptop->levelname);
            }
        } else {
            strcpy(fejlec, "Level ");
            char tmp[10];
            itoa(level + 1, tmp, 10);
            strcat(fejlec, tmp);
            strcat(fejlec, ": ");
            strcat(fejlec, getleveldescription(level));
        }

        if (!Single && Tag) {
            if (Aleszfogo) {
                sprintf(Idoszovegek[0].text, "*$$^&|@A start with the flag next.");
            } else {
                sprintf(Idoszovegek[0].text, "*$$^&|@B start with the flag next.");
            }
        } else {
            strcpy(Idoszovegek[0].text, idoeredmeny);
        }
        Idoszovegek[0].x = 320;
        if (Single) {
            Idoszovegek[0].y = 370;
        } else {
            Idoszovegek[0].y = 314;
        }

        int idoszovegszam = 1;

        if (Single) {
        } else {
            int tagisvan = 0;
            if (Tag) {
                tagisvan = 1;
            }

            int dx = 0;
            if (!tagisvan) {
                dx = 100; // Ha nem kell fogoidot irni, mindent jobbra tolunk
            }

            int ya = 380;
            int yb = 415;
            int hosszunevek = 0;
            if (MenuFont->len(pjatekosa->name) > 160 || MenuFont->len(pjatekosb->name) > 160) {
                hosszunevek = 1;
            }
            // A jatekos:
            if (hosszunevek) {
                sprintf(Idoszovegek[1].text, "Player A: %s", pjatekosa->name);
            } else {
                sprintf(Idoszovegek[1].text, "Player A:     %s", pjatekosa->name);
            }
            Idoszovegek[1].x = 10 + dx;
            Idoszovegek[1].y = ya;

            sprintf(Idoszovegek[2].text, "%d", Motor1->apple_count);
            Idoszovegek[2].x = 380 + dx;
            Idoszovegek[2].y = ya;

            if (tagisvan) {
                long longido = Afogoido * Idod2lszorzo;
                centiseconds_to_string(longido, Idoszovegek[3].text);
                Idoszovegek[3].x = 440 + dx;
                Idoszovegek[3].y = ya;
            } else {
                strcpy(Idoszovegek[3].text, " ");
                Idoszovegek[3].x = 100 + dx;
                Idoszovegek[3].y = 100;
            }

            // B jatekos:
            if (hosszunevek) {
                sprintf(Idoszovegek[4].text, "Player B: %s", pjatekosb->name);
            } else {
                sprintf(Idoszovegek[4].text, "Player B:     %s", pjatekosb->name);
            }
            Idoszovegek[4].x = 10 + dx;
            Idoszovegek[4].y = yb;

            sprintf(Idoszovegek[5].text, "%d", Motor2->apple_count);
            Idoszovegek[5].x = 380 + dx;
            Idoszovegek[5].y = yb;

            if (tagisvan) {
                long longido = Bfogoido * Idod2lszorzo;
                centiseconds_to_string(longido, Idoszovegek[6].text);
                Idoszovegek[6].x = 440 + dx;
                Idoszovegek[6].y = yb;
            }
            if (tagisvan) {
                idoszovegszam += 6;
            } else {
                idoszovegszam += 5;
            }

            if (hosszunevek) {
                Idoszovegek[2].x += 40;
                Idoszovegek[3].x += 40;
                Idoszovegek[5].x += 40;
                Idoszovegek[6].x += 40;
            }
        }

        // Valasztas:
        menu_nav val;
        // val.szam = 5+nextisvan+skipisvan;
        val.selected_index = kurrens;
        val.max_visible_entries = 6;
        val.x_left = 230;
        if (Single) {
            val.y_entries = 110;
            val.dy = 42;
        } else {
            val.y_entries = 110;
            val.dy = 42;
        }
        val.enable_esc = true;
        strcpy(val.title, fejlec);

        if (nextisvan || skipisvan) {
            strcpy(NavEntriesLeft[0], "Play again");
            if (nextisvan) {
                strcpy(NavEntriesLeft[1], "Play next");
            } else {
                strcpy(NavEntriesLeft[1], "Skip level");
            }
            strcpy(NavEntriesLeft[2], "Replay");
            strcpy(NavEntriesLeft[3], "Save play");
            strcpy(NavEntriesLeft[4], "Best times");
            // strcpy( NavEntriesLeft[5], "Main menu" );
        } else {
            strcpy(NavEntriesLeft[0], "Play again");
            strcpy(NavEntriesLeft[1], "Replay");
            strcpy(NavEntriesLeft[2], "Save play");
            strcpy(NavEntriesLeft[3], "Best times");
            // strcpy( NavEntriesLeft[4], "Main menu" );
        }

        val.setup(4 + nextisvan + skipisvan);

        // Idoszoveg helyere beirjuk ecset-ben levok lefoglaltak szamat:
        // sprintf( Idoszovegek[0].text, "%d", Osszegszam );

        int eredmeny = val.navigate(Idoszovegek, idoszovegszam);
        kurrens = eredmeny;
        // eredmeny-t nextisvan esetere konvertalja:
        if (eredmeny > 0 && !nextisvan && !skipisvan) {
            eredmeny++;
        }

        if (eredmeny < 0 /*|| eredmeny == 5*/) {
            return 0;
        }
        if (eredmeny == 0) {
            return 1;
        }
        if (eredmeny == 1) {
            // Next, vagy skip:
            if (!nextisvan && !skipisvan) {
                hiba("Itt nem kene lennie (!nextisvan && !skipisvan)!");
            }
            if (nextisvan) {
                return 2;
            }
            // SKIP:
            if (skippelheto(level)) {
                return 3;
            }
        }
        if (eredmeny == 2) {
            replayprec();
            MenuPal->set();
        }
        if (eredmeny == 3) {
            if (level != INTERNAL_LEVEL_COUNT - 1) { // Utolso palyat nem lehet menteni
                saveplay(Ptop->belyeg);
            }
        }
        if (eredmeny == 4) {
            if (extlev) {
                menu_external_topten(Ptop, Single);
            } else {
                menu_internal_topten(level, Single);
            }
        }
    }
}

void kiirloading(void) {
    menu_pic szovlist;
    szovlist.clear();
    szovlist.add_line_centered("Loading", 320, 230);
    szovlist.render(true); // Golyok nelkul
}

static void playlevel(int level) {
    player* pjatekos = State->get_player(State->player1);
    while (1) {
        char tmpnev[20];
        if (level + 1 < 10) {
            sprintf(tmpnev, "QWQUU00%d.LEV", level + 1);
        } else {
            if (level + 1 < 100) {
                sprintf(tmpnev, "QWQUU0%d.LEV", level + 1);
            } else {
                sprintf(tmpnev, "QWQUU%d.LEV", level + 1);
            }
        }
        kiirloading();
        floadlevel_p(tmpnev);

        Prec1->erase(tmpnev);
        Prec2->erase(tmpnev);
        long ido = lejatszo(tmpnev);
        MenuPal->set();
        if (Ptop->kerekekfejjellefele) {
            hiba("playlevel-ben Ptop->kerekekfejjellefel!");
        }
        char valasz[100] = "";
        idoelintezes(ido, valasz, level, NULL);

        int ujpalya = 0;
        if (ido > 0) {
            // Sikeres volt szint:
            pjatekos->skipped[level] = 0;
            if (pjatekos->levels_completed == level && Single) {
                // Csak SINGLE jatek alatt novelheto levels_completed!!!
                // Novelte eggyel sikeres palyak szamat:
                pjatekos->levels_completed++;
                if (pjatekos->levels_completed < INTERNAL_LEVEL_COUNT) {
                    ujpalya = 1;
                } else {
                    // Most eloszor sikeresen teljesitette utolso palyat:
                }
            }
            // idoelintezes ido > 0 es nem external eseten reload-olja:
            State->save();
        }
        int eredmeny = afterplay(level, ujpalya, valasz, NULL);
        Prec1->erase(tmpnev);
        Prec2->erase(tmpnev);
        if (eredmeny == 0) {
            pjatekos->selected_level = level + ujpalya;
            return;
        }
        if (eredmeny == 2) {
            level++;
            pjatekos->selected_level = level;
        }
        if (eredmeny == 3) {
            // Skip level:
            if (level != pjatekos->levels_completed) {
                hiba("level != pjatekos->levels_completed! 674");
            }
            pjatekos->skipped[level] = 1;
            level++;
            pjatekos->selected_level = level;
            pjatekos->levels_completed++;
            State->reload_toptens();
            State->save();
        }
    }
}

void play(void) {
    while (1) {
        player* pjatekos = State->get_player(State->player1);
        int kirpalyszam = pjatekos->levels_completed + 1;
        if (!State->single) {
            // Ha ketten jatszanak, akkor a nagyobb sikeru szamit:
            player* pjatekos2 = State->get_player(State->player2);
            if (kirpalyszam < pjatekos2->levels_completed + 1) {
                kirpalyszam = pjatekos2->levels_completed + 1;
            }
        }

        if (kirpalyszam > INTERNAL_LEVEL_COUNT) {
            kirpalyszam = INTERNAL_LEVEL_COUNT;
        }

        // Valasztas:
        menu_nav val;
        val.selected_index = int(pjatekos->selected_level) + 1;
        val.max_visible_entries = LISTegykepen;
        val.x_left = LISTx0;
        val.y_entries = LISTy0;
        val.dy = LISTdy;
        val.enable_esc = true;
        strcpy(val.title, "Select Level!");

        strcpy(NavEntriesLeft[0], "External File");
        for (int i = 0; i < kirpalyszam; i++) {
            char tmp[NAV_ENTRY_TEXT_MAX_LENGTH];
            itoa(i + 1, tmp, 10);
            strcat(tmp, " ");
            if (pjatekos->skipped[i]) {
                strcat(tmp, "SKIPPED!");
            } else {
                strcat(tmp, getleveldescription(i));
                // strcat( tmp, "Nincs level nev!" );
            }
            if (strlen(tmp) > NAV_ENTRY_TEXT_MAX_LENGTH - 5) {
                hiba("play-ben! strlen( tmp ) > NAV_ENTRY_TEXT_MAX_LENGTH-5!");
            }
            strcpy(NavEntriesLeft[i + 1], tmp);
        }

        val.setup(kirpalyszam + 1);
        int eredmeny = val.navigate();

        if (eredmeny < 0) {
            return;
        }

        if (eredmeny == 0) {
            // External file lejatszast valasztotta, ilyenkor ha visszajon
            // folytatodik tovabb:
            pjatekos->selected_level = -1;
            void playextmenu(void);
            playextmenu();
        } else {
            // Egy normal level-t valasztott ki:
            playlevel(eredmeny - 1);
            // return;
        }
    }
}
