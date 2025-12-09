#include "all.h"

pic8* Buffsima = NULL;
pic8* Buffgolyo = NULL;

static pic8 *Kisnevsima = NULL, *Kisnevgolyo = NULL;
abc8* Pmenuabc = NULL;
anim* Psisak = NULL;

ddpal* Pmenupal = NULL;
unsigned char Menupaltomb[768];

/*void retusal( pic8* ppic ) {
    int xsize = ppic->getxsize();
    int ysize = ppic->getysize();
    for( int y = 0; y < ysize; y++ ) {
        for( int x = 0; x < xsize; x++ ) {
            unsigned szin = ppic->gpixel( x, y );
            if( szin == 2 )
                ppic->ppixel( x, y, 30 );
        }
    }
} */

void initmenukep1(void) {
    if (Buffsima) {
        hiba("initmenukep1-ben Buffsima!");
    }
    Buffsima = new pic8(640, 480);
    Buffgolyo = new pic8(640, 480);
    Kisnevsima = new pic8("szoveg1.pcx");  // retusal( Kisnevsima );
    Kisnevgolyo = new pic8("szoveg2.pcx"); // retusal( Kisnevgolyo );
    Pmenuabc = new abc8("menu.abc");
    Pmenuabc->settav(2);

    pic8* sisaktmp = new pic8("sisak.pcx");
    forditkepet(sisaktmp);
    Psisak = new anim(sisaktmp, "sisak.pcx");
    delete sisaktmp;
    sisaktmp = NULL;
    Psisak->korrigal();

    initgolyo();
}

void initmenukep2(void) {
    if (Pmenupal) {
        hiba("initmenukep2-ben Pmenupal!");
    }
    pcxtopal("intro.pcx", &Pmenupal); // O new-ik egy ddpal-t
    pcxtopal("intro.pcx", Menupaltomb);
}

szoveglista::szoveglista(void) {
    sisakx = sisaky = -100;

    elemszam = 0;
    ervenyes = 0;
    sorok = new egyszovsor[MAXSZOVEGSOR];
    if (!sorok) {
        hiba("Memoria!");
    }
}

szoveglista::~szoveglista(void) {
    if (!sorok) {
        hiba("szoveglista::~szoveglista !sorok!");
    }
    delete sorok;
    sorok = NULL;
}

void szoveglista::addszoveg(char* text, int x, int y) {
    if (strlen(text) > 100) {
        hiba("89ygui");
    }
    if (elemszam >= MAXSZOVEGSOR - 1) {
        hiba("849yptgp8");
    }
    strcpy(sorok[elemszam].text, text);
    sorok[elemszam].x = x;
    sorok[elemszam].y = y;
    elemszam++;
    ervenyes = 0;
}

void szoveglista::addszoveg_kozep(char* text, int x, int y) {
    addszoveg(text, x - Pmenuabc->len(text) / 2, y);
}

void szoveglista::setsisak(int x, int y) {
    sisakx = x;
    sisaky = y;
}

void szoveglista::clear(void) {
    elemszam = 0;
    ervenyes = 0;
}

static pic8* Tmpbuffer = NULL; // Ideiglenes

static int Scrolly1 = 0; // Ennel kiseb x-ek helyere sotet zoldet rak kirazgolyo
static unsigned char Introhatter = 254;
static unsigned char* Zoldsor = NULL;
static unsigned char* Hattersor = NULL;

static void kirajzgolyo(vekt2 g, double r, pic8* forras) {
    if (!Zoldsor) {
        Zoldsor = new unsigned char[640];
        Hattersor = new unsigned char[640];
        for (int i = 0; i < 640; i++) {
            Zoldsor[i] = 248;
            Hattersor[i] = Introhatter;
        }
    }

    if (Nincsfal) {
        if (g.x < -500.0 || g.x > 1500.0 || g.y < -500.0 || g.y > 1500.0) {
            return;
        }
    }

    double xo = g.x;
    double yo = g.y;
    int y = yo - r;
    while (y < yo + r) {
        if (y < 0 || y >= 480) {
            y++;
            continue;
        }

        double dx = r * r - (y - yo) * (y - yo);
        if (dx < 0.5) {
            y++;
            continue;
        }
        dx = sqrt(dx);
        int x1 = xo - dx;
        int x2 = xo + dx;
        if (x1 < 0) {
            x1 = 0;
        }
        if (x1 > 639) {
            x1 = 639;
        }
        if (x2 < 0) {
            x2 = 0;
        }
        if (x2 > 639) {
            x2 = 639;
        }
        if (x1 >= x2) {
            y++;
            continue;
        }
        unsigned char* dest = Tmpbuffer->getptr(y) + x1;
        unsigned char* source;
        if (y < Scrolly1) {
            if (forras == Buffgolyo) {
                source = Zoldsor;
            } else {
                source = Hattersor;
            }
        } else {
            source = forras->getptr(y) + x1;
        }
        memcpy(dest, source, x2 - x1 + 1);

        y++;
    }
}

static double Regiido = 0.0;
static double Elsostabilido = 0.0; // Scroll befejezesekor
static int Kezdoanimjon = 1;

void szoveglista::kirajzol(int nemkellgolyo, pic8* rarakkep1, pic8* rarakkep2, pic8* rarakkep3,
                           pic8* rarakkep4) {
    // Kirakjuk hatteret:
    if (!Tmpbuffer) { // Ideiglenes
        Tmpbuffer = new pic8(640, 480);
    }

    if (Kezdoanimjon && !Pintro) {
        Kezdoanimjon = 0;
    }

    if (!ervenyes && !Kezdoanimjon) {
        // Ujrarajzoljuk hatterkepeket:
        ervenyes = 1;
        int y = -47;
        int paros = 0;
        int kezdox = 0;
        while (y < 480) {
            int x = kezdox;
            kezdox += 110;
            while (x > 0) {
                x -= Kisnevsima->getxsize();
            }
            paros = !paros;
            while (x < 640) {
                blt8(Buffsima, Kisnevsima, x, y);
                blt8(Buffgolyo, Kisnevgolyo, x, y);
                x += Kisnevsima->getxsize();
            }
            y += Kisnevsima->getysize();
        }

        // Menu lemezre mentese:
        // Buffsima->save( "menu.pcx", Menupaltomb );
        // hiba( "Menu kimentve!" );

        // Rairjuk szovegeket vagy kepet:
        if (rarakkep4) {
            blt8(Buffsima, rarakkep1);
            blt8(Buffsima, rarakkep2, 0, 120);
            blt8(Buffsima, rarakkep3, 0, 240);
            blt8(Buffsima, rarakkep4, 0, 360);

            blt8(Buffgolyo, rarakkep1);
            blt8(Buffgolyo, rarakkep2, 0, 120);
            blt8(Buffgolyo, rarakkep3, 0, 240);
            blt8(Buffgolyo, rarakkep4, 0, 360);
        } else {
            for (int i = 0; i < elemszam; i++) {
                Pmenuabc->write(Buffsima, sorok[i].x, sorok[i].y, sorok[i].text);
                Pmenuabc->write(Buffgolyo, sorok[i].x, sorok[i].y, sorok[i].text);
            }
        }

        // Ha kell logokep, azt itt rarajzoljuk:
        /*if( logokep ) {
            static int elsologo = 1;
            if( !elsologo )
                hiba( "8989654" );
            elsologo = 0;

            pic8* ppiclogo = new pic8( "intro.pcx" );
            spriteosit( ppiclogo );
            blt8( Buffsima, ppiclogo );
            blt8( Buffgolyo, ppiclogo );
            delete ppiclogo; ppiclogo = NULL;
        } */
    }

    double ujido = mv_stopperido();
    if (Kezdoanimjon) {
        Pmenupal->set();
        while (1) {
            /*if( mk_kbhit() ) {
                int c = mk_getextchar();
                if( c == MK_ESC )
                    return;
            }*/
            if (!kirajzolanim(ujido)) {
                Kezdoanimjon = 0;
                Regiido = ujido;
                return;
            }

            ujido = mv_stopperido();
            Elsostabilido = ujido;
            Kezdoanimjon = 0;
        }
    }

    double dt;
    if (ujido < Elsostabilido + 10.0) {
        dt = 0.0000000000001;
    } else {
        dt = ujido - Regiido;
        Elsostabilido = -100.0;
    }

    Regiido = ujido;
    if (dt < 0.0) {
        dt = 0.001;
    }

    dt *= 3.6;
    if (dt > 100.0) {
        dt = 100.0;
    }

    leptetgolyo(dt); // 12.0 volt regebben

    blt8(Tmpbuffer, Buffsima);

    for (int i = 0; i < Ngolyok; i++) {
        if (nemkellgolyo || !State->animatedmenus) {
            break;
        }

        kirajzgolyo(Golyok[i].rmostani, Golyok[i].sug, Buffgolyo);

        vekt2 diff(Golyok[i].sug * 0.5 * cos(Golyok[i].alfamostani),
                   Golyok[i].sug * 0.5 * sin(Golyok[i].alfamostani));
        kirajzgolyo(Golyok[i].rmostani + diff, Golyok[i].sug * 0.25, Buffsima);
        kirajzgolyo(Golyok[i].rmostani - diff, Golyok[i].sug * 0.25, Buffsima);
    }
    if (!nemkellgolyo) {
        pic8* sisakframe = NULL;
        if (State->animatedmenus) {
            sisakframe = Psisak->getframe(ujido * 0.0024);
        } else {
            sisakframe = Psisak->getframebyindex(25);
        }

        blt8(Tmpbuffer, sisakframe, sisakx - 20, sisaky - 7);
    }

    // Tmpbuffer->save( "kezdszov.pcx", Menupaltomb );
    // hiba( "kezdszov.pcx kiirva!" );
    bltfront(Tmpbuffer);
}

static int Elsokiranim = 1;
// Teljes olvassa be es elso kiirloading szabaditja fel:
extern pic8* Pintro = NULL;

static double Animkezdete = 0.0;

// Hamis ha animnak mar vege:
int szoveglista::kirajzolanim(double ido) {
    if (Elsokiranim) {
        leptetgolyo(0.000000001);
        Elsokiranim = 0;
        Animkezdete = ido;

        // Berajzoljuk hatterkepeket:
        int y = -47;
        int paros = 0;
        int kezdox = 0;
        while (y < 480) {
            int x = kezdox;
            kezdox += 110;
            while (x > 0) {
                x -= Kisnevsima->getxsize();
            }
            paros = !paros;
            while (x < 640) {
                blt8(Buffsima, Kisnevsima, x, y);
                blt8(Buffgolyo, Kisnevgolyo, x, y);
                x += Kisnevsima->getxsize();
            }
            y += Kisnevsima->getysize();
        }
        // Most egyszer kirakunk minden elemet kepre hogy
        // ne akadjon meg elso lejatszas kozben:
        Tmpbuffer->fillbox(Introhatter);
        blt8(Tmpbuffer, Buffsima, 0, 0);

        Scrolly1 = 0;
        double yeltolas = 0.0;
        for (int i = 0; i < Ngolyok; i++) {
            if (!State->animatedmenus) {
                break;
            }

            Golyok[i].rmostani.y += yeltolas;

            kirajzgolyo(Golyok[i].rmostani, Golyok[i].sug, Buffgolyo);

            vekt2 diff(Golyok[i].sug * 0.5 * cos(Golyok[i].alfamostani),
                       Golyok[i].sug * 0.5 * sin(Golyok[i].alfamostani));
            kirajzgolyo(Golyok[i].rmostani + diff, Golyok[i].sug * 0.25, Buffsima);
            kirajzgolyo(Golyok[i].rmostani - diff, Golyok[i].sug * 0.25, Buffsima);

            Golyok[i].rmostani.y -= yeltolas;
        }

        // Rairjuk szovegeket:
        for (int i = 0; i < elemszam; i++) {
            Pmenuabc->write(Tmpbuffer, sorok[i].x, sorok[i].y, sorok[i].text);
        }

        pic8* sisakframe = NULL;
        sisakframe = Psisak->getframebyindex(25);
        blt8(Tmpbuffer, sisakframe, sisakx - 20, sisaky - 7);
        blt8(Tmpbuffer, Pintro, 0, 0);
        // Idaig tartott minden elem kirakasa egyszer
    }

    ido -= Animkezdete;

    int frame = ido * 2.3;
    // int framex = ido*2.3*(640.0/480.0);

    /*if( frame < 480 ) {
        // Kirakjuk legordulo hatteret:
        Tmpbuffer->fillbox( Introhatter );
        blt8( Tmpbuffer, Buffsima, 0, frame-480 );
        blt8( Tmpbuffer, Pintro );
        bltfront( Tmpbuffer );
        return 1;
    }

    frame -= 480;*/

    if (frame >= 480) {
        Scrolly1 = 0;
        return 0;
    }

    // Most mar masodik animaciot rajzoljuk:
    // blt8( Tmpbuffer, Buffsima );

    Tmpbuffer->fillbox(Introhatter);
    blt8(Tmpbuffer, Buffsima, 0, 480 - frame);

    Scrolly1 = 480 - frame;
    double yeltolas = frame - 480.0;
    for (int i = 0; i < Ngolyok; i++) {
        if (!State->animatedmenus) {
            break;
        }

        Golyok[i].rmostani.y += yeltolas;

        kirajzgolyo(Golyok[i].rmostani, Golyok[i].sug, Buffgolyo);

        vekt2 diff(Golyok[i].sug * 0.5 * cos(Golyok[i].alfamostani),
                   Golyok[i].sug * 0.5 * sin(Golyok[i].alfamostani));
        kirajzgolyo(Golyok[i].rmostani + diff, Golyok[i].sug * 0.25, Buffsima);
        kirajzgolyo(Golyok[i].rmostani - diff, Golyok[i].sug * 0.25, Buffsima);

        Golyok[i].rmostani.y -= yeltolas;
    }

    // Rairjuk szovegeket:
    for (int i = 0; i < elemszam; i++) {
        Pmenuabc->write(Tmpbuffer, sorok[i].x, sorok[i].y + frame - 480, sorok[i].text);
    }

    // Rarakjuk sisakot:
    pic8* sisakframe = NULL;
    if (State->animatedmenus) {
        sisakframe = Psisak->getframe(ido * 0.0024);
    } else {
        sisakframe = Psisak->getframebyindex(25);
    }

    blt8(Tmpbuffer, sisakframe, sisakx - 20, sisaky - 7 - 479 + frame);

    blt8(Tmpbuffer, Pintro, 0, frame);
    // blt8( Tmpbuffer, Pintro, framex, 0, 0, 0, 639, 206 );
    // blt8( Tmpbuffer, Pintro, -framex, 206, 0, 206, 639, 479 );
    bltfront(Tmpbuffer);
    return 1;
}

#define MAXHIBASORHOSSZ (34)
typedef char hibasor[MAXHIBASORHOSSZ + 4];
#define MAXHIBASOR (10)
static hibasor Hibasortomb[MAXHIBASOR];

void grafikushiba(char* text1, char* text2, char* text3) {
    // Kirajzoljuk hatteret Buffsima-ba:
    int y = 0;
    int paros = 0;
    while (y < 480) {
        int x = 0;
        if (paros) {
            x -= Kisnevsima->getxsize() / 2;
        }
        paros = !paros;
        while (x < 640) {
            blt8(Buffsima, Kisnevsima, x, y);
            x += Kisnevsima->getxsize();
        }
        y += Kisnevsima->getysize();
    }

    int sorszam = 0;
    for (int i = 0; i < 3; i++) {
        char* text = text1;
        if (i == 1) {
            text = text2;
        }
        if (i == 2) {
            text = text3;
        }
        if (!text) {
            break;
        }

        while (1) {
            if (sorszam >= MAXHIBASOR) {
                break;
            }
            if (strlen(text) <= MAXHIBASORHOSSZ) {
                // Siman belefer:
                strcpy(Hibasortomb[sorszam], text);
                sorszam++;
                break;
            }
            // Tul hosszu sor, le kell vagni belole:
            int j = MAXHIBASORHOSSZ;
            while (j > 0) {
                if (text[j] == ' ') {
                    break;
                }

                j--;
            }
            if (j <= 0) {
                // Nem talalt szovegben space-t:
                char c = text[MAXHIBASORHOSSZ];
                text[MAXHIBASORHOSSZ] = 0;
                strcpy(Hibasortomb[sorszam], text);
                sorszam++;
                text[MAXHIBASORHOSSZ] = c;
                text += MAXHIBASORHOSSZ;
            } else {
                char c = text[j];
                text[j] = 0;
                strcpy(Hibasortomb[sorszam], text);
                sorszam++;
                text[j] = c;
                text += j;
                while (text[j] == ' ') {
                    text++;
                }
            }
        }
    }

    // Rairjuk szovegeket:
    for (int i = 0; i < sorszam; i++) {
        int y = 240 - 20 - sorszam * (40 / 2) + i * 40;
        Pmenuabc->writekozep(Buffsima, 320, y, Hibasortomb[i]);
    }
    // Kirakjuk:
    bltfront(Buffsima);
    if (Pmenupal) {
        Pmenupal->set();
    }
}

void menuproba(void) {
    szoveglista szl;
    szl.addszoveg("ABCDEFGH", 100, 100);
    szl.addszoveg("abcdefgh", 100, 200);
    Pmenupal->set();
    while (1) {
        if (mk_kbhit()) {
            hiba("Vege!");
        }
        szl.kirajzol();
    }
}

/*void szoveglista::kirajzol( int nemkellgolyo ) {
    if( !ervenyes ) {
        // Ujrarajzoljuk hatterkepeket:
        ervenyes = 1;
        int y = 0;
        int paros = 0;
        while( y < 480 ) {
            int x = 0;
            if( paros )
                x -= Kisnevsima->getxsize()/2;
            paros = !paros;
            while( x < 640 ) {
                blt8( Buffsima, Kisnevsima, x, y );
                blt8( Buffgolyo, Kisnevgolyo, x, y );
                x += Kisnevsima->getxsize();
            }
            y += Kisnevsima->getysize();
        }
        // Rairjuk szovegeket:
        for( int i = 0; i < elemszam; i++ ) {
            Pmenuabc->write( Buffsima, sorok[i].x, sorok[i].y,
                                sorok[i].text );
            Pmenuabc->write( Buffgolyo, sorok[i].x, sorok[i].y,
                                sorok[i].text );
        }
    }

    double ujido = mv_stopperido();
    double dt = ujido-Regiido;
    Regiido = ujido;
    if( dt < 0.0 )
        dt = 0.001;

    dt *= 3.6;
    if( dt > 100.0 )
        dt = 100.0;

    leptetgolyo( dt ); // 12.0 volt regebben

    // Kirakjuk hatteret:
    if( !Tmpbuffer ) // Ideiglenes
        Tmpbuffer = new pic8( 640, 480 );

    blt8( Tmpbuffer, Buffsima );

    for( int i = 0; i < Ngolyok; i++ ) {
        if( nemkellgolyo || !State->animatedmenus )
            break;

        kirajzgolyo( Golyok[i].rmostani, Golyok[i].sug, Buffgolyo );

        vekt2 diff( Golyok[i].sug*0.5*cos( Golyok[i].alfamostani ),
                    Golyok[i].sug*0.5*sin( Golyok[i].alfamostani ) );
        kirajzgolyo( Golyok[i].rmostani + diff,
                    Golyok[i].sug*0.25, Buffsima );
        kirajzgolyo( Golyok[i].rmostani - diff,
                    Golyok[i].sug*0.25, Buffsima );
    }
    if( !nemkellgolyo ) {
        pic8* sisakframe = NULL;
        if( State->animatedmenus )
            sisakframe = Psisak->getframe( ujido*0.0024 );
        else
            sisakframe = Psisak->getframebyindex( 25 );

        blt8( Tmpbuffer, sisakframe, sisakx-20, sisaky-7 );
    }

    bltfront( Tmpbuffer );
} */
