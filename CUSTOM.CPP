#include "all.h"

char* kodtobillnev(int billkod);

typedef int* billmuttomb[MAXRUBRIKASZAM_TAB + 1];

static billmuttomb Pbilltomb0;
static billmuttomb Pbilltomb1;
static billmuttomb Pbilltomb2;

static int Rubrikaszam0 = 6;
static int Rubrikaszam12 = 8;

static void betolt(billmuttomb bmuttomb, int sorszam, char* szoveg, int* pbillkod) {
    strcpy(Rubrikak[sorszam], szoveg);

    char* billnev = kodtobillnev(*pbillkod);
    char tmp[20] = "";
    if (!billnev) {
        if (*pbillkod == 0) {
            sprintf(tmp, "???");
        } else {
            sprintf(tmp, "Key code: %d", *pbillkod);
        }
        billnev = tmp;
    }

    strcpy(Rubrikak_tab[sorszam], billnev);
    bmuttomb[sorszam] = pbillkod;
}

static void gombotvalaszt(int rubrikaszam, billmuttomb bmuttomb, int sorszam) {
    strcpy(Rubrikak_tab[sorszam], "_");
    valaszt2 val;
    val.kur = sorszam;
    val.egykepen = 10;
    val.x0 = 60;
    val.x0_tab = 400;
    val.y0 = 86;
    val.dy = 40;
    val.escelheto = 1;

    strcpy(val.cim, "Customize controls");
    strcpy(Rubrikak_tab[sorszam], "_");

    val.bead(rubrikaszam, 1); // tab-os kiiras

    val.valassz(NULL, 0, 1); // Csak kirajzolja kepet

    // Megnezzuk milyen gombot nyomott le:
    while (1) {
        mv_check(/*"cust.cpp"*/);
        for (int i = 1; i < 256; i++) {
            if (mk_getstate(MK_ESC)) {
                return;
            }

            if (i == 28 || i == MK_ESC) {
                continue;
            }

            if (!mk_getstate(i)) {
                continue;
            }

            // Ez a billentyu le van nyomva:

            // char tmp[10];
            // sprintf( tmp, "Bill: %d", i );
            // hiba( tmp );

            // Most meg van nyomva egy gomb:
            // Kiszedjuk mindenhonnan mashonnan ezt a billt:
            for (int j = 3; j < Rubrikaszam0; j++) { // 1-tol megy
                if (*Pbilltomb0[j] == i) {
                    *Pbilltomb0[j] = 0;
                }
            }
            for (int j = 0; j < Rubrikaszam12; j++) { // 0-tol megy
                if (*Pbilltomb1[j] == i) {
                    *Pbilltomb1[j] = 0;
                }
                if (*Pbilltomb2[j] == i) {
                    *Pbilltomb2[j] = 0;
                }
            }
            *bmuttomb[sorszam] = i;
            return;
        }
        // Golyokat leptetjuk:
        val.kirajzol();
    }
}

static void bejegyez0(void) {
    strcpy(Rubrikak[0], "Reset all controls to default");
    Rubrikak_tab[0][0] = 0;
    strcpy(Rubrikak[1], "Customize Player A");
    Rubrikak_tab[1][0] = 0;
    strcpy(Rubrikak[2], "Customize Player B");
    Rubrikak_tab[2][0] = 0;
    betolt(Pbilltomb0, 3, "Inc. Screen Size", &State->billplussz);
    betolt(Pbilltomb0, 4, "Dec. Screen Size", &State->billminusz);
    betolt(Pbilltomb0, 5, "Make a Screenshot", &State->billsnap);
}

static void bejegyez12(billmuttomb bmuttomb, jatekosopciok* pjatopc) {
    betolt(bmuttomb, 0, "Throttle", &pjatopc->billgaz);
    betolt(bmuttomb, 1, "Brake", &pjatopc->billfek);
    betolt(bmuttomb, 2, "Rotate left", &pjatopc->billugras2);
    betolt(bmuttomb, 3, "Rotate right", &pjatopc->billugras1);
    betolt(bmuttomb, 4, "Change direction", &pjatopc->billfordul);
    betolt(bmuttomb, 5, "Toggle Navigator", &pjatopc->billview);
    betolt(bmuttomb, 6, "Toggle Time", &pjatopc->billtime);
    betolt(bmuttomb, 7, "Toggle Show/Hide", &pjatopc->billshowkep);
}

static void customizeplayer(billmuttomb bmuttomb, jatekosopciok* pjatopc, char* ABjel) {
    int kurrens = 0;
    while (1) {
        valaszt2 val;
        val.kur = kurrens;
        val.egykepen = 10;
        val.x0 = 60;
        val.x0_tab = 400;
        val.y0 = 86;
        val.dy = 40;
        val.escelheto = 1;

        strcpy(val.cim, "Customize Player ");
        strcat(val.cim, ABjel);

        bejegyez12(bmuttomb, pjatopc);

        val.bead(Rubrikaszam12, 1); // tab-os kiiras

        kurrens = val.valassz();

        if (kurrens < 0) {
            return;
        }

        if (kurrens >= 0 && kurrens < Rubrikaszam12) {
            gombotvalaszt(Rubrikaszam12, bmuttomb, kurrens);
        }
    }
}

void customize(void) {
    bejegyez12(Pbilltomb1, &State->opciok1);
    bejegyez12(Pbilltomb2, &State->opciok2);

    int kurrens = 0;
    while (1) {
        valaszt2 val;
        val.kur = kurrens;
        val.egykepen = 10;
        val.x0 = 60;
        val.x0_tab = 400;
        val.y0 = 86;
        val.dy = 40;
        val.escelheto = 1;

        strcpy(val.cim, "Customize controls");

        bejegyez0();

        val.bead(Rubrikaszam0, 1); // tab-os kiiras

        kurrens = val.valassz();

        if (kurrens < 0) {
            return;
        }

        if (kurrens == 0) {
            State->resetcontrols();
            bejegyez12(Pbilltomb1, &State->opciok1);
            bejegyez12(Pbilltomb2, &State->opciok2);
            bejegyez0();
        }

        if (kurrens == 1) {
            customizeplayer(Pbilltomb1, &State->opciok1, "A");
        }

        if (kurrens == 2) {
            customizeplayer(Pbilltomb2, &State->opciok2, "B");
        }

        if (kurrens >= 3 && kurrens < Rubrikaszam0) {
            gombotvalaszt(Rubrikaszam0, Pbilltomb0, kurrens);
        }
    }
}
