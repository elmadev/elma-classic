#include "all.h"

#define DEFAULTSIZE (20.0)

#define IDOEGYEDISZAM1 (6754362)
#define IDOEGYEDISZAM2 (8674642)

// values are displayed as comments also

#define BELYEGSZORZO (3247.764325643)

// if defined save with internal lev format, .leb extension
// save as if shareware, as well as the function hakellmentbelleveket
// all levels are saved like this:
// #define BELSOTIR

static char Levelnevek[MAXPALYASZAM + 2][14] = {
    "nulla.leb",    "a01.leb",    "a02.leb",      "a03.leb",   "a04.leb",     "a05.leb",
    "a06.leb",      "a07.leb",    "ujtag.leb",    "a08.leb",   "a09.leb",     "ujgrav.leb",
    "a10.leb",

    "a11.leb",      "a12.leb",    "a13.leb",      "a14.leb",   "a15.leb",     "a16.leb",
    "a17.leb",

    "ujupdown.leb", "a18.leb",    "a19.leb",      "a20.leb",

    "a21.leb",      "a22.leb",    "a23.leb",      "a24.leb",   "a25.leb",     "a26.leb",
    "ujkomb.leb",   "a27.leb",    "ujtolcs.leb",  "a28.leb",   "ujzuhan.leb", "a29.leb",
    "a30.leb",

    "a31.leb",      "a32.leb",    "ujvissza.leb", "a33.leb",   "a34.leb",     "a35.leb",
    "a36.leb",      "a37.leb",    "ujcsab.leb",   "Mate.leb",  "a38.leb",     "ujdownhi.leb",
    "ujcsomo.leb",  "a39.leb",    "a40.leb",

    "a41.leb",      "ujhook.leb", "a42.leb",      "visit.leb", "a04.leb",     "a05.leb",
    "a06.leb",      "a07.leb",    "a08.leb",      "a09.leb",   "a10.leb"};

int Leveldescinited = 0;
#define MAXBEEPLEVELSZAM (55)
#define DESCNEVHOSSZ (30)
char Leveldescek[(DESCNEVHOSSZ + 2) * MAXBEEPLEVELSZAM + 6];

// also exits
void kiirdesceket(void) {
    FILE* h = fopen("desclist.txt", "wt");
    if (!h) {
        hiba("6deyhqe");
    }

    int nemolvastobbet = 0;
    for (int i = 0; i < MAXBEEPLEVELSZAM; i++) {
        if (nemolvastobbet || i >= MAXPALYASZAM) {
            fprintf(h, "Itt nincs nev\n");
            continue;
        }
        // generate "QWQUU" names:
        char tmp[10];
        sprintf(tmp, "%d", i + 1);
        char nev[20];
        strcpy(nev, "QWQUU");
        if (i + 1 < 10) {
            strcat(nev, "0");
        }
        if (i + 1 < 100) {
            strcat(nev, "0");
        }
        strcat(nev, tmp);
        strcat(nev, ".LEV");

        if (access_topol(nev) != 0) {
            // this topol file no longer exists
            nemolvastobbet = 1;
            fprintf(h, "Itt nincs nev\n");
            continue;
        }
        topol* ptop = new topol(nev);
        fprintf(h, "%s\n", ptop->levelname);
        delete ptop;
        ptop = NULL;
    }

    fclose(h);
    hiba("desclist.txt displayed!");
}

// this is declared as friend of topol
//and is called by m_v.cpp on every start(??:
void hakellmentbelleveket(void) {
#ifdef BELSOTIR
    finame fname;
    int done = fifirst("lev/*.lev", fname);
    // iterate through all lev files and save them as internal
    while (!done) {
        topol* ptop = new topol(fname);
        ptop->save(fname, 1);
        delete ptop;

        done = finext(fname);
    }

    ficlose();

    hiba("lev files displayed as internal!");
#endif
}

static char Sor[110];

// Van ugyanilyen neven state.cpp-ben is ket fv.:

// returns successful:
static int titkosread(void* mut, int hossz, FILE* h) {
    if (fread(mut, 1, hossz, h) != hossz) {
        return 0;
    }
    unsigned char* pc = (unsigned char*)mut;
    short a = 21, b = 9783, c = 3389;

    for (int i = 0; i < hossz; i++) {
        pc[i] ^= a;
        a %= c;
        b += a * c;
        a = 31 * b + c;
    }
    return 1;
}

// returns successful:
static int titkoswrite(void* mut, int hossz, FILE* h) {
    unsigned char* pc = (unsigned char*)mut;
    short a = 21, b = 9783, c = 3389;

    for (int i = 0; i < hossz; i++) {
        pc[i] ^= a;
        a %= c;
        b += a * c;
        a = 31 * b + c;
    }
    if (fwrite(mut, 1, hossz, h) != hossz) {
        return 0;
    }
    a = 21, b = 9783, c = 3389;

    for (int i = 0; i < hossz; i++) {
        pc[i] ^= a;
        a %= c;
        b += a * c;
        a = 31 * b + c;
    }
    return 1;
}

char* getleveldescription(int level) {
    if (!Leveldescinited) {
        Leveldescinited = 1;
        FILE* h = qopen("desclist.txt", "rt");
        if (!h) {
            kiirdesceket(); // also exits
        }

        for (int i = 0; i < MAXBEEPLEVELSZAM; i++) {
            if (!fgets(Sor, 100, h)) {
                hiba("7de2y7");
            }
            if (strchr(Sor, '\n')) {
                *strchr(Sor, '\n') = 0;
            }
            if (strlen(Sor) > DESCNEVHOSSZ) {
                hiba("tct86");
            }
            strcpy(&Leveldescek[i * (DESCNEVHOSSZ + 2)], Sor);
        }

        qclose(h);
    }
    if (level < 0 || level >= MAXBEEPLEVELSZAM) {
        hiba("8u9r5tg");
    }
    if (level == 30) { // Animal Farm
        return "Animal Farm";
    }
    return &Leveldescek[level * (DESCNEVHOSSZ + 2)];
}

// KEREK KEREK KEREK KEREK KEREK KEREK KEREK KEREK KEREK2
// KEREK KEREK KEREK KEREK KEREK KEREK KEREK KEREK KEREK
// KEREK KEREK KEREK KEREK KEREK KEREK KEREK KEREK KEREK
// KEREK KEREK KEREK KEREK KEREK KEREK KEREK KEREK KEREK

kerek::kerek(double x, double y, int tipusp) {
    r.x = x;
    r.y = y;
    tipus = tipusp;
    kajatipus = 0; // need to move on it to make it food
    foodsorszam = 0;
}

static const double Hal_eltol = -0.15 + 0.3 * (0.07 / 0.37);

void kerek::kirajzol(void) {
    int szegmensszam = 16;
    double sugar = 0.4;

    double elfordulas = 2.0 * K_pi / szegmensszam;
    for (int i = 0; i < szegmensszam; i++) {
        double alfa1 = i * elfordulas;
        double alfa2 = (i + 1) * elfordulas;
        vekt2 r1(sugar * sin(alfa1), sugar * cos(alfa1));
        vekt2 r2(sugar * sin(alfa2), sugar * cos(alfa2));
        vonalrajz(r + r1, r + r2, 0);
    }

    // small sign on middle of circle
    double meret = 0.02;
    vonalrajz(r + vekt2(-meret, -meret), r + vekt2(meret, meret), 0);
    vonalrajz(r + vekt2(meret, -meret), r + vekt2(-meret, meret), 0);

    // draw letter inside circle
    if (tipus == T_CEL) {
        vonalrajz(r + vekt2(-0.15, 0.3), r + vekt2(-0.15, -0.3), 0);
        vonalrajz(r + vekt2(-0.15, 0.3), r + vekt2(0.15, 0.3), 0);
        vonalrajz(r + vekt2(-0.15, -0.3), r + vekt2(0.15, -0.3), 0);
        vonalrajz(r + vekt2(-0.15, 0.0), r + vekt2(0.1, 0.0), 0);
        return;
    }
    if (tipus == T_KAJA) {
        vonalrajz(r + vekt2(-0.15, 0.3), r + vekt2(-0.15, -0.3), 0);
        vonalrajz(r + vekt2(-0.15, -0.3), r + vekt2(0.15, -0.3), 0);
        vonalrajz(r + vekt2(-0.15, 0.0), r + vekt2(0.1, 0.0), 0);
        return;
    }
    if (tipus == T_KEZDO) {
        vonalrajz(r + vekt2(0.15, 0.3), r + vekt2(0.15, 0.0), 0);
        vonalrajz(r + vekt2(-0.15, -0.3), r + vekt2(-0.15, 0.0), 0);

        vonalrajz(r + vekt2(-0.15, 0.3), r + vekt2(0.15, 0.3), 0);
        vonalrajz(r + vekt2(-0.15, -0.3), r + vekt2(0.15, -0.3), 0);
        vonalrajz(r + vekt2(-0.15, 0.0), r + vekt2(0.15, 0.0), 0);
        return;
    }
    if (tipus == T_HALALOS) {
        vonalrajz(r + vekt2(-0.15, 0.3), r + vekt2(-0.15, -0.3), 0);
        vonalrajz(r + vekt2(-0.15, 0.07), r + vekt2(0.15, -0.3), 0);
        vonalrajz(r + vekt2(Hal_eltol, 0.0), r + vekt2(0.15, 0.3), 0);
        return;
    }
    hiba("iugiffiif");
}

kerek::kerek(FILE* h, int verzio) {
    if (fread(&r.x, 1, sizeof(r.x), h) != sizeof(r.x)) {
        hiba("Cannot read from file in kerek::kerek!");
    }
    if (fread(&r.y, 1, sizeof(r.y), h) != sizeof(r.y)) {
        hiba("Cannot read from file in kerek::kerek!");
    }
    if (fread(&tipus, 1, sizeof(tipus), h) != sizeof(tipus)) {
        hiba("Cannot read from file in kerek::kerek!");
    }

    kajatipus = 0;
    if (verzio >= 9) {
        if (fread(&kajatipus, 1, 4, h) != 4) {
            hiba("Cannot read from file in kerek::kerek!");
        }
    }
    foodsorszam = 0;
    if (verzio >= 11) {
        if (fread(&foodsorszam, 1, 4, h) != 4) {
            hiba("Cannot read from file in kerek::kerek!");
        }
    }
    if (foodsorszam < 0 || foodsorszam > 8) {
        hiba("65767yr");
    }
}

void kerek::save(FILE* h) {
    if (fwrite(&r.x, 1, sizeof(r.x), h) != sizeof(r.x)) {
        hiba("Cannot write to file in kerek::save!");
    }
    if (fwrite(&r.y, 1, sizeof(r.y), h) != sizeof(r.y)) {
        hiba("Cannot write to file in kerek::save!");
    }
    if (fwrite(&tipus, 1, sizeof(tipus), h) != sizeof(tipus)) {
        hiba("Cannot write to file in kerek::save!");
    }
    if (fwrite(&kajatipus, 1, 4, h) != 4) {
        hiba("Cannot write to file in kerek::save!");
    }
    if (fwrite(&foodsorszam, 1, 4, h) != 4) {
        hiba("Cannot write to file in kerek::save!");
    }
}

double kerek::belyegszamitas(void) {
    double belyeg = 0;
    belyeg += r.x;
    belyeg += r.y;
    belyeg += tipus;
    return belyeg;
}

// SPRITE SPRITE SPRITE SPRITE SPRITE SPRITE SPRITE SPRITE SPRITE SPRITE
// SPRITE SPRITE SPRITE SPRITE SPRITE SPRITE SPRITE SPRITE SPRITE SPRITE
// SPRITE SPRITE SPRITE SPRITE SPRITE SPRITE SPRITE SPRITE SPRITE SPRITE
// SPRITE SPRITE SPRITE SPRITE SPRITE SPRITE SPRITE SPRITE SPRITE SPRITE
// SPRITE SPRITE SPRITE SPRITE SPRITE SPRITE SPRITE SPRITE SPRITE SPRITE
// SPRITE SPRITE SPRITE SPRITE SPRITE SPRITE SPRITE SPRITE SPRITE SPRITE

sprite::sprite(double x, double y, char* kepnev_p, char* texturanev_p, char* maszknev_p) {
    if (!Plgr) {
        hiba("7uyhfg");
    }

    r = vekt2(x, y);
    if (strlen(kepnev_p) > 8 || strlen(maszknev_p) > 8 || strlen(texturanev_p) > 8) {
        hiba("87vgrg");
    }
    strcpy(kepnev, kepnev_p);
    strcpy(texturanev, texturanev_p);
    strcpy(maszknev, maszknev_p);
    tavolsag = 100;
    hatarol = 0;

    // Default size:
    xsize = Aranyrec * DEFAULTSIZE;
    ysize = Aranyrec * DEFAULTSIZE;

    if (kepnev[0] && (maszknev[0] || texturanev[0])) {
        hiba("Too many names per polygon in sp::sp!");
    }
    if (kepnev[0]) {
        int index = Plgr->getkepindex(kepnev);
        if (index < 0) {
            kepnev[0] = 0;
        } else {
            // kep is acquired:
            xsize = Plgr->kepek[index].xsize;
            ysize = Plgr->kepek[index].ysize;
            xsize *= Aranyrec;
            ysize *= Aranyrec;
            tavolsag = Plgr->kepek[index].tavolsag;
            hatarol = Plgr->kepek[index].hatarol;
        }
    } else {
        // not kep sprite:
        if (maszknev[0]) {
            int index = Plgr->getmaszkindex(maszknev);
            if (index < 0) {
                maszknev[0] = 0;
            } else {
                xsize = Plgr->maszkok[index].xsize;
                ysize = Plgr->maszkok[index].ysize;
                xsize *= Aranyrec;
                ysize *= Aranyrec;
            }
        }
        if (texturanev[0]) {
            int index = Plgr->gettexturaindex(texturanev);
            if (index < 0) {
                texturanev[0] = 0;
            } else {
                tavolsag = Plgr->texturak[index].tavolsag;
                hatarol = Plgr->texturak[index].hatarol;
            }
        }
    }
}

void sprite::kirajzol(void) {
    vonalrajz(r, r + vekt2(xsize, 0.0), 0);
    vonalrajz(r, r + vekt2(0.0, ysize), 0);
    vekt2 r2(xsize + r.x, ysize + r.y);
    vonalrajz(r2, r2 - vekt2(xsize, 0.0), 0);
    vonalrajz(r2, r2 - vekt2(0.0, ysize), 0);
}

sprite::sprite(FILE* h) {
    int meret = sizeof(tavolsag);
    if (meret != 4) {
        hiba("sizeof( tavolsag ) != 4 in sprite::sprite!");
    }

    if (fread(kepnev, 1, 10, h) != 10) {
        hiba("Nemo sp!");
    }
    kepnev[10] = 0; // security measure
    if (fread(texturanev, 1, 10, h) != 10) {
        hiba("Nemo sp!");
    }
    texturanev[10] = 0; // security measure
    if (fread(maszknev, 1, 10, h) != 10) {
        hiba("Nemo sp!");
    }
    maszknev[10] = 0; // security measure

    if (fread(&r.x, 1, sizeof(r.x), h) != sizeof(r.x)) {
        hiba("Cannot read from file in sprite::sprite!");
    }
    if (fread(&r.y, 1, sizeof(r.y), h) != sizeof(r.y)) {
        hiba("Cannot read from file in sprite::sprite!");
    }
    if (fread(&tavolsag, 1, sizeof(tavolsag), h) != sizeof(tavolsag)) {
        hiba("Cannot read from file in sprite::sprite!");
    }
    if (fread(&hatarol, 1, sizeof(hatarol), h) != sizeof(hatarol)) {
        hiba("Cannot read from file in sprite::sprite!");
    }
}

void sprite::save(FILE* h) {
    if (fwrite(kepnev, 1, 10, h) != 10) {
        hiba("Nir sp!");
    }
    if (fwrite(texturanev, 1, 10, h) != 10) {
        hiba("Nir sp!");
    }
    if (fwrite(maszknev, 1, 10, h) != 10) {
        hiba("Nir sp!");
    }
    if (fwrite(&r.x, 1, sizeof(r.x), h) != sizeof(r.x)) {
        hiba("Nir sp!");
    }
    if (fwrite(&r.y, 1, sizeof(r.y), h) != sizeof(r.y)) {
        hiba("Nir sp!");
    }
    if (fwrite(&tavolsag, 1, sizeof(tavolsag), h) != sizeof(tavolsag)) {
        hiba("Nir sp!");
    }
    if (fwrite(&hatarol, 1, sizeof(hatarol), h) != sizeof(hatarol)) {
        hiba("Nir sp!");
    }
}

double sprite::belyegszamitas(void) {
    double belyeg = 0;
    belyeg += r.x;
    belyeg += r.y;
    return belyeg;
}

// TOPOL TOPOL TOPOL TOPOL TOPOL TOPOL TOPOL TOPOL TOPOL TOPOL TOPOL TOPOL
// TOPOL TOPOL TOPOL TOPOL TOPOL TOPOL TOPOL TOPOL TOPOL TOPOL TOPOL TOPOL
// TOPOL TOPOL TOPOL TOPOL TOPOL TOPOL TOPOL TOPOL TOPOL TOPOL TOPOL TOPOL
// TOPOL TOPOL TOPOL TOPOL TOPOL TOPOL TOPOL TOPOL TOPOL TOPOL TOPOL TOPOL
// TOPOL TOPOL TOPOL TOPOL TOPOL TOPOL TOPOL TOPOL TOPOL TOPOL TOPOL TOPOL
// TOPOL TOPOL TOPOL TOPOL TOPOL TOPOL TOPOL TOPOL TOPOL TOPOL TOPOL TOPOL
// TOPOL TOPOL TOPOL TOPOL TOPOL TOPOL TOPOL TOPOL TOPOL TOPOL TOPOL TOPOL
// TOPOL TOPOL TOPOL TOPOL TOPOL TOPOL TOPOL TOPOL TOPOL TOPOL TOPOL TOPOL
// TOPOL TOPOL TOPOL TOPOL TOPOL TOPOL TOPOL TOPOL TOPOL TOPOL TOPOL TOPOL
// TOPOL TOPOL TOPOL TOPOL TOPOL TOPOL TOPOL TOPOL TOPOL TOPOL TOPOL TOPOL
// TOPOL TOPOL TOPOL TOPOL TOPOL TOPOL TOPOL TOPOL TOPOL TOPOL TOPOL TOPOL
// TOPOL TOPOL TOPOL TOPOL TOPOL TOPOL TOPOL TOPOL TOPOL TOPOL TOPOL TOPOL
// TOPOL TOPOL TOPOL TOPOL TOPOL TOPOL TOPOL TOPOL TOPOL TOPOL TOPOL TOPOL
// TOPOL TOPOL TOPOL TOPOL TOPOL TOPOL TOPOL TOPOL TOPOL TOPOL TOPOL TOPOL
// TOPOL TOPOL TOPOL TOPOL TOPOL TOPOL TOPOL TOPOL TOPOL TOPOL TOPOL TOPOL
// TOPOL TOPOL TOPOL TOPOL TOPOL TOPOL TOPOL TOPOL TOPOL TOPOL TOPOL TOPOL
// TOPOL TOPOL TOPOL TOPOL TOPOL TOPOL TOPOL TOPOL TOPOL TOPOL TOPOL TOPOL
// TOPOL TOPOL TOPOL TOPOL TOPOL TOPOL TOPOL TOPOL TOPOL TOPOL TOPOL TOPOL
// TOPOL TOPOL TOPOL TOPOL TOPOL TOPOL TOPOL TOPOL TOPOL TOPOL TOPOL TOPOL
// TOPOL TOPOL TOPOL TOPOL TOPOL TOPOL TOPOL TOPOL TOPOL TOPOL TOPOL TOPOL
// TOPOL TOPOL TOPOL TOPOL TOPOL TOPOL TOPOL TOPOL TOPOL TOPOL TOPOL TOPOL
// TOPOL TOPOL TOPOL TOPOL TOPOL TOPOL TOPOL TOPOL TOPOL TOPOL TOPOL TOPOL
// TOPOL TOPOL TOPOL TOPOL TOPOL TOPOL TOPOL TOPOL TOPOL TOPOL TOPOL TOPOL
// TOPOL TOPOL TOPOL TOPOL TOPOL TOPOL TOPOL TOPOL TOPOL TOPOL TOPOL TOPOL
// TOPOL TOPOL TOPOL TOPOL TOPOL TOPOL TOPOL TOPOL TOPOL TOPOL TOPOL TOPOL
// TOPOL TOPOL TOPOL TOPOL TOPOL TOPOL TOPOL TOPOL TOPOL TOPOL TOPOL TOPOL
// TOPOL TOPOL TOPOL TOPOL TOPOL TOPOL TOPOL TOPOL TOPOL TOPOL TOPOL TOPOL
// TOPOL TOPOL TOPOL TOPOL TOPOL TOPOL TOPOL TOPOL TOPOL TOPOL TOPOL TOPOL
// TOPOL TOPOL TOPOL TOPOL TOPOL TOPOL TOPOL TOPOL TOPOL TOPOL TOPOL TOPOL
// TOPOL TOPOL TOPOL TOPOL TOPOL TOPOL TOPOL TOPOL TOPOL TOPOL TOPOL TOPOL
// TOPOL TOPOL TOPOL TOPOL TOPOL TOPOL TOPOL TOPOL TOPOL TOPOL TOPOL TOPOL
// TOPOL TOPOL TOPOL TOPOL TOPOL TOPOL TOPOL TOPOL TOPOL TOPOL TOPOL TOPOL
// TOPOL TOPOL TOPOL TOPOL TOPOL TOPOL TOPOL TOPOL TOPOL TOPOL TOPOL TOPOL
// TOPOL TOPOL TOPOL TOPOL TOPOL TOPOL TOPOL TOPOL TOPOL TOPOL TOPOL TOPOL
// TOPOL TOPOL TOPOL TOPOL TOPOL TOPOL TOPOL TOPOL TOPOL TOPOL TOPOL TOPOL
// TOPOL TOPOL TOPOL TOPOL TOPOL TOPOL TOPOL TOPOL TOPOL TOPOL TOPOL TOPOL
// TOPOL TOPOL TOPOL TOPOL TOPOL TOPOL TOPOL TOPOL TOPOL TOPOL TOPOL TOPOL
// TOPOL TOPOL TOPOL TOPOL TOPOL TOPOL TOPOL TOPOL TOPOL TOPOL TOPOL TOPOL
// TOPOL TOPOL TOPOL TOPOL TOPOL TOPOL TOPOL TOPOL TOPOL TOPOL TOPOL TOPOL
// TOPOL TOPOL TOPOL TOPOL TOPOL TOPOL TOPOL TOPOL TOPOL TOPOL TOPOL TOPOL
// TOPOL TOPOL TOPOL TOPOL TOPOL TOPOL TOPOL TOPOL TOPOL TOPOL TOPOL TOPOL
// TOPOL TOPOL TOPOL TOPOL TOPOL TOPOL TOPOL TOPOL TOPOL TOPOL TOPOL TOPOL

topol::topol(void) {
    belyeg = 0;
    nincslgrje = 0;
    kerekekfejjellefele = 0;
    hibas = 0;
    lezart = 0;
    idokhelye = 0;

    for (int i = 0; i < MAXGYURU; i++) {
        ptomb[i] = NULL;
    }
    for (int i = 0; i < MAXKEREK; i++) {
        kerektomb[i] = NULL;
    }
    for (int i = 0; i < MAXSPRITE; i++) {
        spritetomb[i] = NULL;
    }

    memset(&idok, 0, sizeof(idok));

    ptomb[0] = new gyuru;
    kerektomb[0] = new kerek(-2, 0.5, T_CEL);
    kerektomb[1] = new kerek(2, 0.5, T_KEZDO);
    strcpy(levelname, "Unnamed");
    strcpy(lgrnev, "DEFAULT");
    strcpy(fgnev, "ground");
    strcpy(bgnev, "sky");
}

topol::~topol(void) {
    for (int i = 0; i < MAXGYURU; i++) {
        if (ptomb[i]) {
            delete ptomb[i];
        }
        ptomb[i] = NULL;
    }
    for (int i = 0; i < MAXKEREK; i++) {
        if (kerektomb[i]) {
            delete kerektomb[i];
        }
        kerektomb[i] = NULL;
    }
    for (int i = 0; i < MAXSPRITE; i++) {
        if (spritetomb[i]) {
            delete spritetomb[i];
        }
        spritetomb[i] = NULL;
    }
}

// return true if changed:
int topol::pic_selejtez(lgrfile* plgr) {
    // delete all sprites if they are not in LGR
    // kep, maszk, or textura nev:

    // can be used for dialogue:
    int volttorles = 0;
    for (int i = 0; i < MAXSPRITE; i++) {
        if (spritetomb[i]) {
            sprite* psp = spritetomb[i];
            // Default size:
            psp->xsize = Aranyrec * DEFAULTSIZE;
            psp->ysize = Aranyrec * DEFAULTSIZE;

            if (psp->kepnev[0] && (psp->maszknev[0] || psp->texturanev[0])) {
                hiba("Too many names per sprite in pic_selejtez!");
            }
            if (psp->kepnev[0]) {
                int index = plgr->getkepindex(psp->kepnev);
                if (index < 0) {
                    psp->kepnev[0] = 0;
                    // delete
                    delete spritetomb[i];
                    spritetomb[i] = NULL;
                    volttorles = 1;
                    continue;
                }
                // kep is acquired
                psp->xsize = plgr->kepek[index].xsize;
                psp->ysize = plgr->kepek[index].ysize;
                psp->xsize *= Aranyrec;
                psp->ysize *= Aranyrec;
            } else {
                // not kep sprite:
                if (psp->maszknev[0]) {
                    int index = plgr->getmaszkindex(psp->maszknev);
                    if (index < 0) {
                        psp->maszknev[0] = 0;
                        // delete
                        delete spritetomb[i];
                        spritetomb[i] = NULL;
                        volttorles = 1;
                    } else {
                        psp->xsize = plgr->maszkok[index].xsize;
                        psp->ysize = plgr->maszkok[index].ysize;
                        psp->xsize *= Aranyrec;
                        psp->ysize *= Aranyrec;
                    }
                }
                if (psp->texturanev[0]) {
                    int index = plgr->gettexturaindex(psp->texturanev);
                    if (index < 0) {
                        psp->texturanev[0] = 0;
                        // delete
                        delete spritetomb[i];
                        spritetomb[i] = NULL;
                        volttorles = 1;
                    }
                }
            }
        }
    }

    // compactify sprites to front of spritetomb:
    int voltvaltozas = 1;
    while (voltvaltozas) {
        voltvaltozas = 0;
        for (int i = 0; i < MAXSPRITE - 1; i++) {
            if (!spritetomb[i] && spritetomb[i + 1]) {
                // get next pointer here
                spritetomb[i] = spritetomb[i + 1];
                spritetomb[i + 1] = NULL;
                voltvaltozas = 1;
            }
        }
    }

    // select and refresh fgnev and bgnev
    if (strcmpi(Ptop->fgnev, Ptop->bgnev) == 0) {
        Ptop->bgnev[0] = 0;
    }
    if (Plgr->gettexturaindex(Ptop->fgnev) < 0) {
        Ptop->fgnev[0] = 0;
    }
    if (Plgr->gettexturaindex(Ptop->bgnev) < 0) {
        Ptop->bgnev[0] = 0;
    }

    if (Plgr->texturaszam < 2) {
        hiba("8u9yr");
    }

    if (!Ptop->fgnev[0]) {
        if (strcmpi(Ptop->bgnev, Plgr->texturak[0].nev) != 0) {
            strcpy(Ptop->fgnev, Plgr->texturak[0].nev);
        } else {
            strcpy(Ptop->fgnev, Plgr->texturak[1].nev);
        }
    }

    if (!Ptop->bgnev[0]) {
        if (strcmpi(Ptop->fgnev, Plgr->texturak[0].nev) != 0) {
            strcpy(Ptop->bgnev, Plgr->texturak[0].nev);
        } else {
            strcpy(Ptop->bgnev, Plgr->texturak[1].nev);
        }
    }
    return volttorles;
}

gyuru* topol::legkozpont(double x, double y, int* pk, double* ptav, gyuru* kimarad) {
    if (ptav) {
        *ptav = 1000000000.0;
    }

    double tav = 1000000.0;
    gyuru* pgy = NULL;
    for (int i = 0; i < MAXGYURU; i++) {
        if (!ptomb[i]) {
            continue;
        }
        if (ptomb[i] == kimarad) {
            continue;
        }
        if (ptomb[i]->koveto && !Rajzolkoveto) {
            continue;
        }
        if (!ptomb[i]->koveto && !Rajzolpoligon) {
            continue;
        }

        int ujk;
        double ujtav = ptomb[i]->legkozpont(x, y, &ujk);
        if (ujtav < tav) {
            tav = ujtav;
            *pk = ujk;
            pgy = ptomb[i];
        }
    }
    if (tav > display_double_kozeltav()) {
        *pk = 0;
        return NULL;
    }
    if (ptav) {
        *ptav = tav;
    }
    return pgy;
}

kerek* topol::legkozkerek(double x, double y, double* ptav) {
    if (ptav) {
        *ptav = 1000000000.0;
    }

    double tav = 1000000.0;
    kerek* pker = NULL;
    vekt2 r(x, y);
    for (int i = 0; i < MAXKEREK; i++) {
        if (kerektomb[i]) {
            double ujtav = abs(kerektomb[i]->r - r);
            if (ujtav < tav) {
                tav = ujtav;
                pker = kerektomb[i];
            }
        }
    }
    if (tav > display_double_kozeltav()) {
        return NULL;
    }
    if (ptav) {
        *ptav = tav;
    }
    return pker;
}

sprite* topol::legkozsprite(double x, double y, double* ptav) {
    if (ptav) {
        *ptav = 1000000000.0;
    }

    double tav = 1000000.0;
    sprite* psp = NULL;
    vekt2 r(x, y);
    for (int i = 0; i < MAXSPRITE; i++) {
        if (!Rajzolkepek) {
            continue;
        }
        if (!spritetomb[i]) {
            continue;
        }

        double ujtav = abs(spritetomb[i]->r - r);
        if (ujtav < tav) {
            tav = ujtav;
            psp = spritetomb[i];
        }
    }
    if (tav > display_double_kozeltav()) {
        return NULL;
    }
    if (ptav) {
        *ptav = tav;
    }
    return psp;
}

void topol::kirajzol(void) {
    for (int i = 0; i < MAXGYURU; i++) {
        if (ptomb[i]) {
            if (ptomb[i]->koveto) {
                // Koveto objects:
                if (Rajzolkoveto) {
                    ptomb[i]->osszrajz();
                }
            } else {
                // Normal poligon objects:
                if (Rajzolpoligon) {
                    ptomb[i]->osszrajz();
                }
            }
        }
    }
    for (int i = 0; i < MAXKEREK; i++) {
        if (kerektomb[i]) {
            kerektomb[i]->kirajzol();
        }
    }
    for (int i = 0; i < MAXSPRITE; i++) {
        if (spritetomb[i]) {
            if (Rajzolkepek) {
                spritetomb[i]->kirajzol();
            }
        }
    }
}

// return true if inside(?):
int topol::levegoben(gyuru* pgy, vekt2* pr) {
    if ((pgy && pr) || (!pgy && !pr)) {
        hiba("Wrong params in topol::levego!");
    }

    vekt2 r;
    if (pgy) {
        r = pgy->ponttomb[0]; // pgy->sulypont();
    } else {
        r = *pr;
    }
    vekt2 v = vekt2(27654.475374565578, 37850.5364775);
    // iterate through everything
    int metszes = 0;
    for (int i = 0; i < MAXGYURU; i++) {
        if (ptomb[i] && !ptomb[i]->koveto && ptomb[i] != pgy) {
            metszes += ptomb[i]->metszesszam(r, v);
        }
    }
    if (metszes % 2) {
        return 1;
    } else {
        return 0;
    }
}

int nevanalizis(char* nev) {
    char ctmp = nev[5];
    nev[5] = 0;
    // (this string is also hardcoded in rekorder.cpp)
    if (strcmpi(nev, "QWQUU") != 0) {
        nev[5] = ctmp;
        return 0;
    }
    nev[5] = ctmp;

    char* fut = &nev[5];
    int sum = 0;
    for (int i = 0; i < 3; i++) {
        sum *= 10;
        int add = *fut - '0';
        if (add < 0 || add > 9) {
            return 0;
        }
        sum += add;
        fut++;
    }

    if (strcmpi(&nev[8], ".lev") != 0) {
        return 0;
    }

    if (sum <= Palyaszam) {
        return sum;
    } else {
        if (sum == 34) { // also search for 33
            return sum;  // in case of shareware
        }
        return 0;
    }
}

// return 0 if file exists
int access_topol(char* filenev) {
    int belsopalyaszam = nevanalizis(filenev);
    if (belsopalyaszam > 0) {
        return 0;
    }
    char tmp[30];
    sprintf(tmp, "lev/%s", filenev);
    return access(tmp, 0);
}

char Ezenlegjobbido[30] = "";

// asks for the name received by topol::topol
void beallitlegjobbidot(char* filenev, int single) {
    if (strlen(filenev) > 20) {
        hiba("4534516");
    }

    int belsopalyaszam = nevanalizis(filenev);
    palyaidok* idok = NULL;
    if (belsopalyaszam > 0) {
        belsopalyaszam--; // because it's bigger than the index by one
                          // internal lev
        if ((belsopalyaszam != 33) && (belsopalyaszam >= Palyaszam)) {
            hiba("67467524");
        }

        idok = &State->palyakidejei[belsopalyaszam];
    } else {
        // external lev
        if (!Ptop) {
            hiba("78236");
        }
        idok = &Ptop->idok;
    }

    palyaegyfeleidok* egyfeleidok = NULL;
    if (single) {
        egyfeleidok = &idok->singleidok;
    } else {
        egyfeleidok = &idok->multiidok;
    }

    if (egyfeleidok->idokszama > 0) {
        ido2string(egyfeleidok->idok[0], Ezenlegjobbido);
    } else {
        Ezenlegjobbido[0] = 0;
    }
}

// first level is indexed 1
static char* Lgrtabla[110] = {NULL, NULL}; // looks for init of first index lev

static char Szo[50] = "";

static void makelgrtabla(void) {
    if (Palyaszam > 105) {
        hiba("uierty895");
    }

    for (int i = 0; i < 110; i++) {
        Lgrtabla[i] = NULL;
    }

    static char* chartomb = NULL;

    chartomb = new char[10 * (Palyaszam + 2)];
    if (!chartomb) {
        hiba("memory");
    }

    for (int i = 1; i <= Palyaszam; i++) {
        Lgrtabla[i] = &chartomb[10 * (i - 1)];
        strcpy(Lgrtabla[i], "default");
    }

    // loaded with default values, open file:
    FILE* h = fopen("lgr/lgrlist.txt", "rt");
    if (!h) {
        return;
    }

    while (1) {
        // read one row
        if (!fgets(Sor, 40, h)) {
            break;
        }
        int levelszam = 0;
        if (sscanf(Sor, "%d%s", &levelszam, Szo) != 2) {
            continue;
        }
        if (levelszam < 1 || levelszam > Palyaszam) {
            continue;
        }
        strlwr(Szo);
        if (!strstr(Szo, ".lgr")) {
            continue;
        }
        *strstr(Szo, ".lgr") = 0;
        char* szo = Szo;
        // cut off spaces and tabs from front
        while ((*szo == ' ') || (*szo == 9)) {
            szo++;
        }
        if ((strlen(szo) > 8) || (strlen(szo) < 1)) {
            continue;
        }
        strcpy(Lgrtabla[levelszam], szo);
    }

    fclose(h);
}

// first lev is QWQUU001
topol::topol(char* filenev_p) {
    if (!Lgrtabla[1]) {
        makelgrtabla();
    }

    int kulso = 1;

    char filenev[21];
    if (strlen(filenev_p) > 20) {
        hiba("7y537yf");
    }
    strcpy(filenev, filenev_p);

    int belsopalyaszam = nevanalizis(filenev);
    if (belsopalyaszam > 0) {
        kulso = 0;
        strcpy(filenev, Levelnevek[belsopalyaszam]);
    }

    // from here on, internals and externals are read by two different routines
    // external reader is good, internal reader is a bit changed
    // and this how file is saved by topol in case of #define BELSOTIR

    // loadbelso reads number of 'kerek' objects before 'gyuru' objects
    // loadkulso reads 'gyuru' objects first

    if (kulso) {
        loadkulso(filenev);
    } else {
        loadbelso(filenev);
        // lgr names are read from personal table
        // #ifdef TEST 		no check because of demo
        //	if( belsopalyaszam < 0 || belsopalyaszam > Palyaszam )
        //		hiba( "4i4fyuif" );
        // #endif
        char tmp[40];
        sprintf(tmp, "lgr/%s.lgr", Lgrtabla[belsopalyaszam]);
        if (access(tmp, 0) == 0) { // only if file exists
            strcpy(lgrnev, Lgrtabla[belsopalyaszam]);
        }
    }
}

void topol::loadkulso(char* filenev) {
    ////int kulso = 1;

    nincslgrje = 0;
    kerekekfejjellefele = 0;
    hibas = 0;
    lezart = 0;

    idokhelye = 0;
    memset(&idok, 0, sizeof(idok));

    for (int i = 0; i < MAXGYURU; i++) {
        ptomb[i] = NULL;
    }
    for (int i = 0; i < MAXKEREK; i++) {
        kerektomb[i] = NULL;
    }
    for (int i = 0; i < MAXSPRITE; i++) {
        spritetomb[i] = NULL;
    }

    FILE* h = NULL;
    ////if( kulso ) {
    char tmpnev[40];
    sprintf(tmpnev, "lev/%s", filenev);
    h = fopen(tmpnev, "rb");
    if (!h) {
        hiba("cannot open file in topol::topol!: ", tmpnev);
    }
    /*////}
    else {
        h = qopen( filenev, "rb" );
        if( !h )
            hiba("cannot open file in topol::topol!: ", tmpnev);
    } */////

    // read Verzio:
    char tmp[10] = "AAAAA";
    if (fread(tmp, 1, 5, h) != 5) {
        hiba("Cannot read from txt file!");
    }
    if (strncmp(tmp, "POT", 3) != 0) {
        hiba("topol::topol( char* filenev ) start is wrong!");
    }

    belyeg = 0;
    double belyeg_d = 0;
    int verzio = tmp[4] - '0' + 10 * (tmp[3] - '0');

    if (verzio > 14) {
        uzenet("Level file's version is too new!: ", filenev);
    }

    // can only read these two verzio-s, but can be commented out to read all
    if (verzio != 6 && verzio != 14) {
        uzenet("Corrupt level file!: ", filenev);
    }

    // verzio 6: ASC 1.2 levels
    // verzio 7: new Verzio
    // verzio 8: new Verzio with 'koveto' and 'gyuru' objects ( maszk, texturanev)
    // verzio 10: also has time table at the end of file
    // verzio 11: has foodsorszam value in 'kerek'
    // verzio 12: new koveto (just the fact of it(?) is stored in gyuru object)
    // verzio 13: read 2 extra bytes before 'belyeg'
    // verzio 14: LEVELNAME (14) increased to (50), doesn't seek 100

    int dummy;
    if (verzio >= 13) {
        if (fread(&dummy, 1, 2, h) != 2) {
            hiba("Cannot read from lev file!");
        }
    }

    // Belyeg:
    if (fread(&belyeg, 1, 4, h) != 4) {
        hiba("Cannot read from lev file!");
    }

    if (fread(&belyeg_d, 1, sizeof(belyeg_d), h) != sizeof(double)) {
        hiba("Cannot read double belyeg from lev file!");
    }

    // read whether it is shareware
    double sarvarertek;
    if (fread(&sarvarertek, 1, sizeof(sarvarertek), h) != sizeof(sarvarertek)) {
        hiba("Cannot read from lev file!");
    }
    if (sarvarertek + belyeg_d < 9786.0 || sarvarertek + belyeg_d > 36546.0) {
        hiba("sarvarertek outside range in topol::topol!");
    }

    // Levels that can only be played in Registered would meet the condition: sarvarertek + belyeg_d
    // <= 20000.0

    // read whether it has error
    double hibaertek;
    if (fread(&hibaertek, 1, sizeof(hibaertek), h) != sizeof(hibaertek)) {
        hiba("Cannot read from lev file!");
    }
    if (hibaertek + belyeg_d < 9786.0 || hibaertek + belyeg_d > 36546.0) {
        hiba("hibaertek outside range in topol::topol!");
    }
    if (hibaertek + belyeg_d > 20000.0) {
        hibas = 1;
    }

    // read whether it is locked
    double lezartertek;
    if (fread(&lezartertek, 1, sizeof(lezartertek), h) != sizeof(lezartertek)) {
        hiba("Cannot read from lev file!");
    }
    if (lezartertek + belyeg_d < 9875.0 || lezartertek + belyeg_d > 32345.0) {
        hiba("lezartertek outside range in topol::topol!");
    }
    if (lezartertek + belyeg_d > 20000.0) {
        lezart = 1;
    }

    // lezart = 0; // if not commented out, unlocks every lev

    // read levname
    int tmpnamehossz = LEVELNAMEHOSSZ;
    if (verzio < 14) {
        tmpnamehossz = 14;
    }
    fread(levelname, 1, tmpnamehossz + 1, h);
    levelname[tmpnamehossz] = 0; // security measure

    // read lgr name
    if (verzio > 6) {
        fread(lgrnev, 1, 16, h);
        lgrnev[15] = 0; // security measure
    } else {
        strcpy(lgrnev, "default");
    }

    if (verzio >= 8) {
        fread(fgnev, 1, 10, h);
        lgrnev[10] = 0; // security measure
        fread(bgnev, 1, 10, h);
        lgrnev[10] = 0; // security measure
    } else {
        strcpy(fgnev, "ground");
        strcpy(bgnev, "sky");
    }

    // gyuru count starts after 100 bytes:
    if (verzio < 14) {
        ////if( kulso )
        fseek(h, 100, SEEK_SET);
    }
    ////else
    ////    qseek( h, 100, SEEK_SET );

    double gyuruolvszam = 0;
    if (fread(&gyuruolvszam, 1, sizeof(gyuruolvszam), h) != sizeof(gyuruolvszam)) {
        hiba("Cannot read gyuru count float from topol::topol!");
    }

    int gyuruszam = gyuruolvszam;
    if (gyuruszam > MAXGYURU) {
        hiba("gyuruszam > MAXGYURU in topol::topol!: ");
    }
    if (gyuruszam <= 0) {
        hiba("gyuruszam <= 0 in topol::topol!");
    }

    // read many 'gyuru' objects
    for (int i = 0; i < gyuruszam; i++) {
        ptomb[i] = new gyuru(h, verzio);
    }

    // number of 'kerek' objects
    double kerekolvszam = 0;
    if (fread(&kerekolvszam, 1, sizeof(kerekolvszam), h) != sizeof(kerekolvszam)) {
        hiba("topol::topol-ban nem tudta olvasni lebego kerekszamot!");
    }

    int kerekszam = kerekolvszam;
    if (kerekszam > MAXKEREK) {
        hiba("topol::topol-ban kerekszam > MAXKEREK!: ");
    }
    if (kerekszam <= 0) {
        hiba("topol::topol-ban kerekszam <= 0!");
    }

    for (int i = 0; i < kerekszam; i++) {
        // read many 'kerek'
        kerektomb[i] = new kerek(h, verzio);
    }

    if (verzio > 6) {
        // read sprites
        // number of 'kerek' objects:
        double spriteolvszam = 0;
        if (fread(&spriteolvszam, 1, sizeof(spriteolvszam), h) != sizeof(spriteolvszam)) {
            hiba("Cannot read sprite count float in topol::topol!");
        }

        int spriteszam = spriteolvszam;
        if (spriteszam > MAXSPRITE) {
            hiba("kerekszam > MAXSPRITE! in topol::topol!: ");
        }
        if (spriteszam < 0) {
            hiba("spriteszam < 0 in topol::topol!");
        }

        if (verzio < 8 && spriteszam > 0) {
            hiba("verzio < 8 && spriteszam > 0!", filenev);
        }

        for (int i = 0; i < spriteszam; i++) {
            // read many 'kerek'
            spritetomb[i] = new sprite(h);
        }
    }

    // only time table after this

    // also read time table in case of external file:
    // idokhelye and idok are 0 so far:
    ////if( kulso ) {
    idokhelye = ftell(h);
    if (idokhelye < 6) {
        hiba("dddhhhew");
    }

    int tmpszam = 0;
    if (fread(&tmpszam, 1, 4, h) != 4 || tmpszam != IDOEGYEDISZAM1) {
        // if( verzio >= 10 )
        //   uzenet( "Corrupt file:", filenev );
    } else {
        if (!titkosread(&idok, sizeof(idok), h) || fread(&tmpszam, 1, 4, h) != 4 ||
            tmpszam != IDOEGYEDISZAM2) {
            memset(&idok, 0, sizeof(idok));
            // uzenet( "Corrupt file:", filenev );
        }
    }
    ////}

    ////if( kulso )
    fclose(h);
    ////else
    ////    qclose( h );

    // check if 'belyeg' matches
    double szamitottbelyeg = belyegszamitas(verzio > 6);
    if (fabs(szamitottbelyeg - belyeg_d) > 0.01) {
        uzenet("Corrupt .LEV file!: ", filenev);
    }
}

// remake by modifying loadkulso if required
void topol::loadbelso(char* filenev) {
    nincslgrje = 0;
    kerekekfejjellefele = 0;
    hibas = 0;
    lezart = 0;

    idokhelye = 0;
    memset(&idok, 0, sizeof(idok));

    for (int i = 0; i < MAXGYURU; i++) {
        ptomb[i] = NULL;
    }
    for (int i = 0; i < MAXKEREK; i++) {
        kerektomb[i] = NULL;
    }
    for (int i = 0; i < MAXSPRITE; i++) {
        spritetomb[i] = NULL;
    }

    FILE* h = NULL;
    h = qopen(filenev, "rb");
    if (!h) {
        hiba("Cannot open file in topol::topol!: ", filenev);
    }

    // read Verzio
    char tmp[10] = "AAAAA";
    if (fread(tmp, 1, 5, h) != 5) {
        hiba("Cannot read from txt file!");
    }
    if (strncmp(tmp, "@@^!@", 3) != 0) {
        hiba("topol::topol( char* filenev ) start is wrong!");
    }

    belyeg = 0;
    double belyeg_d = 0;
    /*int verzio = tmp[4] - '0' + 10*(tmp[3] - '0');

    if( verzio < 6 )
        uzenet( "Corrupt level file!: ", filenev );

    if( verzio > 14 )
        uzenet( "Level file's version is too new!: ", filenev );
    */
    int verzio = 14; // leb files are always saved like this

    // verzio 6: ASC 1.2 levels
    // verzio 7: new Verzio
    // verzio 8: new Verzio with 'koveto' and 'gyuru' objects ( maszk, texturanev)
    // verzio 10: also has time table at the end of file
    // verzio 11: has foodsorszam value in 'kerek'
    // verzio 12: new koveto (just the fact of it(?) is stored in gyuru object)
    // verzio 13: read 2 extra bytes before 'belyeg'
    // verzio 14: LEVELNAME (14) increased to (50), doesn't seek 100

    int dummy;
    if (verzio >= 13) {
        if (fread(&dummy, 1, 2, h) != 2) {
            hiba("Cannot read from lev file!");
        }
    }

    // Belyeg:
    if (fread(&belyeg, 1, 4, h) != 4) {
        hiba("Cannot read from lev file!");
    }

    if (fread(&belyeg_d, 1, sizeof(belyeg_d), h) != sizeof(double)) {
        hiba("Cannot read double belyeg from lev file!");
    }

    // read whether it is shareware
    double sarvarertek;
    if (fread(&sarvarertek, 1, sizeof(sarvarertek), h) != sizeof(sarvarertek)) {
        hiba("Cannot read from lev file!");
    }
    if (sarvarertek + belyeg_d < 9786.0 || sarvarertek + belyeg_d > 36546.0) {
        hiba("sarvarertek outside of range in topol::topol!");
    }

    // read whether it has error
    double hibaertek;
    if (fread(&hibaertek, 1, sizeof(hibaertek), h) != sizeof(hibaertek)) {
        hiba("Cannot read from lev file!");
    }
    if (hibaertek + belyeg_d < 9786.0 || hibaertek + belyeg_d > 36546.0) {
        hiba("hibaertek outside of range in topol::topol!");
    }
    if (hibaertek + belyeg_d > 20000.0) {
        hibas = 1;
    }

    // read whether it is locked
    double lezartertek;
    if (fread(&lezartertek, 1, sizeof(lezartertek), h) != sizeof(lezartertek)) {
        hiba("Cannot read from lev file!");
    }
    if (lezartertek + belyeg_d < 9875.0 || lezartertek + belyeg_d > 32345.0) {
        hiba("lezartertek outside of range in topol::topol!");
    }
    if (lezartertek + belyeg_d > 20000.0) {
        lezart = 1;
    }

    // if( lezart ) {
    //  only for debug:
    //  dialog( "was locked" );
    //}

    // lezart = 0; // if not commented out, unlock every lev

    // read levname
    int tmpnamehossz = LEVELNAMEHOSSZ;
    if (verzio < 14) {
        tmpnamehossz = 14;
    }
    fread(levelname, 1, tmpnamehossz + 1, h);
    levelname[tmpnamehossz] = 0; // security measure

    // read lgr name
    if (verzio > 6) {
        fread(lgrnev, 1, 16, h);
        lgrnev[15] = 0; // security measure
    } else {
        strcpy(lgrnev, "default");
    }

    if (verzio >= 8) {
        fread(fgnev, 1, 10, h);
        lgrnev[10] = 0; // security measure
        fread(bgnev, 1, 10, h);
        lgrnev[10] = 0; // security measure
    } else {
        strcpy(fgnev, "ground");
        strcpy(bgnev, "sky");
    }

    // gyuru count starts after 100 bytes:
    if (verzio < 14) {
        qseek(h, 100, SEEK_SET);
    }
    double gyuruolvszam = 0;
    if (fread(&gyuruolvszam, 1, sizeof(gyuruolvszam), h) != sizeof(gyuruolvszam)) {
        hiba("Cannot read gyuru count float in topol::topol!");
    }

    int gyuruszam = gyuruolvszam;
    if (gyuruszam > MAXGYURU) {
        hiba("gyuruszam > MAXGYURU in topol::topol!: ");
    }
    if (gyuruszam <= 0) {
        hiba("gyuruszam <= 0 in topol::topol!");
    }

    // number of 'kerek' objects:
    double kerekolvszam = 0;
    if (fread(&kerekolvszam, 1, sizeof(kerekolvszam), h) != sizeof(kerekolvszam)) {
        hiba("Cannot read kerek count float in topol::topol!");
    }

    int kerekszam = kerekolvszam;
    if (kerekszam > MAXKEREK) {
        hiba("kerekszam > MAXKEREK in topol::topol!: ");
    }
    if (kerekszam <= 0) {
        hiba("kerekszam <= 0 in topol::topol!");
    }

    // read many 'gyuru' objects:
    for (int i = 0; i < gyuruszam; i++) {
        ptomb[i] = new gyuru(h, verzio);
    }

    // read many 'kerek' objects:
    for (int i = 0; i < kerekszam; i++) {
        kerektomb[i] = new kerek(h, verzio);
    }

    if (verzio > 6) {
        // read sprites
        // number of 'kerek' objects:
        double spriteolvszam = 0;
        if (fread(&spriteolvszam, 1, sizeof(spriteolvszam), h) != sizeof(spriteolvszam)) {
            hiba("Cannot read sprite count float in topol::topol!");
        }

        int spriteszam = spriteolvszam;
        if (spriteszam > MAXSPRITE) {
            hiba("kerekszam > MAXSPRITE in topol::topol!: ");
        }
        if (spriteszam < 0) {
            hiba("spriteszam < 0 in topol::topol!");
        }

        if (verzio < 8 && spriteszam > 0) {
            hiba("verzio < 8 && spriteszam > 0!", filenev);
        }

        for (int i = 0; i < spriteszam; i++) {
            // read many 'kerek' objects:
            spritetomb[i] = new sprite(h);
        }
    }

    // no time table in internal levs
    qclose(h);

    // check if 'belyeg' matches
    double szamitottbelyeg = belyegszamitas(verzio > 6);
    if (fabs(szamitottbelyeg - belyeg_d) > 0.01) {
        uzenet("Corrupt .LEV file!: ", filenev);
    }
}

static long longosit(double belyeg) {
    srand(clock());
    unsigned long veletlen = s_random(6542);
    veletlen *= s_random(7042);
    veletlen += s_random(4542);
    veletlen *= s_random(3042);
    veletlen *= s_random(3742);
    veletlen += s_random(9187);

    belyeg = sin(belyeg);
    belyeg *= (belyeg + 1.0001) * 40000;
    unsigned long l = belyeg;

    unsigned long belyeg_l = (l & 0x0000ffff) | (veletlen & 0xffff0000);
    return belyeg_l;
}

void topol::save(char* filenev, int nemkellcheck) {
    memset(&idok, 0, sizeof(idok));
    // if( lezart )
    //   hiba( "topol::save-ben lezart!" );

    Volttopsave = 1; // required by load.cpp
    if (kerekekfejjellefele) {
        hiba("topol::save kerekekfejjellefele!");
    }
    // this can still change polygons
    // (change of iteration, exclude overlapping points, translate acute angles)
    // so it is put in front
    if (nemkellcheck) {
        hibas = 0;
    } else {
        hibas = check_topology(0);
        if (hibas && lezart) {
            dialog("You cannot save this file as a locked file because there are",
                   "some topology errors in the design and you could not play on it!");
            return;
        }
    }

    char tmpnev[40];
    sprintf(tmpnev, "lev/%s", filenev);
#ifdef BELSOTIR
    if (!strstr(tmpnev, ".lev") && !strstr(tmpnev, ".LEV")) {
        hiba("78t4678r4");
    }
    if (strstr(tmpnev, ".lev")) {
        strcpy(strstr(tmpnev, ".lev"), ".leb");
    }
    if (strstr(tmpnev, ".LEV")) {
        strcpy(strstr(tmpnev, ".LEV"), ".leb");
    }
#endif
    FILE* h = fopen(tmpnev, "wb");
    if (!h) {
        hiba("Cannot open file in topol::save! :", tmpnev);
    }

#ifdef BELSOTIR
    fwrite("@@^!@", 1, 5, h);
#else
    fwrite("POT14", 1, 5, h);
#endif

    // calculate and display two types of 'belyeg'
    double belyeg_d = belyegszamitas(1);
    belyeg = longosit(belyeg_d);

    fwrite(&belyeg, 1, 2, h); // display two bytes

    fwrite(&belyeg, 1, 4, h);
    fwrite(&belyeg_d, 1, sizeof(belyeg_d), h);

    // display if shareware
    // Shareware levels would meet the criteria sarvarertek + double(belyeg_l) > 20000.0
    // which originally was calculated as 20961.0 + s_random( 4982 ) - belyeg_d;
    double sarvarertek = 11877.0 + s_random(5871) - belyeg_d;

// also display file from registered version as shareware
#ifdef BELSOTIR
    sarvarertek = 20961.0 + s_random(4982) - belyeg_d;
#endif

    fwrite(&sarvarertek, 1, sizeof(sarvarertek), h);

    // display if has error
    // if hibaertek + double(belyeg_l) > 20000.0, then has error:
    double hibaertek = 11877.0 + s_random(5871) - belyeg_d;
    if (hibas) { // already determined at the start
        hibaertek = 20961.0 + s_random(4982) - belyeg_d;
    }
    fwrite(&hibaertek, 1, sizeof(hibaertek), h);

    // display if locked
    double lezartertek = 12112.0 + s_random(6102) - belyeg_d;
    if (lezart) {
        lezartertek = 23090.0 + s_random(6310) - belyeg_d;
    }
    fwrite(&lezartertek, 1, sizeof(hibaertek), h);

// display levname
#ifdef BELSOTIR
    // delete levname if .leb
    for (int iiii = 0; iiii < LEVELNAMEHOSSZ; iiii++) {
        levelname[iiii] = 0;
    }
#endif
    fwrite(levelname, 1, LEVELNAMEHOSSZ + 1, h);

    // display levname
    fwrite(lgrnev, 1, 16, h);
    // display background and foreground name
    fwrite(fgnev, 1, 10, h);
    fwrite(bgnev, 1, 10, h);

    // count 'gyuru' and 'kerek' objects
    // fseek( h, 100, SEEK_SET ); no seek since Verzio 14

    int gyuruszam = 0;
    for (int i = 0; i < MAXGYURU; i++) {
        if (ptomb[i]) {
            gyuruszam++;
        }
    }
    double gyuruirtszam = gyuruszam + 0.4643643;

    int kerekszam = 0;
    for (int i = 0; i < MAXKEREK; i++) {
        if (kerektomb[i]) {
            kerekszam++;
        }
    }
    double kerekirtszam = kerekszam + 0.4643643;

    // save count of 'gyuru' objects after 100 bytes
    fwrite(&gyuruirtszam, 1, sizeof(gyuruirtszam), h);

// if saved as internal file, add count of 'kerek' objects here
#ifdef BELSOTIR
    fwrite(&kerekirtszam, 1, sizeof(kerekirtszam), h);
#endif

    // save many 'gyuru' objects
    for (int i = 0; i < MAXGYURU; i++) {
        if (ptomb[i]) {
            ptomb[i]->save(h, this);
        }
    }

// if saved as internal file, add count of 'kerek' objects here
#ifndef BELSOTIR
    fwrite(&kerekirtszam, 1, sizeof(kerekirtszam), h);
#endif

    // save many 'kerek' objects
    for (int i = 0; i < MAXKEREK; i++) {
        if (kerektomb[i]) {
            kerektomb[i]->save(h);
        }
    }
    // save sprites
    int spriteszam = 0;
    for (int i = 0; i < MAXSPRITE; i++) {
        if (spritetomb[i]) {
            spriteszam++;
        }
    }
    double spriteirtszam = spriteszam + 0.2345672;
    fwrite(&spriteirtszam, 1, sizeof(spriteirtszam), h);
    for (int i = 0; i < MAXSPRITE; i++) {
        if (spritetomb[i]) {
            spritetomb[i]->save(h);
        }
    }

    // display time table
    int tmpszo = IDOEGYEDISZAM1;
    fwrite(&tmpszo, 1, 4, h);
    titkoswrite(&idok, sizeof(idok), h);
    tmpszo = IDOEGYEDISZAM2;
    fwrite(&tmpszo, 1, 4, h);

    fclose(h);

    if (hibas) {
#ifdef BELSOTIR
        hiba("Wrong topol file saved with #define BELSOTIR:", filenev);
#endif
        dialog("Though the level file was successfully saved, there are some errors in the design.",
               "You cannot play on this level until you correct these problems. To see what the",
               "problems are, please push the Check Topology button in the editor.");
    }
}

// should only be called with originally inputted filenev:
void topol::saveidok(char* filenev_p) {
    if (idokhelye < 6) { // arbitrary value
        hiba("hjdvgyj");
    }

    // add lev directory to filenev
    char filenev[40];
    sprintf(filenev, "lev/%s", filenev_p);

    FILE* h = fopen(filenev, "r+b");
    if (!h) {
        uzenet("Could not open file!", filenev);
    }

    if (fseek(h, idokhelye, SEEK_SET) != 0) {
        uzenet("Could not write to file:", filenev);
    }
    int tmpszo = IDOEGYEDISZAM1;
    if (fwrite(&tmpszo, 1, 4, h) != 4) {
        uzenet("Could not write file:", filenev);
    }
    if (!titkoswrite(&idok, sizeof(idok), h)) {
        uzenet("Could not write file:", filenev);
    }
    tmpszo = IDOEGYEDISZAM2;
    if (fwrite(&tmpszo, 1, 4, h) != 4) {
        uzenet("Could not write file:", filenev);
    }

    fclose(h);
}

void topol::getminmax(double* px1, double* py1, double* px2, double* py2, int kerekkel) {
    *px1 = 100000000000.0;
    *py1 = 100000000000.0;
    *px2 = -100000000000.0;
    *py2 = -100000000000.0;
    for (int i = 0; i < MAXGYURU; i++) {
        if (ptomb[i]) {
            ptomb[i]->getminmax(px1, py1, px2, py2);
        }
    }
    if (kerekkel) {
        for (int i = 0; i < MAXKEREK; i++) {
            if (kerektomb[i]) {
                if (*px1 > kerektomb[i]->r.x) {
                    *px1 = kerektomb[i]->r.x;
                }
                if (*px2 < kerektomb[i]->r.x) {
                    *px2 = kerektomb[i]->r.x;
                }
                if (*py1 > kerektomb[i]->r.y) {
                    *py1 = kerektomb[i]->r.y;
                }
                if (*py2 < kerektomb[i]->r.y) {
                    *py2 = kerektomb[i]->r.y;
                }
            }
        }
        for (int i = 0; i < MAXSPRITE; i++) {
            if (spritetomb[i]) {
                if (*px1 > spritetomb[i]->r.x) {
                    *px1 = spritetomb[i]->r.x;
                }
                if (*px2 < spritetomb[i]->r.x) {
                    *px2 = spritetomb[i]->r.x;
                }
                if (*py1 > spritetomb[i]->r.y) {
                    *py1 = spritetomb[i]->r.y;
                }
                if (*py2 < spritetomb[i]->r.y) {
                    *py2 = spritetomb[i]->r.y;
                }
            }
        }
    }
}

double topol::belyegszamitas(int kellspriteis) {
    double belyeg = 0.0;
    for (int i = 0; i < MAXGYURU; i++) {
        if (ptomb[i]) {
            belyeg += ptomb[i]->belyegszamitas();
        }
    }
    for (int i = 0; i < MAXKEREK; i++) {
        if (kerektomb[i]) {
            belyeg += kerektomb[i]->belyegszamitas();
        }
    }
    if (kellspriteis) {
        for (int i = 0; i < MAXSPRITE; i++) {
            if (spritetomb[i]) {
                belyeg += spritetomb[i]->belyegszamitas();
            }
        }
    }

    return BELYEGSZORZO * belyeg;
}

// return 1 in case of any error
unsigned long getbelyeg(char* nev) {
    char tmp[30];
    sprintf(tmp, "lev/%s", nev);
    FILE* h = fopen(tmp, "rb");
    if (!h) {
        return 1;
    }
    unsigned long l = 0;
    if (fread(&l, 1, 1, h) != 1) {
        return 1;
    }
    if (fread(&l, 1, 4, h) != 4) {
        return 1;
    }
    if (fread(&l, 1, 4, h) != 4) {
        return 1;
    }
    fclose(h);
    return l;
}

// return true if name acquired
/*int getbelyegheznev( unsigned long belyeg, char* nev ) {
    find_t ffblk;
    int done = _dos_findfirst( "*.lev", _A_NORMAL, &ffblk );
    while( !done ) {
        if( getbelyeg( ffblk.name ) == belyeg ) {
            // filenev is acquired
            strcpy( nev, ffblk.name );
            return 1;
        }

        done = _dos_findnext( &ffblk );
    }
    return 0;
} */

// only for external files
// return NULL if has errors
/*char* getlevelname( char* nev ) {
    static char levelnamebuffer[LEVELNAMEHOSSZ+4] = "";

    char tmpnev[40];
    sprintf( tmpnev, "lev\\%s", nev );
    FILE* h = fopen( tmpnev, "rb" );
    if( !h )
        return NULL;

    if( fseek( h, 41, SEEK_CUR ) != 0 ) {
        fclose( h );
        return NULL;
    }

    if( fread( levelnamebuffer, 1, LEVELNAMEHOSSZ+1, h ) != LEVELNAMEHOSSZ+1 ) {
        fclose( h );
        return NULL;
    }
    levelnamebuffer[LEVELNAMEHOSSZ] = 0; // Ez csak biztonsagert

    fclose( h );
    return levelnamebuffer;
} */

/*unsigned long double2belyeg( double* px ) {
    double y = *px;
    if( y < 0 )
        y = 0 - y;
    y *= 1000.0;
    if( y > double(0x6fffffff) )
        hiba( "double2belyeg-ben y > 0x6fffffff!" );

    unsigned long l = y;
    return l;
}

unsigned long double2belyeg( long l ) {
    double x = l;
    return double2belyeg( &x );
} */

// initializations related to 'kerek'

// biker in flag tag is put back by lejatszo.cpp based on this
vekt2 Kezdetihelydiff;

// sets bike position, returns kajaszam (food number)
// activates 'kerek' objects which are food
int topol::setallaktiv(motorst* pmot) {
    int kajaszam = 0;
    int voltkezdo = 0;
    for (int i = 0; i < MAXKEREK; i++) {
        kerek* pker = Ptop->kerektomb[i];
        if (pker) {
            pker->sinfazisint = s_random(1000);
            pker->sinfazis = pker->sinfazisint * 2.0 * K_pi / 1000.0;

            pker->aktiv = 1;
            if (pker->tipus == T_KAJA) {
                kajaszam++;
            }
            if (pker->tipus == T_KEZDO) {
                if (voltkezdo) {
                    hiba("Lev has two starts in topol::setallaktiv!");
                }
                voltkezdo = 1;
                pker->aktiv = 0;
                Kezdetihelydiff = pker->r - pmot->kor2.r;
                pmot->kor1.r = pmot->kor1.r + Kezdetihelydiff;
                pmot->kor2.r = pmot->kor2.r + Kezdetihelydiff;
                pmot->kor4.r = pmot->kor4.r + Kezdetihelydiff;
                pmot->vezetor = pmot->vezetor + Kezdetihelydiff;
                // Fejr = Kor1.r;
            }
        }
    }
    if (!voltkezdo) {
        hiba("No start in topol::setallaktiv!");
    }
    return kajaszam;
}

// sorts 'kerek' objects
void topol::killerekelore(void) {
    int szam = 0;
    for (int i = 0; i < MAXKEREK; i++) {
        kerek* pker = Ptop->kerektomb[i];
        if (pker) {
            szam++;
        }
    }
    for (int i = 0; i < szam; i++) {
        kerek* pker = Ptop->kerektomb[i];
        if (!pker) {
            hiba("Missing array elements in topol::killerekelore!");
        }
    }

    if (szam < 2) {
        hiba("szam < 2 in topol::killerekelore!");
    }
    for (int j = 0; j < szam + 4; j++) {
        for (int i = 0; i < szam - 1; i++) {
            int tipus1 = Ptop->kerektomb[i]->tipus;
            int tipus2 = Ptop->kerektomb[i + 1]->tipus;

            // types are weighted to make viable for sorting
            int tip1 = 10;
            if (tipus1 == T_HALALOS) {
                tip1 = 1;
            }
            if (tipus1 == T_KAJA) {
                tip1 = 2;
            }
            if (tipus1 == T_CEL) {
                tip1 = 3;
            }

            int tip2 = 10;
            if (tipus2 == T_HALALOS) {
                tip2 = 1;
            }
            if (tipus2 == T_KAJA) {
                tip2 = 2;
            }
            if (tipus2 == T_CEL) {
                tip2 = 3;
            }

            if (tip1 > tip2) {
                // switch needed
                kerek tmpker = *Ptop->kerektomb[i];
                *Ptop->kerektomb[i] = *Ptop->kerektomb[i + 1];
                *Ptop->kerektomb[i + 1] = tmpker;
            }
        }
    }
}

// sets color of all 'kerek' objects
void topol::kitoltiszineket(void) {
    for (int i = 0; i < MAXKEREK; i++) {
        kerek* pker = Ptop->kerektomb[i];
        if (pker) {
            if (pker->tipus == T_CEL) {
                pker->szin = 0;
            }
            if (pker->tipus == T_KAJA) {
                pker->szin = 0;
            }
            if (pker->tipus == T_HALALOS) {
                pker->szin = 0;
            }
        }
    }
}

kerek* topol::getptrkerek(int index) {
    if (index < 0 || index >= MAXKEREK) {
        hiba("index < 0 || index >= MAXKEREK! in topol::getptrkerek!");
    }
    kerek* pker = Ptop->kerektomb[index];
    if (!pker) {
        hiba("!pker in topol::getptrkerek!");
    }
    return pker;
}

// kerekekfejjellefele;

void topol::kereklefejjel(void) {
    if (kerekekfejjellefele) {
        return;
    }
    kerekekfejjellefele = 1;
    for (int i = 0; i < MAXKEREK; i++) {
        kerek* pker = Ptop->kerektomb[i];
        if (pker) {
            pker->r.y = -pker->r.y;
        }
    }
}

void topol::kerekjolalljon(void) {
    if (!kerekekfejjellefele) {
        return;
    }
    kerekekfejjellefele = 0;
    for (int i = 0; i < MAXKEREK; i++) {
        kerek* pker = Ptop->kerektomb[i];
        if (pker) {
            pker->r.y = -pker->r.y;
        }
    }
}
