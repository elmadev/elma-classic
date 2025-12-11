#include "all.h"

static int Szakaszmaxszam = 5130;

szakaszok* Pszak = NULL;
// szakaszok* Pszakt = NULL;

// write ptop->lezart into loadszakasz if topology file is read:
// this is used by afterplayext:
int Ucsoloadszakasz_lezart = 0;

szakaszok::szakaszok(topol* ptop) {
    // same as void constructor at first:
    tomb = NULL;
    maxszam = szam = 0;
    tertomb = NULL;
    xdim = 1;
    ydim = 1;
    cellameret = 1.0;
    origo = vekt2(0, 0);
    nextnode = NULL;
    pelsocsomag = NULL;
    csomagbanbetelt = 0;

    tomb = new vonal[Szakaszmaxszam];
    if (!tomb) {
        uzenet("memory for segments!");
        return;
    }
    maxszam = Szakaszmaxszam;
    for (int i = 0; i < maxszam; i++) {
        memset(&tomb[i], 0, sizeof(vonal));
    }

    // read normal segments
    for (int i = 0; i < MAXGYURU; i++) {
        gyuru* pgy = ptop->ptomb[i];
        if (!pgy) {
            continue;
        }
        if (pgy->koveto) {
            continue;
        }
        for (int j = 0; j < pgy->pontszam; j++) {
            if (szam >= Szakaszmaxszam) {
                hiba("z >= Szakaszmaxszam in szakaszok::szakaszok!");
            }
            vekt2 r1, r2;
            if (j < pgy->pontszam - 1) {
                r1 = pgy->ponttomb[j];
                r2 = pgy->ponttomb[j + 1];
            } else {
                r1 = pgy->ponttomb[j];
                r2 = pgy->ponttomb[0];
            }
            tomb[szam].r = r1;
            tomb[szam].v = r2 - r1;
            tomb[szam].r.y = -tomb[szam].r.y; // upside down
            tomb[szam].v.y = -tomb[szam].v.y; // upside down
            szam++;
        }
    }
}

szakaszok::~szakaszok(void) {
    if (tomb) {
        delete tomb;
    }
    if (tertomb) {
        delete tertomb;
    }
    deleteallnodes();
}

// node management

vonalnode* szakaszok::getnewnode(void) {
    if (!pelsocsomag) {
        {
            pelsocsomag = new soknodecsomag;
        }
        if (!pelsocsomag) {
            uzenet("memory!");
        }
        pelsocsomag->nextcsomag = NULL;
        csomagbanbetelt = 0;
    }
    soknodecsomag* curcsomag = pelsocsomag;
    while (curcsomag->nextcsomag) {
        curcsomag = curcsomag->nextcsomag;
    }
    if (csomagbanbetelt == CSOMAGBANNODESZAM) {
        {
            curcsomag->nextcsomag = new soknodecsomag;
        }
        curcsomag = curcsomag->nextcsomag;
        if (!curcsomag) {
            uzenet("memory!");
        }
        curcsomag->nextcsomag = NULL;
        csomagbanbetelt = 0;
    }
    csomagbanbetelt++;
    return &curcsomag->nodetomb[csomagbanbetelt - 1];
}

void szakaszok::deleteallnodes(void) {
    soknodecsomag* curcsomag = pelsocsomag;
    pelsocsomag = NULL;
    while (curcsomag) {
        soknodecsomag* elozocsomag = curcsomag;
        curcsomag = curcsomag->nextcsomag;
        delete elozocsomag;
    }
}

// connect line into a cell if not yet done but is in table:
void szakaszok::bekot(int cellax, int cellay, vonal* pv) {
#ifdef TEST
    if (cellax < 0 || cellay < 0) {
        hiba("cellax < 0 || cellay < 0!");
    }
#endif
    if (cellax >= xdim || cellay >= ydim) {
        return;
    }
    vonalnode* pujnode = getnewnode();
    pujnode->pnext = NULL;
    pujnode->pvonal = pv;
    vonalnode* pn = tertomb[xdim * cellay + cellax];
    if (!pn) {
        tertomb[xdim * cellay + cellax] = pujnode;
        return;
    }
    while (pn->pnext) {
        pn = pn->pnext;
    }
    // pn points to last node
    pn->pnext = pujnode;
}

// assign line to all cells it might be related to
void szakaszok::beindexelvonalat(vonal* pv, double maxtav) {
    // fill in line unit and length
    pv->hossz = abs(pv->v);
    if (pv->hossz < 0.00000001) {
        hiba("Line length too short!");
    }
    pv->egyseg = egys(pv->v);

    vekt2 v = pv->v;
    vekt2 r = pv->r - origo;

    v = v * (1 / cellameret);
    r = r * (1 / cellameret);
    maxtav *= 1.5 / cellameret;
    // cell size should be 1.0 from here
    int invertalt = 0;
    if (fabs(v.y) > fabs(v.x)) {
        invertalt = 1;
        double tmpd = v.x;
        v.x = v.y;
        v.y = tmpd;
        tmpd = r.x;
        r.x = r.y;
        r.y = tmpd;
    }
    // x length is larger or equal to y length from here
    if (v.x < 0) {
        // Vegpont csere:
        r = r + v;
        v = Vekt2null - v;
    }
    // v has positive x direction
    // calculate parameters of y = m*x + y0:
    double m = v.y / v.x;
    double y0 = r.y - m * r.x;
    double xkezdo = r.x - maxtav;
    int cellax = 0;
    if (xkezdo > 0) {
        cellax = xkezdo;
    }
    if (r.x + v.x + maxtav < 0) {
        hiba("r.x+v.x+maxtav < 0 in szakaszok::beindexelvonalat!");
    }
    int ucsocellax = r.x + v.x + maxtav;
    while (cellax <= ucsocellax) {
        double y1 = m * cellax + y0;
        double y2 = m * (cellax + 1) + y0;
        // y1 should be less than y2
        if (y1 > y2) {
            double tmpd = y1;
            y1 = y2;
            y2 = tmpd;
        }
        y1 -= maxtav;
        y2 += maxtav;
#ifdef TEST
        if (y1 > y2) {
            hiba("Bug!");
        }
#endif
        if (y2 < 0.0) {
            hiba("Bug!");
        }
        int cellay = 0;
        if (y1 > 0) {
            cellay = y1;
        }
        int ucsocellay = y2;
        while (cellay <= ucsocellay) {
            if (invertalt) {
                bekot(cellay, cellax, pv);
            } else {
                bekot(cellax, cellay, pv);
            }

            cellay++;
        }

        cellax++;
    }
}

void szakaszok::rendez(double maxtav) {
    // check
    if (szam <= 0) {
        hiba("szam <= 0 in szakaszok::rendez!");
    }
    if (tertomb) {
        hiba("nodetomb != NULL in szakaszok::rendez!");
    }

    cellameret = 1.0;

    // find level size
    felsorolasresetszak();
    vonal* psz = getnextszak();
    if (!psz) {
        hiba("No segments are defined!");
    }
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

        psz = getnextszak();
    }

    // slightly expand borders
    minx -= 6.0;
    miny -= 6.0;
    maxx += 6.0;
    maxy += 6.0;

    // set origin
    origo = vekt2(minx, miny);
    double xsized = maxx - minx;
    double ysized = maxy - miny;

    // calculate dimension
    xdim = xsized / cellameret + 1;
    ydim = ysized / cellameret + 1;
    if (xdim < 0 || ydim < 0) {
        hiba("xdim < 0 || ydim < 0!");
    }
    if (xdim > 200 || ydim > 200) {
        hiba("xdim > 200 || ydim > 200!");
    }

    // allocates for array
    unsigned tombmeret = xdim * ydim;
    tertomb = new pvonalnode[tombmeret];
    if (!tertomb) {
        uzenet("memory!");
    }
    int hatar = xdim * ydim;
    for (int i = 0; i < hatar; i++) {
        tertomb[i] = NULL;
    }

    // iterate through segments and insert them
    felsorolasresetszak();
    psz = getnextszak();
    while (psz) {
        beindexelvonalat(psz, maxtav);
        psz = getnextszak();
    }
}

void szakaszok::felsorolasreset(vekt2 r) {
    if (!tertomb) {
        hiba("!tertomb in szakaszok::felsorolasreset!");
    }
    r = (r - origo) * (1 / cellameret);
    int cellax = 0;
    if (r.x > 0) {
        cellax = r.x;
    }
    int cellay = 0;
    if (r.y > 0) {
        cellay = r.y;
    }

    if (cellax > xdim) {
        hiba("cellax > xdim in szakaszok::felsorolasreset!");
    }
    if (cellax == xdim) {
        cellax = xdim - 1;
    }
    if (cellay > ydim) {
        hiba("cellay > ydim in szakaszok::felsorolasreset!");
    }
    if (cellay == ydim) {
        cellay = ydim - 1;
    }
    nextnode = tertomb[xdim * cellay + cellax];
}

vonal* szakaszok::getnext(void) {
    if (!nextnode) {
        return NULL;
    }
#ifdef TEST
    if (!nextnode->pvonal) {
        hiba("!nextnode->pvonal in szakaszok::getnext!");
    }
#endif
    vonal* pret = nextnode->pvonal;
    nextnode = nextnode->pnext;
    return pret;
}

// Szakaszos kereso:
void szakaszok::felsorolasresetszak(void) { szakfuto = 0; }

vonal* szakaszok::getnextszak(void) {
    if (szakfuto >= szam) {
        return NULL;
    }
    szakfuto++;
    return &tomb[szakfuto - 1];
}

vonal* szakaszok::getptrszakasz(int sorszam) {
    if (sorszam >= szam) {
        hiba("sorszam >= szam in szakaszok::getptrszakasz!");
    }
    return &tomb[sorszam];
}
