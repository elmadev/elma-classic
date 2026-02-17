#ifndef OBJECT_OVERLAY_H
#define OBJECT_OVERLAY_H

#include "object.h"

class pic8;

void init_gravity_arrows();
void draw_gravity_arrow(pic8* pic, int obj_i, int obj_j, object::Property property);
#endif
