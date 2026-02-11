#ifndef LOAD_H
#define LOAD_H

extern int ReloadLevel;

void invalidate_level();

// Beolvas egy level-t ha kell.
// Torli es kitolti Ptop, Lgr, Segments, ecsetek mutatokat:
// Ha belso file-ban is megvan palya onnan veszi:
// Hamissal ter vissza, ha hibas topologiaju level:
int load_level_play(const char* levelname);

// Beolvas egy level-t ha kell.
// Torli es kitolti Ptop, Lgr, mutatokat:
// Segments es ecseteket torli:
// Ha belso file-ban is megvan palya onnan veszi:
// Igaz ha file nem volt lezarva, kulonben new palya hamissal:
int load_level_editor(const char* levelname, int reload = 0);
// Ha nevnek _uj_topol_-t adunk meg, ujat csinal

// Opciok minibike-ja hasznalja:
void invalidate_ptop(void);

void dialog_warn_lgr_assets_deleted(void);

#endif
