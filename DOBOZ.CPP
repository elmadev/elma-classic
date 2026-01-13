#include "DOBOZ.H"
#include "affine_pic.h"
#include "KIRAJZOL.H"
#include "pic8.h"
#include "vect2.h"
#include <cmath>

// Affinitassal kapcsolatos dolgok:
int Affinitas = 0;
static double Affinnagysag = 1.0;
static vect2 Affinkozep = Vect2i;
static vect2 Affinirany = Vect2i;
static double Affinaranyx = 1.0;
static double Affinaranyy = 1.0;

static double Tulvizszintes = 0.005;

void egysor(unsigned char lyuk, unsigned char szam, unsigned char* sor, unsigned char* tomb,
            double futox, double futoy, double dx, double dy) {
    for (short x = 0; x < szam; x++) {
        int fx = (int)(futox);
        int fy = (int)(futoy);

        unsigned char c = tomb[(fy << 8) + fx];
        if (c != lyuk) {
            sor[x] = c;
        }

        futox += dx;
        futoy += dy;
    }
}

// Ez a fuggveny kilogas eseten hivodik meg, plussz parametere xhossz:
void egysor(unsigned char lyuk, short szam, unsigned char* sor, unsigned char* tomb, double futox,
            double futoy, double dx, double dy, int xhossz) {
    for (short x = 0; x < szam; x++) {
        if (x >= xhossz) {
            return;
        }
        int fx = (int)(futox);
        int fy = (int)(futoy);

        unsigned char c = tomb[(fy << 8) + fx];
        if (c != lyuk) {
            sor[x] = c;
        }

        futox += dx;
        futoy += dy;
    }
}

void setaffinitas(vect2 kozep, vect2 irany, double nagysag, double aranyx, double aranyy) {
    Affinkozep = kozep;
    irany.normalize();
    Affinirany = irany;
    Affinnagysag = nagysag;
    Affinaranyx = aranyx;
    Affinaranyy = aranyy;
}

// static unsigned char Egysor[400];

void dobozki(pic8* ppic8, affine_pic* pkiskep, vect2 u, vect2 v, vect2 r) {
    unsigned char lyuk = pkiskep->transparency;

    // Affinitas:
    if (Affinitas) {
        // Kicsit atalakitja vektorokat:
        // Visszaalakitjuk 1:1-es aspect-ratio-ra r, u, v-t:
        r.x /= Affinaranyx;
        r.y /= Affinaranyy;
        u.x /= Affinaranyx;
        u.y /= Affinaranyy;
        v.x /= Affinaranyx;
        v.y /= Affinaranyy;

        // szamitas:
        double tav = (r - Affinkozep) * Affinirany;
        vect2 kivonando = (tav * (1.0 - Affinnagysag)) * Affinirany;
        r = r - kivonando;

        tav = u * Affinirany;
        kivonando = (tav * (1.0 - Affinnagysag)) * Affinirany;
        u = u - kivonando;

        tav = v * Affinirany;
        kivonando = (tav * (1.0 - Affinnagysag)) * Affinirany;
        v = v - kivonando;

        // Visszaalakitjuk aranyx:aranyy-os aspect-ratio-ra r, u, v-t:
        r.x *= Affinaranyx;
        r.y *= Affinaranyy;
        u.x *= Affinaranyx;
        u.y *= Affinaranyy;
        v.x *= Affinaranyx;
        v.y *= Affinaranyy;
    }

    // Megnezi, hogy nem tul vizszintes-e:
    int vizszintes = 0;
    int pozitivan = 0;
    if (u.x == 0.0) {
        vizszintes = 1;
    } else {
        if (fabs(u.y / u.x) < Tulvizszintes) {
            vizszintes = 1;
            if (u.y / u.x > 0) {
                pozitivan = 1;
            }
        }
    }

    if (v.x == 0.0) {
        vizszintes = 1;
    } else {
        if (fabs(v.y / v.x) < Tulvizszintes) {
            vizszintes = 1;
            if (v.y / v.x > 0) {
                pozitivan = 1;
            }
        }
    }

    if (vizszintes) {
        if (pozitivan) {
            u.rotate(Tulvizszintes);
            v.rotate(Tulvizszintes);
        } else {
            u.rotate(-Tulvizszintes);
            v.rotate(-Tulvizszintes);
        }
    }
    // Most mar biztos, hogy egy kicsit ferde az egesz!
    // Kiveve Affin esetet, amikor azert ez lehetseges:
    if (Affinitas) {
        vizszintes = 1;
        while (vizszintes) {
            vizszintes = 0;
            if (u.x == 0) {
                vizszintes = 1;
            }
            if (fabs(u.y / u.x) < Tulvizszintes) {
                vizszintes = 1;
            }
            if (u.y == 0) {
                vizszintes = 1;
            }
            if (fabs(u.x / u.y) < Tulvizszintes) {
                vizszintes = 1;
            }
            if (v.x == 0) {
                vizszintes = 1;
            }
            if (fabs(v.y / v.x) < Tulvizszintes) {
                vizszintes = 1;
            }
            if (v.y == 0) {
                vizszintes = 1;
            }
            if (fabs(v.x / v.y) < Tulvizszintes) {
                vizszintes = 1;
            }
            if (vizszintes) {
                u.rotate(Tulvizszintes);
                v.rotate(Tulvizszintes);
            }
        }
    }

    // Inverz matrix kiszamitasa:
    vect2 uu = u * (1.0 / (pkiskep->width - 1));
    vect2 vv = v * (1.0 / (pkiskep->height - 1));
    double recdet = 1.0 / (uu.x * vv.y - vv.x * uu.y);
    vect2 invi(vv.y * recdet, -uu.y * recdet);
    vect2 invj(-vv.x * recdet, uu.x * recdet);

    // Most megnezzuk, hogy nem log-e ki a negy csucs kozul valamelyik
    // kepernyorol (negy csucs: r, r+u, r+v, r+u+v

    // X irany;
    double maxert = 0.0;
    double minert = 0.0;
    if (u.x > 0) {
        if (v.x > 0) {
            // u poz, v poz:
            maxert = r.x + u.x + v.x;
            minert = r.x;
        } else {
            // u poz, v neg:
            maxert = r.x + u.x;
            minert = r.x + v.x;
        }
    } else {
        if (v.x > 0) {
            // u neg, v poz:
            maxert = r.x + v.x;
            minert = r.x + u.x;
        } else {
            // u neg, v neg:
            maxert = r.x;
            minert = r.x + u.x + v.x;
        }
    }
    int kilog = 0;
    if (maxert > Hatarx2) {
        kilog = 1;
    }
    if (minert < Hatarx1) {
        kilog = 1;
    }

    // A legmagasabban fekvo pontot kell erre valasztani:
    vect2 csucs;

    // Y irany;
    if (u.y > 0) {
        if (v.y > 0) {
            // u poz, v poz:
            csucs = r + u + v;
            minert = r.y;
        } else {
            // u poz, v neg:
            csucs = r + u;
            minert = r.y + v.y;
        }
    } else {
        if (v.y > 0) {
            // u neg, v poz:
            csucs = r + v;
            minert = r.y + u.y;
        } else {
            // u neg, v neg:
            csucs = r;
            minert = r.y + u.y + v.y;
        }
    }
    maxert = csucs.y;
    if (maxert > Hatary2) {
        kilog = 1;
    }
    if (minert < Hatary1) {
        kilog = 1;
    }

    // if( kilog )
    //	return;

    // Kivalasztjuk egesz kezdopontot:
    int xbal = (int)(csucs.x);
    int ybal = (int)(csucs.y);
    double szint = ybal;

    // Kiszamolja hatarolo egyenesek parametereit:
    double hatbal1, hatjobb1;
    double m1d = u.x / u.y;
    double hatbal2, hatjobb2;
    double m2d = v.x / v.y;
    if (Affinitas) {
        // u es v nem biztos, hogy merolegesek egymasra, ezert szamitas
        // bonyolultab:
        // Most u-ra meroleges eleket intezzuk el:
        // megkeressuk r magassagaban nem r-en atmeno u x koordjat:
        double xmetszet = r.x + v.x - (u.x / u.y) * v.y;
        if (r.x < xmetszet) {
            // u a ball es u+v a jobb!:
            hatbal1 = r.x + (szint - r.y) * m1d;
            hatjobb1 = r.x + v.x + (szint - r.y - v.y) * m1d;
        } else {
            hatjobb1 = r.x + (szint - r.y) * m1d;
            hatbal1 = r.x + v.x + (szint - r.y - v.y) * m1d;
        }
        // Most v-ra meroleges eleket intezzuk el:
        // megkeressuk r magassagaban nem r-en atmeno v x koordjat:
        xmetszet = r.x + u.x - (v.x / v.y) * u.y;
        if (r.x < xmetszet) {
            // v a ball es v+u a jobb!:
            hatbal2 = r.x + (szint - r.y) * m2d;
            hatjobb2 = r.x + u.x + (szint - r.y - u.y) * m2d;
        } else {
            hatjobb2 = r.x + (szint - r.y) * m2d;
            hatbal2 = r.x + u.x + (szint - r.y - u.y) * m2d;
        }
    } else {
        // u es v merolegesek egymasra, ezert szamitas gyors lehet:
        // Most u-ra meroleges eleket intezzuk el:
        if (v.x > 0) {
            // u a ball es u+v a jobb!:
            hatbal1 = r.x + (szint - r.y) * m1d;
            hatjobb1 = r.x + v.x + (szint - r.y - v.y) * m1d;
        } else {
            hatjobb1 = r.x + (szint - r.y) * m1d;
            hatbal1 = r.x + v.x + (szint - r.y - v.y) * m1d;
        }
        // Most v-ra meroleges eleket intezzuk el:
        if (u.x > 0) {
            // v a ball es v+u a jobb!:
            hatbal2 = r.x + (szint - r.y) * m2d;
            hatjobb2 = r.x + u.x + (szint - r.y - u.y) * m2d;
        } else {
            hatjobb2 = r.x + (szint - r.y) * m2d;
            hatbal2 = r.x + u.x + (szint - r.y - u.y) * m2d;
        }
    }

    // Most kiszamoljuk kis kepen futok elejet:
    vect2 diff = vect2(xbal, ybal) - r;
    vect2 kiskeporigo = vect2(0.5, 0.5) + diff.x * invi + diff.y * invj;
    double xkiskep = kiskeporigo.x;
    double ykiskep = kiskeporigo.y;

    if (kilog) {
        // Voltkilogas = 1; //Csak teszteleshez
        //  Nagy ciklus kezdetet veszi:
        while (1) {
            // Kiszamoljuk bal es jobb hatarokat ebben a sorban!
            int x1 = (int)(hatbal1);
            int xb = (int)(hatbal2);
            if (xb > x1) {
                x1 = xb;
            }
            x1++;
            int x2 = (int)(hatjobb1);
            xb = (int)(hatjobb2);
            if (xb < x2) {
                x2 = xb;
            }
            // x1 elso, x2 utolso hely, amit meg ki kell rakni!
            if (x1 <= x2 && ybal < Cysize) { // VALT
                // Most raallitjuk xbal-t x1-re:
                while (xbal > x1 && xbal > 0) { // VALT
                    xbal--;
                    xkiskep -= invi.x;
                    ykiskep -= invi.y;
                }
                while (xbal < x1 || xbal < 0) {
                    xbal++;
                    xkiskep += invi.x;
                    ykiskep += invi.y;
                }
                unsigned char* cim = ppic8->get_row(ybal);
                cim += xbal;
                int xhossz = Cxsize - xbal;
                egysor(lyuk, short(x2 - xbal + 1), cim, pkiskep->pixels, xkiskep, ykiskep, invi.x,
                       invi.y, xhossz);
            } else {
                if (x1 > x2 + 1) {
                    return;
                }
            }
            // Egy sorral lejjebb kell lepni!:
            ybal--;
            if (ybal < 0) { // VALT
                return;     // VALT
            }
            xkiskep -= invj.x;
            ykiskep -= invj.y;
            hatbal1 -= m1d;
            hatjobb1 -= m1d;
            hatbal2 -= m2d;
            hatjobb2 -= m2d;
        }
    } else {
        // Nagy ciklus kezdetet veszi:
        while (1) {
            // Kiszamoljuk bal es jobb hatarokat ebben a sorban!
            int x1 = (int)(hatbal1);
            int xb = (int)(hatbal2);
            if (xb > x1) {
                x1 = xb;
            }
            x1++;
            int x2 = (int)(hatjobb1);
            xb = (int)(hatjobb2);
            if (xb < x2) {
                x2 = xb;
            }
            // x1 elso, x2 utolso hely, amit meg ki kell rakni!
            if (x1 <= x2) {
                // Most raallitjuk xbal-t x1-re:
                while (xbal > x1) {
                    xbal--;
                    xkiskep -= invi.x;
                    ykiskep -= invi.y;
                }
                while (xbal < x1) {
                    xbal++;
                    xkiskep += invi.x;
                    ykiskep += invi.y;
                }
                unsigned char* cim = ppic8->get_row(ybal);
                cim += xbal;
                egysor(lyuk, short(x2 - x1 + 1), cim, pkiskep->pixels, xkiskep, ykiskep, invi.x,
                       invi.y);
            } else {
                if (x1 > x2 + 1) {
                    return;
                }
            }
            // Egy sorral lejjebb kell lepni!:
            ybal--;
            xkiskep -= invj.x;
            ykiskep -= invj.y;
            hatbal1 -= m1d;
            hatjobb1 -= m1d;
            hatbal2 -= m2d;
            hatjobb2 -= m2d;
        }
    }
}
