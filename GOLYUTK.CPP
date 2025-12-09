#include "all.h"

int Nincsfal = 0;

static int Balls3D = 0; // Ha igaz, akkor tomegek 3d aranyuak

static void osszeigazit(golyo* m1, golyo* m2) {
    if (m1->t < m2->t) {
        m1->r = m1->r + ((m2->t - m1->t) * m1->v);
        m1->alfa = m1->alfa + ((m2->t - m1->t) * m1->omega);
        m1->t = m2->t;
    } else {
        m2->r = m2->r + ((m1->t - m2->t) * m2->v);
        m2->alfa = m2->alfa + ((m1->t - m2->t) * m2->omega);
        m2->t = m1->t;
    }
}
void idoreigazit(golyo* m, double t) {
    m->r = m->r + ((t - m->t) * m->v);
    m->alfa = m->alfa + ((t - m->t) * m->omega);
    m->t = t;
}

double ggutkozesideje(golyo* m1, golyo* m2) {
    vekt2 r, v;
    double a, b, c, det, t;

    osszeigazit(m1, m2);
    v = m1->v - m2->v;
    r = m2->r - m1->r;
    a = v.x * v.x + v.y * v.y;
    if (a == 0) {
        return (Hosszuido);
    }
    b = (-2) * (v.x * r.x + v.y * r.y);
    if (b >= 0) {
        return (Hosszuido);
    }
    c = r.x * r.x + r.y * r.y - (m1->sug + m2->sug) * (m1->sug + m2->sug);
    det = b * b - 4 * a * c;
    if (det <= 0) {
        return (Hosszuido);
    }
    t = (-b - gyok(det)) / (2 * a);
    return (t + m1->t);
}

void ggutkozes(golyo* g1, golyo* g2, double t) {
    double ero = abs(g1->v - g2->v);

    idoreigazit(g1, t);
    idoreigazit(g2, t);
    // Kiszamolja tomegkozeppont sebesseg vektorat (v):
    double m1, m2;
    double theta1, theta2;
    if (Balls3D) {
        m1 = g1->sug * g1->sug * g1->sug;
        m2 = g2->sug * g2->sug * g2->sug;
        theta1 = 0.4 * m1 * g1->sug * g1->sug; // Gomb
        theta2 = 0.4 * m2 * g2->sug * g2->sug;
    } else {
        m1 = g1->sug * g1->sug;
        m2 = g2->sug * g2->sug;
        theta1 = 0.5 * m1 * g1->sug * g1->sug;
        theta2 = 0.5 * m2 * g2->sug * g2->sug;
    }
    // Kiszamitjuk ero radialis komponenset:
    double mall = m1 + m2;
    vekt2 v = ((m1 / mall) * g1->v) + ((m2 / mall) * g2->v);

    // Megmaradas kesobbi vizsgalatahoz:
    // vekt2 tomegpontteszt = v;
    double oldenergia = m1 * g1->v * g1->v + m2 * g2->v * g2->v + theta1 * g1->omega * g1->omega +
                        theta2 * g2->omega * g2->omega;

    vekt2 n = (1 / (g1->sug + g2->sug)) * (g1->r - g2->r);
    // Szamitas elsore:
    vekt2 diff1 = 2 * (n * (g1->v - v)) * n;
    // g1->v = g1->v - diff1;
    //  Szamitas masodikra:
    vekt2 diff2 = 2 * (n * (g2->v - v)) * n;
    // g2->v = g2->v - diff2;
    //  Eloallt radialis sebesseg valtozas diff1, diff2 vektorokban. Ezeket
    //  majd vegen adjuk hozza sebesseghez, eloszor tangencialis szamolunk:

    // De most csak ezt szamoljuk, ugyhogy hozzaadjuk:
    // g1->v = g1->v - diff1;
    // g2->v = g2->v - diff2;

    vekt2 dr = g2->r - g1->r;
    dr.normal();
    vekt2 tangens = forgatas90fokkal(dr);
    double v1 = tangens * g1->v;
    double v2 = tangens * g2->v;
    double tangenergia = m1 * v1 * v1 + m2 * v2 * v2 + theta1 * g1->omega * g1->omega +
                         theta2 * g2->omega * g2->omega;
    double omega1 = g1->omega;
    double omega2 = g2->omega;
    double R1 = g1->sug;
    double R2 = g2->sug;
    double F = (v2 - v1 - omega1 * R1 - omega2 * R2) /
               (1 / m1 + 1 / m2 + R1 * R1 / theta1 + R1 * R1 / theta1);

    F /= 2.0;

    g1->omega += F * R1 / theta1;
    g2->omega += F * R2 / theta2;
    // g1->v = g1->v + F/m1*tangens - diff1;
    // g2->v = g2->v - F/m2*tangens - diff2;

    // vekt2 vuj = ((m1/mall) * g1->v) + ((m2/mall) * g2->v );
    // double elteres1 = abs( tomegpontteszt - vuj );

    int i = 0;
    double szorzo = 1.0;
    double dszorzo = 0.5;
    int felfele = 1;
    while (1) {
        i++;

        vekt2 ujv1 = g1->v + F / m1 * tangens - diff1 * szorzo;
        vekt2 ujv2 = g2->v - F / m2 * tangens - diff2 * szorzo;

        double ujenergia = m1 * ujv1 * ujv1 + m2 * ujv2 * ujv2 + theta1 * g1->omega * g1->omega +
                           theta2 * g2->omega * g2->omega;

        if (felfele) {
            if (ujenergia > oldenergia) {
                felfele = 0;
            } else {
                szorzo *= 2.0;
                dszorzo *= 2.0;
            }
        } else {
            if (i == 30) {
                g1->v = ujv1;
                g2->v = ujv2;
                if (fabs(ujenergia - oldenergia) > 0.001) {
                    hiba("fabs( ujenergia - oldenergia ) > 0.001!");
                }
                break;
            }

            if (ujenergia > oldenergia) {
                szorzo -= dszorzo;
            } else {
                szorzo += dszorzo;
            }
            dszorzo *= 0.5001;
        }
    }

    // double elteresenerg = ujenergia - energiateszt;
    // if( elteresenerg > 0.0 )
    //	hiba( "elteresnergia > 0.0!" );
}

static int Balszel = 0, Jobbszel = 640;
static int Alsoszel = 0, Felsoszel = 480;

double goutkozesideje(golyo* m, int old) {
    if (Nincsfal) {
        return Hosszuido;
    }
    switch (old) {
    case ALSO:
        if (m->v.y >= 0) {
            return Hosszuido;
        }
        if (m->r.y <= Alsoszel + m->sug) {
            return Hosszuido;
        }
        return m->t - ((m->r.y - Alsoszel - m->sug) / m->v.y);
    case FELSO:
        if (m->v.y <= 0) {
            return Hosszuido;
        }
        if (m->r.y >= Felsoszel - m->sug) {
            return Hosszuido;
        }
        return m->t + ((Felsoszel - m->sug - m->r.y) / m->v.y);
    case JOBB:
        if (m->v.x <= 0) {
            return Hosszuido;
        }
        if (m->r.x >= Jobbszel - m->sug) {
            return Hosszuido;
        }
        return m->t + ((Jobbszel - m->sug - m->r.x) / m->v.x);
    case BAL:
        if (m->v.x >= 0) {
            return Hosszuido;
        }
        if (m->r.x <= Balszel + m->sug) {
            return Hosszuido;
        }
        return m->t - ((m->r.x - Balszel - m->sug) / m->v.x);
    }
    return (Hosszuido);
}

void goutkozes(golyo* m, int old, double t) {
    if (Nincsfal) {
        return;
    }
    idoreigazit(m, t);
    switch (old) {
    case ALSO:
    case FELSO:
        m->v.y = -m->v.y;
        break;
    case JOBB:
    case BAL:
        m->v.x = -m->v.x;
        break;
    }
    // double ero = dsvabs( &m->v );
    // GG_go_utkozes( ero );
}

// Visszaadja osszenergiat:
/*double getenergia( void ) {
    double sum = 0.0;
    for( int i = 0; i < Ngolyok; i++ ) {
        golyo* g1 = &Golyok[i];
        double m;
        double theta;
        if( Balls3D ) {
            m = g1->sug*g1->sug*g1->sug;
            theta = 0.4*m*g1->sug*g1->sug; // Gomb
        }
        else {
            m = g1->sug*g1->sug;
            theta = 0.5*m*g1->sug*g1->sug;
        }
        sum += m*g1->v*g1->v + theta*g1->omega*g1->omega;
    }
    return sum;
} */

// Minden sebesseget es forgasit megszoroz ezzel:
/*void szorozmindet( double szorzo ) {
    for( int i = 0; i < Ngolyok; i++ ) {
        golyo* g1 = &Golyok[i];
        g1->v = g1->v * szorzo;
        g1->omega = g1->omega * szorzo;
    }
} */
