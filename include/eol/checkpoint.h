#ifndef EOL_CHECKPOINT_H
#define EOL_CHECKPOINT_H

#include "eol/clickable.h"
#include "vect2.h"

class pic8;

class checkpoint {
    struct endpoint : game_clickable {
        void left_clicked(int x, int y, vect2 coord) override;
        void right_clicked(int x, int y, vect2 coord) override;

        endpoint(vect2 coord)
            : game_clickable(coord) {}
    };

    endpoint start;
    endpoint end;

    void render(pic8& screen, vect2 corner) const;

    static inline endpoint* held_end = nullptr;

  public:
    checkpoint(vect2 coord)
        : start(coord),
          end(coord) {}

    static void editor_update(vect2 coord, bool left_click, bool right_click);
    static void render_all(pic8& screen, vect2 corner);
    static void get_closest(vect2 coord, int& dist, clickable*& closest);
};

#endif
