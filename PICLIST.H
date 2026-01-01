#ifndef PICLIST_H
#define PICLIST_H

#define MAX_PICLIST_LENGTH (3000)

class piclist {
  public:
    enum { None = 10, Palette0, TopLeft, TopRight, BottomLeft, BottomRight };

    enum { Picture = 100, Texture, Mask };

    int length;
    char name[(MAX_PICLIST_LENGTH + 10) * 10];
    int type[MAX_PICLIST_LENGTH + 10];
    int default_distance[MAX_PICLIST_LENGTH + 10];
    int default_clipping[MAX_PICLIST_LENGTH + 10];
    int transparency[MAX_PICLIST_LENGTH + 10];

    piclist(FILE* h);

    int get_index(const char* pic_name);
};

#endif
