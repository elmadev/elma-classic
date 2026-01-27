#include "polygon.h"
#include "editor_canvas.h"
#include "editor_dialog.h"
#include "level.h"
#include "main.h"
#include "platform_utils.h"
#include <cmath>

polygon::polygon() {
    vertex_count = 0;
    allocated_vertex_count = 10;
    is_grass = 0;

    vertices = new vect2[allocated_vertex_count];
    for (int i = 0; i < allocated_vertex_count; i++) {
        vertices[i].x = 0.0;
        vertices[i].y = 0.0;
    }
    vertices[0].x = -24.0;
    vertices[0].y = -8.0;
    vertices[1].x = 24.0;
    vertices[1].y = -8.0;
    vertices[2].x = 24.0;
    vertices[2].y = 2.0;
    vertices[3].x = -24.0;
    vertices[3].y = 2.0;
    vertex_count = 4;
}

polygon::~polygon() {
    if (vertices) {
        delete vertices;
    }
    vertices = nullptr;
}

void polygon::set_vertex(int v, double x, double y) const {
    if (v < 0 || v >= vertex_count) {
        internal_error("polygon::set_vertex v < 0 || v >= vertex_count!");
    }
    vertices[v].x = x;
    vertices[v].y = y;
}

void polygon::render_one_line(int v, bool forward, bool dotted) const {
    if (v < 0 || v >= vertex_count) {
        internal_error("polygon::render_one_line v < 0 || v >= vertex_count!");
    }
    if ((forward && v == vertex_count - 1) || (!forward && v == 0)) {
        // Wrap around the first and last vertices
        render_line(vertices[vertex_count - 1], vertices[0], dotted);
    } else {
        // Normal case
        if (forward) {
            render_line(vertices[v], vertices[v + 1], dotted);
        } else {
            render_line(vertices[v - 1], vertices[v], dotted);
        }
    }
}

void polygon::render_outline() {
    if (vertex_count < 3 || vertex_count > MAX_VERTICES) {
        internal_error("polygon::render_outline vertex_count < 3 || vertex_count > MAX_VERTICES!");
    }
    for (int i = 0; i < vertex_count; i++) {
        render_one_line(i, true, false);
    }
}

bool polygon::insert_vertex(int v) {
    if (vertex_count + 1 > MAX_VERTICES) {
        dialog("You cannot add more points to this polygon!");
        return false;
    }
    if (v >= vertex_count) {
        internal_error("polygon::insert_vertex v >= vertex_count!");
    }

    // Reallocate more memory for new vertices
    if (vertex_count >= allocated_vertex_count) {
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

    // Duplicate the vertex
    for (int i = vertex_count - 1; i >= v; i--) {
        vertices[i + 1].x = vertices[i].x;
        vertices[i + 1].y = vertices[i].y;
    }
    vertex_count++;
    return true;
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

double polygon::get_closest_vertex(double x, double y, int* v) const {
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

int polygon::count_intersections(vect2 r1, vect2 v1) const {
#ifdef DEBUG
    if (is_grass) {
        internal_error("polygon::count_intersections grass cannot intersect!");
    }
#endif
    int intersections = 0;
    for (int i = 0; i < vertex_count; i++) {
        double x1 = vertices[i].x;
        double y1 = vertices[i].y;
        double x2;
        double y2;
        if (i < vertex_count - 1) {
            x2 = vertices[i + 1].x;
            y2 = vertices[i + 1].y;
        } else {
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

bool polygon::intersection_point(vect2 r1, vect2 v1, int skip_v, vect2* intersect_point) const {
#ifdef DEBUG
    if (is_grass) {
        internal_error("polygon::intersection_point grass cannot intersect!");
    }
#endif
    // Approximate the line r1->v1 as a box
    double minx;
    if (v1.x >= 0.0) {
        minx = r1.x;
    } else {
        minx = r1.x + v1.x;
    }
    double maxx;
    if (v1.x >= 0.0) {
        maxx = r1.x + v1.x;
    } else {
        maxx = r1.x;
    }
    double miny;
    if (v1.y >= 0.0) {
        miny = r1.y;
    } else {
        miny = r1.y + v1.y;
    }
    double maxy;
    if (v1.y >= 0.0) {
        maxy = r1.y + v1.y;
    } else {
        maxy = r1.y;
    }
    // Check each line of the polygon for an intersection
    for (int i = 0; i < vertex_count; i++) {
        // Skip index skip_v
        if (i == skip_v) {
            continue;
        }
        // Skip index skip_v +- 1
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
        double x2;
        double y2;
        if (i < vertex_count - 1) {
            x2 = vertices[i + 1].x;
            y2 = vertices[i + 1].y;
        } else {
            x2 = vertices[0].x;
            y2 = vertices[0].y;
        }

        // If the box of line x1y1 x2y2 doesn't intersect with the box of r1->v1, don't even bother
        // checking for intersections
        if ((x1 < minx && x2 < minx) || (x1 > maxx && x2 > maxx) || (y1 < miny && y2 < miny) ||
            (y1 > maxy && y2 > maxy)) {
            continue;
        }

        // Do a proper intersection check
        vect2 r2(x1, y1);
        vect2 v2 = vect2(x2, y2) - r2;
        if (segments_intersect_inexact(r1, v1, r2, v2)) {
            *intersect_point = intersection(r1, v1, r2, v2);
            return true;
        }
    }
    return false;
}

// Load a polygon from a file
polygon::polygon(FILE* h, int version) {
    vertex_count = 0;
    allocated_vertex_count = 0;
    vertices = nullptr;
    is_grass = 0;

    if (version == 14) {
        if (fread(&is_grass, 1, sizeof(is_grass), h) != 4) {
            internal_error("polygon::polygon: Failed to read file!");
        }
    } else if (version != 6) {
        internal_error("polygon::polygon unknown version!");
    }

    if (fread(&vertex_count, 1, sizeof(vertex_count), h) != 4) {
        internal_error("polygon::polygon: Failed to read file!");
    }
    if (vertex_count < 3 || vertex_count > MAX_VERTICES) {
        internal_error("polygon::polygon vertex_count < 3 || vertex_count > MAX_VERTICES!");
    }
    allocated_vertex_count = vertex_count + 10;
    vertices = new vect2[allocated_vertex_count];
    for (int i = 0; i < allocated_vertex_count; i++) {
        vertices[i].x = 0.0;
        vertices[i].y = 0.0;
    }
    if (fread(vertices, 1, sizeof(vect2) * vertex_count, h) != sizeof(vect2) * vertex_count) {
        internal_error("polygon::polygon: Failed to read file!");
    }
}

void polygon::save(FILE* h, level* lev) {
    if (fwrite(&is_grass, 1, sizeof(is_grass), h) != 4) {
        internal_error("polygon::polygon: Failed to write file!");
    }

    if (!is_grass) {
        // Swap orientation of polygon if necessary to correctly display as a ground or sky polygon
        bool sky = lev->is_sky(this);
        bool clockwise = is_clockwise();
        if ((sky && clockwise) || (!sky && !clockwise)) {
            for (int i = 0; i < vertex_count / 2; i++) {
                int j = vertex_count - 1 - i;
                vect2 tmpv = vertices[i];
                vertices[i] = vertices[j];
                vertices[j] = tmpv;
            }
        }
    }

    if (fwrite(&vertex_count, 1, sizeof(vertex_count), h) != 4) {
        internal_error("polygon::polygon: Failed to write file!");
    }
    if (fwrite(vertices, 1, sizeof(vect2) * vertex_count, h) != sizeof(vect2) * vertex_count) {
        internal_error("polygon::polygon: Failed to write file!");
    }
}

// Disallow angle a->b->c to be less than 0.0000002 radians.
// Move point a until the angle is greater than 0.0000002.
// Returns the angle.
static double get_and_fix_angle(vect2* a, vect2 b, vect2 c) {
    while (true) {
        // a->b angle
        double dy1 = a->y - b.y;
        double dx1 = a->x - b.x;
        if (dy1 == 0.0 && dx1 == 0) {
            // Unable to calculate angle of a line of length 0
            internal_error("get_and_fix_angle dx1 and dy1 == 0.0!");
        }
        double angle1 = atan2(dy1, dx1);
        // c->b angle
        double dy2 = c.y - b.y;
        double dx2 = c.x - b.x;
        if (dy2 == 0.0 && dx2 == 0) {
            // Unable to calculate angle of a line of length 0
            internal_error("get_and_fix_angle dx2 and dy2 == 0.0!");
        }
        double angle2 = atan2(dy2, dx2);
        // Real angle
        double angle = angle1 - angle2;
        if (angle < 0) {
            angle += 2 * PI;
        }
        if (angle > 2 * PI - 0.0000001) {
            angle -= 2 * PI;
        }
        if (angle < 0.0000002) {
            // Move a perpendicularly to the line
            constexpr double displacement = 0.0002;
            vect2 perpendicular = *a - b;
            perpendicular = rotate_90deg(perpendicular);
            perpendicular.normalize();
            *a = *a + perpendicular * displacement;
            continue;
        }
        return angle;
    }
}

// If two points are too close, randomly move the first
// point until minimum separation is achieved This is
// because lines of length 0 result in undefined angles.
static void separate_two_stacked_vertices(vect2* a, vect2* b) {
    constexpr double displacement = 0.0002;
    if (fabs(a->x - b->x) < 0.0000002 && fabs(a->y - b->y) < 0.0000002) {
        a->x += displacement + displacement * random_range(1000) / 1200.0;
        a->y += displacement + displacement * random_range(1000) / 1200.0;
    }
}

void polygon::separate_stacked_vertices() const {
    for (int i = 0; i < vertex_count; i++) {
        if (i < vertex_count - 1) {
            separate_two_stacked_vertices(&vertices[i], &vertices[i + 1]);
        } else {
            separate_two_stacked_vertices(&vertices[i], &vertices[0]);
        }
    }
}

// The function would seem like it actually checks if a
// polygon is counter-clockwise, but the y-axis is inverted
// (negative y is up).
bool polygon::is_clockwise() const {
#ifdef DEBUG
    if (is_grass) {
        internal_error("polygon::is_clockwise grass has no orientation!");
    }
#endif
    // Calculate the sum of all angles (fixing too narrow angles at the same time)
    double sum = 0.0;
    for (int i = 0; i < vertex_count - 2; i++) {
        sum += get_and_fix_angle(&vertices[i], vertices[i + 1], vertices[i + 2]);
    }
    sum += get_and_fix_angle(&vertices[vertex_count - 2], vertices[vertex_count - 1], vertices[0]);
    sum += get_and_fix_angle(&vertices[vertex_count - 1], vertices[0], vertices[1]);
    // Check the orientation of the polygon
    double normalized_angle = sum - vertex_count * PI;
    if (fabs(fabs(normalized_angle) - 2 * PI) > 0.1) {
        // Angle !~= 2*Pi
        // Probably a self-intersecting polygon, return false for fun
        return false;
    }

    // Angle ~= 2*Pi (counter-clockwise) returns false
    // Angle ~= -2*Pi (clockwise) returns true
    return normalized_angle < 0.0;
}

vect2 polygon::center() const {
    vect2 centerpoint(0.0, 0.0);
    for (int i = 0; i < vertex_count; i++) {
        centerpoint = centerpoint + vertices[i];
    }
    centerpoint = centerpoint * (1.0 / vertex_count);
    return centerpoint;
}

void polygon::update_boundaries(double* x1, double* y1, double* x2, double* y2) const {
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

double polygon::checksum() const {
    double sum = 0;
    for (int i = 0; i < vertex_count; i++) {
        sum += vertices[i].x;
        sum += vertices[i].y;
    }
    return sum;
}
