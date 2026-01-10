#ifndef GRASS_H
#define GRASS_H

class pic8;
class polygon;
class vect2;

#define MAX_GRASS_PICS (100)

#define QGRASS_EXTRA_HEIGHT (20)

class grass {
  public:
    int length;
    pic8* pics[MAX_GRASS_PICS];
    bool is_up[MAX_GRASS_PICS];
    grass();
    ~grass();
    void add(pic8* pic, bool up);
};

bool create_grass_polygon_heightmap(polygon* poly, int* heightmap, int* heightmap_length, int* x0,
                                    int max_heightmap_length, vect2* origin);

#endif
