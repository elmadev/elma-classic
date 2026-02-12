#ifndef EOL_TYPES_H
#define EOL_TYPES_H

#include "LEJATSZO.H"
#include "fs_utils.h"
#include "physics_init.h"
#include <cstdint>
#include <optional>

struct spy_data {
    unsigned int kuski_id;
    uint32_t time;
    motorst mot;
    valtozok metadata;
};

struct kuski {
    unsigned int id;
    char nick[16];
    char level[MAX_FILENAME_LEN + 1];
    const struct spy_data* spy_data() const;
    void add_spy_data(const struct spy_data& sd);
    void clear_spy_data();

  private:
    std::optional<struct spy_data> data;
};

enum class TableType { None, PlayersOnline };

#endif
