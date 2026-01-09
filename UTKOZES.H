#ifndef UTKOZES_H
#define UTKOZES_H

class vect2;
struct vonal;

// Talppontszamot adja vissza (0-2):
int talppontkereses(vect2 r, double sugar, vect2* pt1, vect2* pt2, vonal** ppv1, vonal** ppv2);

#endif
