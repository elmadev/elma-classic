#include "all.h"

motorst Motorst1, Motorst2; // Ez nem hasznalando
motorst *Pmot1 = &Motorst1, *Pmot2 = &Motorst2;

double Elszakadasisebhat, Belsosav, G;
double Talppontegybeolvadasitav;

double Ugrassebesseg1, Ugrassebesseg2;
double Ugroturelem;
double Vegenvaras;

double Magassag = 2, Szelesseg = 0.1;

double Drsugar, Drtang, Sr;

vekt2 P_alap1, P_alap2, P_alap3, P_alap4, P_alap6;
double Fejsugar;

double Objektumsugar = 0.4;

// Hajszalnyival kisebb kor.sugarnal, hogy kirajzolasnal ne logjon foldbe:
double Kerekrajzsugar = 0.395;

double Ketmaxsugar, Spritemaxsugar;

// Match-ben fej es kerek tavolsag vizsgalathoz:
double Fejkerektavnegyzet = 1.0;

double Fekegyutthato;
double Kord2x, Kord2y, Kord4x, Kord4y,
    Kord5y; // Ez az utolso vezeto helye

double Arany, Aranyrec;

/* Eredeti szamitas play.cpp-bol:
Arany = 100.0;
Arany *= Displaysizex/320.0;
Arany *= fakt;
Aranyx = Arany*320.0/640.0;
Aranyy = Aranyx*Displaysizey/Displaysizex*(640.0/480.0);*/

int Viewzoom = 10;

void initmotor(motorst* pmot) {
    pmot->hatra_f = 0;
    pmot->hatra_h = 0;
    pmot->gravirany = 1;
    pmot->voltfek = 0;
    // pmot->labalfa = 0.0; Bicigli

    // Kor1:
    strcpy(pmot->kor1.nev, "Test elso");
    pmot->kor1.alfa = 0.0;
    pmot->kor1.omega = 0.0;
    pmot->kor1.sugar = 0.3;
    pmot->kor1.m = 200; // 100;
    // theta = m * r*r;
    pmot->kor1.theta = 200.0 * 0.55 * 0.55;
    pmot->kor1.r = vekt2(2.75, 3.6);
    pmot->kor1.v = vekt2(0, 0);
    // pmot->kor1.utoljaraleert = 0;

    // Kor2:
    strcpy(pmot->kor2.nev, "Kerek elso");
    pmot->kor2.alfa = 0.0; // 30
    pmot->kor2.omega = 0.0;
    pmot->kor2.sugar = 0.4;
    pmot->kor2.m = 10;
    pmot->kor2.theta = 0.32;
    pmot->kor2.r = vekt2(1.9, 3.0);
    pmot->kor2.v = vekt2(0, 0);
    // pmot->kor2.utoljaraleert = 0;

    // Kor4:
    strcpy(pmot->kor4.nev, "Kerek hatso");
    pmot->kor4.alfa = 0.0; // 40
    pmot->kor4.omega = 0.0;
    pmot->kor4.sugar = 0.4;
    pmot->kor4.m = 10;
    pmot->kor4.theta = 0.32;
    pmot->kor4.r = vekt2(3.6, 3.0);
    pmot->kor4.v = vekt2(0, 0);
    // pmot->kor4.utoljaraleert = 0;

    pmot->vezetor = vekt2(2.75, 4.04);
    pmot->vezetov = vekt2(0.0, 0.0);
}

void initadatok(void) {
    initmotor(Pmot1);
    initmotor(Pmot2);

    double fakt = 0.48; // 0.54; volt mar 28 is
    if (State->minibike) {
        fakt = 0.30;
    }

    Arany = 100.0 * fakt;
    Aranyrec = 1.0 / Arany;

    // Viewzoom = 0.28*Arany*0.5;
    Viewzoom = 0.42 * Arany * 0.5;

    Fekegyutthato = 100.0;
    Kord2x = -0.1, Kord2y = -0.5, Kord4x = 0.5, Kord4y = -0.5;

    Elszakadasisebhat = 0.01;
    Belsosav = 0.005; // uj
    // Belsosav = 0.02; // regi
    G = 10.0; // nehezsegi gyorsulas: a [m/s^2]
    Talppontegybeolvadasitav = 0.1;

    Ugrassebesseg1 = 5;
    Ugrassebesseg2 = 5;
    // Ugroturelem = 0.2;
    Ugroturelem = 0.4;
    Vegenvaras = 1.0;

    Drsugar = 10000.0; // rudban fuggoleges modulus: F/l [N/m]
    Drtang = 10000.0;  // rudban vizszintes modulus: F/l [N/m]
    Sr = 1000.0;       // rudban mozgasi ellenallas: F/v [N*s/m]

    // MCGA kirajzolasos:
    P_alap1 = vekt2(0.2, 0.05);
    P_alap2 = vekt2(-0.6, -0.04);
    P_alap3 = vekt2(-0.05, 0.4);
    P_alap4 = vekt2(-0.3, -0.5);
    P_alap6 = vekt2(0.0, 0.6);
    Fejsugar = 0.238; // 0.26;

    Spritemaxsugar = 0.5;
    Ketmaxsugar = Spritemaxsugar + Pmot1->kor4.sugar;
    Fejkerektavnegyzet = Pmot1->kor1.sugar + Pmot1->kor2.sugar;
    Fejkerektavnegyzet = Fejkerektavnegyzet * Fejkerektavnegyzet;

    vekt2 vtmp = Pmot1->kor2.r - Pmot1->kor1.r;
    Kord2x = vtmp.x;
    Kord2y = vtmp.y;
    vtmp = Pmot1->kor4.r - Pmot1->kor1.r;
    Kord4x = vtmp.x;
    Kord4y = vtmp.y;

    Kord5y = Pmot1->vezetor.y - Pmot1->kor1.r.y;
}

/*static double atmeretez( void ) {
    FILE* h = fopen( "size.inf", "rt" );
    if( !h )
        return 1.0;
    long meret = 1000;
    if( fscanf( h, "%ld", &meret ) != 1 ) {
        fclose( h );
        return 1.0;
    }
    fclose( h );
    if( meret < 100 )
        meret = 100;
    if( meret > 10000 )
        meret = 10000;
    return (double)meret/1000.0;
} */

// reorder.h-ban friend-kent vannak deklaralva mar:
// pgazhatra-ba beepiti betoltvet:
void berakrecbehosszat(recorder* prec) {
    if (prec->betoltve < 80) {
        return;
    }
    unsigned int beirando = prec->betoltve;
    for (int i = 0; i < 32; i++) {
        prec->pgazhatra[40 + i] = prec->pgazhatra[40 + i] & 127;
        if (beirando & 1) {
            prec->pgazhatra[40 + i] += 128;
        }
        beirando = beirando >> 1;
    }
}

// pgazhatra alapjan ellenorzi hosszt:
// Ezt vegleges verziobol kiszedjuk, mert ezt csak mi ellenorizzuk Csabival:
/*int jorechossza( recorder* prec ) {
    if( prec->betoltve < 80 )
        return 1;
    unsigned int szam = 0;
    for( int i = 0; i < 32; i++ ) {
        szam *= 2;
        if( prec->pgazhatra[40+31-i] & 128 )
            szam += 1;
    }
    if( szam == prec->betoltve )
        return 1;
    else
        return 0;
} */
