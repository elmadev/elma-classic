#include "all.h"

state_s* State = NULL;

void cheatplayer(void) {
    // can't have any players
    if (State->jatekosokszama != 0) {
        hiba("Already has player in cheatplayer!");
    }

    State->jatekosokszama = 1;

    int hanypalya = Palyaszam;
    // hanypalya = 40;

    jatekos* pjat = &State->jatekosok[0];
    pjat->sikerespalyakszama = hanypalya;
    strcpy(pjat->nev, "Cheat");
    pjat->jelenlegipalya = 0;

    for (int i = 0; i < hanypalya; i++) {
        palyaegyfeleidok* pidok = &State->palyakidejei[i].singleidok;
        if (pidok->idokszama != 0) {
            hiba("Bug in cheatplayer!");
        }
        pidok->idokszama = 1;
        pidok->idok[0] = 180000l;
        strcpy(pidok->nevek1[0], "Cheat");
    }
}

/*static void kiraklogot( void ) {
    szoveglista szl;

    mk_emptychar();
    while( 1 ) {
        if( mk_kbhit() ) {
            int c = mk_getextchar();
            //if( c == MK_ESC || c == MK_ENTER )
                return;
        }
        szl.kirajzol( 1, 1 );
    }
} */

static void mergestatedat(void);

void teljes(void) {
    State = new state_s;
    if (!State) {
        uzenet("memory");
    }

    mergestatedat();

    initadatok();

    // State->save(); // can make empty state.dat here

    // can make cheat player here
    // first clear state.dat
    // cheatplayer();

    // put tile on picture
    /*pic8* tmpintro = new pic8( 640, 480 );
    pic8* tile = new pic8( "introtl.pcx" );
    int y = 0;
    while( y < 480 ) {
        int x = 0;
        while( x < 640 ) {
            blt8( tmpintro, tile, x, y );
            x += tile->getxsize();
        }
        y += tile->getysize();
    }
    pic8* pintro = new pic8( "intro.pcx" );
    spriteosit( pintro );
    blt8( tmpintro, pintro );
    delete pintro; pintro = NULL;
    delete tile; tile = NULL;*/

    // will be freed by first kiirloading (defined in menukep):
    Pintro = new pic8("intro.pcx");
    // reset version to 1.1
    // blt8( Pintro, Pintro, 343, 433, 520, 459, 530, 468 ); // a betu
    blt8(Pintro, Pintro, 321, 420, 297, 420, 315, 441);
    blt8(Pintro, Pintro, 321 + 15, 420, 297, 420, 315, 441);
    blt8(Pintro, Pintro, 321 + 15 + 16, 432, 88, 458, 98, 468);
    spriteosit(Pintro);
    pic8* tmpintro = new pic8(640, 480); // will be freed later
    tmpintro->fillbox(254);
    blt8(tmpintro, Pintro);

    ddpal* pintropal = NULL;
    // slop, with this one name pcxtopal(pic.cpp) first 3 bytes are zeroed
    pcxtopal("intro.pcx", &pintropal);

    // moved back for consistent output because of alt-tab
    pintropal->set();
    bltfront(tmpintro);
    delete pintropal;
    pintropal = NULL;

    // a few reads
    Pabc1 = new abc8("kisbetu1.abc");
    Pabc1->settav(1);
    Pabc2 = new abc8("kisbetu2.abc");
    Pabc2->settav(1);

    // void menuproba( void );
    // menuproba();

    Prec1 = new recorder;
    Prec2 = new recorder;

    seteditorpal();

    // mouse:
    int mmx = 355;
    int mmy = 265;
    Moux = mmx;
    Mouy = mmy;
    setmou(mmx, mmy);
    Moux = mmx;
    Mouy = mmy;

    // always have timer
    mv_startstopper();

    // wait with intro screen until everything is loaded
    while (1) {
        if (mk_kbhit()) {
            mk_getextchar();
            break;
        }
        bltfront(tmpintro);
    }
    delete tmpintro;
    tmpintro = NULL;

    /*delete tmpintro;
    tmpintro = new pic8( "torol.pcx" );
    mk_getextchar();
    for( int i = 1; i < 480; i += 5  ) {
        pbuffer->fillbox( 254 );
        //blt8( pbuffer, tmpintro, 0, -240+i, 0, 0, 639, 239 );
        //blt8( pbuffer, tmpintro, 0, 479-i, 0, 240, 639, 479 );
        blt8( pbuffer, tmpintro, 0, -480+i );
        blt8( pbuffer, pintro );
        bltfront( pbuffer );
    }
    for( i = 0; i < 480; i += 5  ) {
        blt8( pbuffer, tmpintro );
        //blt8( pbuffer, pintro1, i, 0 );
        //blt8( pbuffer, pintro2, -i, 204 );
        blt8( pbuffer, pintro, 0, i );
        bltfront( pbuffer );
    } */

    Pmenupal->set();

    // needed for valaszt2
    initrubrikak();

    // kiraklogot();

    if (State->jatekosokszama == 0) {
        newjatekos(1, 0); // won't return from here
    } else {
        jatekosvalasztas(1, 0); // won't return from here
    }
    hiba("Should not be here!");
}

/* old exit
void kilep( void ) {
    pic8* ppic = new pic8( "vegekep.pcx" );
    ddpal* pvegepal = NULL;
    pcxtopal( "vegekep.pcx", &pvegepal );
    pvegepal->set();
    bltfront( ppic );
    while( 1 ) {
        int c = mk_getextchar();
        if( c == ' ' || c == MK_ENTER || c == MK_ESC ) {
            mv_exit();
        }
    }
} */

void kilep(void) {
    Nincsfal = 1; // balls will go outside of screen
    szoveglista* pszovlist = new szoveglista;
    pszovlist->addszoveg_kozep("Thank you for registering the game!", 320, 220);
    pszovlist->addszoveg_kozep("Please do not distribute!", 320, 300);

    double kezdoido = mv_stopperido();
    while (1) {
        pszovlist->kirajzol();
        double idovaras = 50.0;
        if ((mv_stopperido() > (kezdoido + idovaras)) && mk_kbhit()) {
            int c = mk_getextchar();
            if (c == ' ' || c == MK_ENTER || c == MK_ESC) {
                mv_exit();
            }
        }
    }
}

static void mergeegyfele(palyaegyfeleidok* pst, palyaegyfeleidok* pmrg, int single) {
    int idokszama = pst->idokszama + pmrg->idokszama;
    int idok[MAXIDOK * 2];
    int ujetomb[MAXIDOK * 2]; // true if time is from merge file
    jatekosnev_type nevek1[MAXIDOK * 2];
    jatekosnev_type nevek2[MAXIDOK * 2];
    // fill table
    for (int i = 0; i < pst->idokszama; i++) {
        idok[i] = pst->idok[i];
        ujetomb[i] = 0;
        strcpy(nevek1[i], pst->nevek1[i]);
        if (single) {
            *(nevek2[i]) = 0;
        } else {
            strcpy(nevek2[i], pst->nevek2[i]);
        }

        // set all VJ32 times to 50 mins
        // if( strcmp( nevek1[i], "VJ32" ) != 0 && strcmp( nevek2[i], "VJ32" ) )
        //	idok[i] = 300000;
    }
    for (int i = 0; i < pmrg->idokszama; i++) {
        int idest = pst->idokszama + i;
        idok[idest] = pmrg->idok[i];
        ujetomb[idest] = 1;
        strcpy(nevek1[idest], pmrg->nevek1[i]);
        if (single) {
            *(nevek2[idest]) = 0;
        } else {
            strcpy(nevek2[idest], pmrg->nevek2[i]);
        }
    }
    // sort times
    for (int i = 0; i < idokszama + 4; i++) {
        for (int j = 0; j < idokszama - 1; j++) {
            // check if switch is needed
            // priority order: time, first name, second name, 
            // and whether time is from merge.dat

            int cserelendo = 0;
            if (idok[j] > idok[j + 1]) {
                cserelendo = 1;
            }

            if (idok[j] == idok[j + 1]) {
                // same time:
                if (strcmp(nevek1[j], nevek1[j + 1]) == 0) {
                    if (strcmp(nevek2[j], nevek2[j + 1]) == 0) {
                        if (ujetomb[j] && !ujetomb[j + 1]) {
                            cserelendo = 1;
                        }
                    } else {
                        // same up to second name
                        if (strcmp(nevek2[j], nevek2[j + 1]) > 0) {
                            cserelendo = 1;
                        }
                    }
                } else {
                    // same time but not first name
                    if (strcmp(nevek1[j], nevek1[j + 1]) > 0) {
                        cserelendo = 1;
                    }
                }
            }

            if (cserelendo) {
                // switch
                int tmp = idok[j];
                idok[j] = idok[j + 1];
                idok[j + 1] = tmp;

                tmp = ujetomb[j];
                ujetomb[j] = ujetomb[j + 1];
                ujetomb[j + 1] = tmp;

                jatekosnev_type tmpnev;
                strcpy(tmpnev, nevek1[j]);
                strcpy(nevek1[j], nevek1[j + 1]);
                strcpy(nevek1[j + 1], tmpnev);

                strcpy(tmpnev, nevek2[j]);
                strcpy(nevek2[j], nevek2[j + 1]);
                strcpy(nevek2[j + 1], tmpnev);
            }
        }
    }

    // delete same times from matching player:
    // (new times only):
    for (int i = 0; i < idokszama + 2; i++) {
        for (int j = 0; j < idokszama - 1; j++) {
            if (idok[j] == idok[j + 1] && strcmp(nevek1[j], nevek1[j + 1]) == 0 &&
                (single || strcmp(nevek2[j], nevek2[j + 1]) == 0) && ujetomb[j + 1]) {

                // delete j+1-th
                for (int k = j + 1; k < idokszama - 1; k++) {
                    // put k-th in k+1-th
                    idok[k] = idok[k + 1];
                    ujetomb[k] = ujetomb[k + 1];
                    strcpy(nevek1[k], nevek1[k + 1]);
                    strcpy(nevek2[k], nevek2[k + 1]);
                }
                idokszama--;
                break; // security measure
            }
        }
    }

    // reset in state.dat
    if (idokszama > MAXIDOK) {
        idokszama = MAXIDOK;
    }
    pst->idokszama = idokszama;

    // security zeroing
    for (int i = 0; i < MAXIDOK; i++) {
        pst->idok[i] = -1;
        *(pst->nevek1[i]) = 0;
        *(pst->nevek2[i]) = 0;
    }
    // copy
    for (int i = 0; i < idokszama; i++) {
        pst->idok[i] = idok[i];
        strcpy(pst->nevek1[i], nevek1[i]);
        strcpy(pst->nevek2[i], nevek2[i]);
    }
}

static void mergestatedat(void) {
    char nev[20] = "merge.dat";
    if (access(nev, 0) != 0) {
        return; // no merge file

    state_s* pmerge = new state_s(nev);
    if (!pmerge) {
        uzenet("memory");
    }

    State->reloadidok();

    // merge besttime tables
    for (int i = 0; i < MAXPALYASZAM; i++) {
        mergeegyfele(&State->palyakidejei[i].singleidok, &pmerge->palyakidejei[i].singleidok, 1);
        mergeegyfele(&State->palyakidejei[i].multiidok, &pmerge->palyakidejei[i].multiidok, 0);
    }

    delete pmerge;

    State->save(); // reload is a few lines above
}
