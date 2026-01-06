#include "GYURU.H"
#include "ALL.H"

polygon::polygon(void) {
    vertex_count = 0;
    lefoglaltmaxpont = 10;

    // Koveto valtozok:
    is_grass = 0;

    vertices = new vect2[lefoglaltmaxpont];
    for (int i = 0; i < lefoglaltmaxpont; i++) {
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
        internal_error("polygon::set_vertex-ban v < 0 || v >= vertex_count!");
    }
    vertices[v].x = x;
    vertices[v].y = y;
}

void polygon::render_one_line(int v, int forward, int dotted) {
    if (v < 0 || v >= vertex_count) {
        internal_error("polygon::render_one_line-ban v < 0 || v >= vertex_count!");
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
            "polygon::render_outline-ban vertex_count < 3 || vertex_count > POLYGON_MAX_VERTICES!");
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
        internal_error("polygon::insert_vertex-ban v >= vertex_count!");
    }

    if (vertex_count >= lefoglaltmaxpont) {
        // Ujra fel kell venni pontok tombjet, csak most tobb ponttal:
        lefoglaltmaxpont += 10;
        vect2* ujponttomb = new vect2[lefoglaltmaxpont];
        for (int i = 0; i < lefoglaltmaxpont; i++) {
            ujponttomb[i].x = 0;
            ujponttomb[i].y = 0;
        }
        for (int i = 0; i < vertex_count; i++) {
            ujponttomb[i].x = vertices[i].x;
            ujponttomb[i].y = vertices[i].y;
        }
        delete vertices;
        vertices = ujponttomb;
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
        internal_error("polygon::delete_vertex-ben v < 0 || v >= vertex_count!");
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
    double tav = 1000000000000000.0;
    *v = 0;
    for (int i = 0; i < vertex_count; i++) {
        double dx = vertices[i].x - x;
        double dy = vertices[i].y - y;
        double ujtav = dx * dx + dy * dy;
        if (ujtav < tav) {
            tav = ujtav;
            *v = i;
        }
    }
    return sqrt(tav);
}

int polygon::count_intersections(vect2 r1, vect2 v1) {
#ifdef DEBUG
    if (is_grass) {
        internal_error("89yy");
    }
#endif

    int metszes = 0;
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
            metszes++;
        }
    }
    return metszes;
}

int polygon::intersection_point(vect2 r1, vect2 v1, int skip_v, vect2* intersect_point) {
#ifdef DEBUG
    if (is_grass) {
        internal_error("0i9p9");
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
    lefoglaltmaxpont = 0;
    vertices = NULL;
    is_grass = 0;

    // Koveto valtozok:
    int kovetos = 0;
    int kiskoveto = 0;
    if (version >= 8) {
        kovetos = 1;
    }
    if (version >= 12) {
        kiskoveto = 1;
    }

    if (kovetos) {
        if (fread(&is_grass, 1, 4, h) != 4) {
            internal_error("86t6yt!");
        }
        if (!kiskoveto && is_grass) {
            char semmi[20];
            if (fread(semmi, 1, 10, h) != 10) {
                internal_error("86t6yt!");
            }
            if (fread(semmi, 1, 10, h) != 10) {
                internal_error("86t6yt!");
            }
            if (fread(semmi, 1, 10, h) != 10) {
                internal_error("86t6yt!");
            }
            if (fread(semmi, 1, 4, h) != 4) {
                internal_error("86t6yt!");
            }
            if (fread(semmi, 1, 4, h) != 4) {
                internal_error("86t6yt!");
            }
        }
    }

    if (fread(&vertex_count, 1, sizeof(vertex_count), h) != sizeof(vertex_count)) {
        internal_error("polygon::polygon-ben nem megy file olvasas!");
    }

    if (vertex_count < 3 || vertex_count > POLYGON_MAX_VERTICES) {
        internal_error(
            "polygon::polygon-ben vertex_count < 3 || vertex_count > POLYGON_MAX_VERTICES!");
    }

    lefoglaltmaxpont = vertex_count + 10;
    vertices = new vect2[lefoglaltmaxpont];
    for (int i = 0; i < lefoglaltmaxpont; i++) {
        vertices[i].x = 0;
        vertices[i].y = 0;
    }
    if (fread(vertices, 1, sizeof(vect2) * vertex_count, h) != sizeof(vect2) * vertex_count) {
        internal_error("polygon::polygon-ben nem megy file olvasasa!");
    }
}

void polygon::save(FILE* h, topol* lev) {
    // Koveto reszek:
    if (fwrite(&is_grass, 1, 4, h) != 4) {
        internal_error("96t6yt!");
    }

    if (!is_grass) {
        // Ugy kell elmenteni, hogy ha sorban megyunk vegig pontokon,
        // akkor mindig baloldalon legyen fold:
        // Eloszor megnezzuk, hogy vonal ureg, vagy folddarab-e:
        int levegoben = lev->levegoben(this); // Ha levegoben, akkor folddarab

        int pozitivjaras = is_clockwise();

        int csere = (levegoben && !pozitivjaras) || (!levegoben && pozitivjaras);

        csere = !csere; // mivel fejjel lefele fogjuk beolvasni

        if (csere) {
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
        internal_error("polygon::save-ben nem megy file-ba iras!");
    }
    if (fwrite(vertices, 1, sizeof(vect2) * vertex_count, h) != sizeof(vect2) * vertex_count) {
        internal_error("polygon::save-ben nem megy file-ba iras!");
    }
}

// Visszaadja b pontban fekvo belso szoget, ha belso oldal bal oldal a, b, c
// haladas eseten:
static double balszog(vect2* pa, vect2 b, vect2 c) {
    while (1) {
        double dy1 = pa->y - b.y;
        double dx1 = pa->x - b.x;
        if (dy1 == 0.0 && dx1 == 0) {
            // Ezt mar topologia check-ben meg kellett volna szuntetni:
            internal_error("balszog-ben ket pont egybe esik!");
        }
        double alfa = atan2(dy1, dx1);
        double dy2 = c.y - b.y;
        double dx2 = c.x - b.x;
        if (dy2 == 0.0 && dx2 == 0) {
            // Ezt mar topologia check-ben meg kellett volna szuntetni:
            internal_error("balszog-ben ket pont egybe esik!");
        }
        double beta = atan2(dy2, dx2);
        double szog = alfa - beta;
        if (szog < 0) {
            szog += 2 * K_pi;
        }
        if (szog > 2 * K_pi - 0.0000001) {
            szog -= 2 * K_pi;
        }
        if (szog < 0.0000002) {
            // Szog tul hegyes, egyik pontjat arrebb rakjuk:
            constexpr double egytav = 0.0002;
            vect2 meroleges = *pa - b;
            meroleges = rotate_90deg(meroleges);
            meroleges.normalize();
            *pa = *pa + meroleges * egytav;
            // dialog( "Arrebb raktuk!" );

            continue;
        }
        return szog;
    }
}

static void szetvalaszt_fv(vect2* pa, vect2* pb) {
    constexpr double egytav = 0.0002;
    if (fabs(pa->x - pb->x) < 0.0000002 && fabs(pa->y - pb->y) < 0.0000002) {
        // Egy helyen vannak:
        pa->x += egytav + egytav * random_range(1000) / 1200.0;
        pa->y += egytav + egytav * random_range(1000) / 1200.0;
    }
}

void polygon::separate_stacked_vertices(void) {
    for (int i = 0; i < vertex_count; i++) {
        if (i < vertex_count - 1) {
            szetvalaszt_fv(&vertices[i], &vertices[i + 1]);
        } else {
            szetvalaszt_fv(&vertices[i], &vertices[0]);
        }
    }
}

// akkor ad igazat, ha korbejaraskor poligon belseje bal oldalon van;
int polygon::is_clockwise(void) {
#ifdef DEBUG
    if (is_grass) {
        internal_error("d6f65");
    }
#endif

    // Sorban osszeadjuk belso szogeket:
    double sum = 0.0;
    for (int i = 0; i < vertex_count - 2; i++) {
        sum += balszog(&vertices[i], vertices[i + 1], vertices[i + 2]);
    }
    sum += balszog(&vertices[vertex_count - 2], vertices[vertex_count - 1], vertices[0]);
    sum += balszog(&vertices[vertex_count - 1], vertices[0], vertices[1]);
    double maradek = sum - vertex_count * K_pi;
    if (fabs(fabs(maradek) - 2 * K_pi) > 0.1) {
        /*char tmp[100];
        sprintf( tmp, "Nagy az elteres, x100 = %d!", int(fabs( fabs( maradek ) - 2*K_pi )*100) );
        dialog( tmp );*/
        return 0; // Ilyenkor biztos hogy van egymast metszo ket vonal
    }
    if (maradek > 0.0) {
        return 0;
    } else {
        return 1;
    }
}

vect2 polygon::center(void) {
    vect2 suly(0.0, 0.0);
    for (int i = 0; i < vertex_count; i++) {
        suly = suly + vertices[i];
    }
    suly = suly * (1.0 / vertex_count);
    return suly;
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
    double belyeg = 0;
    for (int i = 0; i < vertex_count; i++) {
        belyeg += vertices[i].x;
        belyeg += vertices[i].y;
    }
    return belyeg;
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
