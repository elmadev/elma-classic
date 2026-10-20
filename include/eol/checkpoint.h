#ifndef EOL_CHECKPOINT_H
#define EOL_CHECKPOINT_H

#include "eol/clickable.h"
#include "vect2.h"

class checkpoint;
class pic8;

struct endpoint : game_clickable {
  private:
    static constexpr double MINIMUM_LENGTH = 0.2;

    checkpoint* parent;
    endpoint* other;

  public:
    void left_clicked(const game_mouse& pos) override;
    void middle_clicked(const game_mouse& pos) override;
    void right_clicked(const game_mouse& pos) override;

    // Set position while also enforcing the minimum line length
    void set_anchor(vect2 coord);

    static void held_callback(const game_mouse& pos, const mouse_input& input);

    endpoint(checkpoint* parent, endpoint* other, vect2 coord)
        : game_clickable(coord),
          parent(parent),
          other(other) {}
};

class checkpoint : clickable {
    endpoint start;
    endpoint end;

    enum class Polarity { Both, Positive, Negative };
    Polarity polarity;

  public:
    void left_clicked(const game_mouse& pos) override;
    void middle_clicked(const game_mouse& pos) override;
    void right_clicked(const game_mouse& pos) override;
    int distance(const game_mouse& pos) const override;
    void render(pic8& screen, vect2 corner) const;

    static inline bool Editor = false;
    static inline bool Render = false;

    checkpoint(vect2 coord)
        : start(this, &end, coord),
          end(this, &start, coord),
          polarity(Polarity::Both) {}

    // Remove a checkpoint from the checkpoint list
    static void remove(checkpoint* target);

    static void held_callback(const game_mouse& pos, const mouse_input& input);
    static void editor_click(const game_mouse& pos, const mouse_input& input);
    static void render_all(pic8& screen, vect2 corner);
    static void get_closest(const game_mouse& pos, int& dist, clickable*& closest);
};

#endif
