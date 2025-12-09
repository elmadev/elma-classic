
enum { ME_URES, ME_TELI, ME_SOREMELES };

struct maszkelem {
    int hossz;
    int tipus;
};

#define MAXMASZKSZAM (200)

struct maszk {
    char nev[10];
    int xsize, ysize;
    maszkelem* adatok;
};

#define MAXKEPSZAM (1000)

struct kep {
    char nev[10];
    int tavolsag, hatarol;

    int xsize, ysize;
    unsigned char* adatok; // leiras kovetkezik:
                           // soronkent ismetlodve: {uresek szama telik szama, adat byte-ok}
                           // uresek szama 60000 felett eseten uj sor
                           // tehat minden sor uressel kezdodik
                           // szamok 2 byte-on tarolodnak elso-t meg kell szorozni 256-tal
};

#define MAXTEXTURASZAM (100)

struct textura {
    char nev[10];
    pic8* ppic;
    int tavolsag, hatarol;
    int foltos;
    int origxsize;
};

struct motkepek {
    kiskep *pkisa, *pkisb, *pkisc, *pkisd;
    kiskep* pkisvezeto;
    kiskep* pkiscomb;
    kiskep* pkislabszar;
    kiskep* pkiskerek;
    kiskep* pkiselsorud;
    kiskep* pkishatsorud;
    kiskep* pkisalkar;
    kiskep* pkisfelkar;
    kiskep* pkisfej;
};

#define MAXKOVETOK (20)
#define MAXFOODSZAM (20) // Valojaban ennek csak 9-nak kene lennie

class lgrfile {
    // Konstruktor hasznalja:
    void chopbiker(pic8* pbiker, motkepek* pkepek);

    void beirkepet(pic8* ppic, piclist* ppiclist, int index);
    void beirtexturat(pic8* ppic, piclist* ppiclist, int index);
    void beirmaszkot(pic8* ppic, piclist* ppiclist, int index);

    lgrfile(char* nev); // kiterj es ut nelkul
    ~lgrfile(void);

    int csereljen(int i);

  public:
    friend void loadlgrfile(char* nev); // Leiras alul
    friend void tesztloadlgr(void);

    int kepszam;
    kep kepek[MAXKEPSZAM];
    int getkepindex(char* nev); // vagy -1

    int maszkszam;
    maszk maszkok[MAXMASZKSZAM];
    int getmaszkindex(char* nev); // vagy -1

    int texturaszam;
    textura texturak[MAXTEXTURASZAM];
    int gettexturaindex(char* nev); // vagy -1

    int vanfu;
    // Textura szelekcio lista utolso eleme kimarad (qgrass):
    int opentextkimarad;
    koveto* pkoveto;

    unsigned char* paltomb;
    ddpal* pal;
    // paltomb alapjan ido kiirashoz negalt 256 byte-os lookup tabla:
    unsigned char* idonegtomb;

    motkepek mkepek1;
    motkepek mkepek2;
    kiskep* pkiszaszlo;

    anim *pkiller, *pexit; // Nagy objektumok
    int foodszam;
    // Ennek indexe 0-tol indul, de ez 1-nek felel meg filenev es
    // set properties-ben:
    anim* foodtomb[MAXFOODSZAM];
    pic8* pkimarad; // Csokkentett meret eseten ez latszik

    // Hater es eloter kepeket mindig kulon vesszuk:
    pic8 *peg, *pfold; // Ezek mar kelloen hosszuak vizszintesen
    int egxmodulus, foldxmodulus;
    char fgnevbent[10], bgnevbent[10];
    // Ezt betoltecseteket hivja (Ptop-bol szedi kep neveket):
    void betolthattereket();

    // Ezeket colors.pcx file-bol veszi:
    unsigned char viewfoldsor[640];
    unsigned char viewegsor[640];
    unsigned char viewmotorindex1;
    unsigned char viewmotorindex2;
    unsigned char idoszin1; // Ezt torolni lehet
    unsigned char idoszin2; // Ezt torolni lehet
    unsigned char keretindex;
    unsigned char viewexit[3];
    unsigned char viewfood;
    unsigned char viewkiller[3];

    // tolt_pickasprite tudja allitani oket egyedul:
    char aktiv_kepnev[10];
    char aktiv_maszknev[10];
    char aktiv_texturanev[10];
};

extern lgrfile* Plgr; // Az eppen bentlevo lgrfile-ra mutat
// Ha nincs ilyen nevu file, akkor default-ot olvassa:
void loadlgrfile(char* nev);

extern int Allandokepszam;
// [stringek szama][egy string hossza]:
extern char Allandotomb[28][28];

// Kulonallo exe-be:
void makelgrfile(void);

// Ez egy kis utility, file bemasolasara nagy file-ba:
// Ez elso negy byte-ba beirja file hosszat is:
void filemasolas(FILE* hout, char* nev, char* outnev);

struct kisbox {
    int x1, y1, x2, y2;
};

extern kisbox KisboxA, KisboxB, KisboxC, KisboxD;
