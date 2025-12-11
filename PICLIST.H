
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
    // if true, counts as border texture and needs to be appended
    int tavolsagok[MAXPICLISTKEPSZAM + 10];
    int hatarolok[MAXPICLISTKEPSZAM + 10];
    int atlatszosagok[MAXPICLISTKEPSZAM + 10];

    piclist(void);      // read from PICTURES.LST
    piclist(FILE* h);   // from binary format
    void save(FILE* h); // to binary format

    int getnevindex(char* nev); // or -1
};
