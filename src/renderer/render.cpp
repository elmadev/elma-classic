#include "renderer/render.h"
#include "editor/editor.h"
#include "eol/console.h"
#include "eol/eol.h"
#include "eol/settings.h"
#include "eol/status_messages.h"
#include "game/driver.h"
#include "game/fps.h"
#include "game/game.h"
#include "level/level.h"
#include "level/object.h"
#include "main.h"
#include "physics/flagtag.h"
#include "physics/init.h"
#include "physics/pacer.h"
#include "pic/abc8.h"
#include "pic/anim.h"
#include "pic/lgr.h"
#include "pic/pic8.h"
#include "pic/surface.h"
#include "platform/implementation.h"
#include "renderer/affine.h"
#include "renderer/canvas.h"
#include "renderer/object_overlay.h"
#include "renderer/timer.h"
#include "util/util.h"
#include <algorithm>
#include <cmath>
#include <cstring>
#include <filesystem>
#include <format>
#include <string>
#include <vector>

static bool GameBackgroundRender = false;

static abc8* SmallFont = nullptr;
static abc8* MediumFont = nullptr;
static abc8* LargeFont = nullptr;

// Percentage of the screen used to render the game (QFRAME drawn on the edge)
static double VisibleFraction = 1.0;
constexpr double VISIBLE_FRACTION_SCALING_FACTOR = 1.1;

void reset_game_background() { GameBackgroundRender = true; }

void increase_view_size() {
    VisibleFraction *= VISIBLE_FRACTION_SCALING_FACTOR;
    if (VisibleFraction >= 0.999) {
        VisibleFraction = 1.0;
    }
    reset_game_background();
}

void decrease_view_size() {
    VisibleFraction /= VISIBLE_FRACTION_SCALING_FACTOR;
    VisibleFraction = std::max(VisibleFraction, 0.7);
    reset_game_background();
}

// Position of the view of player1 and player2 in pixels on the screen
static int GameViewLeft;
static int GameViewBottom1;
static int GameViewRight;
static int GameViewTop1;
static int GameViewBottom2;
static int GameViewTop2;
int GameViewWidth;
int GameViewHeight;

// In meters from the bottom-left corner of screen
static double CameraY;
static double CameraX;
static double CameraDx;

// In pixels from the bottom-left corner of screen
double AffinePicScreenLeft;
double AffinePicScreenRight;
double AffinePicScreenBottom;
double AffinePicScreenTop;

// In pixels from the bottom-left corner of screen
int MinimapWidth;
int MinimapHeight;
int MinimapX;
int MinimapDx;

pic8* shirt = nullptr;

void init_renderer() {
    shirt = eol::load_shirt(EolSettings->nick());
    init_gravity_arrows();

    SmallFont = new abc8("small.abc", 1, 12);
    MediumFont = new abc8("medium.abc", 1, 18);
    LargeFont = new abc8("large.abc", 2, 90);

    Console = new console();
    Console->register_console_commands();
    Console->set_font(SmallFont);

    StatusMessages = new status_messages();
}

// Determine the position of the view of player1 +- player2 on the screen
static void calculate_viewpoints(bool splitscreen) {
    // Determine viewpoint of player1 and player2
    GameViewWidth = (int)(SCREEN_WIDTH * VisibleFraction);
    GameViewHeight = (int)(SCREEN_HEIGHT * VisibleFraction);
    GameViewLeft = (SCREEN_WIDTH - GameViewWidth) / 2;
    GameViewBottom1 = (SCREEN_HEIGHT - GameViewHeight) / 2;
    GameViewRight = GameViewLeft + GameViewWidth - 1;
    GameViewTop1 = GameViewBottom1 + GameViewHeight - 1;
    if (splitscreen) {
        GameViewHeight = (SCREEN_HEIGHT / 2) - 6;
        GameViewBottom1 = (SCREEN_HEIGHT / 2) + 6;
        GameViewTop1 = SCREEN_HEIGHT - 1;
        GameViewBottom2 = 0;
        GameViewTop2 = (SCREEN_HEIGHT / 2) - 7;
    }
#ifdef DEBUG
    if (GameViewRight >= SCREEN_WIDTH) {
        internal_error("calculate_viewpoints GameViewRight >= SCREEN_WIDTH!");
    }
    if (GameViewTop1 >= SCREEN_HEIGHT) {
        internal_error("calculate_viewpoints GameViewTop1 >= SCREEN_HEIGHT!");
    }
#endif

    // Determine boundary for the purposes of drawing affine pictures
    AffinePicScreenLeft = 2.0;
    AffinePicScreenBottom = 2.0;
    AffinePicScreenRight = GameViewWidth - 3.0;
    AffinePicScreenTop = GameViewHeight - 3.0;

    // Determine camera position in meters from the bottom-left corner
    if (EolSettings->center_camera()) {
        CameraX = (GameViewWidth / MetersToPixels) * 0.50;
    } else {
        CameraX = (SCREEN_WIDTH / MetersToPixels) * 0.15 * EolSettings->zoom();
    }
    CameraDx = GameViewWidth / MetersToPixels - 2.0 * CameraX;
    CameraY = GameViewHeight / MetersToPixels / 2.0;

    // Determine minimap position and size
    MinimapWidth =
        (int)(EolSettings->minimap_width() * sqrt(double(GameViewHeight) / double(SCREEN_HEIGHT)));
    MinimapHeight =
        (int)(EolSettings->minimap_height() * sqrt(double(GameViewHeight) / double(SCREEN_HEIGHT)));
    MinimapX = (int)(40.0 * (VisibleFraction - 0.6) / 0.4);
    MinimapDx = GameViewWidth - 2 * MinimapX - MinimapWidth;
}

static void handle_screenshot(pic8* pic) {
    if (VideoRecordingMode) {
        std::string filename = std::format("snp{:05}.pcx", VideoFrameIndex);
        std::filesystem::path path = std::filesystem::path(VideoOutputDirectory) / filename;
        pic->vertical_flip();
        pic->save(path.string().c_str(), Lgr->palette_data);
        pic->vertical_flip();
        return;
    }

    if (ScreenshotRequested) {
        ScreenshotRequested = false;
        platform_save_screenshot();
    }
}

// Cover the screen with qframe
static void render_background(pic8* pic) {
    for (int i = 0; i < pic->get_height(); i += Lgr->qframe->get_height()) {
        for (int j = 0; j < pic->get_width(); j += Lgr->qframe->get_width()) {
            blit8(pic, Lgr->qframe, j, i);
        }
    }
}

// Local (2-player) flag tag: does this player's bike currently show the flag?
static bool local_flag_tag_has_flag(bool player1, double time) {
    if (Single || !FlagTag) {
        return false;
    }
    if ((player1 && FlagTagAHasFlag) || (!player1 && !FlagTagAHasFlag)) {
        return true;
    }
    // Other player has flag, but this player just lost the flag
    // Blink the flag away while immunity applies
    return FlagTagImmunity && (int)(time * 30.0) % 2 != 0;
}

static bool bike_in_view(const motorst* mot, vect2 center) {
    double distance = (mot->bike.r - center).length();
    return distance < (std::max(SCREEN_WIDTH, SCREEN_HEIGHT) * 27.0 / 32.0) * PixelsToMeters;
}

struct info_panel_row {
    std::string label;
    std::string value;
};

// Render the bottom-right info panel: rows[0] is the bottom row, each later row stacks above it
// (the backbuffer is upside-down, so larger y is higher on screen).
static void render_info_panel(pic8* pic, const std::vector<info_panel_row>& rows) {
    constexpr int RIGHT_MARGIN = 10;
    constexpr int BOTTOM_MARGIN = 10;
    constexpr int LABEL_OFFSET = 180;
    constexpr int EXTRA_SPACE_PER_CHAR = 6;

    int max_value_length = 0;
    for (const info_panel_row& row : rows) {
        max_value_length = std::max(max_value_length, (int)row.value.size());
    }
    int label_offset = LABEL_OFFSET;
    if (max_value_length > 10) {
        label_offset += (max_value_length - 10) * EXTRA_SPACE_PER_CHAR;
    }

    int value_x = GameViewWidth - RIGHT_MARGIN;
    int label_x = GameViewWidth - label_offset;
    for (size_t i = 0; i < rows.size(); i++) {
        int y = BOTTOM_MARGIN + (int)i * SmallFont->line_height();
        SmallFont->write(pic, label_x, y, rows[i].label.c_str());
        SmallFont->write_right_align(pic, value_x, y, rows[i].value.c_str());
    }
}

// Render the view for one player
static void render_view(bool player1, bool bottom_player, pic8* pic, double time, driver& driv,
                        driver& other_driv, camera& current_camera, GameLoop loop) {
    // Calculate frame of reference
    vect2 bike_center = driv.mot->bike.r;
    if (current_camera.mode == CameraMode::MapViewer) {
        bike_center = vect2(current_camera.x, current_camera.y);
    }

    const kuski* spy_kuski = EolClient->spy_kuski();
    if (spy_kuski) {
        bike_center = spy_kuski->spy_data()->mot.bike.r;
    }

    vect2 bottomleft_corner(bike_center.x -
                                (CameraX + driv.meta.camera_turning.turn_phase * CameraDx),
                            bike_center.y - CameraY);
    vect2 center(bottomleft_corner.x + (SCREEN_WIDTH / 2.0) * PixelsToMeters,
                 bottomleft_corner.y + (SCREEN_HEIGHT / 2.0) * PixelsToMeters);

    // Draw the background
    CanvasBack->render(player1, pic, bottomleft_corner, 0, 0, GameViewWidth - 1,
                       GameViewHeight - 1);

    // Draw the objects
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
                obj_frame = Lgr->food[obj->animation % Lgr->food_count]->get_frame_by_time(time);
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

        blit8(pic, obj_frame, obj->canvas_x - corner_x, obj->canvas_y - corner_y + phase_y_offset);

        if (EolSettings->show_gravity_arrows() && obj->type == object::Type::Food &&
            obj->property != object::Property::None) {
            draw_gravity_arrow(pic, obj->canvas_x - corner_x,
                               obj->canvas_y - corner_y + phase_y_offset, obj->property);
        }
    }

    // Select the correct bike for each player
    bike_pics* bike1 = &Lgr->bike1;
    bike_pics* bike2 = &Lgr->bike2;
    if ((State->player1_bike1 && !player1) || (!State->player1_bike1 && player1)) {
        bike1 = &Lgr->bike2;
        bike2 = &Lgr->bike1;
    }

    if (EolSettings->show_others()) {
        for (const kuski& ku : EolClient->kuskis()) {
            if (&ku == spy_kuski) {
                continue;
            }
            const spy_data* k = ku.spy_data();
            if (!k) {
                continue;
            }

            if (bike_in_view(&k->mot, center)) {
                render_bike(pic, EolClient->kuski_has_flag(ku.id), bottomleft_corner, &k->mot,
                            &k->metadata, bike2, ku.shirt);
            }
        }
    }

    if (spy_kuski) {
        const spy_data* k = spy_kuski->spy_data();
        if (k && bike_in_view(&k->mot, center)) {
            render_bike(pic, EolClient->kuski_has_flag(spy_kuski->id), bottomleft_corner, &k->mot,
                        &k->metadata, bike2, spy_kuski->shirt);
        }
    }

    if (current_camera.mode == CameraMode::Normal) {
        if (!Single) {
            // Draw the other bike if it's on-screen
            if (bike_in_view(other_driv.mot, center)) {
                render_bike(pic, local_flag_tag_has_flag(!player1, time), bottomleft_corner,
                            other_driv.mot, &other_driv.meta, bike2, nullptr);
            }
        }

        // Draw the current player's bike
        render_bike(pic, local_flag_tag_has_flag(player1, time) || EolClient->own_bike_has_flag(),
                    bottomleft_corner, driv.mot, &driv.meta, bike1, shirt);
    }

    // Draw the foreground
    if (!EolSettings->pictures_in_background()) {
        CanvasFront->render(player1, pic, bottomleft_corner, 0, 0, GameViewWidth - 1,
                            GameViewHeight - 1);
    }

    // Draw the minimap
    if (driv.hud->minimap) {
        if (Single) {
            render_minimap(player1, pic, driv.meta.camera_turning.turn_phase, bike_center, nullptr);
        } else {
            render_minimap(player1, pic, driv.meta.camera_turning.turn_phase, bike_center,
                           other_driv.mot);
        }
    }

    // Draw the timers
    if (driv.hud->timer) {
        double flagtag_time = -1.0;
        if (!Single && FlagTag) {
            flagtag_time = player1 ? FlagTimeA : FlagTimeB;
        }
        double shown_time = time;
        if (Single && EolClient->is_spying()) {
            shown_time =
                spy_kuski && !EolClient->battle_hides_times()
                    ? spy_kuski->spy_data()->time * (STOPWATCH_MULTIPLIER * STOPWATCH_TO_PHYS_TIME)
                    : 0.0;
        }
        draw_timers(BestTime, flagtag_time, shown_time, pic, GameViewWidth, GameViewHeight);
    }

    // Build the bottom-right info panel rows.
    // rows are rendered in the order they were added (last added on top)
    std::vector<info_panel_row> info_rows;

    if (loop == GameLoop::Game) {
        if (current_camera.mode != CameraMode::MapViewer) {
            if (EolSettings->show_speedometer()) {
                info_rows.push_back({"max speed", driv.stats.format_max_speed()});
                info_rows.push_back({"speed", driv.stats.format_speed()});
            }

            if (EolSettings->show_one_wheel_status()) {
                info_rows.push_back({"one wheel", driv.mot->one_wheel_failed ? "no" : "yes"});
            }
        }

        if (bottom_player) {
            // FPS
            if (EolSettings->show_fps()) {
                info_rows.push_back({"FPS", fps::format_fps() + pacer::format_fps_limit()});
            }

            // UPS
            if (EolSettings->show_ups() && current_camera.mode == CameraMode::Normal) {
                info_rows.push_back({"UPS", fps::format_ups()});
            }
        }
    }

    // Apple count/time
    if (driv.mot->apple_count && EolSettings->show_last_apple_time()) {
        char apple_time[32];
        util::text::centiseconds_to_string(driv.mot->last_apple_time, apple_time, true, true);
        info_rows.push_back(
            {std::format("last apple ({})", driv.mot->apple_count - driv.mot->apple_bug_count),
             apple_time});
    }

    if (!EolClient->play_offline() && !EolClient->connected()) {
        MediumFont->write_right_align(
            pic, GameViewWidth - 10, GameViewHeight - MediumFont->line_height() * 2,
            std::format("Lost connection ({} to reconnect)", dik_to_string(State->key_reconnect))
                .c_str());
    }

    render_info_panel(pic, info_rows);
}

void render_game(double time, driver& driv1, driver& driv2, camera& current_camera, GameLoop loop) {
    reload_graphic_assets();

    fps::update();

    // Determine who we are going to draw (player 1, player 2 or both)
    bool draw_player1 = driv1.draw_view;
    bool draw_player2 = driv2.draw_view;
    if (Single || current_camera.mode == CameraMode::MapViewer) {
        draw_player1 = true;
        draw_player2 = false;
    }
    if (!draw_player1 && !draw_player2) {
        internal_error("render_game nobody visible!");
    }
    bool splitscreen = draw_player1 && draw_player2;

    // Get the screen, upside-down
    pic8* pic = lock_backbuffer_pic(true);

    // If we need to recalculate the screen position, redraw the background qframe
    if (GameBackgroundRender) {
        GameBackgroundRender = false;
        calculate_viewpoints(splitscreen);
        render_background(pic);
    }

    // Draw 1 or 2 players
    static pic8 player_view = pic8();
    if (splitscreen) {
        player_view.subview(GameViewLeft, GameViewBottom1, GameViewRight, GameViewTop1, pic);
        render_view(true, false, &player_view, time, driv1, driv2, current_camera, loop);

        player_view.subview(GameViewLeft, GameViewBottom2, GameViewRight, GameViewTop2, pic);
        render_view(false, true, &player_view, time, driv2, driv1, current_camera, loop);
    } else {
        player_view.subview(GameViewLeft, GameViewBottom1, GameViewRight, GameViewTop1, pic);
        if (draw_player1) {
            render_view(true, true, &player_view, time, driv1, driv2, current_camera, loop);
        } else {
            render_view(false, true, &player_view, time, driv2, driv1, current_camera, loop);
        }
    }

    // Draw EOL overlays
    Console->render(*pic);
    StatusMessages->render(*pic, *SmallFont);
    EolClient->render_table(*pic, *MediumFont, *SmallFont);
    EolClient->render_battle_status(*pic, *SmallFont);
    EolClient->render_battle_leader(*pic, *SmallFont);
    EolClient->render_battle_countdown(*pic, *LargeFont, *SmallFont);

    // Conditionally save screenshot
    handle_screenshot(pic);

    unlock_backbuffer_pic();
}
