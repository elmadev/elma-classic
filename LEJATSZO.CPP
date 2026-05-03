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

static int Kajakell;

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

struct viewtimest {
    int viewkinplay, viewkinreplay;
    int timekinplay, timekinreplay;
};

static void initsoundlejatszoban() {
    static int Hangmarmegy = 0;
    if (State->sound_on && !Hangmarmegy) {
        Hangmarmegy = 1;
        sound_engine_init();
    }
}

// Player 1 es 2-re:
static viewtimest Viewtime1 = {1, 0, 1, 0};
static viewtimest Viewtime2 = {1, 0, 1, 0};

static BikeState spritefeldolgoz(int sorszam, int* pkajaszam, motorst* pmot) {
    if (sorszam < 0 || sorszam >= MAX_OBJECTS) {
        internal_error("spritefeldolgoz-ban sorszam < 0 || sorszam >= MAXKEREK!");
    }
    if (!Ptop->objects[sorszam]) {
        internal_error("spritefeldolgoz-ban !Ptop->objects[sorszam]!");
    }

    object::Type tipus = Ptop->objects[sorszam]->type;

    if (tipus == object::Type::Killer) {
        return BikeState::Dead;
    }
    if (tipus == object::Type::Food) {
        Ptop->objects[sorszam]->active = false;
        (*pkajaszam)++;
        add_event_buffer(WavEvent::Food, 0.99, -1);
        switch (Ptop->objects[sorszam]->property) {
        case object::Property::None:
            break;
        case object::Property::GravityUp:
            pmot->gravity_direction = MotorGravity::Up;
            break;
        case object::Property::GravityDown:
            pmot->gravity_direction = MotorGravity::Down;
            break;
        case object::Property::GravityLeft:
            pmot->gravity_direction = MotorGravity::Left;
            break;
        case object::Property::GravityRight:
            pmot->gravity_direction = MotorGravity::Right;
            break;
        }
        return BikeState::Normal;
    }
    if (tipus == object::Type::Exit) {
        if (Motor1->apple_count + Motor2->apple_count >= Kajakell) {
            return BikeState::Finish;
        }
    }
    return BikeState::Normal;
}

static double baljobbszamol(double eltelt, int hatra) {
    eltelt = std::max<double>(eltelt, 0);

    double camera_flip_time = EolSettings->turn_time() + 0.15;
    if (hatra) {
        if (eltelt > camera_flip_time) {
            return 0.0;
        }
        return 1.0 - eltelt / camera_flip_time;
    } else {
        if (eltelt > camera_flip_time) {
            return 1.0;
        }
        return eltelt / camera_flip_time;
    }
}

// static int Marvoltgaz = 0;

// Ide csak *pmeghalt == 0-val erkezhet
static void belsoresz(motorst* pmot, player_keys* popciok, bike_metadata* pvalt, recorder* prec,
                      long* pmegvanido, int* pmeghalt, double eddig, double dt) {
    // Ugras elintezese:
    int ugrik1 = 0, ugrik2 = 0;

    bool is_gas_down = is_game_key_down(popciok->gas);
    bool is_brake_down = is_game_key_down(popciok->brake) ||
                         is_game_key_down(popciok->brake_alias) ||
                         was_game_key_just_pressed(popciok->one_frame_brake);
    bool is_right_volt_down = is_game_key_down(popciok->right_volt);
    bool is_left_volt_down = is_game_key_down(popciok->left_volt);

    if (EolSettings->cripple_drunk() && ((pmot->apple_count - pmot->apple_bug_count) & 1)) {
        std::swap(is_gas_down, is_brake_down);
        std::swap(is_right_volt_down, is_left_volt_down);
    }

    if (eddig > pvalt->volt_time + VoltDelay && !EolSettings->cripple_no_volt()) {
        if (is_right_volt_down || is_game_key_down(popciok->alovolt)) {
            ugrik1 = 1;
            pvalt->volt_time = eddig;
            pvalt->volt_is_right = 1;
            add_event_buffer(WavEvent::RightVolt, 0.99, -1);
        }
        if (is_left_volt_down || is_game_key_down(popciok->alovolt)) {
            ugrik2 = 1;
            pvalt->volt_time = eddig;
            pvalt->volt_is_right = 0;
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
    simulate_bike_physics(pmot, eddig, dt, is_gas_down, is_brake_down, ugrik1, ugrik2);

    BikeState eredmeny = check_object_collision(pmot);
    if (eredmeny == BikeState::Dead) {
        // Beteszunk egy 0 hangot:
        pvalt->sound.friction_volume = 0;
        pvalt->sound.motor_frequency = -1;
        pvalt->sound.gas = 0;
        prec->store_frames(pmot, eddig, &pvalt->sound);
        if (!Single && FlagTag && !OutOfBounds) {
            // Flag Tag modban nem allnak meg, hanem visszamennek starthelyre:
            init_motor(pmot);
            // Kezdetihelydiff-et setallaktiv allitja be:
            pmot->bike.r = pmot->bike.r + BikeStartOffset;
            pmot->left_wheel.r = pmot->left_wheel.r + BikeStartOffset;
            pmot->right_wheel.r = pmot->right_wheel.r + BikeStartOffset;
            pmot->body_r = pmot->body_r + BikeStartOffset;

            // Valtozok inicializalasa:
            int showkep = pvalt->draw_view;
            memset(pvalt, 0, sizeof(bike_metadata));
            pvalt->bike_turning.camera_turn_time = -1000.0;
            pvalt->bike_turning.bike_turn_time = -1000.0;
            pvalt->camera_turning.camera_turn_time = -1000.0;
            pvalt->camera_turning.bike_turn_time = -1000.0;
            pvalt->volt_time = -100.0;
            pvalt->draw_view = showkep;

            if ((pmot == Motor1 && FlagTagAHasFlag) || (pmot == Motor2 && !FlagTagAHasFlag)) {
                // Nala volt zaszlo
                FlagTagAHasFlag = !FlagTagAHasFlag;
            }
        } else {
            *pmeghalt = 1;
            return;
        }
    }

    // eddig += dt; // Ezt meg kell ismetelni eggyel feljebb is
    //  Berreges es nyikorgas:
    pvalt->sound.friction_volume = get_bike_friction_volume();

    if (pmot->flipped_bike) {
        pvalt->sound.motor_frequency = fabs(pmot->left_wheel.angular_velocity) * 0.025;
    } else {
        pvalt->sound.motor_frequency = fabs(pmot->right_wheel.angular_velocity) * 0.025;
    }
    // Frekvenciat betesszuk 1.0-2.0 tartomanyba:
    if (pvalt->sound.motor_frequency > 30.0) {
        pvalt->sound.motor_frequency = 30.0;
    }
    pvalt->sound.motor_frequency = 2.0 - exp(-pvalt->sound.motor_frequency);

    pvalt->sound.gas = (char)is_gas_down;
    prec->store_frames(pmot, eddig, &pvalt->sound);

    // Egyedi hangok:
    WavEvent wavazonosito;
    double hangero;
    int objszam;
    while (get_event_buffer(&wavazonosito, &hangero, &objszam)) {
        if (objszam >= 0) {
            int prev_apple_count = pmot->apple_count;
            BikeState eredmeny = spritefeldolgoz(objszam, &pmot->apple_count, pmot);
            if (eredmeny == BikeState::Dead || eredmeny == BikeState::Finish) {
                if (eredmeny == BikeState::Dead) {
                    *pmeghalt = 1;
                } else {
                    *pmegvanido = eddig * TimeToCentiseconds;
                }
            }

            if (prev_apple_count < pmot->apple_count) {
                pmot->last_apple_time = eddig * TimeToCentiseconds;
            }
        } else {
            start_wav(wavazonosito, hangero);
        }
        prec->store_event(eddig, wavazonosito, hangero, objszam);
    }
}

static void toggleresz(player_keys* popciok, bike_metadata* pvalt, int* pviewkin, int* ptimekin,
                       int* pmasikshow) {
    // Showkep:
    if (was_game_key_just_pressed(popciok->toggle_visibility)) {
        reset_game_background();
        if (!*pmasikshow) {
            // Ha masik jatekos ablaka nem latszott, mindegyik fog latszani:
            *pmasikshow = 1;
            pvalt->draw_view = 1;
        } else {
            pvalt->draw_view = !pvalt->draw_view;
        }
    }
    // View ablak:
    if (was_game_key_just_pressed(popciok->toggle_minimap)) {
        *pviewkin = !*pviewkin;
    }

    // Ido kint legyen vagy ne:
    if (was_game_key_just_pressed(popciok->toggle_timer)) {
        *ptimekin = !*ptimekin;
    }
}

static void baljobbelintez(turning_data* pvalt, recorder* prec, double eddig, int hatra) {
    if (pvalt->flipped != hatra) {
        pvalt->bike_turn_time = eddig;
        double eltelt = eddig - pvalt->camera_turn_time;
        double camera_flip_time = EolSettings->turn_time() + 0.15;
        if (camera_flip_time > 0.0 && eltelt < camera_flip_time) {
            pvalt->camera_turn_time = eddig + eltelt - camera_flip_time;
        } else {
            pvalt->camera_turn_time = eddig;
        }
        pvalt->flipped = hatra;
        // Hang adas:
        if (prec) {
            // Most kihagyjuk wavgyujtot:
            start_wav(WavEvent::Turn, 0.99);
            prec->store_event(eddig, WavEvent::Turn, 0.99, -1);
        }
    }

    // Kirajzolashoz kellenek:
    double turn_time = EolSettings->turn_time();
    if (turn_time == 0.0) {
        // Instant turn
        pvalt->bike_turn_phase = 1.0;
    } else {
        pvalt->bike_turn_phase = (eddig - pvalt->bike_turn_time) / turn_time;

        pvalt->bike_turn_phase = std::clamp(pvalt->bike_turn_phase, 0.0, 1.0);
    }

    pvalt->camera_turn_phase = baljobbszamol(eddig - pvalt->camera_turn_time, hatra);
}

static void kulsoresz(motorst* pmot, player_keys* popciok, bike_metadata* pvalt, recorder* prec,
                      viewtimest* pvt, double eddig, int* pmasikshow, int meghalt) {
    toggleresz(popciok, pvalt, &pvt->viewkinplay, &pvt->timekinplay, pmasikshow);

    // Hatra fordulas elintezes:
    if (!meghalt) {
        bool is_turn_down = is_game_key_down(popciok->turn);
        if (EolSettings->cripple_no_turn()) {
            is_turn_down = false;
        }
        if (EolSettings->cripple_one_turn() && pvalt->one_turn_used) {
            is_turn_down = false;
        }
        if (!pvalt->turn_key_previous && is_turn_down) {
            pmot->flipped_bike = !pmot->flipped_bike;
            set_head_position(pmot);
            pvalt->one_turn_used = true;
        }
        pvalt->turn_key_previous = is_turn_down;
    }
    baljobbelintez(&pvalt->bike_turning, prec, eddig, pmot->flipped_bike);

    // Hatra elintezes:
    if (pmot->gravity_direction == MotorGravity::Up) {
        pmot->flipped_camera = !pmot->flipped_bike;
    } else {
        pmot->flipped_camera = pmot->flipped_bike;
    }
    baljobbelintez(&pvalt->camera_turning, nullptr, eddig, pmot->flipped_camera);

    // Kirajzolashoz kell:
    pvalt->arm_position = 1.0 - (eddig - pvalt->volt_time) / VoltDelay;
    if (pvalt->arm_position < 0) {
        pvalt->arm_position = 0;
    }
#ifdef DEBUG
    if (pvalt->arm_position > 1.00001) {
        internal_error("pvalt->arm_position > 1.00001!");
    }
#endif
}

// Common setup function
static void setup_gameloop(const char* filenev) {
    load_best_time(filenev, Single);

    init_physics_data();
    if (!Ptop) {
        internal_error("setup_gameloop() !Ptop!");
    }
    Ptop->flip_objects();
    Ptop->sort_objects();
    // setallaktiv allitja be motor kezdeti helyzetet is!:
    Kajakell = Ptop->initialize_objects(Motor1);
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

    double eddig = 0.0;
    reset_event_buffer();

    if (Single) {
        EolClient->enter_level(filename, Ptop);
    }

    stopwatch_reset();

    // Valtozok inicializalasa:
    bike_metadata valt1; // Inicializalni kell !!!!!!!!!!!!
    memset(&valt1, 0, sizeof(valt1));
    bike_metadata valt2;
    memset(&valt2, 0, sizeof(valt2));
    valt1.bike_turning.camera_turn_time = valt2.bike_turning.camera_turn_time = -1000.0;
    valt1.bike_turning.bike_turn_time = valt2.bike_turning.bike_turn_time = -1000.0;
    valt1.camera_turning.camera_turn_time = valt2.camera_turning.camera_turn_time = -1000.0;
    valt1.camera_turning.bike_turn_time = valt2.camera_turning.bike_turn_time = -1000.0;
    valt1.volt_time = valt2.volt_time = -100.0;
    valt1.draw_view = valt2.draw_view = 1;

    int meghalt1 = 0;
    int meghalt2 = 0;
    long megvanido1 = 0;
    long megvanido2 = 0;
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

    initsoundlejatszoban();
    // Stay muted if no bike is visible.
    Mute = !(current_camera.mode != CameraMode::MapViewer);
    if (Single) {
        start_motor_sound(true);
    } else {
        start_motor_sound(true);
        start_motor_sound(false);
    }

    long l = 0;
    int mindjar = 1;
    while (1) {
        // Kiszamolja mennyit kell leptetni:
        double cel = stopwatch() * 0.0024;
        if (cel < 0.000001) {
            cel = 0.000001;
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

        while (eddig <= cel - 0.000001) {
            // char tmp[10];
            // sprintf( tmp, "Gaz kodja: %d", (int)State->keys1.gas );
            // external_error( tmp );
            // double dt = 0.003; // Ez it 1.2-es verzionak megfelelo
            // double dt = 0.006; // Ez itt uj lepeskoz (jol mukodik regen)

            // Ez ELASTO MANIA 1.0 lepeskoze:
            // double dt = 0.0055; // Ez leeseskor sem rezonal

            double dt = 0.0055; // 0.0065

            if (eddig + dt > cel) {
                dt = cel - eddig;
            }

            if (current_camera.mode == CameraMode::MapViewer) {
                update_freecam(dt, current_camera);
                eddig += dt;
                continue;
            }

            if (!meghalt1) {
                belsoresz(Motor1, &State->keys1, &valt1, Rec1, &megvanido1, &meghalt1, eddig, dt);
            }

            if (!Single && !meghalt2) {
                belsoresz(Motor2, &State->keys2, &valt2, Rec2, &megvanido2, &meghalt2, eddig, dt);
            }

            if (!(meghalt1 && meghalt2)) {
                // Ha meg nem halt meg mindketto:
                if (meghalt1) {
                    WhoDiedFirst = 1;
                }
                if (meghalt2) {
                    WhoDiedFirst = 2;
                }
            }

            if (!Single && mindjar) {
                if (meghalt1 || meghalt2) {
                    // Ha meg mindket motor jar,
                    // de mar egyik meghalt:
                    if (meghalt1) {
                        stop_motor_sound(true);
                    } else {
                        stop_motor_sound(false);
                    }
                    mindjar = 0;
                }
            }

            if ((Single && (megvanido1 || meghalt1)) ||
                (!Single && (megvanido1 || megvanido2 || (meghalt1 && meghalt2)))) {
                // Vege jateknak:
                long megvanido = megvanido1;
                if (megvanido2) {
                    megvanido = megvanido2;
                }

                if (!(meghalt1 && meghalt2) || Single) {
                    // Ha nem halt meg mindketto, akkor ne irjon semmit:
                    WhoDiedFirst = 0;
                }
                if (megvanido1) {
                    Player1Finished = 1;
                } else {
                    Player1Finished = 0;
                }

                set_motor_frequency(true, 1.0, 0);
                set_motor_frequency(false, 1.0, 0);
                if (megvanido) {
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
                    EolClient->exit_level(filename, eddig * TimeToCentiseconds, Motor1->apple_count,
                                          Kajakell, megvanido ? false : true);
                }
                if (megvanido) {
                    Ptop->unflip_objects();
                    Rec1->encode_frame_count();
                    Rec2->encode_frame_count();
                    return megvanido;
                } else {
                    Ptop->unflip_objects();
                    Rec1->encode_frame_count();
                    Rec2->encode_frame_count();
                    return -1;
                }
            }

            eddig += dt;
        }

        // Fogocska elintezes:
        if (!Single && FlagTag) {
            flagtag(eddig);
        }

        if (Single) {
            set_friction_volume(valt1.sound.friction_volume);
            set_motor_frequency(true, valt1.sound.motor_frequency, valt1.sound.gas);
        } else {
            set_friction_volume(valt1.sound.friction_volume + valt2.sound.friction_volume);
            set_motor_frequency(true, valt1.sound.motor_frequency, valt1.sound.gas);
            set_motor_frequency(false, valt2.sound.motor_frequency, valt2.sound.gas);
        }

        kulsoresz(Motor1, &State->keys1, &valt1, Rec1, &Viewtime1, eddig, &valt2.draw_view,
                  meghalt1);

        if (!Single) {
            kulsoresz(Motor2, &State->keys2, &valt2, Rec2, &Viewtime2, eddig, &valt1.draw_view,
                      meghalt2);
        }

        render_game(eddig, &valt1, &valt2, Viewtime1.viewkinplay, Viewtime1.timekinplay,
                    Viewtime2.viewkinplay, Viewtime2.timekinplay, current_camera);

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
                EolClient->exit_level(filename, eddig * TimeToCentiseconds, Motor1->apple_count,
                                      Kajakell, true);
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

static void reverse_events(motorst* pmot, recorder* prec, double time) {
    WavEvent wav;
    double vol;
    int obj;
    while (prec->recall_event_reverse(time, &wav, &vol, &obj)) {
        if (obj >= 0 && Ptop->objects[obj] && Ptop->objects[obj]->type == object::Type::Food) {
            Ptop->objects[obj]->active = true;
            pmot->apple_count--;
        }
    }
}

// During rewind, compute animation state from the recorder's event list
// instead of relying on the forward-only state machine.
static void rewind_override_animations(bike_metadata* pvalt, motorst* pmot, recorder* prec,
                                       double eddig) {
    double turn_time = prec->find_last_turn_frame_time(eddig);

    pvalt->bike_turning.flipped = pmot->flipped_bike;
    pvalt->bike_turning.bike_turn_time = turn_time;
    pvalt->bike_turning.camera_turn_time = turn_time;
    baljobbelintez(&pvalt->bike_turning, nullptr, eddig, pmot->flipped_bike);

    pvalt->camera_turning.flipped = pmot->flipped_camera;
    pvalt->camera_turning.bike_turn_time = eddig - 1000.0;
    pvalt->camera_turning.camera_turn_time = eddig - 1000.0;
    pvalt->camera_turning.bike_turn_phase = 1.0;
    pvalt->camera_turning.camera_turn_phase = pmot->flipped_camera ? 0.0 : 1.0;

    bool is_right_volt = true;
    double volt_time = prec->find_last_volt_time(eddig, &is_right_volt);
    pvalt->volt_time = volt_time;
    pvalt->volt_is_right = is_right_volt ? 1 : 0;
    pvalt->arm_position = 1.0 - (eddig - pvalt->volt_time) / VoltDelay;
    pvalt->arm_position = std::max<double>(pvalt->arm_position, 0);
}

// Hamissal ter vissza, ha nincsen mar tobb:
static bool replaymag(motorst* pmot, player_keys* popciok, bike_metadata* pvalt, recorder* prec,
                      double eddig, viewtimest* pvt, int* pmasikshow) {

    toggleresz(popciok, pvalt, &pvt->viewkinreplay, &pvt->timekinreplay, pmasikshow);

    // recall koordokat es Hatra-t is beallitja:
    bool visszaad = prec->recall_frame(pmot, eddig, &pvalt->sound);
    set_head_position(pmot); // mivel nincs leptet ami kiszamolna
    // HANG ELINTEZES:
    WavEvent wavindex;
    double hangero;
    int objszam;
    while (prec->recall_event(eddig, &wavindex, &hangero, &objszam)) {
        if (objszam >= 0) {
            int prev_apple_count = pmot->apple_count;
            spritefeldolgoz(objszam, &pmot->apple_count, pmot);
            if (prev_apple_count < pmot->apple_count) {
                pmot->last_apple_time = eddig * TimeToCentiseconds;
            }
        } else {
            start_wav(wavindex, hangero);
            if (wavindex == WavEvent::RightVolt) {
                pvalt->volt_is_right = 1;
                pvalt->volt_time = eddig;
            }
            if (wavindex == WavEvent::LeftVolt) {
                pvalt->volt_is_right = 0;
                pvalt->volt_time = eddig;
            }
        }
    }

    // Hatra elintezes:
    baljobbelintez(&pvalt->bike_turning, nullptr, eddig, pmot->flipped_bike);

    if (pmot->gravity_direction == MotorGravity::Up) {
        pmot->flipped_camera = !pmot->flipped_bike;
    } else {
        pmot->flipped_camera = pmot->flipped_bike;
    }
    baljobbelintez(&pvalt->camera_turning, nullptr, eddig, pmot->flipped_camera);

    // Kirajzolashoz kellenek:
    pvalt->arm_position = 1.0 - (eddig - pvalt->volt_time) / VoltDelay;
    pvalt->arm_position = std::max<double>(pvalt->arm_position, 0);

    return visszaad;
}

static int Elozoshowkep1 = 1, Elozoshowkep2 = 1;

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
    bike_metadata valt1; // Inicializalni kell !!!!!!!!!!!!
    memset(&valt1, 0, sizeof(valt1));
    bike_metadata valt2;
    memset(&valt2, 0, sizeof(valt2));
    valt1.bike_turning.camera_turn_time = valt2.bike_turning.camera_turn_time = -1000.0;
    valt1.bike_turning.bike_turn_time = valt2.bike_turning.bike_turn_time = -1000.0;
    valt1.camera_turning.camera_turn_time = valt2.camera_turning.camera_turn_time = -1000.0;
    valt1.camera_turning.bike_turn_time = valt2.camera_turning.bike_turn_time = -1000.0;
    valt1.volt_time = valt2.volt_time = -100.0;
    if (restore_player_visibility) {
        valt1.draw_view = Elozoshowkep1;
        valt2.draw_view = Elozoshowkep2;
    } else {
        valt1.draw_view = 1;
        valt2.draw_view = 1;
    }

    reset_motor_forces(Motor1);
    reset_motor_forces(Motor2);

    camera current_camera;
    current_camera.mode = CameraMode::Normal;

    initsoundlejatszoban();
    Mute = false;
    if (Single) {
        start_motor_sound(true);
    } else {
        start_motor_sound(true);
        start_motor_sound(false);
    }

    long l = 0;
    int jar1 = 1, jar2 = 1;

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
        double eddig = current_replay_time;

        // recall koordokat es Hatra-t is beallitja:
        int befejezte1 =
            !replaymag(Motor1, &State->keys1, &valt1, Rec1, eddig, &Viewtime1, &valt2.draw_view);

        int befejezte2 = 0;
        if (!Single) {
            befejezte2 = !replaymag(Motor2, &State->keys2, &valt2, Rec2, eddig, &Viewtime2,
                                    &valt1.draw_view);
        }

        if (rewinding) {
            reverse_events(Motor1, Rec1, eddig);
            rewind_override_animations(&valt1, Motor1, Rec1, eddig);
            if (!Single) {
                reverse_events(Motor2, Rec2, eddig);
                rewind_override_animations(&valt2, Motor2, Rec2, eddig);
            }
        }

        if ((Single && befejezte1) || (!Single && befejezte1 && befejezte2)) {
            // Motor hangjat kikapcsolja:
            set_motor_frequency(true, 1.0, 0);
            set_motor_frequency(false, 1.0, 0);
            stop_motor_sound(true);
            stop_motor_sound(false);
            set_friction_volume(0);
            delay((int)(LevelEndDelay * 400.0));

            Mute = true;
            Ptop->unflip_objects();

            Elozoshowkep1 = valt1.draw_view;
            Elozoshowkep2 = valt2.draw_view;
            Single = saved_single;
            FlagTag = saved_tag;
            return 0;
        }

        if (!Single) {
            if (jar1 && befejezte1) {
                stop_motor_sound(true);
                jar1 = 0;
            }
            if (jar2 && befejezte2) {
                stop_motor_sound(false);
                jar2 = 0;
            }

            // Fogocska:
            flagtag_replay(eddig);
        }

        // HANG ELINTEZES:
        if (Single) {
            set_motor_frequency(true, valt1.sound.motor_frequency, valt1.sound.gas);
            // if( valt1.sound.motor_frequency > 1.5 )
            //	internal_error( "> 1.5!" );
            set_friction_volume(valt1.sound.friction_volume);
        } else {
            set_motor_frequency(true, valt1.sound.motor_frequency, valt1.sound.gas);
            set_motor_frequency(false, valt2.sound.motor_frequency, valt2.sound.gas);
            set_friction_volume(valt1.sound.friction_volume + valt2.sound.friction_volume);
        }

        render_game(eddig, &valt1, &valt2, Viewtime1.viewkinreplay, Viewtime1.timekinreplay,
                    Viewtime2.viewkinreplay, Viewtime2.timekinreplay, current_camera);

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

            Elozoshowkep1 = valt1.draw_view;
            Elozoshowkep2 = valt2.draw_view;
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

    bike_metadata valt1;
    memset(&valt1, 0, sizeof(valt1));
    bike_metadata valt2;
    memset(&valt2, 0, sizeof(valt2));

    valt1.bike_turning.camera_turn_time = valt2.bike_turning.camera_turn_time = -1000.0;
    valt1.bike_turning.bike_turn_time = valt2.bike_turning.bike_turn_time = -1000.0;
    valt1.camera_turning.camera_turn_time = valt2.camera_turning.camera_turn_time = -1000.0;
    valt1.camera_turning.bike_turn_time = valt2.camera_turning.bike_turn_time = -1000.0;
    valt1.volt_time = valt2.volt_time = -100.0;
    valt1.draw_view = 1;
    valt2.draw_view = 1;

    camera current_camera;
    current_camera.mode = CameraMode::Normal;

    VideoRecordingMode = true;
    VideoFrameIndex = 0;

    while (true) {
        handle_events();
        if (is_game_key_down(DIK_ESCAPE)) {
            break;
        }

        double eddig = (double)VideoFrameIndex * (STOPWATCH_MULTIPLIER * 1000.0 * 0.0024) /
                       EolSettings->recording_fps();

        int befejezte1 =
            !replaymag(Motor1, &State->keys1, &valt1, Rec1, eddig, &Viewtime1, &valt2.draw_view);
        int befejezte2 = 0;
        if (!Single) {
            befejezte2 = !replaymag(Motor2, &State->keys2, &valt2, Rec2, eddig, &Viewtime2,
                                    &valt1.draw_view);
        }

        if ((Single && befejezte1) || (!Single && befejezte1 && befejezte2)) {
            break;
        }

        if (!Single) {
            flagtag_replay(eddig);
        }

        render_game(eddig, &valt1, &valt2, Viewtime1.viewkinreplay, Viewtime1.timekinreplay,
                    Viewtime2.viewkinreplay, Viewtime2.timekinreplay, current_camera);

        VideoFrameIndex++;
    }

    VideoRecordingMode = false;
    Ptop->unflip_objects();
}
