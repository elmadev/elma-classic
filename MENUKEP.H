#ifndef MENUKEP_H
#define MENUKEP_H

// Ezt mv_main elott meghivja es grafikus hibakiirashoz
// kell beolvasni peldaul abc-t es hatteret (teljes.cpp-ben):
// void initmenukep1( void );
// void initmenukep2( void );

void grafikushiba(const char* text1, const char* text2 = NULL, const char* text3 = NULL);

extern pic8* Buffsima;
extern pic8* Buffgolyo;

extern abc8* Pmenuabc;

extern ddpal* Pmenupal;
extern unsigned char Menupaltomb[768];

struct egyszovsor {
    char text[102];
    int x, y;
};

#define MAXSZOVEGSOR (30)

class szoveglista {
    int sisakx, sisaky;

    egyszovsor* sorok;
    int elemszam;
    int ervenyes;

  public:
    szoveglista(void);
    ~szoveglista(void);
    void addszoveg(const char* text, int x, int y);
    void addszoveg_kozep(const char* text, int x, int y);
    void setsisak(int x, int y);
    void clear(void);
    void kirajzol(int nemkellgolyo = 0, pic8* rarakkep1 = NULL, pic8* rarakkep2 = NULL,
                  pic8* rarakkep3 = NULL, pic8* rarakkep4 = NULL);
    int kirajzolanim(double ido);
};

// Teljes olvassa be es elso kiirloading szabaditja fel:
extern pic8* Pintro;

#endif
