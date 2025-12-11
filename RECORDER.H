
struct hanginfo {
    double frekvencia;
    char gaz;
    double surlero;
};

struct egyhang {
    double t;
    short objszam;
    char wavindex;
    float hangero;
};

long loadrecek(char* nev, int demo);
void saverecek(char* nev, long belyeg, int tag);

class recorder {
    // open from res file in case of demo
    // returns 0 or belyeg
    friend long loadrecek(char* nev, int demo);
    friend void saverecek(char* nev, long belyeg, int tag);
    //build and check betoltvet for pgazhat:
    friend void berakrecbehosszat(recorder* prec);
    friend int jorechossza(recorder* prec);
    friend void replay(void); // for access to betolt

    float *pk1rx, *pk1ry;
    short *pk2rx, *pk2ry, *pk4rx, *pk4ry;
    short *pvezrx, *pvezry;
    unsigned char *pk2alfa, *pk4alfa;
    short* pk1alfa;
    unsigned char *pfrekvencia, *psurlero;
    unsigned char* pgazhatra;

    long betoltve;
    int ucsokiadva;

    egyhang* phangok;
    long hangbetoltve;
    long hangkov;

    vekt2 ucso_r;
    double ucsot, legkozt;
    int legkozindex;

    int tag;
    // open from res file in case of demo
    // returns 0 or belyeg
    long load(char* nev, FILE* h, int demo);
    void save(char* nev, FILE* h, long belyeg, int tag);

  public:
    char palyanev[16];

    recorder(void);
    ~recorder(void);
    void erase(char* palyanev);
    void rewind(void);
    // these functions set the two global variables: Aafogo Kozelvannak:
    int recall(motorst* pmot, double dt, hanginfo* phinfo);
    void store(motorst* pmot, double dt, hanginfo* phinfo);

    void addhang(double t, char wavindex, double hangero, int objszam);
    // return true if sound is enabled
    int gethang(double t, char* pwavindex, double* phangero, int* pobjszam);

    int gettag(void);
    void settag(int tag_p);
};

extern recorder *Prec1, *Prec2;
// returns true if rec is multiplayer
// can be set by long lejatszo( void ) or loadrec
extern int Multirec;

void startwavegyujto(int wavazonosito, double hangero, int objszam);
void resetwavegyujto(void);
// returns true if there is wav
int getwavegyujto(int* pwavazonosito, double* phangero, int* pobjszam);
