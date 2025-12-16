#ifndef PHYSICS_MOVE_H
#define PHYSICS_MOVE_H

void rigidbody_movement(rigidbody* rb, vekt2 force, double torque, double dt, int do_collision);

void body_movement(motorst* mot, vekt2 gravity, vekt2 i, vekt2 j, double dt);

#endif
