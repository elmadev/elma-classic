#include "editor/editor.h"
#include "eol/eol.h"
#include "game/driver.h"
#include "game/game.h"
#include "level/object.h"
#include "pic/pic8.h"
#include "renderer/canvas.h"
#include "renderer/render.h"

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

// Render the entire minimap
void render_minimap(bool player1, pic8* pic, double camera_turn_phase, vect2 bike_center,
                    motorst* other_motor) {
    // Calculate minimap size and minimap frame of reference
    double minimap_width = MinimapWidth * MinimapScaleFactor * PixelsToMeters;
    double minimap_height = MinimapHeight * MinimapScaleFactor * PixelsToMeters;

    double camera_x = EolSettings->center_map() ? 0.5 : 0.2;
    double camera_dx = 1.0 - 2.0 * camera_x;
    vect2 camera_pos(minimap_width * (camera_x + camera_turn_phase * camera_dx),
                     minimap_height / 2);
    vect2 bottomleft_corner = bike_center - camera_pos;

    double align;
    switch (EolSettings->map_alignment()) {
    case MapAlignment::None:
        align = camera_turn_phase;
        break;
    case MapAlignment::Left:
        align = 0.0;
        break;
    case MapAlignment::Middle:
        align = 0.5;
        break;
    case MapAlignment::Right:
        align = 1.0;
        break;
    }

    const int minimap_x1 = std::max(1, (int)(MinimapX + align * MinimapDx));
    const int minimap_x2 = minimap_x1 + MinimapWidth - 1;
    const int minimap_y1 = 1;
    const int minimap_y2 = minimap_y1 + MinimapHeight - 1;

    const int border_x1 = minimap_x1 - 1;
    const int border_x2 = minimap_x2 + 1;
    const int border_y1 = minimap_y1 - 1;
    const int border_y2 = minimap_y2 + 1;

    if (border_x1 < 0 || border_y1 < 0 || border_x2 >= pic->get_width() ||
        border_y2 >= pic->get_height()) {
        // Minimap doesn't fit on the screen, so skip drawing it entirely
        return;
    }

    static pic8 minimap_view = pic8();
    minimap_view.subview(minimap_x1, minimap_y1, minimap_x2, minimap_y2, pic);
    static pic8 border_view = pic8();
    border_view.subview(border_x1, border_y1, border_x2, border_y2, pic);

    // Save game scene pixels under the minimap area (including 1px border margin)
    int opacity = EolSettings->minimap_opacity();
    static pic8* save_pic = nullptr;
    if (opacity < 100) {
        if (!save_pic || save_pic->get_width() != border_view.get_width() ||
            save_pic->get_height() != border_view.get_height()) {
            delete save_pic;
            save_pic = new pic8(border_view.get_width(), border_view.get_height());
        }
        blit8(save_pic, &border_view);
    }

    // Draw the minimap border
    border_view.fill_box(Lgr->minimap_border_palette_id);

    // Draw the background (polygons)
    CanvasMinimap->render_minimap(player1, &minimap_view, bottomleft_corner, 0, 0, MinimapWidth - 1,
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

        render_minimap_icon(&minimap_view, obj->minimap_canvas_x - corner_x,
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
            render_minimap_icon(&minimap_view, k_x, k_y, bike2_id);
        }
    }

    // Draw the other bike
    if (other_motor) {
        vect2 other_pos = other_motor->bike.r - bottomleft_corner;
        int other_x = (int)(other_pos.x * MetersToMinimapPixels);
        int other_y = (int)(other_pos.y * MetersToMinimapPixels);
        render_minimap_icon(&minimap_view, other_x, other_y, bike2_id);
    }

    // Draw the current player's bike
    int bike_x = (int)(camera_pos.x * MetersToMinimapPixels);
    int bike_y = (int)(camera_pos.y * MetersToMinimapPixels);
    render_minimap_icon(&minimap_view, bike_x, bike_y, bike1_id);

    // Bring back pixels from the saved game scene based on opacity
    if (opacity < 100) {
        blit8_dither(&border_view, save_pic, 0, 0, opacity);
    }
}
