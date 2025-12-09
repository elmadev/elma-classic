
class ecset;
struct mdarab;

// Egy iterator class (inkabb cache) ecsethez (foltoz hasznalja oket):
class mdbiter {
    ecset* pecset;
    int xposbent, ybent;
    mdarab* pmd;

  public:
    mdbiter(ecset* pecset);
    mdarab* getpmd(int x, int y);
};

// Elso par strukturat be lehetne majd vinni cpp-be:

#define PX_FOLD (1)
#define PX_EG (NULL)
#define PX_URES (2)

struct mdarab {
    mdarab* pkov;
    int xsize;
    int tavolsag;
    unsigned char* pixelek;
};

#define TOMBBENMDARAB (10000)

struct mdarabtomb {
    mdarab tomb[TOMBBENMDARAB];
    mdarabtomb* kovtomb;
};

#define MAXECSETSOR (12000)

struct darab {
    int xsize;
    unsigned char* pixelek;
};

void betoltecseteket(void);

class ecset {
    friend void betoltecseteket(void);
    friend void segedfv(void);
    friend mdarab* mdbiter::getpmd(int x, int y);

    // madarab allokalas:
    mdarabtomb* elsotomb;
    mdarabtomb* kurtomb;
    int tombbenkov;
    mdarab* newmdarab(void);
    void deletemdarabok(void);

    darab* nagydarabtomb;

    int view;
    vekt2 origo;
    double xsizedb, ysizedb;
    int maxx, sorszam;
    mdarab* msorok[MAXECSETSOR];
    darab* sorok[MAXECSETSOR];
    // Kulon van ket jatekosnak:
    int kurxposok_A[MAXECSETSOR];     // Eppen hol all mutatohoz tartozo x
    darab* curdarabok_A[MAXECSETSOR]; // Melyik darab kurrens
    int kurxposok_B[MAXECSETSOR];     // Eppen hol all mutatohoz tartozo x
    darab* curdarabok_B[MAXECSETSOR]; // Melyik darab kurrens

    void getorigoandsize(void);
    void addszakasz(vonal* psz);
    // Rovid folddarabokat atalakitja konkret mutatova:
    void foldmutatocsere(void);
    // Egymas utani azonos maszkokat es ureseket lecsereli egy hosszabbra:
    void duplaeliminacio(void);
    void textura2mutato(void);
    void mutatotlanit(void);

    // Ptop kerek-jeinek egesz koordinatait kitolti:
    void kitoltfoodkoordokat(void);

    void levon10000t(void);
    mdarab* beepitegymadarabnyit(mdarab* pmd, unsigned char* ujpixelek, int x1, int ujx1, int ujx2,
                                 int ujtavolsag, mdarab* vege, int* pujavege, int fazis);
    void addbytesor(unsigned char* ujpixelek, int ujtavolsag, int ujx1, int ujx2, int y, int fazis);
    void addegymaszk(sprite* psp, int index_t, int index_m, int fazis);
    void addspriteok(int fazis);
    void addobjektumok(void); // Ez most nem view eseten visszter

    void kovetotextura(koveto* pkov, int index, int xo, int yo, int fazis);
    void kiegykovetokep(koveto* pkov, int index, int xo, int yo, int fazis);
    void kikovetokepek(koveto* pkov, int* ytomb, int hossz, int x0, int fazis);
    void addkovetok(int fazis);

    void kiegysor_A(unsigned char* pc, int ye);
    void kiegysor_B(unsigned char* pc, int ye);
    void kiegysor_FF(unsigned char* pc, int ye); // fekete-feher

    void foltoz(void);

    ecset(int view);    // Ptop es Pszak alapjan
    ecset(ecset* pold); // Ures sortomboket hoz benne letre
    ~ecset(void);

  public:
    void getbalalso_int(vekt2 balalso, int* px, int* py);
    void kitesz(int ajatekos, pic8* ppic, vekt2 balalso, int x1, int y1, int x2, int y2);
    void kiteszview(int ajatekos, pic8* ppic, vekt2 balalso, int x1, int y1, int x2, int y2);

    // void egetkeszit( void );
};

extern ecset* Pecsetalso;
extern ecset* Pecsetfelso;
extern ecset* Pecsetview;

// Ket kis utility fuggveny (hasznalja oket LGR.CPP):
// Visszaadja hany pixel lyuk szinu x-tol kezdve:
int uresszam(int x, int xsize, unsigned char* sor, unsigned char lyuk);
// Visszaadja hany pixel nem lyuk szinu x-tol kezdve:
int teliszam(int x, int xsize, unsigned char* sor, unsigned char lyuk);

// betoltecseteket ide osszegzi osszes lefoglalt szakaszt:
extern int Osszegszam;
