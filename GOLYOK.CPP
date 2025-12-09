#include "all.h"

double Hosszuido = 500000000.0;

double Maxgolyoseb = 0;

int Ngolyok = 0;
golyo* Golyok = NULL;
int Konstrualva = 0;
double** Idok = NULL;

static double Legkozelebbiutkozesideje = -2, Pillanatnyiido = 0;

typedef double* duplaramutato;

static void kitoltgolyokat(void) {
    Ngolyok = 9;
    Golyok = new golyo[Ngolyok];
    // Nullazas:
    for (int x = 0; x < 3; x++) {
        for (int y = 0; y < 3; y++) {
            int i = y * 3 + x;
            if (x == 0) {
                Golyok[i].sug = 24;
            }
            if (x == 1) {
                Golyok[i].sug = 30;
            }
            if (x == 2) {
                Golyok[i].sug = 50;
            }
            Golyok[i].t = 0;
            Golyok[i].v.x = 0;
            Golyok[i].v.y = 0;
            Golyok[i].r.x = 320 + (x - 1) * 120;
            Golyok[i].r.y = 240 + (y - 1) * 120;
            Golyok[i].alfa = 0.0;
            Golyok[i].alfamostani = 0.0;
            Golyok[i].omega = 0.0;
        }
    }

    // Beallitjuk kezdeti sebesseget:
    srand(time(NULL));
    double alfa = s_random(1000);
    alfa *= K_pip2 / 1000.0;
    alfa *= 0.999;
    alfa += 0.0005;

    Golyok[0].v.x = -sin(alfa);
    Golyok[0].v.y = -cos(alfa);
}

// Siker eseten igazzal ter vissza:
void initgolyo(void) {
    kitoltgolyokat();

    int sikerultlefoglalni, i, j;
    Legkozelebbiutkozesideje = -2;
    Pillanatnyiido = 0;
    if (Konstrualva) {
        hiba("Nem destrualtad golyokat!");
    }
    sikerultlefoglalni = 1;
    Idok = new duplaramutato[Ngolyok + 4];
    if (!Idok) {
        sikerultlefoglalni = 0;
    }
    if (sikerultlefoglalni) {
        for (int i = 0; i < Ngolyok + 4; i++) {
            Idok[i] = NULL;
        }
    }
    if (sikerultlefoglalni) {
        for (int i = 0; i < Ngolyok + 4 && sikerultlefoglalni; i++) {
            Idok[i] = new double[Ngolyok + 4];
            if (Idok[i]) {
                for (int j = 0; j < Ngolyok + 4; j++) {
                    Idok[i][j] = -2;
                }
            }
            if (!Idok[i]) {
                sikerultlefoglalni = 0;
            }
        }
    }
    if (!sikerultlefoglalni) {
        if (Idok) {
            for (int i = 0; i < Ngolyok + 4; i++) {
                if (Idok[i]) {
                    delete Idok[i];
                }
            }
            delete Idok;
            Idok = NULL;
        }
        hiba("Out of memory!");
    }

    Konstrualva = 1;
}

/*void destrual( void ) {
    int i;

    if( Konstrualva ) {
        for( i = 0; i < Ngolyok; i ++ )
            delete Idok[i];
        delete Idok;
        Idok = NULL;
        Konstrualva = 0;
    }
} */

static double Kezdoszamlalo = 0.0;

void leptetgolyo(double stept) {
    if (Kezdoszamlalo < 1.0 /*400.0*/) {
        Kezdoszamlalo += stept;
        stept = 0.00000000001;
    }

    // stept *= 2.0;

    if (!Konstrualva) {
        hiba("Nem konstrualtad golyokat!");
        return;
    }

    // Kiszamoljuk osszenergiat:
    /*double sum = 0.0;
    for( int i = 0; i < Ngolyok; i++ )
        sum += Golyok[i].v.x*Golyok[i].v.x + Golyok[i].v.y*Golyok[i].v.y;

    sum = sqrt( sum );
    // Korrigaljuk energiat:
    stept /= sum;*/

    // Atmasolja mostani r-t regi r-be:
    // for( int i = 0; i < Ngolyok; i++ ) {
    //	Golyok[i].relozo.x = Golyok[i].rmostani.x;
    //	Golyok[i].relozo.y = Golyok[i].rmostani.y;
    //}

    // Most jon az a resz, ami biztositja, hogy ha nincs utkozes ne kelljen
    // sokat szamolni (Egyszeruen kihagyja hatso reszt):
    if (Legkozelebbiutkozesideje > -1 && Legkozelebbiutkozesideje > Pillanatnyiido + stept) {
        Pillanatnyiido += stept;
        for (int i = 0; i < Ngolyok; i++) {
            Golyok[i].rmostani = Golyok[i].r + (Pillanatnyiido * Golyok[i].v);
            Golyok[i].alfamostani = Golyok[i].alfa + (Pillanatnyiido * Golyok[i].omega);
        }
        return;
    }

    stept += Pillanatnyiido;

    int done = 0;
    while (!done) {
        int x, y;
        Legkozelebbiutkozesideje = megkeresidot(&x, &y);
        // if( Legkozelebbiutkozesideje > Hosszuido-1.0 ) {
        //	Maxgolyoseb = 0;
        //	return;
        // }
        if (Legkozelebbiutkozesideje > stept) {
            done = 1;
        } else {
            utkoztet(x, y);
        }
    }
    // stept idore igazit minden golyot es lenullazza idejet:
    for (int i = 0; i < Ngolyok; i++) {
        // dsvlet( &Golyok[i].r, dsvossz( &Golyok[i].r,
        //					dsvmul( stept-Golyok[i].t, &Golyok[i].v ) ) );
        Golyok[i].r = Golyok[i].r + ((stept - Golyok[i].t) * Golyok[i].v);
        Golyok[i].rmostani = Golyok[i].r;
        Golyok[i].alfa = Golyok[i].alfa + ((stept - Golyok[i].t) * Golyok[i].omega);
        Golyok[i].alfamostani = Golyok[i].alfa;
        Golyok[i].t = 0;
    }
    // Idok tablaban mindenbol kivon stept idot:
    for (int i = 0; i < Ngolyok; i++) {
        for (int j = i + 1; j < Ngolyok + 4; j++) {
            Idok[i][j] -= stept;
        }
    }
    Pillanatnyiido = 0;
    Legkozelebbiutkozesideje -= stept;
}

/*void ujraszamol( void ) {
    for( int i = 0; i < Ngolyok; i++ )
        for( int j = i+1; j < Ngolyok+4; j++ )
            Idok[i][j] = -2;
    Legkozelebbiutkozesideje = -2;
    Pillanatnyiido = 0;
}

void idoreigazitmindet( void ) {
    for( int i = 0; i < Ngolyok; i++ ) {
        //dsvlet( &Golyok[i].r, dsvossz( &Golyok[i].r,
        //		dsvmul( Pillanatnyiido-Golyok[i].t, &Golyok[i].v ) ) );
        Golyok[i].r = Golyok[i].r +
                        ((Pillanatnyiido-Golyok[i].t) * Golyok[i].v);
        Golyok[i].t = 0;
    }
} */
