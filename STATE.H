
#define MAXJATEKOS (50)
#define JATEKOSNEVHOSSZ (14)

#define MAXPALYASZAM (90) // Ennyit soha nem er el

extern int Palyaszam; // Ez fugg attol, hogy shareware, vagy regisztralt

struct jatekos {
    char nev[JATEKOSNEVHOSSZ + 2];
    char skippedtomb[((MAXPALYASZAM / 4) + 1) * 4];
    long sikerespalyakszama;
    long jelenlegipalya; // -1 eseten external files
};

int getjatekosindex(char* nev);

#define MAXIDOK (10)

typedef char jatekosnev_type[JATEKOSNEVHOSSZ + 1];

struct palyaegyfeleidok {
    long idokszama;
    long idok[MAXIDOK];
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
    long jatekosokszama;
    jatekosnev_type jatekosa, jatekosb;

    // Opciok:
    int soundon;
    int secondarybuffer;
    // Ehelyett Single valtozot kell figyelni, mivel rec visszajatszasnal
    // lehet hogy mas kell legyen mint state-ben levo:
    int single;
    // Itt pedig Tag valtozot kell figyelni:
    int tag;
    int minibike; // Ezt most meg csak nem is mentjuk
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

    state_s(char* nev = NULL);
    ~state_s(void);

    void reloadidok(void);

    void save(void);
    void exportjatekossum(FILE* h, char* nev, int single);
    void exportanonim(FILE* h, int single, char* text1, char* text2, char* text3);
    void export_(void);
    void resetcontrols(void);
};

extern state_s* State; // teljes.cpp-ben van inicializalva!
