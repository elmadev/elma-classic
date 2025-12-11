
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

// uses Korny rubrikak and picbuffer
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
    int x0_tab; // only relevant if the one with "tab" is true
    int cimy;   // this is set here, but can be redefined elsewhere
    int escelheto;
    char cim[100];

    valaszt2(void);
    ~valaszt2(void);
    void bead(int szam, int tabos = 0); // takes in rubrics
    // returns the selected number (-1 in case of ESC)
    // in case of tobbrajz, first render multiple times (Win95):
    int valassz(egyszovsor* preszovegek = NULL, int preszovegszam = 0, int csakkirajzol = 0);

    void kirajzol(void);
};

// required for RECidomentes (also in mainmenu.cpp):
// if RECidokellhet is set, then also set RECidokellis
// in case of Ctrl+Alt+Enter (need to set RECidokellis to 0 beforehand):
extern int RECidokellhet;
extern int RECidokellis;
