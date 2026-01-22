#include "editor_canvas.h"
#include "EDITUJ.H"
#include "level.h"
#include "M_PIC.H"
#include "main.h"
#include "menu_pic.h"
#include "pic8.h"
#include <cmath>

// If false, when drawing lines, immediately update the screen
bool RedrawingEditor = false;

constexpr double ZOOM_IN_LIMIT = 0.017;
constexpr double ZOOM_OUT_LIMIT = 170.0;
static double AspectRatio =
    double((SCREEN_WIDTH - EDITOR_MENU_X - 1)) / (SCREEN_HEIGHT - EDITOR_MENU_Y - 1);
static double ZOOM_OUT_LIMIT_HEIGHT = ZOOM_OUT_LIMIT / AspectRatio;

static vect2 CanvasTopLeft(-10.0, -10.0);
static vect2 CanvasBottomRight(-10.0 + 20.0 * AspectRatio, 10.0);

static double CanvasMetersToPixels = 1.0;
static double CanvasPixelsToMeters = 1.0;

// Convert editor canvas pixel to in-game meters
double pixel_to_meter_x(int x) {
    return (x - EDITOR_MENU_X) * CanvasPixelsToMeters + CanvasTopLeft.x;
}

// Convert editor canvas pixel to in-game meters
double pixel_to_meter_y(int y) {
    return (y - EDITOR_MENU_Y) * CanvasPixelsToMeters + CanvasTopLeft.y;
}

// Convert in-game meter to editor canvas pixels
int meter_to_pixel_x(double x) {
    double pixel_x = (x - CanvasTopLeft.x) * CanvasMetersToPixels;
    return (int)(pixel_x + 0.5 + EDITOR_MENU_X);
}

// Convert in-game meter to editor canvas pixels
int meter_to_pixel_y(double y) {
    double pixel_y = (y - CanvasTopLeft.y) * CanvasMetersToPixels;
    return (int)(pixel_y + 0.5 + EDITOR_MENU_Y);
}

// Convert editor canvas pixel to in-game meters
vect2 pixel_to_meter(int x, int y) { return vect2(pixel_to_meter_x(x), pixel_to_meter_y(y)); }

static vect2 CanvasCenter;
static double CanvasDiagonalLength = 1.0;

// Zoom to specified size
void zoom(vect2 center, double width) {
    // Bound min/max zoom
    if (width < ZOOM_IN_LIMIT) {
        width = ZOOM_IN_LIMIT;
    }
    if (width > ZOOM_OUT_LIMIT) {
        width = ZOOM_OUT_LIMIT;
    }
    // Calculate top-left and bottom-right screen corners
    CanvasTopLeft.x = center.x - width * 0.5;
    CanvasBottomRight.x = center.x + width * 0.5;
    double height = width / AspectRatio;
    CanvasTopLeft.y = center.y - height * 0.5;
    CanvasBottomRight.y = center.y + height * 0.5;

    // Calculate conversion ratio between pixels and meters
    CanvasMetersToPixels =
        (SCREEN_WIDTH - EDITOR_MENU_X - 1) / (CanvasBottomRight.x - CanvasTopLeft.x);
    CanvasPixelsToMeters = 1.0 / CanvasMetersToPixels;

    // Disallow zooming too far away from the level.
    // Nudge the view back towards the center if needed
    double x1, y1, x2, y2;
    Ptop->get_boundaries(&x1, &y1, &x2, &y2, true);
    double dx = 0.0;
    double dy = 0.0;
    if (CanvasTopLeft.x < x2 - ZOOM_OUT_LIMIT) {
        dx = (x2 - ZOOM_OUT_LIMIT) - CanvasTopLeft.x;
    }
    if (CanvasBottomRight.x > x1 + ZOOM_OUT_LIMIT) {
        dx = -(CanvasBottomRight.x - (x1 + ZOOM_OUT_LIMIT));
    }
    if (CanvasTopLeft.y < y2 - ZOOM_OUT_LIMIT_HEIGHT) {
        dy = (y2 - ZOOM_OUT_LIMIT_HEIGHT) - CanvasTopLeft.y;
    }
    if (CanvasBottomRight.y > y1 + ZOOM_OUT_LIMIT_HEIGHT) {
        dy = -(CanvasBottomRight.y - (y1 + ZOOM_OUT_LIMIT_HEIGHT));
    }
    CanvasTopLeft.x += dx;
    CanvasTopLeft.y += dy;
    CanvasBottomRight.x += dx;
    CanvasBottomRight.y += dy;

    // Recalculate center of display and diagonal length from center to corner of display
    CanvasCenter = (CanvasTopLeft + CanvasBottomRight) * 0.5;
    CanvasDiagonalLength = (CanvasBottomRight.x - CanvasTopLeft.x) * sqrt(2.0) / 2.0 * 1.01;
}

void zoom_out() {
    zoom((CanvasTopLeft + CanvasBottomRight) * 0.5,
         fabs(CanvasBottomRight.x - CanvasTopLeft.x) * 1.5);
}

// Zoom in with meters as input
static void zoom_in(double x1, double y1, double x2, double y2) {
    // Normalize so x1, y1 are smaller than x2, y2
    if (x2 < x1) {
        double tmp = x1;
        x1 = x2;
        x2 = tmp;
    }
    if (y2 < y1) {
        double tmp = y1;
        y1 = y2;
        y2 = tmp;
    }
    // Don't zoom if zero width or height
    if (x2 - x1 < 0.0000001 || y2 - y1 < 0.0000001) {
        return;
    }
    // Use either the width or height to zoom. Use the dimension that allows you to see everything
    vect2 center((x2 + x1) * 0.5, (y2 + y1) * 0.5);
    if ((x2 - x1) / (y2 - y1) > AspectRatio) {
        zoom(center, x2 - x1);
    } else {
        zoom(center, (y2 - y1) * AspectRatio);
    }
}

// Zoom in with pixels as input
void zoom_in(int x1, int y1, int x2, int y2) {
    zoom_in(pixel_to_meter_x(x1), pixel_to_meter_y(y1), pixel_to_meter_x(x2), pixel_to_meter_y(y2));
}

void zoom_fill() {
    double x1, y1, x2, y2;
    Ptop->get_boundaries(&x1, &y1, &x2, &y2, true);
    double x_center = (x1 + x2) * 0.5;
    double x_edge = (x2 - x1) * 0.5 * 1.05;
    double y_center = (y1 + y2) * 0.5;
    double y_edge = (y2 - y1) * 0.5 * 1.05;
    x1 = x_center - x_edge;
    x2 = x_center + x_edge;
    y1 = y_center - y_edge;
    y2 = y_center + y_edge;
    zoom_in(x1, y1, x2, y2);
}

// Draw one line into the editor canvas from v1 to v2
void render_line(vect2 v1, vect2 v2, bool dotted) {
    // Skip very short lines
    if ((v2 - v1).length() < 0.00000001) {
        return;
    }
    // Check to see if the endpoints are outside of the screen render zone
    bool v1_off_screen = (v1 - CanvasCenter).length() > CanvasDiagonalLength * 0.999;
    bool v2_off_screen = (v2 - CanvasCenter).length() > CanvasDiagonalLength * 0.999;
    // If both points are outside the screen render zone, calculate the closest distance between the
    // center of the screen and the line. If this line isn't within the radius of the screen render
    // zone, we don't need to render the line at all
    if (v1_off_screen && v2_off_screen) {
        if (point_segment_distance(CanvasCenter, v1, v2 - v1) / CanvasDiagonalLength > 0.998) {
            return;
        }
    }
    // For each point that is out of screen, shorten the line to only be near the screen render zone
    // Shorten the line assuming the screen is a circle, and getting the intersection between this
    // circle and the line
    if (v1_off_screen) {
        if (!line_circle_intersection(v1, v2 - v1, CanvasCenter, CanvasDiagonalLength, &v1)) {
            internal_error("vonal !line_circle_intersection!");
        }
    }
    if (v2_off_screen) {
        if (!line_circle_intersection(v2, v1 - v2, CanvasCenter, CanvasDiagonalLength, &v2)) {
            internal_error("vonal !line_circle_intersection!");
        }
    }
    // Convert line coordinates to pixels
    double x1 = (v1.x - CanvasTopLeft.x) * CanvasMetersToPixels + EDITOR_MENU_X;
    double y1 = (v1.y - CanvasTopLeft.y) * CanvasMetersToPixels + EDITOR_MENU_Y;
    double x2 = (v2.x - CanvasTopLeft.x) * CanvasMetersToPixels + EDITOR_MENU_X;
    double y2 = (v2.y - CanvasTopLeft.y) * CanvasMetersToPixels + EDITOR_MENU_Y;
    // Draw along the longest axis
    if (fabs(x2 - x1) > fabs(y2 - y1)) {
        // Draw along x
        if (x2 == x1) {
            internal_error("render_line x2 == x1!");
        }

        // Invert the points so x1 < x2 if needed
        bool inverted = false;
        if (x2 < x1) {
            inverted = true;
            double tmp = x1;
            x1 = x2;
            x2 = tmp;
            tmp = y1;
            y1 = y2;
            y2 = tmp;
        }

        // Calculate slope and y-intercept
        double slope = (y2 - y1) / (x2 - x1);
        double y0 = y1 - slope * x1;

        // Draw from x1 + 0.5 to x2 + 0.5
        // but remove the last pixel of the line to avoid double-drawing where two lines connect
        int xstart = (int)(x1 + 0.5);
        int xend = (int)(x2 + 0.5);
        if (inverted) {
            xstart++;
        } else {
            xend--;
        }
        // Draw the line
        for (int x = xstart; x <= xend; x++) {
            if (dotted && x % 4) {
                continue;
            }
            // Calculate the y position and draw it if it is within the internal editor render box
            double yd = y0 + slope * x;
            int y = (int)(yd + 0.5);
            if (x >= EDITOR_MENU_X && y >= EDITOR_MENU_Y && x < SCREEN_WIDTH && y < SCREEN_HEIGHT) {
                // Invert the palette ID
                unsigned char pal_index = BufferMain->gpixel(x, y);
                pal_index += 128;
                BufferMain->ppixel(x, y, pal_index);
                if (!RedrawingEditor) {
                    ppixelfront(x, y, pal_index);
                }
            }
        }
    } else {
        // Draw along y
        if (y2 == y1) {
            internal_error("render_line y2 == y1!");
        }

        // Invert the points so y1 < y2 if needed
        bool inverted = false;
        if (y2 < y1) {
            inverted = true;
            double tmp = x1;
            x1 = x2;
            x2 = tmp;
            tmp = y1;
            y1 = y2;
            y2 = tmp;
        }

        // Calculate slope and x-intercept
        double slope = (x2 - x1) / (y2 - y1);
        double x0 = x1 - slope * y1;

        // Draw from y1 + 0.5 to y2 + 0.5
        // but remove the last pixel of the line to avoid double-drawing where two lines connect
        int ystart = (int)(y1 + 0.5);
        int yend = (int)(y2 + 0.5);
        if (inverted) {
            ystart++;
        } else {
            yend--;
        }
        // Draw the line
        for (int y = ystart; y <= yend; y++) {
            if (dotted && y % 4) {
                continue;
            }
            // Calculate the x position and draw it if it is within the internal editor render box
            double xd = x0 + slope * y;
            int x = (int)(xd + 0.5);
            if (x >= EDITOR_MENU_X && y >= EDITOR_MENU_Y && x < SCREEN_WIDTH && y < SCREEN_HEIGHT) {
                // Invert the palette ID
                unsigned char pal_index = BufferMain->gpixel(x, y);
                pal_index += 128;
                BufferMain->ppixel(x, y, pal_index);
                if (!RedrawingEditor) {
                    ppixelfront(x, y, pal_index);
                }
            }
        }
    }
}

// Get a number representing our zoom, where 1.000 represents the most you can zoom out in Elma 1.11
double get_zoom() {
    if (fabs(CanvasBottomRight.x - CanvasTopLeft.x) == 0.0) {
        internal_error("get_zoom 0!");
    }
    return ZOOM_OUT_LIMIT / fabs(CanvasBottomRight.x - CanvasTopLeft.x);
}

// Convert a length of 10 pixels to elmameters, as 10 pixels is the max grab distance for the cursor
double max_grab_distance() {
    constexpr int MAX_GRAB_DISTANCE = 10;
    return fabs(pixel_to_meter_x(100 + MAX_GRAB_DISTANCE) - pixel_to_meter_x(100));
}
