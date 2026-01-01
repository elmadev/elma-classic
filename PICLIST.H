#ifndef PICLIST_H
#define PICLIST_H

#define MAXPICLISTKEPSZAM (3000)

class piclist {
  public:
    enum {
        ATLAT_NEMATLATSZO = 10,
        ATLAT_0,
        ATLAT_TOPLEFT,
        ATLAT_TOPRIGHT,
        ATLAT_BOTTOMLEFT,
        ATLAT_BOTTOMRIGHT
    };

    enum { PLTIP_KEP = 100, PLTIP_TEXT, PLTIP_MINTA };

    int kepszam;
    char nevek[(MAXPICLISTKEPSZAM + 10) * 10];
    int tipusok[MAXPICLISTKEPSZAM + 10];
    // Ha igaz, border texturanak szamit es kellhet tetejere tenni foltot:
    int tavolsagok[MAXPICLISTKEPSZAM + 10];
    int hatarolok[MAXPICLISTKEPSZAM + 10];
    int atlatszosagok[MAXPICLISTKEPSZAM + 10];

    piclist(FILE* h); // Binaris formatumbol

    int getnevindex(const char* nev); // vagy -1
};

#endif
