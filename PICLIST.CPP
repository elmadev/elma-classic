#include "all.h"

static char Plnev[] = "pictures.lst";

static char Sor[210];
static char Szo[210];

// returns next pos or -1
static int egyszobe(int pos) {
    // go to start of word
    while (1) {
        if (!Sor[pos]) {
            return -1;
        }
        if (Sor[pos] != ' ') {
            break;
        }
        pos++;
    }
    // read word
    int szohossz = 0;
    while (1) {
        if (!Sor[pos] || Sor[pos] == ' ') {
            if (szohossz <= 0) {
                return -1;
            }
            Szo[szohossz] = 0;
            return pos;
        }
        Szo[szohossz] = Sor[pos];
        szohossz++;
        pos++;
    }
}

int piclist::beolvasegysort(FILE* h, int index) {
    // set to 0
    nevek[index * 10] = 0;
    tipusok[index] = 0;
    tavolsagok[index] = 0;
    hatarolok[index] = 0;
    atlatszosagok[index] = 0;

    char hibatext[100];
    sprintf(hibatext, "File: PICTURES.LST, Line: %d", index + 1);

    if (!fgets(Sor, 205, h)) {
        return 0;
    }
    // cut endline
    if (strchr(Sor, '\n')) {
        *strchr(Sor, '\n') = 0;
    }
    // cut tabs
    while (strchr(Sor, 9)) {
        *strchr(Sor, 9) = ' ';
    }

    // insert small picture name
    int pos = egyszobe(0);
    if (pos < 0) {
        return 0; // empty row
    }

    if (strchr(Szo, '.')) {
        uzenet("There is an extension in the picture name:", Sor, hibatext);
    }
    if (strlen(Szo) > 8) {
        uzenet("The picture name is longer than 8 characters:", Sor, hibatext);
    }
    strcpy(&nevek[index * 10], Szo);

    // insert type
    pos = egyszobe(pos);
    if (pos < 0) {
        uzenet("There is no type information in line:", Sor, hibatext);
    }

    tipusok[index] = 0;
    if (strcmpi(Szo, "mask") == 0) {
        tipusok[index] = PLTIP_MINTA;
    }
    if (strcmpi(Szo, "pict") == 0) {
        tipusok[index] = PLTIP_KEP;
    }
    if (strcmpi(Szo, "text") == 0) {
        tipusok[index] = PLTIP_TEXT;
    }
    if (strcmpi(Szo, "opti") == 0) {
        tipusok[index] = PLTIP_OPTI;
    }
    if (!tipusok[index]) {
        uzenet("Incorrect type in line:", Sor, hibatext);
    }

    if (tipusok[index] == PLTIP_OPTI) {
        return 1;
    }

    if (tipusok[index] != PLTIP_MINTA) {
        // in case of texture and picture, read length and borders
        pos = egyszobe(pos);
        if (pos < 0) {
            uzenet("There is no picture distance in line:", Sor, hibatext);
        }

        int tavolsag = -1;
        if (sscanf(Szo, "%d", &tavolsag) != 1) {
            uzenet("Could not read picture distance from line:", Szo, hibatext);
        }

        if (tavolsag < 0 || tavolsag > 999) {
            uzenet("Picture distance is out of range (0-999) in line:", Szo, hibatext);
        }
        tavolsagok[index] = tavolsag;

        // read borders
        pos = egyszobe(pos);
        if (pos < 0) {
            uzenet("There is no picture cutting information (F,B,U) in line:", Sor, hibatext);
        }

        hatarolok[index] = -1;
        if (Szo[0] == 'G' || Szo[0] == 'g') {
            hatarolok[index] = HATAROL_G;
        }
        if (Szo[0] == 'S' || Szo[0] == 's') {
            hatarolok[index] = HATAROL_S;
        }
        if (Szo[0] == 'U' || Szo[0] == 'u') {
            hatarolok[index] = 0;
        }

        if (hatarolok[index] < 0) {
            uzenet("The picture cutting information should be G, S, or U in line:", Sor, hibatext);
        }
    }

    // read transparency
    atlatszosagok[index] = ATLAT_TOPLEFT;
    pos = egyszobe(pos);
    if (pos < 0) {
        return 1;
    }

    // check transparency
    atlatszosagok[index] = -1;
    if (strcmpi(Szo, "tl") == 0) {
        atlatszosagok[index] = ATLAT_TOPLEFT;
    }
    if (strcmpi(Szo, "tr") == 0) {
        atlatszosagok[index] = ATLAT_TOPRIGHT;
    }
    if (strcmpi(Szo, "bl") == 0) {
        atlatszosagok[index] = ATLAT_BOTTOMLEFT;
    }
    if (strcmpi(Szo, "br") == 0) {
        atlatszosagok[index] = ATLAT_BOTTOMRIGHT;
    }

    if (strcmpi(Szo, "no") == 0) {
        atlatszosagok[index] = ATLAT_NEMATLATSZO;
    }
    if (strcmpi(Szo, "t0") == 0) {
        atlatszosagok[index] = ATLAT_0;
    }

    if (atlatszosagok[index] < 0) {
        uzenet("The transparency color information is invalid (TL, TR, BR, BL, NO, T0):", Sor,
               hibatext);
    }

    return 1;
}

piclist::piclist(void) {
    FILE* h = fopen(Plnev, "rt");
    if (!h) {
        uzenet("Could not open file:", Plnev);
    }

    for (int i = 0; i < (MAXPICLISTKEPSZAM + 10) * 10; i++) {
        nevek[i] = 0;
    }

    for (int i = 0; i < MAXPICLISTKEPSZAM + 10; i++) {
        tipusok[i] = 0;
        tavolsagok[i] = 0;
        hatarolok[i] = 0;
        atlatszosagok[i] = 0;
    }

    kepszam = 0;
    while (1) {
        if (kepszam >= MAXPICLISTKEPSZAM) {
            uzenet("Too many lines in file:", Plnev);
        }
        if (!beolvasegysort(h, kepszam)) {
            break;
        }
        kepszam++;
    }

    fclose(h);
}

// from binary format
piclist::piclist(FILE* h) {
    // first verzio 1002
    int verzio = 0;
    if (fread(&verzio, 1, 4, h) != 4) {
        uzenet("Cannot read pictures.lst information!");
    }
    if (verzio != 1002) {
        uzenet("In LGR file the pictures.lst information has illegal version!");
    }
    if (fread(&kepszam, 1, 4, h) != 4) {
        uzenet("Cannot read pictures.lst information!");
    }
    if (fread(nevek, 1, kepszam * 10, h) != kepszam * 10) {
        uzenet("Cannot read pictures.lst information!");
    }
    if (fread(tipusok, 1, kepszam * 4, h) != kepszam * 4) {
        uzenet("Cannot read pictures.lst information!");
    }
    if (fread(tavolsagok, 1, kepszam * 4, h) != kepszam * 4) {
        uzenet("Cannot read pictures.lst information!");
    }
    if (fread(hatarolok, 1, kepszam * 4, h) != kepszam * 4) {
        uzenet("Cannot read pictures.lst information!");
    }
    if (fread(atlatszosagok, 1, kepszam * 4, h) != kepszam * 4) {
        uzenet("Cannot read pictures.lst information!");
    }
}

// to binary format
void piclist::save(FILE* h) {
    int verzio = 1002;
    if (fwrite(&verzio, 1, 4, h) != 4) {
        uzenet("Cannot write pictures.lst information!");
    }
    if (fwrite(&kepszam, 1, 4, h) != 4) {
        uzenet("Cannot write pictures.lst information!");
    }
    if (fwrite(nevek, 1, kepszam * 10, h) != kepszam * 10) {
        uzenet("Cannot write pictures.lst information!");
    }
    if (fwrite(tipusok, 1, kepszam * 4, h) != kepszam * 4) {
        uzenet("Cannot write pictures.lst information!");
    }
    if (fwrite(tavolsagok, 1, kepszam * 4, h) != kepszam * 4) {
        uzenet("Cannot write pictures.lst information!");
    }
    if (fwrite(hatarolok, 1, kepszam * 4, h) != kepszam * 4) {
        uzenet("Cannot write pictures.lst information!");
    }
    if (fwrite(atlatszosagok, 1, kepszam * 4, h) != kepszam * 4) {
        uzenet("Cannot write pictures.lst information!");
    }
}

int piclist::getnevindex(char* nev) {
    for (int i = 0; i < kepszam; i++) {
        if (strcmpi(&nevek[i * 10], nev) == 0) {
            return i;
        }
    }
    return -1;
}
