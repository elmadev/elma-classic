#ifndef LEPTET_H
#define LEPTET_H

struct motorst;

void reset_motor_forces(motorst* mot);
void simulate_bike_physics(motorst* mot, double time, double dt, bool gas, bool brake,
                           bool right_volt, bool left_volt);
void set_head_position(motorst* mot);
int check_object_collision(motorst* mot);
double get_bike_friction_volume();

#endif
