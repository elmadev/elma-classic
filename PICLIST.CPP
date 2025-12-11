#include "all.h"

// Binaris formatumbol:
piclist::piclist(FILE* h) {
    // Elso verzio 1002
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

int piclist::getnevindex(const char* nev) {
    for (int i = 0; i < kepszam; i++) {
        if (strcmpi(&nevek[i * 10], nev) == 0) {
            return i;
        }
    }
    return -1;
}
