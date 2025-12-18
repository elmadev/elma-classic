#include "ALL.H"

int Nincsfal = 0;

static int Balls3D = 0; // Ha igaz, akkor tomegek 3d aranyuak

static void osszeigazit(ball* m1, ball* m2) {
    if (m1->keyframe_time < m2->keyframe_time) {
        m1->keyframe_r = m1->keyframe_r + ((m2->keyframe_time - m1->keyframe_time) * m1->v);
        m1->keyframe_rotation = m1->keyframe_rotation +
                                ((m2->keyframe_time - m1->keyframe_time) * m1->angular_velocity);
        m1->keyframe_time = m2->keyframe_time;
    } else {
        m2->keyframe_r = m2->keyframe_r + ((m1->keyframe_time - m2->keyframe_time) * m2->v);
        m2->keyframe_rotation = m2->keyframe_rotation +
                                ((m1->keyframe_time - m2->keyframe_time) * m2->angular_velocity);
        m2->keyframe_time = m1->keyframe_time;
    }
}
void idoreigazit(ball* m, double t) {
    m->keyframe_r = m->keyframe_r + ((t - m->keyframe_time) * m->v);
    m->keyframe_rotation = m->keyframe_rotation + ((t - m->keyframe_time) * m->angular_velocity);
    m->keyframe_time = t;
}

double ggutkozesideje(ball* m1, ball* m2) {
    vekt2 r, v;
    double a, b, c, det, t;

    osszeigazit(m1, m2);
    v = m1->v - m2->v;
    r = m2->keyframe_r - m1->keyframe_r;
    a = v.x * v.x + v.y * v.y;
    if (a == 0) {
        return (NO_COLLISION_TIME);
    }
    b = (-2) * (v.x * r.x + v.y * r.y);
    if (b >= 0) {
        return (NO_COLLISION_TIME);
    }
    c = r.x * r.x + r.y * r.y - (m1->radius + m2->radius) * (m1->radius + m2->radius);
    det = b * b - 4 * a * c;
    if (det <= 0) {
        return (NO_COLLISION_TIME);
    }
    t = (-b - gyok(det)) / (2 * a);
    return (t + m1->keyframe_time);
}

void ggutkozes(ball* g1, ball* g2, double t) {
    idoreigazit(g1, t);
    idoreigazit(g2, t);
    // Kiszamolja tomegkozeppont sebesseg vektorat (v):
    double m1, m2;
    double theta1, theta2;
    if (Balls3D) {
        m1 = g1->radius * g1->radius * g1->radius;
        m2 = g2->radius * g2->radius * g2->radius;
        theta1 = 0.4 * m1 * g1->radius * g1->radius; // Gomb
        theta2 = 0.4 * m2 * g2->radius * g2->radius;
    } else {
        m1 = g1->radius * g1->radius;
        m2 = g2->radius * g2->radius;
        theta1 = 0.5 * m1 * g1->radius * g1->radius;
        theta2 = 0.5 * m2 * g2->radius * g2->radius;
    }
    // Kiszamitjuk ero radialis komponenset:
    double mall = m1 + m2;
    vekt2 v = ((m1 / mall) * g1->v) + ((m2 / mall) * g2->v);

    // Megmaradas kesobbi vizsgalatahoz:
    // vekt2 tomegpontteszt = v;
    double oldenergia = m1 * g1->v * g1->v + m2 * g2->v * g2->v +
                        theta1 * g1->angular_velocity * g1->angular_velocity +
                        theta2 * g2->angular_velocity * g2->angular_velocity;

    vekt2 n = (1 / (g1->radius + g2->radius)) * (g1->keyframe_r - g2->keyframe_r);
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

    vekt2 dr = g2->keyframe_r - g1->keyframe_r;
    dr.normal();
    vekt2 tangens = forgatas90fokkal(dr);
    double v1 = tangens * g1->v;
    double v2 = tangens * g2->v;
    double omega1 = g1->angular_velocity;
    double omega2 = g2->angular_velocity;
    double R1 = g1->radius;
    double R2 = g2->radius;
    double F = (v2 - v1 - omega1 * R1 - omega2 * R2) /
               (1 / m1 + 1 / m2 + R1 * R1 / theta1 + R1 * R1 / theta1);

    F /= 2.0;

    g1->angular_velocity += F * R1 / theta1;
    g2->angular_velocity += F * R2 / theta2;
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

        double ujenergia = m1 * ujv1 * ujv1 + m2 * ujv2 * ujv2 +
                           theta1 * g1->angular_velocity * g1->angular_velocity +
                           theta2 * g2->angular_velocity * g2->angular_velocity;

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

static int Balszel = 0, Jobbszel = SCREEN_WIDTH;
static int Alsoszel = 0, Felsoszel = SCREEN_HEIGHT;

double goutkozesideje(ball* m, int old) {
    if (Nincsfal) {
        return NO_COLLISION_TIME;
    }
    switch (old) {
    case ALSO:
        if (m->v.y >= 0) {
            return NO_COLLISION_TIME;
        }
        if (m->keyframe_r.y <= Alsoszel + m->radius) {
            return NO_COLLISION_TIME;
        }
        return m->keyframe_time - ((m->keyframe_r.y - Alsoszel - m->radius) / m->v.y);
    case FELSO:
        if (m->v.y <= 0) {
            return NO_COLLISION_TIME;
        }
        if (m->keyframe_r.y >= Felsoszel - m->radius) {
            return NO_COLLISION_TIME;
        }
        return m->keyframe_time + ((Felsoszel - m->radius - m->keyframe_r.y) / m->v.y);
    case JOBB:
        if (m->v.x <= 0) {
            return NO_COLLISION_TIME;
        }
        if (m->keyframe_r.x >= Jobbszel - m->radius) {
            return NO_COLLISION_TIME;
        }
        return m->keyframe_time + ((Jobbszel - m->radius - m->keyframe_r.x) / m->v.x);
    case BAL:
        if (m->v.x >= 0) {
            return NO_COLLISION_TIME;
        }
        if (m->keyframe_r.x <= Balszel + m->radius) {
            return NO_COLLISION_TIME;
        }
        return m->keyframe_time - ((m->keyframe_r.x - Balszel - m->radius) / m->v.x);
    }
    return (NO_COLLISION_TIME);
}

void goutkozes(ball* m, int old, double t) {
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
