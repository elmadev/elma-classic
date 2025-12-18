#include "ALL.H"

int WallsDisabled = 0;

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

double ball_ball_collision_time(ball* ball1, ball* ball2) {
    vekt2 r, v;
    double a, b, c, det, t;

    osszeigazit(ball1, ball2);
    v = ball1->v - ball2->v;
    r = ball2->keyframe_r - ball1->keyframe_r;
    a = v.x * v.x + v.y * v.y;
    if (a == 0) {
        return (NO_COLLISION_TIME);
    }
    b = (-2) * (v.x * r.x + v.y * r.y);
    if (b >= 0) {
        return (NO_COLLISION_TIME);
    }
    c = r.x * r.x + r.y * r.y - (ball1->radius + ball2->radius) * (ball1->radius + ball2->radius);
    det = b * b - 4 * a * c;
    if (det <= 0) {
        return (NO_COLLISION_TIME);
    }
    t = (-b - gyok(det)) / (2 * a);
    return (t + ball1->keyframe_time);
}

void simulate_ball_ball_collision(ball* ball1, ball* ball2, double time) {
    idoreigazit(ball1, time);
    idoreigazit(ball2, time);
    // Kiszamolja tomegkozeppont sebesseg vektorat (v):
    double m1, m2;
    double theta1, theta2;
    if (Balls3D) {
        m1 = ball1->radius * ball1->radius * ball1->radius;
        m2 = ball2->radius * ball2->radius * ball2->radius;
        theta1 = 0.4 * m1 * ball1->radius * ball1->radius; // Gomb
        theta2 = 0.4 * m2 * ball2->radius * ball2->radius;
    } else {
        m1 = ball1->radius * ball1->radius;
        m2 = ball2->radius * ball2->radius;
        theta1 = 0.5 * m1 * ball1->radius * ball1->radius;
        theta2 = 0.5 * m2 * ball2->radius * ball2->radius;
    }
    // Kiszamitjuk ero radialis komponenset:
    double mall = m1 + m2;
    vekt2 v = ((m1 / mall) * ball1->v) + ((m2 / mall) * ball2->v);

    // Megmaradas kesobbi vizsgalatahoz:
    // vekt2 tomegpontteszt = v;
    double oldenergia = m1 * ball1->v * ball1->v + m2 * ball2->v * ball2->v +
                        theta1 * ball1->angular_velocity * ball1->angular_velocity +
                        theta2 * ball2->angular_velocity * ball2->angular_velocity;

    vekt2 n = (1 / (ball1->radius + ball2->radius)) * (ball1->keyframe_r - ball2->keyframe_r);
    // Szamitas elsore:
    vekt2 diff1 = 2 * (n * (ball1->v - v)) * n;
    // ball1->v = ball1->v - diff1;
    //  Szamitas masodikra:
    vekt2 diff2 = 2 * (n * (ball2->v - v)) * n;
    // ball2->v = ball2->v - diff2;
    //  Eloallt radialis sebesseg valtozas diff1, diff2 vektorokban. Ezeket
    //  majd vegen adjuk hozza sebesseghez, eloszor tangencialis szamolunk:

    // De most csak ezt szamoljuk, ugyhogy hozzaadjuk:
    // ball1->v = ball1->v - diff1;
    // ball2->v = ball2->v - diff2;

    vekt2 dr = ball2->keyframe_r - ball1->keyframe_r;
    dr.normal();
    vekt2 tangens = forgatas90fokkal(dr);
    double v1 = tangens * ball1->v;
    double v2 = tangens * ball2->v;
    double omega1 = ball1->angular_velocity;
    double omega2 = ball2->angular_velocity;
    double R1 = ball1->radius;
    double R2 = ball2->radius;
    double F = (v2 - v1 - omega1 * R1 - omega2 * R2) /
               (1 / m1 + 1 / m2 + R1 * R1 / theta1 + R1 * R1 / theta1);

    F /= 2.0;

    ball1->angular_velocity += F * R1 / theta1;
    ball2->angular_velocity += F * R2 / theta2;
    // ball1->v = ball1->v + F/m1*tangens - diff1;
    // ball2->v = ball2->v - F/m2*tangens - diff2;

    // vekt2 vuj = ((m1/mall) * ball1->v) + ((m2/mall) * ball2->v );
    // double elteres1 = abs( tomegpontteszt - vuj );

    int i = 0;
    double szorzo = 1.0;
    double dszorzo = 0.5;
    int felfele = 1;
    while (1) {
        i++;

        vekt2 ujv1 = ball1->v + F / m1 * tangens - diff1 * szorzo;
        vekt2 ujv2 = ball2->v - F / m2 * tangens - diff2 * szorzo;

        double ujenergia = m1 * ujv1 * ujv1 + m2 * ujv2 * ujv2 +
                           theta1 * ball1->angular_velocity * ball1->angular_velocity +
                           theta2 * ball2->angular_velocity * ball2->angular_velocity;

        if (felfele) {
            if (ujenergia > oldenergia) {
                felfele = 0;
            } else {
                szorzo *= 2.0;
                dszorzo *= 2.0;
            }
        } else {
            if (i == 30) {
                ball1->v = ujv1;
                ball2->v = ujv2;
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

double ball_wall_collision_time(ball* ball, int wall) {
    if (WallsDisabled) {
        return NO_COLLISION_TIME;
    }
    switch (wall) {
    case ALSO:
        if (ball->v.y >= 0) {
            return NO_COLLISION_TIME;
        }
        if (ball->keyframe_r.y <= Alsoszel + ball->radius) {
            return NO_COLLISION_TIME;
        }
        return ball->keyframe_time - ((ball->keyframe_r.y - Alsoszel - ball->radius) / ball->v.y);
    case FELSO:
        if (ball->v.y <= 0) {
            return NO_COLLISION_TIME;
        }
        if (ball->keyframe_r.y >= Felsoszel - ball->radius) {
            return NO_COLLISION_TIME;
        }
        return ball->keyframe_time + ((Felsoszel - ball->radius - ball->keyframe_r.y) / ball->v.y);
    case JOBB:
        if (ball->v.x <= 0) {
            return NO_COLLISION_TIME;
        }
        if (ball->keyframe_r.x >= Jobbszel - ball->radius) {
            return NO_COLLISION_TIME;
        }
        return ball->keyframe_time + ((Jobbszel - ball->radius - ball->keyframe_r.x) / ball->v.x);
    case BAL:
        if (ball->v.x >= 0) {
            return NO_COLLISION_TIME;
        }
        if (ball->keyframe_r.x <= Balszel + ball->radius) {
            return NO_COLLISION_TIME;
        }
        return ball->keyframe_time - ((ball->keyframe_r.x - Balszel - ball->radius) / ball->v.x);
    }
    return (NO_COLLISION_TIME);
}

void simulate_ball_wall_collision(ball* ball, int wall, double time) {
    if (WallsDisabled) {
        return;
    }
    idoreigazit(ball, time);
    switch (wall) {
    case ALSO:
    case FELSO:
        ball->v.y = -ball->v.y;
        break;
    case JOBB:
    case BAL:
        ball->v.x = -ball->v.x;
        break;
    }
    // double ero = dsvabs( &ball->v );
    // GG_go_utkozes( ero );
}
