#ifndef EOL_CLICK_H
#define EOL_CLICK_H

#include "vect2.h"

// A clickable item
struct clickable {
    static constexpr int DEFAULT_RADIUS = 10;

    enum class Mode {
        Normal,
        CheckpointEndHeld,
    };
    static inline Mode ClickMode = Mode::Normal;

    virtual void left_clicked(int x, int y, vect2 coord) = 0;
    virtual void right_clicked(int x, int y, vect2 coord) = 0;
};

struct overlay_clickable : clickable {
    int click_anchor_x;                           // pixels
    int click_anchor_y;                           // pixels
    int click_radius = clickable::DEFAULT_RADIUS; // pixels

    virtual int distance(int mou_x, int mou_y) const;

    overlay_clickable(int anchor_x, int anchor_y)
        : click_anchor_x(anchor_x),
          click_anchor_y(anchor_y) {}
};

struct game_clickable : clickable {
    vect2 click_anchor;                           // meters
    int click_radius = clickable::DEFAULT_RADIUS; // pixels

    virtual int distance(vect2 coord) const;

    game_clickable(vect2 anchor)
        : click_anchor(anchor) {}
};

#endif
