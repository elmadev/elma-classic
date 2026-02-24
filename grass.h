#ifndef GRASS_H
#define GRASS_H

#include "lgr.h"
#include <memory>
#include <vector>

class pic8;
class polygon;
class vect2;

constexpr int MAX_GRASS_PICS = 100;
constexpr int QUPDOWN_MARGIN = 20;
constexpr int QGRASS_MARGIN = QUPDOWN_MARGIN + 20;

struct updown {
    std::unique_ptr<pic8> pic;
    bool is_up;
    int slope;
    mask msk;
};

class grass {
  public:
    std::vector<updown> elements;
    grass() = default;
    ~grass();
    void add(pic8* pic, bool up, int target_height, double qupdown_zoom, double zoom);
};

bool create_grass_polygon_heightmap(polygon* poly, int* heightmap, int* heightmap_length, int* x0,
                                    int max_heightmap_length, vect2* origin);

#endif
