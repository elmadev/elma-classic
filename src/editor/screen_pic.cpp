#include "editor/screen_pic.h"
#include "editor/canvas.h"
#include "editor/editor.h"
#include "main.h"
#include "pic8.h"
#include "M_PIC.H"
#include "platform/implementation.h"
#include <algorithm>

static void draw_background(pic8& pic, screen_pic::Mode mode) {
    switch (mode) {
    case screen_pic::Mode::OutsideEditor:
        pic.fill_box(EditorPaletteId::BACKGROUND);
        return;
    case screen_pic::Mode::EditorGui:
        pic.fill_box(EditorPaletteId::MENU);
        return;
    case screen_pic::Mode::EditorCanvas:
        pic.fill_box(EditorPaletteId::BACKGROUND);
        pic.fill_box(0, EDITOR_MENU_Y, EDITOR_MENU_X - 1, pic.get_height() - 1,
                     EditorPaletteId::MENU);
        return;
    }
}

screen_pic::screen_pic(pic8* initial, Mode mode)
    : initial_(initial),
      mode_(mode) {
    int width = MIN_WIDTH;
    int height = MIN_HEIGHT;
    if (initial_) {
        width = std::max(width, initial_->get_width());
        height = std::max(height, initial_->get_height());
    }

    pic_ = std::make_unique<pic8>(width, height);

    if (initial_) {
        reset();
    }
}

static void draw_cursor_pixel(pic8& surface, int x, int y) {
    if (x < 0 || y < 0 || x >= surface.get_width() || y > surface.get_height()) {
        return;
    }
    unsigned char palette_id = surface.gpixel(x, y);
    palette_id += 128;
    surface.ppixel(x, y, palette_id);
}

void screen_pic::blit_to_screen(bool cursor_shape_is_x) {
    get_mouse_position(&MouseX, &MouseY);

    pic8* surface = lockbackbuffer_pic(false);

    draw_background(*surface, mode_);
    blit8(surface, pic());

    constexpr int CURSOR_RADIUS = 4;
    for (int i = -CURSOR_RADIUS; i <= CURSOR_RADIUS; i++) {
        if (cursor_shape_is_x) {
            // Draw "x"
            draw_cursor_pixel(*surface, MouseX + i, MouseY + i);
            if (i != 0) {
                draw_cursor_pixel(*surface, MouseX + i, MouseY - i);
            }
        } else {
            // Draw "+"
            draw_cursor_pixel(*surface, MouseX + i, MouseY);
            if (i != 0) {
                draw_cursor_pixel(*surface, MouseX, MouseY + i);
            }
        }
    }

    unlockbackbuffer_pic();
}

void screen_pic::reset() {
    if (!initial_) {
        internal_error("screen_pic::restore !initial_");
    }
    draw_background(*pic(), mode_);
    blit8(pic(), initial_);
}
