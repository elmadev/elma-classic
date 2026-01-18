#ifndef DOBOZ_H
#define DOBOZ_H

class affine_pic;
class pic8;
class vect2;

void draw_affine_pic(pic8* dest, affine_pic* aff, vect2 u, vect2 v, vect2 r);

extern int StretchEnabled;
// kozepet es iranyt majd o megszorozza arany-okkal:
void set_stretch_parameters(vect2 bike_center, vect2 bike_i, double stretch,
                            double meters_to_pixels_x, double meters_to_pixels_y);

#endif
