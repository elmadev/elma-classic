#include "HANGHIGH.H"
#include "main.h"
#include "platform_impl.h"
#include "TOPOL.H"
#include "WAV.H"
#include <cstring>

int SoundInitialized = 0;

int Mute = 1;

#define WAV_BANK_LENGTH (20)
typedef wav* wavramut;
wavramut WavBank[WAV_BANK_LENGTH];

static wav *SoundMotorIgnition = NULL, *SoundMotorIdle = NULL, *SoundMotorGasStart = NULL,
           *SoundMotorGas = NULL, *SoundFriction = NULL;
static wav2 *SoundMotorGas1 = NULL, *SoundMotorGas2 = NULL;

static int WAV_FADE_LENGTH = 100, HARL_MAX_INDEX_1 = 7526, HARL_MAX_INDEX_2 = 34648,
           HARL_MAX_INDEX_3 = 38766, HARL2_MIN_INDEX = 14490, HARL2_MAX_INDEX = 18906;

static double WAV_FADE_LENGTH_RECIPROCAL = 1.0 / WAV_FADE_LENGTH;

// static int Buffsize = 500;
static int Atmenet = 499;

static int SoundEngineInitialized = 0;

static int ActiveWavEvents = 0;
#define MAX_WAV_EVENTS (5)
static int WavEventActive[MAX_WAV_EVENTS];
static int WavEventPlaybackIndex[MAX_WAV_EVENTS];
static unsigned long WavEventVolume[MAX_WAV_EVENTS];
static wav* WavEventSound[MAX_WAV_EVENTS];

static int SoundEngineUninitialized = 1;

// Load all sounds into memory
void sound_engine_init(void) {
    if (!SoundEngineUninitialized) {
        internal_error("sound_engine_init !SoundEngineUninitialized!");
    }
    SoundEngineUninitialized = 0;
    if (!SoundInitialized) {
        return;
    }

    Mute = 1;
    if (SoundEngineInitialized) {
        internal_error("SoundEngineInitialized igaz sound_engine_init-ban!");
    }
    SoundEngineInitialized = 1;

    // Load Wavbank
    for (int i = 0; i < WAV_BANK_LENGTH; i++) {
        WavBank[i] = NULL;
    }
    WavBank[WAV_BUMP] = new wav("utodes.wav", 0.25);      // collision
    WavBank[WAV_DEAD] = new wav("torik.wav", 0.34);       // shatter
    WavBank[WAV_WIN] = new wav("siker.wav", 0.8);         // success
    WavBank[WAV_FOOD] = new wav("eves.wav", 0.5);         // eating
    WavBank[WAV_TURN] = new wav("fordul.wav", 0.3);       // turn
    WavBank[WAV_RIGHT_VOLT] = new wav("ugras.wav", 0.34); // jump
    WavBank[WAV_LEFT_VOLT] = WavBank[WAV_RIGHT_VOLT];

    // Bike squeak sound
    SoundFriction = new wav("dorzsol.wav", 0.44); // friction
    SoundFriction->loopol(WAV_FADE_LENGTH);

    // Bike engine sound
    double volume = 0.26;
    // Filename possibly a reference to Harley-Davidson motorcycles
    SoundMotorIgnition = new wav("harl.wav", volume, 0, HARL_MAX_INDEX_1 + WAV_FADE_LENGTH);
    SoundMotorIdle = new wav("harl.wav", volume, HARL_MAX_INDEX_1, HARL_MAX_INDEX_2);
    SoundMotorGasStart =
        new wav("harl.wav", volume, HARL_MAX_INDEX_2 - WAV_FADE_LENGTH, HARL_MAX_INDEX_3);
    SoundMotorGas = new wav("harl2.wav", volume, HARL2_MIN_INDEX, HARL2_MAX_INDEX);

    // Post-process bike sounds
    SoundMotorIdle->loopol(WAV_FADE_LENGTH);
    SoundMotorGas->loopol(WAV_FADE_LENGTH);
    SoundMotorGasStart->vegereilleszt(SoundMotorGas, WAV_FADE_LENGTH);

    // Make gas sound able to be played back at different speeds
    SoundMotorGas1 = new wav2(SoundMotorGas);
    SoundMotorGas1->reset();
    SoundMotorGas2 = new wav2(SoundMotorGas);
    SoundMotorGas2->reset();

    // Initialize wavbank playing sounds buffer
    for (int i = 0; i < MAX_WAV_EVENTS; i++) {
        WavEventActive[i] = 0;
        WavEventPlaybackIndex[i] = 0;
        WavEventVolume[i] = 0;
        WavEventSound[i] = NULL;
    }
    Mute = 0;

    // Change from across: lower the volume of idle motor
    // We could have directly done this above as the file was loaded, but we didn't
    SoundMotorIdle->hangero(0.4);
}

// Which sound the motor is currently generating
enum MotorState {
    MOTOR_IGNITION = 0,
    MOTOR_IDLE,
    MOTOR_IDLE_TO_GAS_TRANSITION,
    MOTOR_GAS_START,
    MOTOR_GASSING,
    A_VISSZAMENET
};

// Sound state of one bike
struct motor_sound {
    int enabled;
    double frequency_prev;
    double frequency_next;
    int gas;
    int motor_state;
    int playback_index_ignition;
    int playback_index_idle;
    int playback_index_gas_start;
};

static motor_sound MotorSound1, MotorSound2;

// Set the playback speed of the bike gassing sound effect (capped between 1.0 and 2.0)
void set_motor_frequency(int is_motor1, double frequency, int gas) {
    if (!SoundInitialized) {
        return;
    }

    motor_sound* mot;
    if (is_motor1) {
        mot = &MotorSound1;
    } else {
        mot = &MotorSound2;
    }

    mot->gas = gas;
    if (frequency > 2.0) {
        frequency = 2.0;
    }
    if (frequency < 1.0) {
        frequency = 0.0;
    }

    mot->frequency_next = frequency;
}

static double FrictionVolumePrev = 0.0;
static double FrictionVolumeNext = 0.0;

// Set bike squeak sound (0.0 to 1.0)
void set_friction_volume(double volume) {
    if (volume > 1.0) {
        volume = 1.0;
    }
    if (volume < 0) {
        volume = 0;
    }
    FrictionVolumeNext = volume;
}

// Start a wavbank event
void start_wav(int event, double volume) {
    if (!SoundInitialized || Mute) {
        return;
    }

    if (volume <= 0.0 || volume >= 1.0) {
        internal_error("start_wav volume <= 0.0 || volume >= 1.0!");
    }

    wav* sound = WavBank[event];

    if (ActiveWavEvents >= MAX_WAV_EVENTS) {
        return;
    }

    for (int i = 0; i < MAX_WAV_EVENTS; i++) {
        if (!WavEventActive[i]) {
            ActiveWavEvents++;
            WavEventActive[i] = 1;
            WavEventPlaybackIndex[i] = 0;
            WavEventSound[i] = sound;
            WavEventVolume[i] = volume * 65536.0;
            return;
        }
    }
    internal_error("start_wav Unable to find free wav slot!");
}

static void mix_into_buffer(short* buffer, short* source, long buffer_length,
                            unsigned long volume) {
    for (unsigned long i = 0; i < buffer_length; i++) {
        buffer[i] += short((source[i] * volume) >> 16);
    }
}

// Initialize motor sound struct
void start_motor_sound(int is_motor1) {
    motor_sound* mot;
    if (is_motor1) {
        mot = &MotorSound1;
    } else {
        mot = &MotorSound2;
    }

    mot->enabled = 1;
    mot->motor_state = MOTOR_IGNITION;
    mot->playback_index_ignition = 0;
    mot->frequency_prev = 1.0;
    mot->frequency_next = 1.0;
}

// Turn off motor sound struct
void stop_motor_sound(int is_motor1) {
    motor_sound* mot;
    if (is_motor1) {
        mot = &MotorSound1;
    } else {
        mot = &MotorSound2;
    }

    mot->enabled = 0;
    mot->motor_state = MOTOR_IGNITION;
    mot->playback_index_ignition = 0;
    mot->frequency_prev = 1.0;
    mot->frequency_next = 1.0;
}

static void mix_into_buffer(short* buffer, short* source, int buffer_length) {
    for (int i = 0; i < buffer_length; i++) {
        buffer[i] += source[i];
    }
}

// Mix in the sound of the engine
static void mix_motor_sounds(int is_motor1, short* buffer, int buffer_length) {
    motor_sound* mot;
    if (is_motor1) {
        mot = &MotorSound1;
    } else {
        mot = &MotorSound2;
    }

    if (!mot->enabled) {
        return;
    }

    int copied_counter = 0;
    int source_length = -1;
    int source_index_end = -1;
    int fade_counter = -1;
    int i = -1;
    while (1) {
        switch (mot->motor_state) {
        case MOTOR_IGNITION:
            // Bike turn on sound at start of level
            if (mot->playback_index_ignition + buffer_length > SoundMotorIgnition->size) {
                // We're finished with the ignition sound. Transition to idle sound
                int source_length2 = SoundMotorIgnition->size - mot->playback_index_ignition;
                mix_into_buffer(&buffer[copied_counter],
                                &SoundMotorIgnition->tomb[mot->playback_index_ignition],
                                source_length2);
                copied_counter += source_length2;
                mot->motor_state = MOTOR_IDLE;
                mot->playback_index_idle = WAV_FADE_LENGTH;
            } else {
                // We're not finished with the ignition sound, but the buffer is full
                int source_length3 = buffer_length - copied_counter;
                mix_into_buffer(&buffer[copied_counter],
                                &SoundMotorIgnition->tomb[mot->playback_index_ignition],
                                source_length3);
                mot->playback_index_ignition += source_length3;
                return;
            }
            break;
        case MOTOR_IDLE:
            // Idle bike sound
            if (mot->gas) {
                // If we are pressing gas, immediately go to transition sound
                mot->motor_state = MOTOR_IDLE_TO_GAS_TRANSITION;
                mot->playback_index_gas_start = 0;
            } else {
                // Otherwise, infinitely loop the idle sound
                int source_length4 = buffer_length - copied_counter;
                if (source_length4 > SoundMotorIdle->size - mot->playback_index_idle) {
                    // Copy until the end of the sound effect, then loop
                    source_length4 = SoundMotorIdle->size - mot->playback_index_idle;
                    mix_into_buffer(&buffer[copied_counter],
                                    &SoundMotorIdle->tomb[mot->playback_index_idle],
                                    source_length4);
                    copied_counter += source_length4;
                    mot->playback_index_idle = 0;
                } else {
                    // Buffer is full
                    mix_into_buffer(&buffer[copied_counter],
                                    &SoundMotorIdle->tomb[mot->playback_index_idle],
                                    source_length4);
                    mot->playback_index_idle += source_length4;
                    return;
                }
            }
            break;
        case MOTOR_IDLE_TO_GAS_TRANSITION:
            // Fade from idle sound effect to start-of-gas sound effect
            source_length = buffer_length - copied_counter;
            source_index_end = mot->playback_index_gas_start + source_length;
            if (source_index_end > WAV_FADE_LENGTH) {
                // When we are done fading, we will move on to the start-of-gas sound effect
                source_index_end = WAV_FADE_LENGTH;
                mot->motor_state = MOTOR_GAS_START;
            }
            // Fade the first WAV_FADE_LENGTH samples
            fade_counter = 0;
            for (i = mot->playback_index_gas_start; i < source_index_end; i++) {
                if (mot->playback_index_idle >= SoundMotorIdle->size) {
                    mot->playback_index_idle = 0;
                }
                double fade_percentage = i * WAV_FADE_LENGTH_RECIPROCAL;
                buffer[copied_counter + fade_counter] +=
                    fade_percentage * SoundMotorGasStart->tomb[i] +
                    (1 - fade_percentage) * SoundMotorIdle->tomb[mot->playback_index_idle];
                mot->playback_index_idle++;
                fade_counter++;
            }
            copied_counter += fade_counter;
            mot->playback_index_gas_start += fade_counter;
            if (copied_counter + fade_counter == buffer_length) {
                return;
            }
            break;
        case MOTOR_GAS_START:
            // start-of-gas sound effect (sound frequency does not yet depend on speed)
            source_length = buffer_length - copied_counter;
            if (source_length > SoundMotorGasStart->size - mot->playback_index_gas_start) {
                // We're finished with the start-of-gas sound. Transition to full gas sound
                source_length = SoundMotorGasStart->size - mot->playback_index_gas_start;
                mix_into_buffer(&buffer[copied_counter],
                                &SoundMotorGasStart->tomb[mot->playback_index_gas_start],
                                source_length);
                copied_counter += source_length;
                mot->motor_state = MOTOR_GASSING;
                if (is_motor1) {
                    SoundMotorGas1->reset(WAV_FADE_LENGTH);
                } else {
                    SoundMotorGas2->reset(WAV_FADE_LENGTH);
                }
                mot->frequency_prev = 1.0;
            } else {
                // We're not finished with the start-of-gas sound, but the buffer is full
                mix_into_buffer(&buffer[copied_counter],
                                &SoundMotorGasStart->tomb[mot->playback_index_gas_start],
                                source_length);
                mot->playback_index_gas_start += source_length;
                return;
            }
            break;
        case MOTOR_GASSING:
            // Gassing sound effect - variable playback speed based on wheel spin speed
            // We interpolate the playback speed over the buffer length,
            // so sound playback isn't deterministic across platforms
            int source_length5 = buffer_length - copied_counter;
            if (!mot->gas && source_length5 > WAV_FADE_LENGTH) {
                // Copy until the end of the sound effect, then loop
                mot->motor_state = MOTOR_IDLE;
                mot->playback_index_idle = WAV_FADE_LENGTH;
                long previous_dt = 65536.0 * mot->frequency_prev;
                for (int i = 0; i < WAV_FADE_LENGTH; i++) {
                    short gas_sample;
                    if (is_motor1) {
                        gas_sample = SoundMotorGas1->getnextsample(previous_dt);
                    } else {
                        gas_sample = SoundMotorGas2->getnextsample(previous_dt);
                    }
                    short idle_sample = SoundMotorIdle->tomb[i];

                    double fade_percentage = (double)i / WAV_FADE_LENGTH;
                    double gas_fade_percentage = 1.0 - fade_percentage;

                    buffer[copied_counter + i] +=
                        fade_percentage * idle_sample + gas_fade_percentage * gas_sample;
                }
                copied_counter += WAV_FADE_LENGTH;
                break;
            } else {
                // Buffer is full
                long previous_dt = 65536.0 * mot->frequency_prev;
                long current_dt = 65536.0 * mot->frequency_next;
                long ddt = 0;
                if (source_length5 > 30) {
                    ddt = (current_dt - previous_dt) / ((double)source_length5);
                }
                for (int i = 0; i < source_length5; i++) {
                    if (is_motor1) {
                        buffer[copied_counter + i] += SoundMotorGas1->getnextsample(previous_dt);
                    } else {
                        buffer[copied_counter + i] += SoundMotorGas2->getnextsample(previous_dt);
                    }
                    previous_dt += ddt;
                }
                mot->frequency_prev = ((double)previous_dt) / 65536.0;
                return;
            }
            break;
        }
    }
}

static int SoundFrictionIndex = 0;

// Bike squeaking sound
static void mix_friction(short* buffer, int buffer_length) {
    if (FrictionVolumeNext < 0.1 && FrictionVolumePrev < 0.1) {
        FrictionVolumePrev = 0.0;
        return;
    }

    // We interpolate the volume over the buffer length,
    // so sound playback isn't deterministic across platforms
    long volume_prev = 65536.0 * FrictionVolumePrev;
    long volume_next = 65536.0 * FrictionVolumeNext;
    long delta_volume = (volume_next - volume_prev) / ((double)buffer_length);
    int sample_length = SoundFriction->size;
    for (int i = 0; i < buffer_length; i++) {
        long sample = SoundFriction->tomb[SoundFrictionIndex];
        SoundFrictionIndex++;
        if (SoundFrictionIndex >= sample_length) {
            SoundFrictionIndex = 0;
        }

        sample *= volume_prev;
        buffer[i] += (short)(sample >> 16);

        volume_prev += delta_volume;
    }
    FrictionVolumePrev = ((double)volume_prev) / 65536.0;
}

void sound_mixer(short* buffer, int buffer_length) {
    if (!SoundInitialized) {
        internal_error("sound_mixer !SoundInitialized!");
    }

    memset(buffer, 0, buffer_length * 2);
    if (Mute || !State->sound_on) {
        if (ActiveWavEvents > 0) {
            ActiveWavEvents = 0;
            for (int i = 0; i < MAX_WAV_EVENTS; i++) {
                WavEventActive[i] = 0;
            }
        }
        return;
    }

    if (Atmenet >= buffer_length) {
        Atmenet = buffer_length - 1;
    }
    if (Atmenet < 20) {
        internal_error("sound_mixer-ban buffer_length < 20!");
    }

    // Do both bike motor sounds, and then bike squeak sound
    mix_motor_sounds(1, buffer, buffer_length);
    mix_motor_sounds(0, buffer, buffer_length);
    mix_friction(buffer, buffer_length);

    // Mix in Wavbank sound effects
    for (int i = 0; i < MAX_WAV_EVENTS; i++) {
        if (WavEventActive[i]) {
            int length = buffer_length;
            if (length > WavEventSound[i]->size - WavEventPlaybackIndex[i]) {
                length = WavEventSound[i]->size - WavEventPlaybackIndex[i];
                WavEventActive[i] = 0;
                ActiveWavEvents--;
                if (ActiveWavEvents < 0) {
                    internal_error("ActiveWavEvents < 0 !");
                }
            }
            mix_into_buffer(buffer, &WavEventSound[i]->tomb[WavEventPlaybackIndex[i]], length,
                            WavEventVolume[i]);
            WavEventPlaybackIndex[i] += length;
        }
    }
}

void delay(int milliseconds) {
    double current_time = stopwatch();
    while (stopwatch() / 182.0 < current_time / 182.0 + milliseconds / 1000.0) {
        handle_events();
    }
}
