#ifndef EOL_TYPES_H
#define EOL_TYPES_H

#include "level/level.h"
#include "main.h"
#include <cstdint>
#include <optional>

enum class TableType { None, PlayersOnline, BattleResults, BattleQueue, FinishedTimes };

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
    Apple,
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
    CrippleMask =
        NoVolt | NoTurn | OneTurn | NoBrake | NoThrottle | AlwaysThrottle | Drunk | OneWheel,
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
    bool level_exists = false;
    bool download_requested = false;
    std::optional<unsigned int> flag_owner_id;
};

struct apple_battle_progress {
    bool taken[MAX_OBJECTS]{};
    void clear();
    void record(int object_index);
    // Deactivate the already-taken apples in the level.
    void apply(const level& lev) const;
};

#endif
