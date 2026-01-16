#ifndef UTKOZES2_H
#define UTKOZES2_H

class vect2;

// Return the index of the first object that a head/wheel touches, or -1 if none.
int get_touching_object(vect2 r, double radius);

#endif
