#include "ALL.H"

static void surlodasverseny(motorst* pmot, vekt2 fgumi, vekt2 sebesseg);

static double Maxsurlodas = 0;

static void szogigazit(double* pd) {
    if (*pd < -K_pi) {
        *pd += 2 * K_pi;
    }
    if (*pd > K_pi) {
        *pd -= 2 * K_pi;
    }
}

/*static double Elmozdhatar1 = 10.4;
static double Elmozdhatar2 = 10.5;
static double Atmenet = Elmozdhatar2 - Elmozdhatar1;
static double Szorzo = 2.0;

static pic8* Ppic8 = NULL;
*/

static void erokszamitasa(motorst* pmot, rigidbody* pkor, vekt2 i1, vekt2 j1, double kordx,
                          double kordy, vekt2* pFkerek, vekt2* pFtest, double* pMtest,
                          double* pMkerek) {
    vekt2 gumis = i1 * kordx + j1 * kordy;
    vekt2 gumisabsz = gumis + pmot->bike.r;

    // Gumi huzoero kiszamolasa:
    vekt2 gumi = gumisabsz - pkor->r;
    double Fsugar = 0;
    double Ftang = 0;
    if (gumi.x < -0.0001 || gumi.x > 0.0001 || gumi.y < -0.0001 || gumi.y > 0.0001) {
        // Gumieronek szetbontasa ket komponensre:
        double rudhossz = abs(gumis);
        vekt2 rudegys = gumis * (1.0 / rudhossz);
        vekt2 rudegysmer = forgatas90fokkal(rudegys);

        Fsugar = (gumi * rudegys) * SpringTensionCoefficient;
        Ftang = (gumi * rudegysmer) * SpringTensionCoefficient;

        *pFkerek = Fsugar * rudegys + Ftang * rudegysmer;

        *pFtest = Vekt2null - *pFkerek;
        *pMtest = -Ftang * rudhossz;
    } else {
        *pFkerek = vekt2();
        *pFtest = vekt2();
        *pMtest = 0;
    }
    // Most jon surlodas szamitasa:
    vekt2 koto = pkor->r - pmot->bike.r;
    double kotol = abs(koto);
    double reckotol = 1.0 / kotol;
    vekt2 kotoe = koto * reckotol;
    vekt2 kotomer = forgatas90fokkal(koto);
    vekt2 kotoemer = forgatas90fokkal(kotoe);
    vekt2 korongrelv = (kotomer * pmot->bike.angular_velocity + pmot->bike.v) - pkor->v;
    double vlong = korongrelv * kotoe;
    double vtang = korongrelv * kotoemer;
    vekt2 Fklong = (vlong * SpringResistanceCoefficient) * kotoe;
    vekt2 Fktang = (vtang * SpringResistanceCoefficient) * kotoemer;

    // Most jon kerek forgatonyomateka:
    vekt2 Ftestnyom = kotoemer * (*pMkerek * reckotol);

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
    vekt2 i(cos(pmot->bike.rotation), sin(pmot->bike.rotation));
    vekt2 j = forgatas90fokkal(i);

    if (pmot->flipped_bike) {
        pmot->head_r = pmot->body_r + i * 0.09 + j * 0.63;
    } else {
        pmot->head_r = pmot->body_r - i * 0.09 + j * 0.63;
    }
}

void leptet(motorst* pmot, double most, double dt, int gaz, int fek, int ugrik1, int ugrik2) {
    Maxsurlodas = 0;

    vekt2 i1(cos(pmot->bike.rotation), sin(pmot->bike.rotation));
    vekt2 j1 = forgatas90fokkal(i1);

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

    vekt2 Fkerek2;
    vekt2 Ftest2;
    double Mtest2;
    erokszamitasa(pmot, &pmot->left_wheel, i1, j1, LeftWheelDX, LeftWheelDY, &Fkerek2, &Ftest2,
                  &Mtest2, &Mkerek2);

    vekt2 Fkerek4;
    vekt2 Ftest4;
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
        //  hiba( "Ugrasban!" );
        pmot->volting_right = 1;
        pmot->angular_velocity_pre_right_volt = pmot->bike.angular_velocity;
        pmot->right_volt_time = most;
        pmot->bike.angular_velocity -= Loket;
    }
    if (ugrik2) {
        // if( Ugrasban1 || Ugrasban2 )
        //   hiba( "Ugrasban!" );
        pmot->volting_left = 1;
        pmot->angular_velocity_pre_left_volt = pmot->bike.angular_velocity;
        pmot->left_volt_time = most;
        pmot->bike.angular_velocity += Loket;
    }
    if (ugrik1 || ugrik2) {
        // Vezetot kulon forgatjuk meg:
        double domega = pmot->bike.angular_velocity - oldomega;
        vekt2 tangens = forgatas90fokkal(pmot->body_r - pmot->bike.r);
        pmot->body_v = pmot->body_v + tangens * domega;
    }

    /*
    rigidbody_movement( &pmot->bike, Ftest2+Ftest4-Vekt2j*pmot->bike.mass*Gravity,
                    Mtest2+Mtest4, dt, 0 );
    rigidbody_movement( &pmot->left_wheel, Fkerek2-Vekt2j*pmot->left_wheel.mass*Gravity, Mkerek2,      dt, 1 );
    rigidbody_movement( &pmot->right_wheel, Fkerek4-Vekt2j*pmot->right_wheel.mass*Gravity, Mkerek4,      dt, 1
    );
    */

    switch (pmot->gravity_direction) {
    case MotorGravity::Down:
        body_movement(pmot, vekt2(0.0, -1.0), i1, j1, dt); // vezeto
        rigidbody_movement(&pmot->bike, Ftest2 + Ftest4 - Vekt2j * pmot->bike.mass * Gravity, Mtest2 + Mtest4,
                dt, 0);
        rigidbody_movement(&pmot->left_wheel, Fkerek2 - Vekt2j * pmot->left_wheel.mass * Gravity, Mkerek2, dt,
                1);
        rigidbody_movement(&pmot->right_wheel, Fkerek4 - Vekt2j * pmot->right_wheel.mass * Gravity, Mkerek4,
                dt, 1);
        break;
    case MotorGravity::Up:
        body_movement(pmot, vekt2(0.0, 1.0), i1, j1, dt); // vezeto
        rigidbody_movement(&pmot->bike, Ftest2 + Ftest4 + Vekt2j * pmot->bike.mass * Gravity, Mtest2 + Mtest4,
                dt, 0);
        rigidbody_movement(&pmot->left_wheel, Fkerek2 + Vekt2j * pmot->left_wheel.mass * Gravity, Mkerek2, dt,
                1);
        rigidbody_movement(&pmot->right_wheel, Fkerek4 + Vekt2j * pmot->right_wheel.mass * Gravity, Mkerek4,
                dt, 1);
        break;
    case MotorGravity::Left:
        body_movement(pmot, vekt2(-1.0, 0.0), i1, j1, dt); // vezeto
        rigidbody_movement(&pmot->bike, Ftest2 + Ftest4 - Vekt2i * pmot->bike.mass * Gravity, Mtest2 + Mtest4,
                dt, 0);
        rigidbody_movement(&pmot->left_wheel, Fkerek2 - Vekt2i * pmot->left_wheel.mass * Gravity, Mkerek2, dt,
                1);
        rigidbody_movement(&pmot->right_wheel, Fkerek4 - Vekt2i * pmot->right_wheel.mass * Gravity, Mkerek4,
                dt, 1);
        break;
    case MotorGravity::Right:
        body_movement(pmot, vekt2(1.0, 0.0), i1, j1, dt); // vezeto
        rigidbody_movement(&pmot->bike, Ftest2 + Ftest4 + Vekt2i * pmot->bike.mass * Gravity, Mtest2 + Mtest4,
                dt, 0);
        rigidbody_movement(&pmot->left_wheel, Fkerek2 + Vekt2i * pmot->left_wheel.mass * Gravity, Mkerek2, dt,
                1);
        rigidbody_movement(&pmot->right_wheel, Fkerek4 + Vekt2i * pmot->right_wheel.mass * Gravity, Mkerek4,
                dt, 1);
        break;
    }

    szamitfejr(pmot);
}

// 0-meghalt, 2-semmi kulonos
int vizsgalat(motorst* pmot) {
    vekt2 t1, t2;
    vonal *pv1, *pv2;
    if (talppontkereses(pmot->head_r, HeadRadius, &t1, &t2, &pv1, &pv2)) {
        return 0;
    }

    // Objektumokkal utkozesvizsgalat:
    int voltkaja = 1;
    while (voltkaja) { // Ha nem volt kaja kilepunk, kulonben vegtelen ciklus
        voltkaja = 0;
        int sorszamtomb[3];
        sorszamtomb[0] = utkozikesprite(pmot->left_wheel.r, pmot->left_wheel.radius);
        sorszamtomb[1] = utkozikesprite(pmot->right_wheel.r, pmot->right_wheel.radius);
        sorszamtomb[2] = utkozikesprite(pmot->head_r, HeadRadius);
        for (int i = 0; i < 3; i++) {
            if (sorszamtomb[i] >= 0) {
                startwavegyujto(0, 0.0, sorszamtomb[i]);
                kerek* pker = Ptop->getptrkerek(sorszamtomb[i]);
                if (pker->tipus == T_KAJA) {
                    pker->aktiv = 0;
                    voltkaja = 1; // Hatha van meg tobb kaja is
                }
            }
        }
    }

    return 2;
}

static double Oszto = 1.0 / 1.0;

static void surlodasverseny(motorst* pmot, vekt2 fgumi_v, vekt2 sebesseg_v) {
    vekt2 joirany(cos(pmot->bike.rotation - K_pip2), sin(pmot->bike.rotation - K_pip2));
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
