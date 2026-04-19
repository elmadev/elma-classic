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

// Ezek mindig beallitodnak Kepmeret-bol kirajzol320-ban:

int GameBackgroundRender = 0;
int GAME_BACKGROUND_RENDER_SET_VALUE = 10;

static abc8* SmallFont = nullptr;
static abc8* MediumFont = nullptr;

static double VisibleFraction = 1.0;

static double VISIBLE_FRACTION_SCALING_FACTOR = 1.1;
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

// Kirajzolando terulet nagysaga:
static int GameViewLeft = 0, GameViewBottom1 = 0, GameViewRight = SCREEN_WIDTH - 1,
           GameViewTop1 = SCREEN_HEIGHT - 1, GameViewBottom2 = 0, GameViewTop2 = 0;
int GameViewWidth = SCREEN_WIDTH, GameViewHeight = SCREEN_HEIGHT;

// Motoros kepernyo mely reszen helyezkedik el:
static double CameraY = 0.0, CameraX = 0.0, CameraDx = 0.0;

double AffinePicScreenLeft, AffinePicScreenRight, AffinePicScreenBottom, AffinePicScreenTop;

// View ablak meretei:
static int MinimapWidth, MinimapHeight;
static int MinimapX, MinimapDx;

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

void calculate_viewpoints(int splitscreen) {
    GameViewBottom2 = 100, GameViewTop2 = 200;

    GameViewWidth = (int)(SCREEN_WIDTH * VisibleFraction);
    GameViewHeight = (int)(SCREEN_HEIGHT * VisibleFraction);
    GameViewLeft = (SCREEN_WIDTH - GameViewWidth) / 2;
    GameViewBottom1 = (SCREEN_HEIGHT - GameViewHeight) / 2;
    GameViewRight = GameViewLeft + GameViewWidth - 1;
    GameViewTop1 = GameViewBottom1 + GameViewHeight - 1;
    if (splitscreen) {
        GameViewHeight = (SCREEN_HEIGHT / 2) - 6;
        GameViewBottom1 =
            (SCREEN_HEIGHT / 2) + 6;      // Mivel kep fejjel lefele van, forditva vannak koordok
        GameViewTop1 = SCREEN_HEIGHT - 1; // Vagyis A player van felul, B pedig lent
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

    // Ezek csak doboz kirajzolashoz kellenek:
    AffinePicScreenLeft = 2.0;
    AffinePicScreenBottom = 2.0;
    AffinePicScreenRight = GameViewWidth - 3.0;
    AffinePicScreenTop = GameViewHeight - 3.0;

    // Motoros kepernyo mely reszen helyezkedik el:
    if (EolSettings->center_camera()) {
        CameraX = (GameViewWidth / MetersToPixels) * 0.50;
    } else {
        CameraX = (SCREEN_WIDTH / MetersToPixels) * 0.15 * EolSettings->zoom();
    }
    CameraDx = GameViewWidth / MetersToPixels - 2.0 * CameraX;
    CameraY = GameViewHeight / MetersToPixels / 2.0;

    MinimapWidth =
        (int)(EolSettings->minimap_width() * sqrt(double(GameViewHeight) / double(SCREEN_HEIGHT)));
    MinimapHeight =
        (int)(EolSettings->minimap_height() * sqrt(double(GameViewHeight) / double(SCREEN_HEIGHT)));
    MinimapX = (int)(40.0 * (VisibleFraction - 0.6) / 0.4);
    MinimapDx = GameViewWidth - 2 * MinimapX - MinimapWidth;
}

// v1 iranyabol v2 fele interpolal:
/*static vect2 interpolal( vect2 v1, vect2 v2, double x ) {
    vect2 vissza = v1+(v2-v1)*x;
    return vissza;
} */

// Meg nem affin koordok:
static void render_affine_pic(vect2 a, vect2 b, pic8* dest, double height, affine_pic* affine,
                              double a_stretch, double b_stretch, int flip = 0) {
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

// radius meg nincs megszorozva:
// Meg nem affin koordok:
static void render_rigidbody(vect2 r, double radius, double rotation, pic8* dest,
                             affine_pic* affine, int flip = 0) {
    vect2 direction(cos(rotation) * radius, sin(rotation) * radius);
    if (flip) {
        render_affine_pic(r + direction, r - direction, dest, radius, affine, 0.0, 0.0, flip);
    } else {
        render_affine_pic(r - direction, r + direction, dest, radius, affine, 0.0, 0.0, flip);
    }
}

static double BikeFrameX = -1, BikeFrameY = -1;
static vect2 BikeFrameI, BikeFrameJ, BikeFrameR;
static vect2 BikeFrameIPixels, BikeFrameJPixels,
    BikeFrameRPixels; // Ezek meg vannak szorozva Aranyossaggal

static void render_bike_part(pic8* dest, affine_pic* part, bike_box* box) {
    vect2 r = BikeFrameIPixels * (box->x1 + 260 - BikeFrameX) +
              BikeFrameJPixels * (BikeFrameY - (box->y1 + 260)) + BikeFrameRPixels;
    vect2 u = BikeFrameIPixels * (box->x2 - box->x1);
    vect2 v = BikeFrameJPixels * (box->y1 - box->y2);

    draw_affine_pic(dest, part, u, v, r);
}

static void render_minimap_icon(pic8* pic, int x, int y, unsigned char palette_id) {
#ifdef DEBUG
    if (x < 1 || x >= pic->get_width() - 1 || y < 1 || y >= pic->get_height() - 1) {
        internal_error("render_minimap_icon out of bounds!");
    }
#endif
    unsigned char* row = pic->get_row(y - 1);
    row[x - 1] = palette_id;
    row[x] = palette_id;
    row[x + 1] = palette_id;
    row = pic->get_row(y + 1);
    row[x - 1] = palette_id;
    row[x] = palette_id;
    row[x + 1] = palette_id;
    pic->ppixel(x - 1, y, palette_id);
    pic->ppixel(x + 1, y, palette_id);
}

/* Most nincsen felho:
static void felhokfel( int x, int y, pic8* ppicbuffer, pic8* pfelho,
                            double lassit ) {
    int kerx = -x*lassit;
    int kery = -y*lassit;
    // Vegigmegy kepernyon:
    int kerxsize = pfelho->get_width();
    kerx %= kerxsize;
    kerx -= kerxsize; // Lehet, hogy enelkul is negativ?
    while( kerx < Displaysizex ) {
        int kerysize = pfelho->get_height();
        kery %= kerysize;
        kery -= kerysize; // Lehet, hogy enelkul is negativ?
        while( kery < Displaysizey ) {
            blit8( ppicbuffer, pfelho, kerx, kery );

            kery += kerysize;
        }
        kerx += kerxsize;
    }
} */

static void render_minimap(bool player1, pic8* pic, double camera_turn_phase, vect2 bike_center,
                           motorst* other_motor) {

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
    int minimap_x1 = (int)(MinimapX + align * MinimapDx);
    int minimap_x2 = minimap_x1 + MinimapWidth - 1;
    int minimap_y1 = 1;
    int minimap_y2 = minimap_y1 + MinimapHeight - 1;

    int opacity = EolSettings->minimap_opacity();

    // Save game scene pixels under the minimap area (including 1px border margin)
    int save_x1 = minimap_x1 - 1;
    int save_y1 = minimap_y1 - 1;
    int save_x2 = minimap_x2 + 1;
    int save_y2 = minimap_y2 + 1;
    int save_w = save_x2 - save_x1 + 1;
    int save_h = save_y2 - save_y1 + 1;
    static pic8* save_pic = nullptr;
    if (opacity < 100) {
        if (!save_pic || save_pic->get_width() != save_w || save_pic->get_height() != save_h) {
            delete save_pic;
            save_pic = new pic8(save_w, save_h);
        }
        blit8(save_pic, pic, 0, 0, save_x1, save_y1, save_x2, save_y2);
    }

    CanvasMinimap->render_minimap(player1, pic, bottomleft_corner, minimap_x1, minimap_y1,
                                  minimap_x2, minimap_y2);

    // Most kirajzoljuk food-okat:
    int corner_x, corner_y;
    CanvasMinimap->meters_to_pixels(bottomleft_corner, &corner_x, &corner_y);
    int object_border_left = corner_x;
    int object_border_bottom = corner_y;
    int object_border_right = corner_x + MinimapWidth - 1;
    int object_border_top = corner_y + MinimapHeight - 1;
    for (int i = 0; i < MAX_OBJECTS; i++) {
        object* obj = Ptop->objects[i];
        if (!obj) {
            break;
        }

        int draw_food = obj->type == object::Type::Food && obj->active;
        int draw_exit = (Single || !Tag) && obj->type == object::Type::Exit;

        if (!draw_food && !draw_exit) {
            continue;
        }

        if (obj->minimap_canvas_x < object_border_left ||
            obj->minimap_canvas_y < object_border_bottom ||
            obj->minimap_canvas_x > object_border_right ||
            obj->minimap_canvas_y > object_border_top) {
            continue;
        }

        // Potty kirajzolasa View ablakba:
        if (obj->type == object::Type::Food) {
            render_minimap_icon(pic, obj->minimap_canvas_x - corner_x + minimap_x1,
                                obj->minimap_canvas_y - corner_y + minimap_y1,
                                Lgr->minimap_food_palette_id);
        } else {
            render_minimap_icon(pic, obj->minimap_canvas_x - corner_x + minimap_x1,
                                obj->minimap_canvas_y - corner_y + minimap_y1,
                                Lgr->minimap_exit_palette_id);
        }
    }

    // Most kirajzoljuk motorost:
    // Eldontjuk szineket:
    unsigned char bike1_id = Lgr->minimap_bike1_palette_id;
    unsigned char bike2_id = Lgr->minimap_bike2_palette_id;
    if ((State->player1_bike1 && !player1) || (!State->player1_bike1 && player1)) {
        bike1_id = Lgr->minimap_bike2_palette_id;
        bike2_id = Lgr->minimap_bike1_palette_id;
    }

    // Most kirajzoljuk masik (hatso) motorost:
    if (other_motor) {
        vect2 other_pos = other_motor->bike.r - bottomleft_corner;
        int other_x = (int)(other_pos.x * MetersToMinimapPixels + minimap_x1);
        int other_y = (int)(other_pos.y * MetersToMinimapPixels + minimap_y1);
        if (other_x >= minimap_x1 && other_x <= minimap_x2 && other_y >= minimap_y1 &&
            other_y <= minimap_y2) {
            render_minimap_icon(pic, other_x, other_y, bike2_id);
        }
    }

    // Most kirajzoljuk elso motorost:
    int bike_x = (int)(camera_pos.x * MetersToMinimapPixels);
    int bike_y = (int)(camera_pos.y * MetersToMinimapPixels);
    render_minimap_icon(pic, minimap_x1 + bike_x, minimap_y1 + bike_y, bike1_id);

    // Kirajzoljuk keretet:
    minimap_x1--;
    minimap_y1--;
    minimap_x2++;
    minimap_y2++;
    for (int y = minimap_y1; y <= minimap_y2; y++) {
        pic->ppixel(minimap_x1, y, Lgr->minimap_border_palette_id);
        pic->ppixel(minimap_x2, y, Lgr->minimap_border_palette_id);
    }
    unsigned char* row = pic->get_row(minimap_y1) + minimap_x1 + 1;
    memset(row, Lgr->minimap_border_palette_id, MinimapWidth);
    row = pic->get_row(minimap_y2) + minimap_x1 + 1;
    memset(row, Lgr->minimap_border_palette_id, MinimapWidth);

    // bring back pixels from the saved game scene based on opacity
    if (opacity < 100) {
        blit8_dither(pic, save_pic, save_x1, save_y1, opacity);
    }
}

// Mentes lemezre:
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
        int i = 0;
        while (1) {
            std::string filename = std::format("snp{:05}.pcx", i);
            if (!std::filesystem::exists(filename)) {
                pic->vertical_flip();
                pic->save(filename.c_str(), Lgr->palette_data);
                pic->vertical_flip();
                break;
            }
            i++;
        }
    }
}

static void render_background(pic8* pic) {
    int y = 0;
    while (y < SCREEN_HEIGHT) {
        int x = 0;
        while (x < SCREEN_WIDTH) {
            // internal_error( "Itt van 14!" ); idejott
            // internal_error( tmp ); (16) kiirja koordokat szepen

            // extern int Debugblt;
            // Debugblt = 1;
            blit8(pic, Lgr->qframe, x, y);
            // Debugblt = 0;
            //  internal_error( "Itt van 20!" ); idejott
            x += Lgr->qframe->get_width();

            // internal_error( tmp ); (21) kiirja x-et 207-nek
        }
        // internal_error( "Itt van 15!" );
        y += Lgr->qframe->get_height();
    }
}

#ifdef DEBUG
int Voltkilogas = 0;
#endif

static void render_bike(int player1, pic8* pic, double time, vect2 bottomleft_corner, motorst* mot,
                        valtozok* metadata, bike_pics* bike, affine_pic* shirt) {

    // Mivel ezeket fv megvaltoztatja:
    double arm_position = metadata->ugrasnagysag;
    double turn_phase = metadata->baljobbv_f.forgas;

    // if( metadata->baljobb < 0 || metadata->baljobb > 1.0 )
    //     internal_error( "metadata->baljobb < 0 || metadata->baljobb > 1.0!" );
    // if( turn_phase < 0.0 || turn_phase > 1.0 )
    //     internal_error( "turn_phase < 0.0 || turn_phase > 1.0!" );

    // Ha kell affinitassal torodni:
    int is_turning = 0;
    StretchEnabled = false;
    if (turn_phase < 0.999) {
        is_turning = 1;
        // forgast 0, 1 intervallumbol atkonvertaljuk -1, 1 intervallumba:
        turn_phase = -cos(turn_phase * PI);
    }

    vect2 left_wheel_r = (mot->left_wheel.r - bottomleft_corner);
    vect2 right_wheel_r = (mot->right_wheel.r - bottomleft_corner);

    int left_wheel_in_back = 1;
    int right_wheel_in_back = 1;

    if (is_turning) {
        if ((turn_phase > 0.0 && !mot->flipped_bike) || (turn_phase <= 0.0 && mot->flipped_bike)) {
            left_wheel_in_back = 0;
        } else {
            right_wheel_in_back = 0;
        }
    }

    // Kerekek:
    if (left_wheel_in_back) {
        render_rigidbody(left_wheel_r, WheelBackgroundRenderRadius, mot->left_wheel.rotation, pic,
                         bike->wheel);
    }
    if (right_wheel_in_back) {
        render_rigidbody(right_wheel_r, WheelBackgroundRenderRadius, mot->right_wheel.rotation, pic,
                         bike->wheel);
    }

    vect2 bike_r = mot->bike.r - bottomleft_corner;
    vect2 bike_i = vect2(cos(mot->bike.rotation), sin(mot->bike.rotation));
    vect2 bike_j = rotate_90deg(bike_i);

    if (is_turning) {
        StretchEnabled = true;
        set_stretch_parameters(bike_r, bike_i, turn_phase, MetersToPixels);
    }

    if (mot->flipped_bike) {
        bike_i = Vect2null - bike_i;
        vect2 tmpv = left_wheel_r;
        left_wheel_r = right_wheel_r;
        right_wheel_r = tmpv;
    }

    BikeFrameX = 390.0, BikeFrameY = 420.0;
    double BIKE_FRAME_ROTATION = 0.62;
    double BIKE_FRAME_WIDTH = 0.0045;
    BikeFrameI = bike_i * (BIKE_FRAME_WIDTH * cos(BIKE_FRAME_ROTATION)) +
                 bike_j * (BIKE_FRAME_WIDTH * sin(BIKE_FRAME_ROTATION));
    BikeFrameJ = rotate_90deg(BikeFrameI);
    if (mot->flipped_bike) {
        BikeFrameJ = Vect2null - BikeFrameJ;
    }
    BikeFrameR = bike_r;
    // Most MetersToPixels-t betesszuk BikeFrameI, BikeFrameJ, BikeFrameR-be:
    BikeFrameIPixels = BikeFrameI * MetersToPixels;
    BikeFrameJPixels = BikeFrameJ * MetersToPixels;
    BikeFrameRPixels = BikeFrameR * MetersToPixels;

    // Elso felfuggesztes:
    vect2 susp1_r =
        BikeFrameI * (365.0 - BikeFrameX) + BikeFrameJ * (BikeFrameY - 292.0) + BikeFrameR; // Uj
    render_affine_pic(left_wheel_r, susp1_r, pic, 0.06, bike->susp1, 0.05, 0.03);

    // Hatso felfuggesztes:
    vect2 susp2_r =
        BikeFrameI * (370.0 - BikeFrameX) + BikeFrameJ * (BikeFrameY - 520.0) + BikeFrameR;
    render_affine_pic(susp2_r, right_wheel_r, pic, 0.06, bike->susp2, 0.0, 0.1);

    // Ha kell zaszlot rajzol:
    int draw_flag = 0;
    if (!Single && Tag) {
        if ((player1 && FlagTagAHasFlag) || (!player1 && !FlagTagAHasFlag)) {
            draw_flag = 1;
        }

        if (!draw_flag && FlagTagImmunity) {
            int blinking_flag_phase = (int)(time * 30.0);
            if (blinking_flag_phase % 2) {
                draw_flag = 1;
            }
        }
    }
    vect2 flag_base_r = BikeFrameI * (500.0 + 107 - BikeFrameX) +
                        BikeFrameJ * (BikeFrameY + 114 - 600.0) + BikeFrameR;
    // 330 bal oldal (5)
    // 340 bal oldal (3)
    // 355 bal oldal (1.5)
    // 365 jobb oldal (1)
    // 361 jobb hajszalnyit
    vect2 flag_tip_r = flag_base_r + (BikeFrameI * 356.0 + BikeFrameJ * 500.0) * 0.2;
    if (draw_flag) {
        render_affine_pic(flag_base_r, flag_tip_r, pic, 0.2, Lgr->flag, 0.0, 0.0,
                          mot->flipped_bike);
    }

    // Motor teste:
    render_bike_part(pic, bike->bike_part1, &BikeBox1);
    render_bike_part(pic, bike->bike_part2, &BikeBox2);
    render_bike_part(pic, bike->bike_part3, &BikeBox3);
    render_bike_part(pic, bike->bike_part4, &BikeBox4);

    // Mozgo test:
    vect2 body_r = (mot->body_r - bottomleft_corner); // Vezeto
    vect2 hip_r = body_r + BikeFrameI * 75.0 + BikeFrameJ * (-47.0);
    vect2 shoulder_r = body_r + BikeFrameI * 47.0 + BikeFrameJ * 65.0;
    vect2 neck_r = body_r + BikeFrameI * 41.0 + BikeFrameJ * 70.0;
    vect2 foot_r =
        BikeFrameI * (346.0 - BikeFrameX) + BikeFrameJ * (BikeFrameY - 514.0) + BikeFrameR;
    vect2 knee_r;

    double THIGH_LENGTH = 0.51;
    double LEG_LENGTH = 0.51;
    /* Bicigli lab hosszak:
    double THIGH_LENGTH = 0.56;
    double LEG_LENGTH = 0.56;
    */

    if (mot->flipped_bike) {
        knee_r = circles_intersection(hip_r, foot_r, THIGH_LENGTH, LEG_LENGTH);
    } else {
        knee_r = circles_intersection(foot_r, hip_r, LEG_LENGTH, THIGH_LENGTH);
    }

    // Fej:
    // double head_rotation = 0.20;
    double head_rotation = 0.0;
    if (mot->flipped_bike) {
        head_rotation = 0 - head_rotation;
    }
    render_rigidbody(mot->head_r - bottomleft_corner, HeadRadius,
                     mot->bike.rotation + head_rotation, pic, bike->head, mot->flipped_bike);

    // Kar ket reszletben:
    vect2 hand_r = susp1_r;

    // Ugras belekombinalasa kar kirajzolasaba:
    // hand_r-et mozditjuk el szukseges mertekben:
    if (arm_position > 0.0001) {
        arm_position = 1.0 - arm_position;
        int arm_goes_up = 1; // kar kiteresenek iranya
        if ((metadata->ugras1volt && !mot->flipped_bike) ||
            (!metadata->ugras1volt && mot->flipped_bike)) { // XOR
            arm_goes_up = 0;
        }
        // alfa es hosszit kiszamolasa:
        double arm_rotation = 0.0;
        double arm_stretch = 0.1;
        if (arm_goes_up) {
            // KAR FELFELE LENDUL KI:
            double arm_apex_time = 0.25;
            double max_arm_rotation = 2.7;
            double max_arm_stretch = -0.3;
            if (arm_position < arm_apex_time) {
                // Kilendit:
                arm_rotation = max_arm_rotation * arm_position / arm_apex_time;
                arm_stretch = max_arm_stretch * arm_position / arm_apex_time + 1.0;
            } else {
                // Visszaenged:
                double interpolation = 1.0 - (arm_position - arm_apex_time) / (1.0 - arm_apex_time);
                arm_rotation = max_arm_rotation * interpolation;
                arm_stretch = max_arm_stretch * interpolation + 1.0;
            }
        } else {
            // KAR LEFELE LENDUL KI:
            double arm_apex_time = 0.2;
            double max_arm_rotation = -1.6;
            double max_arm_stretch = 0.15;
            if (arm_position < arm_apex_time) {
                // Kilendit:
                arm_rotation = max_arm_rotation * arm_position / arm_apex_time;
                arm_stretch = max_arm_stretch * arm_position / arm_apex_time + 1.0;
            } else {
                // Visszaenged:
                double interpolation = 1.0 - (arm_position - arm_apex_time) / (1.0 - arm_apex_time);
                arm_rotation = max_arm_rotation * interpolation;
                arm_stretch = max_arm_stretch * interpolation + 1.0;
            }
        }
        // alfa es hosszit alapjan hand_r forgatasa es nyujtasa vall korul:
        vect2 arm_vector = hand_r - shoulder_r;
        if (!mot->flipped_bike) {
            arm_vector.rotate(-arm_rotation);
        } else {
            arm_vector.rotate(arm_rotation);
        }
        arm_vector = arm_vector * arm_stretch;
        hand_r = shoulder_r + arm_vector;
    }

    double FORARM_LENGTH = 0.308 * 1.05;
    double UP_ARM_LENGTH = 0.328 * 1.05;
    vect2 elbow_r;
    if (mot->flipped_bike) {
        elbow_r = circles_intersection(hand_r, shoulder_r, FORARM_LENGTH, UP_ARM_LENGTH);
    } else {
        elbow_r = circles_intersection(shoulder_r, hand_r, UP_ARM_LENGTH, FORARM_LENGTH);
    }

    // Lab:
    render_affine_pic(knee_r, hip_r, pic, 0.14, bike->thigh, 0.03, 0.1, mot->flipped_bike);

    render_affine_pic(foot_r, knee_r, pic, 0.21, bike->leg, 0.03, 0.03, mot->flipped_bike);

    // Vezeto:
    affine_pic* body = shirt ? shirt : bike->body;
    render_affine_pic(hip_r, neck_r, pic, 0.2, body, 0.1, 0.05, mot->flipped_bike);

    // Kez:
    render_affine_pic(elbow_r, shoulder_r, pic, 0.11, bike->up_arm, 0.08, 0.1, !mot->flipped_bike);

    render_affine_pic(hand_r, elbow_r, pic, 0.076, bike->forarm, 0.08, 0.1, mot->flipped_bike);

    StretchEnabled = false;

    // Kerekek:
    if (!left_wheel_in_back || !right_wheel_in_back) {
        if (mot->flipped_bike) {
            // Visszacserelem kerekek koordjait:
            vect2 tmpv = left_wheel_r;
            left_wheel_r = right_wheel_r;
            right_wheel_r = tmpv;
        }
        if (!left_wheel_in_back) {
            render_rigidbody(left_wheel_r, mot->left_wheel.radius, mot->left_wheel.rotation, pic,
                             bike->wheel);
        }
        if (!right_wheel_in_back) {
            render_rigidbody(right_wheel_r, mot->right_wheel.radius, mot->right_wheel.rotation, pic,
                             bike->wheel);
        }
    }
}

static void render_view(bool player1, pic8* pic, double time, motorst* mot, valtozok* metadata,
                        int show_minimap, int show_timer, motorst* other_mot,
                        valtozok* other_metadata, camera& current_camera) {

    // Kiszamoljuk kepernyo bal also sarkat:
    vect2 bike_center = mot->bike.r;
    if (current_camera.mode == CameraMode::MapViewer) {
        bike_center = vect2(current_camera.x, current_camera.y);
    }

    vect2 bottomleft_corner(bike_center.x - (CameraX + metadata->baljobbv_h.baljobb * CameraDx),
                            bike_center.y - CameraY);

    // ppic->fill_box( 100 );
    CanvasBack->render(player1, pic, bottomleft_corner, 0, 0, GameViewWidth - 1,
                       GameViewHeight - 1);

    // Objektumok kirajzolasa, de view-ba csak kesobb kerulnek:
    int corner_x, corner_y;
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

        if (obj->type == object::Type::Start || (obj->type == object::Type::Food && !obj->active) ||
            (!Single && Tag && obj->type == object::Type::Exit)) {
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
            // Elso frame-et adja vissza mindig:
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

    // Motorosok kirajzolasa:
    bike_pics* bike1 = &Lgr->bike1;
    bike_pics* bike2 = &Lgr->bike2;
    if ((State->player1_bike1 && !player1) || (!State->player1_bike1 && player1)) {
        bike1 = &Lgr->bike2;
        bike2 = &Lgr->bike1;
    }

    if (current_camera.mode == CameraMode::Normal) {
        if (!Single) {
            // Hatso motoros kirajzolasa:
            vect2 center(bottomleft_corner.x + (SCREEN_WIDTH / 2.0) * PixelsToMeters,
                         bottomleft_corner.y + (SCREEN_HEIGHT / 2.0) * PixelsToMeters);
            double distance = (other_mot->bike.r - center).length();
            if (distance < (std::max(SCREEN_WIDTH, SCREEN_HEIGHT) * 27.0 / 32.0) * PixelsToMeters) {
                render_bike(!player1, pic, time, bottomleft_corner, other_mot, other_metadata,
                            bike2, nullptr);
            }
        }

        // Motoros kirajzolasa:
        render_bike(player1, pic, time, bottomleft_corner, mot, metadata, bike1, shirt);
    }

    if (!EolSettings->pictures_in_background()) {
        // Felso canvas kitevese:
        CanvasFront->render(player1, pic, bottomleft_corner, 0, 0, GameViewWidth - 1,
                            GameViewHeight - 1);
    }

    if (show_minimap) {
        if (Single) {
            render_minimap(player1, pic, metadata->baljobbv_h.baljobb, bike_center, nullptr);
        } else {
            render_minimap(player1, pic, metadata->baljobbv_h.baljobb, bike_center, other_mot);
        }
    }

    double flagtag_time = -1.0;
    if (!Single) {
        if (player1) {
            flagtag_time = FlagTimeA;
        } else {
            flagtag_time = FlagTimeB;
        }
    }

    // Idokiiras:
    if (show_timer) {
        // Korny->legjobbido - volt eredetileg "" helyett:
        /*if( sordarabszam > 0 ) {
            char darabok[10];
            sprintf( darabok, "%d", sordarabszam );
            strrev( darabok );
            while( strlen( darabok ) < 6 )
                strcat( darabok, "0" );
            strrev( darabok );
            darabok[7] = darabok[5];
            darabok[6] = darabok[4];
            darabok[4] = darabok[3];
            darabok[3] = darabok[2];
            draw_timers( darabok, flagtag_time, time, ppic, GameViewWidth, GameViewHeight );
        }
        else*/
        if (Tag) {
            draw_timers(BestTime, flagtag_time, time, pic, GameViewWidth, GameViewHeight);
        } else {
            draw_timers(BestTime, -1.0, time, pic, GameViewWidth, GameViewHeight);
        }
    }

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

static pic8 player_view = pic8();

extern int Ucsosurlodas;

void render_game(double time, valtozok* metadata1, valtozok* metadata2, int show_minimap1,
                 int show_timer1, int show_minimap2, int show_timer2, camera& current_camera) {

    int splitscreen = 0;
    int draw_player1_only = 1;

    if (!Single) {
        // Multiplayer modban:
        if (!metadata1->showkep && !metadata2->showkep) {
            internal_error("render_game nobody visible!");
        }
        if (metadata1->showkep && metadata2->showkep) {
            splitscreen = 1;
        } else {
            // Csak egyik latszik:
            if (metadata1->showkep) {
                draw_player1_only = 1;
            } else {
                draw_player1_only = 0;
            }
        }
        if (current_camera.mode == CameraMode::MapViewer) {
            splitscreen = 0;
            draw_player1_only = 1;
        }
    }

    pic8* pic = lockbackbuffer_pic();

    // internal_error( "Itt van 11!" ); idejott

    if (GameBackgroundRender > 0) {
        GameBackgroundRender--;
        calculate_viewpoints(splitscreen);

        // internal_error( "Itt van 13!" ); idejott

        render_background(pic);
    }

    // internal_error( "Itt van 12!" ); ide nemjon

    if (splitscreen) {
        // 1. Jatekos:
        player_view.subview(GameViewLeft, GameViewBottom1, GameViewRight, GameViewTop1, pic);

        render_view(true, &player_view, time, Motor1, metadata1, show_minimap1, show_timer1, Motor2,
                    metadata2, current_camera);

        // 2. Jatekos:
        player_view.subview(GameViewLeft, GameViewBottom2, GameViewRight, GameViewTop2, pic);

        render_view(false, &player_view, time, Motor2, metadata2, show_minimap2, show_timer2,
                    Motor1, metadata1, current_camera);
    } else {
        player_view.subview(GameViewLeft, GameViewBottom1, GameViewRight, GameViewTop1, pic);
        if (draw_player1_only) {
            render_view(true, &player_view, time, Motor1, metadata1, show_minimap1, show_timer1,
                        Motor2, metadata2, current_camera);
        } else {
            render_view(false, &player_view, time, Motor2, metadata2, show_minimap2, show_timer2,
                        Motor1, metadata1, current_camera);
        }
    }

    Console->render(*pic, *SmallFont);
    StatusMessages->render(*pic, *SmallFont);
    EolClient->render_table(*pic, *MediumFont, *SmallFont);

    // Mentes lemezre:
    handle_screenshot(pic);

    // for( int i = 0; i < Ucsosurlodas; i++ )
    //	pic->ppixel( i, 100, 0 );

    unlockbackbuffer_pic();
}
