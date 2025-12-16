#include "ALL.H"

// Tudom hogy hiba WheelDeformationLength-ot physics_init.cpp-ben 0.02-re venni, amikor peldaul
// 0.005 is jo lenne. Hiba tovabba biztostalppont regi verziojat hasznalni
// uj helyett, de ezekkel nem ugyanugy viselkedne motor, mint regen.

// Nem engedi vonalon+egykicsi tovabb kereket!:
static void move_wheel_out_of_ground(rigidbody* rb, vekt2* point) {
    double length = abs(rb->r - *point);
    vekt2 n = (rb->r - *point) * (1.0 / length);
    if (length < rb->radius - WheelDeformationLength) {
        rb->r = rb->r + n * (rb->radius - WheelDeformationLength - length);
    }
}

// static double BumpThreshold = 1.5; // Itt mar nem hallatszik szelrol levalas
static double BumpThreshold = 1.5;

// Igazzal ter vissza, ha talppont tenylegesen az:
static int simulate_anchor_point_collision(rigidbody* rb, vekt2* point, vekt2 force) {
    double length = abs(rb->r - *point);
    vekt2 n = (rb->r - *point) * (1.0 / length);
    if (n * rb->v > -GroundEscapeVelocity && n * force > 0) {
        // Nem talppont:
        return 0;
    }
    // Sebesseg korrigalasa csak n-re meroleges komponensre:
    vekt2 deleted_velocity = n * rb->v * n;
    rb->v = rb->v - deleted_velocity;
    // Hang:
    double bump_magnitude = abs(deleted_velocity);
    if (bump_magnitude > BumpThreshold) {
        // bump_magnitude = bump_magnitude/BumpThreshold*0.1;
        bump_magnitude = bump_magnitude / 0.8 * 0.1;
        if (bump_magnitude >= 0.99) {
            bump_magnitude = 0.99;
        }
        startwavegyujto(WAV_UTODES, bump_magnitude, -1);
    }
    return 1;
}

// Igaz, ha talppont point1 (regi modszer):
static int valid_anchor_points_old(vekt2 point1, vekt2 point2, rigidbody* rb, vekt2 force,
                                   double torque) {
    double length = abs(rb->r - point2);
    vekt2 n = (rb->r - point2) * (1.0 / length);
    vekt2 n90 = forgatas90fokkal(n);
    double total_torque = torque + length * n90 * force;
    if ((point1 - point2) * n90 * total_torque < 0) {
        return 0;
    } else {
        return 1;
    }
}

// Igaz, ha talppont point1 (uj modszer):
static int valid_anchor_points_new(vekt2 point1, vekt2 point2, rigidbody* rb, vekt2 force,
                                   double torque) {
    double length = abs(rb->r - point2);
    vekt2 n = (rb->r - point2) * (1.0 / length);
    vekt2 n90 = forgatas90fokkal(n);
    double speed_direction = rb->angular_velocity + length * n90 * rb->v;
    if ((point1 - point2) * n90 * speed_direction < 0) {
        return 0;
    } else {
        return 1;
    }
}

/*int Voltosszemosas = 0;

int Konvex = 0;
int Konkav = 0;*/

/*int Pnyomva = 0;

rigidbody buf_pk;
vekt2 buf_F;
double buf_M, buf_dt;*/

void rigidbody_movement(rigidbody* rb, vekt2 force, double torque, double dt, int do_collision) {

    /*int betoltkezd = 0;
    if( betoltkezd ) {
        pk = &buf_pk;
        F = buf_F;
        M = buf_M;
        dt = buf_dt;
    }
    else {
        buf_pk = *pk;
        buf_F = F;
        buf_M = M;
        buf_dt = dt;
    } */

    int anchor_point_count = 0;
    vekt2 point1, point2;
    vonal *line1 = NULL, *line2 = NULL;
    if (do_collision) {
        anchor_point_count = talppontkereses(rb->r, rb->radius, &point1, &point2, &line1, &line2);
// Egy kis ellenorzes:
#ifdef TEST
        switch (anchor_point_count) {
        case 0:
            if (line1 || line2) {
                hiba("rigidbody_movement: 0 anchor_point_count assertion fail!");
            }
            break;
        case 1:
            if (!line1 || line2) {
                hiba("rigidbody_movement: 1 anchor_point_count assertion fail!");
            }
            break;
        case 2:
            if (!line1 || !line2) {
                hiba("rigidbody_movement: 2 anchor_point_count assertion fail!");
            }
            break;
        }
#endif
    }

    if (anchor_point_count > 0) {
        move_wheel_out_of_ground(rb, &point1);
    }
    if (anchor_point_count > 1) {
        move_wheel_out_of_ground(rb, &point2);
    }

    // Csak tanulmanyozasra:
    /*if( anchor_point_count > 1 ) {
        double tav = abs( point1-point2 );
        if( tav < 1.0 )
            //hiba( "tav < 1.0!" );
            Voltosszemosas = 1;
    } */

    // Megprobalja csokkenteni talppontszamok szamat ugy, hogy figyelembe
    // veszi masik talppont koruli elfordulast is!:
    // Uj modszer azt nezi, hogy sebesseg kiforditja-e
    // talppontbol.
    // Csak egy bizonyos sebesseg felett csinaljuk, hogy
    // Allo helyzetben ne forogjon magatol kerek:
    if (anchor_point_count == 2 && abs(rb->v) > 1.0) {
        if (!valid_anchor_points_new(point1, point2, rb, force, torque)) {
            anchor_point_count = 1;
            point1 = point2;
        } else {
            if (!valid_anchor_points_new(point2, point1, rb, force, torque)) {
                anchor_point_count = 1;
            }
        }
    }
    // Megprobalja csokkenteni talppontszamok szamat ugy, hogy figyelembe
    // veszi masik talppont koruli elfordulast is, de regi
    // modszerrel. Regi modszer azt nezi, hogy erok
    // kiforditjak-e talppontbol.
    // De csak kis sebessegeknel csinalja, mert eloszor egy
    // menetben megvarja amig 2 talppont lenullazza
    // sebesseget:
    if (anchor_point_count == 2 && abs(rb->v) < 1.0) {
        if (!valid_anchor_points_old(point1, point2, rb, force, torque)) {
            anchor_point_count = 1;
            point1 = point2;
        } else {
            if (!valid_anchor_points_old(point2, point1, rb, force, torque)) {
                anchor_point_count = 1;
            }
        }
    }

    if (anchor_point_count == 2) {
        if (!simulate_anchor_point_collision(rb, &point2, force)) {
            anchor_point_count = 1;
        }
    }
    if (anchor_point_count >= 1) {
        if (!simulate_anchor_point_collision(rb, &point1, force)) {
            if (anchor_point_count == 2) {
                anchor_point_count = 1;
                point1 = point2;
            } else {
                anchor_point_count = 0;
            }
        }
    }

    // Talppontok most mar be vannak allitva!
    if (anchor_point_count == 0) {
        // Szabadon forog a levegoben:
        double angular_acceleration = torque / rb->inertia;
        rb->angular_velocity += angular_acceleration * dt;
        rb->rotation += rb->angular_velocity * dt;
        vekt2 a = force * (1.0 / rb->mass);
        rb->v = rb->v + a * dt;
        rb->r = rb->r + rb->v * dt;
        return;
    }
    if (anchor_point_count == 2) {
        // Kerek meg van fogva:
        rb->v = Vekt2null;
        rb->angular_velocity = 0;
        // Voltosszemosas = 20; //kirajz-ban egy kis teglalaot rajzol ki
        return;
    }

    // Egy talppont korul fordul el kerek:
    double length = abs(rb->r - point1);
    vekt2 n = (rb->r - point1) * (1.0 / length);
    vekt2 n90 = forgatas90fokkal(n);
    rb->angular_velocity = rb->v * n90 * (1.0 / rb->radius);
    torque += force * n90 * rb->radius; // M-hez hozzajon meg ero is, mivel talpponthoz
                                        // vonatkoztatjuk!
    double inertia_edge = rb->inertia + rb->mass * length * length;
    double angular_acceleration = torque / inertia_edge;

    rb->angular_velocity += angular_acceleration * dt;
    rb->rotation += rb->angular_velocity * dt;
    rb->v = rb->angular_velocity * rb->radius * n90;
    rb->r = rb->r + rb->v * dt;
    return;
}

static void body_boundaries(motorst* mot, vekt2 i, vekt2 j) {
    // Konverzio i, j koordinata rendszerbe:
    double body_x, body_y;
    if (mot->flipped_bike) {
        body_x = i * (mot->bike.r - mot->body_r);
        body_y = j * (mot->body_r - mot->bike.r);
    } else {
        body_x = i * (mot->body_r - mot->bike.r);
        body_y = j * (mot->body_r - mot->bike.r);
    }
    vekt2 body_r(body_x, body_y);

    // Ulessel hatarolja vezetot:
    static vekt2 LINE_POINT(-0.35, 0.13);
    // static vekt2 LINE_SLOPE( 0.2-(-0.35), 0.22-(0.13) );
    static vekt2 LINE_SLOPE(0.14 - (-0.35), 0.36 - (0.13));

    static vekt2 LINE_SLOPE_ORTHO(-LINE_SLOPE.y, LINE_SLOPE.x);
    static vekt2 LINE_SLOPE_ORTHO_UNIT = egys(LINE_SLOPE_ORTHO);

    if ((body_r - LINE_POINT) * LINE_SLOPE_ORTHO_UNIT < 0.0) {
        // Vezeto ules alatt van:
        double distance = (body_r - LINE_POINT) * LINE_SLOPE_ORTHO_UNIT;
        body_r = body_r - LINE_SLOPE_ORTHO_UNIT * distance;
    }

    // Felulrol hatarolja vezetot:
    static const double ELLIPSE_HEIGHT = 0.48; // 0.5
    if (body_r.y > ELLIPSE_HEIGHT) {
        body_r.y = ELLIPSE_HEIGHT;
    }

    // Oldalrol hatarolja vezetot:
    if (body_r.x < -0.5) {
        body_r.x = -0.5;
    }

    static const double ELLIPSE_WIDTH = 0.26;
    if (body_r.x > ELLIPSE_WIDTH) {
        body_r.x = ELLIPSE_WIDTH;
    }

    static const double ELLIPSE_R2 = ELLIPSE_HEIGHT * ELLIPSE_HEIGHT;
    static const double ELLIPSE_R = sqrt(ELLIPSE_R2);
    if (body_r.x > 0 && body_r.y > 0) {
        static const double ELLIPSE_A2 =
            (ELLIPSE_HEIGHT / ELLIPSE_WIDTH) * (ELLIPSE_HEIGHT / ELLIPSE_WIDTH);
        double distance2 = body_r.x * body_r.x * ELLIPSE_A2 + body_r.y * body_r.y;
        if (distance2 > ELLIPSE_R2) {
            double distance = sqrt(distance2);
            double displacement_ratio = ELLIPSE_R / distance;
            body_r.x *= displacement_ratio;
            body_r.y *= displacement_ratio;
        }
    }

    // body_r = vekt2( 0.2, 0.22 );
    // body_r = body_r + vekt2( -(0.06), (0.14) );

    // Konverzio i, j koordinata rendszerbol:
    if (mot->flipped_bike) {
        mot->body_r = j * body_r.y - i * body_r.x + mot->bike.r;
    } else {
        mot->body_r = i * body_r.x + j * body_r.y + mot->bike.r;
    }
}

void body_movement(motorst* mot, vekt2 gravity, vekt2 i, vekt2 j, double dt) {
    body_boundaries(mot, i, j);

    // Rugoero szamitas:
    vekt2 neutral_body_r = mot->bike.r + j * BodyDY;
    vekt2 delta_body_r = neutral_body_r - mot->body_r;
    double spring_length = abs(delta_body_r);
    if (spring_length < 0.0000001) {
        spring_length = 0.0000001;
    }
    vekt2 force_spring_unit = delta_body_r * (1.0 / spring_length);
    double force_spring_scalar = spring_length * SpringTensionCoefficient * 5.0;
    vekt2 force_spring = force_spring_unit * force_spring_scalar;

    // Surlodasi ero szamitas:
    // Kiszamoljuk motor testenek sebesseget body_r helyen:
    vekt2 body_length_ortho = forgatas90fokkal(mot->body_r - mot->bike.r);
    vekt2 neutral_v = body_length_ortho * mot->bike.angular_velocity + mot->bike.v;
    vekt2 relative_v = mot->body_v - neutral_v;
    vekt2 force_damping = relative_v * SpringResistanceCoefficient * 3.0;

    vekt2 force_total = force_spring - force_damping + gravity * mot->bike.mass * Gravity;
    vekt2 a = force_total * (1.0 / mot->bike.mass); // Egyenlore kerek tomegevel szamolunk
    mot->body_v = mot->body_v + a * dt;
    mot->body_r = mot->body_r + mot->body_v * dt;
}
