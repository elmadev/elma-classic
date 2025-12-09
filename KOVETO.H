

#define MAXKOVETOKEPSZAM (100)

// Hany pixel magasan log tul textura:
#define FUGGTOLDAS (20)

class koveto {
  public:
    int kepszam;
    pic8* pkeptomb[MAXKOVETOKEPSZAM];
    int feltomb[MAXKOVETOKEPSZAM];
    koveto(void);
    ~koveto(void);
    void addkep(pic8* ppic, int up);
};

// Ez nem hasznal semmit koveto adatokbol, csak Ptop-bol:
int makevonal(gyuru* pgy, int* ytomb, int* phossz, int* px0, int maxhely, vekt2* porigo);

void makekoveto(void);

void tesztkoveto(void);
