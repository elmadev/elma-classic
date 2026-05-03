#include "LEJATSZO.H"
#include "EDITUJ.H"
#include "eol/console.h"
#include "eol/eol.h"
#include "eol/status_messages.h"
#include "eol_settings.h"
#include "flagtag.h"
#include "sound_engine.h"
#include "renderer/render.h"
#include "physics_forces.h"
#include "level.h"
#include "lgr.h"
#include "main.h"
#include "object.h"
#include "physics_init.h"
#include "platform/implementation.h"
#include "segments.h"
#include "timer.h"
#include <algorithm>
#include <cmath>
#include <cstring>
#include <directinput/scancodes.h>
#include <filesystem>

// Jatek modja ket fele lehet:
int Single = 1;
// Van-e fogocska:
int FlagTag = 0;
// int Multi = 0;
bool OutOfBounds = false;

int ScreenshotRequested = 0;
bool VideoRecordingMode = false;
int VideoFrameIndex = 0;
std::string VideoOutputDirectory;

static int TotalApples;

static bool handle_console_input() {
    bool was_active = Console->is_input_active();
    if (was_key_just_pressed(DIK_F9)) {
        Console->toggle_active();
    } else if (Console->is_input_active()) {
        Console->handle_input();
    }
    return was_active;
}

static bool is_game_key_down(DikScancode code) {
    if (Console->is_input_active()) {
        return false;
    }
    return is_key_down(code);
}

static bool was_game_key_just_pressed(DikScancode code) {
    if (Console->is_input_active()) {
        return false;
    }
    return was_key_just_pressed(code);
}

static void update_freecam(double dt, camera& current_camera) {
    double speed = 30.0;
    if (is_game_key_down(DIK_LSHIFT) || is_game_key_down(DIK_RSHIFT)) {
        speed *= 4.0;
    }
    double move = speed * dt;
    if (is_game_key_down(DIK_UP)) {
        current_camera.y += move;
    }
    if (is_game_key_down(DIK_DOWN)) {
        current_camera.y -= move;
    }
    if (is_game_key_down(DIK_LEFT)) {
        current_camera.x -= move;
    }
    if (is_game_key_down(DIK_RIGHT)) {
        current_camera.x += move;
    }

    current_camera.x = std::clamp(current_camera.x, current_camera.min_x, current_camera.max_x);
    current_camera.y = std::clamp(current_camera.y, current_camera.min_y, current_camera.max_y);
}

struct hud_visibility {
    int game_minimap, replay_minimap;
    int game_timer, replay_timer;
};

static void sound_init() {
    static int SoundInitialized = 0;
    if (State->sound_on && !SoundInitialized) {
        SoundInitialized = 1;
        sound_engine_init();
    }
}

// Player 1 es 2-re:
static hud_visibility HudVisibility1 = {1, 0, 1, 0};
static hud_visibility HudVisibility2 = {1, 0, 1, 0};

static BikeState handle_object_interaction(int object_id, int* apple_count, motorst* mot) {
    if (object_id < 0 || object_id >= MAX_OBJECTS) {
        internal_error("handle_object_interaction object_id < 0 || object_id >= MAX_OBJECTS!");
    }
    if (!Ptop->objects[object_id]) {
        internal_error("handle_object_interaction !Ptop->objects[object_id]!");
    }

    object::Type type = Ptop->objects[object_id]->type;

    if (type == object::Type::Killer) {
        return BikeState::Dead;
    }
    if (type == object::Type::Food) {
        Ptop->objects[object_id]->active = false;
        (*apple_count)++;
        add_event_buffer(WavEvent::Food, 0.99, -1);
        switch (Ptop->objects[object_id]->property) {
        case object::Property::None:
            break;
        case object::Property::GravityUp:
            mot->gravity_direction = MotorGravity::Up;
            break;
        case object::Property::GravityDown:
            mot->gravity_direction = MotorGravity::Down;
            break;
        case object::Property::GravityLeft:
            mot->gravity_direction = MotorGravity::Left;
            break;
        case object::Property::GravityRight:
            mot->gravity_direction = MotorGravity::Right;
            break;
        }
        return BikeState::Normal;
    }
    if (type == object::Type::Exit) {
        if (Motor1->apple_count + Motor2->apple_count >= TotalApples) {
            return BikeState::Finish;
        }
    }
    return BikeState::Normal;
}

static double calculate_camera_phase(double elapsed_time, int flipped) {
    elapsed_time = std::max<double>(elapsed_time, 0);

    double camera_flip_time = EolSettings->turn_time() + 0.15;
    if (flipped) {
        if (elapsed_time > camera_flip_time) {
            return 0.0;
        }
        return 1.0 - elapsed_time / camera_flip_time;
    } else {
        if (elapsed_time > camera_flip_time) {
            return 1.0;
        }
        return elapsed_time / camera_flip_time;
    }
}

// static int Marvoltgaz = 0;

// Ide csak *pmeghalt == 0-val erkezhet
static void physics_subframe(motorst* mot, player_keys* keys, bike_metadata* metadata,
                             recorder* rec, long* finish_time, int* dead, double time, double dt) {
    // Ugras elintezese:
    int right_volt = 0, left_volt = 0;

    bool is_gas_down = is_game_key_down(keys->gas);
    bool is_brake_down = is_game_key_down(keys->brake) || is_game_key_down(keys->brake_alias) ||
                         was_game_key_just_pressed(keys->one_frame_brake);
    bool is_right_volt_down = is_game_key_down(keys->right_volt);
    bool is_left_volt_down = is_game_key_down(keys->left_volt);

    if (EolSettings->cripple_drunk() && ((mot->apple_count - mot->apple_bug_count) & 1)) {
        std::swap(is_gas_down, is_brake_down);
        std::swap(is_right_volt_down, is_left_volt_down);
    }

    if (time > metadata->volt_time + VoltDelay && !EolSettings->cripple_no_volt()) {
        if (is_right_volt_down || is_game_key_down(keys->alovolt)) {
            right_volt = 1;
            metadata->volt_time = time;
            metadata->volt_is_right = 1;
            add_event_buffer(WavEvent::RightVolt, 0.99, -1);
        }
        if (is_left_volt_down || is_game_key_down(keys->alovolt)) {
            left_volt = 1;
            metadata->volt_time = time;
            metadata->volt_is_right = 0;
            add_event_buffer(WavEvent::LeftVolt, 0.99, -1);
        }
    }

    // LEPTET!!!!!!!:
    // 0-meghalt, 1-megnyerte, 2-semmi kulonos
    if (EolSettings->cripple_no_brake()) {
        is_brake_down = false;
    }
    if (EolSettings->cripple_no_throttle()) {
        is_gas_down = false;
    }
    if (EolSettings->cripple_always_throttle()) {
        is_gas_down = true;
        is_brake_down = false;
    }
    simulate_bike_physics(mot, time, dt, is_gas_down, is_brake_down, right_volt, left_volt);

    BikeState head_state = check_object_collision(mot);
    if (head_state == BikeState::Dead) {
        // Beteszunk egy 0 hangot:
        metadata->sound.friction_volume = 0;
        metadata->sound.motor_frequency = -1;
        metadata->sound.gas = 0;
        rec->store_frames(mot, time, &metadata->sound);
        if (!Single && FlagTag && !OutOfBounds) {
            // Flag Tag modban nem allnak meg, hanem visszamennek starthelyre:
            init_motor(mot);
            // Kezdetihelydiff-et setallaktiv allitja be:
            mot->bike.r = mot->bike.r + BikeStartOffset;
            mot->left_wheel.r = mot->left_wheel.r + BikeStartOffset;
            mot->right_wheel.r = mot->right_wheel.r + BikeStartOffset;
            mot->body_r = mot->body_r + BikeStartOffset;

            // Valtozok inicializalasa:
            int draw_view = metadata->draw_view;
            memset(metadata, 0, sizeof(bike_metadata));
            metadata->bike_turning.camera_turn_time = -1000.0;
            metadata->bike_turning.bike_turn_time = -1000.0;
            metadata->camera_turning.camera_turn_time = -1000.0;
            metadata->camera_turning.bike_turn_time = -1000.0;
            metadata->volt_time = -100.0;
            metadata->draw_view = draw_view;

            if ((mot == Motor1 && FlagTagAHasFlag) || (mot == Motor2 && !FlagTagAHasFlag)) {
                // Nala volt zaszlo
                FlagTagAHasFlag = !FlagTagAHasFlag;
            }
        } else {
            *dead = 1;
            return;
        }
    }

    // eddig += dt; // Ezt meg kell ismetelni eggyel feljebb is
    //  Berreges es nyikorgas:
    metadata->sound.friction_volume = get_bike_friction_volume();

    if (mot->flipped_bike) {
        metadata->sound.motor_frequency = fabs(mot->left_wheel.angular_velocity) * 0.025;
    } else {
        metadata->sound.motor_frequency = fabs(mot->right_wheel.angular_velocity) * 0.025;
    }
    // Frekvenciat betesszuk 1.0-2.0 tartomanyba:
    if (metadata->sound.motor_frequency > 30.0) {
        metadata->sound.motor_frequency = 30.0;
    }
    metadata->sound.motor_frequency = 2.0 - exp(-metadata->sound.motor_frequency);

    metadata->sound.gas = (char)is_gas_down;
    rec->store_frames(mot, time, &metadata->sound);

    // Egyedi hangok:
    WavEvent wav_id;
    double volume;
    int object_id;
    while (get_event_buffer(&wav_id, &volume, &object_id)) {
        if (object_id >= 0) {
            int prev_apple_count = mot->apple_count;
            BikeState bike_state = handle_object_interaction(object_id, &mot->apple_count, mot);
            if (bike_state == BikeState::Dead || bike_state == BikeState::Finish) {
                if (bike_state == BikeState::Dead) {
                    *dead = 1;
                } else {
                    *finish_time = time * TimeToCentiseconds;
                }
            }

            if (prev_apple_count < mot->apple_count) {
                mot->last_apple_time = time * TimeToCentiseconds;
            }
        } else {
            start_wav(wav_id, volume);
        }
        rec->store_event(time, wav_id, volume, object_id);
    }
}

static void update_view_settings(player_keys* keys, bike_metadata* metadata, int* minimap,
                                 int* timer, int* other_draw_view) {
    // Showkep:
    if (was_game_key_just_pressed(keys->toggle_visibility)) {
        reset_game_background();
        if (!*other_draw_view) {
            // Ha masik jatekos ablaka nem latszott, mindegyik fog latszani:
            *other_draw_view = 1;
            metadata->draw_view = 1;
        } else {
            metadata->draw_view = !metadata->draw_view;
        }
    }
    // View ablak:
    if (was_game_key_just_pressed(keys->toggle_minimap)) {
        *minimap = !*minimap;
    }

    // Ido kint legyen vagy ne:
    if (was_game_key_just_pressed(keys->toggle_timer)) {
        *timer = !*timer;
    }
}

static void update_turning_data(turning_data* data, recorder* rec, double time, int flipped) {
    if (data->flipped != flipped) {
        data->bike_turn_time = time;
        double elapsed_time = time - data->camera_turn_time;
        double camera_flip_time = EolSettings->turn_time() + 0.15;
        if (camera_flip_time > 0.0 && elapsed_time < camera_flip_time) {
            data->camera_turn_time = time + elapsed_time - camera_flip_time;
        } else {
            data->camera_turn_time = time;
        }
        data->flipped = flipped;
        // Hang adas:
        if (rec) {
            // Most kihagyjuk wavgyujtot:
            start_wav(WavEvent::Turn, 0.99);
            rec->store_event(time, WavEvent::Turn, 0.99, -1);
        }
    }

    // Kirajzolashoz kellenek:
    double turn_time = EolSettings->turn_time();
    if (turn_time == 0.0) {
        // Instant turn
        data->bike_turn_phase = 1.0;
    } else {
        data->bike_turn_phase = (time - data->bike_turn_time) / turn_time;

        data->bike_turn_phase = std::clamp(data->bike_turn_phase, 0.0, 1.0);
    }

    data->camera_turn_phase = calculate_camera_phase(time - data->camera_turn_time, flipped);
}

static void physics_frame_end(motorst* mot, player_keys* keys, bike_metadata* metadata,
                              recorder* rec, hud_visibility* hud, double time, int* other_draw_view,
                              int dead) {
    update_view_settings(keys, metadata, &hud->game_minimap, &hud->game_timer, other_draw_view);

    // Hatra fordulas elintezes:
    if (!dead) {
        bool is_turn_down = is_game_key_down(keys->turn);
        if (EolSettings->cripple_no_turn()) {
            is_turn_down = false;
        }
        if (EolSettings->cripple_one_turn() && metadata->one_turn_used) {
            is_turn_down = false;
        }
        if (!metadata->turn_key_previous && is_turn_down) {
            mot->flipped_bike = !mot->flipped_bike;
            set_head_position(mot);
            metadata->one_turn_used = true;
        }
        metadata->turn_key_previous = is_turn_down;
    }
    update_turning_data(&metadata->bike_turning, rec, time, mot->flipped_bike);

    // Hatra elintezes:
    if (mot->gravity_direction == MotorGravity::Up) {
        mot->flipped_camera = !mot->flipped_bike;
    } else {
        mot->flipped_camera = mot->flipped_bike;
    }
    update_turning_data(&metadata->camera_turning, nullptr, time, mot->flipped_camera);

    // Kirajzolashoz kell:
    metadata->arm_position = 1.0 - (time - metadata->volt_time) / VoltDelay;
    if (metadata->arm_position < 0) {
        metadata->arm_position = 0;
    }
#ifdef DEBUG
    if (metadata->arm_position > 1.00001) {
        internal_error("metadata->arm_position > 1.00001!");
    }
#endif
}

// Common setup function
static void setup_gameloop(const char* filename) {
    load_best_time(filename, Single);

    init_physics_data();
    if (!Ptop) {
        internal_error("setup_gameloop() !Ptop!");
    }
    Ptop->flip_objects();
    Ptop->sort_objects();
    // setallaktiv allitja be motor kezdeti helyzetet is!:
    TotalApples = Ptop->initialize_objects(Motor1);
    Ptop->initialize_objects(Motor2);

    // Eloszor keretet tobbszor is kirakja kirajzol320:
    reset_game_background();

    flagtag_reset();

    Motor1->apple_count = Motor2->apple_count = 0;
    Motor1->apple_bug_count = Motor2->apple_bug_count = 0;

    OutOfBounds = false;

    Lgr->pal->set();
}

// lejatszo ezt beallitja: 0 -> nem kell kiirni, 1 -> A eloszor, 2 -> B
// csak akkor A vagy B eloszor, ha mindketten meghaltak, tehat
// menu_level menu-nek ezt mar feldolgozta elore:
int WhoDiedFirst = 0;

// Ha sikeres szint, ide teszi be hogy ki ment ki:
int Player1Finished = 0;

int Masodikmenet = 0;

// Idot adja vissza szazadmasodpercben:
long game_loop(const char* filename, CameraMode camera_mode) {
    // internal_error( "Ezegyhosszusor,szetkellvagnibiztosanhibaEzegy hosszu sor, szet kell vagni
    // biztosan!",
    //	"Ez egy masik hosszu sor, szet kell vagni biztosan!" );

    // Azert allitom 1-be, hogy mindenkeppen eszrevegyem, ha nem allitom
    // meg kilepes elott be rendesen:
    WhoDiedFirst = 1;
    Player1Finished = 0;

    // Jatekmod eldontese:
    Single = State->single;
    FlagTag = !Single && State->flag_tag;
    Rec1->set_flagtag(FlagTag);
    Rec2->set_flagtag(FlagTag);

    if (Single) {
        MultiplayerRec = 0;
    } else {
        MultiplayerRec = 1;
    }

    setup_gameloop(filename);

    double time = 0.0;
    reset_event_buffer();

    if (Single) {
        EolClient->enter_level(filename, Ptop);
    }

    stopwatch_reset();

    // Valtozok inicializalasa:
    bike_metadata metadata1; // Inicializalni kell !!!!!!!!!!!!
    memset(&metadata1, 0, sizeof(metadata1));
    bike_metadata metadata2;
    memset(&metadata2, 0, sizeof(metadata2));
    metadata1.bike_turning.camera_turn_time = metadata2.bike_turning.camera_turn_time = -1000.0;
    metadata1.bike_turning.bike_turn_time = metadata2.bike_turning.bike_turn_time = -1000.0;
    metadata1.camera_turning.camera_turn_time = metadata2.camera_turning.camera_turn_time = -1000.0;
    metadata1.camera_turning.bike_turn_time = metadata2.camera_turning.bike_turn_time = -1000.0;
    metadata1.volt_time = metadata2.volt_time = -100.0;
    metadata1.draw_view = metadata2.draw_view = 1;

    int dead1 = 0;
    int dead2 = 0;
    long finish_time1 = 0;
    long finish_time2 = 0;
    reset_motor_forces(Motor1);
    reset_motor_forces(Motor2);

    camera current_camera;
    current_camera.mode = camera_mode;
    current_camera.x = Motor1->bike.r.x;
    current_camera.y = Motor1->bike.r.y;

    double level_min_y;
    double level_max_y;
    Ptop->get_boundaries(&current_camera.min_x, &level_min_y, &current_camera.max_x, &level_max_y,
                         false);
    // convert level y-coordinates to camera y-coordinates
    current_camera.min_y = -level_max_y;
    current_camera.max_y = -level_min_y;

    sound_init();
    // Stay muted if no bike is visible.
    Mute = !(current_camera.mode != CameraMode::MapViewer);
    if (Single) {
        start_motor_sound(true);
    } else {
        start_motor_sound(true);
        start_motor_sound(false);
    }

    long l = 0;
    int both_bikes_alive = 1;
    while (1) {
        // Kiszamolja mennyit kell leptetni:
        double target_time = stopwatch() * 0.0024;
        if (target_time < 0.000001) {
            target_time = 0.000001;
        }

        // Ha tul sok kimaradt, akkor nem kell annyit behozni:
        // Ezt most elvileg stopper vegzi:
        // if( eddig < cel-1.0 )
        //	eddig = cel-1.0;

        /* Most nincs idoellenorzes:
        if( cel - eddig > 2.0*0.432 && idoszerint ) // Maximum 2 sec a turelem!
            external_error( "System not fast enough!" );
        */

        handle_events(); // Billentyut itt olvassuk be

        bool console_was_active = handle_console_input();

        while (time <= target_time - 0.000001) {
            // char tmp[10];
            // sprintf( tmp, "Gaz kodja: %d", (int)State->keys1.gas );
            // external_error( tmp );
            // double dt = 0.003; // Ez it 1.2-es verzionak megfelelo
            // double dt = 0.006; // Ez itt uj lepeskoz (jol mukodik regen)

            // Ez ELASTO MANIA 1.0 lepeskoze:
            // double dt = 0.0055; // Ez leeseskor sem rezonal

            double dt = 0.0055; // 0.0065

            if (time + dt > target_time) {
                dt = target_time - time;
            }

            if (current_camera.mode == CameraMode::MapViewer) {
                update_freecam(dt, current_camera);
                time += dt;
                continue;
            }

            if (!dead1) {
                physics_subframe(Motor1, &State->keys1, &metadata1, Rec1, &finish_time1, &dead1,
                                 time, dt);
            }

            if (!Single && !dead2) {
                physics_subframe(Motor2, &State->keys2, &metadata2, Rec2, &finish_time2, &dead2,
                                 time, dt);
            }

            if (!(dead1 && dead2)) {
                // Ha meg nem halt meg mindketto:
                if (dead1) {
                    WhoDiedFirst = 1;
                }
                if (dead2) {
                    WhoDiedFirst = 2;
                }
            }

            if (!Single && both_bikes_alive) {
                if (dead1 || dead2) {
                    // Ha meg mindket motor jar,
                    // de mar egyik meghalt:
                    if (dead1) {
                        stop_motor_sound(true);
                    } else {
                        stop_motor_sound(false);
                    }
                    both_bikes_alive = 0;
                }
            }

            if ((Single && (finish_time1 || dead1)) ||
                (!Single && (finish_time1 || finish_time2 || (dead1 && dead2)))) {
                // Vege jateknak:
                long finish_time = finish_time1;
                if (finish_time2) {
                    finish_time = finish_time2;
                }

                if (!(dead1 && dead2) || Single) {
                    // Ha nem halt meg mindketto, akkor ne irjon semmit:
                    WhoDiedFirst = 0;
                }
                if (finish_time1) {
                    Player1Finished = 1;
                } else {
                    Player1Finished = 0;
                }

                set_motor_frequency(true, 1.0, 0);
                set_motor_frequency(false, 1.0, 0);
                if (finish_time) {
                    start_wav(WavEvent::Win, 0.999);
                } else {
                    start_wav(WavEvent::Dead, 0.999);
                }
                delay((int)(LevelEndDelay * 1000.0));

                Console->deactivate_input();

                stop_motor_sound(true);
                stop_motor_sound(false);

                Mute = true;
                if (Single) {
                    EolClient->exit_level(filename, time * TimeToCentiseconds, Motor1->apple_count,
                                          TotalApples, finish_time ? false : true);
                }
                if (finish_time) {
                    Ptop->unflip_objects();
                    Rec1->encode_frame_count();
                    Rec2->encode_frame_count();
                    return finish_time;
                } else {
                    Ptop->unflip_objects();
                    Rec1->encode_frame_count();
                    Rec2->encode_frame_count();
                    return -1;
                }
            }

            time += dt;
        }

        // Fogocska elintezes:
        if (!Single && FlagTag) {
            flagtag(time);
        }

        if (Single) {
            set_friction_volume(metadata1.sound.friction_volume);
            set_motor_frequency(true, metadata1.sound.motor_frequency, metadata1.sound.gas);
        } else {
            set_friction_volume(metadata1.sound.friction_volume + metadata2.sound.friction_volume);
            set_motor_frequency(true, metadata1.sound.motor_frequency, metadata1.sound.gas);
            set_motor_frequency(false, metadata2.sound.motor_frequency, metadata2.sound.gas);
        }

        physics_frame_end(Motor1, &State->keys1, &metadata1, Rec1, &HudVisibility1, time,
                          &metadata2.draw_view, dead1);

        if (!Single) {
            physics_frame_end(Motor2, &State->keys2, &metadata2, Rec2, &HudVisibility2, time,
                              &metadata1.draw_view, dead2);
        }

        render_game(time, &metadata1, &metadata2, HudVisibility1.game_minimap,
                    HudVisibility1.game_timer, HudVisibility2.game_minimap,
                    HudVisibility2.game_timer, current_camera);

        // Egy par kozos toggle:
        // Plusz elintezes:
        if (was_game_key_just_pressed(State->key_increase_screen_size)) {
            increase_view_size();
        }
        // Minusz elintezes:
        if (was_game_key_just_pressed(State->key_decrease_screen_size)) {
            decrease_view_size();
        }
        // Snap nyomas elintezese:
        if (was_game_key_just_pressed(State->key_screenshot)) {
            ScreenshotRequested = 1;
        }

        if (was_game_key_just_pressed(DIK_F1)) {
            EolSettings->set_show_others(!EolSettings->show_others());
            StatusMessages->add(EolSettings->show_others() ? "other players shown"
                                                           : "other players hidden");
        }

        if (was_game_key_just_pressed(DIK_F5)) {
            EolClient->set_table(TableType::PlayersOnline);
        }

        // Vizsgalat kilepesre:
        if (!console_was_active &&
            (was_key_just_pressed(DIK_ESCAPE) || was_key_just_pressed(State->key_escape_alias))) {
            WhoDiedFirst = 0;
            double eltelido = stopwatch();
            // Motor hangjat kikapcsolja:
            stop_motor_sound(true);
            stop_motor_sound(false);

            Mute = true;

            // Kiirja framerate-et:
            double tpl = double(l) / eltelido;
            tpl *= STOPWATCH_MULTIPLIER * 1000.0;
            int framerate = (int)(tpl);
            if (std::filesystem::exists("f_rate.inf")) {
                // igyhagyni fopen-t!:
                FILE* h = fopen("f_rate.inf", "w");
                if (!h) {
                    internal_error("framerate nem nyilik!");
                }
                fprintf(h, "%d", framerate);
                fclose(h);
            }
            Ptop->unflip_objects();
            Rec1->encode_frame_count();
            Rec2->encode_frame_count();
            Masodikmenet = 1;
            if (Single) {
                EolClient->exit_level(filename, time * TimeToCentiseconds, Motor1->apple_count,
                                      TotalApples, true);
            }
            return -1;
        }
        l++;
        /*vect2 tomba[100], tombb[100];
        double dtomb[100];
        for( int i = 0; i < 100; i++ ) {
            tomba[i] = vect2( i+2, i+3 );
            tombb[i] = vect2( i-23, i-70 );
        }
        for( i = 0; i < 200000; i++ ) {
            for( int j = 0; j < 100; j++ )
                dtomb[j] = tomba[j] % tombb[j];
        } */
    }
}

// REPLAY REPLAY REPLAY REPLAY REPLAY REPLAY REPLAY REPLAY REPLAY REPLAY
// REPLAY REPLAY REPLAY REPLAY REPLAY REPLAY REPLAY REPLAY REPLAY REPLAY
// REPLAY REPLAY REPLAY REPLAY REPLAY REPLAY REPLAY REPLAY REPLAY REPLAY
// REPLAY REPLAY REPLAY REPLAY REPLAY REPLAY REPLAY REPLAY REPLAY REPLAY
// REPLAY REPLAY REPLAY REPLAY REPLAY REPLAY REPLAY REPLAY REPLAY REPLAY
// REPLAY REPLAY REPLAY REPLAY REPLAY REPLAY REPLAY REPLAY REPLAY REPLAY
// REPLAY REPLAY REPLAY REPLAY REPLAY REPLAY REPLAY REPLAY REPLAY REPLAY
// REPLAY REPLAY REPLAY REPLAY REPLAY REPLAY REPLAY REPLAY REPLAY REPLAY
// REPLAY REPLAY REPLAY REPLAY REPLAY REPLAY REPLAY REPLAY REPLAY REPLAY
// REPLAY REPLAY REPLAY REPLAY REPLAY REPLAY REPLAY REPLAY REPLAY REPLAY
// REPLAY REPLAY REPLAY REPLAY REPLAY REPLAY REPLAY REPLAY REPLAY REPLAY
// REPLAY REPLAY REPLAY REPLAY REPLAY REPLAY REPLAY REPLAY REPLAY REPLAY
// REPLAY REPLAY REPLAY REPLAY REPLAY REPLAY REPLAY REPLAY REPLAY REPLAY
// REPLAY REPLAY REPLAY REPLAY REPLAY REPLAY REPLAY REPLAY REPLAY REPLAY
// REPLAY REPLAY REPLAY REPLAY REPLAY REPLAY REPLAY REPLAY REPLAY REPLAY
// REPLAY REPLAY REPLAY REPLAY REPLAY REPLAY REPLAY REPLAY REPLAY REPLAY
// REPLAY REPLAY REPLAY REPLAY REPLAY REPLAY REPLAY REPLAY REPLAY REPLAY
// REPLAY REPLAY REPLAY REPLAY REPLAY REPLAY REPLAY REPLAY REPLAY REPLAY
// REPLAY REPLAY REPLAY REPLAY REPLAY REPLAY REPLAY REPLAY REPLAY REPLAY
// REPLAY REPLAY REPLAY REPLAY REPLAY REPLAY REPLAY REPLAY REPLAY REPLAY
// REPLAY REPLAY REPLAY REPLAY REPLAY REPLAY REPLAY REPLAY REPLAY REPLAY
// REPLAY REPLAY REPLAY REPLAY REPLAY REPLAY REPLAY REPLAY REPLAY REPLAY
// REPLAY REPLAY REPLAY REPLAY REPLAY REPLAY REPLAY REPLAY REPLAY REPLAY
// REPLAY REPLAY REPLAY REPLAY REPLAY REPLAY REPLAY REPLAY REPLAY REPLAY
// REPLAY REPLAY REPLAY REPLAY REPLAY REPLAY REPLAY REPLAY REPLAY REPLAY
// REPLAY REPLAY REPLAY REPLAY REPLAY REPLAY REPLAY REPLAY REPLAY REPLAY
// REPLAY REPLAY REPLAY REPLAY REPLAY REPLAY REPLAY REPLAY REPLAY REPLAY
// REPLAY REPLAY REPLAY REPLAY REPLAY REPLAY REPLAY REPLAY REPLAY REPLAY
// REPLAY REPLAY REPLAY REPLAY REPLAY REPLAY REPLAY REPLAY REPLAY REPLAY
// REPLAY REPLAY REPLAY REPLAY REPLAY REPLAY REPLAY REPLAY REPLAY REPLAY
// REPLAY REPLAY REPLAY REPLAY REPLAY REPLAY REPLAY REPLAY REPLAY REPLAY
// REPLAY REPLAY REPLAY REPLAY REPLAY REPLAY REPLAY REPLAY REPLAY REPLAY
// REPLAY REPLAY REPLAY REPLAY REPLAY REPLAY REPLAY REPLAY REPLAY REPLAY
// REPLAY REPLAY REPLAY REPLAY REPLAY REPLAY REPLAY REPLAY REPLAY REPLAY
// REPLAY REPLAY REPLAY REPLAY REPLAY REPLAY REPLAY REPLAY REPLAY REPLAY
// REPLAY REPLAY REPLAY REPLAY REPLAY REPLAY REPLAY REPLAY REPLAY REPLAY
// REPLAY REPLAY REPLAY REPLAY REPLAY REPLAY REPLAY REPLAY REPLAY REPLAY
// REPLAY REPLAY REPLAY REPLAY REPLAY REPLAY REPLAY REPLAY REPLAY REPLAY
// REPLAY REPLAY REPLAY REPLAY REPLAY REPLAY REPLAY REPLAY REPLAY REPLAY
// REPLAY REPLAY REPLAY REPLAY REPLAY REPLAY REPLAY REPLAY REPLAY REPLAY
// REPLAY REPLAY REPLAY REPLAY REPLAY REPLAY REPLAY REPLAY REPLAY REPLAY
// REPLAY REPLAY REPLAY REPLAY REPLAY REPLAY REPLAY REPLAY REPLAY REPLAY
// REPLAY REPLAY REPLAY REPLAY REPLAY REPLAY REPLAY REPLAY REPLAY REPLAY
// REPLAY REPLAY REPLAY REPLAY REPLAY REPLAY REPLAY REPLAY REPLAY REPLAY
// REPLAY REPLAY REPLAY REPLAY REPLAY REPLAY REPLAY REPLAY REPLAY REPLAY
// REPLAY REPLAY REPLAY REPLAY REPLAY REPLAY REPLAY REPLAY REPLAY REPLAY
// REPLAY REPLAY REPLAY REPLAY REPLAY REPLAY REPLAY REPLAY REPLAY REPLAY
// REPLAY REPLAY REPLAY REPLAY REPLAY REPLAY REPLAY REPLAY REPLAY REPLAY

// char Ccc[] = "Cs1_2";

static void reverse_events(motorst* mot, recorder* rec, double time) {
    WavEvent wav;
    double vol;
    int obj;
    while (rec->recall_event_reverse(time, &wav, &vol, &obj)) {
        if (obj >= 0 && Ptop->objects[obj] && Ptop->objects[obj]->type == object::Type::Food) {
            Ptop->objects[obj]->active = true;
            mot->apple_count--;
        }
    }
}

// During rewind, compute animation state from the recorder's event list
// instead of relying on the forward-only state machine.
static void rewind_override_animations(bike_metadata* metadata, motorst* mot, recorder* rec,
                                       double time) {
    double turn_time = rec->find_last_turn_frame_time(time);

    metadata->bike_turning.flipped = mot->flipped_bike;
    metadata->bike_turning.bike_turn_time = turn_time;
    metadata->bike_turning.camera_turn_time = turn_time;
    update_turning_data(&metadata->bike_turning, nullptr, time, mot->flipped_bike);

    metadata->camera_turning.flipped = mot->flipped_camera;
    metadata->camera_turning.bike_turn_time = time - 1000.0;
    metadata->camera_turning.camera_turn_time = time - 1000.0;
    metadata->camera_turning.bike_turn_phase = 1.0;
    metadata->camera_turning.camera_turn_phase = mot->flipped_camera ? 0.0 : 1.0;

    bool is_right_volt = true;
    double volt_time = rec->find_last_volt_time(time, &is_right_volt);
    metadata->volt_time = volt_time;
    metadata->volt_is_right = is_right_volt ? 1 : 0;
    metadata->arm_position = 1.0 - (time - metadata->volt_time) / VoltDelay;
    metadata->arm_position = std::max<double>(metadata->arm_position, 0);
}

// Hamissal ter vissza, ha nincsen mar tobb:
static bool replay_frame(motorst* mot, player_keys* keys, bike_metadata* metadata, recorder* rec,
                         double time, hud_visibility* hud, int* other_draw_view) {

    update_view_settings(keys, metadata, &hud->replay_minimap, &hud->replay_timer, other_draw_view);

    // recall koordokat es Hatra-t is beallitja:
    bool alive = rec->recall_frame(mot, time, &metadata->sound);
    set_head_position(mot); // mivel nincs leptet ami kiszamolna
    // HANG ELINTEZES:
    WavEvent wav_id;
    double volume;
    int object_id;
    while (rec->recall_event(time, &wav_id, &volume, &object_id)) {
        if (object_id >= 0) {
            int prev_apple_count = mot->apple_count;
            handle_object_interaction(object_id, &mot->apple_count, mot);
            if (prev_apple_count < mot->apple_count) {
                mot->last_apple_time = time * TimeToCentiseconds;
            }
        } else {
            start_wav(wav_id, volume);
            if (wav_id == WavEvent::RightVolt) {
                metadata->volt_is_right = 1;
                metadata->volt_time = time;
            }
            if (wav_id == WavEvent::LeftVolt) {
                metadata->volt_is_right = 0;
                metadata->volt_time = time;
            }
        }
    }

    // Hatra elintezes:
    update_turning_data(&metadata->bike_turning, nullptr, time, mot->flipped_bike);

    if (mot->gravity_direction == MotorGravity::Up) {
        mot->flipped_camera = !mot->flipped_bike;
    } else {
        mot->flipped_camera = mot->flipped_bike;
    }
    update_turning_data(&metadata->camera_turning, nullptr, time, mot->flipped_camera);

    // Kirajzolashoz kellenek:
    metadata->arm_position = 1.0 - (time - metadata->volt_time) / VoltDelay;
    metadata->arm_position = std::max<double>(metadata->arm_position, 0);

    return alive;
}

static int PreviousReplayDrawView1 = 1, PreviousReplayDrawView2 = 1;

// Termeszetes befejezes eseten 0-t ad vissza:
long replay_loop(const char* filename, int restore_player_visibility) {
    // Refuse to play zero-length replays (from map-viewer mode)
    if (Rec1->is_empty()) {
        return -2;
    }
    if (MultiplayerRec && Rec2->is_empty()) {
        return -2;
    }

    int saved_single = Single;
    int saved_tag = FlagTag;

    Single = !MultiplayerRec;
    FlagTag = Rec1->flagtag();
    setup_gameloop(filename);

    reset_event_buffer();
    stopwatch_reset();

    // Valtozok inicializalasa:
    bike_metadata metadata1; // Inicializalni kell !!!!!!!!!!!!
    memset(&metadata1, 0, sizeof(metadata1));
    bike_metadata metadata2;
    memset(&metadata2, 0, sizeof(metadata2));
    metadata1.bike_turning.camera_turn_time = metadata2.bike_turning.camera_turn_time = -1000.0;
    metadata1.bike_turning.bike_turn_time = metadata2.bike_turning.bike_turn_time = -1000.0;
    metadata1.camera_turning.camera_turn_time = metadata2.camera_turning.camera_turn_time = -1000.0;
    metadata1.camera_turning.bike_turn_time = metadata2.camera_turning.bike_turn_time = -1000.0;
    metadata1.volt_time = metadata2.volt_time = -100.0;
    if (restore_player_visibility) {
        metadata1.draw_view = PreviousReplayDrawView1;
        metadata2.draw_view = PreviousReplayDrawView2;
    } else {
        metadata1.draw_view = 1;
        metadata2.draw_view = 1;
    }

    reset_motor_forces(Motor1);
    reset_motor_forces(Motor2);

    camera current_camera;
    current_camera.mode = CameraMode::Normal;

    sound_init();
    Mute = false;
    if (Single) {
        start_motor_sound(true);
    } else {
        start_motor_sound(true);
        start_motor_sound(false);
    }

    long l = 0;
    int alive1 = 1, alive2 = 1;

    double current_replay_time = 0.0;
    double last_stopwatch = stopwatch();

    while (1) {
        handle_events(); // Billentyut itt olvassuk be

        bool console_was_active = handle_console_input();

        double now = stopwatch();
        double dt = (now - last_stopwatch) * 0.0024;
        last_stopwatch = now;

        double speed = 1.0;

        if (is_game_key_down(State->key_replay_fast_2x)) {
            speed *= 2.0;
        }
        if (is_game_key_down(State->key_replay_fast_4x)) {
            speed *= 4.0;
        }
        if (is_game_key_down(State->key_replay_fast_8x)) {
            speed *= 8.0;
        }

        if (is_game_key_down(State->key_replay_slow_2x)) {
            speed *= 0.5;
        }
        if (is_game_key_down(State->key_replay_slow_4x)) {
            speed *= 0.25;
        }

        bool rewinding = is_game_key_down(State->key_replay_rewind);

        bool paused = is_game_key_down(State->key_replay_pause);

        if (!paused) {
            if (rewinding) {
                current_replay_time -= dt * speed;
                current_replay_time = std::max(current_replay_time, 0.0);
            } else {
                current_replay_time += dt * speed;
            }
        }

        // Kiszamolja mennyit kell leptetni:
        double time = current_replay_time;

        // recall koordokat es Hatra-t is beallitja:
        int dead1 = !replay_frame(Motor1, &State->keys1, &metadata1, Rec1, time, &HudVisibility1,
                                  &metadata2.draw_view);

        int dead2 = 0;
        if (!Single) {
            dead2 = !replay_frame(Motor2, &State->keys2, &metadata2, Rec2, time, &HudVisibility2,
                                  &metadata1.draw_view);
        }

        if (rewinding) {
            reverse_events(Motor1, Rec1, time);
            rewind_override_animations(&metadata1, Motor1, Rec1, time);
            if (!Single) {
                reverse_events(Motor2, Rec2, time);
                rewind_override_animations(&metadata2, Motor2, Rec2, time);
            }
        }

        if ((Single && dead1) || (!Single && dead1 && dead2)) {
            // Motor hangjat kikapcsolja:
            set_motor_frequency(true, 1.0, 0);
            set_motor_frequency(false, 1.0, 0);
            stop_motor_sound(true);
            stop_motor_sound(false);
            set_friction_volume(0);
            delay((int)(LevelEndDelay * 400.0));

            Mute = true;
            Ptop->unflip_objects();

            PreviousReplayDrawView1 = metadata1.draw_view;
            PreviousReplayDrawView2 = metadata2.draw_view;
            Single = saved_single;
            FlagTag = saved_tag;
            return 0;
        }

        if (!Single) {
            if (alive1 && dead1) {
                stop_motor_sound(true);
                alive1 = 0;
            }
            if (alive2 && dead2) {
                stop_motor_sound(false);
                alive2 = 0;
            }

            // Fogocska:
            flagtag_replay(time);
        }

        // HANG ELINTEZES:
        if (Single) {
            set_motor_frequency(true, metadata1.sound.motor_frequency, metadata1.sound.gas);
            // if( valt1.sound.motor_frequency > 1.5 )
            //	internal_error( "> 1.5!" );
            set_friction_volume(metadata1.sound.friction_volume);
        } else {
            set_motor_frequency(true, metadata1.sound.motor_frequency, metadata1.sound.gas);
            set_motor_frequency(false, metadata2.sound.motor_frequency, metadata2.sound.gas);
            set_friction_volume(metadata1.sound.friction_volume + metadata2.sound.friction_volume);
        }

        render_game(time, &metadata1, &metadata2, HudVisibility1.replay_minimap,
                    HudVisibility1.replay_timer, HudVisibility2.replay_minimap,
                    HudVisibility2.replay_timer, current_camera);

        // Egy par kozos toggle:
        // Plusz elintezes:
        if (was_game_key_just_pressed(State->key_increase_screen_size)) {
            increase_view_size();

            // extern int Mostirdki;
            // Mostirdki = 1;
        }
        // Minusz elintezes:
        if (was_game_key_just_pressed(State->key_decrease_screen_size)) {
            decrease_view_size();
        }
        // Snap nyomas elintezese:
        if (was_game_key_just_pressed(State->key_screenshot)) {
            ScreenshotRequested = 1;
        }

        // Vizsgalat kilepesre:
        if (!console_was_active && was_key_just_pressed(DIK_ESCAPE)) {
            double eltelido = stopwatch();
            // Kiirja framerate-et:
            double tpl = double(l) / eltelido;
            tpl *= STOPWATCH_MULTIPLIER * 1000.0;
            int framerate = (int)(tpl);
            if (std::filesystem::exists("f_rate.inf")) {
                // igyhagyni fopen-t!:
                FILE* h = fopen("f_rate.inf", "w");
                if (!h) {
                    internal_error("framerate nem nyilik!");
                }
                fprintf(h, "%d", framerate);
                fclose(h);
            }
            // Hang kikapcsolas:
            set_motor_frequency(true, 1.0, 0);
            set_motor_frequency(false, 1.0, 0);
            stop_motor_sound(true);
            stop_motor_sound(false);
            set_friction_volume(0);
            delay((int)(LevelEndDelay * 400.0));

            Mute = true;

            Ptop->unflip_objects();

            PreviousReplayDrawView1 = metadata1.draw_view;
            PreviousReplayDrawView2 = metadata2.draw_view;
            Single = saved_single;
            FlagTag = saved_tag;
            return -1;
        }
        l++;
    }
}

// Sets the path of VideoOutputDirectory to "./renders/{replay_stem}/"
void setup_render_directory(const std::string& replay_filename) {
    std::filesystem::path p(replay_filename);
    std::string stem = p.stem().string();
    std::filesystem::path out_dir = std::filesystem::path("renders") / stem;
    std::error_code ec;
    std::filesystem::create_directories(out_dir, ec);
    if (ec) {
        internal_error("Failed to create output directory for replay render:\n" + ec.message());
    }

    VideoOutputDirectory = out_dir.string();
}

void render_replay(const char* level_filename) {
    Single = !MultiplayerRec;
    FlagTag = Rec1->flagtag();
    setup_gameloop(level_filename);

    bike_metadata metadata1;
    memset(&metadata1, 0, sizeof(metadata1));
    bike_metadata metadata2;
    memset(&metadata2, 0, sizeof(metadata2));

    metadata1.bike_turning.camera_turn_time = metadata2.bike_turning.camera_turn_time = -1000.0;
    metadata1.bike_turning.bike_turn_time = metadata2.bike_turning.bike_turn_time = -1000.0;
    metadata1.camera_turning.camera_turn_time = metadata2.camera_turning.camera_turn_time = -1000.0;
    metadata1.camera_turning.bike_turn_time = metadata2.camera_turning.bike_turn_time = -1000.0;
    metadata1.volt_time = metadata2.volt_time = -100.0;
    metadata1.draw_view = 1;
    metadata2.draw_view = 1;

    camera current_camera;
    current_camera.mode = CameraMode::Normal;

    VideoRecordingMode = true;
    VideoFrameIndex = 0;

    while (true) {
        handle_events();
        if (is_game_key_down(DIK_ESCAPE)) {
            break;
        }

        double time = (double)VideoFrameIndex * (STOPWATCH_MULTIPLIER * 1000.0 * 0.0024) /
                      EolSettings->recording_fps();

        int dead1 = !replay_frame(Motor1, &State->keys1, &metadata1, Rec1, time, &HudVisibility1,
                                  &metadata2.draw_view);
        int dead2 = 0;
        if (!Single) {
            dead2 = !replay_frame(Motor2, &State->keys2, &metadata2, Rec2, time, &HudVisibility2,
                                  &metadata1.draw_view);
        }

        if ((Single && dead1) || (!Single && dead1 && dead2)) {
            break;
        }

        if (!Single) {
            flagtag_replay(time);
        }

        render_game(time, &metadata1, &metadata2, HudVisibility1.replay_minimap,
                    HudVisibility1.replay_timer, HudVisibility2.replay_minimap,
                    HudVisibility2.replay_timer, current_camera);

        VideoFrameIndex++;
    }

    VideoRecordingMode = false;
    Ptop->unflip_objects();
}
