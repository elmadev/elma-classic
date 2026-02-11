#ifndef EOL_EVENTS_H
#define EOL_EVENTS_H

#include "eol/eol_types.h"

struct login {
    bool success;
    unsigned int id;
    unsigned int id2;
};

struct new_kuski {
    kuski k;
};

#endif
