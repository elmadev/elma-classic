#include "KIRAJZOL.H"
#include "abc8.h"
#include "affine_pic.h"
#include "anim.h"
#include "affine_pic_render.h"
#include "canvas.h"
#include "EDITUJ.H"
#include "eol/console.h"
#include "eol/eol.h"
#include "eol/status_messages.h"
#include "eol_settings.h"
#include "flagtag.h"
#include "LEJATSZO.H"
#include "level.h"
#include "lgr.h"
#include "M_PIC.H"
#include "main.h"
#include "object.h"
#include "pic8.h"
#include "platform/implementation.h"
#include "platform/utils.h"
#include "physics_init.h"
#include "renderer/object_overlay.h"
#include "timer.h"
#include "util/util.h"
#include <algorithm>
#include <cmath>
#include <cstring>
#include <string>
#include <format>
#include <filesystem>

int GameBackgroundRender = 0;

static abc8* SmallFont = nullptr;
static abc8* MediumFont = nullptr;

// Percentage of the screen used to render the game (QFRAME drawn on the edge)
static double VisibleFraction = 1.0;
constexpr double VISIBLE_FRACTION_SCALING_FACTOR = 1.1;

void increase_view_size() {
    VisibleFraction *= VISIBLE_FRACTION_SCALING_FACTOR;
    if (VisibleFraction >= 0.999) {
        VisibleFraction = 1.0;
    }
    GameBackgroundRender = GAME_BACKGROUND_RENDER_SET_VALUE;
}

void decrease_view_size() {
    VisibleFraction /= VISIBLE_FRACTION_SCALING_FACTOR;
    if (VisibleFraction < 0.7) {
        VisibleFraction = 0.7;
    }
    GameBackgroundRender = GAME_BACKGROUND_RENDER_SET_VALUE;
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
static int MinimapWidth;
static int MinimapHeight;
static int MinimapX;
static int MinimapDx;

affine_pic* shirt = nullptr;

static void init_shirt() {
    constexpr int SHIRT_BMP_WIDTH = 149;
    constexpr int SHIRT_BMP_HEIGHT = 101;

    pic8* pic_shirt = pic8::from_bmp("bmp/shirt.bmp");
    if (pic_shirt && pic_shirt->get_width() == SHIRT_BMP_WIDTH &&
        pic_shirt->get_height() == SHIRT_BMP_HEIGHT) {
        shirt = new affine_pic(nullptr, pic_shirt);
        return;
    }

    delete pic_shirt;
}

void init_renderer() {
    init_shirt();
    init_gravity_arrows();

    SmallFont = new abc8("small.abc");
    SmallFont->set_spacing(1);
    SmallFont->set_line_height(12);

    MediumFont = new abc8("medium.abc");
    MediumFont->set_spacing(1);

    Console = new console();
    Console->register_console_commands();

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

// Render an affine_pic (remember all affine_pic images are loaded sideways in the lgr)
// All units are in meters
// a = coordinate of middle left of affine_pic position (distal end of the limb)
// b = coordinate of middle right of affine_pic position (proximal end of the limb)
// Along the axis of the vector b->a, displace coordinate a by `a_stretch` meters
// Along the axis of the vector a->b, displace coordinate b by `b_stretch` meters
// height represents the vertical length of the affine_pic (thickness of the limb)
static void render_affine_pic(vect2 a, vect2 b, pic8* dest, double height, affine_pic* affine,
                              double a_stretch, double b_stretch, bool flip) {
    vect2 i = unit_vector(b - a);
    b = b + i * b_stretch;
    a = a - i * a_stretch;
    vect2 u = b - a;
    vect2 v;
    if (flip) {
        v = rotate_90deg(i) * height;
    } else {
        v = rotate_minus90deg(i) * height;
    }
    a = a - v;

    a.x *= MetersToPixels;
    a.y *= MetersToPixels;
    u.x *= MetersToPixels;
    u.y *= MetersToPixels;
    v.x *= MetersToPixels;
    v.y *= MetersToPixels;
    draw_affine_pic(dest, affine, u, v * 2.0, a);
}

// Render a wheel or head affine_pic
static void render_rigidbody(vect2 r, double radius, double rotation, pic8* dest,
                             affine_pic* affine, bool flip) {
    vect2 direction(cos(rotation) * radius, sin(rotation) * radius);
    if (flip) {
        render_affine_pic(r + direction, r - direction, dest, radius, affine, 0.0, 0.0, flip);
    } else {
        render_affine_pic(r - direction, r + direction, dest, radius, affine, 0.0, 0.0, flip);
    }
}

static double BikeFrameX;
static double BikeFrameY;
static vect2 BikeFrameI;
static vect2 BikeFrameJ;
static vect2 BikeFrameR;
static vect2 BikeFrameIPixels;
static vect2 BikeFrameJPixels;
static vect2 BikeFrameRPixels;

// Render a bike frame fragment
static void render_bike_part(pic8* dest, affine_pic* part, bike_box* box) {
    vect2 r = BikeFrameIPixels * (box->x1 + 260 - BikeFrameX) +
              BikeFrameJPixels * (BikeFrameY - (box->y1 + 260)) + BikeFrameRPixels;
    vect2 u = BikeFrameIPixels * (box->x2 - box->x1);
    vect2 v = BikeFrameJPixels * (box->y1 - box->y2);

    draw_affine_pic(dest, part, u, v, r);
}

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
static void render_minimap(bool player1, pic8* pic, double camera_turn_phase, vect2 bike_center,
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

    const int minimap_x1 = (int)(MinimapX + align * MinimapDx);
    const int minimap_x2 = minimap_x1 + MinimapWidth - 1;
    const int minimap_y1 = 1;
    const int minimap_y2 = minimap_y1 + MinimapHeight - 1;
    static pic8 minimap_view = pic8();
    minimap_view.subview(minimap_x1, minimap_y1, minimap_x2, minimap_y2, pic);

    const int border_x1 = minimap_x1 - 1;
    const int border_x2 = minimap_x2 + 1;
    const int border_y1 = minimap_y1 - 1;
    const int border_y2 = minimap_y2 + 1;
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
    for (int i = 0; i < MAX_OBJECTS; i++) {
        object* obj = Ptop->objects[i];
        if (!obj) {
            break;
        }

        unsigned char palette_id;
        switch (obj->type) {
        case object::Type::Food:
            if (!obj->active) {
                continue;
            }
            palette_id = Lgr->minimap_food_palette_id;
            break;
        case object::Type::Exit:
            if (!Single && Tag) {
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

static void handle_screenshot(pic8* pic) {
    if (VideoRecordingMode) {
        std::string filename = std::format("snp{:05}.pcx", VideoFrameIndex);
        std::filesystem::path path = std::filesystem::path(VideoOutputDirectory) / filename;
        pic->vertical_flip();
        pic->save(path.string().c_str(), Lgr->palette_data);
        pic->vertical_flip();
        return;
    }

    if (Legkozment) {
        Legkozment = 0;
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

// Render an entire bike + kuski
static void render_bike(bool player1, pic8* pic, double time, vect2 bottomleft_corner, motorst* mot,
                        valtozok* metadata, bike_pics* bike, affine_pic* shirt) {
    double arm_position = metadata->ugrasnagysag;
    double turn_phase = metadata->baljobbv_f.forgas;

    // Check to see if bike is turning, and calculate the progress from -1.0 to 1.0 using cos
    bool is_turning = false;
    StretchEnabled = false;
    if (turn_phase < 0.999) {
        is_turning = true;
        turn_phase = -cos(turn_phase * PI);
    }

    // Calculate wheel position relative to screen
    vect2 left_wheel_r = (mot->left_wheel.r - bottomleft_corner);
    vect2 right_wheel_r = (mot->right_wheel.r - bottomleft_corner);

    // If turning, we will be rendering one wheel in the foreground (usually they are rendered in
    // background)
    bool left_wheel_in_back = true;
    bool right_wheel_in_back = true;
    if (is_turning) {
        if ((turn_phase > 0.0 && !mot->flipped_bike) || (turn_phase <= 0.0 && mot->flipped_bike)) {
            left_wheel_in_back = false;
        } else {
            right_wheel_in_back = false;
        }
    }

    // Render background wheels
    if (left_wheel_in_back) {
        render_rigidbody(left_wheel_r, WheelBackgroundRenderRadius, mot->left_wheel.rotation, pic,
                         bike->wheel, false);
    }
    if (right_wheel_in_back) {
        render_rigidbody(right_wheel_r, WheelBackgroundRenderRadius, mot->right_wheel.rotation, pic,
                         bike->wheel, false);
    }

    // Get the bike position and angle
    vect2 bike_r = mot->bike.r - bottomleft_corner;
    vect2 bike_i = vect2(cos(mot->bike.rotation), sin(mot->bike.rotation));
    vect2 bike_j = rotate_90deg(bike_i);

    // If bike is turning, squish the bike
    if (is_turning) {
        StretchEnabled = true;
        set_stretch_parameters(bike_r, bike_i, turn_phase, MetersToPixels);
    }

    // If the bike is turned, flip the bike
    // Swap the wheels temporarily for the purposes of drawing the suspension
    if (mot->flipped_bike) {
        bike_i = Vect2null - bike_i;
        std::swap(left_wheel_r, right_wheel_r);
    }

    // Bike frame calculations. Rotate the bike frame by 0.62 radians
    BikeFrameX = 390.0;
    BikeFrameY = 420.0;
    constexpr double BIKE_FRAME_ROTATION = 0.62;
    constexpr double BIKE_FRAME_WIDTH = 0.0045;
    BikeFrameI = bike_i * (BIKE_FRAME_WIDTH * cos(BIKE_FRAME_ROTATION)) +
                 bike_j * (BIKE_FRAME_WIDTH * sin(BIKE_FRAME_ROTATION));
    BikeFrameJ = rotate_90deg(BikeFrameI);
    if (mot->flipped_bike) {
        BikeFrameJ = Vect2null - BikeFrameJ;
    }
    BikeFrameR = bike_r;

    // Convert bike position from meters to pixels
    BikeFrameIPixels = BikeFrameI * MetersToPixels;
    BikeFrameJPixels = BikeFrameJ * MetersToPixels;
    BikeFrameRPixels = BikeFrameR * MetersToPixels;

    // Draw susp1
    vect2 susp1_r =
        BikeFrameI * (365.0 - BikeFrameX) + BikeFrameJ * (BikeFrameY - 292.0) + BikeFrameR;
    render_affine_pic(left_wheel_r, susp1_r, pic, 0.06, bike->susp1, 0.05, 0.03, false);

    // Draw susp2
    vect2 susp2_r =
        BikeFrameI * (370.0 - BikeFrameX) + BikeFrameJ * (BikeFrameY - 520.0) + BikeFrameR;
    render_affine_pic(susp2_r, right_wheel_r, pic, 0.06, bike->susp2, 0.0, 0.1, false);

    // Draw flagtag flag
    bool draw_flag = false;
    if (!Single && Tag) {
        if ((player1 && FlagTagAHasFlag) || (!player1 && !FlagTagAHasFlag)) {
            // Current player has flag
            draw_flag = true;
        } else if (FlagTagImmunity) {
            // Other player has flag, but current player just lost the flag
            // Blink the flag away while immunity applies
            int blinking_flag_phase = (int)(time * 30.0);
            if (blinking_flag_phase % 2) {
                draw_flag = true;
            }
        }
    }
    if (draw_flag) {
        vect2 flag_base_r = BikeFrameI * (500.0 + 107 - BikeFrameX) +
                            BikeFrameJ * (BikeFrameY + 114 - 600.0) + BikeFrameR;
        vect2 flag_tip_r = flag_base_r + (BikeFrameI * 356.0 + BikeFrameJ * 500.0) * 0.2;
        render_affine_pic(flag_base_r, flag_tip_r, pic, 0.2, Lgr->flag, 0.0, 0.0,
                          mot->flipped_bike);
    }

    // Draw bike frame
    render_bike_part(pic, bike->bike_part1, &BikeBox1);
    render_bike_part(pic, bike->bike_part2, &BikeBox2);
    render_bike_part(pic, bike->bike_part3, &BikeBox3);
    render_bike_part(pic, bike->bike_part4, &BikeBox4);

    // Calculations to draw the kuski
    vect2 body_r = (mot->body_r - bottomleft_corner);
    vect2 hip_r = body_r + BikeFrameI * 75.0 + BikeFrameJ * (-47.0);
    vect2 shoulder_r = body_r + BikeFrameI * 47.0 + BikeFrameJ * 65.0;
    vect2 neck_r = body_r + BikeFrameI * 41.0 + BikeFrameJ * 70.0;
    vect2 foot_r =
        BikeFrameI * (346.0 - BikeFrameX) + BikeFrameJ * (BikeFrameY - 514.0) + BikeFrameR;

    // Calculate how to bend the knee based on the hip and foot positions
    vect2 knee_r;
    constexpr double THIGH_LENGTH = 0.51;
    constexpr double LEG_LENGTH = 0.51;
    if (mot->flipped_bike) {
        knee_r = circles_intersection(hip_r, foot_r, THIGH_LENGTH, LEG_LENGTH);
    } else {
        knee_r = circles_intersection(foot_r, hip_r, LEG_LENGTH, THIGH_LENGTH);
    }

    // Draw head
    render_rigidbody(mot->head_r - bottomleft_corner, HeadRadius, mot->bike.rotation, pic,
                     bike->head, mot->flipped_bike);

    // Hand is located on the handlebars, unless we are volting
    vect2 hand_r = susp1_r;
    if (arm_position > 0.0001) {
        // Invert the arm volt percentage to progress from 0->1
        arm_position = 1.0 - arm_position;
        // Left volt + facing left OR right volt + facing left -> Arm goes up
        bool arm_goes_up = true;
        if ((metadata->ugras1volt && !mot->flipped_bike) ||
            (!metadata->ugras1volt && mot->flipped_bike)) {
            // Right volt + facing left OR left volt + facing right -> arm goes down
            arm_goes_up = false;
        }

        // Describe the arm movement for up and down movements
        const double arm_apex_time = arm_goes_up ? 0.25 : 0.2;    // 0.0 to 1.0
        const double max_arm_rotation = arm_goes_up ? 2.7 : -1.6; // radians
        const double max_arm_stretch = arm_goes_up ? -0.3 : 0.15; // meters

        // Calculate arm movement progression away from neutral (0.0 to 1.0)
        double interpolation;
        if (arm_position < arm_apex_time) {
            interpolation = arm_position / arm_apex_time;
        } else {
            interpolation = 1.0 - (arm_position - arm_apex_time) / (1.0 - arm_apex_time);
        }

        // Calculate arm rotation and stretch
        double arm_rotation = max_arm_rotation * interpolation;
        double arm_stretch = max_arm_stretch * interpolation + 1.0;

        // Update hand position based on arm rotation and stretch
        vect2 arm_vector = hand_r - shoulder_r;
        if (!mot->flipped_bike) {
            arm_vector.rotate(-arm_rotation);
        } else {
            arm_vector.rotate(arm_rotation);
        }
        arm_vector = arm_vector * arm_stretch;
        hand_r = shoulder_r + arm_vector;
    }

    // Calculate how to bend the elbow based on shoulder and hand position
    constexpr double FORARM_LENGTH = 0.308 * 1.05;
    constexpr double UP_ARM_LENGTH = 0.328 * 1.05;
    vect2 elbow_r;
    if (mot->flipped_bike) {
        elbow_r = circles_intersection(hand_r, shoulder_r, FORARM_LENGTH, UP_ARM_LENGTH);
    } else {
        elbow_r = circles_intersection(shoulder_r, hand_r, UP_ARM_LENGTH, FORARM_LENGTH);
    }

    // Draw the whole kuski (excluding head)
    render_affine_pic(knee_r, hip_r, pic, 0.14, bike->thigh, 0.03, 0.1, mot->flipped_bike);
    render_affine_pic(foot_r, knee_r, pic, 0.21, bike->leg, 0.03, 0.03, mot->flipped_bike);
    affine_pic* body = shirt ? shirt : bike->body;
    render_affine_pic(hip_r, neck_r, pic, 0.2, body, 0.1, 0.05, mot->flipped_bike);
    render_affine_pic(elbow_r, shoulder_r, pic, 0.11, bike->up_arm, 0.08, 0.1, !mot->flipped_bike);
    render_affine_pic(hand_r, elbow_r, pic, 0.076, bike->forarm, 0.08, 0.1, mot->flipped_bike);

    // Draw foreground wheels
    StretchEnabled = false;
    if (!left_wheel_in_back || !right_wheel_in_back) {
        if (mot->flipped_bike) {
            // If we had temporarily inverted the wheels earlier in this function, undo that now
            std::swap(left_wheel_r, right_wheel_r);
        }
        if (!left_wheel_in_back) {
            render_rigidbody(left_wheel_r, mot->left_wheel.radius, mot->left_wheel.rotation, pic,
                             bike->wheel, false);
        }
        if (!right_wheel_in_back) {
            render_rigidbody(right_wheel_r, mot->right_wheel.radius, mot->right_wheel.rotation, pic,
                             bike->wheel, false);
        }
    }
}

// Render the view for one player
static void render_view(bool player1, pic8* pic, double time, motorst* mot, valtozok* metadata,
                        bool show_minimap, bool show_timer, motorst* other_mot,
                        valtozok* other_metadata, camera& current_camera) {
    // Calculate frame of reference
    vect2 bike_center = mot->bike.r;
    if (current_camera.mode == CameraMode::MapViewer) {
        bike_center = vect2(current_camera.x, current_camera.y);
    }

    vect2 bottomleft_corner(bike_center.x - (CameraX + metadata->baljobbv_h.baljobb * CameraDx),
                            bike_center.y - CameraY);

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
        object* obj = Ptop->objects[i];
        if (!obj) {
            break;
        }

        if (obj->type == object::Type::Start) {
            continue;
        }
        if (obj->type == object::Type::Food && !obj->active) {
            continue;
        }
        if (obj->type == object::Type::Exit && !Single && Tag) {
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

    if (current_camera.mode == CameraMode::Normal) {
        if (!Single) {
            // Draw the other bike if it's on-screen
            vect2 center(bottomleft_corner.x + (SCREEN_WIDTH / 2.0) * PixelsToMeters,
                         bottomleft_corner.y + (SCREEN_HEIGHT / 2.0) * PixelsToMeters);
            double distance = (other_mot->bike.r - center).length();
            if (distance < (std::max(SCREEN_WIDTH, SCREEN_HEIGHT) * 27.0 / 32.0) * PixelsToMeters) {
                render_bike(!player1, pic, time, bottomleft_corner, other_mot, other_metadata,
                            bike2, nullptr);
            }
        }

        // Draw the current player's bike
        render_bike(player1, pic, time, bottomleft_corner, mot, metadata, bike1, shirt);
    }

    // Draw the foreground
    if (!EolSettings->pictures_in_background()) {
        CanvasFront->render(player1, pic, bottomleft_corner, 0, 0, GameViewWidth - 1,
                            GameViewHeight - 1);
    }

    // Draw the minimap
    if (show_minimap) {
        if (Single) {
            render_minimap(player1, pic, metadata->baljobbv_h.baljobb, bike_center, nullptr);
        } else {
            render_minimap(player1, pic, metadata->baljobbv_h.baljobb, bike_center, other_mot);
        }
    }

    // Draw the timers
    if (show_timer) {
        double flagtag_time = -1.0;
        if (!Single && Tag) {
            flagtag_time = player1 ? FlagTimeA : FlagTimeB;
        }
        draw_timers(BestTime, flagtag_time, time, pic, GameViewWidth, GameViewHeight);
    }

    // Draw the bottom-right info panel
    if (mot->apple_count && EolSettings->show_last_apple_time()) {
        char tmp[100];
        sprintf(tmp, "last apple (%d)        ", mot->apple_count - mot->apple_bug_count);
        util::text::centiseconds_to_string(mot->last_apple_time, tmp + strlen(tmp), true, true);

        constexpr int RIGHT_MARGIN = 5;
        constexpr int BOTTOM_MARGIN = 5;
        int x = GameViewWidth - RIGHT_MARGIN;
        int y = BOTTOM_MARGIN;
        SmallFont->write_right_align(pic, x, y, tmp);
    }
}

void render_game(double time, valtozok* metadata1, valtozok* metadata2, bool show_minimap1,
                 bool show_timer1, bool show_minimap2, bool show_timer2, camera& current_camera) {
    // Determine who we are going to draw (player 1, player 2 or both)
    bool draw_player1 = metadata1->showkep;
    bool draw_player2 = metadata2->showkep;
    if (Single || current_camera.mode == CameraMode::MapViewer) {
        draw_player1 = true;
        draw_player2 = false;
    }
    if (!draw_player1 && !draw_player2) {
        internal_error("render_game nobody visible!");
    }
    bool splitscreen = draw_player1 && draw_player2;

    // Get the screen, upside-down
    pic8* pic = lockbackbuffer_pic();

    // If we need to recalculate the screen position, redraw the background qframe
    if (GameBackgroundRender > 0) {
        GameBackgroundRender--;
        calculate_viewpoints(splitscreen);
        render_background(pic);
    }

    // Draw 1 or 2 players
    static pic8 player_view = pic8();
    if (splitscreen) {
        player_view.subview(GameViewLeft, GameViewBottom1, GameViewRight, GameViewTop1, pic);
        render_view(true, &player_view, time, Motor1, metadata1, show_minimap1, show_timer1, Motor2,
                    metadata2, current_camera);

        player_view.subview(GameViewLeft, GameViewBottom2, GameViewRight, GameViewTop2, pic);
        render_view(false, &player_view, time, Motor2, metadata2, show_minimap2, show_timer2,
                    Motor1, metadata1, current_camera);
    } else {
        player_view.subview(GameViewLeft, GameViewBottom1, GameViewRight, GameViewTop1, pic);
        if (draw_player1) {
            render_view(true, &player_view, time, Motor1, metadata1, show_minimap1, show_timer1,
                        Motor2, metadata2, current_camera);
        } else {
            render_view(false, &player_view, time, Motor2, metadata2, show_minimap2, show_timer2,
                        Motor1, metadata1, current_camera);
        }
    }

    // Draw EOL overlays
    Console->render(*pic, *SmallFont);
    StatusMessages->render(*pic, *SmallFont);
    EolClient->render_table(*pic, *MediumFont, *SmallFont);

    // Conditionally save screenshot
    handle_screenshot(pic);

    unlockbackbuffer_pic();
}
