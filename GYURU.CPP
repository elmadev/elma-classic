#include "GYURU.H"
#include "ALL.H"

polygon::polygon(void) {
    vertex_count = 0;
    allocated_vertex_count = 10;

    // Koveto valtozok:
    is_grass = 0;

    vertices = new vect2[allocated_vertex_count];
    for (int i = 0; i < allocated_vertex_count; i++) {
        vertices[i].x = 0;
        vertices[i].y = 0;
    }
    // Most csak beteszunk harom pontot:
    vertices[0].x = -24;
    vertices[0].y = -8;
    vertices[1].x = 24;
    vertices[1].y = -8;
    vertices[2].x = 24;
    vertices[2].y = 2;
    vertices[3].x = -24;
    vertices[3].y = 2;
    vertex_count = 4;
}

polygon::~polygon(void) {
    if (vertices) {
        delete vertices;
    }
    vertices = NULL;
}

void polygon::set_vertex(int v, double x, double y) {
    if (v < 0 || v >= vertex_count) {
        internal_error("polygon::set_vertex v < 0 || v >= vertex_count!");
    }
    vertices[v].x = x;
    vertices[v].y = y;
}

void polygon::render_one_line(int v, int forward, int dotted) {
    if (v < 0 || v >= vertex_count) {
        internal_error("polygon::render_one_line v < 0 || v >= vertex_count!");
    }
    if ((forward && v == vertex_count - 1) || (!forward && v == 0)) {
        // Elejet a vegevel kell osszekotni:
        render_line(vertices[vertex_count - 1], vertices[0], dotted);
    } else {
        // Normal osszekotes:
        if (forward) {
            render_line(vertices[v], vertices[v + 1], dotted);
        } else {
            render_line(vertices[v - 1], vertices[v], dotted);
        }
    }
}

void polygon::render_outline(void) {
    if (vertex_count < 3 || vertex_count > POLYGON_MAX_VERTICES) {
        internal_error(
            "polygon::render_outline vertex_count < 3 || vertex_count > POLYGON_MAX_VERTICES!");
    }
    for (int i = 0; i < vertex_count; i++) {
        render_one_line(i, 1, 0);
    }
}

// k pont utan beszur egyet ugyanolyan koordokkal
int polygon::insert_vertex(int v) {
    if (vertex_count + 1 > POLYGON_MAX_VERTICES) {
        dialog("You cannot add more points to this polygon!");
        return 0;
    }
    if (v >= vertex_count) {
        internal_error("polygon::insert_vertex v >= vertex_count!");
    }

    if (vertex_count >= allocated_vertex_count) {
        // Ujra fel kell venni pontok tombjet, csak most tobb ponttal:
        allocated_vertex_count += 10;
        vect2* new_vertices = new vect2[allocated_vertex_count];
        for (int i = 0; i < allocated_vertex_count; i++) {
            new_vertices[i].x = 0;
            new_vertices[i].y = 0;
        }
        for (int i = 0; i < vertex_count; i++) {
            new_vertices[i].x = vertices[i].x;
            new_vertices[i].y = vertices[i].y;
        }
        delete vertices;
        vertices = new_vertices;
    }

    for (int i = vertex_count - 1; i >= v; i--) {
        vertices[i + 1].x = vertices[i].x;
        vertices[i + 1].y = vertices[i].y;
    }
    vertex_count++;
    return 1;
}

void polygon::delete_vertex(int v) {
    if (v < 0 || v >= vertex_count) {
        internal_error("polygon::delete_vertex v < 0 || v >= vertex_count!");
    }
    if (vertex_count <= 3) {
        return;
    }
    for (int i = v; i < vertex_count - 1; i++) {
        vertices[i].x = vertices[i + 1].x;
        vertices[i].y = vertices[i + 1].y;
    }
    vertex_count--;
}

double polygon::get_closest_vertex(double x, double y, int* v) {
    double distance = 1000000000000000.0;
    *v = 0;
    for (int i = 0; i < vertex_count; i++) {
        double dx = vertices[i].x - x;
        double dy = vertices[i].y - y;
        double new_distance = dx * dx + dy * dy;
        if (new_distance < distance) {
            distance = new_distance;
            *v = i;
        }
    }
    return sqrt(distance);
}

int polygon::count_intersections(vect2 r1, vect2 v1) {
#ifdef DEBUG
    if (is_grass) {
        internal_error("polygon::count_intersections grass cannot intersect!");
    }
#endif

    int intersections = 0;
    for (int i = 0; i < vertex_count; i++) {
        double x1 = vertices[i].x;
        double y1 = vertices[i].y;
        double x2, y2;
        if (i < vertex_count - 1) {
            // Nem az utolso pont:
            x2 = vertices[i + 1].x;
            y2 = vertices[i + 1].y;
        } else {
            // Az utolso pont:
            x2 = vertices[0].x;
            y2 = vertices[0].y;
        }
        vect2 r2(x1, y1);
        vect2 v2 = vect2(x2, y2) - r2;
        if (segments_intersect(r1, v1, r2, v2)) {
            intersections++;
        }
    }
    return intersections;
}

int polygon::intersection_point(vect2 r1, vect2 v1, int skip_v, vect2* intersect_point) {
#ifdef DEBUG
    if (is_grass) {
        internal_error("polygon::intersection_point grass cannot intersect!");
    }
#endif

    // Eloszor felvesszuk allando szakasz min es max pontjait:
    double minx;
    if (v1.x >= 0) {
        minx = r1.x;
    } else {
        minx = r1.x + v1.x;
    }
    double maxx;
    if (v1.x >= 0) {
        maxx = r1.x + v1.x;
    } else {
        maxx = r1.x;
    }
    // Y:
    double miny;
    if (v1.y >= 0) {
        miny = r1.y;
    } else {
        miny = r1.y + v1.y;
    }
    double maxy;
    if (v1.y >= 0) {
        maxy = r1.y + v1.y;
    } else {
        maxy = r1.y;
    }

    // Vegigmegyunk polygon pontjain:
    for (int i = 0; i < vertex_count; i++) {
        if (i == skip_v) {
            continue;
        }

        // Szomszedos szakaszokat nem vizsgalunk:
        if (skip_v >= 0) {
            if (skip_v == 0) {
                if (i == vertex_count - 1) {
                    continue;
                }
            } else {
                if (i == skip_v - 1) {
                    continue;
                }
            }

            if (skip_v == vertex_count - 1) {
                if (i == 0) {
                    continue;
                }
            } else {
                if (i == skip_v + 1) {
                    continue;
                }
            }
        }

        double x1 = vertices[i].x;
        double y1 = vertices[i].y;
        double x2, y2;
        if (i < vertex_count - 1) {
            // Nem az utolso pont:
            x2 = vertices[i + 1].x;
            y2 = vertices[i + 1].y;
        } else {
            // Az utolso pont:
            x2 = vertices[0].x;
            y2 = vertices[0].y;
        }

        // Eloalltak masodik szakasz pontjai is, megnezzuk hatha annyira
        // kilognak egymasbol, hogy rogton kizarhatjuk metszest:
        if ((x1 < minx && x2 < minx) || (x1 > maxx && x2 > maxx) || (y1 < miny && y2 < miny) ||
            (y1 > maxy && y2 > maxy)) {
            continue;
        }

        vect2 r2(x1, y1);
        vect2 v2 = vect2(x2, y2) - r2;
        if (segments_intersect_inexact(r1, v1, r2, v2)) {
            *intersect_point = intersection(r1, v1, r2, v2);
            return 1;
        }
    }
    return 0;
}

int polygon::get_vertex_count(void) { return vertex_count; }

polygon::polygon(FILE* h, int version) {
    vertex_count = 0;
    allocated_vertex_count = 0;
    vertices = NULL;
    is_grass = 0;

    // Koveto valtozok:
    int deprecated_grass_feature = 0;
    int grass_feature = 0;
    if (version >= 8) {
        deprecated_grass_feature = 1;
    }
    if (version >= 12) {
        grass_feature = 1;
    }

    if (deprecated_grass_feature) {
        if (fread(&is_grass, 1, 4, h) != 4) {
            internal_error("polygon::polygon: Failed to read file!");
        }
        if (!grass_feature && is_grass) {
            char unused[20];
            if (fread(unused, 1, 10, h) != 10) {
                internal_error("polygon::polygon: Failed to read file!");
            }
            if (fread(unused, 1, 10, h) != 10) {
                internal_error("polygon::polygon: Failed to read file!");
            }
            if (fread(unused, 1, 10, h) != 10) {
                internal_error("polygon::polygon: Failed to read file!");
            }
            if (fread(unused, 1, 4, h) != 4) {
                internal_error("polygon::polygon: Failed to read file!");
            }
            if (fread(unused, 1, 4, h) != 4) {
                internal_error("polygon::polygon: Failed to read file!");
            }
        }
    }

    if (fread(&vertex_count, 1, sizeof(vertex_count), h) != sizeof(vertex_count)) {
        internal_error("polygon::polygon: Failed to read file!");
    }

    if (vertex_count < 3 || vertex_count > POLYGON_MAX_VERTICES) {
        internal_error("polygon::polygon vertex_count < 3 || vertex_count > POLYGON_MAX_VERTICES!");
    }

    allocated_vertex_count = vertex_count + 10;
    vertices = new vect2[allocated_vertex_count];
    for (int i = 0; i < allocated_vertex_count; i++) {
        vertices[i].x = 0;
        vertices[i].y = 0;
    }
    if (fread(vertices, 1, sizeof(vect2) * vertex_count, h) != sizeof(vect2) * vertex_count) {
        internal_error("polygon::polygon: Failed to read file!");
    }
}

void polygon::save(FILE* h, topol* lev) {
    // Koveto reszek:
    if (fwrite(&is_grass, 1, 4, h) != 4) {
        internal_error("polygon::polygon: Failed to write file!");
    }

    if (!is_grass) {
        // Ugy kell elmenteni, hogy ha sorban megyunk vegig pontokon,
        // akkor mindig baloldalon legyen fold:
        // Eloszor megnezzuk, hogy vonal ureg, vagy folddarab-e:
        int sky = lev->levegoben(this); // Ha levegoben, akkor folddarab

        int clockwise = is_clockwise();

        int swap_orientation = (sky && !clockwise) || (!sky && clockwise);

        swap_orientation = !swap_orientation; // mivel fejjel lefele fogjuk beolvasni

        if (swap_orientation) {
            // dialog( "Most cserel." );

            // Meg kell cserelni pontok sorrendjet:
            for (int i = 0; i < vertex_count / 2; i++) {
                int j = vertex_count - 1 - i;
                vect2 tmpv = vertices[i];
                vertices[i] = vertices[j];
                vertices[j] = tmpv;
            }
        }
    }

    // Itt jon a mentes lenyegeben:
    if (fwrite(&vertex_count, 1, sizeof(vertex_count), h) != sizeof(vertex_count)) {
        internal_error("polygon::polygon: Failed to write file!");
    }
    if (fwrite(vertices, 1, sizeof(vect2) * vertex_count, h) != sizeof(vect2) * vertex_count) {
        internal_error("polygon::polygon: Failed to write file!");
    }
}

// Visszaadja b pontban fekvo belso szoget, ha belso oldal bal oldal a, b, c
// haladas eseten:
static double get_and_fix_angle(vect2* a, vect2 b, vect2 c) {
    while (1) {
        double dy1 = a->y - b.y;
        double dx1 = a->x - b.x;
        if (dy1 == 0.0 && dx1 == 0) {
            // Ezt mar topologia check-ben meg kellett volna szuntetni:
            internal_error("get_and_fix_angle dx1 and dy1 == 0.0!");
        }
        double angle1 = atan2(dy1, dx1);
        double dy2 = c.y - b.y;
        double dx2 = c.x - b.x;
        if (dy2 == 0.0 && dx2 == 0) {
            // Ezt mar topologia check-ben meg kellett volna szuntetni:
            internal_error("get_and_fix_angle dx2 and dy2 == 0.0!");
        }
        double angle2 = atan2(dy2, dx2);
        double angle = angle1 - angle2;
        if (angle < 0) {
            angle += 2 * K_pi;
        }
        if (angle > 2 * K_pi - 0.0000001) {
            angle -= 2 * K_pi;
        }
        if (angle < 0.0000002) {
            // Szog tul hegyes, egyik pontjat arrebb rakjuk:
            constexpr double displacement = 0.0002;
            vect2 perpendicular = *a - b;
            perpendicular = rotate_90deg(perpendicular);
            perpendicular.normalize();
            *a = *a + perpendicular * displacement;
            // dialog( "Arrebb raktuk!" );

            continue;
        }
        return angle;
    }
}

static void separate_two_stacked_vertices(vect2* a, vect2* b) {
    constexpr double displacement = 0.0002;
    if (fabs(a->x - b->x) < 0.0000002 && fabs(a->y - b->y) < 0.0000002) {
        // Egy helyen vannak:
        a->x += displacement + displacement * random_range(1000) / 1200.0;
        a->y += displacement + displacement * random_range(1000) / 1200.0;
    }
}

void polygon::separate_stacked_vertices(void) {
    for (int i = 0; i < vertex_count; i++) {
        if (i < vertex_count - 1) {
            separate_two_stacked_vertices(&vertices[i], &vertices[i + 1]);
        } else {
            separate_two_stacked_vertices(&vertices[i], &vertices[0]);
        }
    }
}

// akkor ad igazat, ha korbejaraskor poligon belseje bal oldalon van;
int polygon::is_clockwise(void) {
#ifdef DEBUG
    if (is_grass) {
        internal_error("polygon::is_clockwise grass has no orientation!");
    }
#endif

    // Sorban osszeadjuk belso szogeket:
    double sum = 0.0;
    for (int i = 0; i < vertex_count - 2; i++) {
        sum += get_and_fix_angle(&vertices[i], vertices[i + 1], vertices[i + 2]);
    }
    sum += get_and_fix_angle(&vertices[vertex_count - 2], vertices[vertex_count - 1], vertices[0]);
    sum += get_and_fix_angle(&vertices[vertex_count - 1], vertices[0], vertices[1]);
    double normalized_angle = sum - vertex_count * K_pi;
    if (fabs(fabs(normalized_angle) - 2 * K_pi) > 0.1) {
        /*char tmp[100];
        sprintf( tmp, "Nagy az elteres, x100 = %d!", int(fabs( fabs( normalized_angle ) - 2*K_pi
        )*100) ); dialog( tmp );*/
        return 0; // Ilyenkor biztos hogy van egymast metszo ket vonal
    }
    if (normalized_angle > 0.0) {
        return 0;
    } else {
        return 1;
    }
}

vect2 polygon::center(void) {
    vect2 centerpoint(0.0, 0.0);
    for (int i = 0; i < vertex_count; i++) {
        centerpoint = centerpoint + vertices[i];
    }
    centerpoint = centerpoint * (1.0 / vertex_count);
    return centerpoint;
}

void polygon::update_boundaries(double* x1, double* y1, double* x2, double* y2) {
    for (int i = 0; i < vertex_count; i++) {
        double x = vertices[i].x;
        double y = vertices[i].y;
        if (*x1 > x) {
            *x1 = x;
        }
        if (*y1 > y) {
            *y1 = y;
        }
        if (*x2 < x) {
            *x2 = x;
        }
        if (*y2 < y) {
            *y2 = y;
        }
    }
}

double polygon::checksum(void) {
    double sum = 0;
    for (int i = 0; i < vertex_count; i++) {
        sum += vertices[i].x;
        sum += vertices[i].y;
    }
    return sum;
}

/*void set_gyuru_attributes( polygon* pgy ) {
    Valtozott = 1;
    if( pgy->is_grass ) {
        pgy->is_grass = 0;
    }
    else {
        pgy->is_grass = 1;
        strcpy( pgy->kovetonev, "noname" );
        strcpy( pgy->alsotexturanev, "diam1_1" );
        strcpy( pgy->felsotexturanev, "elso_n_1" );
        pgy->tavolsag = 100;
        pgy->hatarol = 0; // Nem hatarolt
    }
}*/
