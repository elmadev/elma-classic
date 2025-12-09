
extern const int LISTegykepen;
extern const int LISTx0;
extern const int LISTy0;
extern const int LISTdy;

#define MAXRUBRIKAHOSSZ (40)
extern int Maxrubrikaszam;
#define MAXRUBRIKASZAM_TAB (110)

typedef char rubrika[MAXRUBRIKAHOSSZ + 2];

extern rubrika* Rubrikak;
extern rubrika Rubrikak_tab[MAXRUBRIKASZAM_TAB + 1];

void initrubrikak(void);

// Felhasznalja, Korny rubrikak-at es picbuffer-t!
class valaszt2 {
    friend void playextmenu(void);

    rubrika* rubrikak;
    rubrika* rubrikak_tab;
    int szam;
    int tabos;
    szoveglista* pszovlist;

  public:
    int kur;
    int x0, y0, dy, egykepen;
    int x0_tab; // Csak akkor erdekes, ha tabos igaz
    int cimy;   // Ezt o maganak beallitja, de nehanyan atdefinialjak
    int escelheto;
    char cim[100];

    valaszt2(void);
    ~valaszt2(void);
    void bead(int szam, int tabos = 0); // Rubrikakat beveszi
    // Visszaadja valasztott szamat, ESC eseten -1-et:
    // tobbrajz eseten tobbszor is kirajzolja kezdetben (Win95):
    int valassz(egyszovsor* preszovegek = NULL, int preszovegszam = 0, int csakkirajzol = 0);

    void kirajzol(void);
};

// RECidomenteshez kell (mainmenu.cpp-ben is):
// Ha RECidokellhet-et beallitjuk, akkor RECidokellis-t beallitja
// Ctrl+Alt+Enter eseten (elotte RECidokellis-t nullazni kell):
extern int RECidokellhet;
extern int RECidokellis;
