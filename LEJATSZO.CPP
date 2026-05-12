#include "LEJATSZO.H"
#include "editor/editor.h"
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
#include "platform/sdl/keyboard.h"
#include "segments.h"
#include "timer.h"
#include <algorithm>
#include <cmath>
#include <cstring>
#include <directinput/scancodes.h>
#include <filesystem>

int Single = 1;
int FlagTag = 0;
bool OutOfBounds = false;

bool ScreenshotRequested = false;
bool VideoRecordingMode = false;
int VideoFrameIndex = 0;
std::string VideoOutputDirectory;

static int TotalApples;

// Returns whether console was active at the beginning of this frame,
// to prevent key presses from affecting the gameplay.
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
    bool game_minimap;
    bool replay_minimap;
    bool game_timer;
    bool replay_timer;
};

static void sound_init() {
    static bool SoundInitialized = false;
    if (State->sound_on && !SoundInitialized) {
        SoundInitialized = true;
        sound_engine_init();
    }
}

static hud_visibility HudVisibility1 = {true, false, true, false};
static hud_visibility HudVisibility2 = {true, false, true, false};

static BikeState handle_object_interaction(int object_id, int* apple_count, motorst* mot) {
    if (object_id < 0 || object_id >= MAX_OBJECTS) {
        internal_error("handle_object_interaction object_id < 0 || object_id >= MAX_OBJECTS!");
    }
    if (!Level->objects[object_id]) {
        internal_error("handle_object_interaction !Level->objects[object_id]!");
    }

    object::Type type = Level->objects[object_id]->type;

    if (type == object::Type::Killer) {
        return BikeState::Dead;
    }
    if (type == object::Type::Food) {
        Level->objects[object_id]->active = false;
        (*apple_count)++;
        add_event_buffer(WavEvent::Food, 0.99, -1);
        std::optional<MotorGravity> gravity = Level->objects[object_id]->gravity();
        if (gravity.has_value()) {
            mot->gravity_direction = gravity.value();
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

// Subframe physics calculation. Contains all the physics calculations except for bike turning
static void physics_subframe(motorst* mot, player_keys* keys, bike_metadata* metadata,
                             recorder* rec, int* finish_time, bool* dead, double time, double dt) {
    // Adjust key inputs to only allow valid inputs, accounting for volt delay and cripples
    bool is_gas_down = is_game_key_down(keys->gas);
    bool is_brake_down = is_game_key_down(keys->brake) || is_game_key_down(keys->brake_alias) ||
                         was_game_key_just_pressed(keys->one_frame_brake);
    bool is_right_volt_down = is_game_key_down(keys->right_volt);
    bool is_left_volt_down = is_game_key_down(keys->left_volt);

    if (EolSettings->cripple_drunk() && ((mot->apple_count - mot->apple_bug_count) & 1)) {
        std::swap(is_gas_down, is_brake_down);
        std::swap(is_right_volt_down, is_left_volt_down);
    }
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

    bool right_volt = false;
    bool left_volt = false;
    if (time > metadata->volt_time + VoltDelay && !EolSettings->cripple_no_volt()) {
        if (is_right_volt_down || is_game_key_down(keys->alovolt)) {
            right_volt = true;
            metadata->volt_time = time;
            metadata->volt_is_right = true;
            add_event_buffer(WavEvent::RightVolt, 0.99, -1);
        }
        if (is_left_volt_down || is_game_key_down(keys->alovolt)) {
            left_volt = true;
            metadata->volt_time = time;
            metadata->volt_is_right = false;
            add_event_buffer(WavEvent::LeftVolt, 0.99, -1);
        }
    }

    // Simulate bike physics given validated inputs
    simulate_bike_physics(mot, time, dt, is_gas_down, is_brake_down, right_volt, left_volt);

    // Check for head death and record object interactions
    BikeState head_state = check_object_collision(mot);
    if (head_state == BikeState::Dead) {
        metadata->sound.friction_volume = 0;
        metadata->sound.motor_frequency = -1;
        metadata->sound.gas = 0;
        rec->store_frames(mot, time, &metadata->sound);

        if (Single || !FlagTag || OutOfBounds) {
            *dead = true;
            return;
        }

        // Flag Tag respawn
        init_motor(mot);
        mot->bike.r = mot->bike.r + BikeStartOffset;
        mot->left_wheel.r = mot->left_wheel.r + BikeStartOffset;
        mot->right_wheel.r = mot->right_wheel.r + BikeStartOffset;
        mot->body_r = mot->body_r + BikeStartOffset;

        bool draw_view = metadata->draw_view;
        memset(metadata, 0, sizeof(bike_metadata));
        metadata->bike_turning.turn_time = -1000.0;
        metadata->camera_turning.turn_time = -1000.0;
        metadata->volt_time = -100.0;
        metadata->draw_view = draw_view;

        // Give flag to other player
        FlagTagAHasFlag = mot == Motor2;
    }

    // Update sound
    metadata->sound.friction_volume = get_bike_friction_volume();
    double wheel_vel =
        mot->flipped_bike ? mot->left_wheel.angular_velocity : mot->right_wheel.angular_velocity;
    wheel_vel = std::min(30.0, fabs(wheel_vel) * 0.025);
    metadata->sound.motor_frequency = 2.0 - exp(-wheel_vel);
    metadata->sound.gas = (char)is_gas_down;

    // Update replay
    rec->store_frames(mot, time, &metadata->sound);

    // Handle object ineractions
    WavEvent wav_id;
    double volume;
    int object_id;
    while (get_event_buffer(&wav_id, &volume, &object_id)) {
        if (object_id >= 0) {
            int prev_apple_count = mot->apple_count;
            BikeState bike_state = handle_object_interaction(object_id, &mot->apple_count, mot);
            if (bike_state == BikeState::Dead) {
                *dead = true;
            }
            if (bike_state == BikeState::Finish) {
                *finish_time = (int)(time * TimeToCentiseconds);
            }
            if (prev_apple_count < mot->apple_count) {
                mot->last_apple_time = (int)(time * TimeToCentiseconds);
            }
        } else {
            start_wav(wav_id, volume);
        }
        rec->store_event(time, wav_id, volume, object_id);
    }
}

static void update_view_settings(player_keys* keys, bike_metadata* metadata, bool* minimap,
                                 bool* timer, bool* other_draw_view) {
    // Visibility of player viewpoint
    if (was_game_key_just_pressed(keys->toggle_visibility)) {
        reset_game_background();
        if (!*other_draw_view) {
            // You cannot have 0 players visible, so make both players visible instead
            *other_draw_view = true;
            metadata->draw_view = true;
        } else {
            metadata->draw_view = !metadata->draw_view;
        }
    }

    if (was_game_key_just_pressed(keys->toggle_minimap)) {
        *minimap = !*minimap;
    }

    if (was_game_key_just_pressed(keys->toggle_timer)) {
        *timer = !*timer;
    }
}

// The `rec` argument is only used for game play, not when playing a replay.
static void update_bike_turn_phase(turning_data* data, recorder* rec, double time, int flipped) {
    if (data->flipped != flipped) {
        // New flip this frame
        data->flipped = flipped;
        data->turn_time = time;
        if (rec) {
            start_wav(WavEvent::Turn, 0.99);
            rec->store_event(time, WavEvent::Turn, 0.99, -1);
        }
    }

    double turn_time = EolSettings->turn_time();
    if (turn_time == 0.0) {
        // Instant turn
        data->turn_phase = 1.0;
    } else {
        data->turn_phase = (time - data->turn_time) / turn_time;
        data->turn_phase = std::clamp(data->turn_phase, 0.0, 1.0);
    }
}

static void update_camera_turn_phase(turning_data* data, double time, int flipped) {
    double camera_flip_time = EolSettings->turn_time() + 0.15;
    if (data->flipped != flipped) {
        // New flip this frame
        data->flipped = flipped;
        double time_since_prev_turn = time - data->turn_time;
        if (camera_flip_time > 0.0 && time_since_prev_turn < camera_flip_time) {
            // If camera is mid-turn, calculate camera start time so it seamlessly continues from
            // the mid-turn position
            data->turn_time = time + time_since_prev_turn - camera_flip_time;
        } else {
            // Camera is not mid-turn, so just set the camera turn time normally
            data->turn_time = time;
        }
    }

    double elapsed_time = std::max(0.0, time - data->turn_time);
    data->turn_phase = std::min(1.0, elapsed_time / camera_flip_time);
    if (flipped) {
        data->turn_phase = 1.0 - data->turn_phase;
    }
}

static void update_graphical_metadata(motorst& mot, bike_metadata& metadata, recorder* rec,
                                      double time) {
    // Update bike turn data
    update_bike_turn_phase(&metadata.bike_turning, rec, time, mot.flipped_bike);

    // Update camera position
    int flipped_camera = mot.flipped_bike;
    if (mot.gravity_direction == MotorGravity::Up) {
        flipped_camera = !flipped_camera;
    }
    update_camera_turn_phase(&metadata.camera_turning, time, flipped_camera);

    // Update arm position
    metadata.arm_position = std::max(0.0, 1.0 - (time - metadata.volt_time) / VoltDelay);
}

// The only physics-relevant part of this function is turning the bike. The rest is "cosmetic" for
// rendering the frame.
static void physics_frame_end(motorst* mot, player_keys* keys, bike_metadata* metadata,
                              recorder* rec, hud_visibility* hud, double time,
                              bool* other_draw_view, bool dead) {
    // Update the hud and player visibility
    update_view_settings(keys, metadata, &hud->game_minimap, &hud->game_timer, other_draw_view);

    // Check for bike turn and update turn graphics
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

    update_graphical_metadata(*mot, *metadata, rec, time);
}

static void handle_eol_inputs() {
    if (was_game_key_just_pressed(DIK_F1)) {
        EolSettings->set_show_others(!EolSettings->show_others());
        StatusMessages->add(EolSettings->show_others() ? "other players shown"
                                                       : "other players hidden");
    }

    if (was_game_key_just_pressed(DIK_F2)) {
        if (is_game_key_down(DIK_LSHIFT) || is_game_key_down(DIK_RSHIFT)) {
            EolClient->spy_prev_kuski();
        } else {
            EolClient->spy_next_kuski();
        }
    }

    if (was_game_key_just_pressed(DIK_F4)) {
        if (is_game_key_down(DIK_LCONTROL) || is_game_key_down(DIK_RCONTROL)) {
            Console->label_mode("[Download] ", "!download ", true, false);
            Console->toggle_active();
        } else {
            EolClient->download_battle_level();
        }
    }

    if (was_game_key_just_pressed(DIK_F5)) {
        EolClient->set_table(TableType::PlayersOnline);
    }

    if (was_game_key_just_pressed(DIK_F3)) {
        EolClient->toggle_battle_queue();
    }

    if (was_game_key_just_pressed(DIK_F6)) {
        EolClient->toggle_battle_results();
    }

    if (was_game_key_just_pressed(DIK_F10)) {
        if (is_game_key_down(DIK_LSHIFT) || is_game_key_down(DIK_RSHIFT)) {
            EolClient->toggle_show_battle_leader();
        } else {
            EolClient->toggle_battle_status();
        }
    }
}

// Common setup function
static void setup_gameloop(const char* filename) {
    load_best_time(filename, Single);

    init_physics_data();

    if (!Level) {
        internal_error("setup_gameloop() !Level!");
    }
    Level->flip_objects();
    Level->sort_objects();

    TotalApples = Level->initialize_objects(Motor1);
    Level->initialize_objects(Motor2);

    reset_game_background();

    flagtag_reset();

    Motor1->apple_count = 0;
    Motor2->apple_count = 0;
    Motor1->apple_bug_count = 0;
    Motor2->apple_bug_count = 0;

    OutOfBounds = false;

    Lgr->pal->set();
}

int WhoDiedFirst = 0;
bool Player1Finished = false;

namespace {
struct NumpadNavGuard {
    NumpadNavGuard() { keyboard::set_numpad_nav(false); }
    ~NumpadNavGuard() { keyboard::set_numpad_nav(true); }
};
} // namespace

int game_loop(const char* filename, CameraMode camera_mode) {
    // Bindings during gameplay must be honored by raw scancode: numpad-6
    // is right-volt, not "Right Arrow when NumLock is off".
    NumpadNavGuard numpad_nav_guard;

    WhoDiedFirst = 0;
    Player1Finished = false;

    Single = State->single;
    FlagTag = !Single && State->flag_tag;
    Rec1->set_flagtag(FlagTag);
    Rec2->set_flagtag(FlagTag);

    MultiplayerRec = Single ? 0 : 1;

    setup_gameloop(filename);

    double time = 0.0;
    reset_event_buffer();

    if (Single) {
        EolClient->enter_level(filename, Level);
    }

    stopwatch_reset();

    bike_metadata metadata1;
    memset(&metadata1, 0, sizeof(metadata1));
    bike_metadata metadata2;
    memset(&metadata2, 0, sizeof(metadata2));
    metadata1.bike_turning.turn_time = metadata2.bike_turning.turn_time = -1000.0;
    metadata1.camera_turning.turn_time = metadata2.camera_turning.turn_time = -1000.0;
    metadata1.volt_time = metadata2.volt_time = -100.0;
    metadata1.draw_view = metadata2.draw_view = true;

    bool dead1 = false;
    bool dead2 = false;
    int finish_time1 = 0;
    int finish_time2 = 0;
    reset_motor_forces(Motor1);
    reset_motor_forces(Motor2);

    camera current_camera;
    current_camera.mode = camera_mode;
    current_camera.x = Motor1->bike.r.x;
    current_camera.y = Motor1->bike.r.y;

    double level_min_y;
    double level_max_y;
    Level->get_boundaries(&current_camera.min_x, &level_min_y, &current_camera.max_x, &level_max_y,
                          false);
    // Convert level y-coordinates to camera y-coordinates
    current_camera.min_y = -level_max_y;
    current_camera.max_y = -level_min_y;

    sound_init();
    // Stay muted if no bike is visible.
    Mute = !(current_camera.mode != CameraMode::MapViewer);
    start_motor_sound(true);
    if (!Single) {
        start_motor_sound(false);
    }

    bool both_bikes_alive = true;
    while (true) {
        // Get timestep
        double target_time = stopwatch() * 0.0024;
        target_time = std::max(0.000001, target_time);

        handle_events();

        bool console_was_active = handle_console_input();

        while (time <= target_time - 0.000001) {
            // Cap slowest frame to 0.0055 (approximately 12.6 milliseconds or 79.4 fps)
            double dt = 0.0055;
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

            if (!Single && both_bikes_alive) {
                // If both die at same time, player 1 is considered to have died first
                if (dead1) {
                    stop_motor_sound(true);
                    WhoDiedFirst = 1;
                    both_bikes_alive = false;
                } else if (dead2) {
                    stop_motor_sound(false);
                    WhoDiedFirst = 2;
                    both_bikes_alive = false;
                }
            }

            if (finish_time1 || finish_time2 || (dead1 && (Single || dead2))) {
                int finish_time = finish_time1;
                if (finish_time2) {
                    finish_time = finish_time2;
                }

                if (finish_time) {
                    WhoDiedFirst = 0;
                }
                Player1Finished = (bool)finish_time1;

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
                                          TotalApples, !finish_time);
                }

                Level->unflip_objects();
                Rec1->encode_frame_count();
                Rec2->encode_frame_count();
                if (finish_time) {
                    return finish_time;
                }
                return -1;
            }
            time += dt;
        }

        if (!Single && FlagTag) {
            flagtag(time);
        }

        set_motor_frequency(true, metadata1.sound.motor_frequency, metadata1.sound.gas);
        if (Single) {
            set_friction_volume(metadata1.sound.friction_volume);
        } else {
            set_friction_volume(metadata1.sound.friction_volume + metadata2.sound.friction_volume);
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

        // Universal controls
        if (was_game_key_just_pressed(State->key_increase_screen_size)) {
            increase_view_size();
        }
        if (was_game_key_just_pressed(State->key_decrease_screen_size)) {
            decrease_view_size();
        }
        if (was_game_key_just_pressed(State->key_screenshot)) {
            ScreenshotRequested = true;
        }

        handle_eol_inputs();

        if (!console_was_active &&
            (was_key_just_pressed(DIK_ESCAPE) || was_key_just_pressed(State->key_escape_alias))) {
            WhoDiedFirst = 0;

            stop_motor_sound(true);
            stop_motor_sound(false);
            Mute = true;

            Level->unflip_objects();
            Rec1->encode_frame_count();
            Rec2->encode_frame_count();
            if (Single) {
                EolClient->exit_level(filename, time * TimeToCentiseconds, Motor1->apple_count,
                                      TotalApples, true);
            }
            return -1;
        }
    }
}

static void reverse_events(motorst* mot, recorder* rec, double time) {
    WavEvent wav;
    double vol;
    int obj_id;
    while (rec->recall_event_reverse(time, &wav, &vol, &obj_id)) {
        if (obj_id < 0) {
            continue;
        }
        object* obj = Level->objects[obj_id];
        if (obj && obj->type == object::Type::Food) {
            obj->active = true;
            mot->apple_count--;

            if (obj->gravity()) {
                mot->gravity_direction = rec->gravity(*Level);
            }
        }
    }
}

// During rewind, compute animation state from the recorder's event list
// instead of relying on the forward-only state machine.
static void rewind_override_animations(bike_metadata* metadata, motorst* mot, recorder* rec,
                                       double time) {
    double turn_time = rec->find_last_turn_frame_time(time);
    metadata->bike_turning.flipped = mot->flipped_bike;
    metadata->bike_turning.turn_time = turn_time;

    metadata->camera_turning.turn_time = -1000.0;
    int flipped_camera = mot->flipped_bike;
    if (mot->gravity_direction == MotorGravity::Up) {
        flipped_camera = !flipped_camera;
    }
    metadata->camera_turning.flipped = flipped_camera;

    metadata->volt_time = rec->find_last_volt_time(time, &metadata->volt_is_right);
}

// Load replay data (instead of simulating bike physics)
static bool replay_frame(motorst* mot, player_keys* keys, bike_metadata* metadata, recorder* rec,
                         double time, hud_visibility* hud, bool* other_draw_view) {
    // Update the hud and player visibility
    update_view_settings(keys, metadata, &hud->replay_minimap, &hud->replay_timer, other_draw_view);

    // Load replay data
    bool alive = rec->recall_frame(mot, time, &metadata->sound);
    set_head_position(mot);

    // Play events
    WavEvent wav_id;
    double volume;
    int object_id;
    while (rec->recall_event(time, &wav_id, &volume, &object_id)) {
        if (object_id >= 0) {
            int prev_apple_count = mot->apple_count;
            handle_object_interaction(object_id, &mot->apple_count, mot);
            if (prev_apple_count < mot->apple_count) {
                mot->last_apple_time = (int)(time * TimeToCentiseconds);
            }
        } else {
            start_wav(wav_id, volume);
            if (wav_id == WavEvent::RightVolt) {
                metadata->volt_is_right = true;
                metadata->volt_time = time;
            }
            if (wav_id == WavEvent::LeftVolt) {
                metadata->volt_is_right = false;
                metadata->volt_time = time;
            }
        }
    }
    return alive;
}

static bool PreviousReplayDrawView1 = true;
static bool PreviousReplayDrawView2 = true;

int replay_loop(const char* filename, int restore_player_visibility) {
    // Bindings during gameplay must be honored by raw scancode: numpad-6
    // is right-volt, not "Right Arrow when NumLock is off".
    NumpadNavGuard numpad_nav_guard;

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

    bike_metadata metadata1;
    memset(&metadata1, 0, sizeof(metadata1));
    bike_metadata metadata2;
    memset(&metadata2, 0, sizeof(metadata2));
    metadata1.bike_turning.turn_time = metadata2.bike_turning.turn_time = -1000.0;
    metadata1.camera_turning.turn_time = metadata2.camera_turning.turn_time = -1000.0;
    metadata1.volt_time = metadata2.volt_time = -100.0;

    metadata1.draw_view = true;
    metadata2.draw_view = !MergedRec;
    if (restore_player_visibility) {
        metadata1.draw_view = PreviousReplayDrawView1;
        metadata2.draw_view = PreviousReplayDrawView2;
    }

    reset_motor_forces(Motor1);
    reset_motor_forces(Motor2);

    camera current_camera;
    current_camera.mode = CameraMode::Normal;

    sound_init();
    Mute = false;
    start_motor_sound(true);
    if (!Single) {
        start_motor_sound(false);
    }

    bool alive1 = true;
    bool alive2 = true;

    double current_replay_time = 0.0;
    double last_stopwatch = stopwatch();

    while (true) {
        handle_events();

        bool console_was_active = handle_console_input();

        // Get timestep
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

        double time = current_replay_time;

        // Load replay data
        bool dead1 = !replay_frame(Motor1, &State->keys1, &metadata1, Rec1, time, &HudVisibility1,
                                   &metadata2.draw_view);
        bool dead2 = false;
        if (!Single) {
            dead2 = !replay_frame(Motor2, &State->keys2, &metadata2, Rec2, time, &HudVisibility2,
                                  &metadata1.draw_view);
        }

        // Reverse events if rewinding
        if (rewinding) {
            reverse_events(Motor1, Rec1, time);
            rewind_override_animations(&metadata1, Motor1, Rec1, time);
            if (!Single) {
                reverse_events(Motor2, Rec2, time);
                rewind_override_animations(&metadata2, Motor2, Rec2, time);
            }
        }

        update_graphical_metadata(*Motor1, metadata1, nullptr, time);
        if (!Single) {
            update_graphical_metadata(*Motor2, metadata2, nullptr, time);
        }

        // End of replay
        if ((Single && dead1) || (!Single && dead1 && dead2)) {
            set_motor_frequency(true, 1.0, 0);
            set_motor_frequency(false, 1.0, 0);
            stop_motor_sound(true);
            stop_motor_sound(false);
            set_friction_volume(0.0);
            delay((int)(LevelEndDelay * 400.0));

            Mute = true;
            Level->unflip_objects();

            PreviousReplayDrawView1 = metadata1.draw_view;
            PreviousReplayDrawView2 = metadata2.draw_view;
            Single = saved_single;
            FlagTag = saved_tag;
            return 0;
        }

        // Death (or finish)
        if (!Single) {
            if (alive1 && dead1) {
                stop_motor_sound(true);
                alive1 = false;
            }
            if (alive2 && dead2) {
                stop_motor_sound(false);
                alive2 = false;
            }

            // Update flagtag time
            flagtag_replay(time);
        }

        set_motor_frequency(true, metadata1.sound.motor_frequency, metadata1.sound.gas);
        if (Single) {
            set_friction_volume(metadata1.sound.friction_volume);
        } else {
            set_motor_frequency(false, metadata2.sound.motor_frequency, metadata2.sound.gas);
            set_friction_volume(metadata1.sound.friction_volume + metadata2.sound.friction_volume);
        }

        render_game(time, &metadata1, &metadata2, HudVisibility1.replay_minimap,
                    HudVisibility1.replay_timer, HudVisibility2.replay_minimap,
                    HudVisibility2.replay_timer, current_camera);

        // Universal controls
        if (was_game_key_just_pressed(State->key_increase_screen_size)) {
            increase_view_size();
        }
        if (was_game_key_just_pressed(State->key_decrease_screen_size)) {
            decrease_view_size();
        }
        if (was_game_key_just_pressed(State->key_screenshot)) {
            ScreenshotRequested = true;
        }

        handle_eol_inputs();

        if (!console_was_active && was_key_just_pressed(DIK_ESCAPE)) {
            set_motor_frequency(true, 1.0, 0);
            set_motor_frequency(false, 1.0, 0);
            stop_motor_sound(true);
            stop_motor_sound(false);
            set_friction_volume(0.0);
            delay((int)(LevelEndDelay * 400.0));

            Mute = true;

            Level->unflip_objects();

            PreviousReplayDrawView1 = metadata1.draw_view;
            PreviousReplayDrawView2 = metadata2.draw_view;
            Single = saved_single;
            FlagTag = saved_tag;
            return -1;
        }
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
    metadata1.bike_turning.turn_time = metadata2.bike_turning.turn_time = -1000.0;
    metadata1.camera_turning.turn_time = metadata2.camera_turning.turn_time = -1000.0;
    metadata1.volt_time = metadata2.volt_time = -100.0;
    metadata1.draw_view = true;
    metadata2.draw_view = true;

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

        bool dead1 = !replay_frame(Motor1, &State->keys1, &metadata1, Rec1, time, &HudVisibility1,
                                   &metadata2.draw_view);
        bool dead2 = false;
        if (!Single) {
            dead2 = !replay_frame(Motor2, &State->keys2, &metadata2, Rec2, time, &HudVisibility2,
                                  &metadata1.draw_view);
        }

        update_graphical_metadata(*Motor1, metadata1, nullptr, time);
        if (!Single) {
            update_graphical_metadata(*Motor2, metadata2, nullptr, time);
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
    Level->unflip_objects();
}
