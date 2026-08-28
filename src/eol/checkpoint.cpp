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

void checkpoint::editor_update(vect2 coord, bool left_click, bool right_click) {
    if (!Editor) {
        return;
    }

    if (!held_end && left_click) {
        // Create a new checkpoint line and hold the end
        linear_checkpoints.emplace_back(coord);
        held_end = &linear_checkpoints.back().end;
        last_coord = held_end->click_anchor;
    } else if (held_end && left_click) {
        // Drop the checkpoint end
        held_end->click_anchor = coord;
        held_end = nullptr;
    } else if (held_end && right_click) {
        // Restore the checkpoint end to its previous position
        internal_error("Not implemented");
    } else if (held_end) {
        // Update the checkpoint end position
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
    if (!Editor && !Render) {
        return;
    }
    for (const checkpoint& linear : linear_checkpoints) {
        linear.render(screen, corner);
    }
}
