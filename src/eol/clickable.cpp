#include "eol/clickable.h"
#include "physics/init.h"
#include <climits>
#include <cmath>

int overlay_clickable::distance(int mou_x, int mou_y) const {
    double dx = mou_x - click_anchor_x;
    double dy = mou_y - click_anchor_y;
    int dist = (int)std::sqrt(dx * dx + dy * dy);
    if (dist > click_radius) {
        return INT_MAX;
    }
    return dist;
}

int game_clickable::distance(vect2 coord) const {
    int dist = (int)((coord - click_anchor).length() * MetersToPixels);
    if (dist > click_radius) {
        return INT_MAX;
    }
    return dist;
}
