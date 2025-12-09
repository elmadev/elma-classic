#include "all.h"

/*void showinstruct( void ) {
    blt8( Korny->picbuffer, Korny->ppic_help );

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
int abcbenelobb(char* text1, char* text2) {
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

int veletlen(void) {
    // Veletlenszeru lejatszas:
    if (Megnemrand) {
        Megnemrand = 0;
        srand((unsigned)clock());
    }

    int sum = 0;
    for (int i = 0; i < 100; i++) {
        sum += rand();
    }
    while (sum < 0) {
        sum *= 3;
    }
    return sum;
}

void replay(void) {
    strcpy(Rubrikak[0], "Randomizer");

    finame fname;
    int done = fifirst("rec\\*.rec", fname);
    // Ez mindig eggyel tobb, mint valosag, mivel az elso
    // rubrika mindig 'Randomizer':
    int szamuk = 1;
    while (!done && szamuk < Maxrubrikaszam) {
        char* nev = Rubrikak[szamuk];
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
        if (szamuk >= Maxrubrikaszam - 4) {
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
            if (abcbenelobb(Rubrikak[j + 1], Rubrikak[j])) {
                // Csere:
                char tmp[100];
                strcpy(tmp, Rubrikak[j]);
                strcpy(Rubrikak[j], Rubrikak[j + 1]);
                strcpy(Rubrikak[j + 1], tmp);
            }
        }
    }

    valaszt2 val;
    val.kur = 0;
    val.egykepen = LISTegykepen;
    val.x0 = LISTx0;
    val.y0 = LISTy0;
    val.dy = LISTdy;
    val.escelheto = 1;
    strcpy(val.cim, "Select replay file!");

    val.bead(szamuk);

    while (1) {
        // Valasztas:
        RECidokellhet = 1;
        RECidokellis = 0;
        int kurrens = val.valassz();
        RECidokellhet = 0;

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
                    pl = rand() % (szamuk - 1);
                }
                elozoelottipl = elozopl;
                elozopl = pl;
                // Nev eloallitasa:
                char tmp[30];
                strcpy(tmp, Rubrikak[pl + 1]);
                strcat(tmp, ".REC");
                // 0 param azt jelenti, hogy nem resource filebol olvas:
                Pmenupal->set();
                kiirloading();
                long belyeg = loadrecek(tmp, 0);
                if (access_topol(Prec1->palyanev) != 0) {
                    int c = dialog320("Cannot find the lev file that corresponds",
                                      "to the record file!", tmp, Prec1->palyanev);
                    if (c == MK_ESC) {
                        return;
                    }
                } else {
                    floadlevel_p(Prec1->palyanev);
                    if (Ptop->belyeg != belyeg) {
                        int c = dialog320("The level file has changed since the",
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
            strcpy(tmp, Rubrikak[kurrens]);
            strcat(tmp, ".REC");
            kiirloading();
            long belyeg = loadrecek(tmp, 0);
            if (RECidokellis) {
                // loadrecek beallitotta Multirec-et.
                int ido = Prec1->betoltve;
                if (Multirec && Prec2->betoltve > ido) {
                    ido = Prec2->betoltve;
                }

                // Innentol csak akkor nem kikommentez, ha recido direkt ki:
                char tmp8[50];
                sprintf(tmp8, "Ido: %d", ido);
                hiba(tmp8);

                ido *= 3.3333333333333;
                ido -= 2;
                if (ido < 1) {
                    ido = 1;
                }

                char idostr[25];
                ido2string(ido, idostr);
                strcat(idostr, "    +- 0.01 sec");
                dialog320(tmp, "The time of this replay file is:", idostr);
                RECidokellis = 0;
                continue;
            }
            if (access_topol(Prec1->palyanev) != 0) {
                int c = dialog320("Cannot find the lev file that corresponds",
                                  "to the record file!", tmp, Prec1->palyanev);
                // if( c == MK_ESC )
                //	return;
            } else {
                floadlevel_p(Prec1->palyanev);
                if (Ptop->belyeg != belyeg) {

                    ////////////
                    /*char tmp1[10];
                    sprintf( tmp1, "%d", belyeg );
                    char tmp2[10];
                    sprintf( tmp2, "%d", Ptop->belyeg );*/

                    int c = dialog320("The level file has changed since the",
                                      "saving of the record file!", tmp, Prec1->palyanev);
                    // tmp1, tmp2 );
                    // if( c == MK_ESC )
                    //	return;

                } else {
                    replayjelenlegi(Prec1->palyanev);
                }
            }
        }
        Pmenupal->set();
    }
}

static void demo(void) {
    int demoszam = 3;
    char demonevek[14][14] = {"demor1.rec", "demor2.rec", "demor3.rec"};

    int elozodemo = -1;
    int elozoelotti = -1;

    while (1) {
        elozoelotti++;
        int demo = veletlen() % demoszam;
        while (demo == elozodemo) {
            demo = rand() % demoszam;
        }
        elozoelotti = elozodemo;
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
            Pmenupal->set();
            return;
        }
        Pmenupal->set();
    }
}

static void kilephakell(void) {
    valaszt2 val;
    val.kur = 0;
    val.egykepen = 6;
    val.x0 = 300;
    val.y0 = 200;
    val.dy = 40;
    val.cimy = 50; // ezt nem szokas allitani egyebkent
    val.escelheto = 0;
    strcpy(val.cim, "Do you want to quit?");

    strcpy(Rubrikak[0], "Yes");
    strcpy(Rubrikak[1], "No");

    val.bead(2);

    int eredmeny = val.valassz();

    if (eredmeny == 0) {
        // Ezt csak watmment.exe-ben szabad kikomentezni:
        State->reloadidok();

        State->save();
        State->export_();
        kilep();
    }
}

void mainmenu(void) {
    Pmenupal->set();

    // State->save_aaa(); ide minek kene?
    valaszt2 val;
    val.kur = 0;
    val.egykepen = 7;
    val.x0 = 200;
    val.y0 = 100;
    val.dy = 50;
    val.escelheto = 1;
    strcpy(val.cim, "Main Menu");

    strcpy(Rubrikak[0], "Play");
    strcpy(Rubrikak[1], "Replay");
    strcpy(Rubrikak[2], "Demo");
    strcpy(Rubrikak[3], "Options");
    strcpy(Rubrikak[4], "Help");
    strcpy(Rubrikak[5], "Best Times");
    strcpy(Rubrikak[6], "Editor");

    val.bead(7);

    while (1) {
        // Korny->pabc_fomenu->write( Korny->picbuffer, 70, 10, "Main menu" );

        // Pmenupal->set(); // Torlendo

        int eredmeny = val.valassz();

        // Kimenti menut pcx file-ba:
        // Buffsima->save( "menu.pcx", Menupaltomb );
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
            besttimes();
        }

        if (eredmeny == 6) {
            Editorban_dialnak = 1;
            editor();
            Editorban_dialnak = 0;
            Pmenupal->set();
        }

        if (eredmeny == -1) {
            // State->save_aaa(); kilep majd menti
            kilephakell();
        }
    }
}
