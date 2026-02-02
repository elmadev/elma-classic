#include "affine_pic_render.h"
#include "affine_pic.h"
#include "KIRAJZOL.H"
#include "pic8.h"
#include "vect2.h"
#include <algorithm>
#include <cmath>

// Affine transformation parameters when bike is turning
bool StretchEnabled = false;
static double StretchFactor = 1.0;
static vect2 StretchCenter = Vect2i;
static vect2 StretchAxis = Vect2i;
static double StretchMetersToPixels = 1.0;

// Render a horizontal slice of pixels into the `dest` by grabbing a diagonal slice of pixel data
// from an affine_pic.
//
// `transparency` is the transparent palette id.
// `length` is the number of pixels to draw in `dest`.
// `source` is the pixels from an affine_pic.
// `source_x` / `source_y` is the starting position within the affine_pic's pixels.
// `source_dx` / `source_dy` is the delta to the next pixel to grab from the affine_pic.
void draw_affine_pic_row(unsigned char transparency, int length, unsigned char* dest,
                         unsigned char* source, double source_x, double source_y, double source_dx,
                         double source_dy) {
    // Draw the horizontal row of pixels
    for (int x = 0; x < length; x++) {
        // Grab the pixel from the affine_pic
        int source_offset = ((int)(source_y) << 8) + (int)(source_x);
        unsigned char c = source[source_offset];
        if (c != transparency) {
            dest[x] = c;
        }

        // Update the affine_pic position from which we copy
        source_x += source_dx;
        source_y += source_dy;
    }
}

void set_stretch_parameters(vect2 bike_center, vect2 bike_i, double stretch,
                            double meters_to_pixels) {
    StretchCenter = bike_center;
    bike_i.normalize();
    StretchAxis = bike_i;
    StretchFactor = stretch;
    StretchMetersToPixels = meters_to_pixels;
}

void draw_affine_pic(pic8* dest, affine_pic* aff, vect2 u, vect2 v, vect2 r) {
    unsigned char transparency = aff->transparency;

    // Bike is turning! Let's stretch the bike
    if (StretchEnabled) {
        // Convert the coordinates from pixels to meters, since the StretchEnabled vars are in
        // meters
        r.x /= StretchMetersToPixels;
        r.y /= StretchMetersToPixels;
        u.x /= StretchMetersToPixels;
        u.y /= StretchMetersToPixels;
        v.x /= StretchMetersToPixels;
        v.y /= StretchMetersToPixels;

        // Stretch coordinate r
        double distance = (r - StretchCenter) * StretchAxis;
        vect2 delta = (distance * (1.0 - StretchFactor)) * StretchAxis;
        r = r - delta;

        // Stretch coordinate u
        distance = u * StretchAxis;
        delta = (distance * (1.0 - StretchFactor)) * StretchAxis;
        u = u - delta;

        // Stretch coordinate v
        distance = v * StretchAxis;
        delta = (distance * (1.0 - StretchFactor)) * StretchAxis;
        v = v - delta;

        // Convert the coordinates back into pixels
        r.x *= StretchMetersToPixels;
        r.y *= StretchMetersToPixels;
        u.x *= StretchMetersToPixels;
        u.y *= StretchMetersToPixels;
        v.x *= StretchMetersToPixels;
        v.y *= StretchMetersToPixels;
    }

    // Check if the picture is rotated at a 90-degree offset +- tolerance of MINIMUM_ROTATION.
    // The reason why we do this is linear algebra: it is impossible to invert a matrix that
    // represents a rotation of exactly 0/90/180/270 degrees.
    //
    // For small angles, Angle = tan⁻¹(y/x) ≈ y/x, so we skip the trigonometric function for speed.
    bool needs_rotation = false;
    bool positive_rotation_direction = 0;
    constexpr double MINIMUM_ROTATION = 0.005; // radians
    if (u.x == 0.0) {
        // Perfectly upright
        needs_rotation = true;
    } else {
        if (fabs(u.y / u.x) < MINIMUM_ROTATION) {
            // Upright within a small tolerance
            needs_rotation = true;
            if (u.y / u.x > 0) {
                // Is slightly rotated clockwise (as opposed to counter-clockwise)
                positive_rotation_direction = true;
            }
        }
    }
    if (v.x == 0.0) {
        // Perfectly sideways
        needs_rotation = true;
    } else {
        if (fabs(v.y / v.x) < MINIMUM_ROTATION) {
            // Sideways within a small tolerance
            needs_rotation = true;
            if (v.y / v.x > 0) {
                // Is slightly rotated clockwise (as opposed to counter-clockwise)
                positive_rotation_direction = true;
            }
        }
    }
    // Rotate the picture away from the 90-degree offset
    if (needs_rotation) {
        if (positive_rotation_direction) {
            u.rotate(MINIMUM_ROTATION);
            v.rotate(MINIMUM_ROTATION);
        } else {
            u.rotate(-MINIMUM_ROTATION);
            v.rotate(-MINIMUM_ROTATION);
        }
    }

    // For turning bike, we give up on a real proper check, and if the bike is still not properly
    // rotated away from 90-degrees, we just keep rotating it in tiny increments...
    if (StretchEnabled) {
        needs_rotation = true;
        while (needs_rotation) {
            needs_rotation = false;
            if (u.x == 0.0) {
                needs_rotation = true;
            }
            if (fabs(u.y / u.x) < MINIMUM_ROTATION) {
                needs_rotation = true;
            }
            if (u.y == 0.0) {
                needs_rotation = true;
            }
            if (fabs(u.x / u.y) < MINIMUM_ROTATION) {
                needs_rotation = true;
            }
            if (v.x == 0.0) {
                needs_rotation = true;
            }
            if (fabs(v.y / v.x) < MINIMUM_ROTATION) {
                needs_rotation = true;
            }
            if (v.y == 0.0) {
                needs_rotation = true;
            }
            if (fabs(v.x / v.y) < MINIMUM_ROTATION) {
                needs_rotation = true;
            }
            if (needs_rotation) {
                u.rotate(MINIMUM_ROTATION);
                v.rotate(MINIMUM_ROTATION);
            }
        }
    }

    // Calculate inverse transformation matrix.
    // u and v describe the render box of the entire image.
    // u_pixel and b_pixel describe the render box of a single pixel.
    vect2 u_pixel = u * (1.0 / (aff->width - 1));
    vect2 v_pixel = v * (1.0 / (aff->height - 1));

    // Matrix:
    // u_pixel.x v_pixel.x    i.e.   a b
    // u_pixel.y v_pixel.y           c d
    // Inverse of matrix = (1/determinant)* d  -b
    //                                    -c  a
    // Determinant of matrix = (ad - bc)
    double determinant_reciprocal = 1.0 / (u_pixel.x * v_pixel.y - v_pixel.x * u_pixel.y);
    vect2 inverse_i(v_pixel.y * determinant_reciprocal, -u_pixel.y * determinant_reciprocal);
    vect2 inverse_j(-v_pixel.x * determinant_reciprocal, u_pixel.x * determinant_reciprocal);

    // Resultant inverse matrix:
    // inverse_i.x inverse_j.x
    // inverse_i.y inverse_j.y

    // Check to see if any part of the image is out of bounds:
    // Get the min/max values of the drawing area
    double max_x;
    double min_x;
    if (u.x > 0) {
        if (v.x > 0) {
            max_x = r.x + u.x + v.x;
            min_x = r.x;
        } else {
            max_x = r.x + u.x;
            min_x = r.x + v.x;
        }
    } else {
        if (v.x > 0) {
            max_x = r.x + v.x;
            min_x = r.x + u.x;
        } else {
            max_x = r.x;
            min_x = r.x + u.x + v.x;
        }
    }
    // At the same time, let's grab the coordinate of the very top of the image (apex).
    double min_y;
    vect2 apex;
    if (u.y > 0) {
        if (v.y > 0) {
            // u pos, v pos:
            apex = r + u + v;
            min_y = r.y;
        } else {
            // u pos, v neg:
            apex = r + u;
            min_y = r.y + v.y;
        }
    } else {
        if (v.y > 0) {
            // u neg, v pos:
            apex = r + v;
            min_y = r.y + u.y;
        } else {
            // u neg, v neg:
            apex = r;
            min_y = r.y + u.y + v.y;
        }
    }
    double max_y = apex.y;

    bool possibly_out_of_bounds = false;
    if (max_x > Hatarx2 || min_x < Hatarx1 || max_y > Hatary2 || min_y < Hatary1) {
        possibly_out_of_bounds = true;
    }

    // We are ready to start rendering. Let's start from the apex
    int x_left = (int)(apex.x);
    int y = (int)(apex.y);
    double apex_y = y;

    // For each y, we need to calculate the x range where we need to render the bike.
    // We do this by calculating two different x ranges for the starting apex row y
    //
    // 1) We extend the lines r->r+u and r+v->r+u+v to height y.
    // We now have an x range plane1_left/plane1_right
    // (plane1_right correspondings roughly to apex.x)
    // 2) We extend the lines r->r+v and r+u->r+u+v to height y.
    // We now have a different x range plane2_left/plane2_right
    // (plane2_left correspondings roughly to apex.x)
    //
    // When the x position is within BOTH plane1_left<->plane1_right and plane2_left<->plane2_right,
    // then we know the pixel should be rendered.
    // We will update these values at every row y using the slope.
    double plane1_slope = u.x / u.y;
    double plane1_left = r.x + (apex_y - r.y) * plane1_slope;
    double plane1_right = r.x + v.x + (apex_y - r.y - v.y) * plane1_slope;
    if (plane1_left > plane1_right) {
        std::swap(plane1_left, plane1_right);
    }
    double plane2_slope = v.x / v.y;
    double plane2_left = r.x + (apex_y - r.y) * plane2_slope;
    double plane2_right = r.x + u.x + (apex_y - r.y - u.y) * plane2_slope;
    if (plane2_left > plane2_right) {
        std::swap(plane2_left, plane2_right);
    }

    // Apply the affine transformation to the apex to convert from meters to affine_pic pixel
    // coordinates.
    // affine_origin = inverted_matrix*diff
    vect2 diff = vect2(x_left, y) - r;
    vect2 affine_origin = vect2(0.5, 0.5) + diff.x * inverse_i + diff.y * inverse_j;
    double affine_x = affine_origin.x;
    double affine_y = affine_origin.y;

    // We add a few extra checks if part of the bike is out of bounds.
    // The else case is the normal case, only differences are noted here.
    if (possibly_out_of_bounds) {
        while (true) {
            int x1_plane = (int)(std::ceil(std::max(plane1_left, plane2_left)));
            int x2_plane = (int)(std::floor(std::min(plane1_right, plane2_right)));
            // Extra x out of bounds check
            int x1 = std::max(0, x1_plane);
            int x2 = std::min(Cxsize - 1, x2_plane);
            // Extra y out of bounds check
            if (x1 <= x2 && y < Cysize) {
                while (x_left > x1) {
                    x_left--;
                    affine_x -= inverse_i.x;
                    affine_y -= inverse_i.y;
                }
                while (x_left < x1) {
                    x_left++;
                    affine_x += inverse_i.x;
                    affine_y += inverse_i.y;
                }
                unsigned char* dest_target = dest->get_row(y);
                dest_target += x_left;
                draw_affine_pic_row(transparency, x2 - x1 + 1, dest_target, aff->pixels, affine_x,
                                    affine_y, inverse_i.x, inverse_i.y);
            } else {
                if (x1_plane > x2_plane + 1) {
                    return;
                }
            }
            y--;
            // Extra y out of bounds check
            if (y < 0) {
                return;
            }
            affine_x -= inverse_j.x;
            affine_y -= inverse_j.y;
            plane1_left -= plane1_slope;
            plane1_right -= plane1_slope;
            plane2_left -= plane2_slope;
            plane2_right -= plane2_slope;
        }
    } else {
        // For each row of the destination
        while (true) {
            // Get the left render edge, rounded up
            int x1 = (int)(std::ceil(std::max(plane1_left, plane2_left)));
            // Get the right render edge, rounded down
            int x2 = (int)(std::floor(std::min(plane1_right, plane2_right)));
            if (x1 <= x2) {
                // If we are drawing at least 1 pixel, we need to update our source pixel position
                // by moving by the transformed matrix units
                while (x_left > x1) {
                    x_left--;
                    affine_x -= inverse_i.x;
                    affine_y -= inverse_i.y;
                }
                while (x_left < x1) {
                    x_left++;
                    affine_x += inverse_i.x;
                    affine_y += inverse_i.y;
                }
                // We know our source affine_pic position and our destination position, so let's
                // draw!
                unsigned char* dest_target = dest->get_row(y);
                dest_target += x_left;
                draw_affine_pic_row(transparency, x2 - x1 + 1, dest_target, aff->pixels, affine_x,
                                    affine_y, inverse_i.x, inverse_i.y);
            } else {
                // If the draw width is 0 pixels, we continue (for very thin images)
                // If the draw width <= -1, then we are completely done rendering and we stop here
                if (x1 > x2 + 1) {
                    return;
                }
            }
            // Go to the next row
            y--;
            // Update our affine_pic source position
            affine_x -= inverse_j.x;
            affine_y -= inverse_j.y;
            // Slide plane1 to the right with updated positions for the current row
            plane1_left -= plane1_slope;
            plane1_right -= plane1_slope;
            // Slide plane2 to the left with updated positions for the current row
            plane2_left -= plane2_slope;
            plane2_right -= plane2_slope;
        }
    }
}
