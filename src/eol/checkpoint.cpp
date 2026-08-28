#include "eol/checkpoint.h"
#include "main.h"
#include "physics/init.h"
#include "pic/pic8.h"
#include "platform/implementation.h"
#include <list>

namespace {

bool editor_mode = true;

std::list<checkpoint> linear_checkpoints;

vect2 last_coord;

constexpr unsigned char LINE_COLOR = 25;

} // namespace

void checkpoint::endpoint::left_clicked(int x, int y, vect2 coord) {
    checkpoint::held_end = this;
    clickable::ClickMode = clickable::Mode::CheckpointEndHeld;
}

void checkpoint::endpoint::right_clicked(int x, int y, vect2 coord) {
    internal_error("Not implemented");
}

void checkpoint::editor_update(vect2 coord, bool left_click, bool right_click) {
    if (!editor_mode) {
        return;
    }

    if (!held_end && left_click) {
        linear_checkpoints.emplace_back(coord);
        held_end = &linear_checkpoints.back().end;
        last_coord = held_end->click_anchor;
        clickable::ClickMode = clickable::Mode::CheckpointEndHeld;
    } else if (held_end && left_click) {
        held_end->click_anchor = coord;
        held_end = nullptr;
        clickable::ClickMode = clickable::Mode::Normal;
    } else if (held_end && right_click) {
        internal_error("Not implemented - we need to enforce minimum line length before being able "
                       "to drop stuff");
        held_end->click_anchor = last_coord;
        held_end = nullptr;
        clickable::ClickMode = clickable::Mode::Normal;
    } else if (held_end) {
        held_end->click_anchor = coord;
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
    if (!editor_mode) {
        return;
    }
    for (const checkpoint& linear : linear_checkpoints) {
        linear.render(screen, corner);
    }
}

void checkpoint::get_closest(vect2 coord, int& dist, clickable*& closest) {
    if (!editor_mode) {
        return;
    }
    ELMA_ASSERT(!held_end);

    for (checkpoint& linear : linear_checkpoints) {
        int start_dist = linear.start.distance(coord);
        if (start_dist < dist) {
            dist = start_dist;
            closest = &linear.start;
        }

        int end_dist = linear.end.distance(coord);
        if (end_dist < dist) {
            dist = end_dist;
            closest = &linear.end;
        }
    }
}
