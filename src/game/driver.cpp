#include "game/driver.h"
#include "physics/forces.h"

void driver::reset_metadata() {
    meta.sound.motor_frequency = 0.0;
    meta.sound.gas = 0;
    meta.sound.friction_volume = 0.0;

    meta.volt_time = -100.0;
    meta.volt_is_right = false;

    meta.turn_key_previous = false;
    meta.one_turn_used = false;

    meta.arm_position = 0.0;

    meta.bike_turning.flipped = 0;
    meta.bike_turning.turn_time = -1000.0;
    meta.bike_turning.turn_phase = 0.0;

    meta.camera_turning.flipped = 0;
    meta.camera_turning.turn_time = -1000.0;
    meta.camera_turning.turn_phase = 0.0;
}

driver::driver(motorst* mot, recorder* rec, player_keys* keys, hud_visibility* hud)
    : mot(mot),
      rec(rec),
      keys(keys),
      hud(hud) {
    reset_metadata();
    reset_motor_forces(mot);
}
