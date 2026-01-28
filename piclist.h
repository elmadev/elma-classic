#ifndef PICLIST_H
#define PICLIST_H

#include "sprite.h"
#include <cstdio>

#define MAX_PICLIST_LENGTH (3000)

class piclist {
  public:
    enum class Transparency {
        None = 10,
        Palette0 = 11,
        TopLeft = 12,
        TopRight = 13,
        BottomLeft = 14,
        BottomRight = 15
    };

    enum class Type { Picture = 100, Texture = 101, Mask = 102 };

    int length;
    char name[MAX_PICLIST_LENGTH + 10][10];
    Type type[MAX_PICLIST_LENGTH + 10];
    int default_distance[MAX_PICLIST_LENGTH + 10];
    Clipping default_clipping[MAX_PICLIST_LENGTH + 10];
    Transparency transparency[MAX_PICLIST_LENGTH + 10];

    piclist(FILE* h);

    int get_index(const char* pic_name);
};

#endif
