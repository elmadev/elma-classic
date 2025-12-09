#include "all.h"

static void surlodasverseny(motorst* pmot, vekt2 fgumi, vekt2 sebesseg);

static double Maxsurlodas = 0;

static void szogigazit(double* pd) {
    if (*pd < -K_pi) {
        *pd += 2 * K_pi;
    }
    if (*pd > K_pi) {
        *pd -= 2 * K_pi;
    }
}

/*static double Elmozdhatar1 = 10.4;
static double Elmozdhatar2 = 10.5;
static double Atmenet = Elmozdhatar2 - Elmozdhatar1;
static double Szorzo = 2.0;

static pic8* Ppic8 = NULL;
*/

static void erokszamitasa(motorst* pmot, kor* pkor, vekt2 i1, vekt2 j1, double kordx, double kordy,
                          vekt2* pFkerek, vekt2* pFtest, double* pMtest, double* pMkerek) {
    vekt2 gumis = i1 * kordx + j1 * kordy;
    vekt2 gumisabsz = gumis + pmot->kor1.r;

    // Gumi huzoero kiszamolasa:
    vekt2 gumi = gumisabsz - pkor->r;
    double Fsugar = 0;
    double Ftang = 0;
    if (gumi.x < -0.0001 || gumi.x > 0.0001 || gumi.y < -0.0001 || gumi.y > 0.0001) {
        // Gumieronek szetbontasa ket komponensre:
        double rudhossz = abs(gumis);
        vekt2 rudegys = gumis * (1.0 / rudhossz);
        vekt2 rudegysmer = forgatas90fokkal(rudegys);

        Fsugar = (gumi * rudegys) * Drsugar;
        Ftang = (gumi * rudegysmer) * Drsugar;

        *pFkerek = Fsugar * rudegys + Ftang * rudegysmer;

        *pFtest = Vekt2null - *pFkerek;
        *pMtest = -Ftang * rudhossz;
    } else {
        *pFkerek = vekt2();
        *pFtest = vekt2();
        *pMtest = 0;
    }
    // Most jon surlodas szamitasa:
    vekt2 koto = pkor->r - pmot->kor1.r;
    double kotol = abs(koto);
    double reckotol = 1.0 / kotol;
    vekt2 kotoe = koto * reckotol;
    vekt2 kotomer = forgatas90fokkal(koto);
    vekt2 kotoemer = forgatas90fokkal(kotoe);
    vekt2 korongrelv = (kotomer * pmot->kor1.omega + pmot->kor1.v) - pkor->v;
    double vlong = korongrelv * kotoe;
    double vtang = korongrelv * kotoemer;
    vekt2 Fklong = (vlong * Sr) * kotoe;
    vekt2 Fktang = (vtang * Sr) * kotoemer;

    // Most jon kerek forgatonyomateka:
    vekt2 Ftestnyom = kotoemer * (*pMkerek * reckotol);

    *pFkerek = *pFkerek + Fklong + Fktang - Ftestnyom;
    *pMtest += -(Fktang * kotomer);
    *pFtest = *pFtest - Fklong - Fktang + Ftestnyom;

    surlodasverseny(pmot, gumi, korongrelv);
    // surlodasverseny( Ftang, vtang );
}

// static Voltfek = 0;
// static double Dfek2 = 0, Dfek4 = 0;
static double Loket = 12.0;
static double Omegavalt = 3.0;

// static int Ugrasban1 = 1, Ugrasban2 = 1; // Ezeket nullazni kell minden kezdesnel!
// static double Ugras1kezd = -1.0, Ugras2kezd = -1.0;
// static double Kezdoomega1 = -1.0, Kezdoomega2 = -1.0;

void resetleptet(motorst* pmot) {
    pmot->voltfek = 0;
    pmot->dfek2 = pmot->dfek4 = 0;
    pmot->ugrasban1 = pmot->ugrasban2 = 0;
    pmot->ugras1kezd = pmot->ugras2kezd = -1.0;
    pmot->kezdoomega1 = pmot->kezdoomega2 = -1.0;
}

// Kor1-bol eloallitja Fejr-et!:
void szamitfejr(motorst* pmot) {
    vekt2 i(cos(pmot->kor1.alfa), sin(pmot->kor1.alfa));
    vekt2 j = forgatas90fokkal(i);

    if (pmot->hatra_f) {
        pmot->fejr = pmot->vezetor + i * 0.09 + j * 0.63;
    } else {
        pmot->fejr = pmot->vezetor - i * 0.09 + j * 0.63;
    }
}

void leptet(motorst* pmot, double most, double dt, int gaz, int fek, int ugrik1, int ugrik2) {
    Maxsurlodas = 0;

    vekt2 i1(cos(pmot->kor1.alfa), sin(pmot->kor1.alfa));
    vekt2 j1 = forgatas90fokkal(i1);

    // Fek, gaz:
    if (!pmot->voltfek && fek) {
        pmot->dfek2 = pmot->kor2.alfa - pmot->kor1.alfa;
        pmot->dfek4 = pmot->kor4.alfa - pmot->kor1.alfa;
    }
    pmot->voltfek = fek;
    double Mkerek2 = 0;
    double Mkerek4 = 0;
    if (gaz) {
        double tulporgesomega = 110.0;
        double gaznyomatek = 600.0;
        if (pmot->hatra_f) {
            if (pmot->kor2.omega > -tulporgesomega) {
                Mkerek2 = -gaznyomatek;
            }
        } else {
            if (pmot->kor4.omega < tulporgesomega) {
                Mkerek4 = gaznyomatek;
            }
        }
    }
    if (fek) {
        double fekero = 1000.0;
        double surlodas = 100.0;

        double dalfa = pmot->kor2.alfa - (pmot->kor1.alfa + pmot->dfek2);
        double domega = pmot->kor2.omega - pmot->kor1.omega;
        Mkerek2 = -fekero * dalfa - surlodas * domega;

        dalfa = pmot->kor4.alfa - (pmot->kor1.alfa + pmot->dfek4);
        domega = pmot->kor4.omega - pmot->kor1.omega;
        Mkerek4 = -fekero * dalfa - surlodas * domega;
    } else {
        szogigazit(&pmot->kor2.alfa);
        szogigazit(&pmot->kor4.alfa);
    }

    int crcido = most * 1000;

    vekt2 Fkerek2;
    vekt2 Ftest2;
    double Mtest2;
    erokszamitasa(pmot, &pmot->kor2, i1, j1, Kord2x, Kord2y, &Fkerek2, &Ftest2, &Mtest2, &Mkerek2);

    vekt2 Fkerek4;
    vekt2 Ftest4;
    double Mtest4;
    erokszamitasa(pmot, &pmot->kor4, i1, j1, Kord4x, Kord4y, &Fkerek4, &Ftest4, &Mtest4, &Mkerek4);

    // Ugras elintezese:
    // Eloszor ugras befejezese, ha kell:
    double oldomega;
    if (ugrik1 || ugrik2) {
        oldomega = pmot->kor1.omega;
    }

    if (pmot->ugrasban1 && (ugrik1 || ugrik2 || most > pmot->ugras1kezd + Ugroturelem * 0.25)) {
        pmot->kor1.omega += Loket;
        if (pmot->kor1.omega > pmot->kezdoomega1) {
            pmot->kor1.omega = pmot->kezdoomega1;
        }
        // Meg marado szogsebesseg valtozast is okozunk, ha nem forgatjuk tul:
        if (pmot->kor1.omega > 0.0) {
            pmot->kor1.omega -= Omegavalt;
            if (pmot->kor1.omega < 0.0) {
                pmot->kor1.omega = 0.0;
            }
        }
        pmot->ugrasban1 = 0;
        pmot->kezdoomega1 = -1.0;
        pmot->ugras1kezd = -1.0;
    }
    if (pmot->ugrasban2 && (ugrik1 || ugrik2 || most > pmot->ugras2kezd + Ugroturelem * 0.25)) {
        pmot->kor1.omega -= Loket;
        if (pmot->kor1.omega < pmot->kezdoomega2) {
            pmot->kor1.omega = pmot->kezdoomega2;
        }
        // Meg marado szogsebesseg valtozast is okozunk, ha nem forgatjuk tul:
        if (pmot->kor1.omega < 0.0) {
            pmot->kor1.omega += Omegavalt;
            if (pmot->kor1.omega > 0.0) {
                pmot->kor1.omega = 0.0;
            }
        }
        pmot->ugrasban2 = 0;
        pmot->kezdoomega2 = -1.0;
        pmot->ugras2kezd = -1.0;
    }

    // if( crcido % 511 == 103 )
    //   crccheck2(); Most ez nincs is belinkelve

    // Most ugras kezdes, ha kell:
    if (ugrik1) {
        // Csak azert van kikomentezve, mert egyszer elojott:
        // if( Ugrasban1 || Ugrasban2 )
        //  hiba( "Ugrasban!" );
        pmot->ugrasban1 = 1;
        pmot->kezdoomega1 = pmot->kor1.omega;
        pmot->ugras1kezd = most;
        pmot->kor1.omega -= Loket;
    }
    if (ugrik2) {
        // if( Ugrasban1 || Ugrasban2 )
        //   hiba( "Ugrasban!" );
        pmot->ugrasban2 = 1;
        pmot->kezdoomega2 = pmot->kor1.omega;
        pmot->ugras2kezd = most;
        pmot->kor1.omega += Loket;
    }
    if (ugrik1 || ugrik2) {
        // Vezetot kulon forgatjuk meg:
        double domega = pmot->kor1.omega - oldomega;
        vekt2 tangens = forgatas90fokkal(pmot->vezetor - pmot->kor1.r);
        pmot->vezetov = pmot->vezetov + tangens * domega;
    }

    /*
    beallit( &pmot->kor1, Ftest2+Ftest4-Vekt2j*pmot->kor1.m*G,
                    Mtest2+Mtest4, dt, 0 );
    beallit( &pmot->kor2, Fkerek2-Vekt2j*pmot->kor2.m*G, Mkerek2,      dt, 1 );
    beallit( &pmot->kor4, Fkerek4-Vekt2j*pmot->kor4.m*G, Mkerek4,      dt, 1 );
    */

    switch (pmot->gravirany) {
    case 1:                                                // Lefele:
        beallitvezeto(pmot, vekt2(0.0, -1.0), i1, j1, dt); // vezeto
        beallit(&pmot->kor1, Ftest2 + Ftest4 - Vekt2j * pmot->kor1.m * G, Mtest2 + Mtest4, dt, 0);
        beallit(&pmot->kor2, Fkerek2 - Vekt2j * pmot->kor2.m * G, Mkerek2, dt, 1);
        beallit(&pmot->kor4, Fkerek4 - Vekt2j * pmot->kor4.m * G, Mkerek4, dt, 1);
        break;
    case 0:                                               // Felfele:
        beallitvezeto(pmot, vekt2(0.0, 1.0), i1, j1, dt); // vezeto
        beallit(&pmot->kor1, Ftest2 + Ftest4 + Vekt2j * pmot->kor1.m * G, Mtest2 + Mtest4, dt, 0);
        beallit(&pmot->kor2, Fkerek2 + Vekt2j * pmot->kor2.m * G, Mkerek2, dt, 1);
        beallit(&pmot->kor4, Fkerek4 + Vekt2j * pmot->kor4.m * G, Mkerek4, dt, 1);
        break;
    case 2:                                                // Balra:
        beallitvezeto(pmot, vekt2(-1.0, 0.0), i1, j1, dt); // vezeto
        beallit(&pmot->kor1, Ftest2 + Ftest4 - Vekt2i * pmot->kor1.m * G, Mtest2 + Mtest4, dt, 0);
        beallit(&pmot->kor2, Fkerek2 - Vekt2i * pmot->kor2.m * G, Mkerek2, dt, 1);
        beallit(&pmot->kor4, Fkerek4 - Vekt2i * pmot->kor4.m * G, Mkerek4, dt, 1);
        break;
    case 3:                                               // Jobbra:
        beallitvezeto(pmot, vekt2(1.0, 0.0), i1, j1, dt); // vezeto
        beallit(&pmot->kor1, Ftest2 + Ftest4 + Vekt2i * pmot->kor1.m * G, Mtest2 + Mtest4, dt, 0);
        beallit(&pmot->kor2, Fkerek2 + Vekt2i * pmot->kor2.m * G, Mkerek2, dt, 1);
        beallit(&pmot->kor4, Fkerek4 + Vekt2i * pmot->kor4.m * G, Mkerek4, dt, 1);
        break;
    }
    /* Bicigli
    if( gaz ) {
        static double attetel = 0.33;
        if( pmot->hatra )
            pmot->labalfa += pmot->kor2.omega*dt*attetel;
        else
            pmot->labalfa -= pmot->kor4.omega*dt*attetel;
    } */

    szamitfejr(pmot);
}

// 0-meghalt, 2-semmi kulonos
int vizsgalat(motorst* pmot) {
    vekt2 t1, t2;
    vonal *pv1, *pv2;
    if (talppontkereses(pmot->fejr, Fejsugar, &t1, &t2, &pv1, &pv2)) {
        return 0;
    }

    // Objektumokkal utkozesvizsgalat:
    int voltkaja = 1;
    while (voltkaja) { // Ha nem volt kaja kilepunk, kulonben vegtelen ciklus
        voltkaja = 0;
        int sorszamtomb[3];
        sorszamtomb[0] = utkozikesprite(pmot->kor2.r, pmot->kor2.sugar);
        sorszamtomb[1] = utkozikesprite(pmot->kor4.r, pmot->kor4.sugar);
        sorszamtomb[2] = utkozikesprite(pmot->fejr, Fejsugar);
        for (int i = 0; i < 3; i++) {
            if (sorszamtomb[i] >= 0) {
                startwavegyujto(0, 0.0, sorszamtomb[i]);
                kerek* pker = Ptop->getptrkerek(sorszamtomb[i]);
                if (pker->tipus == T_KAJA) {
                    pker->aktiv = 0;
                    voltkaja = 1; // Hatha van meg tobb kaja is
                }
            }
        }
    }

    return 2;
}

int elsomegoltemasodikat(motorst* pmot1, motorst* pmot2) {
    if (absnegyzet(pmot2->fejr - pmot1->kor2.r) < Fejkerektavnegyzet) {
        return 1;
    }
    if (absnegyzet(pmot2->fejr - pmot1->kor4.r) < Fejkerektavnegyzet) {
        return 1;
    }
    return 0;
}

static double Oszto = 1.0 / 1.0;

static void surlodasverseny(motorst* pmot, vekt2 fgumi_v, vekt2 sebesseg_v) {
    vekt2 joirany(cos(pmot->kor1.alfa - K_pip2), sin(pmot->kor1.alfa - K_pip2));
    double fgumi = joirany * fgumi_v;
    double sebesseg = joirany * sebesseg_v;
    if (fgumi <= 0 || sebesseg <= 0) {
        return;
    }
    double ertek = fgumi * sebesseg * Oszto;
    if (ertek > Maxsurlodas) {
        Maxsurlodas = ertek;
    }
}

double kiszamolsurlodast(void) { return Maxsurlodas; }
