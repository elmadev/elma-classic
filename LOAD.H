#ifndef LOAD_H
#define LOAD_H

extern int ReloadLevel;

void invalidate_level();

int load_level_play(const char* levelname);

int load_level_editor(const char* levelname, int reload = 0);

void invalidate_ptop(void);

void dialog_warn_lgr_assets_deleted(void);

#endif
