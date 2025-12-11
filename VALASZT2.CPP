#include "all.h"

const int LISTegykepen = 11;
const int LISTx0 = 200;
const int LISTy0 = 90;
const int LISTdy = 33;

int Maxrubrikaszam = 1;

rubrika* Rubrikak = NULL;
rubrika Rubrikak_tab[MAXRUBRIKASZAM_TAB + 1];

void initrubrikak(void) {
    if (Rubrikak) {
        hiba("98y5t4");
    }
    // count the max amount of rec and lev files:
    int levszam = 0;
    finame fname;
    int done = fifirst("lev/*.lev", fname);
    while (!done) {
        done = finext(fname);
        levszam++;
    }
    ficlose();

    int recszam = 0;
    done = fifirst("rec/*.rec", fname);
    while (!done) {
        done = finext(fname);
        recszam++;
    }
    ficlose();

    int szam = levszam;
    if (szam < recszam) {
        szam = recszam;
    }

    // if( szam < MAXKEPSZAM )
    //	szam = MAXKEPSZAM;

    // max szam is acquired
    szam += 220;

    if (szam > 40000) {
        szam = 40000;
    }

    Rubrikak = new rubrika[szam + 10];
    if (!Rubrikak) {
        hiba("Not enough memory to allocate for Rubrikak!");
    }
    Maxrubrikaszam = szam;
    if (Maxrubrikaszam < 200) {
        hiba("87iuo");
    }
}

valaszt2::valaszt2(void) {
    rubrikak = NULL;
    rubrikak_tab = NULL;
    szam = 0;
    kur = x0 = y0 = dy = egykepen = escelheto = 0;
    cim[0] = 0;
    // this is set here, but can be redefined elsewhere
    cimy = 30;
    pszovlist = NULL;
}

valaszt2::~valaszt2(void) {
    if (rubrikak) {
        delete rubrikak;
    }
    if (rubrikak_tab) {
        delete rubrikak_tab;
    }
    if (pszovlist) {
        delete pszovlist;
    }

    rubrikak = NULL;
    rubrikak_tab = NULL;
    pszovlist = NULL;
    szam = 0;
    kur = x0 = y0 = dy = egykepen = escelheto = 0;
}

// takes in rubrics
void valaszt2::bead(int szam_p, int tabos_p) {
    if (rubrikak) {
        hiba("valaszt2::bead rubrikak!");
    }
    szam = szam_p;
    tabos = tabos_p;
    if (szam < 1 || szam > Maxrubrikaszam) {
        hiba("valaszt2::bead wrong number!");
    }
    if (tabos && szam > MAXRUBRIKASZAM_TAB) {
        hiba("valaszt2::bead wrong number (tab)!");
    }
    rubrikak = new rubrika[szam];
    if (!rubrikak) {
        hiba("memory");
    }
    rubrikak_tab = NULL;
    if (tabos) {
        rubrikak_tab = new rubrika[szam];
        if (!rubrikak_tab) {
            hiba("memory");
        }
    }
    for (int i = 0; i < szam; i++) {
        if (strlen(Rubrikak[i]) > MAXRUBRIKAHOSSZ) {
            hiba("valaszt2::bead string too long!:", Rubrikak[i]);
        }
        strcpy(rubrikak[i], Rubrikak[i]);
        if (tabos) {
            if (strlen(Rubrikak_tab[i]) > MAXRUBRIKAHOSSZ) {
                hiba("valaszt2::bead string too long (tab)!:", Rubrikak_tab[i]);
            }
            strcpy(rubrikak_tab[i], Rubrikak_tab[i]);
        }
    }
}

int RECidokellhet = 0;
int RECidokellis = 0;

// returns the selected number (-1 in case of ESC)
// in case of tobbrajz, first render multiple times (Win95):
int valaszt2::valassz(egyszovsor* preszovegek, int preszovegszam, int csakkirajzol) {
    if (szam < 1 || egykepen < 2) {
        hiba("valaszt2::valassz wrong at start!");
    }

    if (kur > szam - 1) {
        kur = szam - 1;
    }

    int felso = kur - egykepen / 2;
    int maxfelso = szam - egykepen;
    if (felso > maxfelso) {
        felso = maxfelso;
    }
    if (felso < 0) {
        felso = 0;
    }

    if (pszovlist) {
        delete pszovlist;
    }
    pszovlist = new szoveglista;

    mk_emptychar();
    int valtozott = 1;
    while (1) {
        while (!csakkirajzol && mk_kbhit()) {
            int c = mk_getextchar();
            if (c == MK_ESC && escelheto) {
                return -1;
            }
            if (c == MK_ENTER) {
                // required for RECidomentes (also in mainmenu.cpp):
                int controlaltnyomva(void);
                if (RECidokellhet && controlaltnyomva()) {
                    RECidokellis = 1;
                }
                return kur;
            }
            if (c == MK_UP) {
                if (kur > 0) {
                    kur--;
                    if (kur < felso) {
                        felso--;
                        valtozott = 1;
                    }
                }
            }
            if (c == MK_PGUP) {
                if (kur > 0) {
                    kur -= egykepen;
                    if (kur < 0) {
                        kur = 0;
                    }
                    if (kur < felso) {
                        felso = kur;
                        valtozott = 1;
                    }
                }
            }
            if (c == MK_DOWN) {
                if (kur < szam - 1) {
                    kur++;
                    if (kur > felso + egykepen - 1) {
                        felso++;
                        valtozott = 1;
                    }
                }
            }
            if (c == MK_PGDOWN) {
                if (kur < szam - 1) {
                    kur += egykepen;
                    if (kur >= szam) {
                        kur = szam - 1;
                    }
                    if (kur > felso + egykepen - 1) {
                        felso = kur - egykepen + 1;
                        valtozott = 1;
                    }
                }
            }
            // setting volume is handled here specially:
            /*if( (c == MK_LEFT || c == MK_RIGHT) &&
                strcmp( rubrikak[kur], "Sound Volume:" ) == 0 &&
                (kur == 3 || kur == 4) &&
                strcmp( rubrikak[kur-1], "Player B:" ) == 0 ) {
                if( c == MK_LEFT ) {
                    State->soundvolume -= 0.05;
                    if( State->soundvolume < 0.0 )
                        State->soundvolume = 0.0;
                }
                if( c == MK_RIGHT ) {
                    State->soundvolume += 0.05;
                    if( State->soundvolume > 1.0 )
                        State->soundvolume = 1.0;
                }
                return kur;
            } */
        }
        if (valtozott) {
            valtozott = 0;
            // feed new kepszoveg into kirajz:
            pszovlist->clear();
            for (int i = 0; i < preszovegszam; i++) {
                if (strncmp(preszovegek[i].text, "*$$^&|@", 7) == 0) {
                    // needs to be centered:
                    pszovlist->addszoveg_kozep(&preszovegek[i].text[7], preszovegek[i].x,
                                               preszovegek[i].y);
                } else {
                    // doesn't need to be centered:
                    pszovlist->addszoveg(preszovegek[i].text, preszovegek[i].x, preszovegek[i].y);
                }
            }

            pszovlist->addszoveg_kozep(cim, 320, cimy);
            for (int i = 0; i < egykepen && i < szam - felso; i++) {
                pszovlist->addszoveg(rubrikak[felso + i], x0, y0 + i * dy);
            }
            if (tabos) {
                for (int i = 0; i < egykepen && i < szam - felso; i++) {
                    pszovlist->addszoveg(rubrikak_tab[felso + i], x0_tab, y0 + i * dy);
                }
            }
        }
        pszovlist->setsisak(x0 - 30, y0 + (kur - felso) * dy);
        pszovlist->kirajzol();
        if (csakkirajzol) {
            return 0;
        }
    }
}

void valaszt2::kirajzol(void) {
    if (!pszovlist) {
        hiba("tegcdy");
    }
    pszovlist->kirajzol();
}
