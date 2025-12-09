#include "all.h"

// Jatek modja ket fele lehet:
int Single = 1;
// Van-e fogocska:
int Tag = 0;
// int Multi = 0;
// int Match = 0;

int Legkozment = 0;

static int Kajakell;

struct viewtimest {
    int viewkinplay, viewkinreplay;
    int timekinplay, timekinreplay;
};

static void initsoundlejatszoban(void) {
    if (State->soundon && !Hangmarmegy) {
        Hangmarmegy = 1;
        Hangenabled = 1;

        Mute = 1;
        starthanghigh();
        void initdsound(int);
        initdsound(State->secondarybuffer);
    }
}

// Player 1 es 2-re:
static viewtimest Viewtime1 = {1, 0, 1, 0};
static viewtimest Viewtime2 = {1, 0, 1, 0};

// 0-meghalt, 1-megnyerte, 2-semmi kulonos:
static int spritefeldolgoz(int sorszam, int* pkajaszam, motorst* pmot) {
    if (sorszam < 0 || sorszam >= MAXKEREK) {
        hiba("spritefeldolgoz-ban sorszam < 0 || sorszam >= MAXKEREK!");
    }
    if (!Ptop->kerektomb[sorszam]) {
        hiba("spritefeldolgoz-ban !Ptop->kerektomb[sorszam]!");
    }

    int tipus = Ptop->kerektomb[sorszam]->tipus;

    if (tipus == T_HALALOS) {
        return 0;
    }
    if (tipus == T_KAJA) {
        Ptop->kerektomb[sorszam]->aktiv = 0;
        (*pkajaszam)++;
        startwavegyujto(WAV_EVES, 0.99, -1);
        switch (Ptop->kerektomb[sorszam]->kajatipus) {
        case KT_UP:
            pmot->gravirany = 0;
            break;
        case KT_DOWN:
            pmot->gravirany = 1;
            break;
        case KT_LEFT:
            pmot->gravirany = 2;
            break;
        case KT_RIGHT:
            pmot->gravirany = 3;
            break;
        }
        return 2;
    }
    if (tipus == T_CEL) {
        if (Pmot1->kajaszam + Pmot2->kajaszam >= Kajakell) {
            return 1;
        }
    }
    return 2;
}

static double Fordido = 0.5;
static double baljobbszamol(double eltelt, int hatra) {
#ifdef TEST
    if (eltelt < 0) {
        hiba("baljobbszamol-ban eltelt < 0!");
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
static void belsoresz(motorst* pmot, jatekosopciok* popciok, valtozok* pvalt, recorder* prec,
                      long* pmegvanido, int* pmeghalt, double eddig, double dt) {
    // Ugras elintezese:
    int ugrik1 = 0, ugrik2 = 0;
    if (eddig > pvalt->utolsougras + Ugroturelem) {
        if ((mk_getstate(popciok->billugras1) && Sajatugras) ||
            (mk_getstate(popciok->billugras2) && !Sajatugras)) {
            ugrik1 = 1;
            pvalt->utolsougras = eddig;
            pvalt->ugras1volt = 1;
            startwavegyujto(WAV_UGRAS1, 0.99, -1);
        }
        if ((mk_getstate(popciok->billugras2) && Sajatugras) ||
            (mk_getstate(popciok->billugras1) && !Sajatugras)) {
            ugrik2 = 1;
            pvalt->utolsougras = eddig;
            pvalt->ugras1volt = 0;
            startwavegyujto(WAV_UGRAS2, 0.99, -1);
        }
    }

    // LEPTET!!!!!!!:
    // 0-meghalt, 1-megnyerte, 2-semmi kulonos
    leptet(pmot, eddig, dt, mk_getstate(popciok->billgaz), mk_getstate(popciok->billfek), ugrik1,
           ugrik2);

    int eredmeny = vizsgalat(pmot);
    if (eredmeny == 0) {
        // Beteszunk egy 0 hangot:
        pvalt->inf.surlero = 0;
        pvalt->inf.frekvencia = -1;
        pvalt->inf.gaz = 0;
        prec->store(pmot, eddig, &pvalt->inf);
        if (!Single && Tag) {
            // Flag Tag modban nem allnak meg, hanem visszamennek starthelyre:
            void initmotor(motorst * pmot); // adatok.cpp-ben
            initmotor(pmot);
            // Kezdetihelydiff-et setallaktiv allitja be:
            pmot->kor1.r = pmot->kor1.r + Kezdetihelydiff;
            pmot->kor2.r = pmot->kor2.r + Kezdetihelydiff;
            pmot->kor4.r = pmot->kor4.r + Kezdetihelydiff;
            pmot->vezetor = pmot->vezetor + Kezdetihelydiff;

            // Valtozok inicializalasa:
            int showkep = pvalt->showkep;
            memset(pvalt, 0, sizeof(valtozok));
            pvalt->baljobbv_f.ucsoford = -1000.0;
            pvalt->baljobbv_f.ucsoforgas = -1000.0;
            pvalt->baljobbv_h.ucsoford = -1000.0;
            pvalt->baljobbv_h.ucsoforgas = -1000.0;
            pvalt->utolsougras = -100.0;
            pvalt->showkep = showkep;

            if ((pmot == Pmot1 && Aafogo) || (pmot == Pmot2 && !Aafogo)) {
                // Nala volt zaszlo
                Aafogo = !Aafogo;
            }
        } else {
            *pmeghalt = 1;
            return;
        }
    }

    // eddig += dt; // Ezt meg kell ismetelni eggyel feljebb is
    //  Berreges es nyikorgas:
    pvalt->inf.surlero = kiszamolsurlodast();

    if (pmot->hatra_f) {
        pvalt->inf.frekvencia = fabs(pmot->kor2.omega) * 0.025;
    } else {
        pvalt->inf.frekvencia = fabs(pmot->kor4.omega) * 0.025;
    }
    // Frekvenciat betesszuk 1.0-2.0 tartomanyba:
    if (pvalt->inf.frekvencia > 30.0) {
        pvalt->inf.frekvencia = 30.0;
    }
    pvalt->inf.frekvencia = 2.0 - exp(-pvalt->inf.frekvencia);

    pvalt->inf.gaz = (char)mk_getstate(popciok->billgaz);
    prec->store(pmot, eddig, &pvalt->inf);

    // Egyedi hangok:
    int wavazonosito;
    double hangero;
    int objszam;
    while (getwavegyujto(&wavazonosito, &hangero, &objszam)) {
        if (objszam >= 0) {
            int eredmeny = spritefeldolgoz(objszam, &pmot->kajaszam, pmot);
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

static void toggleresz(jatekosopciok* popciok, valtozok* pvalt, int* pviewkin, int* ptimekin,
                       int* pmasikshow) {
    // Showkep:
    if (!pvalt->showkepnyomva && mk_getstate(popciok->billshowkep)) {
        Kitoltestmegrak = Kitoltestmegrakkezd;
        if (!*pmasikshow) {
            // Ha masik jatekos ablaka nem latszott, mindegyik fog latszani:
            *pmasikshow = 1;
            pvalt->showkep = 1;
        } else {
            pvalt->showkep = !pvalt->showkep;
        }
    }
    pvalt->showkepnyomva = mk_getstate(popciok->billshowkep);
    // View ablak:
    if (!pvalt->viewnyomva && mk_getstate(popciok->billview)) {
        *pviewkin = !*pviewkin;
    }
    pvalt->viewnyomva = mk_getstate(popciok->billview);
    // Ido kint legyen vagy ne:
    if (!pvalt->timenyomva && mk_getstate(popciok->billtime)) {
        *ptimekin = !*ptimekin;
    }
    pvalt->timenyomva = mk_getstate(popciok->billtime);
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
#ifdef TEST
    if (pvalt->forgas < 0.0) {
        hiba("lejatszo-ban forgas < 0.0!");
    }
#endif
    if (pvalt->forgas > 1.0) {
        pvalt->forgas = 1.0;
    }

    pvalt->baljobb = baljobbszamol(eddig - pvalt->ucsoford, hatra);
}

static void kulsoresz(motorst* pmot, jatekosopciok* popciok, valtozok* pvalt, recorder* prec,
                      viewtimest* pvt, double eddig, int* pmasikshow, int meghalt) {
    toggleresz(popciok, pvalt, &pvt->viewkinplay, &pvt->timekinplay, pmasikshow);

    // Hatra fordulas elintezes:
    if (!meghalt) {
        if (!pvalt->hatranyomva && mk_getstate(popciok->billfordul)) {
            pmot->hatra_f = !pmot->hatra_f;
            szamitfejr(pmot);
        }
        pvalt->hatranyomva = mk_getstate(popciok->billfordul);
    }
    baljobbelintez(&pvalt->baljobbv_f, prec, eddig, pmot->hatra_f);

    // Hatra elintezes:
    if (pmot->gravirany == 0) { // UP
        pmot->hatra_h = !pmot->hatra_f;
    } else {
        pmot->hatra_h = pmot->hatra_f;
    }
    baljobbelintez(&pvalt->baljobbv_h, NULL, eddig, pmot->hatra_h);

    // Kirajzolashoz kell:
    pvalt->ugrasnagysag = 1.0 - (eddig - pvalt->utolsougras) / Ugroturelem;
    if (pvalt->ugrasnagysag < 0) {
        pvalt->ugrasnagysag = 0;
    }
#ifdef TEST
    if (pvalt->ugrasnagysag > 1.00001) {
        hiba("pvalt->ugrasnagysag > 1.00001!");
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
long lejatszo(char* filenev) {
    // hiba( "Ezegyhosszusor,szetkellvagnibiztosanhibaEzegy hosszu sor, szet kell vagni biztosan!",
    //	"Ez egy masik hosszu sor, szet kell vagni biztosan!" );

    // Azert allitom 1-be, hogy mindenkeppen eszrevegyem, ha nem allitom
    // meg kilepes elott be rendesen:
    MeghalteloszorAB = 1;
    Aerintetteviragot = 0;

    // Jatekmod eldontese:
    Single = State->single;
    Tag = State->tag;
    Prec1->settag(Tag);
    Prec2->settag(Tag);

    beallitlegjobbidot(filenev, Single);

    if (Single) {
        Multirec = 0;
    } else {
        Multirec = 1;
    }

    int palmegnincs = 1;
    initadatok();
    if (!Ptop) {
        hiba("lejatszo-ban !Ptop!");
    }
    Ptop->kereklefejjel();
    Ptop->kitoltiszineket();
    Ptop->killerekelore();
    // setallaktiv allitja be motor kezdeti helyzetet es fazisokat is!:
    Kajakell = Ptop->setallaktiv(Pmot1);
    Ptop->setallaktiv(Pmot2);

    // Eloszor keretet tobbszor kirakja kirajzol320:
    Kitoltestmegrak = Kitoltestmegrakkezd;

    double eddig = 0.0;
    resetwavegyujto();

    int plussznyomva = 0;
    int minusznyomva = 0;
    int snapnyomva = 0;
    mv_startstopper();

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
    resetleptet(Pmot1);
    resetleptet(Pmot2);

    initsoundlejatszoban();
    Mute = 0;
    if (Single) {
        startmotor(1);
    } else {
        startmotor(1);
        startmotor(0);
    }

    fogocskareset();

    Pmot1->kajaszam = Pmot2->kajaszam = 0;
    long l = 0;
    int mindjar = 1;
    while (1) {
        // Kiszamolja mennyit kell leptetni:
        double cel = mv_stopperido() * 0.0024;
        if (cel < 0.000001) {
            cel = 0.000001;
        }

        // Ha tul sok kimaradt, akkor nem kell annyit behozni:
        // Ezt most elvileg stopper vegzi:
        // if( eddig < cel-1.0 )
        //	eddig = cel-1.0;

        /* Most nincs idoellenorzes:
        if( cel - eddig > 2.0*0.432 && idoszerint ) // Maximum 2 sec a turelem!
            uzenet( "System not fast enough!" );
        */

        // mv_check( "Lej torlendo, ciklus kezdodik" );

        mv_check(/*"Lej 112"*/); // Billentyut itt olvassuk be
        int hangfuto = 1;
        while (eddig <= cel - 0.000001) {
            if ((hangfuto++) % 10 == 0) {
                hang(/*"hangfuto 10-nel tart"*/);
            }

            // char tmp[10];
            // sprintf( tmp, "Gaz kodja: %d", (int)State->opciok1.billgaz );
            // uzenet( tmp );
            // double dt = 0.003; // Ez it 1.2-es verzionak megfelelo
            // double dt = 0.006; // Ez itt uj lepeskoz (jol mukodik regen)

            // Ez ELASTO MANIA 1.0 lepeskoze:
            // double dt = 0.0055; // Ez leeseskor sem rezonal

            double dt = 0.0055; // 0.0065

            if (eddig + dt > cel) {
                dt = cel - eddig;
            }

            // if( eddig > 2.0 ) // kb. 5 masodperc
            //	semmi();

            if (!meghalt1) {
                belsoresz(Pmot1, &State->opciok1, &valt1, Prec1, &megvanido1, &meghalt1, eddig, dt);
            }

            if (!Single && !meghalt2) {
                belsoresz(Pmot2, &State->opciok2, &valt2, Prec2, &megvanido2, &meghalt2, eddig, dt);
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
                hangosdelay(Vegenvaras * 1000.0);

                stopmotor(1);
                stopmotor(0);

                Mute = 1;
                nullazhangot();

                ////////////////////////////////////////////
                // Motor hangjat kikapcsolja:
                /*
                stopmotor( 1 );
                stopmotor( 0 );

                //while( vanmegwav() )
                //	hang();

                Mute = 1;
                nullazhangot();
                */
                ////////////////////////////////////////////

                // while( vanmegwav() )
                //	hang( /*"Lej play 1"*/ );

                Mute = 1;
                nullazhangot();
                if (megvanido) {
                    Ptop->kerekjolalljon();
                    berakrecbehosszat(Prec1);
                    berakrecbehosszat(Prec2);
                    return megvanido;
                } else {
                    Ptop->kerekjolalljon();
                    berakrecbehosszat(Prec1);
                    berakrecbehosszat(Prec2);
                    return -1;
                }
            }

            eddig += dt;
        }

        // Fogocska elintezes:
        if (!Single && Tag) {
            fogocska(eddig);
        }

        if (Single) {
            setsurlodas(valt1.inf.surlero);
            setmotor(1, valt1.inf.frekvencia, valt1.inf.gaz);
        } else {
            setsurlodas(valt1.inf.surlero + valt2.inf.surlero);
            setmotor(1, valt1.inf.frekvencia, valt1.inf.gaz);
            setmotor(0, valt2.inf.frekvencia, valt2.inf.gaz);
        }

        kulsoresz(Pmot1, &State->opciok1, &valt1, Prec1, &Viewtime1, eddig, &valt2.showkep,
                  meghalt1);

        if (!Single) {
            kulsoresz(Pmot2, &State->opciok2, &valt2, Prec2, &Viewtime2, eddig, &valt1.showkep,
                      meghalt2);
        }

        kirajzol320(eddig, &valt1, &valt2, Viewtime1.viewkinplay, Viewtime1.timekinplay,
                    Viewtime2.viewkinplay, Viewtime2.timekinplay);

        if (palmegnincs) {
            palmegnincs = 0;
            Plgr->pal->set();
        }

        // Egy par kozos toggle:
        // Plusz elintezes:
        if (!plussznyomva && mk_getstate(State->billplussz)) {
            // extern int Pnyomva;
            // Pnyomva = 1;
            novelkepmeret();
        }
        plussznyomva = mk_getstate(State->billplussz);
        // Minusz elintezes:
        if (!minusznyomva && mk_getstate(State->billminusz)) {
            csokkentkepmeret();
        }
        minusznyomva = mk_getstate(State->billminusz);
        // Snap nyomas elintezese:
        if (!snapnyomva && mk_getstate(State->billsnap)) {
            Legkozment = 1;
        }
        snapnyomva = mk_getstate(State->billsnap);

        // Vizsgalat kilepesre:
        if (mk_getstate(MK_ESC)) {
            MeghalteloszorAB = 0;
            double eltelido = mv_stopperido();
            // Motor hangjat kikapcsolja:
            stopmotor(1);
            stopmotor(0);

            // while( vanmegwav() )
            //	hang();

            Mute = 1;

            nullazhangot();

            // Kiirja framerate-et:
            double tpl = double(l) / eltelido;
            tpl *= 182;
            int framerate = tpl;
            if (access("f_rate.inf", 0) == 0) {
                // igyhagyni fopen-t!:
                FILE* h = fopen("f_rate.inf", "wt");
                if (!h) {
                    hiba("framerate nem nyilik!");
                }
                fprintf(h, "%d", framerate);
                fclose(h);
            }
            Ptop->kerekjolalljon();
            berakrecbehosszat(Prec1);
            berakrecbehosszat(Prec2);
            Masodikmenet = 1;
            return -1;
        }
        l++;
        /*vekt2 tomba[100], tombb[100];
        double dtomb[100];
        for( int i = 0; i < 100; i++ ) {
            tomba[i] = vekt2( i+2, i+3 );
            tombb[i] = vekt2( i-23, i-70 );
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
static int replaymag(motorst* pmot, jatekosopciok* popciok, valtozok* pvalt, recorder* prec,
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
    baljobbelintez(&pvalt->baljobbv_f, NULL, eddig, pmot->hatra_f);

    if (pmot->gravirany == 0) { // UP
        pmot->hatra_h = !pmot->hatra_f;
    } else {
        pmot->hatra_h = pmot->hatra_f;
    }
    baljobbelintez(&pvalt->baljobbv_h, NULL, eddig, pmot->hatra_h);

    // Kirajzolashoz kellenek:
    pvalt->ugrasnagysag = 1.0 - (eddig - pvalt->utolsougras) / Ugroturelem;
    if (pvalt->ugrasnagysag < 0) {
        pvalt->ugrasnagysag = 0;
    }
#ifdef TEST
    if (pvalt->ugrasnagysag > 1.00001) {
        hiba("pvalt->ugrasnagysag > 1.00001!");
    }
#endif

    return visszaad;
}

static int Elozoshowkep1 = 1, Elozoshowkep2 = 1;

// Termeszetes befejezes eseten 0-t ad vissza:
long lejatszo_r(char* filenev, int showkepmarad) {
    Kitoltestmegrak = 3; // kirajz meg 3x kirakja keretet

    // Jatekmod eldontese:
    Single = !Multirec;
    Tag = Prec1->gettag();

    beallitlegjobbidot(filenev, Single);

    int palmegnincs = 1;
    initadatok();
    if (!Ptop) {
        hiba("lejatszo-ban !Ptop!");
    }
    Ptop->kereklefejjel();
    Ptop->kitoltiszineket();
    Ptop->killerekelore();
    // setallaktiv allitja be motor kezdeti helyzetet is!:
    Kajakell = Ptop->setallaktiv(Pmot1);
    Ptop->setallaktiv(Pmot2);

    // Eloszor keretet tobbszor is kirakja kirajzol320:
    Kitoltestmegrak = Kitoltestmegrakkezd;

    resetwavegyujto();
    mv_startstopper();

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

    resetleptet(Pmot1);
    resetleptet(Pmot2);

    initsoundlejatszoban();
    Mute = 0;
    if (Single) {
        startmotor(1);
    } else {
        startmotor(1);
        startmotor(0);
    }

    fogocskareset();

    // Pmot1->kajaszam = Pmot2->kajaszam = 0;
    long l = 0;
    int jar1 = 1, jar2 = 1;
    while (1) {
        mv_check(/*"Lej 534"*/); // Billentyut itt olvassuk be
        // Kiszamolja mennyit kell leptetni:
        double eddig = mv_stopperido() * 0.0024;
        // recall koordokat es Hatra-t is beallitja:
        int befejezte1 =
            !replaymag(Pmot1, &State->opciok1, &valt1, Prec1, eddig, &Viewtime1, &valt2.showkep);

        int befejezte2 = 0;
        if (!Single) {
            befejezte2 = !replaymag(Pmot2, &State->opciok2, &valt2, Prec2, eddig, &Viewtime2,
                                    &valt1.showkep);
        }

        if ((Single && befejezte1) || (!Single && befejezte1 && befejezte2)) {
            // Motor hangjat kikapcsolja:
            setmotor(1, 1.0, 0.0001);
            setmotor(0, 1.0, 0.0001);
            stopmotor(1);
            stopmotor(0);
            setsurlodas(0);
            hangosdelay(Vegenvaras * 400.0);

            // while( vanmegwav() )
            //	hang( /*"Lej replay 1"*/ );

            Mute = 1;
            nullazhangot();
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
            fogocska_r(eddig);
        }

        // HANG ELINTEZES:
        if (Single) {
            setmotor(1, valt1.inf.frekvencia, valt1.inf.gaz);
            // if( valt1.inf.frekvencia > 1.5 )
            //	hiba( "> 1.5!" );
            setsurlodas(valt1.inf.surlero);
        } else {
            setmotor(1, valt1.inf.frekvencia, valt1.inf.gaz);
            setmotor(0, valt2.inf.frekvencia, valt2.inf.gaz);
            setsurlodas(valt1.inf.surlero + valt2.inf.surlero);
        }

        // KIRAJZOLAS:
        kirajzol320(eddig, &valt1, &valt2, Viewtime1.viewkinreplay, Viewtime1.timekinreplay,
                    Viewtime2.viewkinreplay, Viewtime2.timekinreplay);

        if (palmegnincs) {
            palmegnincs = 0;
            Plgr->pal->set();
        }

        // Egy par kozos toggle:
        // Plusz elintezes:
        if (!plussznyomva && mk_getstate(State->billplussz)) {
            novelkepmeret();

            // extern int Mostirdki;
            // Mostirdki = 1;
        }
        plussznyomva = mk_getstate(State->billplussz);
        // Minusz elintezes:
        if (!minusznyomva && mk_getstate(State->billminusz)) {
            csokkentkepmeret();
        }
        minusznyomva = mk_getstate(State->billminusz);
        // Snap nyomas elintezese:
        if (!snapnyomva && mk_getstate(State->billsnap)) {
            Legkozment = 1;
        }
        snapnyomva = mk_getstate(State->billsnap);

        // Vizsgalat kilepesre:
        if (mk_getstate(MK_ESC)) {
            double eltelido = mv_stopperido();
            // Kiirja framerate-et:
            double tpl = double(l) / eltelido;
            tpl *= 182;
            int framerate = tpl;
            if (access("f_rate.inf", 0) == 0) {
                // igyhagyni fopen-t!:
                FILE* h = fopen("f_rate.inf", "wt");
                if (!h) {
                    hiba("framerate nem nyilik!");
                }
                fprintf(h, "%d", framerate);
                fclose(h);
            }
            // Hang kikapcsolas:
            setmotor(1, 1.0, 0.0001);
            setmotor(0, 1.0, 0.0001);
            stopmotor(1);
            stopmotor(0);
            setsurlodas(0);
            hangosdelay(Vegenvaras * 400.0);

            // while( vanmegwav() )
            //	hang( /*"Lej replay 1"*/ );

            Mute = 1;
            nullazhangot();

            Ptop->kerekjolalljon();

            Elozoshowkep1 = valt1.showkep;
            Elozoshowkep2 = valt2.showkep;
            return -1;
        }
        l++;
    }
}

/*if( Match ) {
    if( !meghalt1 )
        meghalt1 = elsomegoltemasodikat( Pmot2, Pmot1 );
    if( !meghalt2 )
        meghalt2 = elsomegoltemasodikat( Pmot1, Pmot2 );
    if( meghalt1 && meghalt2 )
        if( int(eddig*100.0) % 2 )
            meghalt1 = 0;
        else
            meghalt2 = 0;
    if( meghalt1 || meghalt2 ) {
        if( meghalt1 )
            dialog320( "ESC", "B jatekos gyozott!" );
        else
            dialog320( "ESC", "A jatekos gyozott!" );
        // Kilep:
        // Motor hangjat kikapcsolja:
        Mute = 1;
        nullazhangot();
        Ptop->kerekjolalljon();
        return -1;
    }
}*/
