#include "editor/screen_pic.h"
#include "editor/editor.h"
#include "main.h"
#include "pic/pic8.h"
#include "pic/surface.h"
#include "platform/implementation.h"
#include <algorithm>

screen_pic::screen_pic(pic8* initial)
    : initial_(initial) {
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
    blit8(surface, pic());
    draw_cursor(*surface, false);
    unlockbackbuffer_pic();
}

void screen_pic::reset() {
    if (initial_) {
        blit8(pic(), initial_);
    }
}
