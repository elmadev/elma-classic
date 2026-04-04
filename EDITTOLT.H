#ifndef EDITTOLT_H
#define EDITTOLT_H

#include "object.h"
#include "polygon.h"

class sprite;

void editor_window_open();
int editor_window_save_as();
int editor_window_save();
void editor_new();
void editor_window_level_properties();
void editor_window_welcome();
void editor_window_choose_sprite();
int editor_window_choose_lgr(char* lgrname);
void editor_window_sprite_properties(sprite* spr);
void editor_window_food_properties(const char* title, object::Property* property, int* animation);
void editor_window_polygon_properties(polygon* poly);

extern int ShowPolygons;
extern int ShowGrassPolygons;
extern int ShowObjects;

void editor_window_view_options();

#endif
