#include "box.h"
#include "editor/editor.h"
#include "pic/pic8.h"
#include "platform/implementation.h"

// Draw a box with a 1-pixel border
void render_box(pic8* pic, int x1, int y1, int x2, int y2, unsigned char fill_id,
                unsigned char border_id) {
    pic->fill_box(x1, y1, x2, y2, fill_id);
    pic->line(x1, y1, x2, y1, border_id);
    pic->line(x1, y2, x2, y2, border_id);
    pic->line(x1, y1, x1, y2, border_id);
    pic->line(x2, y1, x2, y2, border_id);
}

// Draw a box with a 1-pixel border
void render_box(pic8* pic, box bx, unsigned char fill_id, unsigned char border_id) {
    render_box(pic, bx.x1, bx.y1, bx.x2, bx.y2, fill_id, border_id);
}

// Return true if the coordinates are within the specified box
bool is_in_box(int x, int y, box bx) {
    return x >= bx.x1 && x <= bx.x2 && y >= bx.y1 && y <= bx.y2;
}

// Return true if we clicked within the specified box
bool clicked_box(box bx, Click click) {
    if (click == Click::Left && !was_left_mouse_just_clicked()) {
        return false;
    }
    if (click == Click::Right && !was_right_mouse_just_clicked()) {
        return false;
    }

    return is_in_box(MouseX, MouseY, bx);
}
