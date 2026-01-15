#ifndef TOPOL_H
#define TOPOL_H

#include "state.h"
#include "vect2.h"

class object;
class polygon;
class sprite;
struct motorst;

void set_gyuru_attributes(polygon* pgy);

#define MAXKEREK (52)
constexpr int MAX_VERTICES = 5000;

#define MAXSPRITE (5000)

#define MAXGYURU (300)

typedef polygon* ptrgyuru;
#define LEVELNAMEHOSSZ (50) // 13-as verzioig (14) volt

class lgrfile;

class topol {
    friend int menu_exit(void);
    friend int floadlevel_e(const char* nev, int forceload);
    friend int floadlevel_p(const char* nev);
    friend void invalidate_ptop(void);
    friend void kiirdesceket(void);

    double belyegszamitas(int kellspriteis);

    topol(void);
    // QWQUU001 elso palya:
    topol(const char* filenev);
    ~topol(void);
    void loadkulso(const char* filenev);
    void loadbelso(const char* filenev);

  public:
    // Beolvasas utan ervenyes, csak rec file ellenorzesere hasznalando
    long belyeg;
    // loadlgrfile allitja 1-be, ha nincs meg lgr es default.lgr-t hasznal:
    int nincslgrje;

    int hibas, lezart;
    ptrgyuru ptomb[MAXGYURU]; // Ez csak gany miatt public
    object* kerektomb[MAXKEREK];
    sprite* spritetomb[MAXSPRITE];
    char levelname[LEVELNAMEHOSSZ + 1];
    char lgrnev[16];
    char fgnev[10];
    char bgnev[10];

    topten_set idok;
    int idokhelye;

    // Igazzal ter vissza, ha volt valtozas:
    int pic_selejtez(lgrfile* plgr);
    polygon* legkozpont(double x, double y, int* pk, double* ptav = NULL, polygon* kimarad = NULL);
    object* legkozkerek(double x, double y, double* ptav = NULL);
    sprite* legkozsprite(double x, double y, double* ptav = NULL);
    // Ha saveidok hivja meg, csak akkor nem kell check:
    void save(const char* filenev, int nemkellcheck = 0);
    // Csak eredetileg beolvasott filenev-vel szabad meghivni:
    void saveidok(const char* filenev);
    void kirajzol(void);
    // pgy levegoben van-e. Akkor folddarab:
    int levegoben(polygon* pgy, vect2* pr = NULL);
    void getminmax(double* px1, double* py1, double* px2, double* py2, int kerekkel);

    // Kerekkel kapcsolatos dolgok:
    // Motor helyzetet is o allitja be, kajaszamot adja vissza:
    int setallaktiv(motorst* pmot); // eheto kerekeket aktivizalja
    void killerekelore(void);       // kerekeket rendezi
    object* getptrkerek(int index);

    // Ha igaz, y negalva van (ez kell lejatszasnal):
    int kerekekfejjellefele;
    void kereklefejjel(void);
    void kerekjolalljon(void);
};

// lejatszo.cpp-ben Falg Tag ennek alapjan teszi vissza motorost:
extern vect2 Kezdetihelydiff; // setallaktiv allitja be

// Ha letezik file, akkor 0-t ad vissza!:
// Ugyanugy viselkedik nevvel, mint topol konstruktora:
int access_topol(const char* filenev);

// Csak external file-hoz:
// Ha NULL, akkor valami baj volt vele:
// char* getlevelname( char* nev ); mar nem hasznalt

// 0-tol kezdodnek:
const char* getleveldescription(int level);

// Belso belyegszamitasokhoz kell:
/*unsigned long double2belyeg( double* px );
unsigned long double2belyeg( long l );*/

extern char Ezenlegjobbido[30];
// Azt a nevet keri, amit topol::topol kap:
void beallitlegjobbidot(const char* filenev, int single);

int nevanalizis(const char* nev);

#endif
