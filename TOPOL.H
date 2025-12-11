
#define MAXPONT (1000) // only allocate this many if required

class topol;

class gyuru {
    int lefoglaltmaxpont;

  public:
    int pontszam;
    vekt2* ponttomb;

    // tracking(?) variables:
    int koveto;

    // functions:
    gyuru(void);
    gyuru(FILE* h, int kovetos);
    ~gyuru(void);

    int getpontszam(void);
    void save(FILE* h, topol* ptop);
    void ujkoord(int k, double x, double y);
    void kirajz(int k, int fel, int szaggatott = 0);
    void osszrajz(void);
    // returns true on success(?):
    int beszur(int k); // after k points, insert another with matching coords
    void torol(int k);
    double legkozpont(double x, double y, int* pk);
    int metszesszam(vekt2 r1, vekt2 v1);
    // this function doesn't consider intersections where the two vectors are barely touching(?)
    int metszeshely(vekt2 r1, vekt2 v1, int kihagy, vekt2* pmetszes);
    // returns true if the inside of the polygon is on the left during iteration(?);
    int pozitivkorjaras(void);
    vekt2 sulypont(void);
    // based on received px1, etc:
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
    int rxint, ryint;     // food for render while playing
    int rxint_v, ryint_v; // same thing for viewing (replay?)
    int tipus;
    int kajatipus; // grav, if food
    int foodsorszam;
    int aktiv;          // taken or not, if food while playing
    unsigned char szin; // only during play
    int sinfazisint;    // between 0-999
    double sinfazis;    // between 0-2Pi

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
    // if nev isn't valid, then [0] == 0:
    // if kepnev is valid, then the other two must be empty:
    char kepnev[10];
    char texturanev[10];
    char maszknev[10];
    vekt2 r;
    double xsize, ysize;
    // in case of 'override', distance and bordering aren't overridden based on LGR:
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
#define LEVELNAMEHOSSZ (50) // up to version 13 (14)

class lgrfile;

class topol {
    friend int menu_exit(void);
    friend int floadlevel_e(char* nev, int forceload);
    friend int floadlevel_p(char* nev);
    friend void invalidate_ptop(void);
    friend void kiirdesceket(void);
    friend void hakellmentbelleveket(void); // in topol.cpp

    double belyegszamitas(int kellspriteis);

    topol(void);
    // QWQUU001 elso palya:
    topol(char* filenev);
    ~topol(void);
    void loadkulso(char* filenev);
    void loadbelso(char* filenev);

  public:
    // only valid after input, used for checking rec files
    long belyeg;
    // set to 1 by loadlgrfile if lgr doesn't exist and default.lgr is used:
    int nincslgrje;

    int hibas, lezart;
    ptrgyuru ptomb[MAXGYURU]; // only public because of slop
    kerek* kerektomb[MAXKEREK];
    sprite* spritetomb[MAXSPRITE];
    char levelname[LEVELNAMEHOSSZ + 1];
    char lgrnev[16];
    char fgnev[10];
    char bgnev[10];

    palyaidok idok;
    int idokhelye;

    // returns true if there was change:
    int pic_selejtez(lgrfile* plgr);
    gyuru* legkozpont(double x, double y, int* pk, double* ptav = NULL, gyuru* kimarad = NULL);
    kerek* legkozkerek(double x, double y, double* ptav = NULL);
    sprite* legkozsprite(double x, double y, double* ptav = NULL);
    // check is only not required when called by saveidok:
    void save(char* filenev, int nemkellcheck = 0);
    // should only be called with originally inputted filenev:
    void saveidok(char* filenev);
    void kirajzol(void);
    // if pgy is airborne, then folddarab (piece of ground?):
    int levegoben(gyuru* pgy, vekt2* pr = NULL);
    void getminmax(double* px1, double* py1, double* px2, double* py2, int kerekkel);

    // wheel related stuff
    // sets bike position, returns kajaszam (food number)
    int setallaktiv(motorst* pmot); // activates 'kerek' objects which are food
    void killerekelore(void);       // sorts 'kerek' objects
    void kitoltiszineket(void);     // sets color of all 'kerek' objects
    kerek* getptrkerek(int index);

    // if true, y is negated (required during playing):
    int kerekekfejjellefele;
    void kereklefejjel(void);
    void kerekjolalljon(void);
};

// biker in flag tag is put back by lejatszo.cpp based on this
extern vekt2 Kezdetihelydiff; // is set by setallaktiv

// returns 0 if file exists
// treats 'nev' the same as the constructor of 'topol'
int access_topol(char* filenev);

unsigned long getbelyeg(char* nev);
// return true if 'nev' is found
int getbelyegheznev(unsigned long belyeg, char* nev);

// only for external files
// returns NULL if there were problems
// char* getlevelname( char* nev ); no longer used

// starts from 0
char* getleveldescription(int level);

// required for internal 'belyeg' calculations
/*unsigned long double2belyeg( double* px );
unsigned long double2belyeg( long l );*/

extern char Ezenlegjobbido[30];
// asks for the name received by topol::topol
void beallitlegjobbidot(char* filenev, int single);

int nevanalizis(char* nev);
