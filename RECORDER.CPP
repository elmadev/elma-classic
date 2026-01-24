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

#define MAGICNUMBER (4796277l)

// #define Darabszam (10000u)
// #define Hangdarabszam (3000u)

// Friss kikomentezes:
// constexpr int Darabszam = 14370;
// constexpr int Hangdarabszam = 3000;

constexpr int Darabszam = 8981; // Ot perc
constexpr int Hangdarabszam = 3900;

static double Valtrate = 30; // 60;
static double Recorderszorzo = Valtrate / (STOPWATCH_MULTIPLIER * 1000.0 * 0.0024);

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
    hossz = Darabszam * sizeof(float);
    if (hossz > 64000l) {
        internal_error("Darabszam*sizeof( float )!");
    }

    hossz = Hangdarabszam * sizeof(event);
    if (hossz > 64000l) {
        internal_error("Hangdarabszam*sizeof( event )!");
    }

    bike_x = new float[Darabszam];
    bike_y = new float[Darabszam];
    left_wheel_x = new short[Darabszam];
    left_wheel_y = new short[Darabszam];
    right_wheel_x = new short[Darabszam];
    right_wheel_y = new short[Darabszam];
    body_x = new short[Darabszam];
    body_y = new short[Darabszam];
    left_wheel_rotation = new unsigned char[Darabszam];
    bike_rotation = new short[Darabszam];
    right_wheel_rotation = new unsigned char[Darabszam];
    motor_frequency = new unsigned char[Darabszam];
    friction_volume = new unsigned char[Darabszam];
    flags = new unsigned char[Darabszam];

    events = new event[Hangdarabszam];

    if (!bike_x || !bike_y || !left_wheel_x || !left_wheel_y || !right_wheel_x || !right_wheel_y ||
        !body_x || !body_y || !left_wheel_rotation || !bike_rotation || !right_wheel_rotation ||
        !motor_frequency || !flags || !events || !friction_volume) {
        external_error("memory");
    }
}

recorder::~recorder(void) {
    // Ez most nincs megcsinalva!
}

void recorder::erase(char* lev_filename) {
    if (strlen(lev_filename) > 12) {
        internal_error("875h8uyf");
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

static double Korrszorzo = 1000.0, Korrszorzo_rec = 1.0 / Korrszorzo;
static double Alfaszorzo = 250 / (2.0 * PI), Alfaszorzo_rec = 1.0 / Alfaszorzo;
static double Alfaszorzo_16 = 10000 / (2.0 * PI), Alfaszorzo_16_rec = 1.0 / Alfaszorzo_16;
static double Frekvszorzo = 250.0, Frekvszorzo_rec = 1.0 / Frekvszorzo;
// static double Frekvszorzo = 250.0/2.0, 	   Frekvszorzo_rec = 1.0 / Frekvszorzo;
static double Surlszorzo = 250 / 2.0, Surlszorzo_rec = 1.0 / Surlszorzo;

// int Mostirdki = 0;

/*static int Filebairsorszam = 0;

static void filebair( float d1, float d2, float d3 ) {
    FILE* h = fopen( "log.txt", "at" );
    if( !h )
        internal_error( "filebair-ban nem nyilik!" );

    fprintf( h, "%d %f %f %f\n", Filebairsorszam, d1, d2, d3 );


    Filebairsorszam++;
    fclose( h );
}*/

int recorder::recall_frame(motorst* mot, double time, bike_sound* sound) {
    if (frame_count <= 0) {
        internal_error("recall_frame-ban frame_count <= 0!");
    }

    int index1 = (int)(Recorderszorzo * time);
    double suly2 = Recorderszorzo * time - index1;
    if (suly2 < 0.0) {
        suly2 = 0.0;
    }
    if (suly2 > 1.0) {
        suly2 = 1.0;
    }
    double suly1 = 1.0 - suly2;
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

    mot->bike.r.x = bike_x[index1] * suly1 + bike_x[index2] * suly2;
    mot->bike.r.y = bike_y[index1] * suly1 + bike_y[index2] * suly2;
    mot->left_wheel.r.x = mot->bike.r.x + Korrszorzo_rec * (left_wheel_x[index1] * suly1 +
                                                            left_wheel_x[index2] * suly2);
    mot->left_wheel.r.y = mot->bike.r.y + Korrszorzo_rec * (left_wheel_y[index1] * suly1 +
                                                            left_wheel_y[index2] * suly2);
    mot->right_wheel.r.x = mot->bike.r.x + Korrszorzo_rec * (right_wheel_x[index1] * suly1 +
                                                             right_wheel_x[index2] * suly2);
    mot->right_wheel.r.y = mot->bike.r.y + Korrszorzo_rec * (right_wheel_y[index1] * suly1 +
                                                             right_wheel_y[index2] * suly2);
    mot->body_r.x =
        mot->bike.r.x + Korrszorzo_rec * (body_x[index1] * suly1 + body_x[index2] * suly2);
    mot->body_r.y =
        mot->bike.r.y + Korrszorzo_rec * (body_y[index1] * suly1 + body_y[index2] * suly2);

    if (index1 >= 2) {
        // Motor:
        if (abs(bike_rotation[index1] - bike_rotation[index2]) > 9000) { // 1000
            // mot->bike.rotation = Alfaszorzo_16_rec * bike_rotation[index2];
            if (bike_rotation[index1] > bike_rotation[index2]) {
                int fixedindex1 = bike_rotation[index1] - 10000;
                mot->bike.rotation =
                    Alfaszorzo_16_rec * (fixedindex1 * suly1 + bike_rotation[index2] * suly2);
            } else {
                int fixedindex2 = bike_rotation[index2] - 10000;
                mot->bike.rotation = Alfaszorzo_16_rec *
                                     (bike_rotation[index1] * suly1 * suly1 + fixedindex2 * suly2);
            }
        } else {
            mot->bike.rotation =
                Alfaszorzo_16_rec * (bike_rotation[index1] * suly1 + bike_rotation[index2] * suly2);
        }

        // Elso kerek:
        if (abs(left_wheel_rotation[index1] - left_wheel_rotation[index2]) > 220) { // 50
            // mot->left_wheel.rotation = Alfaszorzo_rec * left_wheel_rotation[index2];
            if (left_wheel_rotation[index1] > left_wheel_rotation[index2]) {
                int fixedindex1 = left_wheel_rotation[index1] - 250;
                mot->left_wheel.rotation =
                    Alfaszorzo_rec * (fixedindex1 * suly1 + left_wheel_rotation[index2] * suly2);
            } else {
                int fixedindex2 = left_wheel_rotation[index2] - 250;
                mot->left_wheel.rotation =
                    Alfaszorzo_rec *
                    (left_wheel_rotation[index1] * suly1 * suly1 + fixedindex2 * suly2);
            }
        } else {
            mot->left_wheel.rotation = Alfaszorzo_rec * (left_wheel_rotation[index1] * suly1 +
                                                         left_wheel_rotation[index2] * suly2);
        }

        // Hatso kerek:
        if (abs(right_wheel_rotation[index1] - right_wheel_rotation[index2]) > 220) {
            // mot->right_wheel.rotation = Alfaszorzo_rec * right_wheel_rotation[index2];
            if (right_wheel_rotation[index1] > right_wheel_rotation[index2]) {
                int fixedindex1 = right_wheel_rotation[index1] - 250;
                mot->right_wheel.rotation =
                    Alfaszorzo_rec * (fixedindex1 * suly1 + right_wheel_rotation[index2] * suly2);
            } else {
                int fixedindex2 = right_wheel_rotation[index2] - 250;
                mot->right_wheel.rotation =
                    Alfaszorzo_rec *
                    (right_wheel_rotation[index1] * suly1 * suly1 + fixedindex2 * suly2);
            }
        } else {
            mot->right_wheel.rotation = Alfaszorzo_rec * (right_wheel_rotation[index1] * suly1 +
                                                          right_wheel_rotation[index2] * suly2);
        }

    } else {
        mot->bike.rotation = Alfaszorzo_16_rec * bike_rotation[index2];
        mot->left_wheel.rotation = Alfaszorzo_rec * left_wheel_rotation[index2];
        mot->right_wheel.rotation = Alfaszorzo_rec * right_wheel_rotation[index2];
    }

    // filebair( index2, left_wheel_rotation[index2], mot->left_wheel.rotation );
    sound->gas = char(flags[index1] & 1);
    mot->flipped_bike = (flags[index1] & 2) >> 1;
    FlagTagAHasFlag = (flags[index1] & 4) >> 2;
    FlagTagImmunity = (flags[index1] & 8) >> 3;

    /*if( Mostirdki ) {
        char tmp[100];
        int tmpfreki = motor_frequency[index1];
        sprintf( tmp, "f:%d, index1:%d", tmpfreki, index1 );
        internal_error( "Itt van eredmeny:", tmp );
    }*/

    sound->motor_frequency = Frekvszorzo_rec * motor_frequency[index1] + 1.0;
    sound->friction_volume = Surlszorzo_rec * friction_volume[index1];

    return 1;
}

static const double Reclepesido = 1.0 / Recorderszorzo;

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
        vect2 kor1r = (mot->bike.r - previous_bike_r) *
                          ((next_frame_time - previous_frame_time) / (time - previous_frame_time)) +
                      previous_bike_r;

        if (next_frame_index >= Darabszam) {
            // betelt:
            return;
        }

        // Beirunk bejegyzesbe:
        int i = next_frame_index;
        bike_x[i] = kor1r.x;
        bike_y[i] = kor1r.y;
        left_wheel_x[i] = (mot->left_wheel.r.x - mot->bike.r.x) * Korrszorzo;
        left_wheel_y[i] = (mot->left_wheel.r.y - mot->bike.r.y) * Korrszorzo;
        right_wheel_x[i] = (mot->right_wheel.r.x - mot->bike.r.x) * Korrszorzo;
        right_wheel_y[i] = (mot->right_wheel.r.y - mot->bike.r.y) * Korrszorzo;
        body_x[i] = (mot->body_r.x - mot->bike.r.x) * Korrszorzo;
        body_y[i] = (mot->body_r.y - mot->bike.r.y) * Korrszorzo;

        // Alfak:
        double alfa1 = mot->bike.rotation;
        while (alfa1 <= 0) {
            alfa1 += TWO_PI;
        }
        while (alfa1 > TWO_PI) {
            alfa1 -= TWO_PI;
        }
        bike_rotation[i] = alfa1 * Alfaszorzo_16;

        if (mot->left_wheel.rotation <= 0) {
            left_wheel_rotation[i] = (mot->left_wheel.rotation + TWO_PI) * Alfaszorzo;
        } else {
            left_wheel_rotation[i] = mot->left_wheel.rotation * Alfaszorzo;
        }
        if (mot->right_wheel.rotation <= 0) {
            right_wheel_rotation[i] = (mot->right_wheel.rotation + TWO_PI) * Alfaszorzo;
        } else {
            right_wheel_rotation[i] = mot->right_wheel.rotation * Alfaszorzo;
        }

        // Encode gibberish into the 4 MSB of the flags
        // Due to a bug, the gibberish is accidentally sourced from the y value of the bike
        memcpy(&flags[i], &kor1r.y, 1);
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
        motor_frequency[i] = Frekvszorzo * (sound->motor_frequency - 1.0);
        friction_volume[i] = Surlszorzo * sound->friction_volume;

        next_frame_index++;
        next_frame_time += Reclepesido;
        if (time < next_frame_time) {
            previous_bike_r = mot->bike.r;
            previous_frame_time = time;
            frame_count = next_frame_index;
            return;
        }
    }
}

/*void recorder::store_frames( motorst* mot, double dt, bike_sound* sound ) {
    int index = Recorderszorzo*dt;
    if( index < 0 )
        index = 0;
    if( index >= Darabszam )
        // betelt:
        return;
    if( index < frame_count-1 )
        internal_error( "recorder::store_frames-ban index < frame_count-1!" );

    for( int i = frame_count; i <= index; i++ ) {
        bike_x[i] = mot->bike.r.x;
        bike_y[i] = mot->bike.r.y;
        left_wheel_x[i] = (mot->left_wheel.r.x-mot->bike.r.x)*Korrszorzo;
        left_wheel_y[i] = (mot->left_wheel.r.y-mot->bike.r.y)*Korrszorzo;
        right_wheel_x[i] = (mot->right_wheel.r.x-mot->bike.r.x)*Korrszorzo;
        right_wheel_y[i] = (mot->right_wheel.r.y-mot->bike.r.y)*Korrszorzo;
        body_x[i] = (mot->body_r.x-mot->bike.r.x)*Korrszorzo;
        body_y[i] = (mot->body_r.y-mot->bike.r.y)*Korrszorzo;

        // torlendo
        //int hatar = 2000;
        //if( left_wheel_x[i] > hatar || right_wheel_x[i] > hatar ||
        //	left_wheel_y[i] > hatar || right_wheel_y[i] > hatar )
        //	external_error( "Nagyobb 5000-nel!" );
        //if( left_wheel_x[i] < -hatar || right_wheel_x[i] < -hatar ||
        //	left_wheel_y[i] < -hatar || right_wheel_y[i] < -hatar )
        //	external_error( "Kisebb -5000-nel!" );

        // Alfak:
        double alfa1 = mot->bike.rotation;
        while( alfa1 <= 0 )
            alfa1 += TWO_PI;
        while( alfa1 > TWO_PI )
            alfa1 -= TWO_PI;
        bike_rotation[i] = alfa1*Alfaszorzo_16;

        if( mot->left_wheel.rotation <= 0 )
            left_wheel_rotation[i] = (mot->left_wheel.rotation + TWO_PI)*Alfaszorzo;
        else
            left_wheel_rotation[i] = mot->left_wheel.rotation*Alfaszorzo;
        if( mot->right_wheel.rotation <= 0 )
            right_wheel_rotation[i] = (mot->right_wheel.rotation + TWO_PI)*Alfaszorzo;
        else
            right_wheel_rotation[i] = mot->right_wheel.rotation*Alfaszorzo;

        if( sound->gas )
            flags[i] = 1;
        else
            flags[i] = 0;

        if( mot->hatra )
            flags[i] += 2;
        if( FlagTagAHasFlag )
            flags[i] += 4;
        if( FlagTagImmunity )
            flags[i] += 8;

        //flags[i] = char( sound->gas + (mot->hatra<<1) );

        if( sound->motor_frequency > 2.0 )
            sound->motor_frequency = 2.0;
        motor_frequency[i] = Frekvszorzo * sound->motor_frequency;
        friction_volume[i]    = Surlszorzo  * sound->friction_volume;
    }
    frame_count = index+1;
}*/

void recorder::store_event(double time, WavEvent event_id, double volume, int object_id) {
    if (event_count >= Hangdarabszam) {
        return;
    }
    if (event_count > 0) {
        if (events[unsigned(event_count - 1)].time > time + 0.00001) {
            char tmp[50];
            double t2 = events[unsigned(event_count - 1)].time;
            sprintf(tmp, "t1: kulonbseg: %f\n", float(time - t2));
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

static void olvhiba(const char* nev) {
    internal_error("Nem sikerult olvasni recorded file-bol!: ", nev);
}

// demo eseten resource file-bol nyit:
// belyeg-et adja vissza, vagy 0-at:
int recorder::load(const char* filename, FILE* h, int demo) {
    int megvoltnyitva = 0;
    if (h) {
        megvoltnyitva = 1;
    }

    if (!h) {
        if (demo) {
            h = qopen(filename, "rb");
            if (!h) {
                internal_error("Nem sikerult megnyitni recorded file-t!: ", filename);
            }
        } else {
            char tmpnev[40];
            sprintf(tmpnev, "rec/%s", filename);
            h = fopen(tmpnev, "rb");
            if (!h) {
                external_error("Could not open for reading record file!:", tmpnev);
            }
        }
    }

    frame_count = 0;
    if (fread(&frame_count, 1, 4, h) != 4) {
        olvhiba(filename);
    }
    if (frame_count <= 0) {
        internal_error("Nem stimmel frame_count recorded file-ban!: ", filename);
    }

    if (frame_count > Darabszam) {
        internal_error("recorder load-ban frame_count > Darabszam!");
    }

    int verzio = 0;
    if (fread(&verzio, 1, 4, h) != 4) {
        internal_error("Nem sikerult olvasni recorded file-bol!: ", filename);
    }

    // Ezt loadrecekben is igazitani kell:
    if (verzio < 131) {
        external_error("Recorded file version is too old!", filename);
    }

    if (verzio > 131) {
        external_error("Recorded file version is too new!", filename);
    }

    int tmp = 0; // Multirec, de ezt mar elintezte loadrecek, aki ezt hivta
    if (fread(&tmp, 1, 4, h) != 4) {
        internal_error("Nem sikerult olvasni recorded file-bol!: ", filename);
    }

    if (fread(&flagtag_, 1, 4, h) != 4) {
        internal_error("Nem sikerult olvasni recorded file-bol!: ", filename);
    }

    int belyeg = 0;
    if (fread(&belyeg, 1, 4, h) != 4) {
        olvhiba(filename);
    }
    if (fread(level_filename, 1, 16, h) != 16) {
        olvhiba(filename);
    }

    int hosszf = frame_count * sizeof(float);
    // int hosszi = frame_count*sizeof( int );
    int hosszc = frame_count * sizeof(char);
    int hosszs = frame_count * sizeof(short);

    if (fread(bike_x, 1, hosszf, h) != hosszf) {
        olvhiba(filename);
    }
    if (fread(bike_y, 1, hosszf, h) != hosszf) {
        olvhiba(filename);
    }
    if (fread(left_wheel_x, 1, hosszs, h) != hosszs) {
        olvhiba(filename);
    }
    if (fread(left_wheel_y, 1, hosszs, h) != hosszs) {
        olvhiba(filename);
    }
    if (fread(right_wheel_x, 1, hosszs, h) != hosszs) {
        olvhiba(filename);
    }
    if (fread(right_wheel_y, 1, hosszs, h) != hosszs) {
        olvhiba(filename);
    }
    if (fread(body_x, 1, hosszs, h) != hosszs) {
        olvhiba(filename);
    }
    if (fread(body_y, 1, hosszs, h) != hosszs) {
        olvhiba(filename);
    }

    if (fread(bike_rotation, 1, hosszs, h) != hosszs) {
        olvhiba(filename);
    }
    if (fread(left_wheel_rotation, 1, hosszc, h) != hosszc) {
        olvhiba(filename);
    }
    if (fread(right_wheel_rotation, 1, hosszc, h) != hosszc) {
        olvhiba(filename);
    }
    if (fread(flags, 1, hosszc, h) != hosszc) { // karakter
        olvhiba(filename);
    }

    if (fread(motor_frequency, 1, hosszc, h) != hosszc) {
        olvhiba(filename);
    }
    if (fread(friction_volume, 1, hosszc, h) != hosszc) {
        olvhiba(filename);
    }

    // Hangok:
    if (fread(&event_count, 1, 4, h) != 4) {
        olvhiba(filename);
    }
    if (event_count < 0) {
        internal_error("event_count nem stimmel loadban!");
    }
    if (event_count > Hangdarabszam) {
        internal_error("rec load-ban event_count > Hangdarabszam!");
    }

    /*static int elsoszam = 0;
    if( elsoszam ) {
        char tmp11[50];
        sprintf( tmp11, "1.: %d, 2.: %d", event_count, elsoszam );
        external_error( tmp11 );
    }
    elsoszam = event_count;*/

    int hossz = event_count * sizeof(event);
    if (fread(events, 1, hossz, h) != hossz) {
        olvhiba(filename);
    }

    int magic = 0;
    if (fread(&magic, 1, 4, h) != 4) {
        olvhiba(filename);
    }
    if (magic != MAGICNUMBER) {
        internal_error("magic != MAGICNUMBER");
    }

    if (!megvoltnyitva) {
        if (demo) {
            qclose(h);
        } else {
            fclose(h);
        }
    }

    // if( !frame_count_integrity( this ) )
    //	internal_error( "recorder::load-ban !frame_count_integrity( this )!" );
    return belyeg;
}

/* 1.0 es 1.1 eseten ez volt:
    h = fopen( nev, "wb" );
    if( !h )
        internal_error( "Nem sikerult megnyitni irasra recorded file-t!: ", nev );

    if( fwrite( &frame_count, 1, 4, h ) != 4 )
        internal_error( "Nem sikerult irni recorded file-ba!: ", nev );
    if( fwrite( &level, 1, 4, h ) != 4 )
        internal_error( "Nem sikerult irni recorded file-ba!: ", nev );


    long hossz = frame_count*sizeof( float );

    if( fwrite( bike_x, 1, unsigned(hossz), h ) != hossz )
        internal_error( "Nem sikerult irni recorded file-ba!: ", nev );
*/

static void irhiba(const char* nev) {
    internal_error("Nem sikerult irni recorded file-ba!: ", nev);
}

void recorder::save(const char* filename, FILE* h, int level_id, int flagtag) {
    flagtag_ = flagtag;

    int megvoltnyitva = 0;
    if (h) {
        megvoltnyitva = 1;
    }

    if (frame_count == 0) {
        internal_error("save-ben frame_count == 0!");
    }

    if (!h) {
        // Itt meg kell hagyni fopen-t!:
        char tmpnev[40];
        sprintf(tmpnev, "rec/%s", filename);
        h = fopen(tmpnev, "wb");
        if (!h) {
            internal_error("Could not open for writing record file!: ", tmpnev);
        }
    }

    if (fwrite(&frame_count, 1, 4, h) != 4) {
        irhiba(filename);
    }

    // Verzio csak 1.2-tol kezdodoen jon bele:
    // Elozoleg itt level volt, ami biztos hogy kisebb volt 120-nal:
    int verzio = 131;
    if (fwrite(&verzio, 1, 4, h) != 4) {
        irhiba(filename);
    }

    // Ezt loadrecekben is olvassa, ugyhogy ennek verzio utan kell maradnia:
    if (fwrite(&MultiplayerRec, 1, 4, h) != 4) {
        irhiba(filename);
    }

    if (fwrite(&flagtag_, 1, 4, h) != 4) {
        irhiba(filename);
    }

    if (fwrite(&level_id, 1, 4, h) != 4) {
        irhiba(filename);
    }

    if (fwrite(level_filename, 1, 16, h) != 16) {
        irhiba(filename);
    }

    int hosszf = frame_count * sizeof(float);
    // int hosszi = frame_count*sizeof( int );
    int hosszc = frame_count * sizeof(char);
    int hosszs = frame_count * sizeof(short);

    if (fwrite(bike_x, 1, hosszf, h) != hosszf) {
        irhiba(filename);
    }
    if (fwrite(bike_y, 1, hosszf, h) != hosszf) {
        irhiba(filename);
    }
    if (fwrite(left_wheel_x, 1, hosszs, h) != hosszs) {
        irhiba(filename);
    }
    if (fwrite(left_wheel_y, 1, hosszs, h) != hosszs) {
        irhiba(filename);
    }
    if (fwrite(right_wheel_x, 1, hosszs, h) != hosszs) {
        irhiba(filename);
    }
    if (fwrite(right_wheel_y, 1, hosszs, h) != hosszs) {
        irhiba(filename);
    }

    if (fwrite(body_x, 1, hosszs, h) != hosszs) {
        irhiba(filename);
    }
    if (fwrite(body_y, 1, hosszs, h) != hosszs) {
        irhiba(filename);
    }

    if (fwrite(bike_rotation, 1, hosszs, h) != hosszs) {
        irhiba(filename);
    }
    if (fwrite(left_wheel_rotation, 1, hosszc, h) != hosszc) {
        irhiba(filename);
    }
    if (fwrite(right_wheel_rotation, 1, hosszc, h) != hosszc) {
        irhiba(filename);
    }
    if (fwrite(flags, 1, hosszc, h) != hosszc) {
        irhiba(filename);
    }

    if (fwrite(motor_frequency, 1, hosszc, h) != hosszc) {
        irhiba(filename);
    }
    if (fwrite(friction_volume, 1, hosszc, h) != hosszc) {
        irhiba(filename);
    }

    // Hangok:
    if (fwrite(&event_count, 1, 4, h) != 4) {
        irhiba(filename);
    }
    int hossz = event_count * sizeof(event);
    if (fwrite(events, 1, hossz, h) != hossz) {
        irhiba(filename);
    }

    int magic = MAGICNUMBER;
    if (fwrite(&magic, 1, 4, h) != 4) {
        irhiba(filename);
    }

    if (!megvoltnyitva) {
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
        char tmpnev[40];
        sprintf(tmpnev, "rec/%s", filename);
        h = fopen(tmpnev, "rb");
        if (!h) {
            external_error("Could not open for reading record file!:", tmpnev);
        }
    }
    int tmp = 0; // frame_count (bejegyzes darabszam)
    if (fread(&tmp, 1, 4, h) != 4) {
        internal_error("Nem sikerult olvasni recorded file-bol!: ", filename);
    }

    int verzio = 0;
    if (fread(&verzio, 1, 4, h) != 4) {
        internal_error("Nem sikerult olvasni recorded file-bol!: ", filename);
    }

    if (verzio < 131) {
        external_error("Recorded file version is too old!", filename);
    }

    if (verzio > 131) {
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
    int belyeg = 0;
    if (MultiplayerRec) {
        FILE* h;
        if (demo) {
            h = qopen(filename, "rb");
            if (!h) {
                external_error("Could not open for reading demo record file!:", filename);
            }
        } else {
            char tmpnev[40];
            sprintf(tmpnev, "rec\\%s", filename);
            h = fopen(tmpnev, "rb");
            if (!h) {
                external_error("Could not open for reading record file!:", tmpnev);
            }
        }

        belyeg = Rec1->load(filename, h, demo);
        Rec2->load(filename, h, demo);

        if (demo) {
            qclose(h);
        } else {
            fclose(h);
        }
    } else {
        belyeg = Rec1->load(filename, NULL, demo);
    }

    return belyeg;
}

// belyeg-et adja vissza, vagy 0-at:
void save_replays(const char* filename, int level_id, int flagtag) {
    if (MultiplayerRec) {
        char tmpnev[40];
        sprintf(tmpnev, "rec/%s", filename);
        FILE* h = fopen(tmpnev, "wb");
        if (!h) {
            external_error("Could not open for writing record file!: !: ", tmpnev);
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

static int Begyujtve = 0;

#define GYUJTODARAB (200)

static WavEvent Azonosito[GYUJTODARAB];
static double Hangero[GYUJTODARAB];
static int Objszam[GYUJTODARAB];

void add_event_buffer(WavEvent event_id, double volume, int object_id) {
    if (Begyujtve < GYUJTODARAB) {
        Azonosito[Begyujtve] = event_id;
        Hangero[Begyujtve] = volume;
        Objszam[Begyujtve] = object_id;
        Begyujtve++;
    }
}

void reset_event_buffer(void) { Begyujtve = 0; }

// Igaz, ha van bent wav:
int get_event_buffer(WavEvent* event_id, double* volume, int* object_id) {
    if (Begyujtve == 0) {
        return 0;
    }
    *event_id = Azonosito[0];
    *volume = Hangero[0];
    *object_id = Objszam[0];
    Begyujtve--;
    for (int i = 0; i < Begyujtve; i++) {
        Azonosito[i] = Azonosito[i + 1];
        Hangero[i] = Hangero[i + 1];
        Objszam[i] = Objszam[i + 1];
    }
    return 1;
}
