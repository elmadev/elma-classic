#ifndef EOL_CHECKPOINT_H
#define EOL_CHECKPOINT_H

#include "eol/clickable.h"
#include "vect2.h"
#include <list>
#include <memory>

struct checkpoint;
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

struct checkpoint : clickable {
    endpoint start;
    endpoint end;

    enum class Polarity { Both, Positive, Negative };
    Polarity polarity;

    void left_clicked(const game_mouse& pos) override;
    void middle_clicked(const game_mouse& pos) override;
    void right_clicked(const game_mouse& pos) override;
    int distance(const game_mouse& pos) const override;
    void render(pic8& screen, vect2 corner) const;

    checkpoint(vect2 coord)
        : start(this, &end, coord),
          end(this, &start, coord),
          polarity(Polarity::Both) {}

    static void held_callback(const game_mouse& pos, const mouse_input& input);
};

struct checkpoints {
    std::list<checkpoint> linear;

    static inline bool Editor = false;
    static inline bool Render = false;

    // Remove a checkpoint from the checkpoint list
    void remove(checkpoint* target);

    void editor_click(const game_mouse& pos, const mouse_input& input);
    void render_all(pic8& screen, vect2 corner);
    void get_closest(const game_mouse& pos, int& dist, clickable*& closest);
};

extern std::unique_ptr<checkpoints> Checkpoints;

#endif
