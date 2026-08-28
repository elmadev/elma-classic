#ifndef EOL_CHECKPOINT_H
#define EOL_CHECKPOINT_H

#include "vect2.h"

class pic8;

class checkpoint {
    struct endpoint {
        vect2 click_anchor;

        endpoint(vect2 coord)
            : click_anchor(coord) {}
    };

    endpoint start;
    endpoint end;

    void render(pic8& screen, vect2 corner) const;

    static inline endpoint* held = nullptr;

  public:
    checkpoint(vect2 coord)
        : start(coord),
          end(coord) {}

    static void editor_update(vect2 coord, bool left_click, bool right_click);
    static void render_all(pic8& screen, vect2 corner);
};

#endif
