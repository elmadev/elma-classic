#ifndef EOL_CHECKPOINT_H
#define EOL_CHECKPOINT_H

#include "eol/clickable.h"
#include "vect2.h"

class pic8;

class checkpoint : clickable {
    static constexpr double MINIMUM_LENGTH = 0.2;

    struct endpoint : game_clickable {
        checkpoint* parent;
        endpoint* other;

        void left_clicked(const game_mouse& pos) override;
        void right_clicked(const game_mouse& pos) override;

        // Set position while also enforcing the minimum line length
        void set_anchor(vect2 coord);

        static void held_callback(const game_mouse& pos, const mouse_input& input);

        endpoint(checkpoint* parent, endpoint* other, vect2 coord)
            : game_clickable(coord),
              parent(parent),
              other(other) {}
    };

    endpoint start;
    endpoint end;

    void left_clicked(const game_mouse& pos) override;
    void right_clicked(const game_mouse& pos) override;
    int distance(const game_mouse& pos) const override;
    void render(pic8& screen, vect2 corner) const;

    static inline endpoint* held_end = nullptr;
    static inline checkpoint* held_line = nullptr;

  public:
    static inline bool Editor = false;
    static inline bool Render = false;

    checkpoint(vect2 coord)
        : start(this, &end, coord),
          end(this, &start, coord + vect2{MINIMUM_LENGTH, 0.0}) {}

    // Remove a checkpoint from the checkpoint list
    static void remove(checkpoint* target);

    static void held_callback(const game_mouse& pos, const mouse_input& input);
    static void editor_click(const game_mouse& pos, const mouse_input& input);
    static void render_all(pic8& screen, vect2 corner);
    static void get_closest(const game_mouse& pos, int& dist, clickable*& closest);
};

#endif
