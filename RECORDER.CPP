#include "all.h"

recorder *Prec1 = NULL, *Prec2 = NULL;
int Multirec = 0; // true if multiplayer

#define MAGICNUMBER (4796277l)

// #define Darabszam (10000u)
// #define Hangdarabszam (3000u)

// fresh outcommenting
// static const int Darabszam = 14370;
// static const int Hangdarabszam = 3000;

static const int Darabszam = 8981; // Ot perc
static const int Hangdarabszam = 3900;

static double Valtrate = 30; // 60;
static double Recorderszorzo = Valtrate / (182.0 * 0.0024);

recorder::recorder(void) {
    palyanev[0] = 0;
    pk1rx = NULL;
    pk1ry = NULL;
    pk2rx = NULL;
    pk2ry = NULL;
    pk4rx = NULL;
    pk4ry = NULL;
    pvezrx = NULL;
    pvezry = NULL;
    pk2alfa = NULL;
    pk1alfa = NULL;
    pk4alfa = NULL;
    pfrekvencia = NULL;
    psurlero = NULL;
    pgazhatra = NULL;

    phangok = NULL;

    betoltve = 0;
    hangbetoltve = 0;

    tag = 0;

    // check
    long hossz = sizeof(float);
    if (hossz != 4) {
        hiba("float hossz != 4!");
    }
    hossz = long(Darabszam) * sizeof(float);
    if (hossz > 64000l) {
        hiba("Darabszam*sizeof( float )!");
    }

    hossz = long(Hangdarabszam) * sizeof(egyhang);
    if (hossz > 64000l) {
        hiba("Hangdarabszam*sizeof( egyhang )!");
    }

    pk1rx = new float[Darabszam];
    pk1ry = new float[Darabszam];
    pk2rx = new short[Darabszam];
    pk2ry = new short[Darabszam];
    pk4rx = new short[Darabszam];
    pk4ry = new short[Darabszam];
    pvezrx = new short[Darabszam];
    pvezry = new short[Darabszam];
    pk2alfa = new unsigned char[Darabszam];
    pk1alfa = new short[Darabszam];
    pk4alfa = new unsigned char[Darabszam];
    pfrekvencia = new unsigned char[Darabszam];
    psurlero = new unsigned char[Darabszam];
    pgazhatra = new unsigned char[Darabszam];

    phangok = new egyhang[Hangdarabszam];

    if (!pk1rx || !pk1ry || !pk2rx || !pk2ry || !pk4rx || !pk4ry || !pvezrx || !pvezry ||
        !pk2alfa || !pk1alfa || !pk4alfa || !pfrekvencia || !pgazhatra || !phangok || !psurlero) {
        uzenet("memory");
    }
}

recorder::~recorder(void) {
    // not done here

void recorder::erase(char* palyanev_p) {
    if (strlen(palyanev_p) > 12) {
        hiba("875h8uyf");
    }
    strcpy(palyanev, palyanev_p);
    betoltve = 0;
    hangbetoltve = 0;
    ucsokiadva = 0;
    hangkov = 0;
    legkozindex = 0;
}

void recorder::rewind(void) {
    ucsokiadva = 0;
    hangkov = 0;
    legkozindex = 0;
}

int recorder::gettag(void) { return tag; }

void recorder::settag(int tag_p) { tag = tag_p; }

static double Korrszorzo = 1000.0, Korrszorzo_rec = 1.0 / Korrszorzo;
static double Alfaszorzo = 250 / (2.0 * K_pi), Alfaszorzo_rec = 1.0 / Alfaszorzo;
static double Alfaszorzo_16 = 10000 / (2.0 * K_pi), Alfaszorzo_16_rec = 1.0 / Alfaszorzo_16;
static double Frekvszorzo = 250.0, Frekvszorzo_rec = 1.0 / Frekvszorzo;
// static double Frekvszorzo = 250.0/2.0, 	   Frekvszorzo_rec = 1.0 / Frekvszorzo;
static double Surlszorzo = 250 / 2.0, Surlszorzo_rec = 1.0 / Surlszorzo;

// int Mostirdki = 0;

/*static int Filebairsorszam = 0;

static void filebair( float d1, float d2, float d3 ) {
    FILE* h = fopen( "log.txt", "at" );
    if( !h )
        hiba( "Cannot open filebair!" );

    fprintf( h, "%d %f %f %f\n", Filebairsorszam, d1, d2, d3 );


    Filebairsorszam++;
    fclose( h );
}*/

int recorder::recall(motorst* pmot, double dt, hanginfo* phinfo) {
    if (betoltve <= 0) {
        hiba("betoltve <= 0 in recall!");
    }

    int index1 = Recorderszorzo * dt;
    double suly2 = Recorderszorzo * dt - index1;
    if (suly2 < 0.0) {
        suly2 = 0.0;
    }
    if (suly2 > 1.0) {
        suly2 = 1.0;
    }
    double suly1 = 1.0 - suly2;
    int index2 = index1 + 1;

    if (index1 < 0) {
        index1 = 0;
    }
    if (index2 < 0) {
        index2 = 0;
    }
    if (ucsokiadva) {
        phinfo->frekvencia = 1.0;
        phinfo->gaz = 0;
        phinfo->surlero = 0;
        return 0;
    }

    if (index1 >= betoltve - 1) {
        index1 = betoltve - 1;
        ucsokiadva = 1;
    }
    if (index2 >= betoltve - 1) {
        index2 = betoltve - 1;
    }

    pmot->kor1.r.x = pk1rx[index1] * suly1 + pk1rx[index2] * suly2;
    pmot->kor1.r.y = pk1ry[index1] * suly1 + pk1ry[index2] * suly2;
    pmot->kor2.r.x =
        pmot->kor1.r.x + Korrszorzo_rec * (pk2rx[index1] * suly1 + pk2rx[index2] * suly2);
    pmot->kor2.r.y =
        pmot->kor1.r.y + Korrszorzo_rec * (pk2ry[index1] * suly1 + pk2ry[index2] * suly2);
    pmot->kor4.r.x =
        pmot->kor1.r.x + Korrszorzo_rec * (pk4rx[index1] * suly1 + pk4rx[index2] * suly2);
    pmot->kor4.r.y =
        pmot->kor1.r.y + Korrszorzo_rec * (pk4ry[index1] * suly1 + pk4ry[index2] * suly2);
    pmot->vezetor.x =
        pmot->kor1.r.x + Korrszorzo_rec * (pvezrx[index1] * suly1 + pvezrx[index2] * suly2);
    pmot->vezetor.y =
        pmot->kor1.r.y + Korrszorzo_rec * (pvezry[index1] * suly1 + pvezry[index2] * suly2);

    if (index1 >= 2) {
        // bike
        if (abs(pk1alfa[index1] - pk1alfa[index2]) > 9000) { // 1000
            // pmot->kor1.alfa = Alfaszorzo_16_rec * pk1alfa[index2];
            if (pk1alfa[index1] > pk1alfa[index2]) {
                int fixedindex1 = pk1alfa[index1] - 10000;
                pmot->kor1.alfa =
                    Alfaszorzo_16_rec * (fixedindex1 * suly1 + pk1alfa[index2] * suly2);
            } else {
                int fixedindex2 = pk1alfa[index2] - 10000;
                pmot->kor1.alfa =
                    Alfaszorzo_16_rec * (pk1alfa[index1] * suly1 * suly1 + fixedindex2 * suly2);
            }
        } else {
            pmot->kor1.alfa =
                Alfaszorzo_16_rec * (pk1alfa[index1] * suly1 + pk1alfa[index2] * suly2);
        }

        // front wheel
        if (abs(pk2alfa[index1] - pk2alfa[index2]) > 220) { // 50
            // pmot->kor2.alfa = Alfaszorzo_rec * pk2alfa[index2];
            if (pk2alfa[index1] > pk2alfa[index2]) {
                int fixedindex1 = pk2alfa[index1] - 250;
                pmot->kor2.alfa = Alfaszorzo_rec * (fixedindex1 * suly1 + pk2alfa[index2] * suly2);
            } else {
                int fixedindex2 = pk2alfa[index2] - 250;
                pmot->kor2.alfa =
                    Alfaszorzo_rec * (pk2alfa[index1] * suly1 * suly1 + fixedindex2 * suly2);
            }
        } else {
            pmot->kor2.alfa = Alfaszorzo_rec * (pk2alfa[index1] * suly1 + pk2alfa[index2] * suly2);
        }

        // back wheel
        if (abs(pk4alfa[index1] - pk4alfa[index2]) > 220) {
            // pmot->kor4.alfa = Alfaszorzo_rec * pk4alfa[index2];
            if (pk4alfa[index1] > pk4alfa[index2]) {
                int fixedindex1 = pk4alfa[index1] - 250;
                pmot->kor4.alfa = Alfaszorzo_rec * (fixedindex1 * suly1 + pk4alfa[index2] * suly2);
            } else {
                int fixedindex2 = pk4alfa[index2] - 250;
                pmot->kor4.alfa =
                    Alfaszorzo_rec * (pk4alfa[index1] * suly1 * suly1 + fixedindex2 * suly2);
            }
        } else {
            pmot->kor4.alfa = Alfaszorzo_rec * (pk4alfa[index1] * suly1 + pk4alfa[index2] * suly2);
        }

    } else {
        pmot->kor1.alfa = Alfaszorzo_16_rec * pk1alfa[index2];
        pmot->kor2.alfa = Alfaszorzo_rec * pk2alfa[index2];
        pmot->kor4.alfa = Alfaszorzo_rec * pk4alfa[index2];
    }

    // filebair( index2, pk2alfa[index2], pmot->kor2.alfa );
    phinfo->gaz = char(pgazhatra[index1] & 1);
    pmot->hatra_f = (pgazhatra[index1] & 2) >> 1;
    Aafogo = (pgazhatra[index1] & 4) >> 2;
    Kozelvannak = (pgazhatra[index1] & 8) >> 3;

    /*if( Mostirdki ) {
        char tmp[100];
        int tmpfreki = pfrekvencia[index1];
        sprintf( tmp, "f:%d, index1:%d", tmpfreki, index1 );
        hiba( "There is result here:", tmp );
    }*/

    phinfo->frekvencia = Frekvszorzo_rec * pfrekvencia[index1] + 1.0;
    phinfo->surlero = Surlszorzo_rec * psurlero[index1];

    return 1;
}

static const double Reclepesido = 1.0 / Recorderszorzo;

void recorder::store(motorst* pmot, double ido, hanginfo* phinfo) {
    if (!legkozindex) {
        ucso_r = pmot->kor1.r;
        ucsot = -0.00000000001;
        legkozt = 0.0;
    }
    if (ido < legkozt) {
        ucso_r = pmot->kor1.r;
        ucsot = ido;
        return;
    }

    while (1) {
#ifdef TEST
        if (ido - ucsot == 0.0) {
            hiba("ido-ucsot == 0.0!");
        }
#endif
        vekt2 kor1r = (pmot->kor1.r - ucso_r) * ((legkozt - ucsot) / (ido - ucsot)) + ucso_r;

        if (legkozindex >= Darabszam) {
            // full
            return;
        }

        // write into bejegyzes
        int i = legkozindex;
        pk1rx[i] = kor1r.x;
        pk1ry[i] = kor1r.y;
        pk2rx[i] = (pmot->kor2.r.x - pmot->kor1.r.x) * Korrszorzo;
        pk2ry[i] = (pmot->kor2.r.y - pmot->kor1.r.y) * Korrszorzo;
        pk4rx[i] = (pmot->kor4.r.x - pmot->kor1.r.x) * Korrszorzo;
        pk4ry[i] = (pmot->kor4.r.y - pmot->kor1.r.y) * Korrszorzo;
        pvezrx[i] = (pmot->vezetor.x - pmot->kor1.r.x) * Korrszorzo;
        pvezry[i] = (pmot->vezetor.y - pmot->kor1.r.y) * Korrszorzo;

        // alphas
        double alfa1 = pmot->kor1.alfa;
        while (alfa1 <= 0) {
            alfa1 += K_2pi;
        }
        while (alfa1 > K_2pi) {
            alfa1 -= K_2pi;
        }
        pk1alfa[i] = alfa1 * Alfaszorzo_16;

        if (pmot->kor2.alfa <= 0) {
            pk2alfa[i] = (pmot->kor2.alfa + K_2pi) * Alfaszorzo;
        } else {
            pk2alfa[i] = pmot->kor2.alfa * Alfaszorzo;
        }
        if (pmot->kor4.alfa <= 0) {
            pk4alfa[i] = (pmot->kor4.alfa + K_2pi) * Alfaszorzo;
        } else {
            pk4alfa[i] = pmot->kor4.alfa * Alfaszorzo;
        }

        // put random number into 4 highest bits
        double tmpb = pmot->kor4.alfa + i * 5.752364;
        memcpy(&pgazhatra[i], (&tmpb) + 4, 1);
        pgazhatra[i] = pgazhatra[i] & 0xf0;

        if (phinfo->gaz) {
            pgazhatra[i] += 1;
        }
        if (pmot->hatra_f) {
            pgazhatra[i] += 2;
        }
        if (Aafogo) {
            pgazhatra[i] += 4;
        }
        if (Kozelvannak) {
            pgazhatra[i] += 8;
        }

        // pgazhatra[i] = char( phinfo->gaz + (pmot->hatra<<1) );

        if (phinfo->frekvencia < 1.0) {
            phinfo->frekvencia = 1.0;
        }
        pfrekvencia[i] = Frekvszorzo * (phinfo->frekvencia - 1.0);
        psurlero[i] = Surlszorzo * phinfo->surlero;

        legkozindex++;
        legkozt += Reclepesido;
        if (ido < legkozt) {
            ucso_r = pmot->kor1.r;
            ucsot = ido;
            betoltve = legkozindex;
            return;
        }
    }
}

/*void recorder::store( motorst* pmot, double dt, hanginfo* phinfo ) {
    int index = Recorderszorzo*dt;
    if( index < 0 )
        index = 0;
    if( index >= Darabszam )
        // betelt:
        return;
    if( index < betoltve-1 )
        hiba( "recorder::store-ban index < betoltve-1!" );

    for( int i = betoltve; i <= index; i++ ) {
        pk1rx[i] = pmot->kor1.r.x;
        pk1ry[i] = pmot->kor1.r.y;
        pk2rx[i] = (pmot->kor2.r.x-pmot->kor1.r.x)*Korrszorzo;
        pk2ry[i] = (pmot->kor2.r.y-pmot->kor1.r.y)*Korrszorzo;
        pk4rx[i] = (pmot->kor4.r.x-pmot->kor1.r.x)*Korrszorzo;
        pk4ry[i] = (pmot->kor4.r.y-pmot->kor1.r.y)*Korrszorzo;
        pvezrx[i] = (pmot->vezetor.x-pmot->kor1.r.x)*Korrszorzo;
        pvezry[i] = (pmot->vezetor.y-pmot->kor1.r.y)*Korrszorzo;

        // torlendo
        //int hatar = 2000;
        //if( pk2rx[i] > hatar || pk4rx[i] > hatar ||
        //	pk2ry[i] > hatar || pk4ry[i] > hatar )
        //	uzenet( "Greater than 5000" );
        //if( pk2rx[i] < -hatar || pk4rx[i] < -hatar ||
        //	pk2ry[i] < -hatar || pk4ry[i] < -hatar )
        //	uzenet( "Less than -5000" );

        // Alfak:
        double alfa1 = pmot->kor1.alfa;
        while( alfa1 <= 0 )
            alfa1 += K_2pi;
        while( alfa1 > K_2pi )
            alfa1 -= K_2pi;
        pk1alfa[i] = alfa1*Alfaszorzo_16;

        if( pmot->kor2.alfa <= 0 )
            pk2alfa[i] = (pmot->kor2.alfa + K_2pi)*Alfaszorzo;
        else
            pk2alfa[i] = pmot->kor2.alfa*Alfaszorzo;
        if( pmot->kor4.alfa <= 0 )
            pk4alfa[i] = (pmot->kor4.alfa + K_2pi)*Alfaszorzo;
        else
            pk4alfa[i] = pmot->kor4.alfa*Alfaszorzo;

        if( phinfo->gaz )
            pgazhatra[i] = 1;
        else
            pgazhatra[i] = 0;

        if( pmot->hatra )
            pgazhatra[i] += 2;
        if( Aafogo )
            pgazhatra[i] += 4;
        if( Kozelvannak )
            pgazhatra[i] += 8;

        //pgazhatra[i] = char( phinfo->gaz + (pmot->hatra<<1) );

        if( phinfo->frekvencia > 2.0 )
            phinfo->frekvencia = 2.0;
        pfrekvencia[i] = Frekvszorzo * phinfo->frekvencia;
        psurlero[i]    = Surlszorzo  * phinfo->surlero;
    }
    betoltve = index+1;
}*/

void recorder::addhang(double t, char wavindex, double hangero, int objszam) {
    if (hangbetoltve >= Hangdarabszam) {
        return;
    }
    if (hangbetoltve > 0) {
        if (phangok[unsigned(hangbetoltve - 1)].t > t + 0.00001) {
            char tmp[50];
            double t2 = phangok[unsigned(hangbetoltve - 1)].t;
            sprintf(tmp, "t1: kulonbseg: %f\n", float(t - t2));
            hiba(tmp);
            // hiba( "reverse time in recorder::addhang!" );
        }
    }
    phangok[unsigned(hangbetoltve)].t = t;
    phangok[unsigned(hangbetoltve)].wavindex = wavindex;
    phangok[unsigned(hangbetoltve)].hangero = hangero;
    phangok[unsigned(hangbetoltve)].objszam = (short)objszam;
    hangbetoltve++;
}

// returns true if sound is enabled
int recorder::gethang(double t, char* pwavindex, double* phangero, int* pobjszam) {
    if (hangkov < hangbetoltve) {
        if (phangok[unsigned(hangkov)].t <= t) {
            *pwavindex = phangok[unsigned(hangkov)].wavindex;
            *phangero = phangok[unsigned(hangkov)].hangero;
            *pobjszam = phangok[unsigned(hangkov)].objszam;
            hangkov++;
            return 1;
        }
    }
    return 0;
}

static void olvhiba(char* nev) { hiba( "Cannot read from recorded file!: ", nev ); }

// open from res file in case of demo
// returns 0 or belyeg
long recorder::load(char* nev, FILE* h, int demo) {
    int megvoltnyitva = 0;
    if (h) {
        megvoltnyitva = 1;
    }

    if (!h) {
        if (demo) {
            h = qopen(nev, "rb");
            if (!h) {
                hiba("Could not open recorded file!: ", nev);
            }
        } else {
            char tmpnev[40];
            sprintf(tmpnev, "rec/%s", nev);
            h = fopen(tmpnev, "rb");
            if (!h) {
                uzenet("Could not open for reading record file!:", tmpnev);
            }
        }
    }

    betoltve = 0;
    if (fread(&betoltve, 1, 4, h) != 4) {
        olvhiba(nev);
    }
    if (betoltve <= 0) {
        hiba("betoltve wrong in recorded file!: ", nev);
    }

    if (betoltve > Darabszam) {
        hiba("betoltve > Darabszam in recorder load!");
    }

    long verzio = 0;
    if (fread(&verzio, 1, 4, h) != 4) {
        hiba( "Cannot read from recorded file!: ", nev );
    }

    // need to adjust in loadrec as well
    if (verzio < 131) {
        uzenet("Recorded file version is too old!", nev);
    }

    if (verzio > 131) {
        uzenet("Recorded file version is too new!", nev);
    }

    int tmp = 0; // Multirec, but loadrec already took care of this
    if (fread(&tmp, 1, 4, h) != 4) {
        hiba( "Cannot read from recorded file!: ", nev );
    }

    if (fread(&tag, 1, 4, h) != 4) {
        hiba( "Cannot read from recorded file!: ", nev );
    }

    long belyeg = 0;
    if (fread(&belyeg, 1, 4, h) != 4) {
        olvhiba(nev);
    }
    if (fread(palyanev, 1, 16, h) != 16) {
        olvhiba(nev);
    }

    int hosszf = betoltve * sizeof(float);
    // int hosszi = betoltve*sizeof( int );
    int hosszc = betoltve * sizeof(char);
    int hosszs = betoltve * sizeof(short);

    if (fread(pk1rx, 1, hosszf, h) != hosszf) {
        olvhiba(nev);
    }
    if (fread(pk1ry, 1, hosszf, h) != hosszf) {
        olvhiba(nev);
    }
    if (fread(pk2rx, 1, hosszs, h) != hosszs) {
        olvhiba(nev);
    }
    if (fread(pk2ry, 1, hosszs, h) != hosszs) {
        olvhiba(nev);
    }
    if (fread(pk4rx, 1, hosszs, h) != hosszs) {
        olvhiba(nev);
    }
    if (fread(pk4ry, 1, hosszs, h) != hosszs) {
        olvhiba(nev);
    }
    if (fread(pvezrx, 1, hosszs, h) != hosszs) {
        olvhiba(nev);
    }
    if (fread(pvezry, 1, hosszs, h) != hosszs) {
        olvhiba(nev);
    }

    if (fread(pk1alfa, 1, hosszs, h) != hosszs) {
        olvhiba(nev);
    }
    if (fread(pk2alfa, 1, hosszc, h) != hosszc) {
        olvhiba(nev);
    }
    if (fread(pk4alfa, 1, hosszc, h) != hosszc) {
        olvhiba(nev);
    }
    if (fread(pgazhatra, 1, hosszc, h) != hosszc) { // character
        olvhiba(nev);
    }

    if (fread(pfrekvencia, 1, hosszc, h) != hosszc) {
        olvhiba(nev);
    }
    if (fread(psurlero, 1, hosszc, h) != hosszc) {
        olvhiba(nev);
    }

    // Hangok:
    if (fread(&hangbetoltve, 1, 4, h) != 4) {
        olvhiba(nev);
    }
    if (hangbetoltve < 0) {
        hiba("hangbetoltve wrong in load!");
    }
    if (hangbetoltve > Hangdarabszam) {
        hiba("hangbetoltve > Hangdarabszam! in rec load");
    }

    /*static int elsoszam = 0;
    if( elsoszam ) {
        char tmp11[50];
        sprintf( tmp11, "1.: %d, 2.: %d", hangbetoltve, elsoszam );
        uzenet( tmp11 );
    }
    elsoszam = hangbetoltve;*/

    int hossz = hangbetoltve * sizeof(egyhang);
    if (fread(phangok, 1, hossz, h) != hossz) {
        olvhiba(nev);
    }

    long magic = 0;
    if (fread(&magic, 1, 4, h) != 4) {
        olvhiba(nev);
    }
    if (magic != MAGICNUMBER) {
        hiba("magic != MAGICNUMBER");
    }

    if (!megvoltnyitva) {
        if (demo) {
            qclose(h);
        } else {
            fclose(h);
        }
    }

    // if( !jorechossza( this ) )
    //	hiba( "!jorechossza( this ) in recorder::load!" );
    return belyeg;
}

/* in 1.0 and 1.1
    h = fopen( nev, "wb" );
    if( !h )
        hiba( "Cannot open recorded file for write!: ", nev );

    if( fwrite( &betoltve, 1, 4, h ) != 4 )
        hiba( "Cannot write into recorded file!: ", nev );
    if( fwrite( &level, 1, 4, h ) != 4 )
        hiba( "Cannot write into recorded file!: ", nev );


    long hossz = betoltve*sizeof( float );

    if( fwrite( pk1rx, 1, unsigned(hossz), h ) != hossz )
        hiba( "Cannot write into recorded file!: ", nev );
*/

static void irhiba(char* nev) { hiba( "Cannot write into recorded file!: ", nev ); }

void recorder::save(char* nev, FILE* h, long belyeg, int tag_p) {
    tag = tag_p;

    int megvoltnyitva = 0;
    if (h) {
        megvoltnyitva = 1;
    }

    if (betoltve == 0) {
        hiba("save-ben betoltve == 0!");
    }

    if (!h) {
        // leave fopen here
        char tmpnev[40];
        sprintf(tmpnev, "rec/%s", nev);
        h = fopen(tmpnev, "wb");
        if (!h) {
            hiba("Could not open for writing record file!: ", tmpnev);
        }
    }

    if (fwrite(&betoltve, 1, 4, h) != 4) {
        irhiba(nev);
    }

    // Verzio is only here after ASC 1.2(?)
    // previously it was level which is always less than 120
    long verzio = 131;
    if (fwrite(&verzio, 1, 4, h) != 4) {
        irhiba(nev);
    }

    // this is also read in loadrec, so it should stay after Verzio
    if (fwrite(&Multirec, 1, 4, h) != 4) {
        irhiba(nev);
    }

    if (fwrite(&tag, 1, 4, h) != 4) {
        irhiba(nev);
    }

    if (fwrite(&belyeg, 1, 4, h) != 4) {
        irhiba(nev);
    }

    if (fwrite(palyanev, 1, 16, h) != 16) {
        irhiba(nev);
    }

    int hosszf = betoltve * sizeof(float);
    // int hosszi = betoltve*sizeof( int );
    int hosszc = betoltve * sizeof(char);
    int hosszs = betoltve * sizeof(short);

    if (fwrite(pk1rx, 1, hosszf, h) != hosszf) {
        irhiba(nev);
    }
    if (fwrite(pk1ry, 1, hosszf, h) != hosszf) {
        irhiba(nev);
    }
    if (fwrite(pk2rx, 1, hosszs, h) != hosszs) {
        irhiba(nev);
    }
    if (fwrite(pk2ry, 1, hosszs, h) != hosszs) {
        irhiba(nev);
    }
    if (fwrite(pk4rx, 1, hosszs, h) != hosszs) {
        irhiba(nev);
    }
    if (fwrite(pk4ry, 1, hosszs, h) != hosszs) {
        irhiba(nev);
    }

    if (fwrite(pvezrx, 1, hosszs, h) != hosszs) {
        irhiba(nev);
    }
    if (fwrite(pvezry, 1, hosszs, h) != hosszs) {
        irhiba(nev);
    }

    if (fwrite(pk1alfa, 1, hosszs, h) != hosszs) {
        irhiba(nev);
    }
    if (fwrite(pk2alfa, 1, hosszc, h) != hosszc) {
        irhiba(nev);
    }
    if (fwrite(pk4alfa, 1, hosszc, h) != hosszc) {
        irhiba(nev);
    }
    if (fwrite(pgazhatra, 1, hosszc, h) != hosszc) {
        irhiba(nev);
    }

    if (fwrite(pfrekvencia, 1, hosszc, h) != hosszc) {
        irhiba(nev);
    }
    if (fwrite(psurlero, 1, hosszc, h) != hosszc) {
        irhiba(nev);
    }

    // sounds
    if (fwrite(&hangbetoltve, 1, 4, h) != 4) {
        irhiba(nev);
    }
    int hossz = hangbetoltve * sizeof(egyhang);
    if (fwrite(phangok, 1, hossz, h) != hossz) {
        irhiba(nev);
    }

    long magic = MAGICNUMBER;
    if (fwrite(&magic, 1, 4, h) != 4) {
        irhiba(nev);
    }

    if (!megvoltnyitva) {
        fclose(h);
    }
}

// open from res file in case of demo
long loadrecek(char* nev, int demo) {
    // first check if multirec or not
    FILE* h;
    if (demo) {
        h = qopen(nev, "rb");
        if (!h) {
            uzenet("Could not open for reading record file!: !: ", nev);
        }
    } else {
        char tmpnev[40];
        sprintf(tmpnev, "rec/%s", nev);
        h = fopen(tmpnev, "rb");
        if (!h) {
            uzenet("Could not open for reading record file!:", tmpnev);
        }
    }
    int tmp = 0; // loaded (darabszam is recorded)
    if (fread(&tmp, 1, 4, h) != 4) {
        hiba( "Cannot read from recorded file!: ", nev );
    }

    long verzio = 0;
    if (fread(&verzio, 1, 4, h) != 4) {
        hiba( "Cannot read from recorded file!: ", nev );
    }

    if (verzio < 131) {
        uzenet("Recorded file version is too old!", nev);
    }

    if (verzio > 131) {
        uzenet("Recorded file version is too new!", nev);
    }

    // load Multirec
    if (fread(&Multirec, 1, 4, h) != 4) {
        hiba( "Cannot read from recorded file!: ", nev );
    }

    if (demo) {
        qclose(h);
    } else {
        fclose(h);
    }

    // actual reading
    long belyeg = 0;
    if (Multirec) {
        FILE* h;
        if (demo) {
            h = qopen(nev, "rb");
            if (!h) {
                uzenet("Could not open for reading demo record file!:", nev);
            }
        } else {
            char tmpnev[40];
            sprintf(tmpnev, "rec\\%s", nev);
            h = fopen(tmpnev, "rb");
            if (!h) {
                uzenet("Could not open for reading record file!:", tmpnev);
            }
        }

        belyeg = Prec1->load(nev, h, demo);
        Prec2->load(nev, h, demo);

        if (demo) {
            qclose(h);
        } else {
            fclose(h);
        }
    } else {
        belyeg = Prec1->load(nev, NULL, demo);
    }

    return belyeg;
}

// returns 0 or belyeg
void saverecek(char* nev, long belyeg, int tag) {
    if (Multirec) {
        char tmpnev[40];
        sprintf(tmpnev, "rec/%s", nev);
        FILE* h = fopen(tmpnev, "wb");
        if (!h) {
            uzenet("Could not open for writing record file!: !: ", tmpnev);
        }

        // multirec field is known from Multirec variable
        Prec1->save(nev, h, belyeg, tag);
        Prec2->save(nev, h, belyeg, tag);

        fclose(h);
    } else {
        Prec1->save(nev, NULL, belyeg, tag);
    }
}

// WAV collector

static int Begyujtve = 0;

#define GYUJTODARAB (200)

static int Azonosito[GYUJTODARAB];
static double Hangero[GYUJTODARAB];
static int Objszam[GYUJTODARAB];

void startwavegyujto(int wavazonosito, double hangero, int objszam) {
    if (Begyujtve < GYUJTODARAB) {
        Azonosito[Begyujtve] = wavazonosito;
        Hangero[Begyujtve] = hangero;
        Objszam[Begyujtve] = objszam;
        Begyujtve++;
    }
}

void resetwavegyujto(void) { Begyujtve = 0; }

// returns true if there is wav
int getwavegyujto(int* pwavazonosito, double* phangero, int* pobjszam) {
    if (Begyujtve == 0) {
        return 0;
    }
    *pwavazonosito = Azonosito[0];
    *phangero = Hangero[0];
    *pobjszam = Objszam[0];
    Begyujtve--;
    for (int i = 0; i < Begyujtve; i++) {
        Azonosito[i] = Azonosito[i + 1];
        Hangero[i] = Hangero[i + 1];
        Objszam[i] = Objszam[i + 1];
    }
    return 1;
}
