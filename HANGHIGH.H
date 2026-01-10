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

// Csak program elejen allithato:
// Ha hamis, semmi directsound fv.-t nem hiv:
extern int SoundInitialized;
// Ha igaz, callback fuggveny 0-kat ir bufferbe:
extern int Mute;

void sound_engine_init(void);
void stophanghigh(void);

// Elinditja alapjarat hangot:
void start_motor_sound(int is_motor1);
// Leallitja motor hangjat (ha meghalt):
void stop_motor_sound(int is_motor1);
// frekvencia 1.0-tol 2.0-ig valtozik csak:
void set_motor_frequency(int is_motor1, double frequency, int gas);

void set_friction_volume(double volume);

void start_wav(int event, double volume);
int vanmegwav(void);

void delay(int milliseconds);

// Ezt meg kell irni hozza:
void sound_mixer(short* buffer, int buffer_length);

#endif
