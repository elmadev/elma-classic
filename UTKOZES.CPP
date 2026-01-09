#include "UTKOZES.H"
#include "ALL.H"
#include "main.h"
#include "physics_init.h"
#include "SZAKASZ.H"
#include "vect2.h"

static int gombszakasz(vect2 r, double sugar, vonal* pv, vect2* pt) {
    vect2 rel = r - pv->r;
    double pozicio = rel * pv->egyseg;
    if (pozicio < 0) {
        if ((r - pv->r).length() < sugar) {
            *pt = pv->r;
            return 1;
        } else {
            return 0;
        }
    }
    if (pozicio > pv->hossz) {
        if ((r - (pv->r + pv->egyseg * pv->hossz)).length() < sugar) {
            *pt = pv->r + pv->egyseg * pv->hossz;
            return 1;
        } else {
            return 0;
        }
    }
    vect2 n = rotate_90deg(pv->egyseg);
    double tav = rel * n;
    if (tav < -sugar || tav > sugar) {
        return 0;
    }
    *pt = pv->r + pv->egyseg * pozicio;
    return 1;
}

int talppontkereses(vect2 r, double sugar, vect2* pt1, vect2* pt2, vonal** ppv1, vonal** ppv2) {
    *ppv1 = *ppv2 = NULL;

    Pszak->felsorolasreset(r);
    int talppontszam = 0;
    vonal* pv = NULL;
    // int vegignezett = 0; // tesztelesre!
    while ((pv = Pszak->getnext()) != NULL) {
        // vegignezett++;
        vect2 t;
        if (gombszakasz(r, sugar, pv, &t)) {
            if (talppontszam == 2) {
                internal_error("Egyszerre kettonel tobb talppontszam is van");
            }
            if (talppontszam == 1) {
                *pt2 = t;
                *ppv2 = pv;
                talppontszam++;
                // Ellenorzes, hogy nincs-e tul kozel ketpont,
                // ha ket pontrol van szo:
                if ((*pt1 - *pt2).length() < TwoPointDiscriminationDistance) {
                    *pt1 = (*pt1 + *pt2) * 0.5; // atlagol
                    talppontszam = 1;
                    *ppv2 = NULL;
                } else {
                    // Kettonel tobb talppont mar ugy sem lehet!
                    return talppontszam;
                }
            }
            if (talppontszam == 0) {
                *pt1 = t;
                *ppv1 = pv;
                talppontszam++;
            }
        }
    }
    // kinyom( vegignezett );
    return talppontszam;
}
