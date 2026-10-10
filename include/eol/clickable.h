#ifndef EOL_CLICKABLE_H
#define EOL_CLICKABLE_H

#include "vect2.h"
#include <functional>
#include <optional>

struct game_mouse {
    // Mouse position in pixels
    int x;
    int y;
    // Mouse position in meters
    std::optional<vect2> coord;
};

struct mouse_input {
    bool left_click = false;
    bool right_click = false;
};

// A clickable item
struct clickable {
    static constexpr int DEFAULT_RADIUS = 10;

    using clickable_callback = std::function<void(const game_mouse& pos, const mouse_input& input)>;
    static inline std::optional<clickable_callback> callback;
    static inline void set_callback(clickable_callback new_callback, const game_mouse& pos) {
        callback = new_callback;
        (*callback)(pos, {}); // Immediately run the callback once with no inputs
    };
    static inline void reset_callback() { callback.reset(); }

    virtual void left_clicked(const game_mouse& pos) = 0;
    virtual void right_clicked(const game_mouse& pos) = 0;
    virtual int distance(const game_mouse& pos) const = 0;

    virtual ~clickable() = default;
};

struct overlay_clickable : clickable {
    int click_anchor_x;                           // pixels
    int click_anchor_y;                           // pixels
    int click_radius = clickable::DEFAULT_RADIUS; // pixels

    int distance(const game_mouse& pos) const override;

    overlay_clickable(int anchor_x, int anchor_y)
        : click_anchor_x(anchor_x),
          click_anchor_y(anchor_y) {}
};

struct game_clickable : clickable {
    vect2 click_anchor;                           // meters
    int click_radius = clickable::DEFAULT_RADIUS; // pixels

    int distance(const game_mouse& pos) const override;

    game_clickable(vect2 anchor)
        : click_anchor(anchor) {}
};

#endif
