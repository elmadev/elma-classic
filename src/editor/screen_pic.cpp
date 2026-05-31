#include "editor/screen_pic.h"
#include "editor/canvas.h"
#include "editor/editor.h"
#include "main.h"
#include "pic/pic8.h"
#include "pic/surface.h"
#include "platform/implementation.h"
#include <algorithm>

static void draw_background(pic8& pic, screen_pic::Mode mode) {
    switch (mode) {
    case screen_pic::Mode::OutsideEditor:
        pic.fill_box(EditorPaletteId::BACKGROUND);
        return;
    case screen_pic::Mode::EditorGui:
        pic.fill_box(EditorPaletteId::MENU);
        break;
    case screen_pic::Mode::EditorCanvas:
        pic.fill_box(EditorPaletteId::BACKGROUND);
        pic.fill_box(0, EDITOR_MENU_Y, EDITOR_MENU_X - 1, pic.get_height() - 1,
                     EditorPaletteId::MENU);
        break;
    }
    // Tooltip section line (top)
    pic.line(0, EDITOR_MENU_Y - 1, pic.get_width() - 1, EDITOR_MENU_Y - 1,
             EditorPaletteId::MENU_BORDER);
    // Menu section line (left)
    pic.line(EDITOR_MENU_X - 1, EDITOR_MENU_Y, EDITOR_MENU_X - 1, pic.get_height() - 1,
             EditorPaletteId::MENU_BORDER);
}

static void draw_foreground(pic8& pic, screen_pic::Mode mode) {
    if (mode == screen_pic::Mode::OutsideEditor) {
        return;
    }
    draw_editor_border(pic);
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

    reset();
}

void screen_pic::blit_to_screen() {
    get_mouse_position(&MouseX, &MouseY);

    pic8* surface = lockbackbuffer_pic(false);
    draw_background(*surface, mode_);
    blit8(surface, pic());
    draw_foreground(*surface, mode_);
    draw_cursor(*surface, false);
    unlockbackbuffer_pic();
}

void screen_pic::reset() {
    draw_background(*pic(), mode_);
    if (initial_) {
        blit8(pic(), initial_);
    }
}
