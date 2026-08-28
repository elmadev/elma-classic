#include "eol/checkpoint.h"
#include "main.h"
#include "physics/init.h"
#include "pic/pic8.h"
#include "platform/implementation.h"
#include <list>

namespace {

constexpr unsigned char LINE_COLOR = 25;

std::list<checkpoint> linear_checkpoints;

vect2 last_coord;

} // namespace

void checkpoint::endpoint::left_clicked(const game_mouse& /*pos*/) {
    // Pick up checkpoint end
    checkpoint::held_end = this;
    clickable::ClickMode = clickable::Mode::CheckpointEndHeld;
}

void checkpoint::endpoint::right_clicked(const game_mouse& /*pos*/) {}

void checkpoint::editor_update(const game_mouse& pos, bool left_click, bool right_click) {
    if (!Editor) {
        return;
    }
    if (!pos.coord) {
        return;
    }
    const vect2& coord = *pos.coord;

    if (clickable::ClickMode == clickable::Mode::Normal) {
        if (left_click) {
            // Create a new checkpoint line and hold the end
            linear_checkpoints.emplace_back(coord);
            held_end = &linear_checkpoints.back().end;
            last_coord = held_end->click_anchor;
            clickable::ClickMode = clickable::Mode::CheckpointEndHeld;
        }
    } else if (clickable::ClickMode == clickable::Mode::CheckpointEndHeld) {
        if (left_click) {
            // Drop the checkpoint end
            held_end->click_anchor = coord;
            held_end = nullptr;
            clickable::ClickMode = clickable::Mode::Normal;
        } else if (right_click) {
            // Restore the checkpoint end to its previous position
            internal_error("Not implemented");
        } else {
            // Update the checkpoint end position
            held_end->click_anchor = coord;
        }
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

void checkpoint::get_closest(const game_mouse& pos, int& dist, clickable*& closest) {
    if (!Editor) {
        return;
    }
    if (!pos.coord) {
        return;
    }
    ELMA_ASSERT(!held_end);

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
    }
}
