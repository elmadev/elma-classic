
#include "game/driver.h"
#include "physics/init.h"
#include "pic/lgr.h"
#include "pic/pic8.h"
#include "renderer/affine.h"
#include "util/util.h"
#include "vect2.h"
#include <cmath>

// Render an affine_pic (remember all affine_pic images are loaded sideways in the lgr)
// All units are in meters
// a = coordinate of middle left of affine_pic position (distal end of the limb)
// b = coordinate of middle right of affine_pic position (proximal end of the limb)
// Along the axis of the vector b->a, displace coordinate a by `a_stretch` meters
// Along the axis of the vector a->b, displace coordinate b by `b_stretch` meters
// height represents the vertical length of the affine_pic (thickness of the limb)
static void render_affine_pic(vect2 a, vect2 b, pic8* dest, double height, const pic8* affine,
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
    draw_affine_pic(dest, affine, affine->gpixel(0, 0), u, v * 2.0, a);
}

// Render a wheel or head affine_pic
static void render_rigidbody(vect2 r, double radius, double rotation, pic8* dest, pic8* affine,
                             bool flip) {
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
static void render_bike_part(pic8* dest, pic8* part, unsigned char transparency, bike_box* box) {
    vect2 r = BikeFrameIPixels * (box->x1 + 260 - BikeFrameX) +
              BikeFrameJPixels * (BikeFrameY - (box->y1 + 260)) + BikeFrameRPixels;
    vect2 u = BikeFrameIPixels * (box->x2 - box->x1);
    vect2 v = BikeFrameJPixels * (box->y1 - box->y2);

    draw_affine_pic(dest, part, transparency, u, v, r);
}

// Render an entire bike + kuski
void render_bike(pic8* pic, bool has_flag, vect2 bottomleft_corner, const motorst* mot,
                 const bike_metadata* metadata, const bike_pics* bike, const pic8* shirt) {
    double arm_position = metadata->arm_position;
    double turn_phase = metadata->bike_turning.turn_phase;

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
    constexpr double WHEEL_RENDER_RADIUS = 0.395;
    if (left_wheel_in_back) {
        render_rigidbody(left_wheel_r, WHEEL_RENDER_RADIUS, mot->left_wheel.rotation, pic,
                         bike->wheel, false);
    }
    if (right_wheel_in_back) {
        render_rigidbody(right_wheel_r, WHEEL_RENDER_RADIUS, mot->right_wheel.rotation, pic,
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
    if (has_flag) {
        vect2 flag_base_r = BikeFrameI * (500.0 + 107 - BikeFrameX) +
                            BikeFrameJ * (BikeFrameY + 114 - 600.0) + BikeFrameR;
        vect2 flag_tip_r = flag_base_r + (BikeFrameI * 356.0 + BikeFrameJ * 500.0) * 0.2;
        render_affine_pic(flag_base_r, flag_tip_r, pic, 0.2, Lgr->flag, 0.0, 0.0,
                          mot->flipped_bike);
    }

    // Draw bike frame
    unsigned char bike_part_transparency = bike->bike_part1->gpixel(0, 0);
    render_bike_part(pic, bike->bike_part1, bike_part_transparency, &BikeBox1);
    render_bike_part(pic, bike->bike_part2, bike_part_transparency, &BikeBox2);
    render_bike_part(pic, bike->bike_part3, bike_part_transparency, &BikeBox3);
    render_bike_part(pic, bike->bike_part4, bike_part_transparency, &BikeBox4);

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
        if ((metadata->volt_is_right && !mot->flipped_bike) ||
            (!metadata->volt_is_right && mot->flipped_bike)) {
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
    const pic8* body = shirt ? shirt : bike->body;
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
            render_rigidbody(left_wheel_r, WHEEL_RENDER_RADIUS, mot->left_wheel.rotation, pic,
                             bike->wheel, false);
        }
        if (!right_wheel_in_back) {
            render_rigidbody(right_wheel_r, WHEEL_RENDER_RADIUS, mot->right_wheel.rotation, pic,
                             bike->wheel, false);
        }
    }
}
