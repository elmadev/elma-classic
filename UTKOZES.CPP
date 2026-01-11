#include "UTKOZES.H"
#include "main.h"
#include "physics_init.h"
#include "SZAKASZ.H"
#include "vect2.h"

static int gombszakasz(vect2 r, double sugar, segment* pv, vect2* pt) {
    vect2 rel = r - pv->r;
    double pozicio = rel * pv->unit_vector;
    if (pozicio < 0) {
        if ((r - pv->r).length() < sugar) {
            *pt = pv->r;
            return 1;
        } else {
            return 0;
        }
    }
    if (pozicio > pv->length) {
        if ((r - (pv->r + pv->unit_vector * pv->length)).length() < sugar) {
            *pt = pv->r + pv->unit_vector * pv->length;
            return 1;
        } else {
            return 0;
        }
    }
    vect2 n = rotate_90deg(pv->unit_vector);
    double tav = rel * n;
    if (tav < -sugar || tav > sugar) {
        return 0;
    }
    *pt = pv->r + pv->unit_vector * pozicio;
    return 1;
}

int talppontkereses(vect2 r, double sugar, vect2* pt1, vect2* pt2, segment** ppv1, segment** ppv2) {
    *ppv1 = *ppv2 = nullptr;

    Segments->iterate_collision_grid_cell_segments(r);
    int talppontszam = 0;
    segment* pv = nullptr;
    // int vegignezett = 0; // tesztelesre!
    while ((pv = Segments->next_collision_grid_segment()) != nullptr) {
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
                    *ppv2 = nullptr;
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
