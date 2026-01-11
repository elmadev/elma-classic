#ifndef UTKOZES_H
#define UTKOZES_H

class vect2;
struct segment;

// Talppontszamot adja vissza (0-2):
int talppontkereses(vect2 r, double sugar, vect2* pt1, vect2* pt2, segment** ppv1, segment** ppv2);

#endif
