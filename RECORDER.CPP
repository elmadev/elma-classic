#include "RECORDER.H"
#include "flagtag.h"
#include "main.h"
#include "physics_init.h"
#include "platform_utils.h"
#include "qopen.h"
#include <cmath>
#include <cstring>
#include "fs_utils.h"

recorder* Rec1 = nullptr;
recorder* Rec2 = nullptr;
int MultiplayerRec = 0;

constexpr int MAGIC_NUMBER = 4796277;

constexpr int FRAME_RATE = 30;
constexpr double TIME_TO_FRAME_INDEX = FRAME_RATE / (STOPWATCH_MULTIPLIER * 1000.0 * 0.0024);
constexpr double FRAME_INDEX_TO_TIME = 1.0 / TIME_TO_FRAME_INDEX;

constexpr int MAX_FRAMES = FRAME_RATE * 60 * 5 - 19;
constexpr int MAX_EVENTS = 3900;

constexpr int FLAG_GAS = 0;
constexpr int FLAG_FLIPPED = 1;
constexpr int FLAG_FLAGTAG_A = 2;
constexpr int FLAG_FLAGTAG_IMMUNITY = 3;

recorder::recorder() {
    frame_count = 0;

    bike_x = nullptr;
    bike_y = nullptr;
    left_wheel_x = nullptr;
    left_wheel_y = nullptr;
    right_wheel_x = nullptr;
    right_wheel_y = nullptr;
    body_x = nullptr;
    body_y = nullptr;
    bike_rotation = nullptr;
    left_wheel_rotation = nullptr;
    right_wheel_rotation = nullptr;
    flags = nullptr;
    motor_frequency = nullptr;
    friction_volume = nullptr;

    event_count = 0;
    events = nullptr;

    flagtag_ = 0;

    level_filename[0] = 0;

    int hossz = sizeof(float);
    if (hossz != 4) {
        internal_error("float hossz != 4!");
    }
    hossz = MAX_FRAMES * sizeof(float);
    if (hossz > 64000l) {
        internal_error("MAX_FRAMES*sizeof( float )!");
    }

    hossz = MAX_EVENTS * sizeof(event);
    if (hossz > 64000l) {
        internal_error("MAX_EVENTS*sizeof( event )!");
    }

    bike_x = new float[MAX_FRAMES];
    bike_y = new float[MAX_FRAMES];
    left_wheel_x = new short[MAX_FRAMES];
    left_wheel_y = new short[MAX_FRAMES];
    right_wheel_x = new short[MAX_FRAMES];
    right_wheel_y = new short[MAX_FRAMES];
    body_x = new short[MAX_FRAMES];
    body_y = new short[MAX_FRAMES];
    bike_rotation = new short[MAX_FRAMES];
    left_wheel_rotation = new unsigned char[MAX_FRAMES];
    right_wheel_rotation = new unsigned char[MAX_FRAMES];
    flags = new unsigned char[MAX_FRAMES];
    motor_frequency = new unsigned char[MAX_FRAMES];
    friction_volume = new unsigned char[MAX_FRAMES];
    events = new event[MAX_EVENTS];

    if (!bike_x || !bike_y || !left_wheel_x || !left_wheel_y || !right_wheel_x || !right_wheel_y ||
        !body_x || !body_y || !left_wheel_rotation || !bike_rotation || !right_wheel_rotation ||
        !motor_frequency || !flags || !events || !friction_volume) {
        external_error("recorder::recorder out of memory!");
    }
}

recorder::~recorder() { internal_error("recorder::~recorder not implemented!"); }

void recorder::erase(char* lev_filename) {
    if (strlen(lev_filename) > MAX_FILENAME_LEN + 4) {
        internal_error("recorder::erase strlen");
    }
    strcpy(level_filename, lev_filename);
    frame_count = 0;
    event_count = 0;
    finished = false;
    current_event_index = 0;
    next_frame_index = 0;
}

void recorder::rewind() {
    finished = false;
    current_event_index = 0;
    next_frame_index = 0;
}

bool recorder::flagtag() { return (bool)(flagtag_); }

void recorder::set_flagtag(bool flagtag) { flagtag_ = (int)(flagtag); }

void recorder::encode_frame_count() {
    if (frame_count < 80) {
        return;
    }
    unsigned int encoded_value = frame_count;
    for (int i = 0; i < 32; i++) {
        flags[40 + i] = flags[40 + i] & 0x7F;
        if (encoded_value & 1) {
            flags[40 + i] += 0x80;
        }
        encoded_value = encoded_value >> 1;
    }
}

bool recorder::frame_count_integrity() {
    if (frame_count < 80) {
        return true;
    }
    unsigned int encoded_value = 0;
    for (int i = 0; i < 32; i++) {
        encoded_value <<= 1;
        if (flags[40 + 31 - i] & 0x80) {
            encoded_value += 1;
        }
    }
    return encoded_value == frame_count;
}

constexpr double POSITION_RATIO = 1000.0, POSITION_RATIO_INV = 1.0 / POSITION_RATIO;
constexpr double WHEEL_ROTATION_RATIO = 250 / (2.0 * PI),
                 WHEEL_ROTATION_INV_RATIO = 1.0 / WHEEL_ROTATION_RATIO;
constexpr double BIKE_ROTATION_RATIO = 10000 / (2.0 * PI),
                 BIKE_ROTATION_INV_RATIO = 1.0 / BIKE_ROTATION_RATIO;
constexpr double MOTOR_FREQUENCY_RATIO = 250.0,
                 MOTOR_FREQUENCY_INV_RATIO = 1.0 / MOTOR_FREQUENCY_RATIO;
constexpr double FRICTION_VOLUME_RATIO = 250 / 2.0,
                 FRICTION_VOLUME_INV_RATIO = 1.0 / FRICTION_VOLUME_RATIO;

bool recorder::recall_frame(motorst* mot, double time, bike_sound* sound) {
    if (frame_count <= 0) {
        internal_error("recall_frame frame_count <= 0!");
    }

    int index1 = (int)(TIME_TO_FRAME_INDEX * time);
    double index2_weight = TIME_TO_FRAME_INDEX * time - index1;
    if (index2_weight < 0.0) {
        index2_weight = 0.0;
    }
    if (index2_weight > 1.0) {
        index2_weight = 1.0;
    }
    double index1_weight = 1.0 - index2_weight;
    int index2 = index1 + 1;

    if (index1 < 0) {
        index1 = 0;
    }
    if (index2 < 0) {
        index2 = 0;
    }

    if (finished) {
        sound->motor_frequency = 1.0;
        sound->gas = 0;
        sound->friction_volume = 0;
        return false;
    }

    if (index1 >= frame_count - 1) {
        index1 = frame_count - 1;
        finished = true;
    }
    if (index2 >= frame_count - 1) {
        index2 = frame_count - 1;
    }

    mot->bike.r.x = bike_x[index1] * index1_weight + bike_x[index2] * index2_weight;
    mot->bike.r.y = bike_y[index1] * index1_weight + bike_y[index2] * index2_weight;
    mot->left_wheel.r.x =
        mot->bike.r.x + POSITION_RATIO_INV * (left_wheel_x[index1] * index1_weight +
                                              left_wheel_x[index2] * index2_weight);
    mot->left_wheel.r.y =
        mot->bike.r.y + POSITION_RATIO_INV * (left_wheel_y[index1] * index1_weight +
                                              left_wheel_y[index2] * index2_weight);
    mot->right_wheel.r.x =
        mot->bike.r.x + POSITION_RATIO_INV * (right_wheel_x[index1] * index1_weight +
                                              right_wheel_x[index2] * index2_weight);
    mot->right_wheel.r.y =
        mot->bike.r.y + POSITION_RATIO_INV * (right_wheel_y[index1] * index1_weight +
                                              right_wheel_y[index2] * index2_weight);
    mot->body_r.x = mot->bike.r.x + POSITION_RATIO_INV * (body_x[index1] * index1_weight +
                                                          body_x[index2] * index2_weight);
    mot->body_r.y = mot->bike.r.y + POSITION_RATIO_INV * (body_y[index1] * index1_weight +
                                                          body_y[index2] * index2_weight);

    if (index1 >= 2) {
        if (abs(bike_rotation[index1] - bike_rotation[index2]) > 9000) {
            if (bike_rotation[index1] > bike_rotation[index2]) {
                int fixed_rotation = bike_rotation[index1] - 10000;
                mot->bike.rotation =
                    BIKE_ROTATION_INV_RATIO *
                    (fixed_rotation * index1_weight + bike_rotation[index2] * index2_weight);
            } else {
                int fixed_rotation = bike_rotation[index2] - 10000;
                mot->bike.rotation = BIKE_ROTATION_INV_RATIO *
                                     (bike_rotation[index1] * index1_weight * index1_weight +
                                      fixed_rotation * index2_weight);
            }
        } else {
            mot->bike.rotation = BIKE_ROTATION_INV_RATIO * (bike_rotation[index1] * index1_weight +
                                                            bike_rotation[index2] * index2_weight);
        }

        if (abs(left_wheel_rotation[index1] - left_wheel_rotation[index2]) > 220) {
            if (left_wheel_rotation[index1] > left_wheel_rotation[index2]) {
                int fixed_rotation = left_wheel_rotation[index1] - 250;
                mot->left_wheel.rotation =
                    WHEEL_ROTATION_INV_RATIO *
                    (fixed_rotation * index1_weight + left_wheel_rotation[index2] * index2_weight);
            } else {
                int fixed_rotation = left_wheel_rotation[index2] - 250;
                mot->left_wheel.rotation =
                    WHEEL_ROTATION_INV_RATIO *
                    (left_wheel_rotation[index1] * index1_weight * index1_weight +
                     fixed_rotation * index2_weight);
            }
        } else {
            mot->left_wheel.rotation =
                WHEEL_ROTATION_INV_RATIO * (left_wheel_rotation[index1] * index1_weight +
                                            left_wheel_rotation[index2] * index2_weight);
        }

        if (abs(right_wheel_rotation[index1] - right_wheel_rotation[index2]) > 220) {
            if (right_wheel_rotation[index1] > right_wheel_rotation[index2]) {
                int fixed_rotation = right_wheel_rotation[index1] - 250;
                mot->right_wheel.rotation =
                    WHEEL_ROTATION_INV_RATIO *
                    (fixed_rotation * index1_weight + right_wheel_rotation[index2] * index2_weight);
            } else {
                int fixed_rotation = right_wheel_rotation[index2] - 250;
                mot->right_wheel.rotation =
                    WHEEL_ROTATION_INV_RATIO *
                    (right_wheel_rotation[index1] * index1_weight * index1_weight +
                     fixed_rotation * index2_weight);
            }
        } else {
            mot->right_wheel.rotation =
                WHEEL_ROTATION_INV_RATIO * (right_wheel_rotation[index1] * index1_weight +
                                            right_wheel_rotation[index2] * index2_weight);
        }
    } else {
        mot->bike.rotation = BIKE_ROTATION_INV_RATIO * bike_rotation[index2];
        mot->left_wheel.rotation = WHEEL_ROTATION_INV_RATIO * left_wheel_rotation[index2];
        mot->right_wheel.rotation = WHEEL_ROTATION_INV_RATIO * right_wheel_rotation[index2];
    }

    sound->gas = char((flags[index1] >> FLAG_GAS) & 1);
    mot->flipped_bike = (flags[index1] >> FLAG_FLIPPED) & 1;
    FlagTagAHasFlag = (flags[index1] >> FLAG_FLAGTAG_A) & 1;
    FlagTagImmunity = (flags[index1] >> FLAG_FLAGTAG_IMMUNITY) & 1;

    sound->motor_frequency = 1.0 + MOTOR_FREQUENCY_INV_RATIO * motor_frequency[index1];
    sound->friction_volume = FRICTION_VOLUME_INV_RATIO * friction_volume[index1];

    return true;
}

void recorder::store_frames(motorst* mot, double time, bike_sound* sound) {
    if (!next_frame_index) {
        previous_bike_r = mot->bike.r;
        previous_frame_time = -1E-11;
        next_frame_time = 0.0;
    }
    if (time < next_frame_time) {
        previous_bike_r = mot->bike.r;
        previous_frame_time = time;
        return;
    }

    while (true) {
#ifdef DEBUG
        if (time - previous_frame_time == 0.0) {
            internal_error("time-previous_frame_time == 0.0!");
        }
#endif
        // Interpolate bike position but not wheel/head positions, leading to jitters at low fps
        vect2 interpolated_bike_r =
            (mot->bike.r - previous_bike_r) *
                ((next_frame_time - previous_frame_time) / (time - previous_frame_time)) +
            previous_bike_r;

        if (next_frame_index >= MAX_FRAMES) {
            return;
        }

        int i = next_frame_index;
        bike_x[i] = interpolated_bike_r.x;
        bike_y[i] = interpolated_bike_r.y;
        left_wheel_x[i] = (short)((mot->left_wheel.r.x - mot->bike.r.x) * POSITION_RATIO);
        left_wheel_y[i] = (short)((mot->left_wheel.r.y - mot->bike.r.y) * POSITION_RATIO);
        right_wheel_x[i] = (short)((mot->right_wheel.r.x - mot->bike.r.x) * POSITION_RATIO);
        right_wheel_y[i] = (short)((mot->right_wheel.r.y - mot->bike.r.y) * POSITION_RATIO);
        body_x[i] = (short)((mot->body_r.x - mot->bike.r.x) * POSITION_RATIO);
        body_y[i] = (short)((mot->body_r.y - mot->bike.r.y) * POSITION_RATIO);

        double bike_rot = mot->bike.rotation;
        while (bike_rot <= 0) {
            bike_rot += TWO_PI;
        }
        while (bike_rot > TWO_PI) {
            bike_rot -= TWO_PI;
        }
        bike_rotation[i] = (short)(bike_rot * BIKE_ROTATION_RATIO);

        // Rotation is constrained between +- Pi, except when doing a break-stretch
        // During the brake-stretch, the wheel position might become slightly desynced
        if (mot->left_wheel.rotation <= 0) {
            left_wheel_rotation[i] =
                (unsigned char)((mot->left_wheel.rotation + TWO_PI) * WHEEL_ROTATION_RATIO);
        } else {
            left_wheel_rotation[i] =
                (unsigned char)(mot->left_wheel.rotation * WHEEL_ROTATION_RATIO);
        }
        if (mot->right_wheel.rotation <= 0) {
            right_wheel_rotation[i] =
                (unsigned char)((mot->right_wheel.rotation + TWO_PI) * WHEEL_ROTATION_RATIO);
        } else {
            right_wheel_rotation[i] =
                (unsigned char)(mot->right_wheel.rotation * WHEEL_ROTATION_RATIO);
        }

        // Encode gibberish into the 4 MSB of the flags
        // Due to a bug, the gibberish is accidentally sourced from the y value of the bike
        memcpy(&flags[i], &interpolated_bike_r.y, 1);
        flags[i] = flags[i] & 0xf0;

        if (sound->gas) {
            flags[i] += 1 << FLAG_GAS;
        }
        if (mot->flipped_bike) {
            flags[i] += 1 << FLAG_FLIPPED;
        }
        if (FlagTagAHasFlag) {
            flags[i] += 1 << FLAG_FLAGTAG_A;
        }
        if (FlagTagImmunity) {
            flags[i] += 1 << FLAG_FLAGTAG_IMMUNITY;
        }

        if (sound->motor_frequency < 1.0) {
            sound->motor_frequency = 1.0;
        }
        motor_frequency[i] =
            (unsigned char)(MOTOR_FREQUENCY_RATIO * (sound->motor_frequency - 1.0));
        friction_volume[i] = (unsigned char)(FRICTION_VOLUME_RATIO * sound->friction_volume);

        next_frame_index++;
        next_frame_time += FRAME_INDEX_TO_TIME;

        if (time < next_frame_time) {
            previous_bike_r = mot->bike.r;
            previous_frame_time = time;
            frame_count = next_frame_index;
            return;
        }
    }
}

void recorder::store_event(double time, WavEvent event_id, double volume, int object_id) {
    if (event_count >= MAX_EVENTS) {
        return;
    }
    if (event_count > 0) {
        if (events[event_count - 1].time > time + 0.00001) {
            char tmp[50];
            double time2 = events[event_count - 1].time;
            sprintf(tmp, "time1: negative time: %f\n", float(time - time2));
            internal_error(tmp);
        }
    }
    events[event_count].time = time;
    events[event_count].event_id = event_id;
    events[event_count].volume = volume;
    events[event_count].object_id = (short)object_id;
    event_count++;
}

bool recorder::recall_event(double time, WavEvent* event_id, double* volume, int* object_id) {
    if (current_event_index < event_count) {
        if (events[current_event_index].time <= time) {
            *event_id = events[current_event_index].event_id;
            *volume = events[current_event_index].volume;
            *object_id = events[current_event_index].object_id;
            current_event_index++;
            return true;
        }
    }
    return false;
}

static void read_error(const char* filename) {
    internal_error("Failed to read rec file: ", filename);
}

int recorder::load(const char* filename, FILE* h, int demo) {
    bool keep_file = false;
    if (h) {
        keep_file = true;
    }

    if (!h) {
        if (demo) {
            h = qopen(filename, "rb");
            if (!h) {
                internal_error("Failed to open demo file: ", filename);
            }
        } else {
            char path[40];
            sprintf(path, "rec/%s", filename);
            h = fopen(path, "rb");
            if (!h) {
                external_error("Failed to open rec file: ", path);
            }
        }
    }

    frame_count = 0;
    if (fread(&frame_count, 1, sizeof(frame_count), h) != 4) {
        read_error(filename);
    }
    if (frame_count <= 0) {
        internal_error("recorder frame_count <= 0: ", filename);
    }
    if (frame_count > MAX_FRAMES) {
        internal_error("recorder frame_count > MAX_FRAMES!");
    }

    int version = 0;
    if (fread(&version, 1, sizeof(version), h) != 4) {
        read_error(filename);
    }
    if (version < 131) {
        external_error("Rec file version is too old!", filename);
    }
    if (version > 131) {
        external_error("Rec file version is too new!", filename);
    }

    int multiplayer_rec_unused = 0;
    if (fread(&multiplayer_rec_unused, 1, sizeof(multiplayer_rec_unused), h) != 4) {
        read_error(filename);
    }
    if (fread(&flagtag_, 1, sizeof(flagtag_), h) != 4) {
        read_error(filename);
    }

    int level_id = 0;
    if (fread(&level_id, 1, sizeof(level_id), h) != 4) {
        read_error(filename);
    }
    if (fread(level_filename, 1, sizeof(level_filename), h) != 16) {
        read_error(filename);
    }

    int float_length = frame_count * sizeof(float);
    int char_length = frame_count * sizeof(char);
    int short_length = frame_count * sizeof(short);
    if (fread(bike_x, 1, float_length, h) != float_length) {
        read_error(filename);
    }
    if (fread(bike_y, 1, float_length, h) != float_length) {
        read_error(filename);
    }
    if (fread(left_wheel_x, 1, short_length, h) != short_length) {
        read_error(filename);
    }
    if (fread(left_wheel_y, 1, short_length, h) != short_length) {
        read_error(filename);
    }
    if (fread(right_wheel_x, 1, short_length, h) != short_length) {
        read_error(filename);
    }
    if (fread(right_wheel_y, 1, short_length, h) != short_length) {
        read_error(filename);
    }
    if (fread(body_x, 1, short_length, h) != short_length) {
        read_error(filename);
    }
    if (fread(body_y, 1, short_length, h) != short_length) {
        read_error(filename);
    }
    if (fread(bike_rotation, 1, short_length, h) != short_length) {
        read_error(filename);
    }
    if (fread(left_wheel_rotation, 1, char_length, h) != char_length) {
        read_error(filename);
    }
    if (fread(right_wheel_rotation, 1, char_length, h) != char_length) {
        read_error(filename);
    }
    if (fread(flags, 1, char_length, h) != char_length) {
        read_error(filename);
    }
    if (fread(motor_frequency, 1, char_length, h) != char_length) {
        read_error(filename);
    }
    if (fread(friction_volume, 1, char_length, h) != char_length) {
        read_error(filename);
    }

    if (fread(&event_count, 1, 4, h) != 4) {
        read_error(filename);
    }
    if (event_count < 0) {
        internal_error("recorder event_count < 0!");
    }
    if (event_count > MAX_EVENTS) {
        internal_error("recorder event_count > MAX_EVENTS!");
    }

    int event_length = event_count * sizeof(event);
    if (fread(events, 1, event_length, h) != event_length) {
        read_error(filename);
    }

    int magic_number = 0;
    if (fread(&magic_number, 1, sizeof(magic_number), h) != 4) {
        read_error(filename);
    }
    if (magic_number != MAGIC_NUMBER) {
        internal_error("magic_number != MAGIC_NUMBER");
    }

    if (!keep_file) {
        if (demo) {
            qclose(h);
        } else {
            fclose(h);
        }
    }

    return level_id;
}

static void save_error(const char* filename) {
    internal_error("Failed to write rec file: ", filename);
}

void recorder::save(const char* filename, FILE* h, int level_id, int flagtag) {
    flagtag_ = flagtag;

    bool keep_file = false;
    if (h) {
        keep_file = true;
    }

    if (frame_count == 0) {
        internal_error("recorder save frame_count == 0!");
    }

    if (!h) {
        char path[40];
        sprintf(path, "rec/%s", filename);
        h = fopen(path, "wb");
        if (!h) {
            internal_error("Failed to open rec file for writing!: ", path);
        }
    }

    if (fwrite(&frame_count, 1, sizeof(frame_count), h) != 4) {
        save_error(filename);
    }
    int version = 131;
    if (fwrite(&version, 1, sizeof(version), h) != 4) {
        save_error(filename);
    }
    if (fwrite(&MultiplayerRec, 1, sizeof(MultiplayerRec), h) != 4) {
        save_error(filename);
    }
    if (fwrite(&flagtag_, 1, sizeof(flagtag_), h) != 4) {
        save_error(filename);
    }
    if (fwrite(&level_id, 1, sizeof(level_id), h) != 4) {
        save_error(filename);
    }
    if (fwrite(level_filename, 1, sizeof(level_filename), h) != 16) {
        save_error(filename);
    }

    int float_length = frame_count * sizeof(float);
    int char_length = frame_count * sizeof(char);
    int short_length = frame_count * sizeof(short);
    if (fwrite(bike_x, 1, float_length, h) != float_length) {
        save_error(filename);
    }
    if (fwrite(bike_y, 1, float_length, h) != float_length) {
        save_error(filename);
    }
    if (fwrite(left_wheel_x, 1, short_length, h) != short_length) {
        save_error(filename);
    }
    if (fwrite(left_wheel_y, 1, short_length, h) != short_length) {
        save_error(filename);
    }
    if (fwrite(right_wheel_x, 1, short_length, h) != short_length) {
        save_error(filename);
    }
    if (fwrite(right_wheel_y, 1, short_length, h) != short_length) {
        save_error(filename);
    }
    if (fwrite(body_x, 1, short_length, h) != short_length) {
        save_error(filename);
    }
    if (fwrite(body_y, 1, short_length, h) != short_length) {
        save_error(filename);
    }
    if (fwrite(bike_rotation, 1, short_length, h) != short_length) {
        save_error(filename);
    }
    if (fwrite(left_wheel_rotation, 1, char_length, h) != char_length) {
        save_error(filename);
    }
    if (fwrite(right_wheel_rotation, 1, char_length, h) != char_length) {
        save_error(filename);
    }
    if (fwrite(flags, 1, char_length, h) != char_length) {
        save_error(filename);
    }
    if (fwrite(motor_frequency, 1, char_length, h) != char_length) {
        save_error(filename);
    }
    if (fwrite(friction_volume, 1, char_length, h) != char_length) {
        save_error(filename);
    }

    if (fwrite(&event_count, 1, 4, h) != 4) {
        save_error(filename);
    }
    int event_length = event_count * sizeof(event);
    if (fwrite(events, 1, event_length, h) != event_length) {
        save_error(filename);
    }

    int magic_number = MAGIC_NUMBER;
    if (fwrite(&magic_number, 1, sizeof(magic_number), h) != 4) {
        save_error(filename);
    }

    if (!keep_file) {
        fclose(h);
    }
}

int recorder::load_replays(const char* filename, int demo) {
    FILE* h;
    if (demo) {
        h = qopen(filename, "rb");
        if (!h) {
            external_error("Could not open for reading record file!: !: ", filename);
        }
    } else {
        char path[40];
        sprintf(path, "rec/%s", filename);
        h = fopen(path, "rb");
        if (!h) {
            external_error("Could not open for reading record file!:", path);
        }
    }

    int frame_count_unused = 0;
    if (fread(&frame_count_unused, 1, sizeof(frame_count_unused), h) != 4) {
        read_error(filename);
    }

    int version = 0;
    if (fread(&version, 1, sizeof(version), h) != 4) {
        read_error(filename);
    }
    if (version < 131) {
        external_error("Recorded file version is too old!", filename);
    }
    if (version > 131) {
        external_error("Recorded file version is too new!", filename);
    }

    if (fread(&MultiplayerRec, 1, sizeof(MultiplayerRec), h) != 4) {
        read_error(filename);
    }

    if (demo) {
        qclose(h);
    } else {
        fclose(h);
    }

    int level_id = 0;
    if (MultiplayerRec) {
        FILE* h;
        if (demo) {
            h = qopen(filename, "rb");
            if (!h) {
                external_error("Failed to open demo file: ", filename);
            }
        } else {
            char path[40];
            sprintf(path, "rec\\%s", filename);
            h = fopen(path, "rb");
            if (!h) {
                external_error("Failed to open rec file: ", path);
            }
        }

        level_id = Rec1->load(filename, h, demo);
        Rec2->load(filename, h, demo);

        if (demo) {
            qclose(h);
        } else {
            fclose(h);
        }
    } else {
        level_id = Rec1->load(filename, nullptr, demo);
    }

    return level_id;
}

void recorder::save_replays(const char* filename, int level_id, int flagtag) {
    if (MultiplayerRec) {
        char path[40];
        sprintf(path, "rec/%s", filename);
        FILE* h = fopen(path, "wb");
        if (!h) {
            external_error("Failed to open rec file for writing!: ", path);
        }
        Rec1->save(filename, h, level_id, flagtag);
        Rec2->save(filename, h, level_id, flagtag);
        fclose(h);
    } else {
        Rec1->save(filename, nullptr, level_id, flagtag);
    }
}

constexpr int EVENT_BUFFER_MAX = 200;
static int EventBufferLength = 0;
static WavEvent EventBufferEventIds[EVENT_BUFFER_MAX];
static double EventBufferVolumes[EVENT_BUFFER_MAX];
static int EventBufferObjectIds[EVENT_BUFFER_MAX];

void add_event_buffer(WavEvent event_id, double volume, int object_id) {
    if (EventBufferLength < EVENT_BUFFER_MAX) {
        EventBufferEventIds[EventBufferLength] = event_id;
        EventBufferVolumes[EventBufferLength] = volume;
        EventBufferObjectIds[EventBufferLength] = object_id;
        EventBufferLength++;
    }
}

void reset_event_buffer() { EventBufferLength = 0; }

bool get_event_buffer(WavEvent* event_id, double* volume, int* object_id) {
    if (EventBufferLength == 0) {
        return false;
    }
    *event_id = EventBufferEventIds[0];
    *volume = EventBufferVolumes[0];
    *object_id = EventBufferObjectIds[0];
    EventBufferLength--;
    for (int i = 0; i < EventBufferLength; i++) {
        EventBufferEventIds[i] = EventBufferEventIds[i + 1];
        EventBufferVolumes[i] = EventBufferVolumes[i + 1];
        EventBufferObjectIds[i] = EventBufferObjectIds[i + 1];
    }
    return true;
}
