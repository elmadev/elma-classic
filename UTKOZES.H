#ifndef UTKOZES_H
#define UTKOZES_H

class vect2;
struct segment;

// Talppontszamot adja vissza (0-2):
int get_two_anchor_points(vect2 r, double radius, vect2* point1, vect2* point2, segment** seg1,
                          segment** seg2);

#endif
