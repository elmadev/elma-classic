#include "eol/checkpoint.h"
#include "level/level.h"
#include "level/object.h"
#include "log.h"
#include "main.h"
#include "physics/init.h"
#include "pic/pic8.h"
#include "platform/implementation.h"
#include "platform/scancode.h"
#include "renderer/timer.h"
#include "util/util.h"
#include <algorithm>
#include <cmath>
#include <format>
#include <limits>
#include <numbers>

std::unique_ptr<checkpoints> Checkpoints = std::make_unique<checkpoints>();

namespace {

constexpr unsigned char LINE_COLOR = 25;

bool delete_endpoint_if_dropped;
vect2 last_coord;
vect2 last_start;
vect2 last_end;

endpoint* held_end = nullptr;
checkpoint* held_line = nullptr;

} // namespace

static std::string format_centiseconds(int centiseconds) {
    char text[64];
    util::text::centiseconds_to_string(centiseconds, text, true, true);
    return text;
}

std::string checkpoint_state::format_latest_centiseconds() const {
    if (!latest_centiseconds) {
        return "";
    }
    return format_centiseconds(*latest_centiseconds);
}

std::string checkpoint_state::format_diff_centiseconds() const {
    if (!diff_centiseconds) {
        return "";
    }

    if (*diff_centiseconds > 0) {
        return std::format("+{}", format_centiseconds(*diff_centiseconds));
    }
    if (*diff_centiseconds < 0) {
        return std::format("-{}", format_centiseconds((-1) * (*diff_centiseconds)));
    }

    return "0:00";
}

std::string checkpoint_state::format_best_time() const {
    if (!best_centiseconds) {
        return "";
    }
    return format_centiseconds(*best_centiseconds);
}

static std::string format_speed(double speed) { return std::format("{:.2f}", speed); }

std::string checkpoint_state::format_latest_speed() const {
    if (!latest_speed) {
        return "";
    }
    return format_speed(*latest_speed);
}

std::string checkpoint_state::format_best_speed() const {
    if (!best_speed) {
        return "";
    }
    return format_speed(*best_speed);
}

void checkpoint_state::trigger(const motorst& mot, double time, bool is_game) {
    if (!active) {
        return;
    }
    active = false;

    latest_centiseconds = (int)(time * TIME_TO_CENTISECONDS);
    if (best_centiseconds) {
        diff_centiseconds = *latest_centiseconds - *best_centiseconds;
    }
    if (is_game) {
        latest_speed = mot.speed();
        if (*latest_centiseconds < best_centiseconds.value_or(std::numeric_limits<int>::max())) {
            best_centiseconds = latest_centiseconds;
            best_speed = latest_speed;
        }
    }

    LOG_DEBUG("{} - time: {}, diff {}, speed: {}", label, format_latest_centiseconds(),
              format_diff_centiseconds(), format_latest_speed());
}

void checkpoint_state::reset() {
    active = true;
    latest_centiseconds.reset();
    diff_centiseconds.reset();
    latest_speed.reset();
};

checkpoint_state::checkpoint_state() { reset(); }

void endpoint::left_clicked(const game_mouse& pos) {
    // Pick up checkpoint end and remember position in case we cancel
    last_coord = this->click_anchor;
    delete_endpoint_if_dropped = false;
    held_end = this;
    clickable::set_callback(held_callback, pos);
}

void endpoint::middle_clicked(const game_mouse& pos) {
    // Toggle polarity
    parent->middle_clicked(pos);
}

void endpoint::right_clicked(const game_mouse& pos) {
    // Delete checkpoint
    parent->right_clicked(pos);
}

void endpoint::set_anchor(vect2 coord) {
    vect2 direction = coord - other->click_anchor;
    double length = direction.length();

    if (is_key_down(DIK_LSHIFT) || is_key_down(DIK_RSHIFT)) {
        // Enforce minimum length
        length = std::max(length, MINIMUM_LENGTH);

        // Round the angle to one of 8 cardinal/ordinal directions
        double angle = std::atan2(direction.y, direction.x);
        constexpr double tau = 2.0 * std::numbers::pi;
        constexpr double fraction = tau / 8.0;
        angle = std::round(angle / fraction) * fraction;

        // Set the line along the angle with the correct distance from the other end
        click_anchor =
            other->click_anchor + vect2{std::cos(angle) * length, std::sin(angle) * length};
        return;
    }

    // No snap (default case)

    // If line as at least MINIMUM_LENGTH, then set to desired coord
    if (length >= MINIMUM_LENGTH) {
        click_anchor = coord;
        return;
    }

    // Handle divide by 0 case
    if (length < 0.0001) {
        direction = vect2{1.0, 0.0};
    }

    // If line as shorter than MINIMUM_LENGTH, project the line in a straight line
    direction.normalize();
    click_anchor = other->click_anchor + direction * MINIMUM_LENGTH;
}

void endpoint::held_callback(const game_mouse& pos, const mouse_input& input) {
    ELMA_ASSERT(checkpoints::Editor);
    ELMA_ASSERT(held_end);
    if (!pos.coord) {
        return;
    }
    const vect2& coord = *pos.coord;

    if (input.left_click) {
        // Drop the checkpoint end
        held_end->set_anchor(coord);
        held_end = nullptr;
        clickable::reset_callback();
    } else if (input.right_click || input.abort) {
        if (delete_endpoint_if_dropped) {
            // Delete a partially created new checkpoint
            held_end->right_clicked(pos);
            held_end = nullptr;
            clickable::reset_callback();
        } else {
            // Restore the checkpoint end to its previous position
            held_end->click_anchor = last_coord;
            held_end = nullptr;
            clickable::reset_callback();
        }
    } else {
        // Update the checkpoint end position
        held_end->set_anchor(coord);
    }
}

void checkpoint::left_clicked(const game_mouse& pos) {
    ELMA_ASSERT(pos.coord);
    last_coord = *pos.coord;
    last_start = start.click_anchor;
    last_end = end.click_anchor;
    held_line = this;
    clickable::set_callback(held_callback, pos);
}

void checkpoint::middle_clicked(const game_mouse& /*pos*/) {
    // Toggle polarity
    switch (polarity) {
    case Polarity::Both:
        polarity = Polarity::Positive;
        break;
    case Polarity::Positive:
        polarity = Polarity::Negative;
        break;
    case Polarity::Negative:
        polarity = Polarity::Both;
        break;
    }
}

void checkpoint::right_clicked(const game_mouse& /*pos*/) {
    // Delete checkpoint
    Checkpoints->remove(this);
}

void checkpoint::held_callback(const game_mouse& pos, const mouse_input& input) {
    ELMA_ASSERT(checkpoints::Editor);
    ELMA_ASSERT(held_line);
    if (!pos.coord) {
        return;
    }
    const vect2& coord = *pos.coord;

    if (input.left_click) {
        // Drop the checkpoint line
        held_line->start.click_anchor = last_start + (coord - last_coord);
        held_line->end.click_anchor = last_end + (coord - last_coord);
        held_line = nullptr;
        clickable::reset_callback();
    } else if (input.right_click || input.abort) {
        // Restore the checkpoint line to its previous position
        held_line->start.click_anchor = last_start;
        held_line->end.click_anchor = last_end;
        held_line = nullptr;
        clickable::reset_callback();
    } else {
        // Update the checkpoint line position
        held_line->start.click_anchor = last_start + (coord - last_coord);
        held_line->end.click_anchor = last_end + (coord - last_coord);
    }
}

void checkpoint::render(pic8& screen, vect2 corner) const {
    // Draw line
    double x1 = start.click_anchor.x - corner.x;
    double y1 = start.click_anchor.y - corner.y;
    double x2 = end.click_anchor.x - corner.x;
    double y2 = end.click_anchor.y - corner.y;
    x1 *= MetersToPixels;
    y1 *= MetersToPixels;
    x2 *= MetersToPixels;
    y2 *= MetersToPixels;
    screen.line(x1, y1, x2, y2, LINE_COLOR);

    // Draw polarity
    if (polarity == Polarity::Both) {
        return;
    }
    // Get polarity direction (orthogonal to checkpoint line)
    double direction_x = x2 - x1;
    double direction_y = y2 - y1;
    double angle = std::atan2(direction_y, direction_x);
    angle += std::numbers::pi / 2.0;
    if (polarity == Polarity::Negative) {
        angle += std::numbers::pi;
    }

    // Draw an arrow starting at the centerpoint of the checkpoint, scaled based on the length
    double x_middle = (x1 + x2) / 2.0;
    double y_middle = (y1 + y2) / 2.0;
    constexpr double LENGTH = 25.0;

    // Describe arrow in vector space
    const double TIP = std::sqrt(0.125);
    vect2 arrow[] = {
        {-1.5, 0.0}, {-0.5 - TIP, 0.0}, {-0.5, 0.0}, {-0.5 - TIP, TIP}, {-0.5 - TIP, -TIP}};

    // Apply matrix transformation
    // https://upload.wikimedia.org/wikipedia/commons/2/2c/2D_affine_transformation_matrix.svg
    // Translate + Scale + Rotate
    // [1 0 X] [len 0  0] [cos -sin 0] [x] [len*cos -len*sin X][x] [len*cos*x - len*sin*y + X]
    // [0 1 Y]*[ 0 len 0]*[sin cos  0]*[y]=[len*sin len*cos  Y][y]=[len*sin*x + len*cos*y + Y]
    // [0 0 1] [ 0  0  1] [ 0   0   1] [1] [   0       0     1][1] [          1              ]
    for (vect2& point : arrow) {
        double x = point.x;
        double y = point.y;
        point.x = LENGTH * std::cos(angle) * x - LENGTH * std::sin(angle) * y + x_middle;
        point.y = LENGTH * std::sin(angle) * x + LENGTH * std::cos(angle) * y + y_middle;
    }

    // Draw arrow
    screen.line(arrow[0].x, arrow[0].y, arrow[1].x, arrow[1].y, LINE_COLOR);
    screen.line(arrow[2].x, arrow[2].y, arrow[3].x, arrow[3].y, LINE_COLOR);
    screen.line(arrow[3].x, arrow[3].y, arrow[4].x, arrow[4].y, LINE_COLOR);
    screen.line(arrow[4].x, arrow[4].y, arrow[2].x, arrow[2].y, LINE_COLOR);
}

int checkpoint::distance(const game_mouse& pos) const {
    if (!pos.coord) {
        return std::numeric_limits<int>::max();
    }
    vect2 start_pos = start.click_anchor;
    vect2 end_pos = end.click_anchor;
    vect2 v = end_pos - start_pos;
    return (int)(point_segment_distance(*pos.coord, start_pos, v) * MetersToPixels);
}

void checkpoints::remove(checkpoint* target) {
    auto it = std::find_if(linear.begin(), linear.end(),
                           [target](const checkpoint& element) { return target == &element; });
    ELMA_ASSERT(it != linear.end());
    linear.erase(it);

    enumerate();
}

void checkpoints::editor_click(const game_mouse& pos, const mouse_input& input) {
    if (!Editor) {
        return;
    }
    if (!pos.coord) {
        return;
    }
    const vect2& coord = *pos.coord;

    if (input.left_click) {
        // Create a new checkpoint line and hold the end
        linear.emplace_back(coord);
        held_end = &linear.back().end;
        delete_endpoint_if_dropped = true;
        last_coord = held_end->click_anchor;

        enumerate();

        clickable::set_callback(endpoint::held_callback, pos);
    }
}

void checkpoints::render_all(pic8& screen, vect2 corner) {
    if (!Editor && !Render) {
        return;
    }
    for (const checkpoint& line : linear) {
        line.render(screen, corner);
    }
}

void checkpoints::get_closest(const game_mouse& pos, int& dist, clickable*& closest) {
    if (!Editor) {
        return;
    }
    if (!pos.coord) {
        return;
    }
    ELMA_ASSERT(!held_end);
    ELMA_ASSERT(!held_line);

    for (checkpoint& line : linear) {
        int start_dist = line.start.distance(pos);
        if (start_dist < dist) {
            dist = start_dist;
            closest = &line.start;
        }

        int end_dist = line.end.distance(pos);
        if (end_dist < dist) {
            dist = end_dist;
            closest = &line.end;
        }

        if (start_dist == std::numeric_limits<int>::max() &&
            end_dist == std::numeric_limits<int>::max()) {
            int middle_dist = line.distance(pos);
            if (middle_dist > clickable::DEFAULT_RADIUS) {
                middle_dist = std::numeric_limits<int>::max();
            } else {
                // Prioritize checkpoint ends over checkpoint lines by giving lines lower priority
                middle_dist += clickable::DEFAULT_RADIUS;
            }

            if (middle_dist < dist) {
                dist = middle_dist;
                closest = &line;
            }
        }
    }
}

bool checkpoint::is_touching(const motorst& mot) const {
    vect2 checkpoint_v = end.click_anchor - start.click_anchor;

    // We check the bike itself to avoid triggering a checkpoint with rotation-induced backwards
    // motion of a wheel
    if (polarity != Polarity::Both) {
        double bike_polarity = rotate_90deg(checkpoint_v) * mot.bike.v;
        if (polarity == Polarity::Positive && bike_polarity < 0.0) {
            return false;
        }
        if (polarity == Polarity::Negative && bike_polarity > 0.0) {
            return false;
        }
    }

    auto rigidbody_distance = [&](const vect2 rb_r) {
        // Optimized implementation of vect2::point_segment_distance()
        vect2 rr = rb_r - start.click_anchor;
        double scalar = checkpoint_v * rr;
        if (scalar <= 0.0) {
            return rr.length();
        }
        double checkpoint_length_squared = checkpoint_v * checkpoint_v;
        if (scalar >= checkpoint_length_squared) {
            return (rb_r - end.click_anchor).length();
        }
        vect2 n = rotate_90deg(checkpoint_v * (1.0 / std::sqrt(checkpoint_length_squared)));
        return fabs(n * rr);
    };
    if (rigidbody_distance(mot.left_wheel.r) <= mot.left_wheel.radius) {
        return true;
    }
    if (rigidbody_distance(mot.right_wheel.r) <= mot.right_wheel.radius) {
        return true;
    }
    if (rigidbody_distance(mot.head_r) <= HeadRadius) {
        return true;
    }
    return false;
}

void checkpoints::update_touching(const motorst& mot, double time, bool is_game) {
    for (checkpoint& line : linear) {
        if (!line.state.active) {
            continue;
        }
        if (!line.is_touching(mot)) {
            continue;
        }
        line.state.trigger(mot, time, is_game);
    }
}

void checkpoints::update_object(int object_id, const motorst& mot, double time, bool is_game) {
    int checkpoint_id = object_id - food_index_offset;
    if (checkpoint_id < 0 || checkpoint_id >= objects.size()) {
        // Killer or Start object
        return;
    }
    objects[checkpoint_id].trigger(mot, time, is_game);
}

void checkpoints::enumerate() {
    int line_i = 1;
    for (checkpoint& line : linear) {
        line.state.label = std::format("Checkpoint {}", line_i++);
    }
    int obj_i = 1;
    for (checkpoint_state& obj : objects) {
        obj.label = std::format("Apple {}", obj_i++);
    }
}

void checkpoints::setup() {
    for (checkpoint& line : linear) {
        line.state.reset();
    }
    for (checkpoint_state& obj : objects) {
        obj.reset();
    }
}

void checkpoints::load_level(const level& lev) {
    // Get level properties
    int food_index_offset = 0;
    int apple_count = 0;
    for (int i = 0; i < MAX_OBJECTS; i++) {
        const object* obj = lev.objects[i];
        if (!obj) {
            continue;
        }
        if (obj->type == object::Type::Killer) {
            food_index_offset++;
        }
        if (obj->type == object::Type::Food) {
            apple_count++;
        }
    }

    // Default checkpoint data
    Checkpoints = std::make_unique<checkpoints>();
    Checkpoints->food_index_offset = food_index_offset;
    Checkpoints->objects.resize(apple_count);

    Checkpoints->enumerate();
}
