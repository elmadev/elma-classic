#ifndef PHYSICS_COLLISION_H
#define PHYSICS_COLLISION_H

class vect2;

// Get up to two points of collision for the circle at position `r` with `radius`.
// Return the number of anchor points (0-2).
// Store the anchor points in `point1` and `point2`.
int get_two_anchor_points(vect2 r, double radius, vect2* point1, vect2* point2);

// Return the index of the first object that a head/wheel touches, or -1 if none.
int get_touching_object(vect2 r, double radius);

#endif
