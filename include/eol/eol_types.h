#ifndef EOL_TYPES_H
#define EOL_TYPES_H

#include "fs_utils.h"

struct kuski {
    unsigned int id;
    char nick[16];
    char level[MAX_FILENAME_LEN + 1];
};

#endif
