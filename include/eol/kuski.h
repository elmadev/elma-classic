#ifndef EOL_KUSKI_H
#define EOL_KUSKI_H

#include "game/driver.h"
#include "level/level.h"
#include "main.h"
#include "physics/init.h"
#include <cstdint>
#include <optional>

class pic8;

struct spy_data {
    unsigned int kuski_id;
    uint8_t run_id;
    uint32_t time;
    motorst mot;
    bike_metadata metadata;
};

class spy_playback {
  public:
    const struct spy_data* spy_data() const;
    void add(const struct spy_data& sd);
    void clear();

  private:
    std::optional<struct spy_data> data;
};

class kuski {
  public:
    unsigned int id;
    char nick[16];
    char level[MAX_FILENAME_LEN + 1];
    bool is_player = true;
    bool is_online = true;
    pic8* shirt;
    bool apples_taken[MAX_OBJECTS];
    void clear_apple_data();
    const struct spy_data* spy_data() const { return spy.spy_data(); }
    void add_spy_data(const struct spy_data& sd) { spy.add(sd); }
    void clear_spy_data() { spy.clear(); }

  private:
    spy_playback spy;
};

#endif
