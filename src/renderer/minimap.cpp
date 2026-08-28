#include "editor/editor.h"
#include "eol/eol.h"
#include "game/driver.h"
#include "game/game.h"
#include "level/object.h"
#include "pic/pic8.h"
#include "renderer/canvas.h"

// In pixels from the bottom-left corner of screen
// (defined in render.cpp)
extern int MinimapWidth;
extern int MinimapHeight;
extern int MinimapX;
extern int MinimapDx;

// Render a 3x3 square onto the minimap
static void render_minimap_icon(pic8* pic, int x, int y, unsigned char palette_id) {
    if (x < -1 || x > pic->get_width() || y < -1 || y > pic->get_height()) {
        // Skip drawing icons that are completely out of bounds
        return;
    }
    pic->ppixel(x - 1, y - 1, palette_id);
    pic->ppixel(x, y - 1, palette_id);
    pic->ppixel(x + 1, y - 1, palette_id);
    pic->ppixel(x - 1, y, palette_id);
    pic->ppixel(x + 1, y, palette_id);
    pic->ppixel(x - 1, y + 1, palette_id);
    pic->ppixel(x, y + 1, palette_id);
    pic->ppixel(x + 1, y + 1, palette_id);
}

void render_minimap_subview(bool player1, pic8* minimap_view, const motorst* other_motor,
                            vect2 bottomleft_corner, vect2 camera_pos) {
    // Draw the background (polygons)
    CanvasMinimap->render_minimap(player1, minimap_view, bottomleft_corner, 0, 0, MinimapWidth - 1,
                                  MinimapHeight - 1);

    // Draw the objects
    int corner_x;
    int corner_y;
    CanvasMinimap->meters_to_pixels(bottomleft_corner, &corner_x, &corner_y);
    const kuski* spy_kuski = EolClient->spy_kuski();
    for (int i = 0; i < MAX_OBJECTS; i++) {
        object* obj = Level->objects[i];
        if (!obj) {
            break;
        }

        unsigned char palette_id;
        switch (obj->type) {
        case object::Type::Food:
            if (!obj->active || (spy_kuski && spy_kuski->apples_taken[i])) {
                continue;
            }
            palette_id = Lgr->minimap_food_palette_id;
            break;
        case object::Type::Exit:
            if ((!Single && FlagTag) || EolClient->battle_hides_exit()) {
                continue;
            }
            palette_id = Lgr->minimap_exit_palette_id;
            break;
        default:
            continue;
        }

        render_minimap_icon(minimap_view, obj->minimap_canvas_x - corner_x,
                            obj->minimap_canvas_y - corner_y, palette_id);
    }

    // Select the correct color for each bike
    unsigned char bike1_id = Lgr->minimap_bike1_palette_id;
    unsigned char bike2_id = Lgr->minimap_bike2_palette_id;
    if ((State->player1_bike1 && !player1) || (!State->player1_bike1 && player1)) {
        bike1_id = Lgr->minimap_bike2_palette_id;
        bike2_id = Lgr->minimap_bike1_palette_id;
    }

    if (EolSettings->show_others()) {
        for (const kuski& ku : EolClient->kuskis()) {
            const spy_data* k = ku.spy_data();
            if (!k) {
                continue;
            }

            vect2 k_pos = k->mot.bike.r - bottomleft_corner;
            int k_x = (int)(k_pos.x * MetersToMinimapPixels);
            int k_y = (int)(k_pos.y * MetersToMinimapPixels);
            render_minimap_icon(minimap_view, k_x, k_y, bike2_id);
        }
    }

    // Draw the other bike
    if (other_motor) {
        vect2 other_pos = other_motor->bike.r - bottomleft_corner;
        int other_x = (int)(other_pos.x * MetersToMinimapPixels);
        int other_y = (int)(other_pos.y * MetersToMinimapPixels);
        render_minimap_icon(minimap_view, other_x, other_y, bike2_id);
    }

    // Draw the current player's bike
    int bike_x = (int)(camera_pos.x * MetersToMinimapPixels);
    int bike_y = (int)(camera_pos.y * MetersToMinimapPixels);
    render_minimap_icon(minimap_view, bike_x, bike_y, bike1_id);
}
