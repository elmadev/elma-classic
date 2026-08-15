#ifndef EOL_KUSKI_H
#define EOL_KUSKI_H

#include "game/driver.h"
#include "level/level.h"
#include "main.h"
#include "physics/init.h"
#include <cstdint>
#include <deque>
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
    void add(const struct spy_data& sd, int min_spy_frames);
    // Queue a stop marker: buffered motion plays out, then the kuski hides.
    void stop();
    // Drop buffered frames and hide the kuski.
    void clear();
    void update(uint32_t now_ms, int min_spy_frames);

  private:
    struct spy_frame {
        struct spy_data data;
        bool stop = false;
    };

    std::deque<spy_playback::spy_frame>::iterator frame_after_time(uint32_t time, uint32_t offset);
    // Reset buffering without hiding the current pose.
    void rebuffer();
    std::deque<spy_frame> frames;
    bool running = false;
    uint32_t time_offset = 0;
    // The pose on screen; update() refreshes it from frames.
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
    void add_spy_data(const struct spy_data& sd, int min_spy_frames) {
        spy.add(sd, min_spy_frames);
    }
    void stop_spy_data() { spy.stop(); }
    void clear_spy_data() { spy.clear(); }
    void update_spy_data(uint32_t now_ms, int min_spy_frames) {
        spy.update(now_ms, min_spy_frames);
    }

  private:
    spy_playback spy;
};

#endif
