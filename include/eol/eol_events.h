#ifndef EOL_EVENTS_H
#define EOL_EVENTS_H

#include "eol/eol_types.h"
#include "fs_utils.h"

class level;

struct login {
    bool success;
    unsigned int id;
    unsigned int id2;
};

struct new_kuski {
    kuski k;
};

struct kuski_logout {
    unsigned int id;
    unsigned int id2;
};

struct kuski_set_level {
    unsigned int id;
    char level[MAX_FILENAME_LEN + 1];
};

struct enter_level {
    const level* lev;
    const char* name;
};

struct exit_level {
    const char* name;
    double time;
    int apple_count;
    int level_apple_count;
    bool dead;
};

#endif
