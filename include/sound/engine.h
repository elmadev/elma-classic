#ifndef SOUND_ENGINE_H
#define SOUND_ENGINE_H

enum class WavEvent : char {
    None = 0,
    Bump = 1,
    Dead = 2,
    Win = 3,
    Food = 4,
    Turn = 5,
    RightVolt = 6,
    LeftVolt = 7,
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

void sound_mixer(short* buffer, int buffer_length);

#endif
