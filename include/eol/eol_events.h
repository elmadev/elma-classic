#ifndef EOL_EVENTS_H
#define EOL_EVENTS_H

#include "eol/eol_types.h"
#include "fs_utils.h"

#include <cstdint>
#include <span>
#include <string_view>
#include <vector>

class level;

constexpr int MAX_MESSAGE_LEN = 65;

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

enum class DownloadResult {
    Success,
    Fail,
    NotFound,
};

struct level_download_request {
    char level[MAX_FILENAME_LEN + 1];
};

struct level_download {
    char level[MAX_FILENAME_LEN + 1];
    std::span<const uint8_t> data;
    DownloadResult result;
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

struct clear_spy_data {
    unsigned int kuski_id;
};

struct show_table {
    TableType table;
};

struct chat_message {
    unsigned int kuski_id;
    uint64_t unix_timestamp;
    char message[MAX_MESSAGE_LEN + 1];
};

struct send_chat {
    unsigned int kuski_id;
    std::string_view message;
};

struct private_message {
    unsigned int from_kuski_id;
    unsigned int to_kuski_id;
    uint64_t unix_timestamp;
    char message[MAX_MESSAGE_LEN + 1];
};

struct team_message {
    unsigned int from_kuski_id;
    uint64_t unix_timestamp;
    char message[MAX_MESSAGE_LEN + 1];
};

struct battle_started {
    battle bat;
};

struct battle_countdown_ended {};

struct battle_ended {
    bool aborted;
};

struct battle_time_sync {
    long long local_start_ms;
};

struct battle_line_update {
    unsigned int kuski_id;
    unsigned int kuski_id2;
    uint32_t score; // Best time, speed, or finish count depending on battle type
    uint16_t apple_count;
    uint16_t rank;
};

struct battle_queue_entry {
    unsigned int designer_id;
    BattleType battle_type;
    uint8_t duration_minutes;
};

struct battle_queue_update {
    std::vector<battle_queue_entry> entries;
};

#endif
