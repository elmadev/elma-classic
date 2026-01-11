#include "SZAKASZ.H"
#include "main.h"
#include "polygon.h"
#include "TOPOL.H"
#include <cmath>
#include <cstring>

static int Szakaszmaxszam = MAX_VERTICES + 130;

segments* Segments = NULL;
// szakaszok* Pszakt = NULL;

// loadszakasz beleirja ptop->lezart-at ha topologia file-t olvas be:
// Ezt azutan afterplayext fuggveny hasznalja ki:
int Ucsoloadszakasz_lezart = 0;

segments::segments(topol* lev) {
    // Elejen ugyanaz, mint void konstruktor:
    seg_list = NULL;
    seg_list_allocated_length = seg_list_length = 0;
    collision_grid = NULL;
    collision_grid_width = 1;
    collision_grid_height = 1;
    collision_grid_cell_size = 1.0;
    collision_grid_origin = vect2(0, 0);
    collision_grid_iterable = NULL;
    node_array = NULL;
    node_array_index = 0;

    seg_list = new segment[Szakaszmaxszam];
    if (!seg_list) {
        external_error("memory szakaszoknak!");
        return;
    }
    seg_list_allocated_length = Szakaszmaxszam;
    for (int i = 0; i < seg_list_allocated_length; i++) {
        memset(&seg_list[i], 0, sizeof(segment));
    }

    // Normal szakaszok beolvasasa:
    for (int i = 0; i < MAXGYURU; i++) {
        polygon* pgy = lev->ptomb[i];
        if (!pgy) {
            continue;
        }
        if (pgy->is_grass) {
            continue;
        }
        for (int j = 0; j < pgy->vertex_count; j++) {
            if (seg_list_length >= Szakaszmaxszam) {
                internal_error("segments::segments-ban z >= Szakaszmaxszam!");
            }
            vect2 r1, r2;
            if (j < pgy->vertex_count - 1) {
                r1 = pgy->vertices[j];
                r2 = pgy->vertices[j + 1];
            } else {
                r1 = pgy->vertices[j];
                r2 = pgy->vertices[0];
            }
            seg_list[seg_list_length].r = r1;
            seg_list[seg_list_length].v = r2 - r1;
            seg_list[seg_list_length].r.y = -seg_list[seg_list_length].r.y; // fejjel lefele
            seg_list[seg_list_length].v.y = -seg_list[seg_list_length].v.y; // fejjel lefele
            seg_list_length++;
        }
    }
}

segments::~segments(void) {
    if (seg_list) {
        delete seg_list;
    }
    if (collision_grid) {
        delete collision_grid;
    }
    delete_all_nodes();
}

// NODE-kezeles:

segment_node* segments::new_node(void) {
    if (!node_array) {
        node_array = new segment_node_array;
        if (!node_array) {
            external_error("memory!");
        }
        node_array->next = NULL;
        node_array_index = 0;
    }
    segment_node_array* curcsomag = node_array;
    while (curcsomag->next) {
        curcsomag = curcsomag->next;
    }
    if (node_array_index == SEGMENT_NODE_BLOCK_LENGTH) {
        curcsomag->next = new segment_node_array;
        curcsomag = curcsomag->next;
        if (!curcsomag) {
            external_error("memory!");
        }
        curcsomag->next = NULL;
        node_array_index = 0;
    }
    node_array_index++;
    return &curcsomag->nodes[node_array_index - 1];
}

void segments::delete_all_nodes(void) {
    segment_node_array* curcsomag = node_array;
    node_array = NULL;
    while (curcsomag) {
        segment_node_array* elozocsomag = curcsomag;
        curcsomag = curcsomag->next;
        delete elozocsomag;
    }
}

// Egy cellaba bekoti vonalat, ha meg nincs benne es rajta van tablan:
void segments::add_node_to_cell(int cell_x, int cell_y, segment* seg) {
#ifdef DEBUG
    if (cell_x < 0 || cell_y < 0) {
        internal_error("cell_x < 0 || cell_y < 0!");
    }
#endif
    if (cell_x >= collision_grid_width || cell_y >= collision_grid_height) {
        return;
    }
    segment_node* pujnode = new_node();
    pujnode->next = NULL;
    pujnode->seg = seg;
    segment_node* pn = collision_grid[collision_grid_width * cell_y + cell_x];
    if (!pn) {
        collision_grid[collision_grid_width * cell_y + cell_x] = pujnode;
        return;
    }
    while (pn->next) {
        pn = pn->next;
    }
    // Most pn az utolso node-ra mutat!
    pn->next = pujnode;
}

// Egy vonalat hozzarendel osszes olyan cellahoz, amihez koze lehet:
void segments::add_segment_to_collision_grid(segment* seg, double max_radius) {
    // Kitolti vonal egyseg es hossz rubrikait:
    seg->length = seg->v.length();
    if (seg->length < 0.00000001) {
        internal_error("Vonal hossza tul kicsi!");
    }
    seg->unit_vector = unit_vector(seg->v);

    vect2 v = seg->v;
    vect2 r = seg->r - collision_grid_origin;

    v = v * (1 / collision_grid_cell_size);
    r = r * (1 / collision_grid_cell_size);
    max_radius *= 1.5 / collision_grid_cell_size;
    // Ezentul cellameretet lehet 1.0-nak venni!
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
    // Most x iranyban biztosan hosszabb vagy egyenlo y iranynal!
    if (v.x < 0) {
        // Vegpont csere:
        r = r + v;
        v = Vect2null - v;
    }
    // Most v x-pozitiv iranyba mutat:
    // Kiszamoljuk y = m*x + y0 fuggveny parametereit:
    double m = v.y / v.x;
    double y0 = r.y - m * r.x;
    double xkezdo = r.x - max_radius;
    int cellax = 0;
    if (xkezdo > 0) {
        cellax = (int)(xkezdo);
    }
    if (r.x + v.x + max_radius < 0) {
        internal_error("segments::add_segment_to_collision_grid-ban r.x+v.x+max_radius < 0!");
    }
    int ucsocellax = (int)(r.x + v.x + max_radius);
    while (cellax <= ucsocellax) {
        double y1 = m * cellax + y0;
        double y2 = m * (cellax + 1) + y0;
        // y1-nek kissebnek kell lennie y2-nel:
        if (y1 > y2) {
            double tmpd = y1;
            y1 = y2;
            y2 = tmpd;
        }
        y1 -= max_radius;
        y2 += max_radius;
#ifdef DEBUG
        if (y1 > y2) {
            internal_error("Bug!");
        }
#endif
        if (y2 < 0.0) {
            internal_error("Bug!");
        }
        int cellay = 0;
        if (y1 > 0) {
            cellay = (int)(y1);
        }
        int ucsocellay = (int)(y2);
        while (cellay <= ucsocellay) {
            if (invertalt) {
                add_node_to_cell(cellay, cellax, seg);
            } else {
                add_node_to_cell(cellax, cellay, seg);
            }

            cellay++;
        }

        cellax++;
    }
}

void segments::setup_collision_grid(double max_radius) {
    // Ellenorzes:
    if (seg_list_length <= 0) {
        internal_error("segments::setup_collision_grid-ben seg_list_length <= 0!");
    }
    if (collision_grid) {
        internal_error("segments::setup_collision_grid-ban nodes != NULL!");
    }

    collision_grid_cell_size = 1.0;

    // Megkeresi palya kiterjedeset:
    iterate_all_segments();
    segment* psz = next_segment();
    if (!psz) {
        internal_error("Nincs egyetlen szakasz sem definialva!");
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

        psz = next_segment();
    }

    // Kicsit kibovitjuk hatarokat:
    minx -= 6.0;
    miny -= 6.0;
    maxx += 6.0;
    maxy += 6.0;

    // Beallitja kezdopontot:
    collision_grid_origin = vect2(minx, miny);
    double xsized = maxx - minx;
    double ysized = maxy - miny;

    // Kiszamolja dimenziot:
    collision_grid_width = (int)(xsized / collision_grid_cell_size + 1.0);
    collision_grid_height = (int)(ysized / collision_grid_cell_size + 1.0);
    if (collision_grid_width < 0 || collision_grid_height < 0) {
        internal_error("collision_grid_width < 0 || collision_grid_height < 0!");
    }
    if (collision_grid_width > 200 || collision_grid_height > 200) {
        internal_error("collision_grid_width > 200 || collision_grid_height > 200!");
    }

    // Lefoglalja tombot:
    unsigned tombmeret = collision_grid_width * collision_grid_height;
    collision_grid = new psegment_node[tombmeret];
    if (!collision_grid) {
        external_error("memory!");
    }
    int hatar = collision_grid_width * collision_grid_height;
    for (int i = 0; i < hatar; i++) {
        collision_grid[i] = NULL;
    }

    // Most vegigmegy szakaszokon es beilleszti oket:
    iterate_all_segments();
    psz = next_segment();
    while (psz) {
        add_segment_to_collision_grid(psz, max_radius);
        psz = next_segment();
    }
}

void segments::iterate_collision_grid_cell_segments(vect2 r) {
    if (!collision_grid) {
        internal_error("segments::iterate_collision_grid_cell_segments-ban !collision_grid!");
    }
    r = (r - collision_grid_origin) * (1 / collision_grid_cell_size);
    int cellax = 0;
    if (r.x > 0) {
        cellax = (int)(r.x);
    }
    int cellay = 0;
    if (r.y > 0) {
        cellay = (int)(r.y);
    }

    if (cellax > collision_grid_width) {
        internal_error(
            "segments::iterate_collision_grid_cell_segments-ben cellax > collision_grid_width!");
    }
    if (cellax == collision_grid_width) {
        cellax = collision_grid_width - 1;
    }
    if (cellay > collision_grid_height) {
        internal_error(
            "segments::iterate_collision_grid_cell_segments-ben cellay > collision_grid_height!");
    }
    if (cellay == collision_grid_height) {
        cellay = collision_grid_height - 1;
    }
    collision_grid_iterable = collision_grid[collision_grid_width * cellay + cellax];
}

segment* segments::next_collision_grid_segment(void) {
    if (!collision_grid_iterable) {
        return NULL;
    }
#ifdef DEBUG
    if (!collision_grid_iterable->seg) {
        internal_error("segments::next_collision_grid_segment-ben !collision_grid_iterable->seg");
    }
#endif
    segment* pret = collision_grid_iterable->seg;
    collision_grid_iterable = collision_grid_iterable->next;
    return pret;
}

// Szakaszos kereso:
void segments::iterate_all_segments(void) { seg_list_iteration_index = 0; }

segment* segments::next_segment(void) {
    if (seg_list_iteration_index >= seg_list_length) {
        return NULL;
    }
    seg_list_iteration_index++;
    return &seg_list[seg_list_iteration_index - 1];
}

segment* segments::getptrszakasz(int sorszam) {
    if (sorszam >= seg_list_length) {
        internal_error("segments::getptrszakasz-ban sorszam >= seg_list_length!");
    }
    return &seg_list[sorszam];
}
