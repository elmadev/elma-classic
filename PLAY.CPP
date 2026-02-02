#include "PLAY.H"
#include "abc8.h"
#include "best_times.h"
#include "EDITUJ.H"
#include "flagtag.h"
#include "keys.h"
#include "LEJATSZO.H"
#include "level.h"
#include "LOAD.H"
#include "main.h"
#include "menu_external.h"
#include "menu_nav.h"
#include "menu_pic.h"
#include "platform_impl.h"
#include "platform_utils.h"
#include "physics_init.h"
#include "skip.h"
#include "timer.h"
#include <cstdlib>
#include <cstring>

// Siker eseten igazzal ter vissza:
static int bevesznevet(char* nev) {
    menu_pic szovlist;

    int i = 0;
    empty_keypress_buffer();
    int valtozott = 1;
    nev[0] = 0;
    while (1) {
        while (has_keypress()) {
            Keycode c = get_keypress();
            if (c == KEY_ESC) {
                return 0;
            }
            if (c == KEY_ENTER) {
                // ENTER
                if (i > 0) {
                    return 1;
                }
            }
            if (MenuFont->has_char(c) && is_char_valid_for_filename(c)) {
                if (i >= 8) {
                    continue;
                }
                nev[i] = (char)c;
                nev[i + 1] = 0;
                i++;
                valtozott = 1;
            }
            if (c == KEY_BACKSPACE) {
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

void saveplay(int belyeg) {
    // Korny->pabc_saveplay->write( Korny->picbuffer, 20, 2*SM, "To exit press ESC!" );
    // Korny->pabc_saveplay->write( Korny->picbuffer, 20, 5*SM, "Please enter filename:" );
    // lassufizre( Korny->picbuffer, Korny->pal_saveplay );
    char tmp[20] = "";
    if (!bevesznevet(tmp)) {
        return;
    }
    strcat(tmp, ".rec");
    recorder::save_rec_file(tmp, belyeg, State->flag_tag);
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
        strcpy(valasz, MENU_CENTER_TEXT "You Failed to Finish!");
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
        sprintf(valasz, MENU_CENTER_TEXT "%s", tmp);
    } else {
        if (Aerintetteviragot) {
            sprintf(valasz, MENU_CENTER_TEXT "A:   %s", tmp);
        } else {
            sprintf(valasz, MENU_CENTER_TEXT "B:   %s", tmp);
        }
    }

    // Kivalasztjuk hogy single, vagy multiplayer idok-e:
    topten_set* pketfeleido;
    if (extlev) {
        pketfeleido = &Ptop->toptens;
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
            Ptop->save_topten(filenev);
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
        Ptop->save_topten(filenev);
    }
}

void replayprec(void) {
    floadlevel_p(Rec1->level_filename);
    int elso = 1;
    while (1) {
        Rec1->rewind();
        Rec2->rewind();
        if (lejatszo_r(Rec1->level_filename, !elso)) {
            if (Ptop->objects_flipped) {
                internal_error("replayprec-ben Ptop->kerekekfejjellefel!");
            }
            return;
        }
        elso = 0;
    }
}

// A jelenleg beolvasott palyan jatszik vissza:
// Azt a nevet kerik, amit topol::topol kap:
// BestTime beallitasahoz kell neki:
void replayjelenlegi(const char* filenev) {
    int elso = 1;
    while (1) {
        Rec1->rewind();
        Rec2->rewind();
        if (lejatszo_r(filenev, !elso)) {
            if (Ptop->objects_flipped) {
                internal_error("replayjelenlegi-ben Ptop->kerekekfejjellefel!");
            }
            return;
        }
        if (Ptop->objects_flipped) {
            internal_error("replayjelenlegi-ben Ptop->kerekekfejjellefel!");
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
            internal_error("nextdefault && !nextisvan!");
        }
        if (skipisvan && nextisvan) {
            internal_error("skipisvan && nextisvan!");
        }
    }

    while (1) {
        char fejlec[100] = "";
        if (extlev) {
            if (strlen(Ptop->level_name) > LEVEL_NAME_LENGTH) {
                internal_error("67834767");
            }
            sprintf(fejlec, "External: %s", Ptop->level_name);
            if (MenuFont->len(fejlec) > 630) {
                sprintf(fejlec, "Ext: %s", Ptop->level_name);
            }
        } else {
            strcpy(fejlec, "Level ");
            char tmp[10];
            itoa(level + 1, tmp, 10);
            strcat(fejlec, tmp);
            strcat(fejlec, ": ");
            strcat(fejlec, get_internal_level_name(level));
        }

        if (!Single && Tag) {
            if (FlagTagAStarts) {
                sprintf(Idoszovegek[0].text, MENU_CENTER_TEXT "A start with the flag next.");
            } else {
                sprintf(Idoszovegek[0].text, MENU_CENTER_TEXT "B start with the flag next.");
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
                long longido = FlagTimeA * TimeToCentiseconds;
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
                long longido = FlagTimeB * TimeToCentiseconds;
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
        val.x_left = 230;
        if (Single) {
            val.y_entries = 110;
            val.dy = 42;
        } else {
            val.y_entries = 110;
            val.dy = 42;
        }
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
                internal_error("Itt nem kene lennie (!nextisvan && !skipisvan)!");
            }
            if (nextisvan) {
                return 2;
            }
            // SKIP:
            if (is_skippable(level)) {
                return 3;
            }
        }
        if (eredmeny == 2) {
            replayprec();
            MenuPalette->set();
        }
        if (eredmeny == 3) {
            if (level != INTERNAL_LEVEL_COUNT - 1) { // Utolso palyat nem lehet menteni
                saveplay(Ptop->level_id);
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

        Rec1->erase(tmpnev);
        Rec2->erase(tmpnev);
        long ido = lejatszo(tmpnev, F1Pressed ? CameraMode::MapViewer : CameraMode::Normal);
        MenuPalette->set();
        if (Ptop->objects_flipped) {
            internal_error("playlevel-ben Ptop->kerekekfejjellefel!");
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
                }
            }
            // idoelintezes ido > 0 es nem external eseten reload-olja:
            State->save();
        }
        int eredmeny = afterplay(level, ujpalya, valasz, NULL);
        Rec1->erase(tmpnev);
        Rec2->erase(tmpnev);
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
                internal_error("level != pjatekos->levels_completed! 674");
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
        val.search_pattern = SearchPattern::Internals;
        val.selected_index = int(pjatekos->selected_level) + 1;
        strcpy(val.title, "Select Level!");

        strcpy(NavEntriesLeft[0], "External File");
        for (int i = 0; i < kirpalyszam; i++) {
            char tmp[NAV_ENTRY_TEXT_MAX_LENGTH];
            itoa(i + 1, tmp, 10);
            strcat(tmp, " ");
            if (pjatekos->skipped[i]) {
                strcat(tmp, "SKIPPED!");
            } else {
                strcat(tmp, get_internal_level_name(i));
                // strcat( tmp, "Nincs level nev!" );
            }
            if (strlen(tmp) > NAV_ENTRY_TEXT_MAX_LENGTH - 5) {
                internal_error("play-ben! strlen( tmp ) > NAV_ENTRY_TEXT_MAX_LENGTH-5!");
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
            menu_external_levels();
        } else {
            // Egy normal level-t valasztott ki:
            playlevel(eredmeny - 1);
            // return;
        }
    }
}
