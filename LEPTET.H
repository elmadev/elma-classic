#ifndef LEPTET_H
#define LEPTET_H

struct motorst;

enum class BikeState {
    Dead,
    Finish,
    Normal,
};

double get_bike_friction_volume();
void reset_motor_forces(motorst* mot);
void simulate_bike_physics(motorst* mot, double time, double dt, bool gas, bool brake,
                           bool right_volt, bool left_volt);
void set_head_position(motorst* mot);
BikeState check_object_collision(motorst* mot);

#endif
