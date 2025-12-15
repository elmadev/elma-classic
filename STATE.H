#ifndef STATE_H
#define STATE_H

#define MAXJATEKOS (50)
#define JATEKOSNEVHOSSZ (14)

#define MAXPALYASZAM (90) // Ennyit soha nem er el

extern int Palyaszam; // Ez fugg attol, hogy shareware, vagy regisztralt

struct jatekos {
    char nev[JATEKOSNEVHOSSZ + 2];
    char skippedtomb[((MAXPALYASZAM / 4) + 1) * 4];
    int sikerespalyakszama;
    int jelenlegipalya; // -1 eseten external files
};

int getjatekosindex(const char* nev);

#define MAXIDOK (10)

typedef char jatekosnev_type[JATEKOSNEVHOSSZ + 1];

struct palyaegyfeleidok {
    int idokszama;
    int idok[MAXIDOK];
    jatekosnev_type nevek1[MAXIDOK];
    jatekosnev_type nevek2[MAXIDOK];
};

struct palyaidok {
    palyaegyfeleidok singleidok, multiidok;
};

struct jatekosopciok {
    int billgaz, billfek, billugras1, billugras2, billfordul;
    int billview, billtime, billshowkep;
};

class state_s {
  public:
    palyaidok palyakidejei[MAXPALYASZAM];
    jatekos jatekosok[MAXJATEKOS];
    int jatekosokszama;
    jatekosnev_type jatekosa, jatekosb;

    // Opciok:
    int soundon;
    int secondarybuffer;
    // Ehelyett Single valtozot kell figyelni, mivel rec visszajatszasnal
    // lehet hogy mas kell legyen mint state-ben levo:
    int single;
    // Itt pedig Tag valtozot kell figyelni:
    int tag;
    int aplayerabike;
    int highquality;

    int animatedobjects;
    int animatedmenus;

    // Billentyuk:
    jatekosopciok opciok1;
    jatekosopciok opciok2;
    int billplussz, billminusz;
    int billsnap;

    char editfilenev[20];
    char playextfilenev[20];

    state_s(const char* nev = NULL);
    ~state_s(void);

    void reloadidok(void);

    void save(void);
    void exportjatekossum(FILE* h, const char* nev, int single);
    void exportanonim(FILE* h, int single, const char* text1, const char* text2, const char* text3);
    void export_(void);
    void resetcontrols(void);
};

extern state_s* State; // teljes.cpp-ben van inicializalva!

#endif
