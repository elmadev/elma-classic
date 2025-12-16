#include "ALL.H"
#include <algorithm>

// Ezek mindig beallitodnak Kepmeret-bol kirajzol320-ban:

int Kitoltestmegrak = 0;
int Kitoltestmegrakkezd = 10;

static double Kepmeret = 1.0;

static double Szorzo = 1.1;
void novelkepmeret(void) {
    Kepmeret *= Szorzo;
    if (Kepmeret > 1.0) {
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

void beallitmereteket(int splitscreen) {
    Cy1Bplayer = 100, Cy2Bplayer = 200;

    Cxsize = SCREEN_WIDTH * (Kepmeret * 1.001);
    Cysize = SCREEN_HEIGHT * (Kepmeret * 1.001);
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
#ifdef TEST
    if (Cx2 >= SCREEN_WIDTH) {
        hiba("beallitmereteket Cx2 >= SCREEN_WIDTH!");
    }
    if (Cy2 >= SCREEN_HEIGHT) {
        hiba("beallitmereteket Cy2 >= SCREEN_HEIGHT!");
    }
#endif

    // Ezek csak doboz kirajzolashoz kellenek:
    Hatarx1 = 2.0;
    Hatary1 = 2.0;
    Hatarx2 = Cxsize - 3.0;
    Hatary2 = Cysize - 3.0;

    // Motoros kepernyo mely reszen helyezkedik el:
    Mo_bal = (SCREEN_WIDTH / MetersToPixels) * 0.15;
    Mo_dx = Cxsize / MetersToPixels - 2.0 * Mo_bal;
    Mo_y = Cysize / MetersToPixels / 2.0;

    Viewxsize = 140 * sqrt(double(Cysize) / double(SCREEN_HEIGHT)); // 200
    Viewysize = 70 * sqrt(double(Cysize) / double(SCREEN_HEIGHT));  // 80
    Viewxorig = 40 * (Kepmeret - 0.6) / 0.4;
    Viewxtolas = Cxsize - 2 * Viewxorig - Viewxsize;
}

void forditkepet(pic8* ppic) {
    for (int y1 = 0; y1 < ppic->getysize() / 2; y1++) {
        int y2 = ppic->getysize() - 1 - y1;
        unsigned char* sora = ppic->getptr(y1);
        unsigned char* sorb = ppic->getptr(y2);
        int xsize = ppic->getxsize();
        for (int x = 0; x < xsize; x++) {
            unsigned char tmp = sora[x];
            sora[x] = sorb[x];
            sorb[x] = tmp;
        }
    }
}

// v1 iranyabol v2 fele interpolal:
/*static vekt2 interpolal( vekt2 v1, vekt2 v2, double x ) {
    vekt2 vissza = v1+(v2-v1)*x;
    return vissza;
} */

// Meg nem affin koordok:
static void kidoboz(vekt2 a, vekt2 b, pic8* ppic8, double szelesseg, kiskep* pk, double tulloga,
                    double tullogb, int tukor = 0) {
    vekt2 vegys = egys(b - a);
    b = b + vegys * tullogb;
    a = a - vegys * tulloga;
    vekt2 v = b - a;
    vekt2 v2fel;
    if (tukor) {
        v2fel = forgatas90fokkal(vegys) * szelesseg;
    } else {
        v2fel = forgatasminusz90fokkal(vegys) * szelesseg;
    }
    a = a - v2fel;

    a.x *= MetersToPixels;
    a.y *= MetersToPixels;
    v.x *= MetersToPixels;
    v.y *= MetersToPixels;
    v2fel.x *= MetersToPixels;
    v2fel.y *= MetersToPixels;
    dobozki(ppic8, pk, v, v2fel * 2.0, a);
}

// radius meg nincs megszorozva:
// Meg nem affin koordok:
static void kidobozkerek(vekt2 r, double radius, double rotation, pic8* ppicbuffer, kiskep* pk,
                         int tukor = 0) {
    vekt2 vfel(cos(rotation) * radius, sin(rotation) * radius);
    if (tukor) {
        kidoboz(r + vfel, r - vfel, ppicbuffer, radius, pk, 0.0, 0.0, tukor);
    } else {
        kidoboz(r - vfel, r + vfel, ppicbuffer, radius, pk, 0.0, 0.0, tukor);
    }
}

static double Mx = -1, My = -1;
static vekt2 Mi, Mj, Mr;
static vekt2 Miar, Mjar, Mrar; // Ezek meg vannak szorozva Aranyossaggal

static void kitag(pic8* ppic8, kiskep* pk, kisbox* pb) {
    vekt2 r = Miar * (pb->x1 + 260 - Mx) + Mjar * (My - (pb->y1 + 260)) + Mrar;
    vekt2 u = Miar * (pb->x2 - pb->x1);
    vekt2 v = Mjar * (pb->y1 - pb->y2);

    dobozki(ppic8, pk, u, v, r);
}

static void kigyuru(pic8* ppic8, int x, int y, unsigned char szin) {
#ifdef TEST
    if (x < 1 || x >= ppic8->getxsize() - 1 || y < 1 || y >= ppic8->getysize() - 1) {
        hiba("kigyuru-ben x, vagy y lelog!");
    }
#endif
    unsigned char* sor = ppic8->getptr(y - 1);
    sor[x - 1] = szin;
    sor[x] = szin;
    sor[x + 1] = szin;
    sor = ppic8->getptr(y + 1);
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
    int kerxsize = pfelho->getxsize();
    kerx %= kerxsize;
    kerx -= kerxsize; // Lehet, hogy enelkul is negativ?
    while( kerx < Displaysizex ) {
        int kerysize = pfelho->getysize();
        kery %= kerysize;
        kery -= kerysize; // Lehet, hogy enelkul is negativ?
        while( kery < Displaysizey ) {
            blt8( ppicbuffer, pfelho, kerx, kery );

            kery += kerysize;
        }
        kerx += kerxsize;
    }
} */

static void kiview(int ajatekos, pic8* ppic, double baljobb, vekt2 motorkozep, motorst* pmot2) {

    double Viewxsize_d = Viewxsize * MinimapScaleFactor * PixelsToMeters;
    double Viewysize_d = Viewysize * MinimapScaleFactor * PixelsToMeters;

    double szeltoltav = 0.2; // 20%
    double atfuthossz = 1.0 - 2.0 * szeltoltav;
    vekt2 sarokbolkozepre(Viewxsize_d * (szeltoltav + baljobb * atfuthossz), Viewysize_d / 2);
    vekt2 sarok = motorkozep - sarokbolkozepre;

    int vx1 = Viewxorig + baljobb * Viewxtolas;
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
    for (int i = 0; i < MAXKEREK; i++) {
        kerek* pker = Ptop->kerektomb[i];
        if (!pker) {
            break;
        }

        int kell1 = pker->tipus == T_KAJA && pker->aktiv;
        int kell2 = (Single || !Tag) && pker->tipus == T_CEL;

        if (!kell1 && !kell2) {
            continue;
        }

        if (pker->rxint_v < objminx || pker->ryint_v < objminy || pker->rxint_v > objmaxx ||
            pker->ryint_v > objmaxy) {
            continue;
        }

        // Potty kirajzolasa View ablakba:
        if (pker->tipus == T_KAJA) {
            kigyuru(ppic, pker->rxint_v - balalsox + vx1, pker->ryint_v - balalsoy + vy1,
                    Plgr->viewfood);
        } else {
            kigyuru(ppic, pker->rxint_v - balalsox + vx1, pker->ryint_v - balalsoy + vy1,
                    Plgr->viewexit[0]);
        }
    }

    // Most kirajzoljuk motorost:
    // Eldontjuk szineket:
    unsigned char viewindex1 = Plgr->viewmotorindex1;
    unsigned char viewindex2 = Plgr->viewmotorindex2;
    if ((State->aplayerabike && !ajatekos) || (!State->aplayerabike && ajatekos)) {
        viewindex1 = Plgr->viewmotorindex2;
        viewindex2 = Plgr->viewmotorindex1;
    }

    // Most kirajzoljuk masik (hatso) motorost:
    if (pmot2) {
        vekt2 sarokrel = pmot2->bike.r - sarok;
        int mx = sarokrel.x * MetersToPixels / MinimapScaleFactor + vx1;
        int my = sarokrel.y * MetersToPixels / MinimapScaleFactor + vy1;
        if (mx >= vx1 && mx <= vx2 && my >= vy1 && my <= vy2) {
            kigyuru(ppic, mx, my, viewindex2);
        }
    }

    // Most kirajzoljuk elso motorost:
    int mx = sarokbolkozepre.x * MetersToPixels / MinimapScaleFactor;
    int my = sarokbolkozepre.y * MetersToPixels / MinimapScaleFactor;
    kigyuru(ppic, vx1 + mx, vy1 + my, viewindex1);

    // Kirajzoljuk keretet:
    vx1--;
    vy1--;
    vx2++;
    vy2++;
    for (int y = vy1; y <= vy2; y++) {
        ppic->ppixel(vx1, y, Plgr->keretindex);
        ppic->ppixel(vx2, y, Plgr->keretindex);
    }
    unsigned char* sor = ppic->getptr(vy1) + vx1 + 1;
    memset(sor, Plgr->keretindex, Viewxsize);
    sor = ppic->getptr(vy2) + vx1 + 1;
    memset(sor, Plgr->keretindex, Viewxsize);
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
                ppic->save(filenev, Plgr->paltomb);
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
            // hiba( "Itt van 14!" ); idejott
            // hiba( tmp ); (16) kiirja koordokat szepen

            // extern int Debugblt;
            // Debugblt = 1;
            blt8(ppic, Plgr->pkimarad, x, y);
            // Debugblt = 0;
            //  hiba( "Itt van 20!" ); idejott
            x += Plgr->pkimarad->getxsize();

            // hiba( tmp ); (21) kiirja x-et 207-nek
        }
        // hiba( "Itt van 15!" );
        y += Plgr->pkimarad->getysize();
    }
}

#ifdef TEST
int Voltkilogas = 0;
#endif

static void kibike(int ajatekos, pic8* ppic, double t, vekt2 sarok, motorst* pmot, valtozok* pvalt,
                   motkepek* pmk) {

    // Mivel ezeket fv megvaltoztatja:
    double ugrasnagysag = pvalt->ugrasnagysag;
    double forgas = pvalt->baljobbv_f.forgas;

    // if( pvalt->baljobb < 0 || pvalt->baljobb > 1.0 )
    //     hiba( "pvalt->baljobb < 0 || pvalt->baljobb > 1.0!" );
    // if( forgas < 0.0 || forgas > 1.0 )
    //     hiba( "forgas < 0.0 || forgas > 1.0!" );

    // Ha kell affinitassal torodni:
    int vanforgas = 0;
    Affinitas = 0;
    if (forgas < 0.999) {
        vanforgas = 1;
        // forgast 0, 1 intervallumbol atkonvertaljuk -1, 1 intervallumba:
        forgas = -cos(forgas * K_pi);
    }

    vekt2 p_k1 = (pmot->left_wheel.r - sarok);
    vekt2 p_k2 = (pmot->right_wheel.r - sarok);

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
                     pmk->pkiskerek);
    }
    if (hatsokerekrogton) {
        kidobozkerek(p_k2, WheelBackgroundRenderRadius, pmot->right_wheel.rotation, ppic,
                     pmk->pkiskerek);
    }

    vekt2 kozep = pmot->bike.r - sarok;
    vekt2 jobbra = vekt2(cos(pmot->bike.rotation), sin(pmot->bike.rotation));
    vekt2 fel = forgatas90fokkal(jobbra);

    if (vanforgas) {
        Affinitas = 1;
        setaffinitas(kozep, jobbra, forgas, MetersToPixels, MetersToPixels);
    }

    if (pmot->flipped_bike) {
        jobbra = Vekt2null - jobbra;
        vekt2 tmpv = p_k1;
        p_k1 = p_k2;
        p_k2 = tmpv;
    }

    Mx = 390.0, My = 420.0;
    double malfa = 0.62;
    double mmeret = 0.0045;
    Mi = jobbra * (mmeret * cos(malfa)) + fel * (mmeret * sin(malfa));
    Mj = forgatas90fokkal(Mi);
    if (pmot->flipped_bike) {
        Mj = Vekt2null - Mj;
    }
    Mr = kozep;
    // Most MetersToPixels-t betesszuk Mi, Mj, Mr-be:
    Miar = Mi * MetersToPixels;
    Mjar = Mj * MetersToPixels;
    Mrar = Mr * MetersToPixels;

    // Elso felfuggesztes:
    vekt2 p_fogantyu = Mi * (365.0 - Mx) + Mj * (My - 292.0) + Mr; // Uj
    kidoboz(p_k1, p_fogantyu, ppic, 0.06, pmk->pkiselsorud, 0.05, 0.03);

    // Hatso felfuggesztes:
    vekt2 p_darab2 = Mi * (370.0 - Mx) + Mj * (My - 520.0) + Mr;
    kidoboz(p_darab2, p_k2, ppic, 0.06, pmk->pkishatsorud, 0.0, 0.1);

    // Ha kell zaszlot rajzol:
    int flages = 0;
    if (!Single && Tag) {
        if ((ajatekos && Aafogo) || (!ajatekos && !Aafogo)) {
            flages = 1;
        }

        if (!flages && Kozelvannak) {
            int egeszido = t * 30.0;
            if (egeszido % 2) {
                flages = 1;
            }
        }
    }
    vekt2 zaszlotalp = Mi * (500.0 + 107 - Mx) + Mj * (My + 114 - 600.0) + Mr;
    // 330 bal oldal (5)
    // 340 bal oldal (3)
    // 355 bal oldal (1.5)
    // 365 jobb oldal (1)
    // 361 jobb hajszalnyit
    vekt2 zaszloteto = zaszlotalp + (Mi * 356.0 + Mj * 500.0) * 0.2;
    if (flages) {
        kidoboz(zaszlotalp, zaszloteto, ppic, 0.2, Plgr->pkiszaszlo, 0.0, 0.0, pmot->flipped_bike);
    }

    // Motor teste:
    kitag(ppic, pmk->pkisa, &KisboxA);
    kitag(ppic, pmk->pkisb, &KisboxB);
    kitag(ppic, pmk->pkisc, &KisboxC);
    kitag(ppic, pmk->pkisd, &KisboxD);

    // Mozgo test:
    vekt2 p_vezeto = (pmot->body_r - sarok); // Vezeto
    vekt2 p_csipo = p_vezeto + Mi * 75.0 + Mj * (-47.0);
    vekt2 p_vall = p_vezeto + Mi * 47.0 + Mj * 65.0;
    vekt2 p_valltest = p_vezeto + Mi * 41.0 + Mj * 70.0;
    vekt2 p_labfej = Mi * (346.0 - Mx) + Mj * (My - 514.0) + Mr;
    vekt2 p_terd;

    double combhossz = 0.51;
    double labszarhossz = 0.51;
    /* Bicigli lab hosszak:
    double combhossz = 0.56;
    double labszarhossz = 0.56;
    */

    if (pmot->flipped_bike) {
        p_terd = ketkormetszete(p_csipo, p_labfej, combhossz, labszarhossz);
    } else {
        p_terd = ketkormetszete(p_labfej, p_csipo, labszarhossz, combhossz);
    }

    // Fej:
    // double fejalfa = 0.20;
    double fejalfa = 0.0;
    if (pmot->flipped_bike) {
        fejalfa = 0 - fejalfa;
    }
    kidobozkerek(pmot->head_r - sarok, HeadRadius, pmot->bike.rotation + fejalfa, ppic,
                 pmk->pkisfej, pmot->flipped_bike);

    // Kar ket reszletben:
    vekt2 p_kezfej = p_fogantyu;

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
        vekt2 kar = p_kezfej - p_vall;
        if (!pmot->flipped_bike) {
            kar.forgatas(-alfa);
        } else {
            kar.forgatas(alfa);
        }
        kar = kar * hosszit;
        p_kezfej = p_vall + kar;
    }

    double alkarhossz = 0.308 * 1.05;
    double felkarhossz = 0.328 * 1.05;
    vekt2 p_konyok;
    if (pmot->flipped_bike) {
        p_konyok = ketkormetszete(p_kezfej, p_vall, alkarhossz, felkarhossz);
    } else {
        p_konyok = ketkormetszete(p_vall, p_kezfej, felkarhossz, alkarhossz);
    }

    // Lab:
    kidoboz(p_terd, p_csipo, ppic, 0.14, pmk->pkiscomb, 0.03, 0.1, pmot->flipped_bike);

    kidoboz(p_labfej, p_terd, ppic, 0.21, pmk->pkislabszar, 0.03, 0.03, pmot->flipped_bike);

    // Vezeto:
    kidoboz(p_csipo, p_valltest, ppic, 0.2, pmk->pkisvezeto, 0.1, 0.05, pmot->flipped_bike);

    // Kez:
    kidoboz(p_konyok, p_vall, ppic, 0.11, pmk->pkisfelkar, 0.08, 0.1, !pmot->flipped_bike);

    kidoboz(p_kezfej, p_konyok, ppic, 0.076, pmk->pkisalkar, 0.08, 0.1, pmot->flipped_bike);

    Affinitas = 0;

    // Kerekek:
    if (!elsokerekrogton || !hatsokerekrogton) {
        if (pmot->flipped_bike) {
            // Visszacserelem kerekek koordjait:
            vekt2 tmpv = p_k1;
            p_k1 = p_k2;
            p_k2 = tmpv;
        }
        if (!elsokerekrogton) {
            kidobozkerek(p_k1, pmot->left_wheel.radius, pmot->left_wheel.rotation, ppic,
                         pmk->pkiskerek);
        }
        if (!hatsokerekrogton) {
            kidobozkerek(p_k2, pmot->right_wheel.radius, pmot->right_wheel.rotation, ppic,
                         pmk->pkiskerek);
        }
    }
}

static void kirakegyjatekost(int ajatekos, pic8* ppic, double t, motorst* pmot, valtozok* pvalt,
                             int viewki, int timeki, motorst* pmot2, valtozok* pvalt2) {

    // Kiszamoljuk kepernyo bal also sarkat:
    vekt2 motorkozep = pmot->bike.r;

    vekt2 sarok(motorkozep.x - (Mo_bal + pvalt->baljobbv_h.baljobb * Mo_dx), motorkozep.y - Mo_y);

    // ppic->fillbox( 100 );
    Pecsetalso->kitesz(ajatekos, ppic, sarok, 0, 0, Cxsize - 1, Cysize - 1);

    hang(/*"Ecsetalso utan"*/);

    // Objektumok kirajzolasa, de view-ba csak kesobb kerulnek:
    int balalsox, balalsoy;
    Pecsetalso->getbalalso_int(sarok, &balalsox, &balalsoy);
    int objminx = balalsox - ANIM_WIDTH - 2;
    int objminy = balalsoy - ANIM_WIDTH - 2;
    int objmaxx = balalsox + SCREEN_WIDTH;
    int objmaxy = balalsoy + SCREEN_HEIGHT;
    for (int i = 0; i < MAXKEREK; i++) {
        kerek* pker = Ptop->kerektomb[i];
        if (!pker) {
            break;
        }

        if (pker->tipus == T_KEZDO || (pker->tipus == T_KAJA && !pker->aktiv) ||
            (!Single && Tag && pker->tipus == T_CEL)) {
            continue;
        }

        if (pker->rxint < objminx || pker->ryint < objminy || pker->rxint > objmaxx ||
            pker->ryint > objmaxy) {
            continue;
        }

        pic8* pobjpic;
        int dy = 0;
        if (State->animatedobjects) {
            switch (pker->tipus) {
            case T_KAJA:
                pobjpic = Plgr->foodtomb[pker->foodsorszam % Plgr->foodszam]->get_frame_by_time(t);
                dy = 5 * sin(t * 15.5 + pker->sinfazis);
                break;
            case T_CEL:
                pobjpic = Plgr->pexit->get_frame_by_time(t);
                dy = 5 * sin(t * 15.5 + pker->sinfazis);
                break;
            case T_HALALOS:
                pobjpic = Plgr->pkiller->get_frame_by_time(t);
                break;
            default:
                hiba("HIBA6751353");
            }
        } else {
            // Elso frame-et adja vissza mindig:
            switch (pker->tipus) {
            case T_KAJA:
                pobjpic = Plgr->foodtomb[pker->foodsorszam % Plgr->foodszam]->get_frame_by_index(0);
                break;
            case T_CEL:
                pobjpic = Plgr->pexit->get_frame_by_index(0);
                break;
            case T_HALALOS:
                pobjpic = Plgr->pkiller->get_frame_by_index(0);
                break;
            default:
                hiba("6754783");
            }
        }

        blt8(ppic, pobjpic, pker->rxint - balalsox, pker->ryint - balalsoy + dy);
    }

    // Motorosok kirajzolasa:
    motkepek* pmkepek1 = &Plgr->mkepek1;
    motkepek* pmkepek2 = &Plgr->mkepek2;
    if ((State->aplayerabike && !ajatekos) || (!State->aplayerabike && ajatekos)) {
        pmkepek1 = &Plgr->mkepek2;
        pmkepek2 = &Plgr->mkepek1;
    }

    if (!Single) {
        // Hatso motoros kirajzolasa:
        vekt2 kozep(sarok.x + (SCREEN_WIDTH / 2.0) * PixelsToMeters,
                    sarok.y + (SCREEN_HEIGHT / 2.0) * PixelsToMeters);
        double tav = abs(pmot2->bike.r - kozep);
        if (tav < (std::max(SCREEN_WIDTH, SCREEN_HEIGHT) * 27.0 / 32.0) * PixelsToMeters) {
            kibike(!ajatekos, ppic, t, sarok, pmot2, pvalt2, pmkepek2);
        }
    }

    // Motoros kirajzolasa:
    kibike(ajatekos, ppic, t, sarok, pmot, pvalt, pmkepek1);

    // Felso ecset kitevese:
    Pecsetfelso->kitesz(ajatekos, ppic, sarok, 0, 0, Cxsize - 1, Cysize - 1);

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
            fogoido = Afogoido;
        } else {
            fogoido = Bfogoido;
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
            kidigit( darabok, fogoido, t, ppic, Cxsize, Cysize );
        }
        else*/
        if (Tag) {
            kidigit(Ezenlegjobbido, fogoido, t, ppic, Cxsize, Cysize);
        } else {
            kidigit(Ezenlegjobbido, -1.0, t, ppic, Cxsize, Cysize);
        }
    }
}

static pic8* Bpic = NULL;

extern int Ucsosurlodas;

void kirajzol320(double t, valtozok* pvalt1, valtozok* pvalt2, int viewki1, int timeki1,
                 int viewki2, int timeki2) {

    int splitscreen = 0;
    int fulljatekosA = 1;

    if (!Single) {
        // Multiplayer modban:
        if (!pvalt1->showkep && !pvalt2->showkep) {
            hiba("g45jnuhbvfr");
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
    }

    Locky0_alul = 1;
    pic8* npic = lockbackbuffer_pic();
    Locky0_alul = 0;

    if (!Bpic) {
        Bpic = new pic8(10, SCREEN_HEIGHT);
    }

    // hiba( "Itt van 11!" ); idejott

    if (Kitoltestmegrak > 0) {
        Kitoltestmegrak--;
        beallitmereteket(splitscreen);

        // hiba( "Itt van 13!" ); idejott

        kitolthatteret(npic);
    }

    // hiba( "Itt van 12!" ); ide nemjon

    if (splitscreen) {
        // 1. Jatekos:
        Bpic->keszitbelsot(Cx1, Cy1, Cx2, Cy2, npic);

        kirakegyjatekost(1, Bpic, t, Motor1, pvalt1, viewki1, timeki1, Motor2, pvalt2);

        // 2. Jatekos:
        Bpic->keszitbelsot(Cx1, Cy1Bplayer, Cx2, Cy2Bplayer, npic);

        kirakegyjatekost(0, Bpic, t, Motor2, pvalt2, viewki2, timeki2, Motor1, pvalt1);
    } else {
        Bpic->keszitbelsot(Cx1, Cy1, Cx2, Cy2, npic);
        if (fulljatekosA) {
            kirakegyjatekost(1, Bpic, t, Motor1, pvalt1, viewki1, timeki1, Motor2, pvalt2);
        } else {
            kirakegyjatekost(0, Bpic, t, Motor2, pvalt2, viewki2, timeki2, Motor1, pvalt1);
        }
    }

    // Mentes lemezre:
    menteshakell(npic);

    // for( int i = 0; i < Ucsosurlodas; i++ )
    //	npic->ppixel( i, 100, 0 );

    hang(/*"unlock elott"*/);

    Locky0_alul = 1;
    unlockbackbuffer_pic();
    Locky0_alul = 0;
}
