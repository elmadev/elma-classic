#ifndef EDITOR_CANVAS_H
#define EDITOR_CANVAS_H

class vect2;

constexpr int EDITOR_MENU_X = 107;
constexpr int EDITOR_MENU_Y = 36;

vect2 pixel_to_meter(int x, int y);
double pixel_to_meter_x(int x);
double pixel_to_meter_y(int y);
int meter_to_pixel_x(double x);
int meter_to_pixel_y(double y);

void zoom(vect2 center, double width);
void zoom_out();
void zoom_in(int x1, int y1, int x2, int y2);
void zoom_fill();

double get_zoom();
double max_grab_distance();

extern bool RedrawingEditor;
void render_line(vect2 v1, vect2 v2, bool dotted);

#endif
