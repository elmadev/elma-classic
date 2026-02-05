#include "LEJATSZO.H"
#include "EDITUJ.H"
#include "eol_settings.h"
#include "flagtag.h"
#include "sound_engine.h"
#include "KIRAJZOL.H"
#include "LEPTET.H"
#include "level.h"
#include "lgr.h"
#include "main.h"
#include "object.h"
#include "physics_init.h"
#include "platform_impl.h"
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
int Tag = 0;
// int Multi = 0;

int Legkozment = 0;

static int Kajakell;

static void update_freecam(double dt, camera& current_camera) {
    double speed = 30.0;
    if (is_key_down(DIK_LSHIFT) || is_key_down(DIK_RSHIFT)) {
        speed *= 4.0;
    }
    double move = speed * dt;
    if (is_key_down(DIK_UP)) {
        current_camera.y += move;
    }
    if (is_key_down(DIK_DOWN)) {
        current_camera.y -= move;
    }
    if (is_key_down(DIK_LEFT)) {
        current_camera.x -= move;
    }
    if (is_key_down(DIK_RIGHT)) {
        current_camera.x += move;
    }

    current_camera.x = std::clamp(current_camera.x, current_camera.min_x, current_camera.max_x);
    current_camera.y = std::clamp(current_camera.y, current_camera.min_y, current_camera.max_y);
}

struct viewtimest {
    int viewkinplay, viewkinreplay;
    int timekinplay, timekinreplay;
};

static void initsoundlejatszoban(void) {
    static int Hangmarmegy = 0;
    if (State->sound_on && !Hangmarmegy) {
        Hangmarmegy = 1;
        sound_engine_init();
    }
}

// Player 1 es 2-re:
static viewtimest Viewtime1 = {1, 0, 1, 0};
static viewtimest Viewtime2 = {1, 0, 1, 0};

// 0-meghalt, 1-megnyerte, 2-semmi kulonos:
static int spritefeldolgoz(int sorszam, int* pkajaszam, motorst* pmot) {
    if (sorszam < 0 || sorszam >= MAX_OBJECTS) {
        internal_error("spritefeldolgoz-ban sorszam < 0 || sorszam >= MAXKEREK!");
    }
    if (!Ptop->objects[sorszam]) {
        internal_error("spritefeldolgoz-ban !Ptop->objects[sorszam]!");
    }

    object::Type tipus = Ptop->objects[sorszam]->type;

    if (tipus == object::Type::Killer) {
        return 0;
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
        return 2;
    }
    if (tipus == object::Type::Exit) {
        if (Motor1->apple_count + Motor2->apple_count >= Kajakell) {
            return 1;
        }
    }
    return 2;
}

static double baljobbszamol(double eltelt, int hatra) {
#ifdef DEBUG
    if (eltelt < 0) {
        internal_error("baljobbszamol-ban eltelt < 0!");
    }
#endif
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
static void belsoresz(motorst* pmot, player_keys* popciok, valtozok* pvalt, recorder* prec,
                      long* pmegvanido, int* pmeghalt, double eddig, double dt) {
    // Ugras elintezese:
    int ugrik1 = 0, ugrik2 = 0;
    if (eddig > pvalt->utolsougras + VoltDelay) {
        if (is_key_down(popciok->right_volt) || is_key_down(popciok->alovolt)) {
            ugrik1 = 1;
            pvalt->utolsougras = eddig;
            pvalt->ugras1volt = 1;
            add_event_buffer(WavEvent::RightVolt, 0.99, -1);
        }
        if (is_key_down(popciok->left_volt) || is_key_down(popciok->alovolt)) {
            ugrik2 = 1;
            pvalt->utolsougras = eddig;
            pvalt->ugras1volt = 0;
            add_event_buffer(WavEvent::LeftVolt, 0.99, -1);
        }
    }

    // LEPTET!!!!!!!:
    // 0-meghalt, 1-megnyerte, 2-semmi kulonos
    leptet(pmot, eddig, dt, is_key_down(popciok->gas),
           is_key_down(popciok->brake) || is_key_down(popciok->brake_alias), ugrik1, ugrik2);

    int eredmeny = vizsgalat(pmot);
    if (eredmeny == 0) {
        // Beteszunk egy 0 hangot:
        pvalt->inf.friction_volume = 0;
        pvalt->inf.motor_frequency = -1;
        pvalt->inf.gas = 0;
        prec->store_frames(pmot, eddig, &pvalt->inf);
        if (!Single && Tag) {
            // Flag Tag modban nem allnak meg, hanem visszamennek starthelyre:
            init_motor(pmot);
            // Kezdetihelydiff-et setallaktiv allitja be:
            pmot->bike.r = pmot->bike.r + BikeStartOffset;
            pmot->left_wheel.r = pmot->left_wheel.r + BikeStartOffset;
            pmot->right_wheel.r = pmot->right_wheel.r + BikeStartOffset;
            pmot->body_r = pmot->body_r + BikeStartOffset;

            // Valtozok inicializalasa:
            int showkep = pvalt->showkep;
            memset(pvalt, 0, sizeof(valtozok));
            pvalt->baljobbv_f.ucsoford = -1000.0;
            pvalt->baljobbv_f.ucsoforgas = -1000.0;
            pvalt->baljobbv_h.ucsoford = -1000.0;
            pvalt->baljobbv_h.ucsoforgas = -1000.0;
            pvalt->utolsougras = -100.0;
            pvalt->showkep = showkep;

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
    pvalt->inf.friction_volume = kiszamolsurlodast();

    if (pmot->flipped_bike) {
        pvalt->inf.motor_frequency = fabs(pmot->left_wheel.angular_velocity) * 0.025;
    } else {
        pvalt->inf.motor_frequency = fabs(pmot->right_wheel.angular_velocity) * 0.025;
    }
    // Frekvenciat betesszuk 1.0-2.0 tartomanyba:
    if (pvalt->inf.motor_frequency > 30.0) {
        pvalt->inf.motor_frequency = 30.0;
    }
    pvalt->inf.motor_frequency = 2.0 - exp(-pvalt->inf.motor_frequency);

    pvalt->inf.gas = (char)is_key_down(popciok->gas);
    prec->store_frames(pmot, eddig, &pvalt->inf);

    // Egyedi hangok:
    WavEvent wavazonosito;
    double hangero;
    int objszam;
    while (get_event_buffer(&wavazonosito, &hangero, &objszam)) {
        if (objszam >= 0) {
            int prev_apple_count = pmot->apple_count;
            int eredmeny = spritefeldolgoz(objszam, &pmot->apple_count, pmot);
            if (eredmeny == 0 || eredmeny == 1) {
                if (eredmeny == 0) {
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

static void toggleresz(player_keys* popciok, valtozok* pvalt, int* pviewkin, int* ptimekin,
                       int* pmasikshow) {
    // Showkep:
    if (!pvalt->showkepnyomva && is_key_down(popciok->toggle_visibility)) {
        Kitoltestmegrak = Kitoltestmegrakkezd;
        if (!*pmasikshow) {
            // Ha masik jatekos ablaka nem latszott, mindegyik fog latszani:
            *pmasikshow = 1;
            pvalt->showkep = 1;
        } else {
            pvalt->showkep = !pvalt->showkep;
        }
    }
    pvalt->showkepnyomva = is_key_down(popciok->toggle_visibility);
    // View ablak:
    if (!pvalt->viewnyomva && is_key_down(popciok->toggle_minimap)) {
        *pviewkin = !*pviewkin;
    }
    pvalt->viewnyomva = is_key_down(popciok->toggle_minimap);
    // Ido kint legyen vagy ne:
    if (!pvalt->timenyomva && is_key_down(popciok->toggle_timer)) {
        *ptimekin = !*ptimekin;
    }
    pvalt->timenyomva = is_key_down(popciok->toggle_timer);
}

static void baljobbelintez(baljobbvaltozok* pvalt, recorder* prec, double eddig, int hatra) {
    if (pvalt->eddighatra != hatra) {
        pvalt->ucsoforgas = eddig;
        double eltelt = eddig - pvalt->ucsoford;
        double camera_flip_time = EolSettings->turn_time() + 0.15;
        if (camera_flip_time > 0.0 && eltelt < camera_flip_time) {
            pvalt->ucsoford = eddig + eltelt - camera_flip_time;
        } else {
            pvalt->ucsoford = eddig;
        }
        pvalt->eddighatra = hatra;
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
        pvalt->forgas = 1.0;
    } else {
        pvalt->forgas = (eddig - pvalt->ucsoforgas) / turn_time;
#ifdef DEBUG
        if (pvalt->forgas < 0.0) {
            internal_error("lejatszo-ban forgas < 0.0!");
        }
#endif
        if (pvalt->forgas > 1.0) {
            pvalt->forgas = 1.0;
        }
    }

    pvalt->baljobb = baljobbszamol(eddig - pvalt->ucsoford, hatra);
}

static void kulsoresz(motorst* pmot, player_keys* popciok, valtozok* pvalt, recorder* prec,
                      viewtimest* pvt, double eddig, int* pmasikshow, int meghalt) {
    toggleresz(popciok, pvalt, &pvt->viewkinplay, &pvt->timekinplay, pmasikshow);

    // Hatra fordulas elintezes:
    if (!meghalt) {
        if (!pvalt->hatranyomva && is_key_down(popciok->turn)) {
            pmot->flipped_bike = !pmot->flipped_bike;
            szamitfejr(pmot);
        }
        pvalt->hatranyomva = is_key_down(popciok->turn);
    }
    baljobbelintez(&pvalt->baljobbv_f, prec, eddig, pmot->flipped_bike);

    // Hatra elintezes:
    if (pmot->gravity_direction == MotorGravity::Up) {
        pmot->flipped_camera = !pmot->flipped_bike;
    } else {
        pmot->flipped_camera = pmot->flipped_bike;
    }
    baljobbelintez(&pvalt->baljobbv_h, NULL, eddig, pmot->flipped_camera);

    // Kirajzolashoz kell:
    pvalt->ugrasnagysag = 1.0 - (eddig - pvalt->utolsougras) / VoltDelay;
    if (pvalt->ugrasnagysag < 0) {
        pvalt->ugrasnagysag = 0;
    }
#ifdef DEBUG
    if (pvalt->ugrasnagysag > 1.00001) {
        internal_error("pvalt->ugrasnagysag > 1.00001!");
    }
#endif
}

// lejatszo ezt beallitja: 0 -> nem kell kiirni, 1 -> A eloszor, 2 -> B
// csak akkor A vagy B eloszor, ha mindketten meghaltak, tehat
// menu_level menu-nek ezt mar feldolgozta elore:
int MeghalteloszorAB = 0;

// Ha sikeres szint, ide teszi be hogy ki ment ki:
int Aerintetteviragot = 0;

int Masodikmenet = 0;

// Idot adja vissza szazadmasodpercben:
long lejatszo(const char* filenev, CameraMode cameramode) {
    // internal_error( "Ezegyhosszusor,szetkellvagnibiztosanhibaEzegy hosszu sor, szet kell vagni
    // biztosan!",
    //	"Ez egy masik hosszu sor, szet kell vagni biztosan!" );

    // Azert allitom 1-be, hogy mindenkeppen eszrevegyem, ha nem allitom
    // meg kilepes elott be rendesen:
    MeghalteloszorAB = 1;
    Aerintetteviragot = 0;

    // Jatekmod eldontese:
    Single = State->single;
    Tag = State->flag_tag;
    Rec1->set_flagtag(Tag);
    Rec2->set_flagtag(Tag);

    load_best_time(filenev, Single);

    if (Single) {
        MultiplayerRec = 0;
    } else {
        MultiplayerRec = 1;
    }

    int palmegnincs = 1;
    init_physics_data();
    if (!Ptop) {
        internal_error("lejatszo-ban !Ptop!");
    }
    Ptop->flip_objects();
    Ptop->sort_objects();
    // setallaktiv allitja be motor kezdeti helyzetet es fazisokat is!:
    Kajakell = Ptop->initialize_objects(Motor1);
    Ptop->initialize_objects(Motor2);

    // Eloszor keretet tobbszor kirakja kirajzol320:
    Kitoltestmegrak = Kitoltestmegrakkezd;

    double eddig = 0.0;
    reset_event_buffer();

    int plussznyomva = 0;
    int minusznyomva = 0;
    int snapnyomva = 0;
    stopwatch_reset();

    // Valtozok inicializalasa:
    valtozok valt1; // Inicializalni kell !!!!!!!!!!!!
    memset(&valt1, 0, sizeof(valt1));
    valtozok valt2;
    memset(&valt2, 0, sizeof(valt2));
    valt1.baljobbv_f.ucsoford = valt2.baljobbv_f.ucsoford = -1000.0;
    valt1.baljobbv_f.ucsoforgas = valt2.baljobbv_f.ucsoforgas = -1000.0;
    valt1.baljobbv_h.ucsoford = valt2.baljobbv_h.ucsoford = -1000.0;
    valt1.baljobbv_h.ucsoforgas = valt2.baljobbv_h.ucsoforgas = -1000.0;
    valt1.utolsougras = valt2.utolsougras = -100.0;
    valt1.showkep = valt2.showkep = 1;

    int meghalt1 = 0;
    int meghalt2 = 0;
    long megvanido1 = 0;
    long megvanido2 = 0;
    resetleptet(Motor1);
    resetleptet(Motor2);

    camera current_camera;
    current_camera.mode = cameramode;
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

    flagtag_reset();

    Motor1->apple_count = Motor2->apple_count = 0;
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
                    MeghalteloszorAB = 1;
                }
                if (meghalt2) {
                    MeghalteloszorAB = 2;
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
                    MeghalteloszorAB = 0;
                }
                if (megvanido1) {
                    Aerintetteviragot = 1;
                } else {
                    Aerintetteviragot = 0;
                }

                set_motor_frequency(true, 1.0, 0);
                set_motor_frequency(false, 1.0, 0);
                if (megvanido) {
                    start_wav(WavEvent::Win, 0.999);
                } else {
                    start_wav(WavEvent::Dead, 0.999);
                }
                delay((int)(LevelEndDelay * 1000.0));

                stop_motor_sound(true);
                stop_motor_sound(false);

                Mute = true;
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
        if (!Single && Tag) {
            flagtag(eddig);
        }

        if (Single) {
            set_friction_volume(valt1.inf.friction_volume);
            set_motor_frequency(true, valt1.inf.motor_frequency, valt1.inf.gas);
        } else {
            set_friction_volume(valt1.inf.friction_volume + valt2.inf.friction_volume);
            set_motor_frequency(true, valt1.inf.motor_frequency, valt1.inf.gas);
            set_motor_frequency(false, valt2.inf.motor_frequency, valt2.inf.gas);
        }

        kulsoresz(Motor1, &State->keys1, &valt1, Rec1, &Viewtime1, eddig, &valt2.showkep, meghalt1);

        if (!Single) {
            kulsoresz(Motor2, &State->keys2, &valt2, Rec2, &Viewtime2, eddig, &valt1.showkep,
                      meghalt2);
        }

        if (palmegnincs) {
            palmegnincs = 0;
            Lgr->pal->set();
        }
        kirajzol320(eddig, &valt1, &valt2, Viewtime1.viewkinplay, Viewtime1.timekinplay,
                    Viewtime2.viewkinplay, Viewtime2.timekinplay, current_camera);

        // Egy par kozos toggle:
        // Plusz elintezes:
        if (!plussznyomva && is_key_down(State->key_increase_screen_size)) {
            novelkepmeret();
        }
        plussznyomva = is_key_down(State->key_increase_screen_size);
        // Minusz elintezes:
        if (!minusznyomva && is_key_down(State->key_decrease_screen_size)) {
            csokkentkepmeret();
        }
        minusznyomva = is_key_down(State->key_decrease_screen_size);
        // Snap nyomas elintezese:
        if (!snapnyomva && is_key_down(State->key_screenshot)) {
            Legkozment = 1;
        }
        snapnyomva = is_key_down(State->key_screenshot);

        // Vizsgalat kilepesre:
        if (is_key_down(DIK_ESCAPE) || is_key_down(State->key_escape_alias)) {
            MeghalteloszorAB = 0;
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

// Hamissal ter vissza, ha nincsen mar tobb:
static bool replaymag(motorst* pmot, player_keys* popciok, valtozok* pvalt, recorder* prec,
                      double eddig, viewtimest* pvt, int* pmasikshow) {

    toggleresz(popciok, pvalt, &pvt->viewkinreplay, &pvt->timekinreplay, pmasikshow);

    // recall koordokat es Hatra-t is beallitja:
    bool visszaad = prec->recall_frame(pmot, eddig, &pvalt->inf);
    szamitfejr(pmot); // mivel nincs leptet ami kiszamolna
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
                pvalt->ugras1volt = 1;
                pvalt->utolsougras = eddig;
            }
            if (wavindex == WavEvent::LeftVolt) {
                pvalt->ugras1volt = 0;
                pvalt->utolsougras = eddig;
            }
        }
    }

    // Hatra elintezes:
    baljobbelintez(&pvalt->baljobbv_f, NULL, eddig, pmot->flipped_bike);

    if (pmot->gravity_direction == MotorGravity::Up) {
        pmot->flipped_camera = !pmot->flipped_bike;
    } else {
        pmot->flipped_camera = pmot->flipped_bike;
    }
    baljobbelintez(&pvalt->baljobbv_h, NULL, eddig, pmot->flipped_camera);

    // Kirajzolashoz kellenek:
    pvalt->ugrasnagysag = 1.0 - (eddig - pvalt->utolsougras) / VoltDelay;
    if (pvalt->ugrasnagysag < 0) {
        pvalt->ugrasnagysag = 0;
    }
#ifdef DEBUG
    if (pvalt->ugrasnagysag > 1.00001) {
        internal_error("pvalt->ugrasnagysag > 1.00001!");
    }
#endif

    return visszaad;
}

static int Elozoshowkep1 = 1, Elozoshowkep2 = 1;

// Termeszetes befejezes eseten 0-t ad vissza:
long lejatszo_r(const char* filenev, int showkepmarad) {
    // Jatekmod eldontese:
    Single = !MultiplayerRec;
    Tag = Rec1->flagtag();

    // Refuse to play zero-length replays (from map-viewer mode)
    if (Rec1->is_empty()) {
        return -2;
    }
    if (!Single && Rec2->is_empty()) {
        return -2;
    }

    load_best_time(filenev, Single);

    int palmegnincs = 1;
    init_physics_data();
    if (!Ptop) {
        internal_error("lejatszo-ban !Ptop!");
    }
    Ptop->flip_objects();
    Ptop->sort_objects();
    // setallaktiv allitja be motor kezdeti helyzetet is!:
    Kajakell = Ptop->initialize_objects(Motor1);
    Ptop->initialize_objects(Motor2);

    // Eloszor keretet tobbszor is kirakja kirajzol320:
    Kitoltestmegrak = Kitoltestmegrakkezd;

    reset_event_buffer();
    stopwatch_reset();

    int plussznyomva = 0;
    int minusznyomva = 0;
    int snapnyomva = 0;

    // Valtozok inicializalasa:
    valtozok valt1; // Inicializalni kell !!!!!!!!!!!!
    memset(&valt1, 0, sizeof(valt1));
    valtozok valt2;
    memset(&valt2, 0, sizeof(valt2));
    valt1.baljobbv_f.ucsoford = valt2.baljobbv_f.ucsoford = -1000.0;
    valt1.baljobbv_f.ucsoforgas = valt2.baljobbv_f.ucsoforgas = -1000.0;
    valt1.baljobbv_h.ucsoford = valt2.baljobbv_h.ucsoford = -1000.0;
    valt1.baljobbv_h.ucsoforgas = valt2.baljobbv_h.ucsoforgas = -1000.0;
    valt1.utolsougras = valt2.utolsougras = -100.0;
    if (showkepmarad) {
        valt1.showkep = Elozoshowkep1;
        valt2.showkep = Elozoshowkep2;
    } else {
        valt1.showkep = 1;
        valt2.showkep = 1;
    }

    resetleptet(Motor1);
    resetleptet(Motor2);

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

    flagtag_reset();

    Motor1->apple_count = Motor2->apple_count = 0;
    long l = 0;
    int jar1 = 1, jar2 = 1;

    double current_replay_time = 0.0;
    double last_stopwatch = stopwatch();
    bool paused = false;

    while (1) {
        handle_events(); // Billentyut itt olvassuk be

        double now = stopwatch();
        double dt = (now - last_stopwatch) * 0.0024;
        last_stopwatch = now;

        double speed = 1.0;

        if (is_key_down(State->key_replay_fast_2x)) {
            speed *= 2.0;
        }
        if (is_key_down(State->key_replay_fast_4x)) {
            speed *= 4.0;
        }
        if (is_key_down(State->key_replay_fast_8x)) {
            speed *= 8.0;
        }

        if (is_key_down(State->key_replay_slow_2x)) {
            speed *= 0.5;
        }
        if (is_key_down(State->key_replay_slow_4x)) {
            speed *= 0.25;
        }

        if (is_key_down(State->key_replay_pause)) {
            paused = true;
        } else {
            paused = false;
        }

        if (!paused) {
            current_replay_time += dt * speed;
        }

        // Kiszamolja mennyit kell leptetni:
        double eddig = current_replay_time;
        // recall koordokat es Hatra-t is beallitja:
        int befejezte1 =
            !replaymag(Motor1, &State->keys1, &valt1, Rec1, eddig, &Viewtime1, &valt2.showkep);

        int befejezte2 = 0;
        if (!Single) {
            befejezte2 =
                !replaymag(Motor2, &State->keys2, &valt2, Rec2, eddig, &Viewtime2, &valt1.showkep);
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

            Elozoshowkep1 = valt1.showkep;
            Elozoshowkep2 = valt2.showkep;
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
            set_motor_frequency(true, valt1.inf.motor_frequency, valt1.inf.gas);
            // if( valt1.inf.motor_frequency > 1.5 )
            //	internal_error( "> 1.5!" );
            set_friction_volume(valt1.inf.friction_volume);
        } else {
            set_motor_frequency(true, valt1.inf.motor_frequency, valt1.inf.gas);
            set_motor_frequency(false, valt2.inf.motor_frequency, valt2.inf.gas);
            set_friction_volume(valt1.inf.friction_volume + valt2.inf.friction_volume);
        }

        // KIRAJZOLAS:
        if (palmegnincs) {
            palmegnincs = 0;
            Lgr->pal->set();
        }
        kirajzol320(eddig, &valt1, &valt2, Viewtime1.viewkinreplay, Viewtime1.timekinreplay,
                    Viewtime2.viewkinreplay, Viewtime2.timekinreplay, current_camera);

        // Egy par kozos toggle:
        // Plusz elintezes:
        if (!plussznyomva && is_key_down(State->key_increase_screen_size)) {
            novelkepmeret();

            // extern int Mostirdki;
            // Mostirdki = 1;
        }
        plussznyomva = is_key_down(State->key_increase_screen_size);
        // Minusz elintezes:
        if (!minusznyomva && is_key_down(State->key_decrease_screen_size)) {
            csokkentkepmeret();
        }
        minusznyomva = is_key_down(State->key_decrease_screen_size);
        // Snap nyomas elintezese:
        if (!snapnyomva && is_key_down(State->key_screenshot)) {
            Legkozment = 1;
        }
        snapnyomva = is_key_down(State->key_screenshot);

        // Vizsgalat kilepesre:
        if (is_key_down(DIK_ESCAPE)) {
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

            Elozoshowkep1 = valt1.showkep;
            Elozoshowkep2 = valt2.showkep;
            return -1;
        }
        l++;
    }
}
