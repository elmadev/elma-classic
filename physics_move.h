#ifndef PHYSICS_MOVE_H
#define PHYSICS_MOVE_H

struct motorst;
struct rigidbody;
class vect2;

void rigidbody_movement(rigidbody* rb, vect2 force, double torque, double dt, bool do_collision);
void body_movement(motorst* mot, vect2 gravity, vect2 i, vect2 j, double dt);

#endif
