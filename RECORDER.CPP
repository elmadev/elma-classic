#include "RECORDER.H"
#include "flagtag.h"
#include "main.h"
#include "physics_init.h"
#include "platform_utils.h"
#include "qopen.h"
#include <cmath>
#include <cstring>

recorder *Rec1 = NULL, *Rec2 = NULL;
int MultiplayerRec = 0; // Ha multiplayer, akkor ez igaz

#define MAGIC_NUMBER (4796277l)

// #define MAX_FRAMES (10000u)
// #define MAX_EVENTS (3000u)

// Friss kikomentezes:
// constexpr int MAX_FRAMES = 14370;
// constexpr int MAX_EVENTS = 3000;

constexpr int MAX_FRAMES = 8981; // Ot perc
constexpr int MAX_EVENTS = 3900;

static double FRAME_RATE = 30; // 60;
static double TIME_TO_FRAME_INDEX = FRAME_RATE / (STOPWATCH_MULTIPLIER * 1000.0 * 0.0024);

recorder::recorder(void) {
    level_filename[0] = 0;
    bike_x = NULL;
    bike_y = NULL;
    left_wheel_x = NULL;
    left_wheel_y = NULL;
    right_wheel_x = NULL;
    right_wheel_y = NULL;
    body_x = NULL;
    body_y = NULL;
    left_wheel_rotation = NULL;
    bike_rotation = NULL;
    right_wheel_rotation = NULL;
    motor_frequency = NULL;
    friction_volume = NULL;
    flags = NULL;

    events = NULL;

    frame_count = 0;
    event_count = 0;

    flagtag_ = 0;

    // Ellenorzes:
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
    left_wheel_rotation = new unsigned char[MAX_FRAMES];
    bike_rotation = new short[MAX_FRAMES];
    right_wheel_rotation = new unsigned char[MAX_FRAMES];
    motor_frequency = new unsigned char[MAX_FRAMES];
    friction_volume = new unsigned char[MAX_FRAMES];
    flags = new unsigned char[MAX_FRAMES];

    events = new event[MAX_EVENTS];

    if (!bike_x || !bike_y || !left_wheel_x || !left_wheel_y || !right_wheel_x || !right_wheel_y ||
        !body_x || !body_y || !left_wheel_rotation || !bike_rotation || !right_wheel_rotation ||
        !motor_frequency || !flags || !events || !friction_volume) {
        external_error("recorder::recorder out of memory!");
    }
}

recorder::~recorder(void) {
    // Ez most nincs megcsinalva!
}

void recorder::erase(char* lev_filename) {
    if (strlen(lev_filename) > 12) {
        internal_error("recorder::erase strlen");
    }
    strcpy(level_filename, lev_filename);
    frame_count = 0;
    event_count = 0;
    finished = 0;
    current_event_index = 0;
    next_frame_index = 0;
}

void recorder::rewind(void) {
    finished = 0;
    current_event_index = 0;
    next_frame_index = 0;
}

int recorder::flagtag(void) { return flagtag_; }

void recorder::set_flagtag(int flagtag) { flagtag_ = flagtag; }

// Encode framecount into MSB of the flags
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

// Check that the framecount matches with the MSB of the flags
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

static double POSITION_RATIO = 1000.0, POSITION_RATIO_INV = 1.0 / POSITION_RATIO;
static double WHEEL_ROTATION_RATIO = 250 / (2.0 * PI),
              WHEEL_ROTATION_INV_RATIO = 1.0 / WHEEL_ROTATION_RATIO;
static double BIKE_ROTATION_RATIO = 10000 / (2.0 * PI),
              BIKE_ROTATION_INV_RATIO = 1.0 / BIKE_ROTATION_RATIO;
static double MOTOR_FREQUENCY_RATIO = 250.0,
              MOTOR_FREQUENCY_INV_RATIO = 1.0 / MOTOR_FREQUENCY_RATIO;
// static double MOTOR_FREQUENCY_RATIO = 250.0/2.0, 	   MOTOR_FREQUENCY_INV_RATIO = 1.0 /
// MOTOR_FREQUENCY_RATIO;
static double FRICTION_VOLUME_RATIO = 250 / 2.0,
              FRICTION_VOLUME_INV_RATIO = 1.0 / FRICTION_VOLUME_RATIO;

int recorder::recall_frame(motorst* mot, double time, bike_sound* sound) {
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
        return 0;
    }

    if (index1 >= frame_count - 1) {
        index1 = frame_count - 1;
        finished = 1;
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
        // Motor:
        if (abs(bike_rotation[index1] - bike_rotation[index2]) > 9000) { // 1000
            // mot->bike.rotation = BIKE_ROTATION_INV_RATIO * bike_rotation[index2];
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

        // Elso kerek:
        if (abs(left_wheel_rotation[index1] - left_wheel_rotation[index2]) > 220) { // 50
            // mot->left_wheel.rotation = WHEEL_ROTATION_INV_RATIO * left_wheel_rotation[index2];
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

        // Hatso kerek:
        if (abs(right_wheel_rotation[index1] - right_wheel_rotation[index2]) > 220) {
            // mot->right_wheel.rotation = WHEEL_ROTATION_INV_RATIO * right_wheel_rotation[index2];
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

    sound->gas = char(flags[index1] & 1);
    mot->flipped_bike = (flags[index1] & 2) >> 1;
    FlagTagAHasFlag = (flags[index1] & 4) >> 2;
    FlagTagImmunity = (flags[index1] & 8) >> 3;

    sound->motor_frequency = MOTOR_FREQUENCY_INV_RATIO * motor_frequency[index1] + 1.0;
    sound->friction_volume = FRICTION_VOLUME_INV_RATIO * friction_volume[index1];

    return 1;
}

static const double FRAME_INDEX_TO_TIME = 1.0 / TIME_TO_FRAME_INDEX;

void recorder::store_frames(motorst* mot, double time, bike_sound* sound) {
    if (!next_frame_index) {
        previous_bike_r = mot->bike.r;
        previous_frame_time = -0.00000000001;
        next_frame_time = 0.0;
    }
    if (time < next_frame_time) {
        previous_bike_r = mot->bike.r;
        previous_frame_time = time;
        return;
    }

    while (1) {
#ifdef DEBUG
        if (time - previous_frame_time == 0.0) {
            internal_error("time-previous_frame_time == 0.0!");
        }
#endif
        vect2 interpolated_bike_r =
            (mot->bike.r - previous_bike_r) *
                ((next_frame_time - previous_frame_time) / (time - previous_frame_time)) +
            previous_bike_r;

        if (next_frame_index >= MAX_FRAMES) {
            // betelt:
            return;
        }

        // Beirunk bejegyzesbe:
        int i = next_frame_index;
        bike_x[i] = interpolated_bike_r.x;
        bike_y[i] = interpolated_bike_r.y;
        left_wheel_x[i] = (mot->left_wheel.r.x - mot->bike.r.x) * POSITION_RATIO;
        left_wheel_y[i] = (mot->left_wheel.r.y - mot->bike.r.y) * POSITION_RATIO;
        right_wheel_x[i] = (mot->right_wheel.r.x - mot->bike.r.x) * POSITION_RATIO;
        right_wheel_y[i] = (mot->right_wheel.r.y - mot->bike.r.y) * POSITION_RATIO;
        body_x[i] = (mot->body_r.x - mot->bike.r.x) * POSITION_RATIO;
        body_y[i] = (mot->body_r.y - mot->bike.r.y) * POSITION_RATIO;

        // Alfak:
        double bike_rot = mot->bike.rotation;
        while (bike_rot <= 0) {
            bike_rot += TWO_PI;
        }
        while (bike_rot > TWO_PI) {
            bike_rot -= TWO_PI;
        }
        bike_rotation[i] = bike_rot * BIKE_ROTATION_RATIO;

        if (mot->left_wheel.rotation <= 0) {
            left_wheel_rotation[i] = (mot->left_wheel.rotation + TWO_PI) * WHEEL_ROTATION_RATIO;
        } else {
            left_wheel_rotation[i] = mot->left_wheel.rotation * WHEEL_ROTATION_RATIO;
        }
        if (mot->right_wheel.rotation <= 0) {
            right_wheel_rotation[i] = (mot->right_wheel.rotation + TWO_PI) * WHEEL_ROTATION_RATIO;
        } else {
            right_wheel_rotation[i] = mot->right_wheel.rotation * WHEEL_ROTATION_RATIO;
        }

        // Encode gibberish into the 4 MSB of the flags
        // Due to a bug, the gibberish is accidentally sourced from the y value of the bike
        memcpy(&flags[i], &interpolated_bike_r.y, 1);
        flags[i] = flags[i] & 0xf0;

        if (sound->gas) {
            flags[i] += 1;
        }
        if (mot->flipped_bike) {
            flags[i] += 2;
        }
        if (FlagTagAHasFlag) {
            flags[i] += 4;
        }
        if (FlagTagImmunity) {
            flags[i] += 8;
        }

        // flags[i] = char( sound->gas + (mot->hatra<<1) );

        if (sound->motor_frequency < 1.0) {
            sound->motor_frequency = 1.0;
        }
        motor_frequency[i] = MOTOR_FREQUENCY_RATIO * (sound->motor_frequency - 1.0);
        friction_volume[i] = FRICTION_VOLUME_RATIO * sound->friction_volume;

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
        if (events[unsigned(event_count - 1)].time > time + 0.00001) {
            char tmp[50];
            double time2 = events[unsigned(event_count - 1)].time;
            sprintf(tmp, "time1: negative time: %f\n", float(time - time2));
            internal_error(tmp);
            // internal_error( "recorder::store_event-ban time forditott sorrendben!" );
        }
    }
    events[unsigned(event_count)].time = time;
    events[unsigned(event_count)].event_id = event_id;
    events[unsigned(event_count)].volume = volume;
    events[unsigned(event_count)].object_id = (short)object_id;
    event_count++;
}

// Ha van hang, akkor igazzal ter vissza:
int recorder::recall_event(double time, WavEvent* event_id, double* volume, int* object_id) {
    if (current_event_index < event_count) {
        if (events[unsigned(current_event_index)].time <= time) {
            *event_id = events[unsigned(current_event_index)].event_id;
            *volume = events[unsigned(current_event_index)].volume;
            *object_id = events[unsigned(current_event_index)].object_id;
            current_event_index++;
            return 1;
        }
    }
    return 0;
}

static void read_error(const char* filename) {
    internal_error("Failed to read rec file: ", filename);
}

// demo eseten resource file-bol nyit:
// belyeg-et adja vissza, vagy 0-at:
int recorder::load(const char* filename, FILE* h, int demo) {
    int keep_file = 0;
    if (h) {
        keep_file = 1;
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
    if (fread(&frame_count, 1, 4, h) != 4) {
        read_error(filename);
    }
    if (frame_count <= 0) {
        internal_error("recorder frame_count <= 0: ", filename);
    }

    if (frame_count > MAX_FRAMES) {
        internal_error("recorder frame_count > MAX_FRAMES!");
    }

    int version = 0;
    if (fread(&version, 1, 4, h) != 4) {
        internal_error("Nem sikerult olvasni recorded file-bol!: ", filename);
    }

    // Ezt loadrecekben is igazitani kell:
    if (version < 131) {
        external_error("Rec file version is too old!", filename);
    }

    if (version > 131) {
        external_error("Rec file version is too new!", filename);
    }

    int multiplayer_rec_unused = 0; // Multirec, de ezt mar elintezte loadrecek, aki ezt hivta
    if (fread(&multiplayer_rec_unused, 1, 4, h) != 4) {
        internal_error("Nem sikerult olvasni recorded file-bol!: ", filename);
    }

    if (fread(&flagtag_, 1, 4, h) != 4) {
        internal_error("Nem sikerult olvasni recorded file-bol!: ", filename);
    }

    int level_id = 0;
    if (fread(&level_id, 1, 4, h) != 4) {
        read_error(filename);
    }
    if (fread(level_filename, 1, 16, h) != 16) {
        read_error(filename);
    }

    int float_length = frame_count * sizeof(float);
    // int hosszi = frame_count*sizeof( int );
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
    if (fread(flags, 1, char_length, h) != char_length) { // karakter
        read_error(filename);
    }

    if (fread(motor_frequency, 1, char_length, h) != char_length) {
        read_error(filename);
    }
    if (fread(friction_volume, 1, char_length, h) != char_length) {
        read_error(filename);
    }

    // Hangok:
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
    if (fread(&magic_number, 1, 4, h) != 4) {
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

    int keep_file = 0;
    if (h) {
        keep_file = 1;
    }

    if (frame_count == 0) {
        internal_error("recorder save frame_count == 0!");
    }

    if (!h) {
        // Itt meg kell hagyni fopen-t!:
        char path[40];
        sprintf(path, "rec/%s", filename);
        h = fopen(path, "wb");
        if (!h) {
            internal_error("Failed to open rec file for writing!: ", path);
        }
    }

    if (fwrite(&frame_count, 1, 4, h) != 4) {
        save_error(filename);
    }

    // Verzio csak 1.2-tol kezdodoen jon bele:
    // Elozoleg itt level volt, ami biztos hogy kisebb volt 120-nal:
    int version = 131;
    if (fwrite(&version, 1, 4, h) != 4) {
        save_error(filename);
    }

    // Ezt loadrecekben is olvassa, ugyhogy ennek verzio utan kell maradnia:
    if (fwrite(&MultiplayerRec, 1, 4, h) != 4) {
        save_error(filename);
    }

    if (fwrite(&flagtag_, 1, 4, h) != 4) {
        save_error(filename);
    }

    if (fwrite(&level_id, 1, 4, h) != 4) {
        save_error(filename);
    }

    if (fwrite(level_filename, 1, 16, h) != 16) {
        save_error(filename);
    }

    int float_length = frame_count * sizeof(float);
    // int hosszi = frame_count*sizeof( int );
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

    // Hangok:
    if (fwrite(&event_count, 1, 4, h) != 4) {
        save_error(filename);
    }
    int event_length = event_count * sizeof(event);
    if (fwrite(events, 1, event_length, h) != event_length) {
        save_error(filename);
    }

    int magic_number = MAGIC_NUMBER;
    if (fwrite(&magic_number, 1, 4, h) != 4) {
        save_error(filename);
    }

    if (!keep_file) {
        fclose(h);
    }
}

// demo eseten resource file-bol nyit:
int load_replays(const char* filename, int demo) {
    // Eloszor megvizsgaljuk, hogy multirec vagy nem:
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
    int frame_count_unused = 0; // frame_count (bejegyzes darabszam)
    if (fread(&frame_count_unused, 1, 4, h) != 4) {
        internal_error("Nem sikerult olvasni recorded file-bol!: ", filename);
    }

    int version = 0;
    if (fread(&version, 1, 4, h) != 4) {
        internal_error("Nem sikerult olvasni recorded file-bol!: ", filename);
    }

    if (version < 131) {
        external_error("Recorded file version is too old!", filename);
    }

    if (version > 131) {
        external_error("Recorded file version is too new!", filename);
    }

    // Betoltjuk Multirec-et:
    if (fread(&MultiplayerRec, 1, 4, h) != 4) {
        internal_error("Nem sikerult olvasni recorded file-bol!: ", filename);
    }

    if (demo) {
        qclose(h);
    } else {
        fclose(h);
    }

    // Most jon tenyleges beolvasas:
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
        level_id = Rec1->load(filename, NULL, demo);
    }

    return level_id;
}

// belyeg-et adja vissza, vagy 0-at:
void save_replays(const char* filename, int level_id, int flagtag) {
    if (MultiplayerRec) {
        char path[40];
        sprintf(path, "rec/%s", filename);
        FILE* h = fopen(path, "wb");
        if (!h) {
            external_error("Failed to open rec file for writing!: ", path);
        }

        // Multirec alapjan tudjak, hogy mit kell rec file multirec
        // rubrikajaba irni:
        Rec1->save(filename, h, level_id, flagtag);
        Rec2->save(filename, h, level_id, flagtag);

        fclose(h);
    } else {
        Rec1->save(filename, NULL, level_id, flagtag);
    }
}

// WAV GYUJTO:

static int EventBufferLength = 0;

#define EVENT_BUFFER_MAX (200)

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

void reset_event_buffer(void) { EventBufferLength = 0; }

// Igaz, ha van bent wav:
int get_event_buffer(WavEvent* event_id, double* volume, int* object_id) {
    if (EventBufferLength == 0) {
        return 0;
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
    return 1;
}
