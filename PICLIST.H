
#define MAXPICLISTKEPSZAM (3000)

enum {
    ATLAT_NEMATLATSZO = 10,
    ATLAT_0,
    ATLAT_TOPLEFT,
    ATLAT_TOPRIGHT,
    ATLAT_BOTTOMLEFT,
    ATLAT_BOTTOMRIGHT
};

enum { PLTIP_KEP = 100, PLTIP_TEXT, PLTIP_MINTA, PLTIP_OPTI };

class piclist {
    int beolvasegysort(FILE* h, int index);

  public:
    int kepszam;
    char nevek[(MAXPICLISTKEPSZAM + 10) * 10];
    int tipusok[MAXPICLISTKEPSZAM + 10];
    // Ha igaz, border texturanak szamit es kellhet tetejere tenni foltot:
    int tavolsagok[MAXPICLISTKEPSZAM + 10];
    int hatarolok[MAXPICLISTKEPSZAM + 10];
    int atlatszosagok[MAXPICLISTKEPSZAM + 10];

    piclist(void);      // PICTURES.LST file-bol olvas
    piclist(FILE* h);   // Binaris formatumbol
    void save(FILE* h); // Binaris formatumba

    int getnevindex(char* nev); // vagy -1
};
