#include "LEPTET.H"
#include "EDITUJ.H"
#include "level.h"
#include "object.h"
#include "physics_collision.h"
#include "physics_init.h"
#include "physics_move.h"
#include "platform_utils.h"
#include "recorder.h"
#include <cmath>

static void surlodasverseny(motorst* pmot, vect2 fgumi, vect2 sebesseg);

static double Maxsurlodas = 0;

static void szogigazit(double* pd) {
    if (*pd < -PI) {
        *pd += 2 * PI;
    }
    if (*pd > PI) {
        *pd -= 2 * PI;
    }
}

/*static double Elmozdhatar1 = 10.4;
static double Elmozdhatar2 = 10.5;
static double Atmenet = Elmozdhatar2 - Elmozdhatar1;
static double Szorzo = 2.0;

static pic8* Ppic8 = NULL;
*/

static void erokszamitasa(motorst* pmot, rigidbody* pkor, vect2 i1, vect2 j1, double kordx,
                          double kordy, vect2* pFkerek, vect2* pFtest, double* pMtest,
                          double* pMkerek) {
    vect2 gumis = i1 * kordx + j1 * kordy;
    vect2 gumisabsz = gumis + pmot->bike.r;

    // Gumi huzoero kiszamolasa:
    vect2 gumi = gumisabsz - pkor->r;
    double Fsugar = 0;
    double Ftang = 0;
    if (gumi.x < -0.0001 || gumi.x > 0.0001 || gumi.y < -0.0001 || gumi.y > 0.0001) {
        // Gumieronek szetbontasa ket komponensre:
        double rudhossz = gumis.length();
        vect2 rudegys = gumis * (1.0 / rudhossz);
        vect2 rudegysmer = rotate_90deg(rudegys);

        Fsugar = (gumi * rudegys) * SpringTensionCoefficient;
        Ftang = (gumi * rudegysmer) * SpringTensionCoefficient;

        *pFkerek = Fsugar * rudegys + Ftang * rudegysmer;

        *pFtest = Vect2null - *pFkerek;
        *pMtest = -Ftang * rudhossz;
    } else {
        *pFkerek = vect2();
        *pFtest = vect2();
        *pMtest = 0;
    }
    // Most jon surlodas szamitasa:
    vect2 koto = pkor->r - pmot->bike.r;
    double kotol = koto.length();
    double reckotol = 1.0 / kotol;
    vect2 kotoe = koto * reckotol;
    vect2 kotomer = rotate_90deg(koto);
    vect2 kotoemer = rotate_90deg(kotoe);
    vect2 korongrelv = (kotomer * pmot->bike.angular_velocity + pmot->bike.v) - pkor->v;
    double vlong = korongrelv * kotoe;
    double vtang = korongrelv * kotoemer;
    vect2 Fklong = (vlong * SpringResistanceCoefficient) * kotoe;
    vect2 Fktang = (vtang * SpringResistanceCoefficient) * kotoemer;

    // Most jon kerek forgatonyomateka:
    vect2 Ftestnyom = kotoemer * (*pMkerek * reckotol);

    *pFkerek = *pFkerek + Fklong + Fktang - Ftestnyom;
    *pMtest += -(Fktang * kotomer);
    *pFtest = *pFtest - Fklong - Fktang + Ftestnyom;

    surlodasverseny(pmot, gumi, korongrelv);
    // surlodasverseny( Ftang, vtang );
}

// static Voltfek = 0;
// static double Dfek2 = 0, Dfek4 = 0;
static double Loket = 12.0;
static double Omegavalt = 3.0;

// static int Ugrasban1 = 1, Ugrasban2 = 1; // Ezeket nullazni kell minden kezdesnel!
// static double Ugras1kezd = -1.0, Ugras2kezd = -1.0;
// static double Kezdoomega1 = -1.0, Kezdoomega2 = -1.0;

void resetleptet(motorst* pmot) {
    pmot->prev_brake = 0;
    pmot->left_wheel_brake_rotation = pmot->right_wheel_brake_rotation = 0;
    pmot->volting_right = pmot->volting_left = 0;
    pmot->right_volt_time = pmot->left_volt_time = -1.0;
    pmot->angular_velocity_pre_right_volt = pmot->angular_velocity_pre_left_volt = -1.0;
}

// Kor1-bol eloallitja Fejr-et!:
void szamitfejr(motorst* pmot) {
    vect2 i(cos(pmot->bike.rotation), sin(pmot->bike.rotation));
    vect2 j = rotate_90deg(i);

    if (pmot->flipped_bike) {
        pmot->head_r = pmot->body_r + i * 0.09 + j * 0.63;
    } else {
        pmot->head_r = pmot->body_r - i * 0.09 + j * 0.63;
    }
}

void leptet(motorst* pmot, double most, double dt, int gaz, int fek, int ugrik1, int ugrik2) {
    Maxsurlodas = 0;

    vect2 i1(cos(pmot->bike.rotation), sin(pmot->bike.rotation));
    vect2 j1 = rotate_90deg(i1);

    // Fek, gaz:
    if (!pmot->prev_brake && fek) {
        pmot->left_wheel_brake_rotation = pmot->left_wheel.rotation - pmot->bike.rotation;
        pmot->right_wheel_brake_rotation = pmot->right_wheel.rotation - pmot->bike.rotation;
    }
    pmot->prev_brake = fek;
    double Mkerek2 = 0;
    double Mkerek4 = 0;
    if (gaz) {
        double tulporgesomega = 110.0;
        double gaznyomatek = 600.0;
        if (pmot->flipped_bike) {
            if (pmot->left_wheel.angular_velocity > -tulporgesomega) {
                Mkerek2 = -gaznyomatek;
            }
        } else {
            if (pmot->right_wheel.angular_velocity < tulporgesomega) {
                Mkerek4 = gaznyomatek;
            }
        }
    }
    if (fek) {
        double fekero = 1000.0;
        double surlodas = 100.0;

        double dalfa =
            pmot->left_wheel.rotation - (pmot->bike.rotation + pmot->left_wheel_brake_rotation);
        double domega = pmot->left_wheel.angular_velocity - pmot->bike.angular_velocity;
        Mkerek2 = -fekero * dalfa - surlodas * domega;

        dalfa =
            pmot->right_wheel.rotation - (pmot->bike.rotation + pmot->right_wheel_brake_rotation);
        domega = pmot->right_wheel.angular_velocity - pmot->bike.angular_velocity;
        Mkerek4 = -fekero * dalfa - surlodas * domega;
    } else {
        szogigazit(&pmot->left_wheel.rotation);
        szogigazit(&pmot->right_wheel.rotation);
    }

    vect2 Fkerek2;
    vect2 Ftest2;
    double Mtest2;
    erokszamitasa(pmot, &pmot->left_wheel, i1, j1, LeftWheelDX, LeftWheelDY, &Fkerek2, &Ftest2,
                  &Mtest2, &Mkerek2);

    vect2 Fkerek4;
    vect2 Ftest4;
    double Mtest4;
    erokszamitasa(pmot, &pmot->right_wheel, i1, j1, RightWheelDX, RightWheelDY, &Fkerek4, &Ftest4,
                  &Mtest4, &Mkerek4);

    // Ugras elintezese:
    // Eloszor ugras befejezese, ha kell:
    double oldomega;
    if (ugrik1 || ugrik2) {
        oldomega = pmot->bike.angular_velocity;
    }

    if (pmot->volting_right &&
        (ugrik1 || ugrik2 || most > pmot->right_volt_time + VoltDelay * 0.25)) {
        pmot->bike.angular_velocity += Loket;
        if (pmot->bike.angular_velocity > pmot->angular_velocity_pre_right_volt) {
            pmot->bike.angular_velocity = pmot->angular_velocity_pre_right_volt;
        }
        // Meg marado szogsebesseg valtozast is okozunk, ha nem forgatjuk tul:
        if (pmot->bike.angular_velocity > 0.0) {
            pmot->bike.angular_velocity -= Omegavalt;
            if (pmot->bike.angular_velocity < 0.0) {
                pmot->bike.angular_velocity = 0.0;
            }
        }
        pmot->volting_right = 0;
        pmot->angular_velocity_pre_right_volt = -1.0;
        pmot->right_volt_time = -1.0;
    }
    if (pmot->volting_left &&
        (ugrik1 || ugrik2 || most > pmot->left_volt_time + VoltDelay * 0.25)) {
        pmot->bike.angular_velocity -= Loket;
        if (pmot->bike.angular_velocity < pmot->angular_velocity_pre_left_volt) {
            pmot->bike.angular_velocity = pmot->angular_velocity_pre_left_volt;
        }
        // Meg marado szogsebesseg valtozast is okozunk, ha nem forgatjuk tul:
        if (pmot->bike.angular_velocity < 0.0) {
            pmot->bike.angular_velocity += Omegavalt;
            if (pmot->bike.angular_velocity > 0.0) {
                pmot->bike.angular_velocity = 0.0;
            }
        }
        pmot->volting_left = 0;
        pmot->angular_velocity_pre_left_volt = -1.0;
        pmot->left_volt_time = -1.0;
    }

    // if( crcido % 511 == 103 )
    //   crccheck2(); Most ez nincs is belinkelve

    // Most ugras kezdes, ha kell:
    if (ugrik1) {
        // Csak azert van kikomentezve, mert egyszer elojott:
        // if( Ugrasban1 || Ugrasban2 )
        //  internal_error( "Ugrasban!" );
        pmot->volting_right = 1;
        pmot->angular_velocity_pre_right_volt = pmot->bike.angular_velocity;
        pmot->right_volt_time = most;
        pmot->bike.angular_velocity -= Loket;
    }
    if (ugrik2) {
        // if( Ugrasban1 || Ugrasban2 )
        //   internal_error( "Ugrasban!" );
        pmot->volting_left = 1;
        pmot->angular_velocity_pre_left_volt = pmot->bike.angular_velocity;
        pmot->left_volt_time = most;
        pmot->bike.angular_velocity += Loket;
    }
    if (ugrik1 || ugrik2) {
        // Vezetot kulon forgatjuk meg:
        double domega = pmot->bike.angular_velocity - oldomega;
        vect2 tangens = rotate_90deg(pmot->body_r - pmot->bike.r);
        pmot->body_v = pmot->body_v + tangens * domega;
    }

    /*
    rigidbody_movement( &pmot->bike, Ftest2+Ftest4-Vect2j*pmot->bike.mass*Gravity,
                    Mtest2+Mtest4, dt, false );
    rigidbody_movement( &pmot->left_wheel, Fkerek2-Vect2j*pmot->left_wheel.mass*Gravity, Mkerek2,
    dt, true ); rigidbody_movement( &pmot->right_wheel,
    Fkerek4-Vect2j*pmot->right_wheel.mass*Gravity, Mkerek4,      dt, true
    );
    */

    switch (pmot->gravity_direction) {
    case MotorGravity::Down:
        body_movement(pmot, vect2(0.0, -1.0), i1, j1, dt); // vezeto
        rigidbody_movement(&pmot->bike, Ftest2 + Ftest4 - Vect2j * pmot->bike.mass * Gravity,
                           Mtest2 + Mtest4, dt, false);
        rigidbody_movement(&pmot->left_wheel, Fkerek2 - Vect2j * pmot->left_wheel.mass * Gravity,
                           Mkerek2, dt, true);
        rigidbody_movement(&pmot->right_wheel, Fkerek4 - Vect2j * pmot->right_wheel.mass * Gravity,
                           Mkerek4, dt, true);
        break;
    case MotorGravity::Up:
        body_movement(pmot, vect2(0.0, 1.0), i1, j1, dt); // vezeto
        rigidbody_movement(&pmot->bike, Ftest2 + Ftest4 + Vect2j * pmot->bike.mass * Gravity,
                           Mtest2 + Mtest4, dt, false);
        rigidbody_movement(&pmot->left_wheel, Fkerek2 + Vect2j * pmot->left_wheel.mass * Gravity,
                           Mkerek2, dt, true);
        rigidbody_movement(&pmot->right_wheel, Fkerek4 + Vect2j * pmot->right_wheel.mass * Gravity,
                           Mkerek4, dt, true);
        break;
    case MotorGravity::Left:
        body_movement(pmot, vect2(-1.0, 0.0), i1, j1, dt); // vezeto
        rigidbody_movement(&pmot->bike, Ftest2 + Ftest4 - Vect2i * pmot->bike.mass * Gravity,
                           Mtest2 + Mtest4, dt, false);
        rigidbody_movement(&pmot->left_wheel, Fkerek2 - Vect2i * pmot->left_wheel.mass * Gravity,
                           Mkerek2, dt, true);
        rigidbody_movement(&pmot->right_wheel, Fkerek4 - Vect2i * pmot->right_wheel.mass * Gravity,
                           Mkerek4, dt, true);
        break;
    case MotorGravity::Right:
        body_movement(pmot, vect2(1.0, 0.0), i1, j1, dt); // vezeto
        rigidbody_movement(&pmot->bike, Ftest2 + Ftest4 + Vect2i * pmot->bike.mass * Gravity,
                           Mtest2 + Mtest4, dt, false);
        rigidbody_movement(&pmot->left_wheel, Fkerek2 + Vect2i * pmot->left_wheel.mass * Gravity,
                           Mkerek2, dt, true);
        rigidbody_movement(&pmot->right_wheel, Fkerek4 + Vect2i * pmot->right_wheel.mass * Gravity,
                           Mkerek4, dt, true);
        break;
    }

    szamitfejr(pmot);
}

// 0-meghalt, 2-semmi kulonos
int vizsgalat(motorst* pmot) {
    vect2 t1, t2;
    if (get_two_anchor_points(pmot->head_r, HeadRadius, &t1, &t2)) {
        return 0;
    }

    // Objektumokkal utkozesvizsgalat:
    int voltkaja = 1;
    while (voltkaja) { // Ha nem volt kaja kilepunk, kulonben vegtelen ciklus
        voltkaja = 0;
        int sorszamtomb[3];
        sorszamtomb[0] = get_touching_object(pmot->left_wheel.r, pmot->left_wheel.radius);
        sorszamtomb[1] = get_touching_object(pmot->right_wheel.r, pmot->right_wheel.radius);
        sorszamtomb[2] = get_touching_object(pmot->head_r, HeadRadius);
        for (int i = 0; i < 3; i++) {
            if (sorszamtomb[i] >= 0) {
                add_event_buffer(WavEvent::None, 0.0, sorszamtomb[i]);
                object* pker = Ptop->get_object(sorszamtomb[i]);
                if (pker->type == object::Type::Food) {
                    pker->active = false;
                    voltkaja = 1; // Hatha van meg tobb kaja is
                }
            }
        }
    }

    return 2;
}

static double Oszto = 1.0 / 1.0;

static void surlodasverseny(motorst* pmot, vect2 fgumi_v, vect2 sebesseg_v) {
    vect2 joirany(cos(pmot->bike.rotation - HALF_PI), sin(pmot->bike.rotation - HALF_PI));
    double fgumi = joirany * fgumi_v;
    double sebesseg = joirany * sebesseg_v;
    if (fgumi <= 0 || sebesseg <= 0) {
        return;
    }
    double ertek = fgumi * sebesseg * Oszto;
    if (ertek > Maxsurlodas) {
        Maxsurlodas = ertek;
    }
}

double kiszamolsurlodast(void) { return Maxsurlodas; }
