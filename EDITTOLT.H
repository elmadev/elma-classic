#ifndef EDITTOLT_H
#define EDITTOLT_H

#include "object.h"
#include "polygon.h"

class sprite;

void editor_window_open();
// Ha sikeresen mentettek, akkor igazak:
int editor_window_save_as();
int editor_window_save();
void editor_new();

void editor_window_level_properties();

// Editfilenev alapjan megprobal josolni, majd oda teszi eredmenyt:
void editor_window_welcome();

// Kivalaszt LGR listabol egy sprite-ot:
void editor_window_choose_sprite();

// Siker eseten igaz:
int editor_window_choose_lgr(char* lgrname); // lgrnevbe masol max 8 chart

void editor_window_sprite_properties(sprite* spr);

void editor_window_food_properties(const char* title, object::Property* property, int* animation);

void editor_window_polygon_properties(polygon* poly);

extern int ShowPolygons;
extern int ShowGrassPolygons;
extern int ShowObjects;

void editor_window_view_options();

#endif
