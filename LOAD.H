
extern int Volttopsave;

// Beolvas egy level-t ha kell.
// Torli es kitolti Ptop, Plgr, Pszak, ecsetek mutatokat:
// Ha belso file-ban is megvan palya onnan veszi:
// Hamissal ter vissza, ha hibas topologiaju level:
int floadlevel_p(char* nev);

// Beolvas egy level-t ha kell.
// Torli es kitolti Ptop, Plgr, mutatokat:
// Pszak es ecseteket torli:
// Ha belso file-ban is megvan palya onnan veszi:
// Igaz ha file nem volt lezarva, kulonben new palya hamissal:
int floadlevel_e(char* nev, int forceload = 0);
// Ha nevnek _uj_topol_-t adunk meg, ujat csinal

// Opciok minibike-ja hasznalja:
void invalidate_ptop(void);

void dialogselejtez(void);
