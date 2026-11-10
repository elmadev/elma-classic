#include "eol/clickable.h"
#include "main.h"
#include "physics/init.h"
#include "pic/lgr.h"
#include <cmath>
#include <limits>

int game_clickable::distance(const game_mouse& pos) const {
    if (!pos.coord) {
        return std::numeric_limits<int>::max();
    }
    int dist = (int)((*pos.coord - click_anchor).length() * MetersToPixels);
    if (dist > click_radius) {
        return std::numeric_limits<int>::max();
    }
    return dist;
}

namespace {

box_overlay_clickable* held_overlay;
box last_box;
box held_box;
int last_x;
int last_y;
int* held_box_x;
int* held_box_y;

} // namespace

void box_overlay_clickable::grab_box(const game_mouse& pos) const {
    // Remember current position
    last_box = click_box();
    last_x = pos.x;
    last_y = pos.y;
    held_box = last_box;

    // Grabbing no edges
    held_box_x = nullptr;
    held_box_y = nullptr;
    if (vertical_stretch) {
        if (std::abs(pos.y - held_box.y1) <= EDGE) {
            // Grab top edge
            held_box_y = &held_box.y1;
        } else if (std::abs(pos.y - held_box.y2) <= EDGE) {
            // Grab bottom edge
            held_box_y = &held_box.y2;
        };
    }
    if (horizontal_stretch) {
        if (std::abs(pos.x - held_box.x1) <= EDGE) {
            // Grab left edge
            held_box_x = &held_box.x1;
        } else if (std::abs(pos.x - held_box.x2) <= EDGE) {
            // Grab right edge
            held_box_x = &held_box.x2;
        }
    }
}

int box_overlay_clickable::distance(const game_mouse& pos) const {
    box bx = click_box();
    if (horizontal_stretch) {
        bx.x1 -= EDGE;
        bx.x2 += EDGE;
    }
    if (vertical_stretch) {
        bx.y1 -= EDGE;
        bx.y2 += EDGE;
    }
    if (is_in_box(pos.x, pos.y, bx)) {
        return 0;
    }
    return std::numeric_limits<int>::max();
}

void box_overlay_clickable::get_closest(const game_mouse& pos, int& dist, clickable*& closest) {
    int new_dist = distance(pos);
    if (new_dist < dist) {
        dist = new_dist;
        closest = this;
    }
}

void box_overlay_clickable::left_clicked(const game_mouse& pos) {
    // Pick up overlay to move it around
    grab_box(pos);
    held_overlay = this;
    clickable::set_callback(held_callback, pos);
}

void box_overlay_clickable::right_clicked(const game_mouse& /*pos*/) { reset_box(); }

void box_overlay_clickable::held_callback(const game_mouse& pos, const mouse_input& input) {
    ELMA_ASSERT(held_overlay);

    // Update the box position
    if (!held_box_x && !held_box_y) {
        // No edges grabbed - move the whole box
        int dx = pos.x - last_x;
        int dy = pos.y - last_y;
        held_box.x1 = last_box.x1 + dx;
        held_box.x2 = last_box.x2 + dx;
        held_box.y1 = last_box.y1 + dy;
        held_box.y2 = last_box.y2 + dy;
    } else {
        // Move the grabbed edges
        if (held_box_x) {
            *held_box_x = pos.x;
        }
        if (held_box_y) {
            *held_box_y = pos.y;
        }
    }

    box new_box = held_box;
    if (new_box.x1 > new_box.x2) {
        std::swap(new_box.x1, new_box.x2);
    }
    if (new_box.y1 > new_box.y2) {
        std::swap(new_box.y1, new_box.y2);
    }

    if (input.left_click) {
        // Drop the overlay at current position
        held_overlay->set_box(new_box);
        held_overlay = nullptr;
        clickable::reset_callback();
    } else if (input.right_click || input.abort) {
        // Restore the overlay to its previous position
        held_overlay->set_box(last_box);
        held_overlay = nullptr;
        clickable::reset_callback();
    } else {
        // Update the current position
        held_overlay->set_box(new_box);
    }
}

void box_overlay_clickable::render_outline(pic8& screen) const {
    ELMA_ASSERT(Lgr);
    ::render_outline(&screen, click_box(), Lgr->minimap_border_palette_id);
}
