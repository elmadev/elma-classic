
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
    // demo eseten resource file-bol nyit:
    // belyeg-et adja vissza, vagy 0-at:
    friend long loadrecek(char* nev, int demo);
    friend void saverecek(char* nev, long belyeg, int tag);
    // pgazhatra-ba beepitik es ellenorzik betoltvet:
    friend void berakrecbehosszat(recorder* prec);
    friend int jorechossza(recorder* prec);
    friend void replay(void); // Hogy betolthoz hozzaferjen

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
    // demo eseten resource file-bol nyit:
    // belyeg-et adja vissza, vagy 0-at:
    long load(char* nev, FILE* h, int demo);
    void save(char* nev, FILE* h, long belyeg, int tag);

  public:
    char palyanev[16];

    recorder(void);
    ~recorder(void);
    void erase(char* palyanev);
    void rewind(void);
    // Ez a ket fuggveny  meg ezt a ket globalis valtozot is
    // beveszi es allitja: Aafogo Kozelvannak:
    int recall(motorst* pmot, double dt, hanginfo* phinfo);
    void store(motorst* pmot, double dt, hanginfo* phinfo);

    void addhang(double t, char wavindex, double hangero, int objszam);
    // Ha van hang, akkor igazzal ter vissza:
    int gethang(double t, char* pwavindex, double* phangero, int* pobjszam);

    int gettag(void);
    void settag(int tag_p);
};

extern recorder *Prec1, *Prec2;
// Ha multiplayer a bentlevo rec, akkor ez igaz:
// long lejatszo( void ) allithatja, vagy loadrecek:
extern int Multirec;

void startwavegyujto(int wavazonosito, double hangero, int objszam);
void resetwavegyujto(void);
// Igaz, ha van bent wav:
int getwavegyujto(int* pwavazonosito, double* phangero, int* pobjszam);
