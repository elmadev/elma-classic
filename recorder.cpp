#include "recorder.h"
#include "flagtag.h"
#include "fs_utils.h"
#include "level.h"
#include "main.h"
#include "physics_init.h"
#include "platform_utils.h"
#include "qopen.h"
#include <cmath>
#include <cstring>

recorder* Rec1 = nullptr;
recorder* Rec2 = nullptr;
int MultiplayerRec = 0;

constexpr int MAGIC_NUMBER = 4796277;

constexpr int FRAME_RATE = 30;
constexpr double TIME_TO_FRAME_INDEX = FRAME_RATE / (STOPWATCH_MULTIPLIER * 1000.0 * 0.0024);
constexpr double FRAME_INDEX_TO_TIME = 1.0 / TIME_TO_FRAME_INDEX;

constexpr int INITIAL_FRAMES = FRAME_RATE * 60 * 60; // 30 fps * 60 sec * 60 min
constexpr int INITIAL_EVENTS = 46895;                // Magic constant from eol

constexpr int FRAME_CHUNK_SIZE = INITIAL_FRAMES;
constexpr int EVENT_CHUNK_SIZE = INITIAL_EVENTS;

constexpr int FLAG_GAS = 0;
constexpr int FLAG_FLIPPED = 1;
constexpr int FLAG_FLAGTAG_A = 2;
constexpr int FLAG_FLAGTAG_IMMUNITY = 3;

recorder::recorder() {
    frame_count = 0;
    event_count = 0;
    flagtag_ = 0;
    level_filename[0] = 0;

    frames.reserve(INITIAL_FRAMES);
    events.reserve(INITIAL_EVENTS);
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

    if (frames.size() > 2 * INITIAL_FRAMES) {
        frames.resize(INITIAL_FRAMES);
        frames.shrink_to_fit();
    }

    if (events.size() > 2 * INITIAL_EVENTS) {
        events.resize(INITIAL_EVENTS);
        events.shrink_to_fit();
    }
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
        frames[40 + i].flags = frames[40 + i].flags & 0x7F;
        if (encoded_value & 1) {
            frames[40 + i].flags += 0x80;
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
        if (frames[40 + 31 - i].flags & 0x80) {
            encoded_value += 1;
        }
    }
    return encoded_value == frame_count;
}

constexpr double POSITION_RATIO = 1000.0;
constexpr int WHEEL_ROTATION_RANGE = 250;
constexpr double WHEEL_ROTATION_RATIO = WHEEL_ROTATION_RANGE / (2.0 * PI);
constexpr int BIKE_ROTATION_RANGE = 10000;
constexpr double BIKE_ROTATION_RATIO = BIKE_ROTATION_RANGE / (2.0 * PI);
constexpr double MOTOR_FREQUENCY_RATIO = 250.0;
constexpr double FRICTION_VOLUME_RATIO = 250 / 2.0;

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

    mot->bike.r.x = frames[index1].bike_x * index1_weight + frames[index2].bike_x * index2_weight;
    mot->bike.r.y = frames[index1].bike_y * index1_weight + frames[index2].bike_y * index2_weight;

    double interp;
    interp =
        frames[index1].left_wheel_x * index1_weight + frames[index2].left_wheel_x * index2_weight;
    mot->left_wheel.r.x = mot->bike.r.x + interp / POSITION_RATIO;

    interp =
        frames[index1].left_wheel_y * index1_weight + frames[index2].left_wheel_y * index2_weight;
    mot->left_wheel.r.y = mot->bike.r.y + interp / POSITION_RATIO;

    interp =
        frames[index1].right_wheel_x * index1_weight + frames[index2].right_wheel_x * index2_weight;
    mot->right_wheel.r.x = mot->bike.r.x + interp / POSITION_RATIO;

    interp =
        frames[index1].right_wheel_y * index1_weight + frames[index2].right_wheel_y * index2_weight;
    mot->right_wheel.r.y = mot->bike.r.y + interp / POSITION_RATIO;

    interp = frames[index1].body_x * index1_weight + frames[index2].body_x * index2_weight;
    mot->body_r.x = mot->bike.r.x + interp / POSITION_RATIO;

    interp = frames[index1].body_y * index1_weight + frames[index2].body_y * index2_weight;
    mot->body_r.y = mot->bike.r.y + interp / POSITION_RATIO;

    int bike_rot1 = frames[index1].bike_rotation;
    int bike_rot2 = frames[index2].bike_rotation;
    if (abs(bike_rot1 - bike_rot2) > BIKE_ROTATION_RANGE / 2) {
        if (bike_rot1 > bike_rot2) {
            bike_rot1 -= BIKE_ROTATION_RANGE;
        } else {
            bike_rot2 -= BIKE_ROTATION_RANGE;
        }
    }
    interp = bike_rot1 * index1_weight + bike_rot2 * index2_weight;
    mot->bike.rotation = interp / BIKE_ROTATION_RATIO;

    int left_rot1 = frames[index1].left_wheel_rotation;
    int left_rot2 = frames[index2].left_wheel_rotation;
    if (abs(left_rot1 - left_rot2) > WHEEL_ROTATION_RANGE / 2) {
        if (left_rot1 > left_rot2) {
            left_rot1 -= WHEEL_ROTATION_RANGE;
        } else {
            left_rot2 -= WHEEL_ROTATION_RANGE;
        }
    }
    interp = left_rot1 * index1_weight + left_rot2 * index2_weight;
    mot->left_wheel.rotation = interp / WHEEL_ROTATION_RATIO;

    int right_rot1 = frames[index1].right_wheel_rotation;
    int right_rot2 = frames[index2].right_wheel_rotation;
    if (abs(right_rot1 - right_rot2) > WHEEL_ROTATION_RANGE / 2) {
        if (right_rot1 > right_rot2) {
            right_rot1 -= WHEEL_ROTATION_RANGE;
        } else {
            right_rot2 -= WHEEL_ROTATION_RANGE;
        }
    }
    interp = right_rot1 * index1_weight + right_rot2 * index2_weight;
    mot->right_wheel.rotation = interp / WHEEL_ROTATION_RATIO;

    sound->gas = char((frames[index1].flags >> FLAG_GAS) & 1);
    mot->flipped_bike = (frames[index1].flags >> FLAG_FLIPPED) & 1;
    FlagTagAHasFlag = (frames[index1].flags >> FLAG_FLAGTAG_A) & 1;
    FlagTagImmunity = (frames[index1].flags >> FLAG_FLAGTAG_IMMUNITY) & 1;

    sound->motor_frequency = 1.0 + frames[index1].motor_frequency / MOTOR_FREQUENCY_RATIO;
    sound->friction_volume = frames[index1].friction_volume / FRICTION_VOLUME_RATIO;

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

        if (next_frame_index >= frames.size()) {
            frames.resize(next_frame_index + FRAME_CHUNK_SIZE);
        }

        int i = next_frame_index;
        frames[i].bike_x = interpolated_bike_r.x;
        frames[i].bike_y = interpolated_bike_r.y;
        frames[i].left_wheel_x = (short)((mot->left_wheel.r.x - mot->bike.r.x) * POSITION_RATIO);
        frames[i].left_wheel_y = (short)((mot->left_wheel.r.y - mot->bike.r.y) * POSITION_RATIO);
        frames[i].right_wheel_x = (short)((mot->right_wheel.r.x - mot->bike.r.x) * POSITION_RATIO);
        frames[i].right_wheel_y = (short)((mot->right_wheel.r.y - mot->bike.r.y) * POSITION_RATIO);
        frames[i].body_x = (short)((mot->body_r.x - mot->bike.r.x) * POSITION_RATIO);
        frames[i].body_y = (short)((mot->body_r.y - mot->bike.r.y) * POSITION_RATIO);

        double bike_rot = mot->bike.rotation;
        while (bike_rot <= 0) {
            bike_rot += 2.0 * PI;
        }
        while (bike_rot > 2.0 * PI) {
            bike_rot -= 2.0 * PI;
        }
        frames[i].bike_rotation = (short)(bike_rot * BIKE_ROTATION_RATIO);

        // Rotation is constrained between +- Pi, except when doing a break-stretch
        // During the brake-stretch, the wheel position might become slightly desynced
        if (mot->left_wheel.rotation <= 0) {
            frames[i].left_wheel_rotation =
                (unsigned char)((mot->left_wheel.rotation + 2.0 * PI) * WHEEL_ROTATION_RATIO);
        } else {
            frames[i].left_wheel_rotation =
                (unsigned char)(mot->left_wheel.rotation * WHEEL_ROTATION_RATIO);
        }
        if (mot->right_wheel.rotation <= 0) {
            frames[i].right_wheel_rotation =
                (unsigned char)((mot->right_wheel.rotation + 2.0 * PI) * WHEEL_ROTATION_RATIO);
        } else {
            frames[i].right_wheel_rotation =
                (unsigned char)(mot->right_wheel.rotation * WHEEL_ROTATION_RATIO);
        }

        // Encode gibberish into the 4 MSB of the flags
        // Due to a bug, the gibberish is accidentally sourced from the y value of the bike
        memcpy(&frames[i].flags, &interpolated_bike_r.y, 1);
        frames[i].flags = frames[i].flags & 0xf0;

        if (sound->gas) {
            frames[i].flags += 1 << FLAG_GAS;
        }
        if (mot->flipped_bike) {
            frames[i].flags += 1 << FLAG_FLIPPED;
        }
        if (FlagTagAHasFlag) {
            frames[i].flags += 1 << FLAG_FLAGTAG_A;
        }
        if (FlagTagImmunity) {
            frames[i].flags += 1 << FLAG_FLAGTAG_IMMUNITY;
        }

        if (sound->motor_frequency < 1.0) {
            sound->motor_frequency = 1.0;
        }
        frames[i].motor_frequency =
            (unsigned char)(MOTOR_FREQUENCY_RATIO * (sound->motor_frequency - 1.0));
        frames[i].friction_volume = (unsigned char)(FRICTION_VOLUME_RATIO * sound->friction_volume);

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
    if (event_count > 0) {
        if (events[event_count - 1].time > time + 0.00001) {
            char tmp[50];
            double time2 = events[event_count - 1].time;
            sprintf(tmp, "time1: negative time: %f\n", float(time - time2));
            internal_error(tmp);
        }
    }

    if (event_count >= events.size()) {
        events.resize(event_count + EVENT_CHUNK_SIZE);
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

int recorder::load(const char* filename, FILE* h, int demo, bool is_first_replay) {
    frame_count = 0;
    if (fread(&frame_count, 1, sizeof(frame_count), h) != 4) {
        read_error(filename);
    }
    if (frame_count <= 0) {
        internal_error("recorder frame_count <= 0: ", filename);
    }

    frames.resize(frame_count);

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

    int multiplayer_rec = 0;
    if (fread(&multiplayer_rec, 1, sizeof(multiplayer_rec), h) != 4) {
        read_error(filename);
    }
    if (is_first_replay) {
        MultiplayerRec = multiplayer_rec;
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

#define READ_FIELD(field)                                                                          \
    {                                                                                              \
        for (int i = 0; i < frame_count; i++) {                                                    \
            if (fread(&frames[i].field, 1, sizeof(frames[i].field), h) !=                          \
                sizeof(frames[i].field)) {                                                         \
                read_error(filename);                                                              \
            }                                                                                      \
        }                                                                                          \
    }
    READ_FIELD(bike_x);
    READ_FIELD(bike_y);
    READ_FIELD(left_wheel_x);
    READ_FIELD(left_wheel_y);
    READ_FIELD(right_wheel_x);
    READ_FIELD(right_wheel_y);
    READ_FIELD(body_x);
    READ_FIELD(body_y);
    READ_FIELD(bike_rotation);
    READ_FIELD(left_wheel_rotation);
    READ_FIELD(right_wheel_rotation);
    READ_FIELD(flags);
    READ_FIELD(motor_frequency);
    READ_FIELD(friction_volume);
#undef READ_FIELD

    if (fread(&event_count, 1, 4, h) != 4) {
        read_error(filename);
    }
    if (event_count < 0) {
        internal_error("recorder event_count < 0!");
    }

    events.resize(event_count);

    int event_length = event_count * sizeof(event);
    if (fread(events.data(), 1, event_length, h) != event_length) {
        read_error(filename);
    }

    int magic_number = 0;
    if (fread(&magic_number, 1, sizeof(magic_number), h) != 4) {
        read_error(filename);
    }
    if (magic_number != MAGIC_NUMBER) {
        internal_error("magic_number != MAGIC_NUMBER");
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

#define WRITE_FIELD(field)                                                                         \
    {                                                                                              \
        for (int i = 0; i < frame_count; i++) {                                                    \
            if (fwrite(&frames[i].field, 1, sizeof(frames[i].field), h) !=                         \
                sizeof(frames[i].field)) {                                                         \
                save_error(filename);                                                              \
            }                                                                                      \
        }                                                                                          \
    }
    WRITE_FIELD(bike_x);
    WRITE_FIELD(bike_y);
    WRITE_FIELD(left_wheel_x);
    WRITE_FIELD(left_wheel_y);
    WRITE_FIELD(right_wheel_x);
    WRITE_FIELD(right_wheel_y);
    WRITE_FIELD(body_x);
    WRITE_FIELD(body_y);
    WRITE_FIELD(bike_rotation);
    WRITE_FIELD(left_wheel_rotation);
    WRITE_FIELD(right_wheel_rotation);
    WRITE_FIELD(flags);
    WRITE_FIELD(motor_frequency);
    WRITE_FIELD(friction_volume);
#undef WRITE_FIELD

    if (fwrite(&event_count, 1, 4, h) != 4) {
        save_error(filename);
    }
    int event_length = event_count * sizeof(event);
    if (fwrite(events.data(), 1, event_length, h) != event_length) {
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

int recorder::load_rec_file(const char* filename, int demo) {
    FILE* h = nullptr;
    if (demo) {
        h = qopen(filename, "rb");
        if (!h) {
            external_error("Failed to open demo file: ", filename);
        }
    } else {
        char path[40];
        sprintf(path, "rec/%s", filename);
        h = fopen(path, "rb");
        if (!h) {
            external_error("Failed to open rec file: ", path);
        }
    }

    int level_id = Rec1->load(filename, h, demo, true);
    if (MultiplayerRec) {
        Rec2->load(filename, h, demo, false);
    }

    if (demo) {
        qclose(h);
    } else {
        fclose(h);
    }

    return level_id;
}

void recorder::save_rec_file(const char* filename, int level_id, int flagtag) {
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

// Max events in one frame: Triple apple bug x MAX_OBJECTS, turn, volt, bump x 2
constexpr int EVENT_BUFFER_MAX = MAX_OBJECTS * 3 + 100;
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
