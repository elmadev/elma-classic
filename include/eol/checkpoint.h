#ifndef EOL_CHECKPOINT_H
#define EOL_CHECKPOINT_H

#include "eol/clickable.h"
#include "vect2.h"

class pic8;

class checkpoint {
    static constexpr double MINIMUM_LENGTH = 0.2;

    struct endpoint : game_clickable {
        endpoint* other;

        void left_clicked(int x, int y, vect2 coord) override;
        void right_clicked(int x, int y, vect2 coord) override;

        // Set position while also enforcing the minimum line length
        void set_anchor(vect2 coord);

        endpoint(endpoint* other, vect2 coord)
            : game_clickable(coord),
              other(other) {}
    };

    endpoint start;
    endpoint end;

    void render(pic8& screen, vect2 corner) const;

    static inline endpoint* held_end = nullptr;

  public:
    checkpoint(vect2 coord)
        : start(&end, coord),
          end(&start, coord + vect2{MINIMUM_LENGTH, 0.0}) {}

    static void editor_update(vect2 coord, bool left_click, bool right_click);
    static void render_all(pic8& screen, vect2 corner);
    static void get_closest(vect2 coord, int& dist, clickable*& closest);
};

#endif
