#include "ALL.H"
#include "pic8.h"

int RedrawingEditor = 0; // vonalrajzolashoz

int EDITOR_MENU_X = 107;
int EDITOR_MENU_Y = 36;

static int Fxsize = SCREEN_WIDTH;
static int Fysize = SCREEN_HEIGHT;

// static double Maxxmeret = 150.0;
static double Maxxmeret = 170.0;
static double Ratio = double((Fxsize - EDITOR_MENU_X - 1)) / (Fysize - EDITOR_MENU_Y - 1);
static double Maxymeret = Maxxmeret / Ratio;

static vect2 Dw1(-10.0, -10.0); // Dw1 a savnelkuli kep bal-felso sarka
static vect2 Dw2(-10.0 + 20.0 * Ratio, 10.0);

static double Nagyitas = 1.0;     // (Fxsize-EDITOR_MENU_X-1)/(Dw2.x-Dw1.x), zoom allitja
static double Nagyitas_rec = 1.0; // Reciprok

double pixel_to_meter_x(int x) { return (x - EDITOR_MENU_X) * Nagyitas_rec + Dw1.x; }

double pixel_to_meter_y(int y) { return (y - EDITOR_MENU_Y) * Nagyitas_rec + Dw1.y; }

int meter_to_pixel_x(double x) {
    double outx = (x - Dw1.x) * Nagyitas;
    return outx + 0.5 + EDITOR_MENU_X;
}

int meter_to_pixel_y(double y) {
    double outy = (y - Dw1.y) * Nagyitas;
    return outy + 0.5 + EDITOR_MENU_Y;
}

vect2 pixel_to_meter(int x, int y) { return vect2(pixel_to_meter_x(x), pixel_to_meter_y(y)); }

int meter_to_pixel_x(vect2 v) { return meter_to_pixel_x(v.x); }

int meter_to_pixel_y(vect2 v) { return meter_to_pixel_y(v.y); }

static vect2 Vonalkozep;
static double Vonalsugar = 1.0; // Egy kicsit nagyobb  tenyleges sugarnal

static double Xsizemin = 0.017;

// Minden zoom-ot ez allit:
void zoom(vect2 center, double width) {
    if (width < Xsizemin) {
        width = Xsizemin;
    }
    if (width > Maxxmeret) {
        width = Maxxmeret;
    }
    Dw1.x = center.x - width * 0.5;
    Dw2.x = center.x + width * 0.5;
    double ysize = width / Ratio;
    Dw1.y = center.y - ysize * 0.5;
    Dw2.y = center.y + ysize * 0.5;
    Nagyitas = (Fxsize - EDITOR_MENU_X - 1) / (Dw2.x - Dw1.x);
    Nagyitas_rec = 1.0 / Nagyitas;
    // Ha palya kilog, kicsit arrebbhelyezzuk:
    double x1, y1, x2, y2;
    Ptop->getminmax(&x1, &y1, &x2, &y2, 1);
    double dx = 0.0;
    double dy = 0.0;
    if (Dw1.x < x2 - Maxxmeret) {
        dx = (x2 - Maxxmeret) - Dw1.x;
    }
    if (Dw2.x > x1 + Maxxmeret) {
        dx = -(Dw2.x - (x1 + Maxxmeret));
    }
    if (Dw1.y < y2 - Maxymeret) {
        dy = (y2 - Maxymeret) - Dw1.y;
    }
    if (Dw2.y > y1 + Maxymeret) {
        dy = -(Dw2.y - (y1 + Maxymeret));
    }
    Dw1.x += dx;
    Dw1.y += dy;
    Dw2.x += dx;
    Dw2.y += dy;
    // Beallitjuk meg vonalrajzolohoz szukseges paramokat:
    Vonalkozep = (Dw1 + Dw2) * 0.5;
    // Kicsit nagyobbra vesszuk:
    Vonalsugar = (Dw2.x - Dw1.x) * sqrt(2.0) / 2.0 * 1.01;
}

void zoom_out(void) { zoom((Dw1 + Dw2) * 0.5, fabs(Dw2.x - Dw1.x) * 1.5); }

static void zoomin_double(double x1, double y1, double x2, double y2) {
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
    vect2 kozep((x2 + x1) * 0.5, (y2 + y1) * 0.5);
    if ((x2 - x1) / (y2 - y1) > Ratio) {
        // Vizszintesen hosszukasabb:
        zoom(kozep, x2 - x1);
    } else {
        // Fuggolegesen hosszukasabb:
        zoom(kozep, (y2 - y1) * Ratio);
    }
}

void zoom_in(int x1, int y1, int x2, int y2) {
    zoomin_double(pixel_to_meter_x(x1), pixel_to_meter_y(y1), pixel_to_meter_x(x2),
                  pixel_to_meter_y(y2));
}

void zoom_fill(void) {
    double x1, y1, x2, y2;
    Ptop->getminmax(&x1, &y1, &x2, &y2, 1);
    double x0 = (x1 + x2) * 0.5;
    double xfels = (x2 - x1) * 0.5 * 1.05;
    double y0 = (y1 + y2) * 0.5;
    double yfels = (y2 - y1) * 0.5 * 1.05;
    x1 = x0 - xfels;
    x2 = x0 + xfels;
    y1 = y0 - yfels;
    y2 = y0 + yfels;
    zoomin_double(x1, y1, x2, y2);
}

// VONAL VONAL VONAL VONAL VONAL VONAL VONAL VONAL VONAL VONAL VONAL VONAL
// VONAL VONAL VONAL VONAL VONAL VONAL VONAL VONAL VONAL VONAL VONAL VONAL
// VONAL VONAL VONAL VONAL VONAL VONAL VONAL VONAL VONAL VONAL VONAL VONAL
// VONAL VONAL VONAL VONAL VONAL VONAL VONAL VONAL VONAL VONAL VONAL VONAL

void render_line(vect2 v1, vect2 v2, int dotted) {
    if ((v2 - v1).length() < 0.00000001) {
        return;
    }
    // Eloszor is levagjuk p1-p2 szakaszt, ha kilog Vonalkozep es Vonalsugar
    // altal meghatarozott korbol:
    int p1kilog = 0;
    if ((v1 - Vonalkozep).length() > Vonalsugar * 0.999) {
        p1kilog = 1;
    }
    int p2kilog = 0;
    if ((v2 - Vonalkozep).length() > Vonalsugar * 0.999) {
        p2kilog = 1;
    }
    if (p1kilog && p2kilog) {
        if (point_segment_distance(Vonalkozep, v1, v2 - v1) / Vonalsugar > 0.998) {
            return;
        }
    }
    // Most mar biztos hogy legalabb belelog korbe szakasz
    if (p1kilog) {
        vect2 ujp1;
        if (!line_circle_intersection(v1, v2 - v1, Vonalkozep, Vonalsugar, &ujp1)) {
            internal_error("vonal-ban !line_circle_intersection!");
        }
        v1 = ujp1;
    }
    if (p2kilog) {
        vect2 ujp2;
        if (!line_circle_intersection(v2, v1 - v2, Vonalkozep, Vonalsugar, &ujp2)) {
            internal_error("vonal-ban !line_circle_intersection!");
        }
        v2 = ujp2;
    }
    // Most mar minden pont jo kozel van
    // Most atkonvertalunk int-es koordinata rendszerbe, de
    // floating pointnal maradunk es meg 0.5-ot is csak vegen adjuk hozza:
    // (savot viszont mar le kell takarni):
    double x1 = (v1.x - Dw1.x) * Nagyitas + EDITOR_MENU_X;
    double y1 = (v1.y - Dw1.y) * Nagyitas + EDITOR_MENU_Y;
    double x2 = (v2.x - Dw1.x) * Nagyitas + EDITOR_MENU_X;
    double y2 = (v2.y - Dw1.y) * Nagyitas + EDITOR_MENU_Y;
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
            internal_error("vonal-ban x2 == x1!");
        }

        int forditott = 0;
        if (x2 < x1) {
            forditott = 1;
            double tmp = x1;
            x1 = x2;
            x2 = tmp;
            tmp = y1;
            y1 = y2;
            y2 = tmp;
        }

        // Egyenes parametereinek meghatarozasa:
        double m = (y2 - y1) / (x2 - x1);
        double y0 = y1 - m * x1;

        int xkezd = x1 + 0.5;
        int xveg = x2 + 0.5;
        if (forditott) {
            xkezd++;
        } else {
            xveg--;
        }
        for (int x = xkezd; x <= xveg; x++) {
            if (dotted && x % 4) {
                continue;
            }
            double yd = y0 + m * x;
            int y = yd + 0.5;
            if (x >= EDITOR_MENU_X && y >= EDITOR_MENU_Y && x < Fxsize && y < Fysize) {
                unsigned char szin = BufferMain->gpixel(x, y);
                szin += 128;
                BufferMain->ppixel(x, y, szin);
                if (!RedrawingEditor) {
                    ppixelfront(x, y, szin);
                }
            }
        }
    } else {
        // Vonal inkabb fuggoleges:
        if (y2 == y1) {
            internal_error("vonal-ban y2 == y1!");
        }

        int forditott = 0;
        if (y2 < y1) {
            forditott = 1;
            double tmp = x1;
            x1 = x2;
            x2 = tmp;
            tmp = y1;
            y1 = y2;
            y2 = tmp;
        }

        // Egyenes parametereinek meghatarozasa:
        double m = (x2 - x1) / (y2 - y1);
        double x0 = x1 - m * y1;

        int yveg = y2 + 0.5;
        if (!forditott) {
            yveg--;
        }
        for (int y = y1 + 0.5; y <= yveg; y++) {
            if (dotted && y % 4) {
                continue;
            }
            double xd = x0 + m * y;
            int x = xd + 0.5;
            if (x >= EDITOR_MENU_X && y >= EDITOR_MENU_Y && x < Fxsize && y < Fysize) {
                unsigned char szin = BufferMain->gpixel(x, y);
                szin += 128;
                BufferMain->ppixel(x, y, szin);
                if (!RedrawingEditor) {
                    ppixelfront(x, y, szin);
                }
            }
        }
    }
}

double get_zoom(void) {
    if (fabs(Dw2.x - Dw1.x) == 0.0) {
        internal_error("get_zoom-ban 0-val osztas!");
    }
    return Maxxmeret / fabs(Dw2.x - Dw1.x);
}

// Visszaadja kepernyon tiz pixel tavolsagat double leptekben:
double max_grab_distance(void) { return fabs(pixel_to_meter_x(110) - pixel_to_meter_x(100)); }

// Ezek csak debughoz kellettek (nincsen h-juk):
/*void saveallapot( void ) {
    FILE* h = fopen( "display.sta", "wb" );
    if( !h )
        internal_error( "Nem nyilik display.sta!" );

    fwrite( &Dw1.x, 1, sizeof( Dw1.x ), h );
    fwrite( &Dw2.x, 1, sizeof( Dw2.x ), h );
    fwrite( &Dw1.y, 1, sizeof( Dw1.y ), h );
    fwrite( &Dw2.y, 1, sizeof( Dw2.y ), h );

    fwrite( &Nagyitas, 1, sizeof( Nagyitas ), h );
    fwrite( &Nagyitas_rec, 1, sizeof( Nagyitas_rec ), h );
    fwrite( &Vonalkozep, 1, sizeof( Vonalkozep ), h );
    fwrite( &Vonalsugar, 1, sizeof( Vonalsugar ), h );

    fclose( h );
}
void loadallapot( void ) {
    FILE* h = fopen( "display.sta", "rb" );
    if( !h )
        internal_error( "Nem nyilik display.sta!" );

    fread( &Dw1.x, 1, sizeof( Dw1.x ), h );
    fread( &Dw2.x, 1, sizeof( Dw2.x ), h );
    fread( &Dw1.y, 1, sizeof( Dw1.y ), h );
    fread( &Dw2.y, 1, sizeof( Dw2.y ), h );

    fread( &Nagyitas, 1, sizeof( Nagyitas ), h );
    fread( &Nagyitas_rec, 1, sizeof( Nagyitas_rec ), h );
    fread( &Vonalkozep, 1, sizeof( Vonalkozep ), h );
    fread( &Vonalsugar, 1, sizeof( Vonalsugar ), h );

    fclose( h );
}*/
