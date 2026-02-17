#include "renderer/object_overlay.h"
#include "anim.h"
#include "eol_settings.h"
#include "main.h"
#include "pic8.h"

// Gravity arrow sprites indexed by object::Property (1=Up, 2=Down, 3=Left, 4=Right)
constexpr int GRAVITY_ARROW_COUNT = 4;
static pic8* GravityArrows[GRAVITY_ARROW_COUNT] = {};

static_assert((int)object::Property::GravityUp == 1);
static_assert((int)object::Property::GravityDown == 2);
static_assert((int)object::Property::GravityLeft == 3);
static_assert((int)object::Property::GravityRight == 4);

static pic8* load_arrow_bmp() { return pic8::from_bmp("resources/gravarrow.bmp"); }

static void free_gravity_arrows() {
    for (int i = 0; i < GRAVITY_ARROW_COUNT; i++) {
        delete GravityArrows[i];
        GravityArrows[i] = nullptr;
    }
}

void init_gravity_arrows() {
    free_gravity_arrows();

    pic8* arrow_down = load_arrow_bmp();
    if (!arrow_down) {
        return;
    }

    unsigned char transparency_color = arrow_down->gpixel(0, 0);

    int target_height = (int)(ANIM_WIDTH * EolSettings->zoom());
    arrow_down = pic8::resize(arrow_down, target_height);
    pic8* arrow_up = arrow_down->clone();
    arrow_up->vertical_flip();

    // Screen is rendered upside-down, so up/down arrows are swapped.
    // Left/right are derived by transposing the vertical arrows.
    pic8* arrows[] = {arrow_down, arrow_up, pic8::transpose(arrow_up), pic8::transpose(arrow_down)};
    for (int i = 0; i < GRAVITY_ARROW_COUNT; i++) {
        arrows[i]->add_transparency(transparency_color);
        GravityArrows[i] = arrows[i];
    }
}

void draw_gravity_arrow(pic8* pic, int obj_i, int obj_j, object::Property property) {
    if (property < object::Property::GravityUp || property > object::Property::GravityRight) {
        internal_error("unknown property in draw_gravity_arrow()!");
    }
    pic8* arrow = GravityArrows[(int)property - 1];
    if (!arrow) {
        return;
    }

    int obj_half_size = (int)(ANIM_WIDTH * EolSettings->zoom()) / 2;
    int arrow_i = obj_i + obj_half_size - arrow->get_width() / 2;
    int arrow_j = obj_j + obj_half_size - arrow->get_height() / 2;

    constexpr unsigned char FONT_PALETTE_INDEX = 0x19;
    blit8_recolor(pic, arrow, arrow_i, arrow_j, FONT_PALETTE_INDEX);
}
