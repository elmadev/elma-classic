#ifndef KOVETO_H
#define KOVETO_H

#define MAX_GRASS_PICS (100)

#define QGRASS_EXTRA_HEIGHT (20)

class koveto {
  public:
    int length;
    pic8* pics[MAX_GRASS_PICS];
    int is_up[MAX_GRASS_PICS];
    koveto(void);
    ~koveto(void);
    void add(pic8* pic, int up);
};

int create_grass_polygon_heightmap(gyuru* poly, int* heightmap, int* heightmap_length, int* x0,
                                   int max_heightmap_length, vect2* origin);

#endif
