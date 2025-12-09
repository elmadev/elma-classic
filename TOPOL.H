
#define MAXPONT (1000) // Ennyit csak akkor foglal le, ha kell

class topol;

class gyuru {
    int lefoglaltmaxpont;

  public:
    int pontszam;
    vekt2* ponttomb;

    // Koveto valtozok:
    int koveto;

    // Fuggvenyek:
    gyuru(void);
    gyuru(FILE* h, int kovetos);
    ~gyuru(void);

    int getpontszam(void);
    void save(FILE* h, topol* ptop);
    void ujkoord(int k, double x, double y);
    void kirajz(int k, int fel, int szaggatott = 0);
    void osszrajz(void);
    // Sikeres-e azt adja vissza:
    int beszur(int k); // k pont utan beszur egyet ugyanolyan koordokkal
    void torol(int k);
    double legkozpont(double x, double y, int* pk);
    int metszesszam(vekt2 r1, vekt2 v1);
    // Ez csak akkor tekinti metszesnek, ha nem csak eppen erintik egymast:
    int metszeshely(vekt2 r1, vekt2 v1, int kihagy, vekt2* pmetszes);
    // akkor ad igazat, ha korbejaraskor poligon belseje bal oldalon van;
    int pozitivkorjaras(void);
    vekt2 sulypont(void);
    // Beadott px1, stb-bol indul ki:
    void getminmax(double* px1, double* py1, double* px2, double* py2);
    double belyegszamitas(void);
    void szetvalaszt(void);
};

void set_gyuru_attributes(gyuru* pgy);

#define MAXKEREK (52)

enum { T_CEL = 1, T_KAJA, T_HALALOS, T_KEZDO };
enum { KT_NORMAL, KT_UP, KT_DOWN, KT_LEFT, KT_RIGHT };

class kerek {
  public:
    vekt2 r;
    int rxint, ryint;     // Kaja lejatszas kozbeni kirajzolasahoz
    int rxint_v, ryint_v; // Ugyanez view-ra
    int tipus;
    int kajatipus; // Ha kaja, akkor lehet gravitacios is
    int foodsorszam;
    int aktiv;          // Lejatszas soran ha kaja, megvan-e meg
    unsigned char szin; // Csak lejatszasnal kell
    int sinfazisint;    // 0-tol 999-ig mehet
    double sinfazis;    // 0-tol 2Pi-ig megy

    kerek(double x, double y, int tipus);
    kerek(FILE* h, int verzio);
    void kirajzol(void);
    void save(FILE* h);
    double belyegszamitas(void);
};

#define MAXSPRITE (5000)

#define HATAROL_G (1)
#define HATAROL_S (2)

class sprite {
  public:
    // Ha nev nem ervenyes, akkor [0] == 0:
    // Ha kepnev ervenyes, akkor masik kettonek uresnek kell lennie:
    char kepnev[10];
    char texturanev[10];
    char maszknev[10];
    vekt2 r;
    double xsize, ysize;
    // Ha override, akkor tavolsagot es hatarolast nem irja felul LGR alapjan:
    int tavolsag;
    int hatarol; // 0, HATAROL_F, HATAROL_B

    sprite(double x, double y, char* kepnev_p, char* texturanev_p, char* maszknev_p);
    sprite(FILE* h);
    void kirajzol(void);
    void save(FILE* h);
    double belyegszamitas(void);
};

#define MAXGYURU (300)

typedef gyuru* ptrgyuru;
#define LEVELNAMEHOSSZ (50) // 13-as verzioig (14) volt

class lgrfile;

class topol {
    friend int menu_exit(void);
    friend int floadlevel_e(char* nev, int forceload);
    friend int floadlevel_p(char* nev);
    friend void invalidate_ptop(void);
    friend void kiirdesceket(void);
    friend void hakellmentbelleveket(void); // topol.cpp-ben

    double belyegszamitas(int kellspriteis);

    topol(void);
    // QWQUU001 elso palya:
    topol(char* filenev);
    ~topol(void);
    void loadkulso(char* filenev);
    void loadbelso(char* filenev);

  public:
    // Beolvasas utan ervenyes, csak rec file ellenorzesere hasznalando
    long belyeg;
    // loadlgrfile allitja 1-be, ha nincs meg lgr es default.lgr-t hasznal:
    int nincslgrje;

    int hibas, lezart;
    ptrgyuru ptomb[MAXGYURU]; // Ez csak gany miatt public
    kerek* kerektomb[MAXKEREK];
    sprite* spritetomb[MAXSPRITE];
    char levelname[LEVELNAMEHOSSZ + 1];
    char lgrnev[16];
    char fgnev[10];
    char bgnev[10];

    palyaidok idok;
    int idokhelye;

    // Igazzal ter vissza, ha volt valtozas:
    int pic_selejtez(lgrfile* plgr);
    gyuru* legkozpont(double x, double y, int* pk, double* ptav = NULL, gyuru* kimarad = NULL);
    kerek* legkozkerek(double x, double y, double* ptav = NULL);
    sprite* legkozsprite(double x, double y, double* ptav = NULL);
    // Ha saveidok hivja meg, csak akkor nem kell check:
    void save(char* filenev, int nemkellcheck = 0);
    // Csak eredetileg beolvasott filenev-vel szabad meghivni:
    void saveidok(char* filenev);
    void kirajzol(void);
    // pgy levegoben van-e. Akkor folddarab:
    int levegoben(gyuru* pgy, vekt2* pr = NULL);
    void getminmax(double* px1, double* py1, double* px2, double* py2, int kerekkel);

    // Kerekkel kapcsolatos dolgok:
    // Motor helyzetet is o allitja be, kajaszamot adja vissza:
    int setallaktiv(motorst* pmot); // eheto kerekeket aktivizalja
    void killerekelore(void);       // kerekeket rendezi
    void kitoltiszineket(void);     // Osszes kerek szinet bejegyzi
    kerek* getptrkerek(int index);

    // Ha igaz, y negalva van (ez kell lejatszasnal):
    int kerekekfejjellefele;
    void kereklefejjel(void);
    void kerekjolalljon(void);
};

// lejatszo.cpp-ben Falg Tag ennek alapjan teszi vissza motorost:
extern vekt2 Kezdetihelydiff; // setallaktiv allitja be

// Ha letezik file, akkor 0-t ad vissza!:
// Ugyanugy viselkedik nevvel, mint topol konstruktora:
int access_topol(char* filenev);

unsigned long getbelyeg(char* nev);
// Igazzal ter vissza, ha megvan nev;
int getbelyegheznev(unsigned long belyeg, char* nev);

// Csak external file-hoz:
// Ha NULL, akkor valami baj volt vele:
// char* getlevelname( char* nev ); mar nem hasznalt

// 0-tol kezdodnek:
char* getleveldescription(int level);

// Belso belyegszamitasokhoz kell:
/*unsigned long double2belyeg( double* px );
unsigned long double2belyeg( long l );*/

extern char Ezenlegjobbido[30];
// Azt a nevet keri, amit topol::topol kap:
void beallitlegjobbidot(char* filenev, int single);

int nevanalizis(char* nev);
