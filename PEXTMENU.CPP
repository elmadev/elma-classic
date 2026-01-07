#include "ALL.H"
#include "LOAD.H"
#include "menu_nav.h"
#include "menu_pic.h"

/*static egyszovsor Idoszovegek[4];

// Igaz, ha ujra jatszik, hamis ha listat ker:
static int afterplayext( char* filenev, long ido ) {
    int kurrens = 0;
    while( 1 ) {
        // Ido feliratot keszitunk:
        char idoszov1[30] = "", idoszov2[30] = "";
        if( ido < 0 ) {
            strcpy( idoszov1, "You Failed" );
            strcpy( idoszov2, " to Finish!" );
        }
        else {
            strcpy( idoszov1, "Your time:" );
            centiseconds_to_string( ido, idoszov2 );
        }

        //Korny->pabc_afterplaymenu->write( Korny->picbuffer, 200, 104, idoszov1 );
        strcpy( Idoszovegek[0].text, idoszov1 );
        Idoszovegek[0].x = 200;
        Idoszovegek[0].y = 104;
        //Korny->pabc_afterplaymenu->write( Korny->picbuffer, 200, 134, idoszov2 );
        strcpy( Idoszovegek[1].text, idoszov2 );
        Idoszovegek[1].x = 200;
        Idoszovegek[1].y = 134;

        // Kiirjuk ha lockolt:
        if( Ptop->lezart ) {
            //Korny->pabc_afterplaymenu->write( Korny->picbuffer,
            //		9, 173, "Locked" );
            strcpy( Idoszovegek[2].text, "Locked" );
            Idoszovegek[2].x = 9;
            Idoszovegek[2].y = 173;
        }

        //Korny->pal_afterplaymenu->set();
        // Valasztas:
        menu_nav val;
        val.selected_index = kurrens;
        val.x_left = 50;
        val.y_entries = 40;
        val.dy = 25;
        char* levdesc = getlevelname( filenev );
        if( levdesc )
            strcpy( val.title, levdesc );
        else
            strcpy( val.title, "External level" );

        strcpy( NavEntriesLeft[0], "Play again" );
        strcpy( NavEntriesLeft[1], "Replay" );
        strcpy( NavEntriesLeft[2], "Save play" );
        strcpy( NavEntriesLeft[3], "File list" );

        // Idoszoveg helyere beirjuk ecset-ben levok lefoglaltak szamat:
        //sprintf( Idoszovegek[0].text, "%d", Osszegszam );

        val.setup( 4 );
        int eredmeny = 0;
        if( Ptop->lezart )
            eredmeny = val.navigate( Idoszovegek, 3 );
        else
            eredmeny = val.navigate( Idoszovegek, 2 );
        kurrens = eredmeny;

        if( eredmeny < 0 || eredmeny == 3 )
            return 0; // Lista
        if( eredmeny == 0 )
            return 1; // Play again
        if( eredmeny == 1 ) {
            replayjelenlegi(); // Replay
            MenuPal->set();
            continue;
        }
        if( eredmeny == 2 ) {
            unsigned long belyeg = getbelyeg( filenev );
            saveplay( belyeg ); // Save play
            continue;
        }
        internal_error( "Itt nem szabadna lennie (afterplayext)!" );
    }
} */

int abcbenelobb(const char* text1, const char* text2);

static void abcberendez(int szam) {
    for (int i = 0; i < szam + 3; i++) {
        for (int j = 0; j < szam - 1; j++) {
            // Ha j es j+1 edik elem forditva van visszacsereljuk oket:
            char* nev1 = NavEntriesLeft[j];
            char* nev2 = NavEntriesLeft[j + 1];
            if (abcbenelobb(nev2, nev1) > 0) {
                // Csere:
                char tmp[20];
                strcpy(tmp, nev1);
                strcpy(nev1, nev2);
                strcpy(nev2, tmp);
            }
        }
    }
}

// Nincs h-ja, hanem prototype ki van irva play.cpp-ben:
void playextmenu(void) {
    // Kiirjuk osszes lev file nevet ide:
    finame fname;
    bool done = find_first("lev/*.lev", fname);
    int szam = 0;
    while (!done) {
        char* filenev = fname;

        strcpy(NavEntriesLeft[szam], filenev);

        done = find_next(fname);
        szam++;
        if (szam >= NavEntriesLeftMaxLength - 4) {
            done = true;
        }
    }

    find_close();

    if (szam <= 0) {
        return;
    }

    abcberendez(szam);

    int kezdo = 0; // Ha megtalalja eddigit, akkor kur-t raallitja
    for (int i = 0; i < szam; i++) {
        char tmp[50];
        strcpy(tmp, NavEntriesLeft[i]);
        // strcat( tmp, ".lev" );
        if (strcmpi(tmp, State->external_filename) == 0) {
            kezdo = i;
        }
    }

    // Feltoltottuk neveket, jon valasztas:
    menu_nav val;
    val.selected_index = kezdo;
    strcpy(val.title, "Select External File!");

    val.setup(szam);

    while (1) {
        int eredmeny = val.navigate();
        if (eredmeny < 0) {
            return;
        }

        char filenev[20];
        if (strlen(val.entries_left[eredmeny]) > 15) {
            internal_error("playextmenu-ben val.entries_left[eredmeny] ) > 15!");
        }
        strcpy(filenev, val.entries_left[eredmeny]);

        strcpy(State->external_filename, filenev);

        while (1) {
            kiirloading();
            // Ha topologiailag hibas lev file (uzenetet load adja):
            if (!floadlevel_p(filenev)) {
                break;
            }
            Prec1->erase(filenev);
            Prec2->erase(filenev);
            int ido = lejatszo(filenev);
            MenuPal->set();
            char valasz[100] = "";
            idoelintezes(ido, valasz, 0, filenev);
            // Igaz, ha ujra jatszik, hamis ha listat ker:
            if (!afterplay(0, 0, valasz, filenev)) {
                Prec1->erase(filenev);
                Prec2->erase(filenev);
                break;
            }
        }
    }
}
