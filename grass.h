#ifndef GRASS_H
#define GRASS_H

#include <memory>
#include <vector>

class pic8;
class polygon;
class vect2;

constexpr int MAX_GRASS_PICS = 100;
constexpr int QGRASS_EXTRA_HEIGHT = 20;
constexpr int GRASS_MARGIN = 20;

struct updown {
    std::unique_ptr<pic8> pic;
    bool is_up;
    int slope;
};

class grass {
  public:
    std::vector<updown> elements;
    grass() = default;
    void add(pic8* pic, bool up);
};

bool create_grass_polygon_heightmap(polygon* poly, int* heightmap, int* heightmap_length, int* x0,
                                    int max_heightmap_length, vect2* origin);

#endif
