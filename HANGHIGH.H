#ifndef HANGHIGH_H
#define HANGHIGH_H

enum class WavEvent : char {
    NONE = 0,
    BUMP = 1,
    DEAD = 2,
    WIN = 3,
    FOOD = 4,
    TURN = 5,
    RIGHT_VOLT = 6,
    LEFT_VOLT = 7,
};

extern bool Mute;

void sound_engine_init();

void start_motor_sound(bool is_motor1);
void stop_motor_sound(bool is_motor1);

// Set the playback speed of the bike gassing sound effect (capped between 1.0 and 2.0)
void set_motor_frequency(bool is_motor1, double frequency, int gas);

// Set bike squeak sound (0.0 to 1.0)
void set_friction_volume(double volume);

// Start a wavbank event
void start_wav(WavEvent event, double volume);

void delay(int milliseconds);

void sound_mixer(short* buffer, int buffer_length);

#endif
