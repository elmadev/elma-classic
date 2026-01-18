#ifndef AFFINE_PIC_RENDER_H
#define AFFINE_PIC_RENDER_H

class affine_pic;
class pic8;
class vect2;

// Apply an affine transformation to a affine_pic (bike graphic) and draw it.
//
//   u = horizontal axis vector
//   v = vertical axis vector
//   r = bottom-left corner of image
//   StretchEnabled if bike is turning (properties set via func set_stretch_parameters)
//
//   See docs/affine_pic_render.png
void draw_affine_pic(pic8* dest, affine_pic* aff, vect2 u, vect2 v, vect2 r);

extern bool StretchEnabled;

// Set the properties of the bike being squished during turning.
// `bike_center` is the centerpoint of the turn.
// `bike_i` is bike right direction (direction of scaling).
// `stretch` is the stretch amount (-1.0 to 1.0)
// `meters_to_pixels_x` / `meters_to_pixels_y` is the zoom amount.
void set_stretch_parameters(vect2 bike_center, vect2 bike_i, double stretch,
                            double meters_to_pixels);

#endif
