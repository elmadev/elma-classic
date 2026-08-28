
#include "editor/editor.h"
#include "eol/eol.h"
#include "eol/settings.h"
#include "game/game.h"
#include "level/object.h"
#include "pic/anim.h"
#include "pic/surface.h"
#include "renderer/affine.h"
#include "renderer/canvas.h"
#include "renderer/object_overlay.h"
#include "renderer/render.h"
#include "renderer/timer.h"
#include <cmath>

/*
 * Renderer which draws to a pic8* buffer
 */

class PicRenderer : public GameRenderer {

  protected:
    // backbuffer
    pic8* pic_main;
    // Not a pointer because we own it
    pic8 pic_view;

    //
    const kuski* spy_kuski = nullptr;

    //
    void start_frame() override {
        spy_kuski = EolClient->spy_kuski();
        pic_main = lock_backbuffer_pic(true);
    }

    //
    void end_frame() override { unlock_backbuffer_pic(); }

    // Specify where game view is drawn
    void subview(int left, int bottom, int right, int top) override {
        pic_view.subview(left, bottom, right, top, pic_main);
    }

    // Cover the screen with qframe
    void render_qframe(bool scale_changed) override {
        if (scale_changed) {
            for (int i = 0; i < pic_main->get_height(); i += Lgr->qframe->get_height()) {
                for (int j = 0; j < pic_main->get_width(); j += Lgr->qframe->get_width()) {
                    blit8(pic_main, Lgr->qframe, j, i);
                }
            }
        }
    }

    // Draw sky
    void render_back(bool player1) override {
        CanvasBack->render(player1, &pic_view, bottomleft_corner, 0, 0, GameViewWidth - 1,
                           GameViewHeight - 1);
    }

    // Draw ground
    void render_front(bool player1) override {
        CanvasFront->render(player1, &pic_view, bottomleft_corner, 0, 0, GameViewWidth - 1,
                            GameViewHeight - 1);
    }

    //
    void render_objects() override {
        int corner_x;
        int corner_y;
        CanvasBack->meters_to_pixels(bottomleft_corner, &corner_x, &corner_y);
        int object_border_left = corner_x - (int)(ANIM_WIDTH * EolSettings->zoom()) - 2;
        int object_border_bottom = corner_y - (int)(ANIM_WIDTH * EolSettings->zoom()) - 2;
        int object_border_right = corner_x + SCREEN_WIDTH;
        int object_border_top = corner_y + SCREEN_HEIGHT;
        for (int i = 0; i < MAX_OBJECTS; i++) {
            object* obj = Level->objects[i];
            if (!obj) {
                break;
            }

            if (obj->type == object::Type::Start) {
                continue;
            }
            if (obj->type == object::Type::Food &&
                (!obj->active || (spy_kuski && spy_kuski->apples_taken[i]))) {
                continue;
            }
            if (obj->type == object::Type::Exit &&
                ((!Single && FlagTag) || EolClient->battle_hides_exit())) {
                continue;
            }

            if (obj->canvas_x < object_border_left || obj->canvas_y < object_border_bottom ||
                obj->canvas_x > object_border_right || obj->canvas_y > object_border_top) {
                continue;
            }

            pic8* obj_frame = nullptr;
            int phase_y_offset = 0;
            if (State->animated_objects) {
                switch (obj->type) {
                case object::Type::Food:
                    obj_frame =
                        Lgr->food[obj->animation % Lgr->food_count]->get_frame_by_time(time);
                    phase_y_offset =
                        (int)(5.0 * EolSettings->zoom() * sin(time * 15.5 + obj->floating_phase));
                    break;
                case object::Type::Exit:
                    obj_frame = Lgr->exit->get_frame_by_time(time);
                    phase_y_offset =
                        (int)(5.0 * EolSettings->zoom() * sin(time * 15.5 + obj->floating_phase));
                    break;
                case object::Type::Killer:
                    obj_frame = Lgr->killer->get_frame_by_time(time);
                    break;
                default:
                    internal_error("render_view invalid object type");
                }

                if (EolSettings->still_objects()) {
                    phase_y_offset = 0;
                }
            } else {
                switch (obj->type) {
                case object::Type::Food:
                    obj_frame = Lgr->food[obj->animation % Lgr->food_count]->get_frame_by_index(0);
                    break;
                case object::Type::Exit:
                    obj_frame = Lgr->exit->get_frame_by_index(0);
                    break;
                case object::Type::Killer:
                    obj_frame = Lgr->killer->get_frame_by_index(0);
                    break;
                default:
                    internal_error("render_view invalid object type");
                }
            }

            blit8(&pic_view, obj_frame, obj->canvas_x - corner_x,
                  obj->canvas_y - corner_y + phase_y_offset);

            if (EolSettings->show_gravity_arrows() && obj->type == object::Type::Food &&
                obj->property != object::Property::None) {
                draw_gravity_arrow(&pic_view, obj->canvas_x - corner_x,
                                   obj->canvas_y - corner_y + phase_y_offset, obj->property);
            }
        }
    }

    //
    void render_minimap(bool player1, const motorst* other_motor, int x1, int y1, int x2, int y2,
                        vect2 bottomleft_corner, vect2 camera_pos) override {

        const int border_x1 = x1 - 1;
        const int border_x2 = x2 + 1;
        const int border_y1 = y1 - 1;
        const int border_y2 = y2 + 1;

        if (border_x1 < 0 || border_y1 < 0 || border_x2 >= pic_view.get_width() ||
            border_y2 >= pic_view.get_height()) {
            // Minimap doesn't fit on the screen, so skip drawing it entirely
            return;
        }

        static pic8 minimap_view = pic8();
        minimap_view.subview(x1, y1, x2, y2, &pic_view);
        static pic8 border_view = pic8();
        border_view.subview(border_x1, border_y1, border_x2, border_y2, &pic_view);

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

        // Draw the minimap
        render_minimap_subview(player1, &minimap_view, other_motor, bottomleft_corner, camera_pos);

        // Bring back pixels from the saved game scene based on opacity
        if (opacity < 100) {
            blit8_dither(&border_view, save_pic, 0, 0, opacity);
        }
    };

    //
    void render_timers(const char* best_time_text, double flag_tag_time, int dest_width,
                       int dest_height) override {
        double shown_time = time;
        if (Single && EolClient->is_spying()) {
            shown_time =
                spy_kuski && !EolClient->battle_hides_times()
                    ? spy_kuski->spy_data()->time * (STOPWATCH_MULTIPLIER * STOPWATCH_TO_PHYS_TIME)
                    : 0.0;
        }
        draw_timers(best_time_text, flag_tag_time, shown_time, &pic_view, dest_width, dest_height);
    };

    // Get the backbuffer (whole frame) picture
    pic8* get_backbuffer_pic() override { return pic_main; }

    // Draw bike / kuski component
    void bike_draw_affine_pic(const pic8* affine, unsigned char transparency, vect2 u, vect2 v,
                              vect2 r) override {

        apply_stretch_parameters(u, v, r);

        r = r - bottomleft_corner;

        u.x *= MetersToPixels;
        u.y *= MetersToPixels;
        v.x *= MetersToPixels;
        v.y *= MetersToPixels;
        r.x *= MetersToPixels;
        r.y *= MetersToPixels;

        draw_affine_pic(&pic_view, affine, transparency, u, v, r);
    }

  public:
    // Include parent methods
    using GameRenderer::GameRenderer;
};

/*
 * Top level definitions other than PicRenderer conditional so that
 * this file can be #included to subclass PicRenderer.
 * Just avoids an additional .h and double definitions of all the methods.
 */

#ifndef RENDER_PIC_IS_INCLUDE
std::unique_ptr<GameRenderer> createPicRenderer(double time, driver& driv1, driver& driv2,
                                                camera& current_camera, GameLoop loop) {
    return std::make_unique<PicRenderer>(time, driv1, driv2, current_camera, loop);
}
#endif
