#include "all.h"

// szamitogep
// monitor

void semmi(void) {}

int Sajatugras = 0;       // (forditva mint regen) Ha igaz, akkor jo ugras (right-clockwise)
int Otszoros = 0;         // Ha igaz, mindig otszor rakja ki palyat
int Dupladuplabuffer = 0; // Ha igaz duplan huzza az idot (Win95)
// Ha nem igaz, csak memoriaba ir es nem flip-pel:
int Waitforretrace = 1; // Csak Windows-os w_v.cpp-ben allitodik

static void ugraseldontes(void) {
    if (access("fordit.inf", 0) == 0) {
        Sajatugras = 0;
    } else {
        Sajatugras = 1;
    }
}

// magassagokeloallitasa-bol kimasolva!!!!!:
/*static void elemszam( char* text, szakaszok* pszakok ) {
    pszakok->felsorolasresetszak();
    szakasz* psz = pszakok->getnextszak();
    if( !psz )
        hiba( "Nincs egyetlen szakasz sem definialva!" );
    double minx = psz->vv.r.x;
    double maxx = psz->vv.r.x;
    double miny = psz->vv.r.y;
    double maxy = psz->vv.r.y;
    while( psz ) {
        if( psz->vv.r.x < minx )
            minx = psz->vv.r.x;
        if( psz->vv.r.x > maxx )
            maxx = psz->vv.r.x;
        if( psz->vv.r.y < miny )
            miny = psz->vv.r.y;
        if( psz->vv.r.y > maxy )
            maxy = psz->vv.r.y;

        if( psz->vv.r.x+psz->vv.v.x < minx )
            minx = psz->vv.r.x+psz->vv.v.x;
        if( psz->vv.r.x+psz->vv.v.x > maxx )
            maxx = psz->vv.r.x+psz->vv.v.x;
        if( psz->vv.r.y+psz->vv.v.y < miny )
            miny = psz->vv.r.y+psz->vv.v.y;
        if( psz->vv.r.y+psz->vv.v.y > maxy )
            maxy = psz->vv.r.y+psz->vv.v.y;

        psz = pszakok->getnextszak();
    }

    vekt2 origo = vekt2( minx-400/Aranyx, miny-300/Aranyy );
    // Egy kis veletlenites!:
    origo.x += 0.0000145839857463;
    origo.y += 0.0000295874632574;

    double xsize = (maxx+400/Aranyx)-origo.x;
    double ysize = (maxy+300/Aranyy)-origo.y;

    // Lefoglalja magassagokat:
    int xszazalek = 100.0*(xsize*Aranyx)/Magmaxelemszam;
    int yszazalek = 100.0*(ysize*Aranyy)/Magmaxelemszam;
    sprintf( text, "Kihasznaltsag szazalekban: x = %d, y = %d", xszazalek,
                                                                yszazalek );
} */

/*
q7/8
    232, 473
o0/1
    336, 343
n8/9
    241, 245, 246, 580, 611, 982
o8/9
    376, 379, 634
*/

void mv_main(void) {
    // new wav( "utodes.wav", 0.25 );

    // initqopen();

    if (access("nemmem.inf", 0) != 0) {
        // Ha nem talalja nemmem.inf-et:
        // mem Totall Free 11600K-nal meg eppen elmegy, 101 mar uzenetet ir:
        int memkell = 100;
        char* tomb[110];
        for (int i = 0; i < memkell; i++) {
            {
                tomb[i] = new char[100000];
            }
            if (!tomb[i]) {
                char tmp[200];
                sprintf(tmp, "You need about %d00K more free memory!\n", memkell - i);
                uzenet(tmp);
            }
        }
        for (int i = 0; i < memkell; i++) {
            delete tomb[i];
        }
    }

    if (access("otszor.inf", 0) == 0) {
        Otszoros = 1;
    }

    if (access("ddbuffer.inf", 0) == 0) {
        Dupladuplabuffer = 1;
    }

    // initadatok(); teljesben van mar ujabban

    ugraseldontes();

    teljes();
}
