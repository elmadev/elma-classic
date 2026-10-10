#include "eol/checkpoint.h"
#include "main.h"
#include "physics/init.h"
#include "pic/pic8.h"
#include "platform/implementation.h"
#include <limits>
#include <list>

namespace {

constexpr unsigned char LINE_COLOR = 25;

std::list<checkpoint> linear_checkpoints;

vect2 last_coord;
vect2 last_start;
vect2 last_end;

} // namespace

void checkpoint::endpoint::left_clicked(const game_mouse& pos) {
    // Pick up checkpoint end and remember position in case we cancel
    last_coord = this->click_anchor;
    checkpoint::held_end = this;
    clickable::set_callback(held_callback, pos);
}

void checkpoint::endpoint::right_clicked(const game_mouse& /*pos*/) {}

void checkpoint::endpoint::set_anchor(vect2 coord) {
    // If line as at least MINIMUM_LENGTH, then set to desired coord
    vect2 direction = coord - other->click_anchor;
    double length = direction.length();
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

void checkpoint::endpoint::held_callback(const game_mouse& pos, const mouse_input& input) {
    ELMA_ASSERT(Editor);
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
    } else if (input.right_click) {
        // Restore the checkpoint end to its previous position
        held_end->set_anchor(last_coord);
        held_end = nullptr;
        clickable::reset_callback();
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
    checkpoint::held_line = this;
    clickable::set_callback(held_callback, pos);
}

void checkpoint::right_clicked(const game_mouse& /*pos*/) {}

void checkpoint::held_callback(const game_mouse& pos, const mouse_input& input) {
    ELMA_ASSERT(Editor);
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
    } else if (input.right_click) {
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

void checkpoint::editor_click(const game_mouse& pos, const mouse_input& input) {
    if (!Editor) {
        return;
    }
    if (!pos.coord) {
        return;
    }
    const vect2& coord = *pos.coord;

    if (input.left_click) {
        // Create a new checkpoint line and hold the end
        linear_checkpoints.emplace_back(coord);
        held_end = &linear_checkpoints.back().end;
        last_coord = held_end->click_anchor;
        clickable::set_callback(endpoint::held_callback, pos);
    }
}

void checkpoint::render(pic8& screen, vect2 corner) const {
    double x1 = start.click_anchor.x - corner.x;
    double y1 = start.click_anchor.y - corner.y;
    double x2 = end.click_anchor.x - corner.x;
    double y2 = end.click_anchor.y - corner.y;
    x1 *= MetersToPixels;
    y1 *= MetersToPixels;
    x2 *= MetersToPixels;
    y2 *= MetersToPixels;
    screen.line(x1, y1, x2, y2, LINE_COLOR);
}

void checkpoint::render_all(pic8& screen, vect2 corner) {
    if (!Editor && !Render) {
        return;
    }
    for (const checkpoint& linear : linear_checkpoints) {
        linear.render(screen, corner);
    }
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

void checkpoint::get_closest(const game_mouse& pos, int& dist, clickable*& closest) {
    if (!Editor) {
        return;
    }
    if (!pos.coord) {
        return;
    }
    ELMA_ASSERT(!held_end);
    ELMA_ASSERT(!held_line);

    for (checkpoint& linear : linear_checkpoints) {
        int start_dist = linear.start.distance(pos);
        if (start_dist < dist) {
            dist = start_dist;
            closest = &linear.start;
        }

        int end_dist = linear.end.distance(pos);
        if (end_dist < dist) {
            dist = end_dist;
            closest = &linear.end;
        }

        if (start_dist == std::numeric_limits<int>::max() &&
            end_dist == std::numeric_limits<int>::max()) {
            int middle_dist = linear.distance(pos);
            if (middle_dist > clickable::DEFAULT_RADIUS) {
                middle_dist = std::numeric_limits<int>::max();
            } else {
                // Prioritize checkpoint ends over checkpoint lines by giving lines lower priority
                middle_dist += clickable::DEFAULT_RADIUS;
            }

            if (middle_dist < dist) {
                dist = middle_dist;
                closest = &linear;
            }
        }
    }
}
