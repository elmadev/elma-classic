#ifndef HANGHIGH_H
#define HANGHIGH_H

enum {
    WAV_NULL = 0,
    WAV_BUMP = 1,
    WAV_DEAD = 2,
    WAV_WIN = 3,
    WAV_FOOD = 4,
    WAV_TURN = 5,
    WAV_RIGHT_VOLT = 6,
    WAV_LEFT_VOLT = 7,
};

extern bool SoundInitialized;
extern bool Mute;

void sound_engine_init();

void start_motor_sound(bool is_motor1);
void stop_motor_sound(bool is_motor1);

// Set the playback speed of the bike gassing sound effect (capped between 1.0 and 2.0)
void set_motor_frequency(bool is_motor1, double frequency, int gas);

// Set bike squeak sound (0.0 to 1.0)
void set_friction_volume(double volume);

// Start a wavbank event
void start_wav(int event, double volume);

void delay(int milliseconds);

void sound_mixer(short* buffer, int buffer_length);

#endif
