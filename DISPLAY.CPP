#include "ALL.H"
#include "pic8.h"

int RedrawingEditor = 0; // vonalrajzolashoz

int EDITOR_MENU_X = 107;
int EDITOR_MENU_Y = 36;

static int Fxsize = SCREEN_WIDTH;
static int Fysize = SCREEN_HEIGHT;

// static double ZOOM_OUT_LIMIT = 150.0;
static double ZOOM_OUT_LIMIT = 170.0;
static double AspectRatio = double((Fxsize - EDITOR_MENU_X - 1)) / (Fysize - EDITOR_MENU_Y - 1);
static double ZOOM_OUT_LIMIT_HEIGHT = ZOOM_OUT_LIMIT / AspectRatio;

static vect2 CanvasTopLeft(-10.0, -10.0); // CanvasTopLeft a savnelkuli kep bal-felso sarka
static vect2 CanvasBottomRight(-10.0 + 20.0 * AspectRatio, 10.0);

static double CanvasMetersToPixels =
    1.0; // (Fxsize-EDITOR_MENU_X-1)/(CanvasBottomRight.x-CanvasTopLeft.x), zoom allitja
static double CanvasPixelsToMeters = 1.0; // Reciprok

double pixel_to_meter_x(int x) {
    return (x - EDITOR_MENU_X) * CanvasPixelsToMeters + CanvasTopLeft.x;
}

double pixel_to_meter_y(int y) {
    return (y - EDITOR_MENU_Y) * CanvasPixelsToMeters + CanvasTopLeft.y;
}

int meter_to_pixel_x(double x) {
    double pixel_x = (x - CanvasTopLeft.x) * CanvasMetersToPixels;
    return pixel_x + 0.5 + EDITOR_MENU_X;
}

int meter_to_pixel_y(double y) {
    double pixel_y = (y - CanvasTopLeft.y) * CanvasMetersToPixels;
    return pixel_y + 0.5 + EDITOR_MENU_Y;
}

vect2 pixel_to_meter(int x, int y) { return vect2(pixel_to_meter_x(x), pixel_to_meter_y(y)); }

int meter_to_pixel_x(vect2 v) { return meter_to_pixel_x(v.x); }

int meter_to_pixel_y(vect2 v) { return meter_to_pixel_y(v.y); }

static vect2 CanvasCenter;
static double CanvasDiagonalLength = 1.0; // Egy kicsit nagyobb  tenyleges sugarnal

static double ZOOM_IN_LIMIT = 0.017;

// Minden zoom-ot ez allit:
void zoom(vect2 center, double width) {
    if (width < ZOOM_IN_LIMIT) {
        width = ZOOM_IN_LIMIT;
    }
    if (width > ZOOM_OUT_LIMIT) {
        width = ZOOM_OUT_LIMIT;
    }
    CanvasTopLeft.x = center.x - width * 0.5;
    CanvasBottomRight.x = center.x + width * 0.5;
    double height = width / AspectRatio;
    CanvasTopLeft.y = center.y - height * 0.5;
    CanvasBottomRight.y = center.y + height * 0.5;
    CanvasMetersToPixels = (Fxsize - EDITOR_MENU_X - 1) / (CanvasBottomRight.x - CanvasTopLeft.x);
    CanvasPixelsToMeters = 1.0 / CanvasMetersToPixels;
    // Ha palya kilog, kicsit arrebbhelyezzuk:
    double x1, y1, x2, y2;
    Ptop->getminmax(&x1, &y1, &x2, &y2, 1);
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
    // Beallitjuk meg vonalrajzolohoz szukseges paramokat:
    CanvasCenter = (CanvasTopLeft + CanvasBottomRight) * 0.5;
    // Kicsit nagyobbra vesszuk:
    CanvasDiagonalLength = (CanvasBottomRight.x - CanvasTopLeft.x) * sqrt(2.0) / 2.0 * 1.01;
}

void zoom_out(void) {
    zoom((CanvasTopLeft + CanvasBottomRight) * 0.5,
         fabs(CanvasBottomRight.x - CanvasTopLeft.x) * 1.5);
}

static void zoom_in(double x1, double y1, double x2, double y2) {
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
    if (x2 - x1 < 0.0000001 || y2 - y1 < 0.0000001) {
        return;
    }
    vect2 center((x2 + x1) * 0.5, (y2 + y1) * 0.5);
    if ((x2 - x1) / (y2 - y1) > AspectRatio) {
        // Vizszintesen hosszukasabb:
        zoom(center, x2 - x1);
    } else {
        // Fuggolegesen hosszukasabb:
        zoom(center, (y2 - y1) * AspectRatio);
    }
}

void zoom_in(int x1, int y1, int x2, int y2) {
    zoom_in(pixel_to_meter_x(x1), pixel_to_meter_y(y1), pixel_to_meter_x(x2), pixel_to_meter_y(y2));
}

void zoom_fill(void) {
    double x1, y1, x2, y2;
    Ptop->getminmax(&x1, &y1, &x2, &y2, 1);
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

// VONAL VONAL VONAL VONAL VONAL VONAL VONAL VONAL VONAL VONAL VONAL VONAL
// VONAL VONAL VONAL VONAL VONAL VONAL VONAL VONAL VONAL VONAL VONAL VONAL
// VONAL VONAL VONAL VONAL VONAL VONAL VONAL VONAL VONAL VONAL VONAL VONAL
// VONAL VONAL VONAL VONAL VONAL VONAL VONAL VONAL VONAL VONAL VONAL VONAL

void render_line(vect2 v1, vect2 v2, int dotted) {
    if ((v2 - v1).length() < 0.00000001) {
        return;
    }
    // Eloszor is levagjuk p1-p2 szakaszt, ha kilog CanvasCenter es CanvasDiagonalLength
    // altal meghatarozott korbol:
    int v1_off_screen = 0;
    if ((v1 - CanvasCenter).length() > CanvasDiagonalLength * 0.999) {
        v1_off_screen = 1;
    }
    int v2_off_screen = 0;
    if ((v2 - CanvasCenter).length() > CanvasDiagonalLength * 0.999) {
        v2_off_screen = 1;
    }
    if (v1_off_screen && v2_off_screen) {
        if (point_segment_distance(CanvasCenter, v1, v2 - v1) / CanvasDiagonalLength > 0.998) {
            return;
        }
    }
    // Most mar biztos hogy legalabb belelog korbe szakasz
    if (v1_off_screen) {
        vect2 new_v1;
        if (!line_circle_intersection(v1, v2 - v1, CanvasCenter, CanvasDiagonalLength, &new_v1)) {
            internal_error("vonal !line_circle_intersection!");
        }
        v1 = new_v1;
    }
    if (v2_off_screen) {
        vect2 new_v2;
        if (!line_circle_intersection(v2, v1 - v2, CanvasCenter, CanvasDiagonalLength, &new_v2)) {
            internal_error("vonal !line_circle_intersection!");
        }
        v2 = new_v2;
    }
    // Most mar minden pont jo kozel van
    // Most atkonvertalunk int-es koordinata rendszerbe, de
    // floating pointnal maradunk es meg 0.5-ot is csak vegen adjuk hozza:
    // (savot viszont mar le kell takarni):
    double x1 = (v1.x - CanvasTopLeft.x) * CanvasMetersToPixels + EDITOR_MENU_X;
    double y1 = (v1.y - CanvasTopLeft.y) * CanvasMetersToPixels + EDITOR_MENU_Y;
    double x2 = (v2.x - CanvasTopLeft.x) * CanvasMetersToPixels + EDITOR_MENU_X;
    double y2 = (v2.y - CanvasTopLeft.y) * CanvasMetersToPixels + EDITOR_MENU_Y;
    // Beallitjuk helyes sorrendet:
    /*if( x2 < x1 ) {
        double tmp = x1;
        x1 = x2;
        x2 = tmp;
    }
    if( y2 < y1 ) {
        double tmp = y1;
        y1 = y2;
        y2 = tmp;
    } */
    if (fabs(x2 - x1) > fabs(y2 - y1)) {
        // Vonal inkabb vizszintes:
        if (x2 == x1) {
            internal_error("render_line x2 == x1!");
        }

        int inverted = 0;
        if (x2 < x1) {
            inverted = 1;
            double tmp = x1;
            x1 = x2;
            x2 = tmp;
            tmp = y1;
            y1 = y2;
            y2 = tmp;
        }

        // Egyenes parametereinek meghatarozasa:
        double slope = (y2 - y1) / (x2 - x1);
        double y0 = y1 - slope * x1;

        int xstart = x1 + 0.5;
        int xend = x2 + 0.5;
        if (inverted) {
            xstart++;
        } else {
            xend--;
        }
        for (int x = xstart; x <= xend; x++) {
            if (dotted && x % 4) {
                continue;
            }
            double yd = y0 + slope * x;
            int y = yd + 0.5;
            if (x >= EDITOR_MENU_X && y >= EDITOR_MENU_Y && x < Fxsize && y < Fysize) {
                unsigned char pal_index = BufferMain->gpixel(x, y);
                pal_index += 128;
                BufferMain->ppixel(x, y, pal_index);
                if (!RedrawingEditor) {
                    ppixelfront(x, y, pal_index);
                }
            }
        }
    } else {
        // Vonal inkabb fuggoleges:
        if (y2 == y1) {
            internal_error("render_line y2 == y1!");
        }

        int inverted = 0;
        if (y2 < y1) {
            inverted = 1;
            double tmp = x1;
            x1 = x2;
            x2 = tmp;
            tmp = y1;
            y1 = y2;
            y2 = tmp;
        }

        // Egyenes parametereinek meghatarozasa:
        double slope = (x2 - x1) / (y2 - y1);
        double x0 = x1 - slope * y1;

        int yend = y2 + 0.5;
        if (!inverted) {
            yend--;
        }
        for (int y = y1 + 0.5; y <= yend; y++) {
            if (dotted && y % 4) {
                continue;
            }
            double xd = x0 + slope * y;
            int x = xd + 0.5;
            if (x >= EDITOR_MENU_X && y >= EDITOR_MENU_Y && x < Fxsize && y < Fysize) {
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

double get_zoom(void) {
    if (fabs(CanvasBottomRight.x - CanvasTopLeft.x) == 0.0) {
        internal_error("get_zoom 0!");
    }
    return ZOOM_OUT_LIMIT / fabs(CanvasBottomRight.x - CanvasTopLeft.x);
}

// Visszaadja kepernyon tiz pixel tavolsagat double leptekben:
double max_grab_distance(void) { return fabs(pixel_to_meter_x(110) - pixel_to_meter_x(100)); }
