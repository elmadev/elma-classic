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

void checkpoint::editor_update(vect2 coord, bool left_click, bool right_click) {
    if (!editor_mode) {
        return;
    }

    if (!held && left_click) {
        linear_checkpoints.emplace_back(coord);
        held = &linear_checkpoints.back().end;
        last_coord = held->click_anchor;
    } else if (held && left_click) {
        held->click_anchor = coord;
        held = nullptr;
    } else if (held && right_click) {
        internal_error("Not implemented - we need to enforce minimum line length before being able "
                       "to drop stuff");
        held->click_anchor = last_coord;
        held = nullptr;
    } else if (held) {
        held->click_anchor = coord;
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
