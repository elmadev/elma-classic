#include "eol/clickable.h"
#include "physics/init.h"
#include <cmath>
#include <limits>

int overlay_clickable::distance(const game_mouse& pos) const {
    double dx = pos.x - click_anchor_x;
    double dy = pos.y - click_anchor_y;
    int dist = (int)std::sqrt(dx * dx + dy * dy);
    if (dist > click_radius) {
        return std::numeric_limits<int>::max();
    }
    return dist;
}

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
