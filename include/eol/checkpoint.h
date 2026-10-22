#ifndef EOL_CHECKPOINT_H
#define EOL_CHECKPOINT_H

#include "eol/clickable.h"
#include "vect2.h"
#include <list>
#include <memory>
#include <optional>
#include <string>

struct checkpoint;
class level;
struct motorst;
class pic8;

struct checkpoint_state {
    bool active;
    std::optional<int> latest_centiseconds;
    std::optional<int> diff_centiseconds;
    std::optional<int> best_centiseconds;
    std::optional<double> latest_speed;
    std::optional<double> best_speed;

    std::string format_latest_centiseconds();
    std::string format_diff_centiseconds();
    std::string format_best_time();
    std::string format_latest_speed();
    std::string format_best_speed();

    void trigger(const motorst& mot, double time, bool is_game);
    void reset();
    checkpoint_state();
};

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

    checkpoint_state state;

    void left_clicked(const game_mouse& pos) override;
    void middle_clicked(const game_mouse& pos) override;
    void right_clicked(const game_mouse& pos) override;
    int distance(const game_mouse& pos) const override;
    void render(pic8& screen, vect2 corner) const;
    bool is_touching(const motorst& mot) const;

    checkpoint(vect2 coord)
        : start(this, &end, coord),
          end(this, &start, coord),
          polarity(Polarity::Both) {}

    static void held_callback(const game_mouse& pos, const mouse_input& input);
};

class checkpoints {
    std::list<checkpoint> linear;
    std::vector<checkpoint_state> objects;

    int food_index_offset = 0;

  public:
    static inline bool Editor = false;
    static inline bool Render = false;

    // Remove a checkpoint from the checkpoint list
    void remove(checkpoint* target);

    // Handle click with checkpoint editor open
    void editor_click(const game_mouse& pos, const mouse_input& input);

    // Render checkpoints
    void render_all(pic8& screen, vect2 corner);

    // Return checkpoint closest to pos within grabbing distance
    void get_closest(const game_mouse& pos, int& dist, clickable*& closest);

    // Trigger linear checkpoints touched by the bike
    void update_touching(const motorst& mot, double time, bool is_game);

    // Trigger one object collected by the bike
    void update_object(int object_id, const motorst& mot, double time, bool is_game);

    // Prepare checkpoints for a new game loop
    void setup();

    // Load checkpoint data for a specific level
    static void load_level(const level& lev);
};

extern std::unique_ptr<checkpoints> Checkpoints;

#endif
