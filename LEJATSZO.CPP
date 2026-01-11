#include "LEJATSZO.H"
#include "ALL.H"
#include "directinput_scancodes.h"
#include "EDITUJ.H"
#include "flagtag.h"
#include "HANGHIGH.H"
#include "KIRAJZOL.H"
#include "LEPTET.H"
#include "LGRFILE.H"
#include "main.h"
#include "object.h"
#include "physics_init.h"
#include "platform_impl.h"
#include "platform_utils.h"
#include "TOPOL.H"
#include <cmath>
#include <cstring>

// Jatek modja ket fele lehet:
int Single = 1;
// Van-e fogocska:
int Tag = 0;
// int Multi = 0;

int Legkozment = 0;

static int Kajakell;

struct viewtimest {
    int viewkinplay, viewkinreplay;
    int timekinplay, timekinreplay;
};

static void initsoundlejatszoban(void) {
    static int Hangmarmegy = 0;
    if (State->sound_on && !Hangmarmegy) {
        Hangmarmegy = 1;
        Hangenabled = 1;

        Mute = 1;
        starthanghigh();
    }
}

// Player 1 es 2-re:
static viewtimest Viewtime1 = {1, 0, 1, 0};
static viewtimest Viewtime2 = {1, 0, 1, 0};

// 0-meghalt, 1-megnyerte, 2-semmi kulonos:
static int spritefeldolgoz(int sorszam, int* pkajaszam, motorst* pmot) {
    if (sorszam < 0 || sorszam >= MAXKEREK) {
        internal_error("spritefeldolgoz-ban sorszam < 0 || sorszam >= MAXKEREK!");
    }
    if (!Ptop->kerektomb[sorszam]) {
        internal_error("spritefeldolgoz-ban !Ptop->kerektomb[sorszam]!");
    }

    object::Type tipus = Ptop->kerektomb[sorszam]->type;

    if (tipus == object::Type::Killer) {
        return 0;
    }
    if (tipus == object::Type::Food) {
        Ptop->kerektomb[sorszam]->active = false;
        (*pkajaszam)++;
        startwavegyujto(WAV_EVES, 0.99, -1);
        switch (Ptop->kerektomb[sorszam]->property) {
        case KT_UP:
            pmot->gravity_direction = MotorGravity::Up;
            break;
        case KT_DOWN:
            pmot->gravity_direction = MotorGravity::Down;
            break;
        case KT_LEFT:
            pmot->gravity_direction = MotorGravity::Left;
            break;
        case KT_RIGHT:
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

static double Fordido = 0.5;
static double baljobbszamol(double eltelt, int hatra) {
#ifdef DEBUG
    if (eltelt < 0) {
        internal_error("baljobbszamol-ban eltelt < 0!");
    }
#endif
    if (hatra) {
        if (eltelt > Fordido) {
            return 0.0;
        }
        return 1.0 - eltelt / Fordido;
    } else {
        if (eltelt > Fordido) {
            return 1.0;
        }
        return eltelt / Fordido;
    }
}

static double Forgasido = 0.35;

// static int Marvoltgaz = 0;

// Ide csak *pmeghalt == 0-val erkezhet
static void belsoresz(motorst* pmot, player_keys* popciok, valtozok* pvalt, recorder* prec,
                      long* pmegvanido, int* pmeghalt, double eddig, double dt) {
    // Ugras elintezese:
    int ugrik1 = 0, ugrik2 = 0;
    if (eddig > pvalt->utolsougras + VoltDelay) {
        if (is_key_down(popciok->right_volt)) {
            ugrik1 = 1;
            pvalt->utolsougras = eddig;
            pvalt->ugras1volt = 1;
            startwavegyujto(WAV_UGRAS1, 0.99, -1);
        }
        if (is_key_down(popciok->left_volt)) {
            ugrik2 = 1;
            pvalt->utolsougras = eddig;
            pvalt->ugras1volt = 0;
            startwavegyujto(WAV_UGRAS2, 0.99, -1);
        }
    }

    // LEPTET!!!!!!!:
    // 0-meghalt, 1-megnyerte, 2-semmi kulonos
    leptet(pmot, eddig, dt, is_key_down(popciok->gas), is_key_down(popciok->brake), ugrik1, ugrik2);

    int eredmeny = vizsgalat(pmot);
    if (eredmeny == 0) {
        // Beteszunk egy 0 hangot:
        pvalt->inf.surlero = 0;
        pvalt->inf.frekvencia = -1;
        pvalt->inf.gaz = 0;
        prec->store(pmot, eddig, &pvalt->inf);
        if (!Single && Tag) {
            // Flag Tag modban nem allnak meg, hanem visszamennek starthelyre:
            init_motor(pmot);
            // Kezdetihelydiff-et setallaktiv allitja be:
            pmot->bike.r = pmot->bike.r + Kezdetihelydiff;
            pmot->left_wheel.r = pmot->left_wheel.r + Kezdetihelydiff;
            pmot->right_wheel.r = pmot->right_wheel.r + Kezdetihelydiff;
            pmot->body_r = pmot->body_r + Kezdetihelydiff;

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
    pvalt->inf.surlero = kiszamolsurlodast();

    if (pmot->flipped_bike) {
        pvalt->inf.frekvencia = fabs(pmot->left_wheel.angular_velocity) * 0.025;
    } else {
        pvalt->inf.frekvencia = fabs(pmot->right_wheel.angular_velocity) * 0.025;
    }
    // Frekvenciat betesszuk 1.0-2.0 tartomanyba:
    if (pvalt->inf.frekvencia > 30.0) {
        pvalt->inf.frekvencia = 30.0;
    }
    pvalt->inf.frekvencia = 2.0 - exp(-pvalt->inf.frekvencia);

    pvalt->inf.gaz = (char)is_key_down(popciok->gas);
    prec->store(pmot, eddig, &pvalt->inf);

    // Egyedi hangok:
    int wavazonosito;
    double hangero;
    int objszam;
    while (getwavegyujto(&wavazonosito, &hangero, &objszam)) {
        if (objszam >= 0) {
            int eredmeny = spritefeldolgoz(objszam, &pmot->apple_count, pmot);
            if (eredmeny == 0 || eredmeny == 1) {
                if (eredmeny == 0) {
                    *pmeghalt = 1;
                } else {
                    *pmegvanido = eddig * 100.0 / (182.0 * 0.0024);
                }
            }
        } else {
            startwave(wavazonosito, hangero);
        }
        prec->addhang(eddig, (char)wavazonosito, hangero, objszam);
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
        if (eltelt < Fordido) {
            pvalt->ucsoford = eddig + eltelt - Fordido;
        } else {
            pvalt->ucsoford = eddig;
        }
        pvalt->eddighatra = hatra;
        // Hang adas:
        if (prec) {
            // Most kihagyjuk wavgyujtot:
            startwave(WAV_FORDULAS, 0.99);
            prec->addhang(eddig, WAV_FORDULAS, 0.99, -1);
        }
    }

    // Kirajzolashoz kellenek:
    pvalt->forgas = (eddig - pvalt->ucsoforgas) / Forgasido;
#ifdef DEBUG
    if (pvalt->forgas < 0.0) {
        internal_error("lejatszo-ban forgas < 0.0!");
    }
#endif
    if (pvalt->forgas > 1.0) {
        pvalt->forgas = 1.0;
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
// afterplay menu-nek ezt mar feldolgozta elore:
int MeghalteloszorAB = 0;

// Ha sikeres szint, ide teszi be hogy ki ment ki:
int Aerintetteviragot = 0;

int Masodikmenet = 0;

// Idot adja vissza szazadmasodpercben:
long lejatszo(const char* filenev) {
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
    Prec1->settag(Tag);
    Prec2->settag(Tag);

    beallitlegjobbidot(filenev, Single);

    if (Single) {
        Multirec = 0;
    } else {
        Multirec = 1;
    }

    int palmegnincs = 1;
    init_physics_data();
    if (!Ptop) {
        internal_error("lejatszo-ban !Ptop!");
    }
    Ptop->kereklefejjel();
    Ptop->killerekelore();
    // setallaktiv allitja be motor kezdeti helyzetet es fazisokat is!:
    Kajakell = Ptop->setallaktiv(Motor1);
    Ptop->setallaktiv(Motor2);

    // Eloszor keretet tobbszor kirakja kirajzol320:
    Kitoltestmegrak = Kitoltestmegrakkezd;

    double eddig = 0.0;
    resetwavegyujto();

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

    initsoundlejatszoban();
    Mute = 0;
    if (Single) {
        startmotor(1);
    } else {
        startmotor(1);
        startmotor(0);
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

            if (!meghalt1) {
                belsoresz(Motor1, &State->keys1, &valt1, Prec1, &megvanido1, &meghalt1, eddig, dt);
            }

            if (!Single && !meghalt2) {
                belsoresz(Motor2, &State->keys2, &valt2, Prec2, &megvanido2, &meghalt2, eddig, dt);
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
                        stopmotor(1);
                    } else {
                        stopmotor(0);
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

                setmotor(1, 1.0, 0);
                setmotor(0, 1.0, 0);
                if (megvanido) {
                    startwave(WAV_SIKER, 0.999);
                } else {
                    startwave(WAV_TORES, 0.999);
                }
                hangosdelay((int)(LevelEndDelay * 1000.0));

                stopmotor(1);
                stopmotor(0);

                Mute = 1;
                if (megvanido) {
                    Ptop->kerekjolalljon();
                    Prec1->encode_frame_count();
                    Prec2->encode_frame_count();
                    return megvanido;
                } else {
                    Ptop->kerekjolalljon();
                    Prec1->encode_frame_count();
                    Prec2->encode_frame_count();
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
            setsurlodas(valt1.inf.surlero);
            setmotor(1, valt1.inf.frekvencia, valt1.inf.gaz);
        } else {
            setsurlodas(valt1.inf.surlero + valt2.inf.surlero);
            setmotor(1, valt1.inf.frekvencia, valt1.inf.gaz);
            setmotor(0, valt2.inf.frekvencia, valt2.inf.gaz);
        }

        kulsoresz(Motor1, &State->keys1, &valt1, Prec1, &Viewtime1, eddig, &valt2.showkep,
                  meghalt1);

        if (!Single) {
            kulsoresz(Motor2, &State->keys2, &valt2, Prec2, &Viewtime2, eddig, &valt1.showkep,
                      meghalt2);
        }

        if (palmegnincs) {
            palmegnincs = 0;
            Plgr->pal->set();
        }
        kirajzol320(eddig, &valt1, &valt2, Viewtime1.viewkinplay, Viewtime1.timekinplay,
                    Viewtime2.viewkinplay, Viewtime2.timekinplay);

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
        if (is_key_down(DIK_ESCAPE)) {
            MeghalteloszorAB = 0;
            double eltelido = stopwatch();
            // Motor hangjat kikapcsolja:
            stopmotor(1);
            stopmotor(0);

            Mute = 1;

            // Kiirja framerate-et:
            double tpl = double(l) / eltelido;
            tpl *= 182;
            int framerate = (int)(tpl);
            if (access("f_rate.inf", 0) == 0) {
                // igyhagyni fopen-t!:
                FILE* h = fopen("f_rate.inf", "wt");
                if (!h) {
                    internal_error("framerate nem nyilik!");
                }
                fprintf(h, "%d", framerate);
                fclose(h);
            }
            Ptop->kerekjolalljon();
            Prec1->encode_frame_count();
            Prec2->encode_frame_count();
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
static int replaymag(motorst* pmot, player_keys* popciok, valtozok* pvalt, recorder* prec,
                     double eddig, viewtimest* pvt, int* pmasikshow) {

    toggleresz(popciok, pvalt, &pvt->viewkinreplay, &pvt->timekinreplay, pmasikshow);

    // recall koordokat es Hatra-t is beallitja:
    int visszaad = prec->recall(pmot, eddig, &pvalt->inf);
    szamitfejr(pmot); // mivel nincs leptet ami kiszamolna
    // HANG ELINTEZES:
    char wavindex;
    double hangero;
    int objszam;
    while (prec->gethang(eddig, &wavindex, &hangero, &objszam)) {
        if (objszam >= 0) {
            int tmpi = 0;
            spritefeldolgoz(objszam, &tmpi, pmot);
        } else {
            startwave(wavindex, hangero);
            if (wavindex == WAV_UGRAS1) {
                pvalt->ugras1volt = 1;
                pvalt->utolsougras = eddig;
            }
            if (wavindex == WAV_UGRAS2) {
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
    Single = !Multirec;
    Tag = Prec1->gettag();

    beallitlegjobbidot(filenev, Single);

    int palmegnincs = 1;
    init_physics_data();
    if (!Ptop) {
        internal_error("lejatszo-ban !Ptop!");
    }
    Ptop->kereklefejjel();
    Ptop->killerekelore();
    // setallaktiv allitja be motor kezdeti helyzetet is!:
    Kajakell = Ptop->setallaktiv(Motor1);
    Ptop->setallaktiv(Motor2);

    // Eloszor keretet tobbszor is kirakja kirajzol320:
    Kitoltestmegrak = Kitoltestmegrakkezd;

    resetwavegyujto();
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

    initsoundlejatszoban();
    Mute = 0;
    if (Single) {
        startmotor(1);
    } else {
        startmotor(1);
        startmotor(0);
    }

    flagtag_reset();

    // Motor1->apple_count = Motor2->apple_count = 0;
    long l = 0;
    int jar1 = 1, jar2 = 1;
    while (1) {
        handle_events(); // Billentyut itt olvassuk be
        // Kiszamolja mennyit kell leptetni:
        double eddig = stopwatch() * 0.0024;
        // recall koordokat es Hatra-t is beallitja:
        int befejezte1 =
            !replaymag(Motor1, &State->keys1, &valt1, Prec1, eddig, &Viewtime1, &valt2.showkep);

        int befejezte2 = 0;
        if (!Single) {
            befejezte2 =
                !replaymag(Motor2, &State->keys2, &valt2, Prec2, eddig, &Viewtime2, &valt1.showkep);
        }

        if ((Single && befejezte1) || (!Single && befejezte1 && befejezte2)) {
            // Motor hangjat kikapcsolja:
            setmotor(1, 1.0, 0);
            setmotor(0, 1.0, 0);
            stopmotor(1);
            stopmotor(0);
            setsurlodas(0);
            hangosdelay((int)(LevelEndDelay * 400.0));

            Mute = 1;
            Ptop->kerekjolalljon();

            Elozoshowkep1 = valt1.showkep;
            Elozoshowkep2 = valt2.showkep;
            return 0;
        }

        if (!Single) {
            if (jar1 && befejezte1) {
                stopmotor(1);
                jar1 = 0;
            }
            if (jar2 && befejezte2) {
                stopmotor(0);
                jar2 = 0;
            }

            // Fogocska:
            flagtag_replay(eddig);
        }

        // HANG ELINTEZES:
        if (Single) {
            setmotor(1, valt1.inf.frekvencia, valt1.inf.gaz);
            // if( valt1.inf.frekvencia > 1.5 )
            //	internal_error( "> 1.5!" );
            setsurlodas(valt1.inf.surlero);
        } else {
            setmotor(1, valt1.inf.frekvencia, valt1.inf.gaz);
            setmotor(0, valt2.inf.frekvencia, valt2.inf.gaz);
            setsurlodas(valt1.inf.surlero + valt2.inf.surlero);
        }

        // KIRAJZOLAS:
        if (palmegnincs) {
            palmegnincs = 0;
            Plgr->pal->set();
        }
        kirajzol320(eddig, &valt1, &valt2, Viewtime1.viewkinreplay, Viewtime1.timekinreplay,
                    Viewtime2.viewkinreplay, Viewtime2.timekinreplay);

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
            tpl *= 182;
            int framerate = (int)(tpl);
            if (access("f_rate.inf", 0) == 0) {
                // igyhagyni fopen-t!:
                FILE* h = fopen("f_rate.inf", "wt");
                if (!h) {
                    internal_error("framerate nem nyilik!");
                }
                fprintf(h, "%d", framerate);
                fclose(h);
            }
            // Hang kikapcsolas:
            setmotor(1, 1.0, 0);
            setmotor(0, 1.0, 0);
            stopmotor(1);
            stopmotor(0);
            setsurlodas(0);
            hangosdelay((int)(LevelEndDelay * 400.0));

            Mute = 1;

            Ptop->kerekjolalljon();

            Elozoshowkep1 = valt1.showkep;
            Elozoshowkep2 = valt2.showkep;
            return -1;
        }
        l++;
    }
}
