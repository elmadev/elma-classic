#ifndef EOL_CLICKABLE_H
#define EOL_CLICKABLE_H

#include "box.h"
#include "vect2.h"
#include <functional>
#include <optional>

class pic8;

struct game_mouse {
    // Mouse position in pixels
    int x;
    int y;
    // Mouse position in meters
    std::optional<vect2> coord;
};

struct mouse_input {
    bool left_click = false;
    bool middle_click = false;
    bool right_click = false;
    bool abort = false;
};

// A clickable item
struct clickable {
    static constexpr int DEFAULT_RADIUS = 10;

    using clickable_callback = std::function<void(const game_mouse& pos, const mouse_input& input)>;
    static inline clickable_callback callback;
    static void set_callback(const clickable_callback& new_callback, const game_mouse& pos) {
        callback = new_callback;
        callback(pos, {}); // Immediately run the callback once with no inputs
    };
    static void reset_callback() { callback = {}; }

    virtual void left_clicked(const game_mouse& pos) = 0;
    virtual void middle_clicked(const game_mouse& pos) = 0;
    virtual void right_clicked(const game_mouse& pos) = 0;
    virtual int distance(const game_mouse& pos) const = 0;

    virtual ~clickable() = default;
};

struct game_clickable : clickable {
    vect2 click_anchor;                           // meters
    int click_radius = clickable::DEFAULT_RADIUS; // pixels

    int distance(const game_mouse& pos) const override;

    game_clickable(vect2 anchor)
        : click_anchor(anchor) {}
};

struct box_overlay_clickable : clickable {
  private:
    static constexpr int EDGE = clickable::DEFAULT_RADIUS / 2;

    bool horizontal_stretch;
    bool vertical_stretch;

    void grab_box(const game_mouse& pos) const;

  public:
    virtual box click_box() const = 0;
    virtual void set_box(const box& new_box) = 0;
    virtual void reset_box() = 0;

    int distance(const game_mouse& pos) const override;
    void get_closest(const game_mouse& pos, int& dist, clickable*& closest);

    void left_clicked(const game_mouse& pos) override;
    void right_clicked(const game_mouse& pos) override;

    static void held_callback(const game_mouse& pos, const mouse_input& input);

    void render_outline(pic8& screen) const;

    box_overlay_clickable(bool horizontal, bool vertical)
        : horizontal_stretch(horizontal),
          vertical_stretch(vertical) {}
};

#endif
