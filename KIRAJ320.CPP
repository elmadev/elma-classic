#include "KIRAJZOL.H"
#include "affine_pic.h"
#include "anim.h"
#include "affine_pic_render.h"
#include "ECSET.H"
#include "EDITUJ.H"
#include "eol_settings.h"
#include "flagtag.h"
#include "LEJATSZO.H"
#include "level.h"
#include "lgr.h"
#include "M_PIC.H"
#include "main.h"
#include "object.h"
#include "pic8.h"
#include "platform_utils.h"
#include "physics_init.h"
#include "timer.h"
#include <algorithm>
#include <cmath>
#include <cstring>

// Ezek mindig beallitodnak Kepmeret-bol kirajzol320-ban:

int Kitoltestmegrak = 0;
int Kitoltestmegrakkezd = 10;

static double Kepmeret = 1.0;

static double Szorzo = 1.1;
void novelkepmeret(void) {
    Kepmeret *= Szorzo;
    if (Kepmeret >= 0.999) {
        Kepmeret = 1.0;
    }
    Kitoltestmegrak = Kitoltestmegrakkezd;
}

void csokkentkepmeret(void) {
    Kepmeret /= Szorzo;
    if (Kepmeret < 0.7) {
        Kepmeret = 0.7;
    }
    Kitoltestmegrak = Kitoltestmegrakkezd;
}

// Kirajzolando terulet nagysaga:
static int Cx1 = 0, Cy1 = 0, Cx2 = SCREEN_WIDTH - 1, Cy2 = SCREEN_HEIGHT - 1, Cy1Bplayer = 0,
           Cy2Bplayer = 0;
int Cxsize = SCREEN_WIDTH, Cysize = SCREEN_HEIGHT;

// Motoros kepernyo mely reszen helyezkedik el:
static double Mo_y = 0.0, Mo_bal = 0.0, Mo_dx = 0.0;

double Hatarx1, Hatarx2, Hatary1, Hatary2;

// View ablak meretei:
static int Viewxsize, Viewysize;
static int Viewxorig, Viewxtolas;

affine_pic* shirt = nullptr;

void init_shirt() {
    constexpr int SHIRT_BMP_WIDTH = 149;
    constexpr int SHIRT_BMP_HEIGHT = 101;

    pic8* pic_shirt = pic8::from_bmp("bmp/shirt.bmp");
    if (pic_shirt && pic_shirt->get_width() == SHIRT_BMP_WIDTH &&
        pic_shirt->get_height() == SHIRT_BMP_HEIGHT) {
        shirt = new affine_pic(nullptr, pic_shirt);
        return;
    }

    delete pic_shirt;
}

void beallitmereteket(int splitscreen) {
    Cy1Bplayer = 100, Cy2Bplayer = 200;

    Cxsize = (int)(SCREEN_WIDTH * Kepmeret);
    Cysize = (int)(SCREEN_HEIGHT * Kepmeret);
    Cx1 = (SCREEN_WIDTH - Cxsize) / 2;
    Cy1 = (SCREEN_HEIGHT - Cysize) / 2;
    Cx2 = Cx1 + Cxsize - 1;
    Cy2 = Cy1 + Cysize - 1;
    if (splitscreen) {
        Cysize = (SCREEN_HEIGHT / 2) - 6;
        Cy1 = (SCREEN_HEIGHT / 2) + 6; // Mivel kep fejjel lefele van, forditva vannak koordok
        Cy2 = SCREEN_HEIGHT - 1;       // Vagyis A player van felul, B pedig lent
        Cy1Bplayer = 0;
        Cy2Bplayer = (SCREEN_HEIGHT / 2) - 7;
    }
#ifdef DEBUG
    if (Cx2 >= SCREEN_WIDTH) {
        internal_error("beallitmereteket Cx2 >= SCREEN_WIDTH!");
    }
    if (Cy2 >= SCREEN_HEIGHT) {
        internal_error("beallitmereteket Cy2 >= SCREEN_HEIGHT!");
    }
#endif

    // Ezek csak doboz kirajzolashoz kellenek:
    Hatarx1 = 2.0;
    Hatary1 = 2.0;
    Hatarx2 = Cxsize - 3.0;
    Hatary2 = Cysize - 3.0;

    // Motoros kepernyo mely reszen helyezkedik el:
    if (EolSettings->center_camera()) {
        Mo_bal = (Cxsize / MetersToPixels) * 0.50;
    } else {
        Mo_bal = (SCREEN_WIDTH / MetersToPixels) * 0.15;
    }
    Mo_dx = Cxsize / MetersToPixels - 2.0 * Mo_bal;
    Mo_y = Cysize / MetersToPixels / 2.0;

    Viewxsize = (int)(140.0 * sqrt(double(Cysize) / double(SCREEN_HEIGHT))); // 200
    Viewysize = (int)(70.0 * sqrt(double(Cysize) / double(SCREEN_HEIGHT)));  // 80
    Viewxorig = (int)(40.0 * (Kepmeret - 0.6) / 0.4);
    Viewxtolas = Cxsize - 2 * Viewxorig - Viewxsize;
}

void forditkepet(pic8* ppic) {
    for (int y1 = 0; y1 < ppic->get_height() / 2; y1++) {
        int y2 = ppic->get_height() - 1 - y1;
        unsigned char* sora = ppic->get_row(y1);
        unsigned char* sorb = ppic->get_row(y2);
        int xsize = ppic->get_width();
        for (int x = 0; x < xsize; x++) {
            unsigned char tmp = sora[x];
            sora[x] = sorb[x];
            sorb[x] = tmp;
        }
    }
}

// v1 iranyabol v2 fele interpolal:
/*static vect2 interpolal( vect2 v1, vect2 v2, double x ) {
    vect2 vissza = v1+(v2-v1)*x;
    return vissza;
} */

// Meg nem affin koordok:
static void kidoboz(vect2 a, vect2 b, pic8* ppic8, double szelesseg, affine_pic* pk, double tulloga,
                    double tullogb, int tukor = 0) {
    vect2 vegys = unit_vector(b - a);
    b = b + vegys * tullogb;
    a = a - vegys * tulloga;
    vect2 v = b - a;
    vect2 v2fel;
    if (tukor) {
        v2fel = rotate_90deg(vegys) * szelesseg;
    } else {
        v2fel = rotate_minus90deg(vegys) * szelesseg;
    }
    a = a - v2fel;

    a.x *= MetersToPixels;
    a.y *= MetersToPixels;
    v.x *= MetersToPixels;
    v.y *= MetersToPixels;
    v2fel.x *= MetersToPixels;
    v2fel.y *= MetersToPixels;
    draw_affine_pic(ppic8, pk, v, v2fel * 2.0, a);
}

// radius meg nincs megszorozva:
// Meg nem affin koordok:
static void kidobozkerek(vect2 r, double radius, double rotation, pic8* ppicbuffer, affine_pic* pk,
                         int tukor = 0) {
    vect2 vfel(cos(rotation) * radius, sin(rotation) * radius);
    if (tukor) {
        kidoboz(r + vfel, r - vfel, ppicbuffer, radius, pk, 0.0, 0.0, tukor);
    } else {
        kidoboz(r - vfel, r + vfel, ppicbuffer, radius, pk, 0.0, 0.0, tukor);
    }
}

static double Mx = -1, My = -1;
static vect2 Mi, Mj, Mr;
static vect2 Miar, Mjar, Mrar; // Ezek meg vannak szorozva Aranyossaggal

static void kitag(pic8* ppic8, affine_pic* pk, bike_box* pb) {
    vect2 r = Miar * (pb->x1 + 260 - Mx) + Mjar * (My - (pb->y1 + 260)) + Mrar;
    vect2 u = Miar * (pb->x2 - pb->x1);
    vect2 v = Mjar * (pb->y1 - pb->y2);

    draw_affine_pic(ppic8, pk, u, v, r);
}

static void kigyuru(pic8* ppic8, int x, int y, unsigned char szin) {
#ifdef DEBUG
    if (x < 1 || x >= ppic8->get_width() - 1 || y < 1 || y >= ppic8->get_height() - 1) {
        internal_error("kigyuru-ben x, vagy y lelog!");
    }
#endif
    unsigned char* sor = ppic8->get_row(y - 1);
    sor[x - 1] = szin;
    sor[x] = szin;
    sor[x + 1] = szin;
    sor = ppic8->get_row(y + 1);
    sor[x - 1] = szin;
    sor[x] = szin;
    sor[x + 1] = szin;
    ppic8->ppixel(x - 1, y, szin);
    ppic8->ppixel(x + 1, y, szin);
}

/* Most nincsen felho:
static void felhokfel( int x, int y, pic8* ppicbuffer, pic8* pfelho,
                            double lassit ) {
    int kerx = -x*lassit;
    int kery = -y*lassit;
    // Vegigmegy kepernyon:
    int kerxsize = pfelho->get_width();
    kerx %= kerxsize;
    kerx -= kerxsize; // Lehet, hogy enelkul is negativ?
    while( kerx < Displaysizex ) {
        int kerysize = pfelho->get_height();
        kery %= kerysize;
        kery -= kerysize; // Lehet, hogy enelkul is negativ?
        while( kery < Displaysizey ) {
            blit8( ppicbuffer, pfelho, kerx, kery );

            kery += kerysize;
        }
        kerx += kerxsize;
    }
} */

static void kiview(int ajatekos, pic8* ppic, double baljobb, vect2 motorkozep, motorst* pmot2) {

    double Viewxsize_d = Viewxsize * MinimapScaleFactor * PixelsToMeters;
    double Viewysize_d = Viewysize * MinimapScaleFactor * PixelsToMeters;

    double szeltoltav = EolSettings->center_map() ? 0.5 : 0.2;
    double atfuthossz = 1.0 - 2.0 * szeltoltav;
    vect2 sarokbolkozepre(Viewxsize_d * (szeltoltav + baljobb * atfuthossz), Viewysize_d / 2);
    vect2 sarok = motorkozep - sarokbolkozepre;

    double align;
    switch (EolSettings->map_alignment()) {
    case MapAlignment::None:
        align = baljobb;
        break;
    case MapAlignment::Left:
        align = 0.0;
        break;
    case MapAlignment::Middle:
        align = 0.5;
        break;
    case MapAlignment::Right:
        align = 1.0;
        break;
    }
    int vx1 = (int)(Viewxorig + align * Viewxtolas);
    int vx2 = vx1 + Viewxsize - 1;
    int vy1 = 1;
    int vy2 = vy1 + Viewysize - 1;

    Pecsetview->kiteszview(ajatekos, ppic, sarok, vx1, vy1, vx2, vy2);

    // Most kirajzoljuk food-okat:
    int balalsox, balalsoy;
    Pecsetview->getbalalso_int(sarok, &balalsox, &balalsoy);
    int objminx = balalsox;
    int objminy = balalsoy;
    int objmaxx = balalsox + Viewxsize - 1;
    int objmaxy = balalsoy + Viewysize - 1;
    for (int i = 0; i < MAX_OBJECTS; i++) {
        object* pker = Ptop->objects[i];
        if (!pker) {
            break;
        }

        int kell1 = pker->type == object::Type::Food && pker->active;
        int kell2 = (Single || !Tag) && pker->type == object::Type::Exit;

        if (!kell1 && !kell2) {
            continue;
        }

        if (pker->minimap_canvas_x < objminx || pker->minimap_canvas_y < objminy ||
            pker->minimap_canvas_x > objmaxx || pker->minimap_canvas_y > objmaxy) {
            continue;
        }

        // Potty kirajzolasa View ablakba:
        if (pker->type == object::Type::Food) {
            kigyuru(ppic, pker->minimap_canvas_x - balalsox + vx1,
                    pker->minimap_canvas_y - balalsoy + vy1, Lgr->minimap_food_palette_id);
        } else {
            kigyuru(ppic, pker->minimap_canvas_x - balalsox + vx1,
                    pker->minimap_canvas_y - balalsoy + vy1, Lgr->minimap_exit_palette_id);
        }
    }

    // Most kirajzoljuk motorost:
    // Eldontjuk szineket:
    unsigned char viewindex1 = Lgr->minimap_bike1_palette_id;
    unsigned char viewindex2 = Lgr->minimap_bike2_palette_id;
    if ((State->player1_bike1 && !ajatekos) || (!State->player1_bike1 && ajatekos)) {
        viewindex1 = Lgr->minimap_bike2_palette_id;
        viewindex2 = Lgr->minimap_bike1_palette_id;
    }

    // Most kirajzoljuk masik (hatso) motorost:
    if (pmot2) {
        vect2 sarokrel = pmot2->bike.r - sarok;
        int mx = (int)(sarokrel.x * MetersToPixels / MinimapScaleFactor + vx1);
        int my = (int)(sarokrel.y * MetersToPixels / MinimapScaleFactor + vy1);
        if (mx >= vx1 && mx <= vx2 && my >= vy1 && my <= vy2) {
            kigyuru(ppic, mx, my, viewindex2);
        }
    }

    // Most kirajzoljuk elso motorost:
    int mx = (int)(sarokbolkozepre.x * MetersToPixels / MinimapScaleFactor);
    int my = (int)(sarokbolkozepre.y * MetersToPixels / MinimapScaleFactor);
    kigyuru(ppic, vx1 + mx, vy1 + my, viewindex1);

    // Kirajzoljuk keretet:
    vx1--;
    vy1--;
    vx2++;
    vy2++;
    for (int y = vy1; y <= vy2; y++) {
        ppic->ppixel(vx1, y, Lgr->minimap_border_palette_id);
        ppic->ppixel(vx2, y, Lgr->minimap_border_palette_id);
    }
    unsigned char* sor = ppic->get_row(vy1) + vx1 + 1;
    memset(sor, Lgr->minimap_border_palette_id, Viewxsize);
    sor = ppic->get_row(vy2) + vx1 + 1;
    memset(sor, Lgr->minimap_border_palette_id, Viewxsize);
}

// Mentes lemezre:
static void menteshakell(pic8* ppic) {
    if (Legkozment) {
        Legkozment = 0;
        int i = 0;
        while (1) {
            char filenev[20];
            if (i < 10) {
                sprintf(filenev, "snap0%d.pcx", i);
            } else {
                sprintf(filenev, "snap%d.pcx", i);
            }
            if (access(filenev, 0) != 0) {
                forditkepet(ppic);
                ppic->save(filenev, Lgr->palette_data);
                forditkepet(ppic);
                break;
            }
            i++;
        }
    }
}

static void kitolthatteret(pic8* ppic) {
    int y = 0;
    while (y < SCREEN_HEIGHT) {
        int x = 0;
        while (x < SCREEN_WIDTH) {
            // internal_error( "Itt van 14!" ); idejott
            // internal_error( tmp ); (16) kiirja koordokat szepen

            // extern int Debugblt;
            // Debugblt = 1;
            blit8(ppic, Lgr->qframe, x, y);
            // Debugblt = 0;
            //  internal_error( "Itt van 20!" ); idejott
            x += Lgr->qframe->get_width();

            // internal_error( tmp ); (21) kiirja x-et 207-nek
        }
        // internal_error( "Itt van 15!" );
        y += Lgr->qframe->get_height();
    }
}

#ifdef DEBUG
int Voltkilogas = 0;
#endif

static void kibike(int ajatekos, pic8* ppic, double t, vect2 sarok, motorst* pmot, valtozok* pvalt,
                   bike_pics* pmk, affine_pic* shirt) {

    // Mivel ezeket fv megvaltoztatja:
    double ugrasnagysag = pvalt->ugrasnagysag;
    double forgas = pvalt->baljobbv_f.forgas;

    // if( pvalt->baljobb < 0 || pvalt->baljobb > 1.0 )
    //     internal_error( "pvalt->baljobb < 0 || pvalt->baljobb > 1.0!" );
    // if( forgas < 0.0 || forgas > 1.0 )
    //     internal_error( "forgas < 0.0 || forgas > 1.0!" );

    // Ha kell affinitassal torodni:
    int vanforgas = 0;
    StretchEnabled = false;
    if (forgas < 0.999) {
        vanforgas = 1;
        // forgast 0, 1 intervallumbol atkonvertaljuk -1, 1 intervallumba:
        forgas = -cos(forgas * PI);
    }

    vect2 p_k1 = (pmot->left_wheel.r - sarok);
    vect2 p_k2 = (pmot->right_wheel.r - sarok);

    int elsokerekrogton = 1;
    int hatsokerekrogton = 1;

    if (vanforgas) {
        if ((forgas > 0.0 && !pmot->flipped_bike) || (forgas <= 0.0 && pmot->flipped_bike)) {
            elsokerekrogton = 0;
        } else {
            hatsokerekrogton = 0;
        }
    }

    // Kerekek:
    if (elsokerekrogton) {
        kidobozkerek(p_k1, WheelBackgroundRenderRadius, pmot->left_wheel.rotation, ppic,
                     pmk->wheel);
    }
    if (hatsokerekrogton) {
        kidobozkerek(p_k2, WheelBackgroundRenderRadius, pmot->right_wheel.rotation, ppic,
                     pmk->wheel);
    }

    vect2 kozep = pmot->bike.r - sarok;
    vect2 jobbra = vect2(cos(pmot->bike.rotation), sin(pmot->bike.rotation));
    vect2 fel = rotate_90deg(jobbra);

    if (vanforgas) {
        StretchEnabled = true;
        set_stretch_parameters(kozep, jobbra, forgas, MetersToPixels);
    }

    if (pmot->flipped_bike) {
        jobbra = Vect2null - jobbra;
        vect2 tmpv = p_k1;
        p_k1 = p_k2;
        p_k2 = tmpv;
    }

    Mx = 390.0, My = 420.0;
    double malfa = 0.62;
    double mmeret = 0.0045;
    Mi = jobbra * (mmeret * cos(malfa)) + fel * (mmeret * sin(malfa));
    Mj = rotate_90deg(Mi);
    if (pmot->flipped_bike) {
        Mj = Vect2null - Mj;
    }
    Mr = kozep;
    // Most MetersToPixels-t betesszuk Mi, Mj, Mr-be:
    Miar = Mi * MetersToPixels;
    Mjar = Mj * MetersToPixels;
    Mrar = Mr * MetersToPixels;

    // Elso felfuggesztes:
    vect2 p_fogantyu = Mi * (365.0 - Mx) + Mj * (My - 292.0) + Mr; // Uj
    kidoboz(p_k1, p_fogantyu, ppic, 0.06, pmk->susp1, 0.05, 0.03);

    // Hatso felfuggesztes:
    vect2 p_darab2 = Mi * (370.0 - Mx) + Mj * (My - 520.0) + Mr;
    kidoboz(p_darab2, p_k2, ppic, 0.06, pmk->susp2, 0.0, 0.1);

    // Ha kell zaszlot rajzol:
    int flages = 0;
    if (!Single && Tag) {
        if ((ajatekos && FlagTagAHasFlag) || (!ajatekos && !FlagTagAHasFlag)) {
            flages = 1;
        }

        if (!flages && FlagTagImmunity) {
            int egeszido = (int)(t * 30.0);
            if (egeszido % 2) {
                flages = 1;
            }
        }
    }
    vect2 zaszlotalp = Mi * (500.0 + 107 - Mx) + Mj * (My + 114 - 600.0) + Mr;
    // 330 bal oldal (5)
    // 340 bal oldal (3)
    // 355 bal oldal (1.5)
    // 365 jobb oldal (1)
    // 361 jobb hajszalnyit
    vect2 zaszloteto = zaszlotalp + (Mi * 356.0 + Mj * 500.0) * 0.2;
    if (flages) {
        kidoboz(zaszlotalp, zaszloteto, ppic, 0.2, Lgr->flag, 0.0, 0.0, pmot->flipped_bike);
    }

    // Motor teste:
    kitag(ppic, pmk->bike_part1, &BikeBox1);
    kitag(ppic, pmk->bike_part2, &BikeBox2);
    kitag(ppic, pmk->bike_part3, &BikeBox3);
    kitag(ppic, pmk->bike_part4, &BikeBox4);

    // Mozgo test:
    vect2 p_vezeto = (pmot->body_r - sarok); // Vezeto
    vect2 p_csipo = p_vezeto + Mi * 75.0 + Mj * (-47.0);
    vect2 p_vall = p_vezeto + Mi * 47.0 + Mj * 65.0;
    vect2 p_valltest = p_vezeto + Mi * 41.0 + Mj * 70.0;
    vect2 p_labfej = Mi * (346.0 - Mx) + Mj * (My - 514.0) + Mr;
    vect2 p_terd;

    double combhossz = 0.51;
    double labszarhossz = 0.51;
    /* Bicigli lab hosszak:
    double combhossz = 0.56;
    double labszarhossz = 0.56;
    */

    if (pmot->flipped_bike) {
        p_terd = circles_intersection(p_csipo, p_labfej, combhossz, labszarhossz);
    } else {
        p_terd = circles_intersection(p_labfej, p_csipo, labszarhossz, combhossz);
    }

    // Fej:
    // double fejalfa = 0.20;
    double fejalfa = 0.0;
    if (pmot->flipped_bike) {
        fejalfa = 0 - fejalfa;
    }
    kidobozkerek(pmot->head_r - sarok, HeadRadius, pmot->bike.rotation + fejalfa, ppic, pmk->head,
                 pmot->flipped_bike);

    // Kar ket reszletben:
    vect2 p_kezfej = p_fogantyu;

    // Ugras belekombinalasa kar kirajzolasaba:
    // p_kezfej-et mozditjuk el szukseges mertekben:
    if (ugrasnagysag > 0.0001) {
        ugrasnagysag = 1.0 - ugrasnagysag;
        int fellendit = 1; // kar kiteresenek iranya
        if ((pvalt->ugras1volt && !pmot->flipped_bike) ||
            (!pvalt->ugras1volt && pmot->flipped_bike)) { // XOR
            fellendit = 0;
        }
        // alfa es hosszit kiszamolasa:
        double alfa = 0.0;
        double hosszit = 0.1;
        if (fellendit) {
            // KAR FELFELE LENDUL KI:
            double hatar = 0.25;
            double maxalfa = 2.7;
            double maxhosszit = -0.3;
            if (ugrasnagysag < hatar) {
                // Kilendit:
                alfa = maxalfa * ugrasnagysag / hatar;
                hosszit = maxhosszit * ugrasnagysag / hatar + 1.0;
            } else {
                // Visszaenged:
                double mertek = 1.0 - (ugrasnagysag - hatar) / (1.0 - hatar);
                alfa = maxalfa * mertek;
                hosszit = maxhosszit * mertek + 1.0;
            }
        } else {
            // KAR LEFELE LENDUL KI:
            double hatar = 0.2;
            double maxalfa = -1.6;
            double maxhosszit = 0.15;
            if (ugrasnagysag < hatar) {
                // Kilendit:
                alfa = maxalfa * ugrasnagysag / hatar;
                hosszit = maxhosszit * ugrasnagysag / hatar + 1.0;
            } else {
                // Visszaenged:
                double mertek = 1.0 - (ugrasnagysag - hatar) / (1.0 - hatar);
                alfa = maxalfa * mertek;
                hosszit = maxhosszit * mertek + 1.0;
            }
        }
        // alfa es hosszit alapjan p_kezfej forgatasa es nyujtasa vall korul:
        vect2 kar = p_kezfej - p_vall;
        if (!pmot->flipped_bike) {
            kar.rotate(-alfa);
        } else {
            kar.rotate(alfa);
        }
        kar = kar * hosszit;
        p_kezfej = p_vall + kar;
    }

    double alkarhossz = 0.308 * 1.05;
    double felkarhossz = 0.328 * 1.05;
    vect2 p_konyok;
    if (pmot->flipped_bike) {
        p_konyok = circles_intersection(p_kezfej, p_vall, alkarhossz, felkarhossz);
    } else {
        p_konyok = circles_intersection(p_vall, p_kezfej, felkarhossz, alkarhossz);
    }

    // Lab:
    kidoboz(p_terd, p_csipo, ppic, 0.14, pmk->thigh, 0.03, 0.1, pmot->flipped_bike);

    kidoboz(p_labfej, p_terd, ppic, 0.21, pmk->leg, 0.03, 0.03, pmot->flipped_bike);

    // Vezeto:
    affine_pic* body = shirt ? shirt : pmk->body;
    kidoboz(p_csipo, p_valltest, ppic, 0.2, body, 0.1, 0.05, pmot->flipped_bike);

    // Kez:
    kidoboz(p_konyok, p_vall, ppic, 0.11, pmk->up_arm, 0.08, 0.1, !pmot->flipped_bike);

    kidoboz(p_kezfej, p_konyok, ppic, 0.076, pmk->forarm, 0.08, 0.1, pmot->flipped_bike);

    StretchEnabled = false;

    // Kerekek:
    if (!elsokerekrogton || !hatsokerekrogton) {
        if (pmot->flipped_bike) {
            // Visszacserelem kerekek koordjait:
            vect2 tmpv = p_k1;
            p_k1 = p_k2;
            p_k2 = tmpv;
        }
        if (!elsokerekrogton) {
            kidobozkerek(p_k1, pmot->left_wheel.radius, pmot->left_wheel.rotation, ppic,
                         pmk->wheel);
        }
        if (!hatsokerekrogton) {
            kidobozkerek(p_k2, pmot->right_wheel.radius, pmot->right_wheel.rotation, ppic,
                         pmk->wheel);
        }
    }
}

static void kirakegyjatekost(int ajatekos, pic8* ppic, double t, motorst* pmot, valtozok* pvalt,
                             int viewki, int timeki, motorst* pmot2, valtozok* pvalt2,
                             camera& current_camera) {

    // Kiszamoljuk kepernyo bal also sarkat:
    vect2 motorkozep = pmot->bike.r;
    if (current_camera.mode == CameraMode::MapViewer) {
        motorkozep = vect2(current_camera.x, current_camera.y);
    }

    vect2 sarok(motorkozep.x - (Mo_bal + pvalt->baljobbv_h.baljobb * Mo_dx), motorkozep.y - Mo_y);

    // ppic->fill_box( 100 );
    Pecsetalso->kitesz(ajatekos, ppic, sarok, 0, 0, Cxsize - 1, Cysize - 1);

    // Objektumok kirajzolasa, de view-ba csak kesobb kerulnek:
    int balalsox, balalsoy;
    Pecsetalso->getbalalso_int(sarok, &balalsox, &balalsoy);
    int objminx = balalsox - (int)(ANIM_WIDTH * EolSettings->zoom()) - 2;
    int objminy = balalsoy - (int)(ANIM_WIDTH * EolSettings->zoom()) - 2;
    int objmaxx = balalsox + SCREEN_WIDTH;
    int objmaxy = balalsoy + SCREEN_HEIGHT;
    for (int i = 0; i < MAX_OBJECTS; i++) {
        object* pker = Ptop->objects[i];
        if (!pker) {
            break;
        }

        if (pker->type == object::Type::Start ||
            (pker->type == object::Type::Food && !pker->active) ||
            (!Single && Tag && pker->type == object::Type::Exit)) {
            continue;
        }

        if (pker->canvas_x < objminx || pker->canvas_y < objminy || pker->canvas_x > objmaxx ||
            pker->canvas_y > objmaxy) {
            continue;
        }

        pic8* pobjpic = nullptr;
        int dy = 0;
        if (State->animated_objects) {
            switch (pker->type) {
            case object::Type::Food:
                pobjpic = Lgr->food[pker->animation % Lgr->food_count]->get_frame_by_time(t);
                dy = (int)(5.0 * sin(t * 15.5 + pker->floating_phase));
                break;
            case object::Type::Exit:
                pobjpic = Lgr->exit->get_frame_by_time(t);
                dy = (int)(5.0 * sin(t * 15.5 + pker->floating_phase));
                break;
            case object::Type::Killer:
                pobjpic = Lgr->killer->get_frame_by_time(t);
                break;
            default:
                internal_error("HIBA6751353");
            }
        } else {
            // Elso frame-et adja vissza mindig:
            switch (pker->type) {
            case object::Type::Food:
                pobjpic = Lgr->food[pker->animation % Lgr->food_count]->get_frame_by_index(0);
                break;
            case object::Type::Exit:
                pobjpic = Lgr->exit->get_frame_by_index(0);
                break;
            case object::Type::Killer:
                pobjpic = Lgr->killer->get_frame_by_index(0);
                break;
            default:
                internal_error("6754783");
            }
        }

        blit8(ppic, pobjpic, pker->canvas_x - balalsox, pker->canvas_y - balalsoy + dy);
    }

    // Motorosok kirajzolasa:
    bike_pics* pmkepek1 = &Lgr->bike1;
    bike_pics* pmkepek2 = &Lgr->bike2;
    if ((State->player1_bike1 && !ajatekos) || (!State->player1_bike1 && ajatekos)) {
        pmkepek1 = &Lgr->bike2;
        pmkepek2 = &Lgr->bike1;
    }

    if (current_camera.mode == CameraMode::Normal) {
        if (!Single) {
            // Hatso motoros kirajzolasa:
            vect2 kozep(sarok.x + (SCREEN_WIDTH / 2.0) * PixelsToMeters,
                        sarok.y + (SCREEN_HEIGHT / 2.0) * PixelsToMeters);
            double tav = (pmot2->bike.r - kozep).length();
            if (tav < (std::max(SCREEN_WIDTH, SCREEN_HEIGHT) * 27.0 / 32.0) * PixelsToMeters) {
                kibike(!ajatekos, ppic, t, sarok, pmot2, pvalt2, pmkepek2, nullptr);
            }
        }

        // Motoros kirajzolasa:
        kibike(ajatekos, ppic, t, sarok, pmot, pvalt, pmkepek1, shirt);
    }

    if (!EolSettings->pictures_in_background()) {
        // Felso ecset kitevese:
        Pecsetfelso->kitesz(ajatekos, ppic, sarok, 0, 0, Cxsize - 1, Cysize - 1);
    }

    if (viewki) {
        if (Single) {
            kiview(ajatekos, ppic, pvalt->baljobbv_h.baljobb, motorkozep, NULL);
        } else {
            kiview(ajatekos, ppic, pvalt->baljobbv_h.baljobb, motorkozep, pmot2);
        }
    }

    double fogoido = -1.0;
    if (!Single) {
        if (ajatekos) {
            fogoido = FlagTimeA;
        } else {
            fogoido = FlagTimeB;
        }
    }

    // Idokiiras:
    if (timeki) {
        // Korny->legjobbido - volt eredetileg "" helyett:
        /*if( sordarabszam > 0 ) {
            char darabok[10];
            sprintf( darabok, "%d", sordarabszam );
            strrev( darabok );
            while( strlen( darabok ) < 6 )
                strcat( darabok, "0" );
            strrev( darabok );
            darabok[7] = darabok[5];
            darabok[6] = darabok[4];
            darabok[4] = darabok[3];
            darabok[3] = darabok[2];
            draw_timers( darabok, fogoido, t, ppic, Cxsize, Cysize );
        }
        else*/
        if (Tag) {
            draw_timers(BestTime, fogoido, t, ppic, Cxsize, Cysize);
        } else {
            draw_timers(BestTime, -1.0, t, ppic, Cxsize, Cysize);
        }
    }
}

static pic8* Bpic = NULL;

extern int Ucsosurlodas;

void kirajzol320(double t, valtozok* pvalt1, valtozok* pvalt2, int viewki1, int timeki1,
                 int viewki2, int timeki2, camera& current_camera) {

    int splitscreen = 0;
    int fulljatekosA = 1;

    if (!Single) {
        // Multiplayer modban:
        if (!pvalt1->showkep && !pvalt2->showkep) {
            internal_error("g45jnuhbvfr");
        }
        if (pvalt1->showkep && pvalt2->showkep) {
            splitscreen = 1;
        } else {
            // Csak egyik latszik:
            if (pvalt1->showkep) {
                fulljatekosA = 1;
            } else {
                fulljatekosA = 0;
            }
        }
        if (current_camera.mode == CameraMode::MapViewer) {
            splitscreen = 0;
            fulljatekosA = 1;
        }
    }

    pic8* npic = lockbackbuffer_pic();

    if (!Bpic) {
        Bpic = new pic8(10, SCREEN_HEIGHT);
    }

    // internal_error( "Itt van 11!" ); idejott

    if (Kitoltestmegrak > 0) {
        Kitoltestmegrak--;
        beallitmereteket(splitscreen);

        // internal_error( "Itt van 13!" ); idejott

        kitolthatteret(npic);
    }

    // internal_error( "Itt van 12!" ); ide nemjon

    if (splitscreen) {
        // 1. Jatekos:
        Bpic->subview(Cx1, Cy1, Cx2, Cy2, npic);

        kirakegyjatekost(1, Bpic, t, Motor1, pvalt1, viewki1, timeki1, Motor2, pvalt2,
                         current_camera);

        // 2. Jatekos:
        Bpic->subview(Cx1, Cy1Bplayer, Cx2, Cy2Bplayer, npic);

        kirakegyjatekost(0, Bpic, t, Motor2, pvalt2, viewki2, timeki2, Motor1, pvalt1,
                         current_camera);
    } else {
        Bpic->subview(Cx1, Cy1, Cx2, Cy2, npic);
        if (fulljatekosA) {
            kirakegyjatekost(1, Bpic, t, Motor1, pvalt1, viewki1, timeki1, Motor2, pvalt2,
                             current_camera);
        } else {
            kirakegyjatekost(0, Bpic, t, Motor2, pvalt2, viewki2, timeki2, Motor1, pvalt1,
                             current_camera);
        }
    }

    // Mentes lemezre:
    menteshakell(npic);

    // for( int i = 0; i < Ucsosurlodas; i++ )
    //	npic->ppixel( i, 100, 0 );

    unlockbackbuffer_pic();
}
