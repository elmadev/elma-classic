#include "ALL.H"

int WallsDisabled = 0;

static int Balls3D = 0; // Ha igaz, akkor tomegek 3d aranyuak

static void synchronize_keyframes(ball* ball1, ball* ball2) {
    if (ball1->keyframe_time < ball2->keyframe_time) {
        ball1->keyframe_r =
            ball1->keyframe_r + ((ball2->keyframe_time - ball1->keyframe_time) * ball1->v);
        ball1->keyframe_rotation =
            ball1->keyframe_rotation +
            ((ball2->keyframe_time - ball1->keyframe_time) * ball1->angular_velocity);
        ball1->keyframe_time = ball2->keyframe_time;
    } else {
        ball2->keyframe_r =
            ball2->keyframe_r + ((ball1->keyframe_time - ball2->keyframe_time) * ball2->v);
        ball2->keyframe_rotation =
            ball2->keyframe_rotation +
            ((ball1->keyframe_time - ball2->keyframe_time) * ball2->angular_velocity);
        ball2->keyframe_time = ball1->keyframe_time;
    }
}
void update_keyframe(ball* ball, double time) {
    ball->keyframe_r = ball->keyframe_r + ((time - ball->keyframe_time) * ball->v);
    ball->keyframe_rotation =
        ball->keyframe_rotation + ((time - ball->keyframe_time) * ball->angular_velocity);
    ball->keyframe_time = time;
}

double ball_ball_collision_time(ball* ball1, ball* ball2) {
    vekt2 r, v;
    double a, b, c, discriminant, t;

    synchronize_keyframes(ball1, ball2);
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
    discriminant = b * b - 4 * a * c;
    if (discriminant <= 0) {
        return (NO_COLLISION_TIME);
    }
    t = (-b - gyok(discriminant)) / (2 * a);
    return (t + ball1->keyframe_time);
}

void simulate_ball_ball_collision(ball* ball1, ball* ball2, double time) {
    update_keyframe(ball1, time);
    update_keyframe(ball2, time);
    // Kiszamolja tomegkozeppont sebesseg vektorat (v):
    double mass1, mass2;
    double inertia1, inertia2;
    if (Balls3D) {
        mass1 = ball1->radius * ball1->radius * ball1->radius;
        mass2 = ball2->radius * ball2->radius * ball2->radius;
        inertia1 = 0.4 * mass1 * ball1->radius * ball1->radius; // Gomb
        inertia2 = 0.4 * mass2 * ball2->radius * ball2->radius;
    } else {
        mass1 = ball1->radius * ball1->radius;
        mass2 = ball2->radius * ball2->radius;
        inertia1 = 0.5 * mass1 * ball1->radius * ball1->radius;
        inertia2 = 0.5 * mass2 * ball2->radius * ball2->radius;
    }
    // Kiszamitjuk ero radialis komponenset:
    double mass = mass1 + mass2;
    vekt2 center_of_mass_velocity = ((mass1 / mass) * ball1->v) + ((mass2 / mass) * ball2->v);

    // Megmaradas kesobbi vizsgalatahoz:
    // vekt2 tomegpontteszt = center_of_mass_velocity;
    double initial_energy = mass1 * ball1->v * ball1->v + mass2 * ball2->v * ball2->v +
                            inertia1 * ball1->angular_velocity * ball1->angular_velocity +
                            inertia2 * ball2->angular_velocity * ball2->angular_velocity;

    vekt2 dr_normal =
        (1 / (ball1->radius + ball2->radius)) * (ball1->keyframe_r - ball2->keyframe_r);
    // Szamitas elsore:
    vekt2 vperp1 = 2 * (dr_normal * (ball1->v - center_of_mass_velocity)) * dr_normal;
    // ball1->v = ball1->v - vperp1;
    //  Szamitas masodikra:
    vekt2 vperp2 = 2 * (dr_normal * (ball2->v - center_of_mass_velocity)) * dr_normal;
    // ball2->v = ball2->v - vperp2;
    //  Eloallt radialis sebesseg valtozas vperp1, vperp2 vektorokban. Ezeket
    //  majd vegen adjuk hozza sebesseghez, eloszor tangencialis szamolunk:

    // De most csak ezt szamoljuk, ugyhogy hozzaadjuk:
    // ball1->v = ball1->v - vperp1;
    // ball2->v = ball2->v - vperp2;

    vekt2 dr = ball2->keyframe_r - ball1->keyframe_r;
    dr.normal();
    vekt2 collision_plane = forgatas90fokkal(dr);
    double vtang1 = collision_plane * ball1->v;
    double vtang2 = collision_plane * ball2->v;
    double angular_v1 = ball1->angular_velocity;
    double angular_v2 = ball2->angular_velocity;
    double rad1 = ball1->radius;
    double rad2 = ball2->radius;
    double tangential_impulse =
        (vtang2 - vtang1 - angular_v1 * rad1 - angular_v2 * rad2) /
        (1 / mass1 + 1 / mass2 + rad1 * rad1 / inertia1 + rad1 * rad1 / inertia1);

    tangential_impulse /= 2.0;

    ball1->angular_velocity += tangential_impulse * rad1 / inertia1;
    ball2->angular_velocity += tangential_impulse * rad2 / inertia2;
    // ball1->v = ball1->v + tangential_impulse/mass1*collision_plane - vperp1;
    // ball2->v = ball2->v - tangential_impulse/mass2*collision_plane - vperp2;

    // vekt2 vuj = ((mass1/mass) * ball1->v) + ((mass2/mass) * ball2->v );
    // double elteres1 = abs( tomegpontteszt - vuj );

    int interation = 0;
    double step = 1.0;
    double adjust = 0.5;
    int increase_energy = 1;
    while (1) {
        interation++;

        vekt2 new_v1 = ball1->v + tangential_impulse / mass1 * collision_plane - vperp1 * step;
        vekt2 new_v2 = ball2->v - tangential_impulse / mass2 * collision_plane - vperp2 * step;

        double new_energy = mass1 * new_v1 * new_v1 + mass2 * new_v2 * new_v2 +
                            inertia1 * ball1->angular_velocity * ball1->angular_velocity +
                            inertia2 * ball2->angular_velocity * ball2->angular_velocity;

        if (increase_energy) {
            if (new_energy > initial_energy) {
                increase_energy = 0;
            } else {
                step *= 2.0;
                adjust *= 2.0;
            }
        } else {
            if (interation == 30) {
                ball1->v = new_v1;
                ball2->v = new_v2;
                if (fabs(new_energy - initial_energy) > 0.001) {
                    hiba("fabs( new_energy - initial_energy ) > 0.001!");
                }
                break;
            }

            if (new_energy > initial_energy) {
                step -= adjust;
            } else {
                step += adjust;
            }
            adjust *= 0.5001;
        }
    }

    // double elteresenerg = new_energy - energiateszt;
    // if( elteresenerg > 0.0 )
    //	hiba( "elteresnergia > 0.0!" );
}

static int WallLeft = 0, WallRight = SCREEN_WIDTH;
static int WallTop = 0, WallBottom = SCREEN_HEIGHT;

double ball_wall_collision_time(ball* ball, int wall) {
    if (WallsDisabled) {
        return NO_COLLISION_TIME;
    }
    switch (wall) {
    case Wall_Top:
        if (ball->v.y >= 0) {
            return NO_COLLISION_TIME;
        }
        if (ball->keyframe_r.y <= WallTop + ball->radius) {
            return NO_COLLISION_TIME;
        }
        return ball->keyframe_time - ((ball->keyframe_r.y - WallTop - ball->radius) / ball->v.y);
    case Wall_Bottom:
        if (ball->v.y <= 0) {
            return NO_COLLISION_TIME;
        }
        if (ball->keyframe_r.y >= WallBottom - ball->radius) {
            return NO_COLLISION_TIME;
        }
        return ball->keyframe_time + ((WallBottom - ball->radius - ball->keyframe_r.y) / ball->v.y);
    case Wall_Right:
        if (ball->v.x <= 0) {
            return NO_COLLISION_TIME;
        }
        if (ball->keyframe_r.x >= WallRight - ball->radius) {
            return NO_COLLISION_TIME;
        }
        return ball->keyframe_time + ((WallRight - ball->radius - ball->keyframe_r.x) / ball->v.x);
    case Wall_Left:
        if (ball->v.x >= 0) {
            return NO_COLLISION_TIME;
        }
        if (ball->keyframe_r.x <= WallLeft + ball->radius) {
            return NO_COLLISION_TIME;
        }
        return ball->keyframe_time - ((ball->keyframe_r.x - WallLeft - ball->radius) / ball->v.x);
    }
    return (NO_COLLISION_TIME);
}

void simulate_ball_wall_collision(ball* ball, int wall, double time) {
    if (WallsDisabled) {
        return;
    }
    update_keyframe(ball, time);
    switch (wall) {
    case Wall_Top:
    case Wall_Bottom:
        ball->v.y = -ball->v.y;
        break;
    case Wall_Right:
    case Wall_Left:
        ball->v.x = -ball->v.x;
        break;
    }
    // double ero = dsvabs( &ball->v );
    // GG_go_utkozes( ero );
}
