
// Jatek modja ket fele lehet:
extern int Single;
extern int Tag; // Van-e fogocska
// extern int Coop;
// extern int Match;

extern int Legkozment; // kiraj320 ebbol tudja, hogy snapshot-ot keszitsen

// Azt a nevet kerik, amit topol::topol kap:
// Ezenlegjobbido beallitasahoz kell nekik:
long lejatszo(char* filenev);
long lejatszo_r(char* filenev, int showkepmarad);

struct baljobbvaltozok {
    int eddighatra;
    double ucsoforgas;
    double ucsoford;
    double baljobb; // kulsoresz szamolja tobbibol
    double forgas;
};

struct valtozok {
    hanginfo inf;
    double surlero;
    double frekvencia;
    double utolsougras;
    int ugras1volt;

    int showkep;

    int hatranyomva;
    int viewnyomva;
    int timenyomva;
    int showkepnyomva; // multiplayer-ben el lehet tuntetni

    double ugrasnagysag;

    baljobbvaltozok baljobbv_f, baljobbv_h;
};

// lejatszo ezt beallitja: 0 -> nem kell kiirni, 1 -> A eloszor, 2 -> B
// csak akkor A vagy B eloszor, ha mindketten meghaltak, tehat
// afterplay menu-nek ezt mar feldolgozta elore:
extern int MeghalteloszorAB;

// Ha sikeres szint, ide teszi be hogy ki ment ki:
extern int Aerintetteviragot;
