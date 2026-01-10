#include "SZAKASZ.H"
#include "main.h"
#include "polygon.h"
#include "TOPOL.H"
#include <cmath>
#include <cstring>

static int MAX_LINES = MAX_VERTICES + 130;

// lines* Pszakt = NULL;
lines* Lines = NULL;

// loadszakasz beleirja ptop->lezart-at ha topologia file-t olvas be:
// Ezt azutan afterplayext fuggveny hasznalja ki:
int Ucsoloadszakasz_lezart = 0;

lines::lines(topol* lev) {
    // Elejen ugyanaz, mint void konstruktor:
    line_list = NULL;
    line_list_allocated_length = line_list_length = 0;
    collision_grid = NULL;
    collision_grid_width = 1;
    collision_grid_height = 1;
    collision_grid_cell_size = 1.0;
    collision_grid_origin = vect2(0, 0);
    collision_grid_next = NULL;
    node_array = NULL;
    node_array_index = 0;

    line_list = new line[MAX_LINES];
    if (!line_list) {
        external_error("lines::lines out of memory!");
        return;
    }
    line_list_allocated_length = MAX_LINES;
    for (int i = 0; i < line_list_allocated_length; i++) {
        memset(&line_list[i], 0, sizeof(line));
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
            if (line_list_length >= MAX_LINES) {
                internal_error("lines::lines line_list_length >= MAX_LINES!");
            }
            vect2 r1, r2;
            if (j < poly->vertex_count - 1) {
                r1 = poly->vertices[j];
                r2 = poly->vertices[j + 1];
            } else {
                r1 = poly->vertices[j];
                r2 = poly->vertices[0];
            }
            line_list[line_list_length].r = r1;
            line_list[line_list_length].v = r2 - r1;
            line_list[line_list_length].r.y = -line_list[line_list_length].r.y; // fejjel lefele
            line_list[line_list_length].v.y = -line_list[line_list_length].v.y; // fejjel lefele
            line_list_length++;
        }
    }
}

lines::~lines(void) {
    if (line_list) {
        delete line_list;
    }
    if (collision_grid) {
        delete collision_grid;
    }
    delete_all_line_nodes();
}

// NODE-kezeles:

line_node* lines::get_new_line_node(void) {
    if (!node_array) {
        node_array = new line_node_array;
        if (!node_array) {
            external_error("lines::get_new_line_node out of memory!");
        }
        node_array->next = NULL;
        node_array_index = 0;
    }
    line_node_array* last_array = node_array;
    while (last_array->next) {
        last_array = last_array->next;
    }
    if (node_array_index == LINE_NODE_BLOCK_SIZE) {
        last_array->next = new line_node_array;
        last_array = last_array->next;
        if (!last_array) {
            external_error("lines::get_new_line_node out of memory!");
        }
        last_array->next = NULL;
        node_array_index = 0;
    }
    node_array_index++;
    return &last_array->nodes[node_array_index - 1];
}

void lines::delete_all_line_nodes(void) {
    line_node_array* current_array = node_array;
    node_array = NULL;
    while (current_array) {
        line_node_array* deleted_array = current_array;
        current_array = current_array->next;
        delete deleted_array;
    }
}

// Egy cellaba bekoti vonalat, ha meg nincs benne es rajta van tablan:
void lines::add_line_to_cell(int cell_x, int cell_y, line* ln) {
#ifdef DEBUG
    if (cell_x < 0 || cell_y < 0) {
        internal_error("lines::add_line_to_cell cell_x < 0 || cell_y < 0!");
    }
#endif
    if (cell_x >= collision_grid_width || cell_y >= collision_grid_height) {
        return;
    }
    line_node* new_node = get_new_line_node();
    new_node->next = NULL;
    new_node->ln = ln;
    line_node* cell_node = collision_grid[collision_grid_width * cell_y + cell_x];
    if (!cell_node) {
        collision_grid[collision_grid_width * cell_y + cell_x] = new_node;
        return;
    }
    while (cell_node->next) {
        cell_node = cell_node->next;
    }
    // Most pn az utolso node-ra mutat!
    cell_node->next = new_node;
}

// Egy vonalat hozzarendel osszes olyan cellahoz, amihez koze lehet:
void lines::add_line_to_collision_grid(line* ln, double max_radius) {
    // Kitolti vonal egyseg es hossz rubrikait:
    ln->length = ln->v.length();
    if (ln->length < 0.00000001) {
        internal_error("lines::add_line_to_collision_grid line too short!");
    }
    ln->unit_vector = unit_vector(ln->v);

    vect2 v = ln->v;
    vect2 r = ln->r - collision_grid_origin;

    v = v * (1 / collision_grid_cell_size);
    r = r * (1 / collision_grid_cell_size);
    max_radius *= 1.5 / collision_grid_cell_size;
    // Ezentul cellameretet lehet 1.0-nak venni!
    int invert_axis = 0;
    if (fabs(v.y) > fabs(v.x)) {
        invert_axis = 1;
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
        internal_error("lines::add_line_to_collision_grid r.x+v.x+max_radius < 0!");
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
            internal_error("lines::add_line_to_collision_grid y1 > y2!");
        }
#endif
        if (y2 < 0.0) {
            internal_error("lines::add_line_to_collision_grid y2 < 0!");
        }
        int cell_y = 0;
        if (y1 > 0) {
            cell_y = (int)(y1);
        }
        int yend = (int)(y2);
        while (cell_y <= yend) {
            if (invert_axis) {
                add_line_to_cell(cell_y, cell_x, ln);
            } else {
                add_line_to_cell(cell_x, cell_y, ln);
            }

            cell_y++;
        }

        cell_x++;
    }
}

void lines::setup_collision_grid(double max_radius) {
    // Ellenorzes:
    if (line_list_length <= 0) {
        internal_error("lines::setup_collision_grid has no lines!");
    }
    if (collision_grid) {
        internal_error("lines::setup_collision_grid already has a collision grid!");
    }

    collision_grid_cell_size = 1.0;

    // Megkeresi palya kiterjedeset:
    iterate_all_lines();
    line* cur_line = get_line();
    if (!cur_line) {
        internal_error("lines::setup no lines found!");
    }
    double minx = cur_line->r.x;
    double maxx = cur_line->r.x;
    double miny = cur_line->r.y;
    double maxy = cur_line->r.y;
    while (cur_line) {
        if (cur_line->r.x < minx) {
            minx = cur_line->r.x;
        }
        if (cur_line->r.x > maxx) {
            maxx = cur_line->r.x;
        }
        if (cur_line->r.y < miny) {
            miny = cur_line->r.y;
        }
        if (cur_line->r.y > maxy) {
            maxy = cur_line->r.y;
        }

        if (cur_line->r.x + cur_line->v.x < minx) {
            minx = cur_line->r.x + cur_line->v.x;
        }
        if (cur_line->r.x + cur_line->v.x > maxx) {
            maxx = cur_line->r.x + cur_line->v.x;
        }
        if (cur_line->r.y + cur_line->v.y < miny) {
            miny = cur_line->r.y + cur_line->v.y;
        }
        if (cur_line->r.y + cur_line->v.y > maxy) {
            maxy = cur_line->r.y + cur_line->v.y;
        }

        cur_line = get_line();
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
    unsigned grid_length = collision_grid_width * collision_grid_height;
    collision_grid = new pline_node[grid_length];
    if (!collision_grid) {
        external_error("lines::setup out of memory!");
    }
    int grid_length_signed = collision_grid_width * collision_grid_height;
    for (int i = 0; i < grid_length_signed; i++) {
        collision_grid[i] = NULL;
    }

    // Most vegigmegy szakaszokon es beilleszti oket:
    iterate_all_lines();
    cur_line = get_line();
    while (cur_line) {
        add_line_to_collision_grid(cur_line, max_radius);
        cur_line = get_line();
    }
}

void lines::select_collision_grid_cell(vect2 r) {
    if (!collision_grid) {
        internal_error("lines::select_collision_grid_cell !collision_grid!");
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
        internal_error("lines::select_collision_grid_cell cell_x > collision_grid_width!");
    }
    if (cell_x == collision_grid_width) {
        cell_x = collision_grid_width - 1;
    }
    if (cell_y > collision_grid_height) {
        internal_error("lines::select_collision_grid_cell cell_y > collision_grid_height!");
    }
    if (cell_y == collision_grid_height) {
        cell_y = collision_grid_height - 1;
    }
    collision_grid_next = collision_grid[collision_grid_width * cell_y + cell_x];
}

line* lines::get_line_from_collision_grid_cell(void) {
    if (!collision_grid_next) {
        return NULL;
    }
#ifdef DEBUG
    if (!collision_grid_next->ln) {
        internal_error("lines::get_line_from_collision_grid_cell !collision_grid_next->ln");
    }
#endif
    line* ret = collision_grid_next->ln;
    collision_grid_next = collision_grid_next->next;
    return ret;
}

// Szakaszos kereso:
void lines::iterate_all_lines(void) { line_list_current_index = 0; }

line* lines::get_line(void) {
    if (line_list_current_index >= line_list_length) {
        return NULL;
    }
    line_list_current_index++;
    return &line_list[line_list_current_index - 1];
}

line* lines::getptrszakasz(int sorszam) {
    if (sorszam >= line_list_length) {
        internal_error("lines::getptrszakasz-ban sorszam >= line_list_length!");
    }
    return &line_list[sorszam];
}
