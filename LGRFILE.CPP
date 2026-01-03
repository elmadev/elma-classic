#include "ALL.H"
#include "LGRFILE.H"
#include "anim.h"
#include "ECSET.H"
#include "grass.h"
#include "KISKEP.H"
#include "M_PIC.H"
#include "menu_pic.h"
#include "pic8.h"
#include "piclist.h"

/*void tesztloadlgr( void ) {
    int a = 1, b = 2, c = 4;
    for( int i = 0; i < 20; i++ ) {
        lgrfile* plgr = new lgrfile( "default" );
        delete plgr;
        a++;
        b = a+c;
    }
} */

constexpr int Checknumber_lgr = 187565543;

lgrfile* Plgr = NULL; // Az eppen bentlevo lgrfile-ra mutat

static char Bentlgrnev[30] = "";

/*
kisbox KisboxA = { 3, 36, 146, 183 };
kisbox KisboxB = { 32, 184, 146, 297 };
kisbox KisboxC = { 147, 141, 272, 264 };
kisbox KisboxD = { 273, 181, 353, 244 };
*/

kisbox KisboxA = {3, 36, 147, 184};
kisbox KisboxB = {32, 183, 147, 297};
kisbox KisboxC = {146, 141, 273, 264};
kisbox KisboxD = {272, 181, 353, 244};

// Ha nincs ilyen nevu file, akkor default-ot olvassa:
void loadlgrfile(const char* nev) {
    // tesztloadlgr();
    // internal_error( "A" );

    if (strlen(nev) > 8) {
        internal_error("loadlgrfile-ban strlen( nev ) > 8!");
    }

    if (strcmpi(nev, Bentlgrnev) == 0) {
        return;
    }

    char lgrtmp[30];
    sprintf(lgrtmp, "lgr/%s.lgr", nev);
    if (access(lgrtmp, 0) != 0) {
        if (!Ptop) {
            internal_error("loadlgrfile-ban !Ptop!");
        }

        char nevtmp[20];
        strcpy(nevtmp, nev);
        strcat(nevtmp, ".LGR");

        // Kiirunk egy uzenetet:
        blit8(BufferBall, BufferMain);
        BufferMain->fill_box(Hatterindex);
        bltfront(BufferMain);
        if (!InEditor) {
            Pal_editor->set();
        }
        dialog(
            "LGR file not found!",
            "The level file uses the pictures that are stored in this LGR file:", nevtmp,
            "This file doesn't exist in the LGR directory, so the DEFAULT.LGR file will be loaded.",
            "This level file will look now different from that it was designed to look.");
        if (!InEditor) {
            MenuPal->set();
        }
        blit8(BufferMain, BufferBall);
        bltfront(BufferMain);

        // (Ptop->lgrnev lehet egy mutato nev-vel)
        strcpy(Ptop->lgrnev, "DEFAULT");
        Valtozott = 1;

        if (strcmpi(Bentlgrnev, "DEFAULT") == 0) {
            return;
        }

        strcpy(lgrtmp, "lgr/default.lgr");
        strcpy(Bentlgrnev, "DEFAULT");

        Ptop->nincslgrje = 1;
        if (access(lgrtmp, 0) != 0) {
            external_error("Could not open file LGR\\DEFAULT.LGR!");
        }
    }

    strcpy(Bentlgrnev, nev);

    if (Plgr) {
        delete Plgr;
    }
    Plgr = new lgrfile(Bentlgrnev);
    return;
}

static void chopdarab(pic8* pbiker, kiskep** ppkiskep, kisbox* pbox) {
    // kivag nagy.tga a.tga 272 275 347 334
    /*pic8* ppic = new pic8( 347 - 272 + 1, 334 - 275 + 1 );
    blit8( ppic, pbiker, -(272-260), -(275-260) );
    pkepek->pkisa = new kiskep( NULL, ppic ); // ppic-et ez deleteli is
    */ // Eddig a regi volt

    pic8* ppic = new pic8(pbox->x2 - pbox->x1 + 1, pbox->y2 - pbox->y1 + 1);
    blit8(ppic, pbiker, -pbox->x1, -pbox->y1);
    *ppkiskep = new kiskep(NULL, ppic); // ppic-et ez deleteli is
}

void lgrfile::chopbiker(pic8* pbiker, motkepek* pkepek) {
    // kivag nagy.tga a.tga 272 275 347 334
    // kivag nagy.tga b.tga 287 334 404 558
    // kivag nagy.tga c.tga 404 369 451 530
    // kivag nagy.tga d.tga 451 289 568 538
    // kivag nagy.tga e.tga 569 474 626 529

    chopdarab(pbiker, &pkepek->pkisa, &KisboxA);
    chopdarab(pbiker, &pkepek->pkisb, &KisboxB);
    chopdarab(pbiker, &pkepek->pkisc, &KisboxC);
    chopdarab(pbiker, &pkepek->pkisd, &KisboxD);

    // Most atlatszo szint pkisa alapjan osszesre beallitjuk:
    pkepek->pkisb->lyuk = pkepek->pkisc->lyuk = pkepek->pkisd->lyuk = pkepek->pkisa->lyuk;
}

static pic8* kepetbovitvizszint(textura* pt) {
    int cellasize = pt->origxsize;
    int szorzo = 1;
    while (cellasize * szorzo < SCREEN_WIDTH) {
        szorzo++;
    }
    szorzo++; // Igy mar legalabb egy keppel szelesebb SCREEN_WIDTH-nel
    pic8* ppic = new pic8(cellasize * szorzo, pt->ppic->get_height());
    for (int i = 0; i < szorzo; i++) {
        blit8(ppic, pt->ppic, i * cellasize, 0);
    }
    return ppic;
}

// h-nak pcx file vegen kell allnia:
static unsigned char* getnewpalfromh(FILE* h) {
    unsigned char* pal = new unsigned char[768];

    if (fseek(h, -769, SEEK_CUR) != 0) {
        internal_error("Nem tud visszalepni palettahoz 768-at LGR-ben!: ");
    }
    char c;
    if (fread(&c, 1, 1, h) != 1) {
        internal_error("Nem tud olvasni pal byte-ot LGR-ben!:");
    }
    if (c != 0x0c) {
        internal_error("Nem 0x0C a paletta elotti byte LGR-ben!: ");
    }
    if (fread(pal, 1, 768, h) != 768) {
        internal_error("Nem tud olvasni palettat LGR-ben!: ");
    }
    return pal;
}

static int getatlatszosag(piclist::Transparency tipus, pic8* ppic) {
    switch (tipus) {
    case piclist::Transparency::None:
        return -1;
    case piclist::Transparency::Palette0:
        return 0;
    case piclist::Transparency::TopLeft:
        return ppic->gpixel(0, 0);
    case piclist::Transparency::TopRight:
        return ppic->gpixel(ppic->get_width() - 1, 0);
    case piclist::Transparency::BottomLeft:
        return ppic->gpixel(0, ppic->get_height() - 1);
    case piclist::Transparency::BottomRight:
        return ppic->gpixel(ppic->get_width() - 1, ppic->get_height() - 1);
    }
    internal_error("getatlatszosag");
    return -1;
}

#define PICTOMBMERET (600000)

static unsigned char* Tmptomb = NULL;

void lgrfile::beirkepet(pic8* ppic, piclist* ppiclist, int index) {
    if (kepszam >= MAXKEPSZAM) {
        external_error("Too many pictures in lgr file!");
    }

    kep* pkep = &kepek[kepszam];

    strcpy(pkep->nev, &ppiclist->name[index * 10]);
    pkep->tavolsag = ppiclist->default_distance[index];
    pkep->hatarol = ppiclist->default_clipping[index];

    // Beetetjuk kepadatokat:
    pkep->xsize = ppic->get_width();
    pkep->ysize = ppic->get_height();

    if (pkep->xsize > 60000) {
        external_error("Picture xsize is too big!", pkep->nev);
    }

    int atlatszo = getatlatszosag(ppiclist->transparency[index], ppic);
    if (atlatszo < 0) {
        external_error("Picture must be transparent in lgr file!", pkep->nev);
    }

    if (!Tmptomb) {
        Tmptomb = new unsigned char[PICTOMBMERET + 10];
    }
    if (!Tmptomb) {
        internal_error("Not enough memory!");
    }

    int i = 0;
    for (int y = 0; y < pkep->ysize; y++) {
        // Egy sor elintezese:
        unsigned char* sor = ppic->get_row(y);
        if (i > PICTOMBMERET) {
            external_error("Picture is too big in lgr file! Picture name:", pkep->nev);
        }
        int x = 0;
        while (1) {
            // Beirjuk uresek szamat:
            int uresx = uresszam(x, pkep->xsize, sor, (unsigned char)atlatszo);
            if (uresx > 60000) {
                internal_error("ghuieg");
            }
            int uresx1 = uresx / 256;
            int uresx2 = uresx % 256;
            Tmptomb[i] = (unsigned char)uresx1;
            Tmptomb[i + 1] = (unsigned char)uresx2;
            x += uresx;
            if (x >= pkep->xsize) {
                // Vege sornak:
                Tmptomb[i] = 255;
                Tmptomb[i + 1] = 255;
                i += 2;
                break;
            }
            i += 2;

            // Beirjuk telik szamat:
            int telix = teliszam(x, pkep->xsize, sor, (unsigned char)atlatszo);
            if (telix <= 0) {
                internal_error("grgpior");
            }
            if (telix > 60000) {
                internal_error("89grwe");
            }
            int telix1 = telix / 256;
            int telix2 = telix % 256;
            Tmptomb[i] = (unsigned char)telix1;
            Tmptomb[i + 1] = (unsigned char)telix2;
            i += 2;

            // Bemasoljuk kep byte-okat:
            if (i + telix > PICTOMBMERET) {
                external_error("Picture is too big in lgr file! Picture name:", pkep->nev);
            }
            memcpy(&Tmptomb[i], &sor[x], telix);

            x += telix;
            i += telix;
        }
    }

    pkep->adatok = new unsigned char[i + 10];
    if (!pkep->adatok) {
        internal_error("Not enough memory!");
    }
    memcpy(pkep->adatok, Tmptomb, i);

    kepszam++;
}

void lgrfile::beirtexturat(pic8* ppic, piclist* ppiclist, int index) {
    if (texturaszam >= MAXTEXTURASZAM) {
        external_error("Too many textures in lgr file!");
    }

    // Megforditjuk textura kepet fejjel lefele:
    forditkepet(ppic);

    textura* ptext = &texturak[texturaszam];

    if (ppiclist) {
        strcpy(ptext->nev, &ppiclist->name[index * 10]);
        ptext->ppic = ppic;
        ptext->tavolsag = ppiclist->default_distance[index];
        ptext->hatarol = ppiclist->default_clipping[index];
        ptext->foltos = 0;
    } else {
        // Csak fu eseten:
        strcpy(ptext->nev, "qgrass");
        ptext->ppic = ppic;
        ptext->tavolsag = 450;
        ptext->hatarol = HATAROL_G;
        ptext->foltos = 1;
    }

    texturaszam++;
}

#define MAXMASZKELEM (20000)
static maszkelem Melemtomb[MAXMASZKELEM];

void lgrfile::beirmaszkot(pic8* ppic, piclist* ppiclist, int index) {
    if (maszkszam >= MAXMASZKSZAM) {
        external_error("Too many masks in lgr file!");
    }

    maszk* pm = &maszkok[maszkszam];

    strcpy(pm->nev, &ppiclist->name[index * 10]);
    pm->xsize = ppic->get_width();
    pm->ysize = ppic->get_height();

    // Most beeszi adatokat:
    int i = 0;
    int atlatszo = getatlatszosag(ppiclist->transparency[index], ppic);
    if (atlatszo >= 0) {
        // Van atlatszosag:
        for (int y = 0; y < pm->ysize; y++) {
            // Kep egy soranak elintezese:
            unsigned char* sor = ppic->get_row(y);

            // Atugorja elso ures pixeleket:
            int x = uresszam(0, pm->xsize, sor, (unsigned char)atlatszo);
            if (x > 0) {
                Melemtomb[i].tipus = ME_URES;
                Melemtomb[i].hossz = x;
                i++;
            }
            while (x <= pm->xsize - 1) {
                // Keresi kis szakaszokat:
                int size = teliszam(x, pm->xsize, sor, (unsigned char)atlatszo);
                if (size <= 0) {
                    internal_error("745gh8g5");
                }

                Melemtomb[i].tipus = ME_TELI;
                Melemtomb[i].hossz = size;
                i++;
                if (i > MAXMASZKELEM - 10) {
                    external_error("Mask picture is too complicated!:",
                                   &ppiclist->name[index * 10]);
                }

                x += size;

                // Atugorja ures pixeleket:
                size = uresszam(x, pm->xsize, sor, (unsigned char)atlatszo);
                if (size > 0) {
                    Melemtomb[i].tipus = ME_URES;
                    Melemtomb[i].hossz = size;
                    i++;
                }
                x += size;
            }
            Melemtomb[i].tipus = ME_SOREMELES;
            Melemtomb[i].hossz = 0;
            i++;
        }
    } else {
        // Nincs atlatszosag:
        for (int y = 0; y < pm->ysize; y++) {
            // Kep egy soranak elintezese:
            Melemtomb[i].tipus = ME_TELI;
            Melemtomb[i].hossz = pm->xsize;
            i++;
            Melemtomb[i].tipus = ME_SOREMELES;
            Melemtomb[i].hossz = 0;
            i++;
        }
    }

    // Most betesszuk lefoglalt helyre atlatszosag infot:
    pm->adatok = new maszkelem[i];
    if (!pm->adatok) {
        internal_error("Memory!");
    }
    for (int j = 0; j < i; j++) {
        pm->adatok[j] = Melemtomb[j];
    }

    delete ppic;
    maszkszam++;
}

// Bemegy egy 768 byte-os paletta es kijon egy 256 byte-os lookup tabla:
// Idokiiras ez alapjan ir ki minel kontrasztosabb szinnel:
static unsigned char* makenegalttomb(unsigned char* pal) {
    unsigned char* lookuptomb = new unsigned char[260];
    if (!lookuptomb) {
        internal_error("memory (h34252)");
    }

    // Megkeressuk legvilagosabb szint:
    int eddigiertek = -1;
    int legvilag = 0;
    for (int i = 0; i < 256; i++) {
        int ujertek = pal[i * 3] + pal[i * 3 + 1] + pal[i * 3 + 2];
        if (eddigiertek < ujertek) {
            eddigiertek = ujertek;
            legvilag = i;
        }
    }

    // Megkeressuk legsotetebb szint:
    eddigiertek = 1000;
    int legsotet = 0;
    for (int i = 0; i < 256; i++) {
        int ujertek = pal[i * 3] + pal[i * 3 + 1] + pal[i * 3 + 2];
        if (eddigiertek > ujertek) {
            eddigiertek = ujertek;
            legsotet = i;
        }
    }

    // Most lookup tabla minden szinehez hozza rendeljuk feketet (0 index),
    // vagy legvilagosabbat:
    for (int i = 0; i < 256; i++) {
        if ((pal[i * 3] < 80) && (pal[i * 3 + 1] < 80) && (pal[i * 3 + 2] < 80)) {
            lookuptomb[i] = (unsigned char)legvilag;
        } else {
            lookuptomb[i] = (unsigned char)legsotet;
        }
    }
    return lookuptomb;
}

lgrfile::lgrfile(const char* lgrnev) {
    // Lenullaz mindent:
    kepszam = 0;
    maszkszam = 0;
    texturaszam = 0;
    for (int i = 0; i < MAXKEPSZAM; i++) {
        memset(&kepek[i], 0, sizeof(kep));
    }
    for (int i = 0; i < MAXMASZKSZAM; i++) {
        memset(&maszkok[i], 0, sizeof(maszk));
    }
    for (int i = 0; i < MAXTEXTURASZAM; i++) {
        memset(&texturak[i], 0, sizeof(textura));
    }

    pkoveto = NULL;

    pal = NULL;
    paltomb = NULL;
    idonegtomb = NULL;

    memset(&mkepek1, 0, sizeof(mkepek1));
    memset(&mkepek2, 0, sizeof(mkepek2));
    pkiszaszlo = NULL;

    pkiller = pexit = NULL;
    pkimarad = NULL;

    peg = pfold = NULL;
    fgnevbent[0] = bgnevbent[0] = 0;

    foodszam = 0;
    for (int i = 0; i < MAXFOODSZAM; i++) {
        foodtomb[i] = NULL;
    }

    pkoveto = new grass;

    // Megnyitja file-t:
    char lgrtmp[30];
    sprintf(lgrtmp, "lgr/%s.lgr", lgrnev);
    FILE* h = fopen(lgrtmp, "rb");
    if (!h) {
        external_error("Cannot find file:", lgrtmp);
    }

    char tmp2[10];
    if (fread(tmp2, 1, 5, h) != 5) {
        external_error("Corrupt LGR file!:", lgrtmp);
    }
    tmp2[5] = 0;
    if (strncmp(tmp2, "LGR", 3) != 0) {
        external_error("This is not an LGR file!:", lgrtmp);
    }
    if (strcmp(tmp2 + 3, "12") != 0) {
        external_error("LGR file's version is too new!:", lgrtmp);
    }

    // Most osszes kep szamat taroljuk kepszam-ban, kesobb csak indexeseket:
    int osszkepszam;
    if (fread(&osszkepszam, 1, 4, h) != 4) {
        external_error("Corrupt LGR file!:", lgrtmp);
    }

    if (osszkepszam < 10 || osszkepszam > 3500) {
        external_error("Corrupt LGR file!:", lgrtmp);
    }

    // Most beolvassuk pictures.lst file binaris valtozatat:
    piclist* ppiclist = new piclist(h);
    if (!ppiclist) {
        internal_error("Nincs eleg memoria!");
    }

    // Vegigmegy kepeken:
    pic8* p1biker = NULL;
    pic8* p2biker = NULL;
    pic8* pcolors = NULL;
    for (int i = 0; i < osszkepszam; i++) {
        // Egy kep:
        char nev[30];
        if (fread(nev, 1, 20, h) != 20) {
            external_error("Corrupt LGR file!:", lgrtmp);
        }
        int hossz = 0;
        if (fread(&hossz, 1, 4, h) != 4) {
            external_error("Corrupt LGR file!:", lgrtmp);
        }
        if (hossz < 1 || hossz > 10000000) {
            external_error("Corrupt LGR file!:", lgrtmp);
        }

        // Beolvassuk kepet:
        long kurpos = ftell(h);
        pic8* ppic = new pic8(nev, h);
        fseek(h, kurpos + hossz, SEEK_SET);

        // Kimentjuk osszes kepet:
        /*static int elso = 1;
        static unsigned char* savepal = NULL;
        if( elso ) {
            elso = 0;
            savepal = getnewpalfromh( h );
        }
        char nev4[50] = "pictmp\\";
        strcat( nev4, nev );
        ppic->save( nev4, savepal );
        */

        // Osztalyozzuk nevet:
        // Eloszor kotelezo kepeket nezzuk:
        if (strcmpi(nev, "q1bike.pcx") == 0) {
            p1biker = ppic;
            paltomb = getnewpalfromh(h);
            idonegtomb = makenegalttomb(paltomb);
            pal = new palette(paltomb);
            continue;
        }
        if (strcmpi(nev, "q2bike.pcx") == 0) {
            p2biker = ppic;
            continue;
        }

        // 1. motoros reszei:
        if (strcmpi(nev, "q1body.pcx") == 0) {
            // kiskep konstructor delete-eli ppic-t:
            mkepek1.pkisvezeto = new kiskep(NULL, ppic);
            continue;
        }
        if (strcmpi(nev, "q1thigh.pcx") == 0) {
            mkepek1.pkiscomb = new kiskep(NULL, ppic);
            continue;
        }
        if (strcmpi(nev, "q1leg.pcx") == 0) {
            mkepek1.pkislabszar = new kiskep(NULL, ppic);
            continue;
        }

        if (strcmpi(nev, "q1wheel.pcx") == 0) {
            mkepek1.pkiskerek = new kiskep(NULL, ppic);
            continue;
        }
        if (strcmpi(nev, "q1susp1.pcx") == 0) {
            mkepek1.pkiselsorud = new kiskep(NULL, ppic);
            continue;
        }
        if (strcmpi(nev, "q1susp2.pcx") == 0) {
            mkepek1.pkishatsorud = new kiskep(NULL, ppic);
            continue;
        }
        if (strcmpi(nev, "q1forarm.pcx") == 0) {
            mkepek1.pkisalkar = new kiskep(NULL, ppic);
            continue;
        }
        if (strcmpi(nev, "q1up_arm.pcx") == 0) {
            mkepek1.pkisfelkar = new kiskep(NULL, ppic);
            continue;
        }
        if (strcmpi(nev, "q1head.pcx") == 0) {
            mkepek1.pkisfej = new kiskep(NULL, ppic);
            continue;
        }

        // 2. motoros reszei:
        if (strcmpi(nev, "q2body.pcx") == 0) {
            // kiskep konstructor delete-eli ppic-t:
            mkepek2.pkisvezeto = new kiskep(NULL, ppic);
            continue;
        }
        if (strcmpi(nev, "q2thigh.pcx") == 0) {
            mkepek2.pkiscomb = new kiskep(NULL, ppic);
            continue;
        }
        if (strcmpi(nev, "q2leg.pcx") == 0) {
            mkepek2.pkislabszar = new kiskep(NULL, ppic);
            continue;
        }

        if (strcmpi(nev, "q2wheel.pcx") == 0) {
            mkepek2.pkiskerek = new kiskep(NULL, ppic);
            continue;
        }
        if (strcmpi(nev, "q2susp1.pcx") == 0) {
            mkepek2.pkiselsorud = new kiskep(NULL, ppic);
            continue;
        }
        if (strcmpi(nev, "q2susp2.pcx") == 0) {
            mkepek2.pkishatsorud = new kiskep(NULL, ppic);
            continue;
        }
        if (strcmpi(nev, "q2forarm.pcx") == 0) {
            mkepek2.pkisalkar = new kiskep(NULL, ppic);
            continue;
        }
        if (strcmpi(nev, "q2up_arm.pcx") == 0) {
            mkepek2.pkisfelkar = new kiskep(NULL, ppic);
            continue;
        }
        if (strcmpi(nev, "q2head.pcx") == 0) {
            mkepek2.pkisfej = new kiskep(NULL, ppic);
            continue;
        }

        if (strcmpi(nev, "qflag.pcx") == 0) {
            pkiszaszlo = new kiskep(NULL, ppic);
            continue;
        }

        // Objektumok:
        if (strcmpi(nev, "qkiller.pcx") == 0) {
            pkiller = new anim(ppic, "qkiller.pcx");
            delete ppic;
            ppic = NULL;
            continue;
        }
        if (strcmpi(nev, "qexit.pcx") == 0) {
            pexit = new anim(ppic, "qexit.pcx");
            delete ppic;
            ppic = NULL;
            continue;
        }

        if (strcmpi(nev, "qframe.pcx") == 0) {
            pkimarad = ppic;
            continue;
        }
        if (strcmpi(nev, "qcolors.pcx") == 0) {
            pcolors = ppic;
            continue;
        }

        int ezfoodvolt = 0;
        for (int foodi = 0; foodi < 9; foodi++) {
            char tmpnev[20];
            sprintf(tmpnev, "qfood%d.pcx", foodi + 1);
            if (strcmpi(nev, tmpnev) == 0) {
                foodtomb[foodi] = new anim(ppic, tmpnev);
                delete ppic;
                ppic = NULL;
                ezfoodvolt = 1;
            }
        }
        if (ezfoodvolt) {
            continue;
        }

        // Kovetok:
        if (_fstrnicmp(nev, "qup_", 4) == 0) {
            pkoveto->add(ppic, true);
            continue;
        }
        if (_fstrnicmp(nev, "qdown_", 6) == 0) {
            pkoveto->add(ppic, false);
            continue;
        }

        // Most mar csak piclistben szereplo kep lehet:
        // Leszedjuk kiterjesztest:
        if (!strchr(nev, '.')) {
            external_error("Cannot find dot in name:", nev);
        }
        *strchr(nev, '.') = 0;

        if (strlen(nev) > 8) {
            external_error("Filename is long in LGR file!:", nev, lgrtmp);
        }

        // Grass:
        if (strcmpi(nev, "qgrass") == 0) {
            beirtexturat(ppic, NULL, 0);
            continue;
        }

        int index = ppiclist->get_index(nev);
        if (index < 0) {
            external_error("There is no line in PICTURES.LST corresponding to picture:", nev);
        }
        if (ppiclist->type[index] == piclist::Type::Picture) {
            beirkepet(ppic, ppiclist, index);
            delete ppic; // Kepre mar nincsen tobbe szuksegunk
            ppic = NULL;
            continue;
        }
        if (ppiclist->type[index] == piclist::Type::Texture) {
            beirtexturat(ppic, ppiclist, index);
            continue;
        }
        if (ppiclist->type[index] == piclist::Type::Mask) {
            // delete-li ppic-et:
            beirmaszkot(ppic, ppiclist, index);
            ppic = NULL;
            continue;
        }
        // Ismeretlen tipus:
        external_error("Corrupt LGR file!:", lgrtmp);
    }

    int checknumber = 0;
    if (fread(&checknumber, 1, 4, h) != 4) {
        external_error("Corrupt LGR file!:", lgrtmp);
    }
    if (checknumber != Checknumber_lgr) {
        external_error("Corrupt LGR file!:", lgrtmp);
    }

    fclose(h);
    h = NULL;
    // Minden be van olvasva, file le van zarva:

    // Most csinalunk egy kis ellenorzest:

    if (texturaszam < 2) {
        external_error("There must be at least two textures in LGR file!", lgrnev);
    }

    // 1. motoros:
    if (!mkepek1.pkisvezeto) {
        external_error("Picture not found in LGR file!: ", "q1body.pcx", lgrtmp);
    }
    if (!mkepek1.pkiscomb) {
        external_error("Picture not found in LGR file!: ", "q1thigh.pcx", lgrtmp);
    }
    if (!mkepek1.pkislabszar) {
        external_error("Picture not found in LGR file!: ", "q1leg.pcx", lgrtmp);
    }

    if (!p1biker) {
        external_error("Picture not found in LGR file!: ", "q1bike.pcx", lgrtmp);
    }
    if (!mkepek1.pkiskerek) {
        external_error("Picture not found in LGR file!: ", "q1wheel.pcx", lgrtmp);
    }
    if (!mkepek1.pkiselsorud) {
        external_error("Picture not found in LGR file!: ", "q1susp1.pcx", lgrtmp);
    }
    if (!mkepek1.pkishatsorud) {
        external_error("Picture not found in LGR file!: ", "q1susp2.pcx", lgrtmp);
    }
    if (!mkepek1.pkisalkar) {
        external_error("Picture not found in LGR file!: ", "q1forarm.pcx", lgrtmp);
    }
    if (!mkepek1.pkisfelkar) {
        external_error("Picture not found in LGR file!: ", "q1up_arm.pcx", lgrtmp);
    }
    if (!mkepek1.pkisfej) {
        external_error("Picture not found in LGR file!: ", "q1head.pcx", lgrtmp);
    }

    // 2. motoros:
    if (!mkepek2.pkisvezeto) {
        external_error("Picture not found in LGR file!: ", "q2body.pcx", lgrtmp);
    }
    if (!mkepek2.pkiscomb) {
        external_error("Picture not found in LGR file!: ", "q2thigh.pcx", lgrtmp);
    }
    if (!mkepek2.pkislabszar) {
        external_error("Picture not found in LGR file!: ", "q2leg.pcx", lgrtmp);
    }

    if (!p2biker) {
        external_error("Picture not found in LGR file!: ", "q2bike.pcx", lgrtmp);
    }
    if (!mkepek2.pkiskerek) {
        external_error("Picture not found in LGR file!: ", "q2wheel.pcx", lgrtmp);
    }
    if (!mkepek2.pkiselsorud) {
        external_error("Picture not found in LGR file!: ", "q2susp1.pcx", lgrtmp);
    }
    if (!mkepek2.pkishatsorud) {
        external_error("Picture not found in LGR file!: ", "q2susp2.pcx", lgrtmp);
    }
    if (!mkepek2.pkisalkar) {
        external_error("Picture not found in LGR file!: ", "q2forarm.pcx", lgrtmp);
    }
    if (!mkepek2.pkisfelkar) {
        external_error("Picture not found in LGR file!: ", "q2up_arm.pcx", lgrtmp);
    }
    if (!mkepek2.pkisfej) {
        external_error("Picture not found in LGR file!: ", "q2head.pcx", lgrtmp);
    }

    if (!pkiszaszlo) {
        external_error("Picture not found in LGR file!: ", "qflag.pcx", lgrtmp);
    }

    if (!pkiller) {
        external_error("Picture not found in LGR file!: ", "qkiller.pcx", lgrtmp);
    }
    if (!pexit) {
        external_error("Picture not found in LGR file!: ", "qexit.pcx", lgrtmp);
    }

    if (!pkimarad) {
        external_error("Picture not found in LGR file!: ", "qframe.pcx", lgrtmp);
    }
    if (!pcolors) {
        external_error("Picture not found in LGR file!: ", "qcolors.pcx", lgrtmp);
    }

    // Elintezzuk biker picture-oket:
    chopbiker(p1biker, &mkepek1);
    delete p1biker;
    p1biker = NULL;

    chopbiker(p2biker, &mkepek2);
    delete p2biker;
    p2biker = NULL;

    // Elintezzuk index szineket:
    viewfoldsor[0] = pcolors->gpixel(6, 6 + 0 * 12);
    viewegsor[0] = pcolors->gpixel(6, 6 + 1 * 12);
    keretindex = pcolors->gpixel(6, 6 + 2 * 12);
    viewmotorindex1 = pcolors->gpixel(6, 6 + 4 * 12);
    viewmotorindex2 = pcolors->gpixel(6, 6 + 5 * 12);
    viewexit[0] = pcolors->gpixel(6, 6 + 6 * 12);
    viewfood = pcolors->gpixel(6, 6 + 7 * 12);
    viewkiller[0] = pcolors->gpixel(6, 6 + 8 * 12);
    delete pcolors;
    pcolors = NULL;
    // ecsetben mutato fog hivatkozni rajuk (max 3 egymas utan):
    viewexit[2] = viewexit[1] = viewexit[0];
    viewkiller[2] = viewkiller[1] = viewkiller[0];
    for (int i = 1; i < SCREEN_WIDTH; i++) {
        viewfoldsor[i] = viewfoldsor[0];
        viewegsor[i] = viewegsor[0];
    }

    // Most texturakat kibovitjuk vizszintes iranyban:
    for (int i = 0; i < texturaszam; i++) {
        textura* pt = &texturak[i];
        if (!pt->ppic) {
            internal_error("iuo4gt");
        }
        pt->origxsize = pt->ppic->get_width();
        int minxmintasize = 600;
        if (pt->ppic->get_width() >= minxmintasize) {
            continue;
        }
        // Kep tul kicsi, kibovitjuk vizszintes iranyban:
        int szorzo = 1;
        while (szorzo * pt->ppic->get_width() < minxmintasize) {
            szorzo++;
        }
        pic8* pbovitett = new pic8(szorzo * pt->ppic->get_width(), pt->ppic->get_height());
        for (int k = 0; k < szorzo; k++) {
            blit8(pbovitett, pt->ppic, k * pt->ppic->get_width(), 0);
        }
        delete pt->ppic;
        pt->ppic = pbovitett;
    }

    // Most kepeket abc sorrendbe rendezzuk:
    for (int i = 0; i < kepszam + 2; i++) {
        for (int j = 0; j < kepszam - 1; j++) {
            if (strcmpi(kepek[j].nev, kepek[j + 1].nev) == 0) {
                external_error("Picture name is duplicated in LGR file!: ", kepek[j].nev);
            }

            if (strcmpi(kepek[j].nev, kepek[j + 1].nev) > 0) {
                // Csere:
                kep tmp = kepek[j];
                kepek[j] = kepek[j + 1];
                kepek[j + 1] = tmp;
            }
        }
    }

    // Most maszkokat abc sorrendbe rendezzuk:
    for (int i = 0; i < maszkszam + 2; i++) {
        for (int j = 0; j < maszkszam - 1; j++) {
            if (strcmpi(maszkok[j].nev, maszkok[j + 1].nev) == 0) {
                external_error("Mask name is duplicated in LGR file!: ", maszkok[j].nev);
            }

            if (strcmpi(maszkok[j].nev, maszkok[j + 1].nev) > 0) {
                // Csere:
                maszk tmp = maszkok[j];
                maszkok[j] = maszkok[j + 1];
                maszkok[j + 1] = tmp;
            }
        }
    }

    // Most texturakat abc sorrendbe rendezzuk:
    for (int i = 0; i < texturaszam + 2; i++) {
        for (int j = 0; j < texturaszam - 1; j++) {
            if (strcmpi(texturak[j].nev, texturak[j + 1].nev) == 0) {
                external_error("Texture name is duplicated in LGR file!: ", texturak[j].nev);
            }

            if (strcmpi(texturak[j + 1].nev, "qgrass") != 0 &&
                (strcmpi(texturak[j].nev, texturak[j + 1].nev) > 0 ||
                 strcmpi(texturak[j].nev, "qgrass") == 0)) { // qgrass vegere
                // Csere:
                textura tmp = texturak[j];
                texturak[j] = texturak[j + 1];
                texturak[j + 1] = tmp;
            }
        }
    }

    delete ppiclist;
    ppiclist = NULL;

    // Pic adatok letrehozasahoz felvett tombot toroljuk, mivel tul nagy:
    if (Tmptomb) {
        delete Tmptomb;
        Tmptomb = NULL;
    }

    aktiv_kepnev[0] = 0;
    aktiv_maszknev[0] = 0;
    aktiv_texturanev[0] = 0;

    // Most meg food-okat megszamoljuk:
    foodszam = 0;
    for (int i = 0; i < 9; i++) {
        if (foodtomb[i]) {
            foodszam = i + 1;
        } else {
            break;
        }
    }
    if (foodszam < 1) {
        external_error("Picture is missing from LGR file:", "qfood1.pcx", lgrtmp);
    }

    vanfu = 1;
    opentextkimarad = 1;
    if (gettexturaindex("qgrass") < 0) {
        vanfu = 0;
        opentextkimarad = 0;
    }
    if (pkoveto->length < 2) {
        // external_error( "Picture not found in LGR file!: ", "qgrass.pcx", lgrtmp );
        vanfu = 0;
    }
}

static void deletemkepekstruct(motkepek* pk) {
    if (!pk->pkisvezeto || !pk->pkiscomb || !pk->pkislabszar || !pk->pkisa || !pk->pkisb ||
        !pk->pkisc || !pk->pkisd || !pk->pkiskerek || !pk->pkiselsorud || !pk->pkishatsorud ||
        !pk->pkisalkar || !pk->pkisfelkar || !pk->pkisfej) {
        internal_error("deletemkepekstruct-ban ervenytelen valamelyik kiskep!");
    }

    delete pk->pkisvezeto;
    delete pk->pkiscomb;
    delete pk->pkislabszar;
    delete pk->pkisa;
    delete pk->pkisb;
    delete pk->pkisc;
    delete pk->pkisd;
    delete pk->pkiskerek;
    delete pk->pkiselsorud;
    delete pk->pkishatsorud;
    delete pk->pkisalkar;
    delete pk->pkisfelkar;
    delete pk->pkisfej;
}

lgrfile::~lgrfile(void) {
    for (int i = 0; i < kepszam; i++) {
        if (!kepek[i].adatok) {
            internal_error("987ty7");
        }
        delete kepek[i].adatok;
        kepek[i].adatok = NULL;
    }
    for (int i = 0; i < maszkszam; i++) {
        if (!maszkok[i].adatok) {
            internal_error("gk7u6tk");
        }
        delete maszkok[i].adatok;
        maszkok[i].adatok = NULL;
    }
    for (int i = 0; i < texturaszam; i++) {
        if (!texturak[i].ppic) {
            internal_error("6r5uy");
        }
        delete texturak[i].ppic;
        texturak[i].ppic = NULL;
    }

    if (!pkoveto) {
        internal_error(";oihki");
    }
    delete pkoveto;
    pkoveto = NULL;

    kepszam = maszkszam = texturaszam = 0;

    if (!pal || !paltomb || !idonegtomb) {
        internal_error("lgrfile::~lgrfile-ban !pal || !paltomb || !idonegtomb!");
    }
    delete pal;
    delete paltomb;
    delete idonegtomb;
    pal = NULL;
    paltomb = NULL;
    idonegtomb = NULL;

    deletemkepekstruct(&mkepek1);
    deletemkepekstruct(&mkepek2);
    if (!pkiszaszlo || !pkimarad) {
        internal_error("7246426!");
    }
    delete pkiszaszlo;
    delete pkimarad;
    pkiszaszlo = NULL;
    pkimarad = NULL;

    if (!pkiller || !pexit) {
        internal_error("lgrfile::~lgrfile-ban ervenytelen valamelyik kep!");
    }

    delete pkiller;
    delete pexit;
    pkiller = pexit = NULL;

    foodszam = 0;
    for (int i = 0; i < MAXFOODSZAM; i++) {
        if (foodtomb[i]) {
            delete foodtomb[i];
            foodtomb[i] = NULL;
        }
    }

    if (peg) {
        delete peg;
        peg = NULL;
    }
    if (pfold) {
        delete pfold;
        pfold = NULL;
    }
}

// Ezt betoltecseteket hivja (Ptop-bol szedi kep neveket):
void lgrfile::betolthattereket(void) {
    if (!Ptop->fgnev[0] || !Ptop->bgnev[0]) {
        internal_error("ydsgyhi");
    }

    if (strcmpi(bgnevbent, Ptop->bgnev) != 0) {
        strcpy(bgnevbent, Ptop->bgnev);
        // Nem jo ami bent van:
        if (peg) {
            delete peg;
        }
        peg = NULL;

        int index = gettexturaindex(bgnevbent);
        if (index < 0) {
            internal_error("oijhoe");
        }
        textura* pt = &texturak[index];
        egxmodulus = pt->origxsize;
        peg = kepetbovitvizszint(pt);
    }

    if (strcmpi(fgnevbent, Ptop->fgnev) != 0) {
        strcpy(fgnevbent, Ptop->fgnev);
        // Nem jo ami bent van:
        if (pfold) {
            delete pfold;
        }
        pfold = NULL;

        int index = gettexturaindex(fgnevbent);
        if (index < 0) {
            internal_error("879grtjoi");
        }
        textura* pt = &texturak[index];
        foldxmodulus = pt->origxsize;
        pfold = kepetbovitvizszint(pt);
    }
}

// Ha nincs ilye nevu, akkor -1-et ad vissza
int lgrfile::getkepindex(const char* nev) {
    if (!nev[0]) {
        return -1;
    }
    for (int i = 0; i < kepszam; i++) {
        if (strcmpi(kepek[i].nev, nev) == 0) {
            return i;
        }
    }
    return -1;
}

// Ha nincs ilye nevu, akkor -1-et ad vissza
int lgrfile::getmaszkindex(const char* nev) {
    if (!nev[0]) {
        return -1;
    }
    for (int i = 0; i < maszkszam; i++) {
        if (strcmpi(maszkok[i].nev, nev) == 0) {
            return i;
        }
    }
    return -1;
}

// Ha nincs ilye nevu, akkor -1-et ad vissza
int lgrfile::gettexturaindex(const char* nev) {
    if (!nev[0]) {
        return -1;
    }
    for (int i = 0; i < texturaszam; i++) {
        if (strcmpi(texturak[i].nev, nev) == 0) {
            return i;
        }
    }
    return -1;
}

// Ez regi, ez meg kihasznalta, hogy abc sorrendben vannak:
// Ha nincs ilye nevu, akkor -1-et ad vissza
/*int lgrfile::getindex( char* nev ) {
    int di = 1;
    while( di*2 < kepszam )
        di *= 2;
    int i = di;
    if( kepszam == 1 )
        i = 0;
    while( 1 ) {
        int eredmeny = strcmpi( nevek[i], nev );
        if( !eredmeny )
            return i;
        if( di == 0 )
            return -1;
        if( eredmeny < 0 ) {
            i += di;
            if( i >= kepszam )
                i = kepszam-1;
        }
        else {
            i -= di;
            if( i < 0 )
                i = 0;
        }
        di /= 2;
    }
} */
