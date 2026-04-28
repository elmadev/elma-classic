#ifndef EDITTOOL_H
#define EDITTOOL_H

#include "object.h"

void draw_tooltip_help(); // Base tooltip for current tool when tool has not yet been used

void tool_move_leftclick(int mouse_x, int mouse_y);
void tool_move_rightclick(int mouse_x, int mouse_y);
void tool_move_esc();
void tool_move_mousemove(int mouse_x, int mouse_y);

extern int CreatingPolygon;
void tool_create_vertex_leftclick(int mouse_x, int mouse_y);
void tool_create_vertex_esc();
void tool_create_vertex_space();
void tool_create_vertex_enter();
void tool_create_vertex_mousemove(int mouse_x, int mouse_y);

void tool_delete_vertex_leftclick(int mouse_x, int mouse_y);
void tool_delete_polygon_leftclick(int mouse_x, int mouse_y);

extern object::Property DefaultFoodProperty;
extern int DefaultFoodAnimation;
void tool_create_food_rightclick();
void tool_create_object_leftclick(int mouse_x, int mouse_y, int is_food);
void tool_delete_object_leftclick(int mouse_x, int mouse_y);

void tool_create_sprite_leftclick(int mouse_x, int mouse_y);
void tool_create_sprite_rightclick();
void tool_delete_sprite_leftclick(int mouse_x, int mouse_y);

extern int SelectingZoomInBox; // True if we selected the top-left corner of zoom in box
void tool_zoom_in_leftclick(int mouse_x, int mouse_y);
void tool_zoom_in_esc();
void tool_zoom_in_mousemove(int mouse_x, int mouse_y);

#endif
