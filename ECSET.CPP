#include "ECSET.H"
#include "anim.h"
#include "EDITUJ.H"
#include "eol_settings.h"
#include "grass.h"
#include "polygon.h"
#include "level.h"
#include "lgr.h"
#include "main.h"
#include "M_PIC.H"
#include "menu_pic.h"
#include "object.h"
#include "physics_init.h"
#include "pic8.h"
#include "segments.h"
#include "sprite.h"
#include <cstdint>
#include <cmath>
#include <cstring>

typedef uintptr_t pixelek_t;

// Regen a file vegen volt egy kikomentezett eget keszit nevu fuggveny is
// hegyekhez, de ezt 99.05.19-en kiszedtem.

// static pic8* Egtmp = NULL;
// static pic8* Hegy1 = NULL;
// static pic8* Hegy2 = NULL;

ecset* Pecsetalso = NULL;
ecset* Pecsetfelso = NULL;
ecset* Pecsetview = NULL;

#define PIXELMASZK (~((uintptr_t)0x7FF))

/*void vizsgal( ecset* pecset ) {

    mdarab* futo = pecset->msorok[1071];
    int xtomb[40];
    int muttomb[40];
    int kurx = 1;
    int i = 0;
    while( futo ) {
        if( i > 38 )
            internal_error( "i > 38!" );

        xtomb[i] = kurx;
        muttomb[i] = (int)futo->pixelek;

        kurx += futo->xsize;

        i++;
        futo = futo->pkov;
    }
}*/

static void hibanincsmem(void) {
    external_error("You do not have enough memory to load this level!",
                   "Try to set the graphic detail to low at the options.");
}

mdarab* ecset::newmdarab(void) {
    if (tombbenkov >= TOMBBENMDARAB) {
        // Ez a tomb betelt:
        tombbenkov = 0;
        kurtomb->kovtomb = new mdarabtomb;
        if (!kurtomb->kovtomb) {
            hibanincsmem();
        }
        kurtomb = kurtomb->kovtomb;
        kurtomb->kovtomb = NULL;
        tombbenkov++;
        mdarab* ret = &kurtomb->tomb[tombbenkov - 1];
        ret->tavolsag = 0;
        return ret;
    }
    tombbenkov++;
    mdarab* ret = &kurtomb->tomb[tombbenkov - 1];
    ret->tavolsag = 0;
    return ret;
}

static void meghelyez(double* pd) {
    int pixel = (int)(*pd * MetersToPixels);
    *pd = (pixel + 0.5) / MetersToPixels;
}

void ecset::getorigoandsize(void) {
    // Megkeresi burkolokat:
    Segments->iterate_all_segments();
    segment* psz = Segments->next_segment();
    double minx = psz->r.x;
    double maxx = psz->r.x;
    double miny = psz->r.y;
    double maxy = psz->r.y;
    while (psz) {
        if (psz->r.x < minx) {
            minx = psz->r.x;
        }
        if (psz->r.x > maxx) {
            maxx = psz->r.x;
        }
        if (psz->r.y < miny) {
            miny = psz->r.y;
        }
        if (psz->r.y > maxy) {
            maxy = psz->r.y;
        }

        if (psz->r.x + psz->v.x < minx) {
            minx = psz->r.x + psz->v.x;
        }
        if (psz->r.x + psz->v.x > maxx) {
            maxx = psz->r.x + psz->v.x;
        }
        if (psz->r.y + psz->v.y < miny) {
            miny = psz->r.y + psz->v.y;
        }
        if (psz->r.y + psz->v.y > maxy) {
            maxy = psz->r.y + psz->v.y;
        }

        psz = Segments->next_segment();
    }

    if (view) {
        // VIEW VIEW VIEW VIEW VIEW VIEW VIEW VIEW VIEW VIEW
        // x-re jol ra lehet hagyni, nem noveli memoria foglalast:
        origo = vect2(minx - 10000 / MetersToPixels, miny - 4000 / MetersToPixels);
        xsizedb = (maxx + 10000 / MetersToPixels) - origo.x;
        ysizedb = (maxy + 4000 / MetersToPixels) - origo.y;
    } else {
        // x-re jol ra lehet hagyni, nem noveli memoria foglalast:
        origo = vect2(minx - 10000 / MetersToPixels, miny - 1000 / MetersToPixels);
        // Beallitjuk ugy, hogy mindig pixel felezo helyen legyen origo:
        meghelyez(&origo.x);
        meghelyez(&origo.y);

        xsizedb = (maxx + 10000 / MetersToPixels) - origo.x;
        ysizedb = (maxy + 1000 / MetersToPixels) - origo.y;
    }
}

void ecset::addszakasz(segment* psz) {
    vect2 r, v;
    r = psz->r - origo;
    r.x *= MetersToPixels;
    r.y *= MetersToPixels;
    v.x = psz->v.x * MetersToPixels;
    v.y = psz->v.y * MetersToPixels;
    if (view) {
        double szorzo = 1.0 / MinimapScaleFactor;
        r.x *= szorzo;
        r.y *= szorzo;
        v.x *= szorzo;
        v.y *= szorzo;
    }

    // Egy kicsit modositjuk palyat, hogy motoros egyforman erintse talajt,
    // ha kulonbozo iranyokban all:
    r.x -= 0.5;
    r.y -= 0.5;

    int foldjon = v.y < 0;

    // Alulrol felfele mutasson:
    if (v.y < 0) {
        r = r + v;
        v = Vect2null - v;
    }
    // Nem vizszintes-e?:
    if (v.y < 0.001) {
        return;
    }
    int y1 = (int)(r.y + 1.0);
    int y2 = (int)(r.y + v.y);

    /*double kozelseg = 0.000001;
    if( fabs( double( y1 )-(r.y+1) ) < kozelseg )
            internal_error( "Nagyon kozeli pont!" );
    if( fabs( double( y2 )-(r.y+v.y) ) < kozelseg )
            internal_error( "Nagyon kozeli pont!" );*/

    // Egyenes egyenletenek parameterei:
    double m = v.x / v.y;
    double dy1 = r.y;
    double dy2 = r.y + v.y;
    double dx1 = r.x;
    double dx2 = r.x + v.x;
    double a = (dx2 * dy1 - dx1 * dy2) / (dy1 - dy2);
    for (int y = y1; y <= y2; y++) {
        int x = (int)(m * y + a + 1.0);
        // if( fabs( double(x)-m*y+a+1.0 ) < kozelseg )
        //	internal_error( "Nagyon kozeli pont!" );

        // Hozzacsapjuk hatar pontot sorhoz:
        if (y < 10 || y >= sorszam) {
            internal_error("ecset::addszakasz y < 10 || y >= sorszam!");
        }
        if (x < 10 || x > maxx) {
            internal_error("ecset::addszakasz x < 10 || x > maxx!");
        }

        mdarab* pmd = newmdarab();
        pmd->pkov = NULL;
        pmd->xsize = x;
        if (foldjon) {
            pmd->pixelek = (unsigned char*)PX_FOLD;
        } else {
            pmd->pixelek = (unsigned char*)PX_EG;
        }

        mdarab* pfut = msorok[y];
        if (!pfut) {
            internal_error("ecset::addszakasz !pfut!");
        }
        while (pfut->pkov) {
            pfut = pfut->pkov;
        }
        pfut->pkov = pmd;
    }
}

static int sizeraallit(mdarab* elso) {
    int eleje = elso->xsize;
    mdarab* futo = elso;
    while (futo) {
        if (futo->pkov) {
            futo->xsize = futo->pkov->xsize - futo->xsize;
        } else {
            futo->xsize = 1000000;
        }

        futo = futo->pkov;
    }
    return eleje;
}

static void rendez(mdarab* elso) {
    // Megszamoljuk elemeket:
    int szam = 0;
    mdarab* futo = elso;
    while (futo) {
        futo = futo->pkov;
        szam++;
    }
    if (szam <= 0) {
        internal_error("(ecset) rendez-ben szam <= 0!");
    }

    if (szam < 2) {
        return;
    }

    // Sorba rendezunk:
    for (int j = 0; j < szam + 2; j++) {
        futo = elso;
        futo = futo->pkov;
        while (futo->pkov) {
            mdarab* pkov = futo->pkov;
            if (futo->xsize > pkov->xsize) {
                // Csere:
                int tmp = futo->xsize;
                futo->xsize = pkov->xsize;
                pkov->xsize = tmp;
                unsigned char* ptmp = futo->pixelek;
                futo->pixelek = pkov->pixelek;
                pkov->pixelek = ptmp;
            }
            futo = pkov;
        }
    }
    // Kikuszoboljuk egy x-en fekvo mdarab-okat:
    mdarab* futoelozo = elso;
    futo = futoelozo->pkov;
    while (futo->pkov) {
        mdarab* pkov = futo->pkov;
        if (futo->xsize == pkov->xsize) {
            if (futo->pixelek == pkov->pixelek) {
                // Azonosak, csak az egyiket vesszuk ki (pkov-et):
                futo->pkov = pkov->pkov;
            } else {
                // Kulonbozoek, mindkettot kiszedjuk:
                futoelozo->pkov = pkov->pkov;
            }
            // Visszaallunk lanc legelejere:
            futoelozo = elso;
            futo = futoelozo->pkov;
            if (!futo) {
                return;
            }
        } else {
            // Nem volt x-juk azonos, megyunk tovabb:
            futoelozo = futo;
            futo = pkov;
        }
    }

    // Kikuszoboljuk egymas utan fekvo, ugyanazt jelenot mdarab-okat:
    futo = elso;
    while (futo->pkov) {
        mdarab* pkov = futo->pkov;
        if (futo->pixelek == pkov->pixelek) {
            futo->pkov = pkov->pkov;
            // Visszaallunk lanc legelejere:
            futo = elso;
        } else {
            // Nem voltak azonosak, megyunk tovabb:
            futo = pkov;
        }
    }
}

// Rovid folddarabokat atalakitja konkret mutatova:
void ecset::foldmutatocsere(void) {
    for (int i = 0; i < sorszam; i++) {
        mdarab* fut = msorok[i];
        int xpos = kurxposok_A[i];
        if (xpos > 100) {
            internal_error("ecset::foldmutatocsere xpos > 100!");
        }
        while (fut) {
            if ((pixelek_t)fut->pixelek == PX_FOLD && fut->xsize < SCREEN_WIDTH) {
                int x = xpos % Lgr->foreground_original_width;
                int y = i % Lgr->foreground->get_height();
                fut->pixelek = Lgr->foreground->get_row(y) + x;
                // fut->pixelek = Lgr->foreground->get_row( 0 ); igy latszik
            }

            xpos += fut->xsize;
            fut = fut->pkov;
        }
    }
}

// Egymas utani azonos texturakat es ureseket lecsereli egy hosszabbra:
void ecset::duplaeliminacio(void) {
    for (int i = 0; i < sorszam; i++) {
        int voltvaltozas = 1;
        while (voltvaltozas) {
            voltvaltozas = 0;
            mdarab* fut = msorok[i];
            while (fut && fut->pkov) {
                mdarab* kov = fut->pkov;
                if (((pixelek_t)fut->pixelek & PIXELMASZK) == 0 && fut->pixelek == kov->pixelek) {
                    fut->pkov = kov->pkov;
                    fut->xsize += kov->xsize;
                    voltvaltozas = 1;
                }

                fut = fut->pkov;
            }
        }
    }
}

void ecset::textura2mutato(void) {
    for (int y = 0; y < sorszam; y++) {
        mdarab* fut = msorok[y];
        int xpos = kurxposok_A[y];
        if (xpos > 100) {
            internal_error("9865fd");
        }
        while (fut) {
            int hozzaad = fut->xsize;

            pixelek_t kepindex = (pixelek_t)fut->pixelek;
            if (((kepindex & PIXELMASZK) == 0) && (kepindex >= 10)) {
                kepindex -= 10;
// Itt van maszk, le kell cserelni:
#ifdef DEBUG
                if (kepindex >= Lgr->texture_count) {
                    internal_error("ecset::maszkokkitoltese kepindex >= Lgr->picture_count!");
                }
                if (Lgr->textures[kepindex].original_width <= 0) {
                    internal_error("7ywijohg!");
                }
#endif
                texture* pt = &Lgr->textures[kepindex];
                pic8* ppic = pt->pic;
                int megvan = fut->xsize;
                int x = xpos;
                int elso = 1;
                while (megvan > 0) {
                    if (!elso) {
                        // Beillesztunk egy uj mdarab-ot:
                        mdarab* ujdarab = newmdarab();
                        ujdarab->pkov = fut->pkov;
                        fut->pkov = ujdarab;
                        fut = ujdarab;
                    }
                    elso = 0;

                    x %= pt->original_width;
                    int size = megvan;
                    if (x + size > ppic->get_width()) {
                        size = ppic->get_width() - x;
                    }

                    fut->xsize = size;
                    fut->pixelek = ppic->get_row(y % ppic->get_height()) + x;
                    if (((pixelek_t)fut->pixelek & PIXELMASZK) == 0) {
                        external_error("Memory address is less than 2048 (723)!");
                    }

                    x += size;
                    megvan -= size;
                }
            }
            xpos += hozzaad;
            fut = fut->pkov;
        }
    }
}

static int megszamol(mdarab* elso) {
    // Megszamoljuk elemeket:
    int szam = 0;
    mdarab* futo = elso;
    while (futo) {
        futo = futo->pkov;
        szam++;
    }
    if (szam <= 0) {
        internal_error("(ecset) megszamol-ban szam <= 0!");
    }
    return szam;
}

int Osszegszam = 0;

void ecset::mutatotlanit(void) {
    // Megszamoljuk osszes mdarab-ot:
    int szam = 0;
    for (int i = 0; i < sorszam; i++) {
        szam += megszamol(msorok[i]);
    }
    Osszegszam += szam;
    // Most lefoglaljuk uj egybefuggo tombot:
    nagydarabtomb = new darab[szam + 10];
    if (!nagydarabtomb) {
        hibanincsmem();
    }
    int szammost = 0;
    for (int i = 0; i < sorszam; i++) {
        mdarab* futo = msorok[i];
        sorok[i] = &nagydarabtomb[szammost];
        curdarabok_A[i] = sorok[i];
        curdarabok_B[i] = sorok[i];
        while (futo) {
            nagydarabtomb[szammost].xsize = futo->xsize;
            nagydarabtomb[szammost].pixelek = futo->pixelek;

            futo = futo->pkov;
            szammost++;
        }
    }
}

// Ptop kerek-jeinek egesz koordinatait kitolti:
void ecset::kitoltfoodkoordokat(void) {
    const double offset = ANIM_WIDTH / 2.0 * EolSettings->zoom();
    for (int i = 0; i < MAX_OBJECTS; i++) {
        object* pker = Ptop->objects[i];
        if (!pker) {
            continue;
        }
        if (view) {
            pker->minimap_canvas_x =
                (int)((pker->r.x - origo.x) * MetersToPixels / MinimapScaleFactor);
            pker->minimap_canvas_y =
                (int)((-pker->r.y - origo.y) * MetersToPixels / MinimapScaleFactor);
        } else {
            pker->canvas_x = (int)((pker->r.x - origo.x) * MetersToPixels - offset);
            pker->canvas_y = (int)((-pker->r.y - origo.y) * MetersToPixels - offset);
        }
    }
}

enum { FAZIS_FOLD, FAZIS_EG, FAZIS_NEMFOLDEG, FAZIS_URESRE };

mdarab* ecset::beepitegymadarabnyit(mdarab* pmd, unsigned char* ujpixelek, int x1, int ujx1,
                                    int ujx2, int ujtavolsag, mdarab* vege, int* pujavege,
                                    int fazis) {

    if ((pixelek_t)ujpixelek == PX_URES) {
        internal_error("ecset::beepitegymadarabnyit-ban nem lehet ureset rarakni!");
    }

    if ((pixelek_t)pmd->pixelek == PX_URES && fazis != FAZIS_URESRE) {
        internal_error("ecset::beepitegymadarabnyit-ban",
                       "(int)ujpixelek == PX_URES && fazis != FAZIS_URESRE!");
    }

    // Eldontjuk, hogy ki kell-e egyaltalan rakni:
    int regitav = pmd->tavolsag;
    int nemrakjaki = 0;
    if (fazis == FAZIS_FOLD) {
        if ((pixelek_t)pmd->pixelek == PX_FOLD) {
            regitav = 1000000;
        }
        if ((pixelek_t)pmd->pixelek == PX_EG) {
            nemrakjaki = 1;
        }
    }
    if (fazis == FAZIS_EG) {
        ujtavolsag += 10000;
        if ((pixelek_t)pmd->pixelek == PX_FOLD) {
            nemrakjaki = 1;
        }
        if ((pixelek_t)pmd->pixelek == PX_EG) {
            regitav = 1000000;
        }
    }
    if (fazis == FAZIS_NEMFOLDEG) {
        if ((pixelek_t)pmd->pixelek == PX_FOLD || (pixelek_t)pmd->pixelek == PX_EG) {
            regitav = 1000000;
        }
    }

    if (fazis == FAZIS_URESRE) {
        if ((pixelek_t)pmd->pixelek == PX_FOLD || (pixelek_t)pmd->pixelek == PX_EG) {
            internal_error(
                "ecset::beepitegymadarabnyit fazis == FAZIS_LYUKRA, megis fold vagy eg!");
        }
        if ((pixelek_t)pmd->pixelek == PX_URES) {
            regitav = 1000000;
        }
    }

    if (nemrakjaki || ujtavolsag >= regitav) {
        *pujavege = 0;
        return pmd;
    }

    // Most mar biztos, hogy kirakjuk:
    int x2 = x1 + pmd->xsize - 1;
    // Levagdossuk ujx1, ujx2-t:
    if (ujx1 < x1) {
        ujx1 = x1;
    }
    if (ujx2 > x2) {
        ujx2 = x2;
    }
    if (ujx2 < x1 || ujx1 > x2) {
        internal_error("ecset::beepitegymadarabnyit ujx2 < x1 || ujx1 > x2!");
    }

    // Most kivalasztjuk melyik esettel allunk szemben:

    //  ------------------------------------
    //  I              XXXXXXXXXXXXXXXXXXXXI
    //  ------------------------------------

    if (x1 != ujx1 && x2 == ujx2) {
        mdarab* puj = newmdarab();
        puj->xsize = ujx2 - ujx1 + 1;
        puj->pixelek = ujpixelek;
        puj->tavolsag = ujtavolsag;
        puj->pkov = pmd->pkov;

        pmd->xsize -= puj->xsize;
        pmd->pkov = puj;

        *pujavege = 1;
        return puj;
    }

    //  ------------------------------------
    //  IXXXXXXXXXXXXXXXXXXXX              I
    //  ------------------------------------

    if (x1 == ujx1 && x2 != ujx2) {
        if (!vege) {
            internal_error("8796kuyg76");
        }
        // Ha minden megfelel, atallitjuk *pujavege-t:
        if (!(*pujavege) && vege->tavolsag == ujtavolsag &&
            !((pixelek_t)vege->pixelek & PIXELMASZK) && vege->pixelek == ujpixelek) {
            *pujavege = 1;
        }
        if (*pujavege) {
            int ujsize = ujx2 - ujx1 + 1;

            vege->xsize += ujsize;
            pmd->xsize -= ujsize;
            if ((pixelek_t)pmd->pixelek & PIXELMASZK) {
                pmd->pixelek += ujsize;
            }

            *pujavege = 0;
            return pmd;
        } else {
            mdarab* puj = newmdarab();
            vege->pkov = puj;

            puj->xsize = ujx2 - ujx1 + 1;
            puj->pixelek = ujpixelek;
            puj->tavolsag = ujtavolsag;
            puj->pkov = pmd;

            pmd->xsize -= puj->xsize;
            if ((pixelek_t)pmd->pixelek & PIXELMASZK) {
                pmd->pixelek += puj->xsize;
            }

            *pujavege = 0;
            return pmd;
        }
    }

    //  ------------------------------------
    //  IXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXI
    //  ------------------------------------

    if (x1 == ujx1 && x2 == ujx2) {
        if (!vege) {
            internal_error("6fg67r57fu6t");
        }
        // Ha minden megfelel, atallitjuk *pujavege-t:
        if (!(*pujavege) && vege->tavolsag == ujtavolsag &&
            !((pixelek_t)vege->pixelek & PIXELMASZK) && vege->pixelek == ujpixelek) {
            *pujavege = 1;
        }
        if (*pujavege) {
            vege->xsize += pmd->xsize;
            vege->pkov = pmd->pkov;

            *pujavege = 1;
            return vege;
        } else {
            pmd->pixelek = ujpixelek;
            pmd->tavolsag = ujtavolsag;

            *pujavege = 1;
            return pmd;
        }
    }

    //  ------------------------------------
    //  I             XXXXXXXXXXXX         I
    //  ------------------------------------

    if (x1 != ujx1 && x2 != ujx2) {
        mdarab* puj1 = newmdarab();
        mdarab* puj2 = newmdarab();

        puj1->xsize = ujx2 - ujx1 + 1;
        puj1->pixelek = ujpixelek;
        puj1->tavolsag = ujtavolsag;
        puj1->pkov = puj2;

        puj2->xsize = x2 - ujx2;
        if ((pixelek_t)pmd->pixelek & PIXELMASZK) {
            puj2->pixelek = pmd->pixelek + (ujx2 + 1 - x1);
        } else {
            puj2->pixelek = pmd->pixelek;
        }
        puj2->tavolsag = pmd->tavolsag;
        puj2->pkov = pmd->pkov;

        pmd->xsize -= puj1->xsize + puj2->xsize;
        pmd->pkov = puj1;

        *pujavege = 0;
        return puj2;
    }

    internal_error("ecset::beepitegymadarabnyit nem talalta meg esetet!");
    return NULL;
}

// static mdarab* Pmddd = NULL;

void ecset::addbytesor(unsigned char* ujpixelek, int ujtavolsag, int ujx1, int ujx2, int y,
                       int fazis) {
    // if( y != 1039 )
    //	return;

    // Beallunk pmd, xpos-sal elso szoba jovo madarab-ra:
    // Visszalepunk amig kell:
    mdarab* pmd = msorok[y];
    // Pmddd = pmd;
    int xpos = kurxposok_A[y];
    if (xpos > 10) {
        internal_error("ecset::addbytesor xpos > 10!");
    }
    // Elorelepunk amig kell:
    mdarab* vege = NULL;
    while (xpos + pmd->xsize - 1 < ujx1) {
        xpos += pmd->xsize;
        vege = pmd;
        pmd = pmd->pkov;
        if (!pmd) {
            internal_error("ecset::addbytesor !pmd (1)!");
        }
    }
    // Most rajta allunk elso mdarabon:
    int ujavege = 0;
    while (1) {
        if (!pmd) {
            internal_error("ecset::addbytesor !pmd (2)!");
        }
        int x1 = xpos;
        int x2 = xpos + pmd->xsize - 1;
        if (ujx2 < x1) {
            internal_error("ecset::addbytesor ujx2 < x1!");
        }

        // Beallitjuk kovetkezo ertekeket:
        int kovmarnincs = 0;
        if (x2 + 1 > ujx2) {
            kovmarnincs = 1;
        }

        unsigned char* kovujpixelek = ujpixelek;
        if ((pixelek_t)kovujpixelek & PIXELMASZK) {
            int kimaradxsize = 0; // Elso mdarab eseteben lehetseges csak
            if (ujx1 > x1) {
                kimaradxsize = ujx1 - x1;
            }
            kovujpixelek = ujpixelek + (pmd->xsize - kimaradxsize);
        }

        int kovxpos = xpos + pmd->xsize;
        mdarab* kovpmd = pmd->pkov;

        vege = beepitegymadarabnyit(pmd, ujpixelek, xpos, ujx1, ujx2, ujtavolsag, vege, &ujavege,
                                    fazis);

        if (kovmarnincs) {
            return;
        }
        ujpixelek = kovujpixelek;
        xpos = kovxpos;
        pmd = kovpmd;
    }
}

// addspriteok hivja:
void ecset::addegymaszk(sprite* psp, int index_t, int index_m, int fazis) {
    // Egy kep berakasa:
    int x1 = (int)((psp->r.x - origo.x) * MetersToPixels);
    int y1 = (int)((-psp->r.y - origo.y) * MetersToPixels);

    mask* pm = &Lgr->masks[index_m];

    int tavolsag = psp->distance;

    int xsize = pm->width;
    int ysize = pm->height;

    if (x1 < 120 || x1 + xsize >= maxx - 50) {
        return;
    }

    int i = 0;
    for (int y = 0; y < ysize; y++) {
        // Kep egy soranak elintezese:
        int x = 0;
        // Maszkos eljaras:
        while (pm->data[i].type != MaskEncoding::EndOfLine) {
            if (pm->data[i].type == MaskEncoding::Solid) {
                addbytesor((unsigned char*)(pixelek_t)(index_t + 10), tavolsag, x1 + x,
                           x1 + x + pm->data[i].length - 1, y1 - y, fazis);
            }
            x += pm->data[i].length;
            i++;
        }
        i++;
    }
}

void ecset::addspriteok(int fazis) {
    for (int j = 0; j < MAX_SPRITES; j++) {
        sprite* psp = Ptop->sprites[j];
        if (!psp) {
            return;
        }

        // Fazis fuggvenyeben csak bizonyos kepeket rakunk be:
        if (fazis == FAZIS_FOLD && psp->clipping != Clipping::Ground) {
            continue;
        }
        if (fazis == FAZIS_EG && psp->clipping != Clipping::Sky) {
            continue;
        }
        if (fazis == FAZIS_NEMFOLDEG && psp->clipping != Clipping::Unclipped) {
            continue;
        }

        if (psp->picture_name[0] == 0) {
            // Csak maszkos textura lehet, kulon intezzuk:
            if (!psp->texture_name[0] || !psp->mask_name[0]) {
                continue;
            }
            int index_t = Lgr->get_texture_index(psp->texture_name);
            if (index_t < 0) {
                internal_error("tgferg");
            }
            int index_m = Lgr->get_mask_index(psp->mask_name);
            if (index_m < 0) {
                internal_error("6t7ffrge");
            }

            // Ez egy maszk, innentol teljesen kulon intezzuk el:
            addegymaszk(psp, index_t, index_m, fazis);
            continue;
        }

        // Ez egy kep:
        if (psp->texture_name[0] || psp->mask_name[0]) {
            internal_error("gkuhlij");
        }

        int index = Lgr->get_picture_index(psp->picture_name);
        if (index < 0) {
            internal_error("kjhhgfchg");
        }
        picture* pkep = &Lgr->pictures[index];

        // Egy kep berakasa:
        int x1 = (int)((psp->r.x - origo.x) * MetersToPixels);
        int y1 = (int)((-psp->r.y - origo.y) * MetersToPixels);

        int ujtavolsag = psp->distance;

        int xsize = pkep->width;
        int ysize = pkep->height;

        if (x1 < 120 || x1 + xsize >= maxx - 50) {
            return;
        }

        unsigned char* tomb = pkep->data;

        if (((pixelek_t)tomb & PIXELMASZK) == 0) {
            external_error("Memory address is less than 2048 (987)!");
        }

        // Vegigmegyunk kepen:
        int i = 0; // byte index tombben
        for (int y = 0; y < ysize; y++) {
            // Kep egy soranak elintezese:
            int x = 0;
            while (1) {
                // Ures elintezes:
                int uresszam = tomb[i] * 256 + tomb[i + 1];
                i += 2;

                if (uresszam > 60000) {
                    break;
                }

                x += uresszam;

                // Teli elintezes:
                int teliszam = tomb[i] * 256 + tomb[i + 1];
                i += 2;

                // Negativ iranyban haladunk y-on (y1-y):
                if (y1 - y >= 0 && y1 - y < sorszam) {
                    addbytesor(&tomb[i], ujtavolsag, x1 + x, x1 + x + teliszam - 1, y1 - y, fazis);
                }

                x += teliszam;
                i += teliszam;
            }
        }
    }
}

// Kovetok beillesztese:

static int getatlaatszoszam(unsigned char* sor, int x, int xsize, unsigned char atlatszo) {
    int sum = 0;
    for (int i = x; i < xsize; i++) {
        if (sor[i] == atlatszo) {
            sum++;
        } else {
            return sum;
        }
    }
    return sum;
}

static int getanyagszam(unsigned char* sor, int x, int xsize, unsigned char atlatszo) {
    int sum = 0;
    for (int i = x; i < xsize; i++) {
        if (sor[i] != atlatszo) {
            sum++;
        } else {
            return sum;
        }
    }
    return sum;
}

#define KOVVONALHOSSZ (10000)
static int Kovetoytomb[KOVVONALHOSSZ + 10];

constexpr int Keltolas = 20;

void ecset::kovetotextura(grass* pkov, int index, int xo, int yo, int fazis) {
    if (!BufferMain) {
        internal_error("6dejuhfe");
    }

    // Most mindig felso:
    int felso = 1;
    int tavolsag = 600;

    char texturanev[20];
    strcpy(texturanev, "qgrass");

    // Kikeressuk textura indexet:
    int textindex = Lgr->get_texture_index(texturanev);
    if (textindex < 0) {
        internal_error("iygtyrt");
    }

    pic8* ppic = pkov->pics[index];
    int xsize = ppic->get_width();
    int ysize = ppic->get_height();
    unsigned char atlatszo = ppic->gpixel(0, 0);

    // Toroljuk seged kepet:
    // BufferMain: 0 -> atlatszo, 1 -> van textura:
    for (int y = 0; y < ysize; y++) {
        for (int x = 0; x < xsize; x++) {
            BufferMain->ppixel(x, y, 0);
        }
    }
    // Feltoltjuk segedkepet:
    for (int x = 0; x < xsize; x++) {
        // Minden oszlopot kitoltunk:
        if (felso) {
            for (int y = 0; y < ysize; y++) {
                if (ppic->gpixel(x, y) == atlatszo) {
                    BufferMain->ppixel(x, y, 1);
                } else {
                    break;
                }
            }
        } else {
            for (int y = ysize - 1; y >= 0; y--) {
                if (ppic->gpixel(x, y) == atlatszo) {
                    BufferMain->ppixel(x, y, 1);
                } else {
                    break;
                }
            }
        }
    }

    // Beadjuk textura darabokat ecset-be:
    for (int y = 0; y < ysize; y++) {
        int x = 0;
        unsigned char* sor = BufferMain->get_row(ysize - 1 - y);
        while (x < xsize) {
            x += getatlaatszoszam(sor, x, xsize, 0);
            if (x >= xsize) {
                break;
            }
            int anyagszam = getanyagszam(sor, x, xsize, 0);
            if (anyagszam <= 0) {
                internal_error("8toiurtg");
            }
            addbytesor((unsigned char*)(pixelek_t)(textindex + 10), tavolsag, xo + x,
                       xo + x + anyagszam - 1, yo + y, fazis);
            x += anyagszam;
        }
    }
    // Most beadjuk alul vagy felul megtoldott reszt is:
    int grass_height_padding = QGRASS_EXTRA_HEIGHT * EolSettings->zoom();
    if (felso) {
        // Felso:
        for (int y = 0; y < grass_height_padding; y++) {
            addbytesor((unsigned char*)(pixelek_t)(textindex + 10), tavolsag, xo, xo + xsize - 1,
                       yo + ysize + y, fazis);
        }
    } else {
        // Also:
        for (int y = 0; y < grass_height_padding; y++) {
            addbytesor((unsigned char*)(pixelek_t)(textindex + 10), tavolsag, xo, xo + xsize - 1,
                       yo - 1 - y, fazis);
        }
    }
}

void ecset::kiegykovetokep(grass* pkov, int index, int xo, int yo, int fazis) {

    int tavolsag = 600;

    pic8* ppic = pkov->pics[index];
    int xsize = ppic->get_width();
    int ysize = ppic->get_height();
    unsigned char atlatszo = ppic->gpixel(0, 0);

    // Kiszamoljuk kep tetejenek helyet:
    if (pkov->is_up[index]) {
        yo -= Keltolas;
    } else {
        yo -= ysize - 1 - Keltolas;
    }

    for (int y = 0; y < ysize; y++) {
        int x = 0;
        unsigned char* sor = ppic->get_row(ysize - 1 - y);
        while (x < xsize) {
            x += getatlaatszoszam(sor, x, xsize, atlatszo);
            if (x >= xsize) {
                break;
            }
            int anyagszam = getanyagszam(sor, x, xsize, atlatszo);
            if (anyagszam <= 0) {
                internal_error("8toiurtg");
            }
            addbytesor(&sor[x], tavolsag, xo + x, xo + x + anyagszam - 1, yo + y, fazis);
            x += anyagszam;
        }
    }

    if (xsize > 640 || ysize > 480) {
#ifdef DEBUG
        internal_error("7yfg4");
#endif
        return;
    }

    kovetotextura(pkov, index, xo, yo, fazis);
    // kovetotextura( 0, pkov, pgy, index, xo, yo, fazis );
}

void ecset::kikovetokepek(grass* pkov, int* ytomb, int hossz, int x0, int fazis) {
#ifdef DEBUG
    if (hossz < 1) {
        internal_error("dd387d3e");
    }
#else
    if (hossz < 1) {
        return;
    }
#endif
    int curx = x0;
    int cury = ytomb[0];
    while (1) {
        if (curx >= x0 + hossz) {
            return; // Befejezte
        }

        // Megkeressuk legjobb kepet:
        int elteres = 10000;
        int joindex = -1;
        int jody = 0;
        for (int i = 0; i < pkov->length; i++) {
            int dy = pkov->pics[i]->get_height();
            if (dy < 2 * Keltolas + 1) {
                char tmp[60];
                sprintf(tmp, "picture's y size is less than %d!", 2 * Keltolas + 1);
                external_error("In grass picture ", tmp);
            }
            dy -= 2 * Keltolas + 1;
            if (!pkov->is_up[i]) {
                dy = -dy;
            }

            int ujx = curx + pkov->pics[i]->get_width();
            int vonaly;
            if (ujx >= x0 + hossz) {
                vonaly = ytomb[hossz - 1];
            } else {
                vonaly = ytomb[ujx - x0];
            }

            int ujelteres = abs(cury + dy - vonaly);

            if (ujelteres < elteres) {
                elteres = ujelteres;
                joindex = i;
                jody = dy;
            }
        }

        // Megvan legjobb kep:
        if (joindex < 0) {
            internal_error("7f908ur5tg");
        }

        kiegykovetokep(pkov, joindex, curx, cury, fazis);

        curx += pkov->pics[joindex]->get_width();
        cury += jody;
    }
}

void ecset::addkovetok(int fazis) {
    if (!Lgr->has_grass) {
        return;
    }
    for (int i = 0; i < MAX_POLYGONS; i++) {
        polygon* pgy = Ptop->polygons[i];
        if (!pgy) {
            return;
        }
        if (!pgy->is_grass) {
            continue;
        }

        Clipping hatarol = Clipping::Ground;

        // Fazis fuggvenyeben csak bizonyos kepeket rakunk be:
        if (fazis == FAZIS_FOLD && hatarol != Clipping::Ground) {
            continue;
        }
        if (fazis == FAZIS_EG && hatarol != Clipping::Sky) {
            continue;
        }
        if (fazis == FAZIS_NEMFOLDEG && hatarol != Clipping::Unclipped) {
            continue;
        }

        grass* pkov = Lgr->grass_pics;

        int vonalhossz = 0, x0 = 0;
        if (!create_grass_polygon_heightmap(pgy, Kovetoytomb, &vonalhossz, &x0, KOVVONALHOSSZ,
                                            &origo)) {
            continue;
        }
        if (vonalhossz > KOVVONALHOSSZ) {
            internal_error("uh9fdw");
        }

        // Kirajzolunk egy minta vonalat kepre:
        /*static unsigned char kovetchar = 0;
        for( int x = x0; x < x0 + vonalhossz; x++ ) {
            addbytesor( &kovetchar, 100, x, x,
                        Kovetoytomb[x-x0], fazis );
        }*/

        kikovetokepek(pkov, Kovetoytomb, vonalhossz, x0, fazis);
    }
}

void ecset::addobjektumok(void) {
    if (!view) {
        return;
    }

    for (int i = 0; i < MAX_OBJECTS; i++) {
        object* pk = Ptop->objects[i];
        if (!pk) {
            return;
        }

        if (pk->type == object::Type::Food || pk->type == object::Type::Start ||
            pk->type == object::Type::Exit) {
            continue;
        }

        // Egy kep berakasa:
        if (view) {
            unsigned char* mut = Lgr->minimap_killer_palette_id;
            int ujtavolsag = 499;
            int x1 = (int)((pk->r.x - origo.x) * (MetersToPixels / MinimapScaleFactor));
            int y1 = (int)((-pk->r.y - origo.y) * (MetersToPixels / MinimapScaleFactor));
            // Kirakja kis 3x3-as gyurut:
            addbytesor(mut, ujtavolsag, x1 - 1, x1 + 1, y1 - 1, FAZIS_NEMFOLDEG);
            addbytesor(mut, ujtavolsag, x1 - 1, x1 - 1, y1, FAZIS_NEMFOLDEG);
            addbytesor(mut, ujtavolsag, x1 + 1, x1 + 1, y1, FAZIS_NEMFOLDEG);
            addbytesor(mut, ujtavolsag, x1 - 1, x1 + 1, y1 + 1, FAZIS_NEMFOLDEG);

            continue;
        }

        /* Ez a resz csak nagy objektumokhoz kellene, de most csak view-val
           foglalkozunk:
        int x1 = (pk->r.x-origo.x)*MetersToPixels;
        int y1 = (-pk->r.y-origo.y)*MetersToPixels;
        x1 -= ANIM_WIDTH/2;
        y1 += ANIM_WIDTH/2;

        pic8* ppic = Lgr->pexit;
        int ujtavolsag = 498;
        if( pk->type == object::Type::Killer ) {
            ppic = Lgr->pkiller;
            ujtavolsag = 499;
        }

        int xsize = ppic->get_width();
        int ysize = ppic->get_height();

        if( x1 < 120 || x1+xsize >= maxx-50 )
            return;

        // Van atlatszosag:
        unsigned char atlatszo = ppic->gpixel( 0, 0 );
        for( int y = 0; y < ysize; y++ ) {
            // Kep egy soranak elintezese:
            unsigned char* sor = ppic->get_row( ysize - 1 - y );
            if( ((int)sor & PIXELMASZK) == 0 )
                external_error( "Memory address is less than 2048 (654)!" );

            // Atugorja elso ures pixeleket:
            int x = uresszam( 0, xsize, sor, atlatszo );
            while( x <= xsize-1 ) {
                // Keresi kis szakaszokat:
                int size = teliszam( x, xsize, sor, atlatszo );
                if( size <= 0 )
                    internal_error( "ecset::addspriteok teliszam utan size <= 0!" );

                // Negativ iranyban haladunk y-on (y1-y):
                if( y1-y >= 0 && y1-y < sorszam )
                    addbytesor( &sor[x], ujtavolsag, x1+x, x1+x+size-1,
                                                    y1-y, FAZIS_NEMFOLDEG );
                x += size;

                // Atugorja ures pixeleket:
                x += uresszam( x, xsize, sor, atlatszo );
            }
        }*/
    }
}

void ecset::levon10000t(void) {
    for (int i = 0; i < sorszam; i++) {
        mdarab* pmd = msorok[i];
        while (pmd) {
            if (pmd->tavolsag >= 10000) {
                pmd->tavolsag -= 10000;
            }

            pmd = pmd->pkov;
        }
    }
};

// Ptop es Segments alapjan:
ecset::ecset(int view_p) {
    view = view_p;

    for (int i = 0; i < MAXECSETSOR; i++) {
        msorok[i] = NULL;
        sorok[i] = NULL;
        kurxposok_A[i] = 0;
        kurxposok_B[i] = 0;
        curdarabok_A[i] = NULL;
        curdarabok_B[i] = NULL;
    }

    // Inicializaljuk elso node tombot:
    elsotomb = kurtomb = NULL;
    tombbenkov = 0;

    nagydarabtomb = NULL;

    xsizedb = ysizedb = 0.0;
    maxx = sorszam = 0;

    if (!Ptop || !Segments) {
        internal_error("ecset::ecset !Ptop || !Segments!");
    }

    elsotomb = new mdarabtomb;
    if (!elsotomb) {
        internal_error("Nincs eleg memoria ecset::ecset!");
    }
    elsotomb->kovtomb = NULL;
    kurtomb = elsotomb;

    getorigoandsize();

    if (view) {
        // VIEW VIEW VIEW:
        maxx = (int)(xsizedb * (MetersToPixels / MinimapScaleFactor));
        sorszam = (int)(ysizedb * (MetersToPixels / MinimapScaleFactor));
    } else {
        maxx = (int)(xsizedb * MetersToPixels);
        sorszam = (int)(ysizedb * MetersToPixels);
    }

    if (sorszam - 10 > MAXECSETSOR) {
        internal_error("ecset::ecset sorszam-10 > MAXECSETSOR!");
    }

    // Inicializaljuk minden sor elejet foldre:
    for (int i = 0; i < sorszam; i++) {
        msorok[i] = newmdarab();
        msorok[i]->pkov = NULL;
        msorok[i]->xsize = 1;
        msorok[i]->tavolsag = 0;
        msorok[i]->pixelek = (unsigned char*)PX_FOLD;
    }

    // Vegigmegyunk osszes szakaszon:
    Segments->iterate_all_segments();
    segment* psz = Segments->next_segment();
    while (psz) {
        addszakasz(psz);
        psz = Segments->next_segment();
    }

    // Sorrendbe rendezzuk egy sor elemeit:
    for (int i = 0; i < sorszam; i++) {
        rendez(msorok[i]);
        kurxposok_A[i] = kurxposok_B[i] = sizeraallit(msorok[i]);
    }

    // Spriteok beadasa:
    if (!view && State->high_quality) {
        addspriteok(FAZIS_FOLD);
        addkovetok(FAZIS_FOLD);
        addspriteok(FAZIS_EG);
        addkovetok(FAZIS_EG);
        levon10000t();
        addspriteok(FAZIS_NEMFOLDEG);
        addkovetok(FAZIS_NEMFOLDEG);
    }

    // Itt adtuk be objektumokat:
    addobjektumok(); // Ez most nem view eseten visszater

    // Ezeket vegen kivulrol hivja loadecsetek:

    // Rovid folddarabokat atalakitjuk konkret mutatova:
    // foldmutatocsere();

    // mutatotlanit();
    // deletemdarabok();
}

// URES sorokat tesz bele:
// Ptop es Segments alapjan:
ecset::ecset(ecset* pold) {
    for (int i = 0; i < MAXECSETSOR; i++) {
        msorok[i] = NULL;
        sorok[i] = NULL;
        kurxposok_A[i] = 0;
        kurxposok_B[i] = 0;
        curdarabok_A[i] = NULL;
        curdarabok_B[i] = NULL;
    }

    // Inicializaljuk elso node tombot:
    elsotomb = kurtomb = NULL;
    tombbenkov = 0;

    nagydarabtomb = NULL;

    view = 0;
    xsizedb = ysizedb = 0.0;
    maxx = sorszam = 0;

    if (!Ptop || !Segments) {
        internal_error("ecset::ecset !Ptop || !Segments!");
    }

    elsotomb = new mdarabtomb;
    if (!elsotomb) {
        internal_error("Nincs eleg memoria ecset::ecset!");
    }
    elsotomb->kovtomb = NULL;
    kurtomb = elsotomb;

    // getorigoandsize(); ehelyett:
    origo = pold->origo;
    xsizedb = pold->xsizedb;
    ysizedb = pold->ysizedb;

    maxx = pold->maxx;
    sorszam = pold->sorszam;

    // Inicializaljuk minden sor elejet uresre:
    for (int i = 0; i < sorszam; i++) {
        kurxposok_A[i] = 1;
        kurxposok_B[i] = 1;

        msorok[i] = newmdarab();
        msorok[i]->pkov = NULL;
        msorok[i]->xsize = 1000000;
        msorok[i]->tavolsag = 0;
        msorok[i]->pixelek = (unsigned char*)PX_URES;
    }
}

ecset::~ecset(void) {
    if (elsotomb) {
        internal_error("ecset::~ecset elsotomb");
    }
    if (!nagydarabtomb) {
        internal_error("ecset::~ecset !nagydarabtomb!");
    }
    delete nagydarabtomb;
    nagydarabtomb = NULL;
}

void ecset::deletemdarabok(void) {
    if (!elsotomb) {
        internal_error("ecset::deletemdarabok !elsotomb!");
    }
    mdarabtomb* cur = elsotomb;
    while (cur) {
        mdarabtomb* kov = cur->kovtomb;
        delete cur;
        cur = kov;
    }
    elsotomb = NULL;
    for (int i = 0; i < MAXECSETSOR; i++) {
        msorok[i] = NULL;
    }
}

static int Xe1, Xe2;

unsigned char *Foldsor = NULL, *Egsor = NULL;

static inline void draw_sky(unsigned char* pc, size_t offset, size_t size, bool minimap) {
    if (!minimap) {
        memcpy(pc, Foldsor + offset, size);
    } else {
        memset(pc, Lgr->minimap_foreground_palette_id, size);
    }
}

static inline void draw_default_ground(unsigned char* pc, size_t offset, size_t size,
                                       bool minimap) {
    if (!minimap) {
        memcpy(pc, Egsor + offset, size);
    } else {
        memset(pc, Lgr->minimap_background_palette_id, size);
    }
}

void ecset::kiegysor_A(unsigned char* pc, int ye) {
    // Visszalepunk amig kell:
    darab* pda = curdarabok_A[ye];
    int xpos = kurxposok_A[ye];
    while (xpos > Xe1) {
        pda--;
        xpos -= pda->xsize;
    }
    // Elorelepunk amig kell:
    while (xpos + pda->xsize <= Xe1) {
        xpos += pda->xsize;
        pda++;
    }
    // Elmentjuk kurrens darabot:
    curdarabok_A[ye] = pda;
    kurxposok_A[ye] = xpos;

    // ELSO SZAKASZ      ELSO SZAKASZ       ELSO SZAKASZ
    // ELSO SZAKASZ      ELSO SZAKASZ       ELSO SZAKASZ
    // ELSO SZAKASZ      ELSO SZAKASZ       ELSO SZAKASZ
    // ELSO SZAKASZ      ELSO SZAKASZ       ELSO SZAKASZ
    int ures = Xe1 - xpos;
    if (xpos + pda->xsize > Xe2) {
        // ATLOG VEGIG:
        int ujpdaxsize = Xe2 - xpos + 1;
        // Ha eppen vegeig log, akkor is ide jon
        if ((pixelek_t)pda->pixelek & PIXELMASZK) {
            // Normal kep copi:
            memcpy(pc, &pda->pixelek[Xe1 - xpos], ujpdaxsize - ures);
        } else {
            // Eg, Fold, vagy ures:
            if (!pda->pixelek) { // Eg
                draw_default_ground(pc, 0, ujpdaxsize - ures, view);
                return;
            }
            if ((pixelek_t)pda->pixelek == PX_FOLD) {
                draw_sky(pc, 0, ujpdaxsize - ures, view);
            } else {
#ifdef DEBUG
                if ((pixelek_t)pda->pixelek != PX_URES) {
                    internal_error("(pixelek_t)pda->pixelek != PX_URES iueghyreg!");
                }
#endif
            }
        }
        return;
    }

    // Elso szakasz nem log at vegig:
    int size = pda->xsize - ures;
    if ((pixelek_t)pda->pixelek & PIXELMASZK) {
        // Normal kep copi:
        memcpy(pc, &pda->pixelek[Xe1 - xpos], size);
    } else {
        // Eg, Fold, vagy ures:
        if (!pda->pixelek) { // Eg
            draw_default_ground(pc, 0, size, view);
        } else {
            if ((pixelek_t)pda->pixelek == PX_FOLD) {
                draw_sky(pc, 0, size, view);
            } else {
#ifdef DEBUG
                if ((pixelek_t)pda->pixelek != PX_URES) {
                    internal_error("(pixelek_t)pda->pixelek != PX_URES iygvfrgiyvr!");
                }
#endif
            }
        }
    }
    // Atugrunk kovetkezore:
    xpos += pda->xsize;
    pda++;
    pc += size;

    // BENT KEZDODO SZAKASZOK          BENT KEZDODO SZAKASZOK
    // BENT KEZDODO SZAKASZOK          BENT KEZDODO SZAKASZOK
    // BENT KEZDODO SZAKASZOK          BENT KEZDODO SZAKASZOK
    // BENT KEZDODO SZAKASZOK          BENT KEZDODO SZAKASZOK
    while (xpos <= Xe2) {
        if (xpos + pda->xsize > Xe2) {
            // Ez az utolso darab, ki is loghat:
            if ((pixelek_t)pda->pixelek & PIXELMASZK) {
                // Normal kep copi:
                memcpy(pc, pda->pixelek, Xe2 - xpos + 1);
                return;
            }
            if (!pda->pixelek) {
                draw_default_ground(pc, xpos - Xe1, Xe2 - xpos + 1, view);
                return;
            }
            if ((pixelek_t)pda->pixelek == PX_FOLD) {
                draw_sky(pc, xpos - Xe1, Xe2 - xpos + 1, view);
                return;
            }
#ifdef DEBUG
            if ((pixelek_t)pda->pixelek != PX_URES) {
                internal_error("(pixelek_t)pda->pixelek != PX_URES 653wetgki!");
            }
#endif
        }

        // Ez a darab nem log ki:
        if ((pixelek_t)pda->pixelek & PIXELMASZK) {
            // Normal kep copi:
            memcpy(pc, pda->pixelek, pda->xsize);
        } else {
            if (!pda->pixelek) {
                draw_default_ground(pc, xpos - Xe1, pda->xsize, view);
            } else {
                if ((pixelek_t)pda->pixelek == PX_FOLD) {
                    draw_sky(pc, xpos - Xe1, pda->xsize, view);
                } else {
#ifdef DEBUG
                    if ((pixelek_t)pda->pixelek != PX_URES) {
                        internal_error("(pixelek_t)pda->pixelek != PX_URES 653wetgki!");
                    }
#endif
                }
            }
        }

        xpos += pda->xsize;
        pc += pda->xsize;
        pda++;
    }
}

void ecset::kiegysor_B(unsigned char* pc, int ye) {
    // Visszalepunk amig kell:
    darab* pda = curdarabok_B[ye];
    int xpos = kurxposok_B[ye];
    while (xpos > Xe1) {
        pda--;
        xpos -= pda->xsize;
    }
    // Elorelepunk amig kell:
    while (xpos + pda->xsize <= Xe1) {
        xpos += pda->xsize;
        pda++;
    }
    // Elmentjuk kurrens darabot:
    curdarabok_B[ye] = pda;
    kurxposok_B[ye] = xpos;

    // ELSO SZAKASZ      ELSO SZAKASZ       ELSO SZAKASZ
    // ELSO SZAKASZ      ELSO SZAKASZ       ELSO SZAKASZ
    // ELSO SZAKASZ      ELSO SZAKASZ       ELSO SZAKASZ
    // ELSO SZAKASZ      ELSO SZAKASZ       ELSO SZAKASZ
    int ures = Xe1 - xpos;
    if (xpos + pda->xsize > Xe2) {
        // ATLOG VEGIG:
        int ujpdaxsize = Xe2 - xpos + 1;
        // Ha eppen vegeig log, akkor is ide jon
        if ((pixelek_t)pda->pixelek & PIXELMASZK) {
            // Normal kep copi:
            memcpy(pc, &pda->pixelek[Xe1 - xpos], ujpdaxsize - ures);
        } else {
            // Eg, Fold, vagy ures:
            if (!pda->pixelek) { // Eg
                draw_default_ground(pc, 0, ujpdaxsize - ures, view);
                return;
            }
            if ((pixelek_t)pda->pixelek == PX_FOLD) {
                draw_sky(pc, 0, ujpdaxsize - ures, view);
            } else {
#ifdef DEBUG
                if ((pixelek_t)pda->pixelek != PX_URES) {
                    internal_error("(pixelek_t)pda->pixelek != PX_URES iueghyreg!");
                }
#endif
            }
        }
        return;
    }

    // Elso szakasz nem log at vegig:
    int size = pda->xsize - ures;
    if ((pixelek_t)pda->pixelek & PIXELMASZK) {
        // Normal kep copi:
        memcpy(pc, &pda->pixelek[Xe1 - xpos], size);
    } else {
        // Eg, Fold, vagy ures:
        if (!pda->pixelek) { // Eg
            draw_default_ground(pc, 0, size, view);
        } else {
            if ((pixelek_t)pda->pixelek == PX_FOLD) {
                draw_sky(pc, 0, size, view);
            } else {
#ifdef DEBUG
                if ((pixelek_t)pda->pixelek != PX_URES) {
                    internal_error("(pixelek_t)pda->pixelek != PX_URES iygvfrgiyvr!");
                }
#endif
            }
        }
    }
    // Atugrunk kovetkezore:
    xpos += pda->xsize;
    pda++;
    pc += size;

    // BENT KEZDODO SZAKASZOK          BENT KEZDODO SZAKASZOK
    // BENT KEZDODO SZAKASZOK          BENT KEZDODO SZAKASZOK
    // BENT KEZDODO SZAKASZOK          BENT KEZDODO SZAKASZOK
    // BENT KEZDODO SZAKASZOK          BENT KEZDODO SZAKASZOK
    while (xpos <= Xe2) {
        if (xpos + pda->xsize > Xe2) {
            // Ez az utolso darab, ki is loghat:
            if ((pixelek_t)pda->pixelek & PIXELMASZK) {
                // Normal kep copi:
                memcpy(pc, pda->pixelek, Xe2 - xpos + 1);
                return;
            }
            if (!pda->pixelek) {
                draw_default_ground(pc, xpos - Xe1, Xe2 - xpos + 1, view);
                return;
            }
            if ((pixelek_t)pda->pixelek == PX_FOLD) {
                draw_sky(pc, xpos - Xe1, Xe2 - xpos + 1, view);
                return;
            }
#ifdef DEBUG
            if ((pixelek_t)pda->pixelek != PX_URES) {
                internal_error("(pixelek_t)pda->pixelek != PX_URES 653wetgki!");
            }
#endif
        }

        // Ez a darab nem log ki:
        if ((pixelek_t)pda->pixelek & PIXELMASZK) {
            // Normal kep copi:
            memcpy(pc, pda->pixelek, pda->xsize);
        } else {
            if (!pda->pixelek) {
                draw_default_ground(pc, xpos - Xe1, pda->xsize, view);
            } else {
                if ((pixelek_t)pda->pixelek == PX_FOLD) {
                    draw_sky(pc, xpos - Xe1, pda->xsize, view);
                } else {
#ifdef DEBUG
                    if ((pixelek_t)pda->pixelek != PX_URES) {
                        internal_error("(pixelek_t)pda->pixelek != PX_URES 653wetgki!");
                    }
#endif
                }
            }
        }

        xpos += pda->xsize;
        pc += pda->xsize;
        pda++;
    }
}

void ecset::getbalalso_int(vect2 balalso, int* px, int* py) {
    vect2 relat = balalso - origo;
    if (view) {
        *px = (int)(relat.x * (MetersToPixels / MinimapScaleFactor)); // Ecset egesz koordinatak
        *py = (int)(relat.y * (MetersToPixels / MinimapScaleFactor)); // Ecset egesz koordinatak
    } else {
        *px = (int)(relat.x * MetersToPixels); // Ecset egesz koordinatak
        *py = (int)(relat.y * MetersToPixels); // Ecset egesz koordinatak
    }
}

/*static int Elsoegkeszit = 1;
static pic8* Pegbuffer = NULL;
static pic8* Felho = NULL;
static pic8* Felho2 = NULL;

static void egetkeszit( pic8* peg, int xhely, int xkezd, int xmodulus ) {
    if( Elsoegkeszit ) {
        Elsoegkeszit = 0;
        Pegbuffer = new pic8( peg->get_width(), peg->get_height() );
        blit8( Pegbuffer, peg );

        Felho = new pic8( "felho.pcx" );
        forditkepet( Felho );
        spriteosit( Felho );

        Felho2 = new pic8( "felho2.pcx" );
        forditkepet( Felho2 );
        spriteosit( Felho2 );
    }

    // Resetaljuk eget:
    blit8( peg, Pegbuffer );

    // Felho2:
    double lassitas = 2.5;
    int modulus = 1000;
    int dx = -xhely/lassitas;
    while( dx < 0 )
        dx += modulus;
    dx -= modulus;
    int x = xkezd+dx;
    blit8( peg, Felho2, x, -80 );
    x += modulus;
    blit8( peg, Felho2, x, -80 );

    // Felho1:
    lassitas = 1.5;
    modulus = 1000;
    dx = -xhely/lassitas;
    while( dx < 0 )
        dx += modulus;
    dx -= modulus;
    x = xkezd+dx;
    blit8( peg, Felho, x, 0 );
    x += modulus;
    blit8( peg, Felho, x, 0 );
} */

void ecset::kitesz(int ajatekos, pic8* ppic, vect2 balalso, int x1, int y1, int x2, int y2) {
    if (x1 >= x2 || y1 >= y2) {
        internal_error("ecset::kitesz x1 >= x2 || y1 >= y2!");
    }

    int ye1 = 0;
    getbalalso_int(balalso, &Xe1, &ye1);
    Xe2 = Xe1 + x2 - x1;
    int ye2 = ye1 + y2 - y1;

    if (Xe1 < 20 || Xe2 > maxx - 20 || ye1 < 20 || ye2 > sorszam - 20) {
        internal_error("ecset::kitesz Xe1 < 20 || Xe2 > maxx-20 || ye1 < 20 || ye2 > sorszam-20!");
    }

    int foldymodulus = Lgr->foreground->get_height();
    int egymodulus = Lgr->background->get_height();

    int folddx = Xe1 % Lgr->foreground_original_width;
    // int eglassitas = 100;
    int eglassitas = 2;
    int egdx = (Xe1 / eglassitas) % Lgr->background_original_width;

    // egetkeszit( Lgr->peg, Xe1, egdx, Lgr->background_original_width );

    int yadd = ye1 - y1;
    if (ajatekos) {
        for (int y = y1; y <= y2; y++) {
            int ye = y + yadd;
            Foldsor = Lgr->foreground->get_row(ye % foldymodulus) + folddx;
            // Eg fol-le is scrollozodik:
            // Egsor = Lgr->peg->get_row( (y-y1+egdy) % egymodulus ) + egdx;

            // Eg fol-le nem scrollozodik:
            Egsor = Lgr->background->get_row((y - y1) % egymodulus) + egdx;
            kiegysor_A(ppic->get_row(y) + x1, ye);
        }
    } else {
        for (int y = y1; y <= y2; y++) {
            int ye = y + yadd;
            Foldsor = Lgr->foreground->get_row(ye % foldymodulus) + folddx;
            // Egsor = Lgr->peg->get_row( (ye+egdy) % egymodulus ) +
            //				egdx;
            Egsor = Lgr->background->get_row((y - y1) % egymodulus) + egdx;
            kiegysor_B(ppic->get_row(y) + x1, ye);
        }
    }

    Foldsor = nullptr;
    Egsor = nullptr;
}

// Szinte ugyanaz, mint kiteszalso:
void ecset::kiteszview(int ajatekos, pic8* ppic, vect2 balalso, int x1, int y1, int x2, int y2) {
    if (x1 >= x2 || y1 >= y2) {
        internal_error("ecset::kitesz x1 >= x2 || y1 >= y2!");
    }

    int ye1 = 0;
    getbalalso_int(balalso, &Xe1, &ye1);
    Xe2 = Xe1 + x2 - x1 + 1;
    int ye2 = ye1 + y2 - y1 + 1;

    if (Xe1 < 20 || Xe2 > maxx - 20 || ye1 < 20 || ye2 > sorszam - 20) {
        internal_error("ecset::kitesz Xe1 < 20 || Xe2 > maxx-20 || ye1 < 20 || ye2 > sorszam-20!");
    }

    int yadd = ye1 - y1;
    if (ajatekos) {
        for (int y = y1; y <= y2; y++) {
            int ye = y + yadd;
            kiegysor_A(ppic->get_row(y) + x1, ye);
        }
    } else {
        for (int y = y1; y <= y2; y++) {
            int ye = y + yadd;
            kiegysor_B(ppic->get_row(y) + x1, ye);
        }
    }
}

static mdarab* Glb_pfoltdarab = NULL;

static int ezfolt(int x, int y, mdbiter* piter1) {
    if (!Glb_pfoltdarab) {
        internal_error("65rw5r7twe");
    }

    // Felette levo sort vizsgaljuk:
    mdarab* pmd = piter1->getpmd(x, y + 1);

    pixelek_t kepindex = (pixelek_t)pmd->pixelek;
    if (kepindex < 10) {
        // Ha fold, eg, vagy ures:
        return 0;
    }

    if (pmd->pixelek == Glb_pfoltdarab->pixelek) {
        // Ha meg ez is koveto textura:
        return 0;
    }

    if (pmd->tavolsag > 500) {
        // Ha nem takarja el motorost:
        return 0;
    }

    return 1;
}

static int getnemfoltszam(int x1, int xkov, int y, mdbiter* piter1) {
    int szam = 0;
    for (int x = x1; x < xkov; x++) {
        if (ezfolt(x, y, piter1)) {
            return szam;
        }
        szam++;
    }
    return szam;
}

static int getfoltszam(int x1, int xkov, int y, mdbiter* piter1) {
    int szam = 0;
    for (int x = x1; x < xkov; x++) {
        if (!ezfolt(x, y, piter1)) {
            return szam;
        }
        szam++;
    }
    return szam;
}

// Meg akkor kell hivni, amig nincsenek kettevalasztva also-ra es felsore es
// meg kis folddarabok sincsenek lecserelve:
void ecset::foltoz(void) {
    mdbiter* piter1 = new mdbiter(this);
    // addbyte-ban tavolsagot valtakozva adjuk meg, nehogy osszeadja:
    int valtakozo = 0;

    // Vegigmegyunk osszes soron alulrol felfele:
    // Azert noveljuk megis y-t, mert fejjel lefele vannak y koordinatak:
    for (int y = 10; y < sorszam - 10; y++) {
        // Vegigmegyunk osszes mdarabon:
        mdarab* pmd = msorok[y];
        int xpos = kurxposok_A[y];
        if (xpos > 10) {
            internal_error("uih67f");
        }

        while (pmd) {
            int pmdorigxsize = pmd->xsize;
            mdarab* pmdorigkov = pmd->pkov;

            Glb_pfoltdarab = pmd;
            // Egy madarab-ot elintezunk:
            pixelek_t kepindex = (pixelek_t)pmd->pixelek;
            if (((kepindex & PIXELMASZK) == 0) && (kepindex >= 10)) {
                kepindex -= 10;
// Itt van maszk, le kell cserelni:
#ifdef DEBUG
                if (kepindex >= Lgr->texture_count) {
                    internal_error("egryiegr");
                }
#endif
                texture* pt = &Lgr->textures[kepindex];

                if (pt->is_qgrass && pmd->tavolsag > 500) {
                    // Ha egy foltos, 500-nal tavolabbi textura mdarabja:
                    // Vegigmegyunk mdarab-on:
                    int x = xpos;
                    // Ez mar kov mdarab elso pixele:
                    int xkov = xpos + pmd->xsize;
                    while (x < xkov) {
                        int nfszam = getnemfoltszam(x, xkov, y, piter1);
                        x += nfszam;
                        int foltszam = getfoltszam(x, xkov, y, piter1);
                        if (foltszam > 0) {
                            valtakozo = !valtakozo;
                            if (valtakozo) {
                                addbytesor( //(unsigned char*)10,
                                    pmd->pixelek, 223, x, x + foltszam - 1, y, FAZIS_NEMFOLDEG);
                            } else {
                                addbytesor( //(unsigned char*)10,
                                    pmd->pixelek, 224, x, x + foltszam - 1, y, FAZIS_NEMFOLDEG);
                            }
                        }
                        x += foltszam;
                    }
                }
            }

            // Ugrunk kovetkezo mdarab-ra:
            xpos += pmdorigxsize;
            pmd = pmdorigkov;
        }
    }
    delete piter1;
    piter1 = NULL;
}

void betoltecseteket(void) {
    Lgr->reload_default_textures();

    Osszegszam = 0; // Ide osszegezzuk osszes lefoglalt szakaszt

    if (Pecsetalso) {
        delete Pecsetalso;
    }
    Pecsetalso = NULL;
    if (Pecsetfelso) {
        delete Pecsetfelso;
    }
    Pecsetfelso = NULL;
    if (Pecsetview) {
        delete Pecsetview;
    }
    Pecsetview = NULL;

    Pecsetalso = new ecset(0);
    if (!Pecsetalso) {
        internal_error("Nincs eleg memoria Pecsetalso-nak!");
    }

    if (!EolSettings->pictures_in_background()) {
        Pecsetalso->foltoz();
    }

    Pecsetfelso = new ecset(Pecsetalso);
    if (!Pecsetfelso) {
        internal_error("Nincs eleg memoria Pecsetfelso-nak!");
    }

    if (!EolSettings->pictures_in_background()) {
        // Most betesszuk felsobe is 500-nal kozelebbi madarabokat:
        // Es egyben uresse tesszuk alsoban helyuket:
        for (int i = 0; i < Pecsetalso->sorszam; i++) {
            mdarab* pmd = Pecsetalso->msorok[i];
            int xpos = Pecsetalso->kurxposok_A[i];
            if (xpos > 100) {
                internal_error("betoltecseteket xpos > 100!");
            }
            while (pmd) {
                if (pmd->tavolsag < 500) {
                    if ((pixelek_t)pmd->pixelek >= 10) {
                        Pecsetfelso->addbytesor(pmd->pixelek, pmd->tavolsag, xpos,
                                                xpos + pmd->xsize - 1, i, FAZIS_URESRE);
                        pmd->pixelek = (unsigned char*)PX_URES;
                    }
                }
                xpos += pmd->xsize;
                pmd = pmd->pkov;
            }
        }
    }

    // Rovid folddarabokat atalakitja konkret mutatova:
    Pecsetalso->foldmutatocsere();
    // Egymas utani ket azonos texturat
    // lecsereli egyre (mar beepiteskor is megtortenik nemileg):
    Pecsetalso->duplaeliminacio();
    Pecsetalso->textura2mutato(); // Lecsereli textura indexeket mutatokka
    Pecsetalso->mutatotlanit();   // mdarab lancolt listabol tombot csinal
    Pecsetalso->deletemdarabok(); // Torli lancolt listat:

    Pecsetfelso->duplaeliminacio();
    Pecsetfelso->textura2mutato();
    Pecsetfelso->mutatotlanit();
    Pecsetfelso->deletemdarabok();

    // View ecset:
    Pecsetview = new ecset(1);
    if (!Pecsetview) {
        internal_error("Nincs eleg memoria Pecsetview-nak!");
    }
    Pecsetview->mutatotlanit();
    Pecsetview->deletemdarabok();

    // Kajak egesz koordjainak kitoltese:
    Pecsetalso->kitoltfoodkoordokat();
    Pecsetview->kitoltfoodkoordokat(); // View koordok
}

void segedfv(void) {
    ecset* pe = NULL;
    pe->kiegysor_A(NULL, 0);
}

// Egy iterator class (inkabb cache) ecsethez (foltoz hasznalja oket):
mdbiter::mdbiter(ecset* pecset_p) {
    xposbent = -1;
    ybent = -1;
    pmd = NULL;

    pecset = pecset_p;
}

mdarab* mdbiter::getpmd(int x, int y) {
    if (y == ybent && x >= xposbent) {
        while (x >= xposbent + pmd->xsize) {
            xposbent += pmd->xsize;
            pmd = pmd->pkov;
        }
    } else {
        // Ujra kell olvasni:
        if (y < 0 || y >= pecset->sorszam) {
            internal_error("789eygrq");
        }
        ybent = y;
        pmd = pecset->msorok[y];
        xposbent = pecset->kurxposok_A[y];
        if (xposbent > 10) {
            internal_error("y7fjksd");
        }
        while (x >= xposbent + pmd->xsize) {
            xposbent += pmd->xsize;
            pmd = pmd->pkov;
        }
    }
    return pmd;
}
