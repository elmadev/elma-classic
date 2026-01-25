#include "sound_engine.h"
#include "main.h"
#include "platform_impl.h"
#include "state.h"
#include "wav.h"
#include <cstring>

bool Mute = true;

constexpr int WAV_BANK_LENGTH = 20;
static wav* WavBank[WAV_BANK_LENGTH];

static wav* SoundMotorIgnition = nullptr;
static wav* SoundMotorIdle = nullptr;
static wav* SoundMotorGasStart = nullptr;
static wav* SoundFriction = nullptr;
static wav* SoundMotorGas = nullptr;
static wav2* SoundMotorGas1 = nullptr;
static wav2* SoundMotorGas2 = nullptr;

constexpr int WAV_FADE_LENGTH = 100;

static bool SoundEngineInitialized = false;

static int ActiveWavEvents = 0;
constexpr int MAX_WAV_EVENTS = 5;
static int WavEventActive[MAX_WAV_EVENTS];
static int WavEventPlaybackIndex[MAX_WAV_EVENTS];
static double WavEventVolume[MAX_WAV_EVENTS];
static wav* WavEventSound[MAX_WAV_EVENTS];

// Load all sounds into memory
void sound_engine_init() {
    if (SoundEngineInitialized) {
        internal_error("sound_engine_init already initialized!");
    }

    // Load Wavbank
    for (int i = 0; i < WAV_BANK_LENGTH; i++) {
        WavBank[i] = nullptr;
    }
    WavBank[(int)WavEvent::Bump] = new wav("utodes.wav", 0.25);     // collision
    WavBank[(int)WavEvent::Dead] = new wav("torik.wav", 0.34);      // shatter
    WavBank[(int)WavEvent::Win] = new wav("siker.wav", 0.8);        // success
    WavBank[(int)WavEvent::Food] = new wav("eves.wav", 0.5);        // eating
    WavBank[(int)WavEvent::Turn] = new wav("fordul.wav", 0.3);      // turn
    WavBank[(int)WavEvent::RightVolt] = new wav("ugras.wav", 0.34); // jump
    WavBank[(int)WavEvent::LeftVolt] = WavBank[(int)WavEvent::RightVolt];

    // Bike squeak sound
    SoundFriction = new wav("dorzsol.wav", 0.44); // friction
    SoundFriction->loop(WAV_FADE_LENGTH);

    // Bike engine sound
    double volume = 0.26;

    constexpr int HARL_MAX_INDEX_1 = 7526;
    constexpr int HARL_MAX_INDEX_2 = 34648;
    constexpr int HARL_MAX_INDEX_3 = 38766;
    constexpr int HARL2_MIN_INDEX = 14490;
    constexpr int HARL2_MAX_INDEX = 18906;
    // Filename possibly a reference to Harley-Davidson motorcycles
    SoundMotorIgnition = new wav("harl.wav", volume, 0, HARL_MAX_INDEX_1 + WAV_FADE_LENGTH);
    SoundMotorIdle = new wav("harl.wav", volume, HARL_MAX_INDEX_1, HARL_MAX_INDEX_2);
    SoundMotorGasStart =
        new wav("harl.wav", volume, HARL_MAX_INDEX_2 - WAV_FADE_LENGTH, HARL_MAX_INDEX_3);
    SoundMotorGas = new wav("harl2.wav", volume, HARL2_MIN_INDEX, HARL2_MAX_INDEX);

    // Post-process bike sounds
    SoundMotorIdle->loop(WAV_FADE_LENGTH);
    SoundMotorGas->loop(WAV_FADE_LENGTH);
    SoundMotorGasStart->fade(SoundMotorGas, WAV_FADE_LENGTH);

    // Make gas sound able to be played back at different speeds
    SoundMotorGas1 = new wav2(SoundMotorGas);
    SoundMotorGas1->reset();
    SoundMotorGas2 = new wav2(SoundMotorGas);
    SoundMotorGas2->reset();

    // Initialize wavbank playing sounds buffer
    for (int i = 0; i < MAX_WAV_EVENTS; i++) {
        WavEventActive[i] = 0;
        WavEventPlaybackIndex[i] = 0;
        WavEventVolume[i] = 0.0;
        WavEventSound[i] = nullptr;
    }

    // Change from across: lower the volume of idle motor
    // We could have directly done this above as the file was loaded, but we didn't
    SoundMotorIdle->volume(0.4);

    SoundEngineInitialized = true;
}

// Which sound the motor is currently generating
enum class MotorState {
    Ignition,
    Idle,
    IdleToGasTransition,
    GasStart,
    Gassing,
};

// Sound state of one bike
struct motor_sound {
    int enabled;
    double frequency_prev;
    double frequency_next;
    int gas;
    MotorState motor_state;
    int playback_index_ignition;
    int playback_index_idle;
    int playback_index_gas_start;
};

static motor_sound MotorSound1;
static motor_sound MotorSound2;

// Set the playback speed of the bike gassing sound effect (capped between 1.0 and 2.0)
void set_motor_frequency(bool is_motor1, double frequency, int gas) {
    if (!SoundEngineInitialized) {
        return;
    }
    motor_sound* mot = is_motor1 ? &MotorSound1 : &MotorSound2;
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
void start_wav(WavEvent event, double volume) {
    if (!SoundEngineInitialized || Mute) {
        return;
    }

    if (volume <= 0.0 || volume >= 1.0) {
        internal_error("start_wav volume <= 0.0 || volume >= 1.0!");
    }

    wav* sound = WavBank[(int)event];

    if (ActiveWavEvents >= MAX_WAV_EVENTS) {
        return;
    }

    for (int i = 0; i < MAX_WAV_EVENTS; i++) {
        if (!WavEventActive[i]) {
            ActiveWavEvents++;
            WavEventActive[i] = 1;
            WavEventPlaybackIndex[i] = 0;
            WavEventSound[i] = sound;
            WavEventVolume[i] = volume;
            return;
        }
    }
    internal_error("start_wav Unable to find free wav slot!");
}

// Initialize motor sound struct
void start_motor_sound(bool is_motor1) {
    motor_sound* mot = is_motor1 ? &MotorSound1 : &MotorSound2;
    mot->enabled = 1;
    mot->motor_state = MotorState::Ignition;
    mot->playback_index_ignition = 0;
    mot->frequency_prev = 1.0;
    mot->frequency_next = 1.0;
}

// Turn off motor sound struct
void stop_motor_sound(bool is_motor1) {
    motor_sound* mot = is_motor1 ? &MotorSound1 : &MotorSound2;
    mot->enabled = 0;
    mot->motor_state = MotorState::Ignition;
    mot->playback_index_ignition = 0;
    mot->frequency_prev = 1.0;
    mot->frequency_next = 1.0;
}

static void mix_into_buffer(short* buffer, short* source, int buffer_length) {
    for (int i = 0; i < buffer_length; i++) {
        buffer[i] += source[i];
    }
}

static void mix_into_buffer(short* buffer, short* source, int buffer_length, double volume) {
    for (int i = 0; i < buffer_length; i++) {
        buffer[i] += short(source[i] * volume);
    }
}

// Mix in the sound of the engine
static void mix_motor_sounds(bool is_motor1, short* buffer, int buffer_length) {
    motor_sound* mot = is_motor1 ? &MotorSound1 : &MotorSound2;
    if (!mot->enabled) {
        return;
    }

    int copied_counter = 0;
    int source_index_end = -1;
    int fade_counter = -1;
    int source_length = -1;
    while (true) {
        switch (mot->motor_state) {
        case MotorState::Ignition:
            // Bike turn on sound at start of level
            if (mot->playback_index_ignition + buffer_length > SoundMotorIgnition->size) {
                // We're finished with the ignition sound. Transition to idle sound
                source_length = SoundMotorIgnition->size - mot->playback_index_ignition;
                mix_into_buffer(&buffer[copied_counter],
                                &SoundMotorIgnition->samples[mot->playback_index_ignition],
                                source_length);
                copied_counter += source_length;
                mot->motor_state = MotorState::Idle;
                mot->playback_index_idle = WAV_FADE_LENGTH;
            } else {
                // We're not finished with the ignition sound, but the buffer is full
                source_length = buffer_length - copied_counter;
                mix_into_buffer(&buffer[copied_counter],
                                &SoundMotorIgnition->samples[mot->playback_index_ignition],
                                source_length);
                mot->playback_index_ignition += source_length;
                return;
            }
            break;
        case MotorState::Idle:
            // Idle bike sound
            if (mot->gas) {
                // If we are pressing gas, immediately go to transition sound
                mot->motor_state = MotorState::IdleToGasTransition;
                mot->playback_index_gas_start = 0;
            } else {
                // Otherwise, infinitely loop the idle sound
                source_length = buffer_length - copied_counter;
                if (source_length > SoundMotorIdle->size - mot->playback_index_idle) {
                    // Copy until the end of the sound effect, then loop
                    source_length = SoundMotorIdle->size - mot->playback_index_idle;
                    mix_into_buffer(&buffer[copied_counter],
                                    &SoundMotorIdle->samples[mot->playback_index_idle],
                                    source_length);
                    copied_counter += source_length;
                    mot->playback_index_idle = 0;
                } else {
                    // Buffer is full
                    mix_into_buffer(&buffer[copied_counter],
                                    &SoundMotorIdle->samples[mot->playback_index_idle],
                                    source_length);
                    mot->playback_index_idle += source_length;
                    return;
                }
            }
            break;
        case MotorState::IdleToGasTransition:
            // Fade from idle sound effect to start-of-gas sound effect
            source_length = buffer_length - copied_counter;
            source_index_end = mot->playback_index_gas_start + source_length;
            if (source_index_end > WAV_FADE_LENGTH) {
                // When we are done fading, we will move on to the start-of-gas sound effect
                source_index_end = WAV_FADE_LENGTH;
                mot->motor_state = MotorState::GasStart;
            }
            // Fade the first WAV_FADE_LENGTH samples
            fade_counter = 0;
            for (int i = mot->playback_index_gas_start; i < source_index_end; i++) {
                if (mot->playback_index_idle >= SoundMotorIdle->size) {
                    mot->playback_index_idle = 0;
                }
                double fade_percentage = i / (double)(WAV_FADE_LENGTH);
                buffer[copied_counter + fade_counter] +=
                    (short)(fade_percentage * SoundMotorGasStart->samples[i] +
                            (1 - fade_percentage) *
                                SoundMotorIdle->samples[mot->playback_index_idle]);
                mot->playback_index_idle++;
                fade_counter++;
            }
            copied_counter += fade_counter;
            mot->playback_index_gas_start += fade_counter;
            if (copied_counter + fade_counter == buffer_length) {
                return;
            }
            break;
        case MotorState::GasStart:
            // start-of-gas sound effect (sound frequency does not yet depend on speed)
            source_length = buffer_length - copied_counter;
            if (source_length > SoundMotorGasStart->size - mot->playback_index_gas_start) {
                // We're finished with the start-of-gas sound. Transition to full gas sound
                source_length = SoundMotorGasStart->size - mot->playback_index_gas_start;
                mix_into_buffer(&buffer[copied_counter],
                                &SoundMotorGasStart->samples[mot->playback_index_gas_start],
                                source_length);
                copied_counter += source_length;
                mot->motor_state = MotorState::Gassing;
                if (is_motor1) {
                    SoundMotorGas1->reset(WAV_FADE_LENGTH);
                } else {
                    SoundMotorGas2->reset(WAV_FADE_LENGTH);
                }
                mot->frequency_prev = 1.0;
            } else {
                // We're not finished with the start-of-gas sound, but the buffer is full
                mix_into_buffer(&buffer[copied_counter],
                                &SoundMotorGasStart->samples[mot->playback_index_gas_start],
                                source_length);
                mot->playback_index_gas_start += source_length;
                return;
            }
            break;
        case MotorState::Gassing:
            // Gassing sound effect - variable playback speed based on wheel spin speed
            // We interpolate the playback speed over the buffer length,
            // so sound playback isn't deterministic across platforms
            source_length = buffer_length - copied_counter;
            if (!mot->gas && source_length > WAV_FADE_LENGTH) {
                // Copy until the end of the sound effect, then loop
                mot->motor_state = MotorState::Idle;
                mot->playback_index_idle = WAV_FADE_LENGTH;
                double dt = mot->frequency_prev;
                for (int i = 0; i < WAV_FADE_LENGTH; i++) {
                    short gas_sample;
                    if (is_motor1) {
                        gas_sample = SoundMotorGas1->get_next_sample(dt);
                    } else {
                        gas_sample = SoundMotorGas2->get_next_sample(dt);
                    }
                    short idle_sample = SoundMotorIdle->samples[i];

                    double fade_percentage = (double)i / WAV_FADE_LENGTH;
                    double gas_fade_percentage = 1.0 - fade_percentage;

                    buffer[copied_counter + i] +=
                        (short)(fade_percentage * idle_sample + gas_fade_percentage * gas_sample);
                }
                copied_counter += WAV_FADE_LENGTH;
                break;
            } else {
                // Buffer is full
                double dt = mot->frequency_prev;
                double next_dt = mot->frequency_next;
                double ddt = 0;
                if (source_length > 30) {
                    ddt = (next_dt - dt) / source_length;
                }
                for (int i = 0; i < source_length; i++) {
                    if (is_motor1) {
                        buffer[copied_counter + i] += SoundMotorGas1->get_next_sample(dt);
                    } else {
                        buffer[copied_counter + i] += SoundMotorGas2->get_next_sample(dt);
                    }
                    dt += ddt;
                }
                mot->frequency_prev = dt;
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
    double volume = FrictionVolumePrev;
    double volume_next = FrictionVolumeNext;
    double delta_volume = (volume_next - volume) / buffer_length;
    int sample_length = SoundFriction->size;
    for (int i = 0; i < buffer_length; i++) {
        short sample = SoundFriction->samples[SoundFrictionIndex];
        SoundFrictionIndex++;
        if (SoundFrictionIndex >= sample_length) {
            SoundFrictionIndex = 0;
        }
        buffer[i] += (short)(sample * volume);
        volume += delta_volume;
    }
    FrictionVolumePrev = volume;
}

void sound_mixer(short* buffer, int buffer_length) {
    memset(buffer, 0, buffer_length * 2);
    if (!SoundEngineInitialized) {
        return;
    }
    if (Mute || !State->sound_on) {
        if (ActiveWavEvents > 0) {
            ActiveWavEvents = 0;
            for (int i = 0; i < MAX_WAV_EVENTS; i++) {
                WavEventActive[i] = 0;
            }
        }
        return;
    }

    // Do both bike motor sounds, and then bike squeak sound
    mix_motor_sounds(true, buffer, buffer_length);
    mix_motor_sounds(false, buffer, buffer_length);
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
            mix_into_buffer(buffer, &WavEventSound[i]->samples[WavEventPlaybackIndex[i]], length,
                            WavEventVolume[i]);
            WavEventPlaybackIndex[i] += length;
        }
    }
}
