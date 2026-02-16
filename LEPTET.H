#ifndef LEPTET_H
#define LEPTET_H

struct motorst;

void reset_motor_forces(motorst* mot);
void simulate_bike_physics(motorst* mot, double time, double dt, int gas, int brake, int right_volt,
                           int left_volt);

// csak replay eseten kell meghivni, mivel leptet elvegzi:
void set_head_position(motorst* mot);

// 0-meghalt, 2-semmi kulonos:
int check_object_collision(motorst* mot);

double get_bike_friction_volume(void);

#endif
