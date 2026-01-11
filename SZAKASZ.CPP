#include "SZAKASZ.H"
#include "main.h"
#include "polygon.h"
#include "TOPOL.H"
#include <cmath>
#include <cstring>

static int MAX_SEGMENTS = MAX_VERTICES + 130;

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

    seg_list = new segment[MAX_SEGMENTS];
    if (!seg_list) {
        external_error("segments::segments out of memory!");
        return;
    }
    seg_list_allocated_length = MAX_SEGMENTS;
    for (int i = 0; i < seg_list_allocated_length; i++) {
        memset(&seg_list[i], 0, sizeof(segment));
    }

    // Normal szakaszok beolvasasa:
    for (int i = 0; i < MAXGYURU; i++) {
        polygon* poly = lev->ptomb[i];
        if (!poly) {
            continue;
        }
        if (poly->is_grass) {
            continue;
        }
        for (int j = 0; j < poly->vertex_count; j++) {
            if (seg_list_length >= MAX_SEGMENTS) {
                internal_error("segments::segments seg_list_length >= MAX_SEGMENTS!");
            }
            vect2 r1, r2;
            if (j < poly->vertex_count - 1) {
                r1 = poly->vertices[j];
                r2 = poly->vertices[j + 1];
            } else {
                r1 = poly->vertices[j];
                r2 = poly->vertices[0];
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
            external_error("segments::new_node out of memory!");
        }
        node_array->next = NULL;
        node_array_index = 0;
    }
    segment_node_array* cur_array = node_array;
    while (cur_array->next) {
        cur_array = cur_array->next;
    }
    if (node_array_index == SEGMENT_NODE_BLOCK_LENGTH) {
        cur_array->next = new segment_node_array;
        cur_array = cur_array->next;
        if (!cur_array) {
            external_error("segments::new_node out of memory!");
        }
        cur_array->next = NULL;
        node_array_index = 0;
    }
    node_array_index++;
    return &cur_array->nodes[node_array_index - 1];
}

void segments::delete_all_nodes(void) {
    segment_node_array* cur_array = node_array;
    node_array = NULL;
    while (cur_array) {
        segment_node_array* delete_array = cur_array;
        cur_array = cur_array->next;
        delete delete_array;
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
    segment_node* new_n = new_node();
    new_n->next = NULL;
    new_n->seg = seg;
    segment_node* cur_n = collision_grid[collision_grid_width * cell_y + cell_x];
    if (!cur_n) {
        collision_grid[collision_grid_width * cell_y + cell_x] = new_n;
        return;
    }
    while (cur_n->next) {
        cur_n = cur_n->next;
    }
    // Most pn az utolso node-ra mutat!
    cur_n->next = new_n;
}

// Egy vonalat hozzarendel osszes olyan cellahoz, amihez koze lehet:
void segments::add_segment_to_collision_grid(segment* seg, double max_radius) {
    // Kitolti vonal egyseg es hossz rubrikait:
    seg->length = seg->v.length();
    if (seg->length < 0.00000001) {
        internal_error("segments::add_nodes_to_collision_grid too short!");
    }
    seg->unit_vector = unit_vector(seg->v);

    vect2 v = seg->v;
    vect2 r = seg->r - collision_grid_origin;

    v = v * (1 / collision_grid_cell_size);
    r = r * (1 / collision_grid_cell_size);
    max_radius *= 1.5 / collision_grid_cell_size;
    // Ezentul cellameretet lehet 1.0-nak venni!
    int invert_axes = 0;
    if (fabs(v.y) > fabs(v.x)) {
        invert_axes = 1;
        double tmp = v.x;
        v.x = v.y;
        v.y = tmp;
        tmp = r.x;
        r.x = r.y;
        r.y = tmp;
    }
    // Most x iranyban biztosan hosszabb vagy egyenlo y iranynal!
    if (v.x < 0) {
        // Vegpont csere:
        r = r + v;
        v = Vect2null - v;
    }
    // Most v x-pozitiv iranyba mutat:
    // Kiszamoljuk y = m*x + y0 fuggveny parametereit:
    double slope = v.y / v.x;
    double y0 = r.y - slope * r.x;
    double xstart = r.x - max_radius;
    int cell_x = 0;
    if (xstart > 0) {
        cell_x = (int)(xstart);
    }
    if (r.x + v.x + max_radius < 0) {
        internal_error("segments::add_segment_to_collision_grid r.x+v.x+max_radius < 0!");
    }
    int xend = (int)(r.x + v.x + max_radius);
    while (cell_x <= xend) {
        double y1 = slope * cell_x + y0;
        double y2 = slope * (cell_x + 1) + y0;
        // y1-nek kissebnek kell lennie y2-nel:
        if (y1 > y2) {
            double tmp = y1;
            y1 = y2;
            y2 = tmp;
        }
        y1 -= max_radius;
        y2 += max_radius;
#ifdef DEBUG
        if (y1 > y2) {
            internal_error("segments::add_nodes_to_collision_grid not y1 > y2!");
        }
#endif
        if (y2 < 0.0) {
            internal_error("segments::add_nodes_to_collision_grid! not y2 < 0.0");
        }
        int cell_y = 0;
        if (y1 > 0) {
            cell_y = (int)(y1);
        }
        int yend = (int)(y2);
        while (cell_y <= yend) {
            if (invert_axes) {
                add_node_to_cell(cell_y, cell_x, seg);
            } else {
                add_node_to_cell(cell_x, cell_y, seg);
            }

            cell_y++;
        }

        cell_x++;
    }
}

void segments::setup_collision_grid(double max_radius) {
    // Ellenorzes:
    if (seg_list_length <= 0) {
        internal_error("segments::setup_collision_grid no lines!");
    }
    if (collision_grid) {
        internal_error("segments::setup_collision_grid already setup!");
    }

    collision_grid_cell_size = 1.0;

    // Megkeresi palya kiterjedeset:
    iterate_all_segments();
    segment* seg = next_segment();
    if (!seg) {
        internal_error("segments::setup_collision_grid no lines found!");
    }
    double minx = seg->r.x;
    double maxx = seg->r.x;
    double miny = seg->r.y;
    double maxy = seg->r.y;
    while (seg) {
        if (seg->r.x < minx) {
            minx = seg->r.x;
        }
        if (seg->r.x > maxx) {
            maxx = seg->r.x;
        }
        if (seg->r.y < miny) {
            miny = seg->r.y;
        }
        if (seg->r.y > maxy) {
            maxy = seg->r.y;
        }

        if (seg->r.x + seg->v.x < minx) {
            minx = seg->r.x + seg->v.x;
        }
        if (seg->r.x + seg->v.x > maxx) {
            maxx = seg->r.x + seg->v.x;
        }
        if (seg->r.y + seg->v.y < miny) {
            miny = seg->r.y + seg->v.y;
        }
        if (seg->r.y + seg->v.y > maxy) {
            maxy = seg->r.y + seg->v.y;
        }

        seg = next_segment();
    }

    // Kicsit kibovitjuk hatarokat:
    minx -= 6.0;
    miny -= 6.0;
    maxx += 6.0;
    maxy += 6.0;

    // Beallitja kezdopontot:
    collision_grid_origin = vect2(minx, miny);
    double width = maxx - minx;
    double height = maxy - miny;

    // Kiszamolja dimenziot:
    collision_grid_width = (int)(width / collision_grid_cell_size + 1.0);
    collision_grid_height = (int)(height / collision_grid_cell_size + 1.0);
    if (collision_grid_width < 0 || collision_grid_height < 0) {
        internal_error("collision_grid_width < 0 || collision_grid_height < 0!");
    }
    if (collision_grid_width > 200 || collision_grid_height > 200) {
        internal_error("collision_grid_width > 200 || collision_grid_height > 200!");
    }

    // Lefoglalja tombot:
    unsigned grid_size = collision_grid_width * collision_grid_height;
    collision_grid = new psegment_node[grid_size];
    if (!collision_grid) {
        external_error("segments::setup_collision_grid out of memory!");
    }
    int grid_size_signed = collision_grid_width * collision_grid_height;
    for (int i = 0; i < grid_size_signed; i++) {
        collision_grid[i] = NULL;
    }

    // Most vegigmegy szakaszokon es beilleszti oket:
    iterate_all_segments();
    seg = next_segment();
    while (seg) {
        add_segment_to_collision_grid(seg, max_radius);
        seg = next_segment();
    }
}

void segments::iterate_collision_grid_cell_segments(vect2 r) {
    if (!collision_grid) {
        internal_error("segments::iterate_collision_grid_cell_segments !collision_grid!");
    }
    r = (r - collision_grid_origin) * (1 / collision_grid_cell_size);
    int cell_x = 0;
    if (r.x > 0) {
        cell_x = (int)(r.x);
    }
    int cell_y = 0;
    if (r.y > 0) {
        cell_y = (int)(r.y);
    }

    if (cell_x > collision_grid_width) {
        internal_error(
            "segments::iterate_collision_grid_cell_segments cell_x > collision_grid_width!");
    }
    if (cell_x == collision_grid_width) {
        cell_x = collision_grid_width - 1;
    }
    if (cell_y > collision_grid_height) {
        internal_error(
            "segments::iterate_collision_grid_cell_segments cell_y > collision_grid_height!");
    }
    if (cell_y == collision_grid_height) {
        cell_y = collision_grid_height - 1;
    }
    collision_grid_iterable = collision_grid[collision_grid_width * cell_y + cell_x];
}

segment* segments::next_collision_grid_segment(void) {
    if (!collision_grid_iterable) {
        return NULL;
    }
#ifdef DEBUG
    if (!collision_grid_iterable->seg) {
        internal_error("segments::next_collision_grid_segment !collision_grid_iterable->seg");
    }
#endif
    segment* ret = collision_grid_iterable->seg;
    collision_grid_iterable = collision_grid_iterable->next;
    return ret;
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
