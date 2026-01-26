#include "lgr.h"
#include "editor_dialog.h"
#include "EDITUJ.H"
#include "affine_pic.h"
#include "anim.h"
#include "ECSET.H"
#include "eol_settings.h"
#include "grass.h"
#include "KIRAJZOL.H"
#include "level.h"
#include "LOAD.H"
#include "M_PIC.H"
#include "main.h"
#include "menu_pic.h"
#include "pic8.h"
#include "piclist.h"
#include "platform_impl.h"
#include "platform_utils.h"
#include "sprite.h"
#include <cstring>

constexpr int Checknumber_lgr = 187565543;

lgrfile* Lgr = NULL; // Az eppen bentlevo lgrfile-ra mutat

static char Bentlgrnev[30] = "";

bike_box BikeBox1 = {3, 36, 147, 184};
bike_box BikeBox2 = {32, 183, 147, 297};
bike_box BikeBox3 = {146, 141, 273, 264};
bike_box BikeBox4 = {272, 181, 353, 244};

void invalidate_lgr_cache() {
    invalidate_ptop();
    Bentlgrnev[0] = '\0';
}

// Ha nincs ilyen nevu file, akkor default-ot olvassa:
void load_lgr_file(const char* lgrname) {
    // tesztloadlgr();
    // internal_error( "A" );

    if (strlen(lgrname) > 8) {
        internal_error("load_lgr_file-ban strlen( lgrname ) > 8!");
    }

    if (strcmpi(lgrname, Bentlgrnev) == 0) {
        return;
    }

    char lgrtmp[30];
    sprintf(lgrtmp, "lgr/%s.lgr", lgrname);
    if (access(lgrtmp, 0) != 0) {
        if (!Ptop) {
            internal_error("load_lgr_file-ban !Ptop!");
        }

        char nevtmp[20];
        strcpy(nevtmp, lgrname);
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
            MenuPalette->set();
        }
        blit8(BufferMain, BufferBall);
        bltfront(BufferMain);

        // (Ptop->lgrnev lehet egy mutato nev-vel)
        strcpy(Ptop->lgr_name, "DEFAULT");
        Valtozott = 1;

        if (strcmpi(Bentlgrnev, "DEFAULT") == 0) {
            return;
        }

        strcpy(lgrtmp, "lgr/default.lgr");
        strcpy(Bentlgrnev, "DEFAULT");

        Ptop->lgr_not_found = true;
        if (access(lgrtmp, 0) != 0) {
            external_error("Could not open file LGR\\DEFAULT.LGR!");
        }
    }

    strcpy(Bentlgrnev, lgrname);

    if (Lgr) {
        delete Lgr;
    }
    Lgr = new lgrfile(Bentlgrnev);
    return;
}

static void chopdarab(pic8* pbiker, affine_pic** ppkiskep, bike_box* pbox) {
    pic8* ppic = new pic8(pbox->x2 - pbox->x1 + 1, pbox->y2 - pbox->y1 + 1);
    blit8(ppic, pbiker, -pbox->x1, -pbox->y1);
    *ppkiskep = new affine_pic(NULL, ppic); // ppic-et ez deleteli is
}

void lgrfile::chop_bike(pic8* bike, bike_pics* bp) {
    chopdarab(bike, &bp->bike_part1, &BikeBox1);
    chopdarab(bike, &bp->bike_part2, &BikeBox2);
    chopdarab(bike, &bp->bike_part3, &BikeBox3);
    chopdarab(bike, &bp->bike_part4, &BikeBox4);

    // Most atlatszo szint pkisa alapjan osszesre beallitjuk:
    bp->bike_part2->transparency = bp->bike_part3->transparency = bp->bike_part4->transparency =
        bp->bike_part1->transparency;
}

static pic8* kepetbovitvizszint(texture* pt) {
    int cellasize = pt->original_width;
    int szorzo = 1;
    while (cellasize * szorzo < SCREEN_WIDTH) {
        szorzo++;
    }
    szorzo++; // Igy mar legalabb egy keppel szelesebb SCREEN_WIDTH-nel
    pic8* ppic = new pic8(cellasize * szorzo, pt->pic->get_height());
    for (int i = 0; i < szorzo; i++) {
        blit8(ppic, pt->pic, i * cellasize, 0);
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

void lgrfile::add_picture(pic8* pic, piclist* list, int index) {
    if (picture_count >= MAX_PICTURES) {
        external_error("Too many pictures in lgr file!");
    }

    picture* pkep = &pictures[picture_count];

    strcpy(pkep->name, &list->name[index * 10]);
    pkep->default_distance = list->default_distance[index];
    pkep->default_clipping = list->default_clipping[index];

    // Beetetjuk kepadatokat:
    pkep->width = pic->get_width();
    pkep->height = pic->get_height();

    if (pkep->width > 60000) {
        external_error("Picture width is too big!", pkep->name);
    }

    int atlatszo = getatlatszosag(list->transparency[index], pic);
    if (atlatszo < 0) {
        external_error("Picture must be transparent in lgr file!", pkep->name);
    }

    if (!Tmptomb) {
        Tmptomb = new unsigned char[PICTOMBMERET + 10];
    }
    if (!Tmptomb) {
        internal_error("Not enough memory!");
    }

    int i = 0;
    for (int y = 0; y < pkep->height; y++) {
        // Egy sor elintezese:
        unsigned char* sor = pic->get_row(y);
        if (i > PICTOMBMERET) {
            external_error("Picture is too big in lgr file! Picture name:", pkep->name);
        }
        int x = 0;
        while (1) {
            // Beirjuk uresek szamat:
            int uresx = uresszam(x, pkep->width, sor, (unsigned char)atlatszo);
            if (uresx > 60000) {
                internal_error("ghuieg");
            }
            int uresx1 = uresx / 256;
            int uresx2 = uresx % 256;
            Tmptomb[i] = (unsigned char)uresx1;
            Tmptomb[i + 1] = (unsigned char)uresx2;
            x += uresx;
            if (x >= pkep->width) {
                // Vege sornak:
                Tmptomb[i] = 255;
                Tmptomb[i + 1] = 255;
                i += 2;
                break;
            }
            i += 2;

            // Beirjuk telik szamat:
            int telix = teliszam(x, pkep->width, sor, (unsigned char)atlatszo);
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
                external_error("Picture is too big in lgr file! Picture name:", pkep->name);
            }
            memcpy(&Tmptomb[i], &sor[x], telix);

            x += telix;
            i += telix;
        }
    }

    pkep->data = new unsigned char[i + 10];
    if (!pkep->data) {
        internal_error("Not enough memory!");
    }
    memcpy(pkep->data, Tmptomb, i);

    picture_count++;
}

void lgrfile::add_texture(pic8* pic, piclist* list, int index) {
    if (texture_count >= MAX_TEXTURES) {
        external_error("Too many textures in lgr file!");
    }

    // Megforditjuk textura kepet fejjel lefele:
    forditkepet(pic);

    texture* ptext = &textures[texture_count];

    if (list) {
        strcpy(ptext->name, &list->name[index * 10]);
        ptext->pic = pic;
        ptext->default_distance = list->default_distance[index];
        ptext->default_clipping = list->default_clipping[index];
        ptext->is_qgrass = 0;
    } else {
        // Csak fu eseten:
        strcpy(ptext->name, "qgrass");
        ptext->pic = pic;
        ptext->default_distance = 450;
        ptext->default_clipping = Clipping::Ground;
        ptext->is_qgrass = 1;
    }

    texture_count++;
}

#define MAXMASZKELEM (20000)
static mask_element Melemtomb[MAXMASZKELEM];

void lgrfile::add_mask(pic8* pic, piclist* list, int index) {
    if (mask_count >= MAX_MASKS) {
        external_error("Too many masks in lgr file!");
    }

    mask* pm = &masks[mask_count];

    strcpy(pm->name, &list->name[index * 10]);
    pm->width = pic->get_width();
    pm->height = pic->get_height();

    // Most beeszi adatokat:
    int i = 0;
    int atlatszo = getatlatszosag(list->transparency[index], pic);
    if (atlatszo >= 0) {
        // Van atlatszosag:
        for (int y = 0; y < pm->height; y++) {
            // Kep egy soranak elintezese:
            unsigned char* sor = pic->get_row(y);

            // Atugorja elso ures pixeleket:
            int x = uresszam(0, pm->width, sor, (unsigned char)atlatszo);
            if (x > 0) {
                Melemtomb[i].type = MaskEncoding_Transparent;
                Melemtomb[i].length = x;
                i++;
            }
            while (x <= pm->width - 1) {
                // Keresi kis szakaszokat:
                int size = teliszam(x, pm->width, sor, (unsigned char)atlatszo);
                if (size <= 0) {
                    internal_error("745gh8g5");
                }

                Melemtomb[i].type = MaskEncoding_Solid;
                Melemtomb[i].length = size;
                i++;
                if (i > MAXMASZKELEM - 10) {
                    external_error("Mask picture is too complicated!:", &list->name[index * 10]);
                }

                x += size;

                // Atugorja ures pixeleket:
                size = uresszam(x, pm->width, sor, (unsigned char)atlatszo);
                if (size > 0) {
                    Melemtomb[i].type = MaskEncoding_Transparent;
                    Melemtomb[i].length = size;
                    i++;
                }
                x += size;
            }
            Melemtomb[i].type = MaskEncoding_EndOfLine;
            Melemtomb[i].length = 0;
            i++;
        }
    } else {
        // Nincs atlatszosag:
        for (int y = 0; y < pm->height; y++) {
            // Kep egy soranak elintezese:
            Melemtomb[i].type = MaskEncoding_Solid;
            Melemtomb[i].length = pm->width;
            i++;
            Melemtomb[i].type = MaskEncoding_EndOfLine;
            Melemtomb[i].length = 0;
            i++;
        }
    }

    // Most betesszuk lefoglalt helyre atlatszosag infot:
    pm->data = new mask_element[i];
    if (!pm->data) {
        internal_error("Memory!");
    }
    for (int j = 0; j < i; j++) {
        pm->data[j] = Melemtomb[j];
    }

    delete pic;
    mask_count++;
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

lgrfile::lgrfile(const char* lgrname) {
    // Lenullaz mindent:
    picture_count = 0;
    mask_count = 0;
    texture_count = 0;
    for (int i = 0; i < MAX_PICTURES; i++) {
        memset(&pictures[i], 0, sizeof(picture));
    }
    for (int i = 0; i < MAX_MASKS; i++) {
        memset(&masks[i], 0, sizeof(mask));
    }
    for (int i = 0; i < MAX_TEXTURES; i++) {
        memset(&textures[i], 0, sizeof(texture));
    }

    grass_pics = NULL;

    pal = NULL;
    palette_data = NULL;
    timer_palette_map = NULL;

    memset(&bike1, 0, sizeof(bike1));
    memset(&bike2, 0, sizeof(bike2));
    flag = NULL;

    killer = exit = NULL;
    qframe = NULL;

    background = foreground = NULL;
    foreground_name[0] = background_name[0] = 0;

    food_count = 0;
    for (int i = 0; i < MAX_QFOOD; i++) {
        food[i] = NULL;
    }

    grass_pics = new grass;

    double zoom = EolSettings->zoom();

    // Megnyitja file-t:
    char lgrtmp[30];
    sprintf(lgrtmp, "lgr/%s.lgr", lgrname);
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

        // Osztalyozzuk nevet:
        // Eloszor kotelezo kepeket nezzuk:
        if (strcmpi(nev, "q1bike.pcx") == 0) {
            p1biker = ppic;
            palette_data = getnewpalfromh(h);
            timer_palette_map = makenegalttomb(palette_data);
            pal = new palette(palette_data);
            continue;
        }
        if (strcmpi(nev, "q2bike.pcx") == 0) {
            p2biker = ppic;
            continue;
        }

        // 1. motoros reszei:
        if (strcmpi(nev, "q1body.pcx") == 0) {
            // affine_pic konstructor delete-eli ppic-t:
            bike1.body = new affine_pic(NULL, ppic);
            continue;
        }
        if (strcmpi(nev, "q1thigh.pcx") == 0) {
            bike1.thigh = new affine_pic(NULL, ppic);
            continue;
        }
        if (strcmpi(nev, "q1leg.pcx") == 0) {
            bike1.leg = new affine_pic(NULL, ppic);
            continue;
        }

        if (strcmpi(nev, "q1wheel.pcx") == 0) {
            bike1.wheel = new affine_pic(NULL, ppic);
            continue;
        }
        if (strcmpi(nev, "q1susp1.pcx") == 0) {
            bike1.susp1 = new affine_pic(NULL, ppic);
            continue;
        }
        if (strcmpi(nev, "q1susp2.pcx") == 0) {
            bike1.susp2 = new affine_pic(NULL, ppic);
            continue;
        }
        if (strcmpi(nev, "q1forarm.pcx") == 0) {
            bike1.forarm = new affine_pic(NULL, ppic);
            continue;
        }
        if (strcmpi(nev, "q1up_arm.pcx") == 0) {
            bike1.up_arm = new affine_pic(NULL, ppic);
            continue;
        }
        if (strcmpi(nev, "q1head.pcx") == 0) {
            bike1.head = new affine_pic(NULL, ppic);
            continue;
        }

        // 2. motoros reszei:
        if (strcmpi(nev, "q2body.pcx") == 0) {
            // affine_pic konstructor delete-eli ppic-t:
            bike2.body = new affine_pic(NULL, ppic);
            continue;
        }
        if (strcmpi(nev, "q2thigh.pcx") == 0) {
            bike2.thigh = new affine_pic(NULL, ppic);
            continue;
        }
        if (strcmpi(nev, "q2leg.pcx") == 0) {
            bike2.leg = new affine_pic(NULL, ppic);
            continue;
        }

        if (strcmpi(nev, "q2wheel.pcx") == 0) {
            bike2.wheel = new affine_pic(NULL, ppic);
            continue;
        }
        if (strcmpi(nev, "q2susp1.pcx") == 0) {
            bike2.susp1 = new affine_pic(NULL, ppic);
            continue;
        }
        if (strcmpi(nev, "q2susp2.pcx") == 0) {
            bike2.susp2 = new affine_pic(NULL, ppic);
            continue;
        }
        if (strcmpi(nev, "q2forarm.pcx") == 0) {
            bike2.forarm = new affine_pic(NULL, ppic);
            continue;
        }
        if (strcmpi(nev, "q2up_arm.pcx") == 0) {
            bike2.up_arm = new affine_pic(NULL, ppic);
            continue;
        }
        if (strcmpi(nev, "q2head.pcx") == 0) {
            bike2.head = new affine_pic(NULL, ppic);
            continue;
        }

        if (strcmpi(nev, "qflag.pcx") == 0) {
            flag = new affine_pic(NULL, ppic);
            continue;
        }

        // Objektumok:
        if (strcmpi(nev, "qkiller.pcx") == 0) {
            killer = new anim(ppic, "qkiller.pcx", zoom);
            delete ppic;
            ppic = NULL;
            continue;
        }
        if (strcmpi(nev, "qexit.pcx") == 0) {
            exit = new anim(ppic, "qexit.pcx", zoom);
            delete ppic;
            ppic = NULL;
            continue;
        }

        if (strcmpi(nev, "qframe.pcx") == 0) {
            qframe = ppic;
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
                food[foodi] = new anim(ppic, tmpnev, zoom);
                delete ppic;
                ppic = NULL;
                ezfoodvolt = 1;
            }
        }
        if (ezfoodvolt) {
            continue;
        }

        // Kovetok:
        if (strnicmp(nev, "qup_", 4) == 0) {
            grass_pics->add(ppic, true);
            continue;
        }
        if (strnicmp(nev, "qdown_", 6) == 0) {
            grass_pics->add(ppic, false);
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
            add_texture(ppic, NULL, 0);
            continue;
        }

        int index = ppiclist->get_index(nev);
        if (index < 0) {
            external_error("There is no line in PICTURES.LST corresponding to picture:", nev);
        }
        if (ppiclist->type[index] == piclist::Type::Picture) {
            ppic = pic8::scale(ppic, zoom);
            add_picture(ppic, ppiclist, index);
            delete ppic; // Kepre mar nincsen tobbe szuksegunk
            ppic = NULL;
            continue;
        }
        if (ppiclist->type[index] == piclist::Type::Texture) {
            if (EolSettings->zoom_textures()) {
                ppic = pic8::scale(ppic, zoom);
            }
            add_texture(ppic, ppiclist, index);
            continue;
        }
        if (ppiclist->type[index] == piclist::Type::Mask) {
            ppic = pic8::scale(ppic, zoom);
            // delete-li ppic-et:
            add_mask(ppic, ppiclist, index);
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

    if (texture_count < 2) {
        external_error("There must be at least two textures in LGR file!", lgrname);
    }

    // 1. motoros:
    if (!bike1.body) {
        external_error("Picture not found in LGR file!: ", "q1body.pcx", lgrtmp);
    }
    if (!bike1.thigh) {
        external_error("Picture not found in LGR file!: ", "q1thigh.pcx", lgrtmp);
    }
    if (!bike1.leg) {
        external_error("Picture not found in LGR file!: ", "q1leg.pcx", lgrtmp);
    }

    if (!p1biker) {
        external_error("Picture not found in LGR file!: ", "q1bike.pcx", lgrtmp);
    }
    if (!bike1.wheel) {
        external_error("Picture not found in LGR file!: ", "q1wheel.pcx", lgrtmp);
    }
    if (!bike1.susp1) {
        external_error("Picture not found in LGR file!: ", "q1susp1.pcx", lgrtmp);
    }
    if (!bike1.susp2) {
        external_error("Picture not found in LGR file!: ", "q1susp2.pcx", lgrtmp);
    }
    if (!bike1.forarm) {
        external_error("Picture not found in LGR file!: ", "q1forarm.pcx", lgrtmp);
    }
    if (!bike1.up_arm) {
        external_error("Picture not found in LGR file!: ", "q1up_arm.pcx", lgrtmp);
    }
    if (!bike1.head) {
        external_error("Picture not found in LGR file!: ", "q1head.pcx", lgrtmp);
    }

    // 2. motoros:
    if (!bike2.body) {
        external_error("Picture not found in LGR file!: ", "q2body.pcx", lgrtmp);
    }
    if (!bike2.thigh) {
        external_error("Picture not found in LGR file!: ", "q2thigh.pcx", lgrtmp);
    }
    if (!bike2.leg) {
        external_error("Picture not found in LGR file!: ", "q2leg.pcx", lgrtmp);
    }

    if (!p2biker) {
        external_error("Picture not found in LGR file!: ", "q2bike.pcx", lgrtmp);
    }
    if (!bike2.wheel) {
        external_error("Picture not found in LGR file!: ", "q2wheel.pcx", lgrtmp);
    }
    if (!bike2.susp1) {
        external_error("Picture not found in LGR file!: ", "q2susp1.pcx", lgrtmp);
    }
    if (!bike2.susp2) {
        external_error("Picture not found in LGR file!: ", "q2susp2.pcx", lgrtmp);
    }
    if (!bike2.forarm) {
        external_error("Picture not found in LGR file!: ", "q2forarm.pcx", lgrtmp);
    }
    if (!bike2.up_arm) {
        external_error("Picture not found in LGR file!: ", "q2up_arm.pcx", lgrtmp);
    }
    if (!bike2.head) {
        external_error("Picture not found in LGR file!: ", "q2head.pcx", lgrtmp);
    }

    if (!flag) {
        external_error("Picture not found in LGR file!: ", "qflag.pcx", lgrtmp);
    }

    if (!killer) {
        external_error("Picture not found in LGR file!: ", "qkiller.pcx", lgrtmp);
    }
    if (!exit) {
        external_error("Picture not found in LGR file!: ", "qexit.pcx", lgrtmp);
    }

    if (!qframe) {
        external_error("Picture not found in LGR file!: ", "qframe.pcx", lgrtmp);
    }
    if (!pcolors) {
        external_error("Picture not found in LGR file!: ", "qcolors.pcx", lgrtmp);
    }

    // Elintezzuk biker picture-oket:
    chop_bike(p1biker, &bike1);
    delete p1biker;
    p1biker = NULL;

    chop_bike(p2biker, &bike2);
    delete p2biker;
    p2biker = NULL;

    // Elintezzuk index szineket:
    minimap_foreground_palette_id = pcolors->gpixel(6, 6 + 0 * 12);
    minimap_background_palette_id = pcolors->gpixel(6, 6 + 1 * 12);
    minimap_border_palette_id = pcolors->gpixel(6, 6 + 2 * 12);
    minimap_bike1_palette_id = pcolors->gpixel(6, 6 + 4 * 12);
    minimap_bike2_palette_id = pcolors->gpixel(6, 6 + 5 * 12);
    minimap_exit_palette_id[0] = pcolors->gpixel(6, 6 + 6 * 12);
    minimap_food_palette_id = pcolors->gpixel(6, 6 + 7 * 12);
    minimap_killer_palette_id[0] = pcolors->gpixel(6, 6 + 8 * 12);
    delete pcolors;
    pcolors = NULL;
    // ecsetben mutato fog hivatkozni rajuk (max 3 egymas utan):
    minimap_exit_palette_id[2] = minimap_exit_palette_id[1] = minimap_exit_palette_id[0];
    minimap_killer_palette_id[2] = minimap_killer_palette_id[1] = minimap_killer_palette_id[0];

    // Most texturakat kibovitjuk vizszintes iranyban:
    for (int i = 0; i < texture_count; i++) {
        texture* pt = &textures[i];
        if (!pt->pic) {
            internal_error("iuo4gt");
        }
        pt->original_width = pt->pic->get_width();
        int minxmintasize = 600;
        if (pt->pic->get_width() >= minxmintasize) {
            continue;
        }
        // Kep tul kicsi, kibovitjuk vizszintes iranyban:
        int szorzo = 1;
        while (szorzo * pt->pic->get_width() < minxmintasize) {
            szorzo++;
        }
        pic8* pbovitett = new pic8(szorzo * pt->pic->get_width(), pt->pic->get_height());
        for (int k = 0; k < szorzo; k++) {
            blit8(pbovitett, pt->pic, k * pt->pic->get_width(), 0);
        }
        delete pt->pic;
        pt->pic = pbovitett;
    }

    // Most kepeket abc sorrendbe rendezzuk:
    for (int i = 0; i < picture_count + 2; i++) {
        for (int j = 0; j < picture_count - 1; j++) {
            if (strcmpi(pictures[j].name, pictures[j + 1].name) == 0) {
                external_error("Picture name is duplicated in LGR file!: ", pictures[j].name);
            }

            if (strcmpi(pictures[j].name, pictures[j + 1].name) > 0) {
                // Csere:
                picture tmp = pictures[j];
                pictures[j] = pictures[j + 1];
                pictures[j + 1] = tmp;
            }
        }
    }

    // Most maszkokat abc sorrendbe rendezzuk:
    for (int i = 0; i < mask_count + 2; i++) {
        for (int j = 0; j < mask_count - 1; j++) {
            if (strcmpi(masks[j].name, masks[j + 1].name) == 0) {
                external_error("Mask name is duplicated in LGR file!: ", masks[j].name);
            }

            if (strcmpi(masks[j].name, masks[j + 1].name) > 0) {
                // Csere:
                mask tmp = masks[j];
                masks[j] = masks[j + 1];
                masks[j + 1] = tmp;
            }
        }
    }

    // Most texturakat abc sorrendbe rendezzuk:
    for (int i = 0; i < texture_count + 2; i++) {
        for (int j = 0; j < texture_count - 1; j++) {
            if (strcmpi(textures[j].name, textures[j + 1].name) == 0) {
                external_error("Texture name is duplicated in LGR file!: ", textures[j].name);
            }

            if (strcmpi(textures[j + 1].name, "qgrass") != 0 &&
                (strcmpi(textures[j].name, textures[j + 1].name) > 0 ||
                 strcmpi(textures[j].name, "qgrass") == 0)) { // qgrass vegere
                // Csere:
                texture tmp = textures[j];
                textures[j] = textures[j + 1];
                textures[j + 1] = tmp;
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

    editor_picture_name[0] = 0;
    editor_mask_name[0] = 0;
    editor_texture_name[0] = 0;

    // Most meg food-okat megszamoljuk:
    food_count = 0;
    for (int i = 0; i < 9; i++) {
        if (food[i]) {
            food_count = i + 1;
        } else {
            break;
        }
    }
    if (food_count < 1) {
        external_error("Picture is missing from LGR file:", "qfood1.pcx", lgrtmp);
    }

    has_grass = 1;
    editor_hide_qgrass = 1;
    if (get_texture_index("qgrass") < 0) {
        has_grass = 0;
        editor_hide_qgrass = 0;
    }
    if (grass_pics->length < 2) {
        // external_error( "Picture not found in LGR file!: ", "qgrass.pcx", lgrtmp );
        has_grass = 0;
    }
}

static void deletemkepekstruct(bike_pics* pk) {
    if (!pk->body || !pk->thigh || !pk->leg || !pk->bike_part1 || !pk->bike_part2 ||
        !pk->bike_part3 || !pk->bike_part4 || !pk->wheel || !pk->susp1 || !pk->susp2 ||
        !pk->forarm || !pk->up_arm || !pk->head) {
        internal_error("deletemkepekstruct-ban ervenytelen valamelyik affine_pic!");
    }

    delete pk->body;
    delete pk->thigh;
    delete pk->leg;
    delete pk->bike_part1;
    delete pk->bike_part2;
    delete pk->bike_part3;
    delete pk->bike_part4;
    delete pk->wheel;
    delete pk->susp1;
    delete pk->susp2;
    delete pk->forarm;
    delete pk->up_arm;
    delete pk->head;
}

lgrfile::~lgrfile(void) {
    for (int i = 0; i < picture_count; i++) {
        if (!pictures[i].data) {
            internal_error("987ty7");
        }
        delete pictures[i].data;
        pictures[i].data = NULL;
    }
    for (int i = 0; i < mask_count; i++) {
        if (!masks[i].data) {
            internal_error("gk7u6tk");
        }
        delete masks[i].data;
        masks[i].data = NULL;
    }
    for (int i = 0; i < texture_count; i++) {
        if (!textures[i].pic) {
            internal_error("6r5uy");
        }
        delete textures[i].pic;
        textures[i].pic = NULL;
    }

    if (!grass_pics) {
        internal_error(";oihki");
    }
    delete grass_pics;
    grass_pics = NULL;

    picture_count = mask_count = texture_count = 0;

    if (!pal || !palette_data || !timer_palette_map) {
        internal_error("lgrfile::~lgrfile-ban !palette || !pal_data || !timer_palette_map!");
    }
    delete pal;
    delete palette_data;
    delete timer_palette_map;
    pal = NULL;
    palette_data = NULL;
    timer_palette_map = NULL;

    deletemkepekstruct(&bike1);
    deletemkepekstruct(&bike2);
    if (!flag || !qframe) {
        internal_error("7246426!");
    }
    delete flag;
    delete qframe;
    flag = NULL;
    qframe = NULL;

    if (!killer || !exit) {
        internal_error("lgrfile::~lgrfile-ban ervenytelen valamelyik kep!");
    }

    delete killer;
    delete exit;
    killer = exit = NULL;

    food_count = 0;
    for (int i = 0; i < MAX_QFOOD; i++) {
        if (food[i]) {
            delete food[i];
            food[i] = NULL;
        }
    }

    if (background) {
        delete background;
        background = NULL;
    }
    if (foreground) {
        delete foreground;
        foreground = NULL;
    }
}

// Ezt betoltecseteket hivja (Ptop-bol szedi kep neveket):
void lgrfile::reload_default_textures(void) {
    if (!Ptop->foreground_name[0] || !Ptop->background_name[0]) {
        internal_error("ydsgyhi");
    }

    if (strcmpi(background_name, Ptop->background_name) != 0) {
        strcpy(background_name, Ptop->background_name);
        // Nem jo ami bent van:
        if (background) {
            delete background;
        }
        background = NULL;

        int index = get_texture_index(background_name);
        if (index < 0) {
            internal_error("oijhoe");
        }
        texture* pt = &textures[index];
        background_original_width = pt->original_width;
        background = kepetbovitvizszint(pt);
    }

    if (strcmpi(foreground_name, Ptop->foreground_name) != 0) {
        strcpy(foreground_name, Ptop->foreground_name);
        // Nem jo ami bent van:
        if (foreground) {
            delete foreground;
        }
        foreground = NULL;

        int index = get_texture_index(foreground_name);
        if (index < 0) {
            internal_error("879grtjoi");
        }
        texture* pt = &textures[index];
        foreground_original_width = pt->original_width;
        foreground = kepetbovitvizszint(pt);
    }
}

// Ha nincs ilye nevu, akkor -1-et ad vissza
int lgrfile::get_picture_index(const char* name) {
    if (!name[0]) {
        return -1;
    }
    for (int i = 0; i < picture_count; i++) {
        if (strcmpi(pictures[i].name, name) == 0) {
            return i;
        }
    }
    return -1;
}

// Ha nincs ilye nevu, akkor -1-et ad vissza
int lgrfile::get_mask_index(const char* name) {
    if (!name[0]) {
        return -1;
    }
    for (int i = 0; i < mask_count; i++) {
        if (strcmpi(masks[i].name, name) == 0) {
            return i;
        }
    }
    return -1;
}

// Ha nincs ilye nevu, akkor -1-et ad vissza
int lgrfile::get_texture_index(const char* name) {
    if (!name[0]) {
        return -1;
    }
    for (int i = 0; i < texture_count; i++) {
        if (strcmpi(textures[i].name, name) == 0) {
            return i;
        }
    }
    return -1;
}
