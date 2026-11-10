#ifndef BOX_H
#define BOX_H

class pic8;

struct box {
    int x1, y1, x2, y2;
};

bool is_in_box(int x, int y, box bx);

enum class Click { Left, Right };
bool clicked_box(box bx, Click click = Click::Left);

void render_box(pic8* pic, int x1, int y1, int x2, int y2, unsigned char fill_id,
                unsigned char border_id);
void render_box(pic8* pic, box bx, unsigned char fill_id, unsigned char border_id);

#endif
