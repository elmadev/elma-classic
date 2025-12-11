
#define MAXJATEKOS (50)
#define JATEKOSNEVHOSSZ (14)

#define MAXPALYASZAM (90) // this is never reached

extern int Palyaszam; // depends on if shareware or registered

struct jatekos {
    char nev[JATEKOSNEVHOSSZ + 2];
    char skippedtomb[((MAXPALYASZAM / 4) + 1) * 4];
    long sikerespalyakszama;
    long jelenlegipalya; // external file if -1
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

    // options
    int soundon;
    int secondarybuffer;
    // should look at Single variable here because value might differ from state during replays
    int single;
    // same with Tag variable
    int tag;
    int minibike; // this is not saved
    int aplayerabike;
    int highquality;

    int animatedobjects;
    int animatedmenus;

    // keys
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

extern state_s* State; // intialized in teljes.cpp
