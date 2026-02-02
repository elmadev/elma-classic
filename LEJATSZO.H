#ifndef LEJATSZO_H
#define LEJATSZO_H

#include "recorder.h"

// Jatek modja ket fele lehet:
extern int Single;
extern int Tag; // Van-e fogocska
// extern int Coop;

extern int Legkozment; // kiraj320 ebbol tudja, hogy snapshot-ot keszitsen

enum class CameraMode { Normal, MapViewer };

struct camera {
    CameraMode mode;
    double x;
    double y;
    double min_x;
    double min_y;
    double max_x;
    double max_y;
};

// Azt a nevet kerik, amit topol::topol kap:
// BestTime beallitasahoz kell nekik:
long lejatszo(const char* filenev, CameraMode cameramode);
long lejatszo_r(const char* filenev, int showkepmarad);

struct baljobbvaltozok {
    int eddighatra;
    double ucsoforgas;
    double ucsoford;
    double baljobb; // kulsoresz szamolja tobbibol
    double forgas;
};

struct valtozok {
    bike_sound inf;
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
// menu_level menu-nek ezt mar feldolgozta elore:
extern int MeghalteloszorAB;

// Ha sikeres szint, ide teszi be hogy ki ment ki:
extern int Aerintetteviragot;

#endif
