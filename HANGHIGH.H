#ifndef HANGHIGH_H
#define HANGHIGH_H

enum {
    WAV_BUMP = 1,
    WAV_DEAD,
    WAV_WIN,
    WAV_FOOD,
    WAV_TURN,
    WAV_RIGHT_VOLT,
    WAV_LEFT_VOLT,
    WAV_ALAP,
    WAV_GAZ,
    WAV_DORZSOL,
    WAV_TURA
};

extern int SoundInitialized;
extern int Mute;

void sound_engine_init(void);
void stophanghigh(void);

void start_motor_sound(int is_motor1);
void stop_motor_sound(int is_motor1);

// Set the playback speed of the bike gassing sound effect (capped between 1.0 and 2.0)
void set_motor_frequency(int is_motor1, double frequency, int gas);

// Set bike squeak sound (0.0 to 1.0)
void set_friction_volume(double volume);

// Start a wavbank event
void start_wav(int event, double volume);
int vanmegwav(void);

void delay(int milliseconds);

void sound_mixer(short* buffer, int buffer_length);

#endif
