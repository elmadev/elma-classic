#ifndef PHYSICS_MOVE_H
#define PHYSICS_MOVE_H

void beallit(rigidbody* pk, vekt2 F, double M, double dt, int kelltalppont);

void beallitvezeto(motorst* pmot, vekt2 gravitacio, vekt2 i, vekt2 j, double dt);

#endif
