#ifndef LEVEL_LOAD_H
#define LEVEL_LOAD_H

#define DEFAULT_LEVEL_FILENAME "_uj_topol_"

void invalidate_level();

// Filename of the currently loaded level, or "".
const char* current_level_filename();

enum class LoadLevelResult { Success, Fail, Abort };
bool load_level_play(const char* levelname);

bool load_level_editor(const char* levelname);

#endif
