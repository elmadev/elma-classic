#include "ALL.H"

/*void showinstruct( void ) {
    blit8( Korny->picbuffer, Korny->ppic_help );

    int x1 = 160;
    int yo = 4;
    int dy = 24;

    Korny->pabc_help->writekozep( Korny->picbuffer, x1, yo,      "After you have eaten all the");
    Korny->pabc_help->writekozep( Korny->picbuffer, x1, yo+dy,   "foods, touch the flower!" );
    Korny->pabc_help->writekozep( Korny->picbuffer, x1, yo+dy*2, "" );
    Korny->pabc_help->writekozep( Korny->picbuffer, x1, yo+dy*3, "Use the four arrow keys and" );
    Korny->pabc_help->writekozep( Korny->picbuffer, x1, yo+dy*4, "the space bar!" );
    Korny->pabc_help->writekozep( Korny->picbuffer, x1, yo+dy*5, "" );
    Korny->pabc_help->writekozep( Korny->picbuffer, x1, yo+dy*6, "For more info, go to help" );
    Korny->pabc_help->writekozep( Korny->picbuffer, x1, yo+dy*7, "from main menu!" );

    lassufizre( Korny->picbuffer, Korny->pal_help );
    int done = 0;
    mk_emptychar();
    while( !done ) {
        mk_getextchar();
        //if( c == MK_ESC || c == MK_ENTER )
        done = 1;
    }
} */

// Ha text1 elobb van abc-ben, igazzal ter vissza:
int abcbenelobb(const char* text1, const char* text2) {
    // Elso betunel bebillennek ha csak kis/nagy-ban ter el:
    int elobbnagy1 = 0;
    int elobbnagy2 = 0;
    while (1) {
        char betu1 = *text1;
        char betu2 = *text2;

        if (!betu1 && !betu2) {
            // Betuk mind stimmeltek, talan kis/nagybetu nem:
            if (elobbnagy2) {
                return 0;
            }
            return 1;
        }

        if (!betu1) {
            return 1;
        }
        if (!betu2) {
            return 0;
        }

        if (betu1 >= 'a') {
            betu1 -= 'a' - 'A';
        }
        if (betu2 >= 'a') {
            betu2 -= 'a' - 'A';
        }
        if (betu1 == betu2) {
            if (!elobbnagy1 && !elobbnagy2) {
                if (*text1 < *text2) {
                    elobbnagy1 = 1;
                }
                if (*text2 < *text1) {
                    elobbnagy2 = 1;
                }
            }
        } else {
            // hiba( "Itt van!" );
            if (betu1 < betu2) {
                return 1;
            }
            if (betu2 < betu1) {
                return 0;
            }
        }

        text1++;
        text2++;
    }
}

static int Megnemrand = 1;

static unsigned int veletlen(void) {
    // Veletlenszeru lejatszas:
    if (Megnemrand) {
        Megnemrand = 0;
        srand((unsigned)clock());
    }

    unsigned int result = 0;
    for (int i = 0; i < 4; i++) {
        result = (result << 8) | (rand() & 0xFF);
    }
    return result;
}

void replay(void) {
    strcpy(NavEntriesLeft[0], "Randomizer");

    finame fname;
    int done = fifirst("rec/*.rec", fname);
    // Ez mindig eggyel tobb, mint valosag, mivel az elso
    // rubrika mindig 'Randomizer':
    int szamuk = 1;
    while (!done && szamuk < NavEntriesLeftMaxLength) {
        char* nev = NavEntriesLeft[szamuk];
        strcpy(nev, fname);
        // Leveszi kiterjesztest:
        int i = 0;
        while (nev[i] != '.') {
            if (!nev[i]) {
                hiba("replay-ban nincs pont nevben!: ", nev);
            }
            i++;
        }
        nev[i] = 0;

        // Kisbetusit:
        /*int j = 0;
        while( nev[j] ) {
            if( nev[j] >= 'A' && nev[j] <= 'Z' )
                nev[j] -= 'A'-'a';
            j++;
        }
        if( strncmp( nev, "r", 1 ) == 0 )
            nev[0] = 'R';
            //strcpy( nev, "R" );
        */

        done = finext(fname);
        szamuk++;
        if (szamuk >= NavEntriesLeftMaxLength - 4) {
            done = 1;
        }
    }
    ficlose();

    if (szamuk < 2) {
        return;
    }

    // ABC sorrendbe rendez:
    for (int i = 0; i < szamuk + 1; i++) {
        for (int j = 1; j < szamuk - 1; j++) {
            if (abcbenelobb(NavEntriesLeft[j + 1], NavEntriesLeft[j])) {
                // Csere:
                char tmp[100];
                strcpy(tmp, NavEntriesLeft[j]);
                strcpy(NavEntriesLeft[j], NavEntriesLeft[j + 1]);
                strcpy(NavEntriesLeft[j + 1], tmp);
            }
        }
    }

    menu_nav val;
    val.selected_index = 0;
    strcpy(val.title, "Select replay file!");

    val.setup(szamuk);

    while (1) {
        // Valasztas:
        int kurrens = val.navigate();

        if (kurrens < 0) { // ESC
            return;
        }

        if (kurrens == 0) {
            // RANDOMIZER:
            // 0-tol kezdve szamolunk:
            int elozopl = -1;
            int elozoelottipl = -1;
            while (1) {
                int pl = veletlen() % (szamuk - 1);
                while ((pl == elozopl && szamuk > 2) || (pl == elozoelottipl && szamuk > 3)) {
                    pl = veletlen() % (szamuk - 1);
                }
                elozoelottipl = elozopl;
                elozopl = pl;
                // Nev eloallitasa:
                char tmp[30];
                strcpy(tmp, NavEntriesLeft[pl + 1]);
                strcat(tmp, ".REC");
                // 0 param azt jelenti, hogy nem resource filebol olvas:
                MenuPal->set();
                kiirloading();
                long belyeg = loadrecek(tmp, 0);
                if (access_topol(Prec1->palyanev) != 0) {
                    int c = menu_dialog("Cannot find the lev file that corresponds",
                                        "to the record file!", tmp, Prec1->palyanev);
                    if (c == MK_ESC) {
                        return;
                    }
                } else {
                    floadlevel_p(Prec1->palyanev);
                    if (Ptop->belyeg != belyeg) {
                        int c = menu_dialog("The level file has changed since the",
                                            "saving of the record file!", tmp, Prec1->palyanev);
                        if (c == MK_ESC) {
                            return;
                        }
                    } else {
                        Prec1->rewind();
                        Prec2->rewind();
                        if (lejatszo_r(Prec1->palyanev, 0)) {
                            break;
                        }
                    }
                }
            }
        } else {
            // Egy rec file lejatszasa:
            char tmp[30];
            strcpy(tmp, NavEntriesLeft[kurrens]);
            strcat(tmp, ".REC");
            kiirloading();
            long belyeg = loadrecek(tmp, 0);
            if (CtrlAltPressed) {
                // loadrecek beallitotta Multirec-et.
                int ido = Prec1->betoltve;
                if (Multirec && Prec2->betoltve > ido) {
                    ido = Prec2->betoltve;
                }

                ido *= 3.3333333333333;
                ido -= 2;
                if (ido < 1) {
                    ido = 1;
                }

                char idostr[25];
                centiseconds_to_string(ido, idostr);
                strcat(idostr, "    +- 0.01 sec");
                menu_dialog(tmp, "The time of this replay file is:", idostr);
                continue;
            }
            if (access_topol(Prec1->palyanev) != 0) {
                menu_dialog("Cannot find the lev file that corresponds", "to the record file!", tmp,
                            Prec1->palyanev);
            } else {
                floadlevel_p(Prec1->palyanev);
                if (Ptop->belyeg != belyeg) {
                    menu_dialog("The level file has changed since the",
                                "saving of the record file!", tmp, Prec1->palyanev);
                } else {
                    replayjelenlegi(Prec1->palyanev);
                }
            }
        }
        MenuPal->set();
    }
}

static void demo(void) {
    int demoszam = 3;
    char demonevek[14][14] = {"demor1.rec", "demor2.rec", "demor3.rec"};

    int elozodemo = -1;

    while (1) {
        int demo = veletlen() % demoszam;
        while (demo == elozodemo) {
            demo = veletlen() % demoszam;
        }
        elozodemo = demo;

        kiirloading();
        long belyeg = loadrecek(demonevek[demo], 1);
        if (access_topol(Prec1->palyanev) != 0) {
            hiba("783654");
        }
        floadlevel_p(Prec1->palyanev);
        if (Ptop->belyeg != belyeg) {
            hiba("6734654");
        }
        Prec1->rewind();
        Prec2->rewind();
        if (lejatszo_r(Prec1->palyanev, 0)) {
            MenuPal->set();
            return;
        }
        MenuPal->set();
    }
}

static void kilephakell(void) {
    menu_nav val;
    val.selected_index = 0;
    val.x_left = 300;
    val.y_entries = 200;
    val.dy = 40;
    val.y_title = 50; // ezt nem szokas allitani egyebkent
    val.enable_esc = false;
    strcpy(val.title, "Do you want to quit?");

    strcpy(NavEntriesLeft[0], "Yes");
    strcpy(NavEntriesLeft[1], "No");

    val.setup(2);

    int eredmeny = val.navigate();

    if (eredmeny == 0) {
        // Ezt csak watmment.exe-ben szabad kikomentezni:
        State->reload_toptens();

        State->save();
        State->write_stats();
        kilep();
    }
}

void mainmenu(void) {
    MenuPal->set();

    // State->save_aaa(); ide minek kene?
    menu_nav val;
    val.selected_index = 0;
    val.x_left = 200;
    val.y_entries = 100;
    val.dy = 50;
    strcpy(val.title, "Main Menu");

    strcpy(NavEntriesLeft[0], "Play");
    strcpy(NavEntriesLeft[1], "Replay");
    strcpy(NavEntriesLeft[2], "Demo");
    strcpy(NavEntriesLeft[3], "Options");
    strcpy(NavEntriesLeft[4], "Help");
    strcpy(NavEntriesLeft[5], "Best Times");
    strcpy(NavEntriesLeft[6], "Editor");

    val.setup(7);

    while (1) {
        // Korny->pabc_fomenu->write( Korny->picbuffer, 70, 10, "Main menu" );

        // MenuPal->set(); // Torlendo

        int eredmeny = val.navigate();

        // Kimenti menut pcx file-ba:
        // BufferMain->save( "menu.pcx", Menupaltomb );
        // uzenet( "Menut mentett menu.pcx file-ba!" );

        if (eredmeny == 0) {
            play();
        }

        if (eredmeny == 1) {
            replay();
        }

        if (eredmeny == 2) {
            demo();
        }

        if (eredmeny == 3) {
            options();
        }

        if (eredmeny == 4) {
            help();
        }

        if (eredmeny == 5) {
            menu_best_times();
        }

        if (eredmeny == 6) {
            Editorban_dialnak = 1;
            hide_cursor();
            editor();
            show_cursor();
            Editorban_dialnak = 0;
            MenuPal->set();
        }

        if (eredmeny == -1) {
            // State->save_aaa(); kilep majd menti
            kilephakell();
        }
    }
}
