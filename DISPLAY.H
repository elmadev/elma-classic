#ifndef DISPLAY_H
#define DISPLAY_H

vect2 pixel_to_meter(int x, int y);
double pixel_to_meter_x(int x);
double pixel_to_meter_y(int y);
int meter_to_pixel_x(double x);
int meter_to_pixel_y(double y);

void zoom(vect2 center, double width);
void zoom_out(void);
void zoom_in(int x1, int y1, int x2, int y2);
void zoom_fill(void);

double get_zoom(void);
double max_grab_distance(void);

extern int RedrawingEditor;
void render_line(vect2 v1, vect2 v2, int dotted);

extern int EDITOR_MENU_X;
extern int EDITOR_MENU_Y;

#endif
