#ifndef LOAD_H
#define LOAD_H

#define DEFAULT_LEVEL_FILENAME "_uj_topol_"

void invalidate_level();

bool load_level_play(const char* levelname);

bool load_level_editor(const char* levelname);

void dialog_warn_lgr_assets_deleted();

#endif
