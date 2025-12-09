#include "all.h"

static int Maxnevhossz = 8; // Ez kisebb kell hogy legyen JATEKOSNEVHOSSZ-nal!

// Siker eseten igazzal ter vissza:
static int bevesznevet(char* nev, int visszater) {
    szoveglista szovlist;

    int i = 0;
    mk_emptychar();
    int valtozott = 1;
    nev[0] = 0;
    while (1) {
        while (mk_kbhit()) {
            int c = mk_getextchar();
            if (c == MK_ESC) {
                if (visszater) {
                    return 0;
                }
                kilep();
            }
            if (c == MK_ENTER) {
                // ENTER
                if (i > 0) {
                    return 1;
                }
            }
            if ((c >= 'A' && c <= 'Z') || (c >= 'a' && c <= 'z') || (c >= '0' && c <= '9')) {
                if (i >= Maxnevhossz) {
                    continue;
                }
                nev[i] = (char)c;
                nev[i + 1] = 0;
                i++;
                valtozott = 1;
            }
            if (c == MK_BACKSPACE) {
                // <- (torles):
                if (i > 0) {
                    i--;
                    nev[i] = 0;
                    valtozott = 1;
                }
            }
        }
        if (valtozott) {
            valtozott = 0;

            szovlist.clear();

            nev[i] = '_';
            nev[i + 1] = 0;
            szovlist.addszoveg_kozep(nev, 320, 240);
            nev[i] = 0;

            szovlist.addszoveg_kozep("Please enter your name:", 320, 180);
            // szovlist.addszoveg_kozep( "Press ESC to exit", 320, 300 );
        }
        szovlist.kirajzol();
    }
}

void newjatekos(int A, int visszater) {
    // Ellenorzes:
    if (State->jatekosokszama >= MAXJATEKOS - 1) {
        uzenet("Sorry, no more players can get onto the list!");
    }
    int maxrub = Maxrubrikaszam;
    if (MAXJATEKOS > maxrub) {
        hiba("MAXJATEKOS > Maxrubrikaszam!");
    }

    if (bevesznevet(State->jatekosok[int(State->jatekosokszama)].nev, visszater)) {
        if (A) {
            strcpy(State->jatekosa, State->jatekosok[int(State->jatekosokszama)].nev);
            State->jatekosokszama++;
            if (State->jatekosokszama == 1) {
                strcpy(State->jatekosb, State->jatekosa);
            }
        } else {
            strcpy(State->jatekosb, State->jatekosok[int(State->jatekosokszama)].nev);
            State->jatekosokszama++;
            if (State->jatekosokszama == 1) {
                strcpy(State->jatekosa, State->jatekosb);
            }
        }
    } else {
        visszater = 1; // Ha elescapelte, akkor visszamegyunk menube
    }

    if (State->jatekosokszama == 0) {
        kilep();
    }

    if (visszater) {
        return;
    }

    mainmenu();
}

void jatekosvalasztas(int A, int visszater) {
    // Valasztas:
    valaszt2* pval = NULL;
    pval = new valaszt2;
    char* eddigjatekos = NULL;
    if (A) {
        eddigjatekos = State->jatekosa;
    } else {
        eddigjatekos = State->jatekosb;
    }
    pval->egykepen = LISTegykepen;
    pval->x0 = LISTx0;
    pval->y0 = LISTy0;
    pval->dy = LISTdy;
    pval->escelheto = 1;
    strcpy(pval->cim, "Choose Player");

    strcpy(Rubrikak[0], "Create New Player");
    pval->kur = 0;
    for (int i = 0; i < State->jatekosokszama; i++) {
        if (strlen(State->jatekosok[i].nev) > MAXRUBRIKAHOSSZ) {
            hiba("Tul hosszu nev jatekosvalasztas-ban!");
        }
        strcpy(Rubrikak[i + 1], State->jatekosok[i].nev);

        if (strcmp(eddigjatekos, State->jatekosok[i].nev) == 0) {
            pval->kur = i + 1;
        }
    }

    pval->bead(State->jatekosokszama + 1);

    RECidokellhet = 1;
    RECidokellis = 0;
    int eredmeny = pval->valassz();
    RECidokellhet = 0;

    delete pval;
    pval = NULL;

    if (eredmeny < 0) {
        if (visszater) {
            return;
        }
        kilep();
    }
    if (eredmeny == 0) {
        newjatekos(A, visszater);
        return;
    }

    if (RECidokellis && visszater) {
        // Ezt a jatekost akarja torolni (opciokbol hivva):
        if (State->jatekosokszama <= 1) {
            return;
        }

        int torlendo = eredmeny - 1;

        // Megjegyezzuk hogy A vagy B nem torolt jatekos-e veletlenul:
        int atorlodott = 0;
        if (strcmp(State->jatekosa, State->jatekosok[torlendo].nev) == 0) {
            atorlodott = 1;
        }
        int btorlodott = 0;
        if (strcmp(State->jatekosb, State->jatekosok[torlendo].nev) == 0) {
            btorlodott = 1;
        }

        // Toroljuk jatekost, tobbit elore hozzuk eggyel:
        for (int i = torlendo; i < State->jatekosokszama - 1; i++) {
            // i-edik helyre elore hozzuk i+1-edik helyrol:
            State->jatekosok[i] = State->jatekosok[i + 1];
        }
        State->jatekosokszama--;

        if (atorlodott) {
            strcpy(State->jatekosa, State->jatekosok[0].nev);
        }
        if (btorlodott) {
            strcpy(State->jatekosb, State->jatekosok[0].nev);
        }

        return;
    }

    if (A) {
        strcpy(State->jatekosa, State->jatekosok[eredmeny - 1].nev);
    } else {
        strcpy(State->jatekosb, State->jatekosok[eredmeny - 1].nev);
    }

    if (visszater) {
        return;
    }

    mainmenu();
}
