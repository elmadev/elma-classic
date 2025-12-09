#include "all.h"

static int Egybol = 1;

static char Nagyfilenev[] = "elma.res";

struct fileleiro {
    char nev[16];
    long hossz, kezdet; // In Shareware the order of these two vars is inversed
};

static fileleiro* Leirotomb = NULL;

#define FILESZAM (150)

#define AZONNUMBER (1347839l)

static long Fileszam = 0;

static void titkosit(void) {
    short a = 23, b = 9982, c = 3391; // In Shareware, b is 9882

    if (!Leirotomb) {
        hiba("titkosit-ban !Leirotomb!");
    }
    int leiromeret = sizeof(fileleiro);
    for (int i = 0; i < FILESZAM; i++) {
        unsigned char* pc = (unsigned char*)(&Leirotomb[i]);
        for (int j = 0; j < leiromeret; j++) {
            pc[j] ^= a;
            a %= c;
            b += a * c;
            a = 31 * b + c;
        }
    }
}

static int Qopeninited = 0;

void initqopen(void) {
    if (Qopeninited) {
        hiba("initqopen tobbszor hivva!");
    }
    Qopeninited = 1;

    Egybol = 1;
    if (access("files.inf", 0) == 0) {
        Egybol = 0;
    }

    if (!Egybol) {
        return;
    }

    {
        Leirotomb = new fileleiro[FILESZAM];
    }
    if (!Leirotomb) {
        uzenet("initqopen-ben memory!");
    }
    FILE* h = fopen(Nagyfilenev, "rb");
    if (!h) {
        uzenet("Missing file!: ", Nagyfilenev);
    }

    if (fread(&Fileszam, 1, 4, h) != 4) {
        hiba("initqopen-ben nem sikerult olvasni!");
    }
    if (Fileszam <= 0 || Fileszam > FILESZAM) {
        hiba("Fileszam <= 0 || Fileszam > FILESZAM!");
    }

    long hossz = sizeof(fileleiro[FILESZAM]);
    if (hossz > 64000l) {
        hiba("Tul hosszu Leirotomb initqopen-ben!");
    }
    if (fread(Leirotomb, 1, unsigned(hossz), h) != hossz) {
        hiba("initqopen-ben nem sikerult olvasni!");
    }
    titkosit();

    long magicnumber = 0;
    if (fread(&magicnumber, 1, 4, h) != 4) {
        hiba("initqopen-ben nem sikerult olvasni!");
    }
    if (magicnumber != AZONNUMBER) {
        hiba("initqopen-ben nem stimmel magicnumber!");
    }

    fclose(h);
}

#define MAXNYITVA (3)
static FILE* Htomb[MAXNYITVA];
static int Indextomb[MAXNYITVA];

static int Nyitva = 0;

FILE* qopen(char* nev, char* paramok) {
    if (!Qopeninited) {
        hiba("qopen initqopen nelkul!");
    }
    if (Nyitva == MAXNYITVA) {
        hiba("qopen-ben Nyitva == MAXNYITVA!");
    }
    if (strcmp(paramok, "rb") != 0 && strcmp(paramok, "rt")) {
        hiba("qopen-ben sem rb, sem rt!: ", nev);
    }

    if (Egybol) {
        for (int i = 0; i < Fileszam; i++) {
            if (strcmpi(nev, Leirotomb[i].nev) == 0) {
                Htomb[Nyitva] = fopen(Nagyfilenev, paramok);
                Indextomb[Nyitva] = i;
                if (!Htomb[Nyitva]) {
                    hiba("Nem sikerult nagy tombot megnyitni!: ", Nagyfilenev);
                }
                if (fseek(Htomb[Nyitva], Leirotomb[i].kezdet, SEEK_SET) != 0) {
                    hiba("Nem mukodik fseek qopen-ben!");
                }

                Nyitva++;
                return Htomb[Nyitva - 1];
            }
        }
        hiba("qopen nem talalta meg nagyfileban file-t!: ", nev);
        return NULL;
    } else {
        Nyitva++;
        char tmp[30] = "..\\files\\";
        if (strlen(nev) > 12 || nev[0] == '.') {
            hiba("Hibas nev qopen-ben!: ", nev);
        }
        strcat(tmp, nev);
        return fopen(tmp, paramok);
    }
}

void qclose(FILE* h) {
    if (!Qopeninited) {
        hiba("qclose initqopen nelkul!");
    }
    if (!Nyitva) {
        hiba("qclose-ban !Nyitva!");
    }
    if (Egybol) {
        for (int i = 0; i < Nyitva; i++) {
            if (Htomb[i] == h) {
                fclose(h);
                for (int j = i; j < Nyitva - 1; j++) {
                    Htomb[j] = Htomb[j + 1];
                    Indextomb[j] = Indextomb[j + 1];
                }
                Nyitva--;
                return;
            }
        }
        hiba("qclose nem talalta meg h-t!");
    } else {
        Nyitva--;
        fclose(h);
    }
}

int qseek(FILE* h, long offset, int whence) {
    if (whence != SEEK_SET && whence != SEEK_END && whence != SEEK_CUR) {
        hiba("whence != SEEK_SET && whence != SEEK_END && whence != SEEK_CUR!");
    }
    if (Egybol) {
        for (int i = 0; i < Nyitva; i++) {
            if (Htomb[i] == h) {
                int index = Indextomb[i];
                if (whence == SEEK_SET) {
                    return fseek(Htomb[i], Leirotomb[index].kezdet + offset, SEEK_SET);
                }
                if (whence == SEEK_END) {
                    return fseek(Htomb[i],
                                 Leirotomb[index].kezdet + Leirotomb[index].hossz + offset,
                                 SEEK_SET);
                }
                return fseek(Htomb[i], offset, SEEK_CUR);
            }
        }
        hiba("qclose nem talalta meg h-t!");
        return NULL;
    } else {
        return fseek(h, offset, whence);
    }
}
