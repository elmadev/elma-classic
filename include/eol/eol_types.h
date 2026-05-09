#ifndef EOL_TYPES_H
#define EOL_TYPES_H

#include "LEJATSZO.H"
#include "fs_utils.h"
#include "physics_init.h"
#include <cstdint>
#include <optional>

class pic8;

struct spy_data {
    unsigned int kuski_id;
    uint32_t time;
    motorst mot;
    bike_metadata metadata;
};

struct kuski {
    unsigned int id;
    char nick[16];
    char level[MAX_FILENAME_LEN + 1];
    bool is_player = true;
    bool is_online = true;
    pic8* shirt;
    const struct spy_data* spy_data() const;
    void add_spy_data(const struct spy_data& sd);
    void clear_spy_data();

  private:
    std::optional<struct spy_data> data;
};

enum class TableType { None, PlayersOnline };

enum class BattleType : uint8_t {
    Normal = 0,
    OneLife,
    FirstFinish,
    Slowness,
    Survivor,
    LastCounts,
    FinishCount,
    HourTT,
    FlagTag,
    AppleCollect,
    Speed,
};

namespace BattleAttributes {
enum Kind : uint16_t {
    SeeOthers = 1 << 0,
    SeeTimes = 1 << 1,
    AllowStarter = 1 << 2,
    AcceptBugs = 1 << 3,
    NoVolt = 1 << 4,
    NoTurn = 1 << 5,
    OneTurn = 1 << 6,
    NoBrake = 1 << 7,
    NoThrottle = 1 << 8,
    AlwaysThrottle = 1 << 9,
    Drunk = 1 << 10,
    Uploaded = 1 << 11,
    OneWheel = 1 << 12,
    Multi = 1 << 13,
};
}

struct battle {
    char level_filename[MAX_FILENAME_LEN + 1];
    BattleType type;
    uint8_t duration;
    BattleAttributes::Kind attributes;
    unsigned int designer_id;
    uint8_t countdown_seconds;
    uint32_t level_apple_count;
    bool in_countdown;
    // Local-clock timestamp at which the battle starts (after countdown, if any).
    long long local_start_ms;
};

#endif
