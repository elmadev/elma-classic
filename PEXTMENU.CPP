#include "all.h"

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
            ido2string( ido, idoszov2 );
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
        valaszt2 val;
        val.kur = kurrens;
        val.egykepen = 6;
        val.x0 = 50;
        val.y0 = 40;
        val.dy = 25;
        val.escelheto = 1;
        char* levdesc = getlevelname( filenev );
        if( levdesc )
            strcpy( val.cim, levdesc );
        else
            strcpy( val.cim, "External level" );

        strcpy( Rubrikak[0], "Play again" );
        strcpy( Rubrikak[1], "Replay" );
        strcpy( Rubrikak[2], "Save play" );
        strcpy( Rubrikak[3], "File list" );

        // Idoszoveg helyere beirjuk ecset-ben levok lefoglaltak szamat:
        //sprintf( Idoszovegek[0].text, "%d", Osszegszam );

        val.bead( 4 );
        int eredmeny = 0;
        if( Ptop->lezart )
            eredmeny = val.valassz( Idoszovegek, 3 );
        else
            eredmeny = val.valassz( Idoszovegek, 2 );
        kurrens = eredmeny;

        if( eredmeny < 0 || eredmeny == 3 )
            return 0; // Lista
        if( eredmeny == 0 )
            return 1; // Play again
        if( eredmeny == 1 ) {
            replayjelenlegi(); // Replay
            Pmenupal->set();
            continue;
        }
        if( eredmeny == 2 ) {
            unsigned long belyeg = getbelyeg( filenev );
            saveplay( belyeg ); // Save play
            continue;
        }
        hiba( "Itt nem szabadna lennie (afterplayext)!" );
    }
} */

int abcbenelobb(char* text1, char* text2);

static void abcberendez(int szam) {
    for (int i = 0; i < szam + 3; i++) {
        for (int j = 0; j < szam - 1; j++) {
            // Ha j es j+1 edik elem forditva van visszacsereljuk oket:
            char* nev1 = Rubrikak[j];
            char* nev2 = Rubrikak[j + 1];
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
    int done = fifirst("lev\\*.lev", fname);
    int szam = 0;
    while (!done) {
        char* filenev = fname;

        strcpy(Rubrikak[szam], filenev);

        done = finext(fname);
        szam++;
        if (szam >= Maxrubrikaszam - 4) {
            done = 1;
        }
    }

    ficlose();

    if (szam <= 0) {
        return;
    }

    abcberendez(szam);

    int kezdo = 0; // Ha megtalalja eddigit, akkor kur-t raallitja
    for (int i = 0; i < szam; i++) {
        char tmp[50];
        strcpy(tmp, Rubrikak[i]);
        // strcat( tmp, ".lev" );
        if (strcmpi(tmp, State->playextfilenev) == 0) {
            kezdo = i;
        }
    }

    // Feltoltottuk neveket, jon valasztas:
    valaszt2 val;
    val.kur = kezdo;
    val.egykepen = LISTegykepen;
    val.x0 = LISTx0;
    val.y0 = LISTy0;
    val.dy = LISTdy;
    val.escelheto = 1;
    strcpy(val.cim, "Select External File!");

    val.bead(szam);

    while (1) {
        int eredmeny = val.valassz();
        if (eredmeny < 0) {
            return;
        }

        char filenev[20];
        if (strlen(val.rubrikak[eredmeny]) > 15) {
            hiba("playextmenu-ben val.rubrikak[eredmeny] ) > 15!");
        }
        strcpy(filenev, val.rubrikak[eredmeny]);

        strcpy(State->playextfilenev, filenev);

        while (1) {
            kiirloading();
            // Ha topologiailag hibas lev file (uzenetet load adja):
            if (!floadlevel_p(filenev)) {
                break;
            }
            Prec1->erase(filenev);
            Prec2->erase(filenev);
            int ido = lejatszo(filenev);
            Pmenupal->set();
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
