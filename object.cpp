#include "object.h"
#include "editor_canvas.h"
#include "main.h"
#include "platform_utils.h"
#include <cmath>

// KEREK KEREK KEREK KEREK KEREK KEREK KEREK KEREK KEREK2
// KEREK KEREK KEREK KEREK KEREK KEREK KEREK KEREK KEREK
// KEREK KEREK KEREK KEREK KEREK KEREK KEREK KEREK KEREK
// KEREK KEREK KEREK KEREK KEREK KEREK KEREK KEREK KEREK

object::object(double x, double y, int typ) {
    r.x = x;
    r.y = y;
    type = typ;
    property = 0; // Ra kell movolni hogy kaja lehessen
    animation = 0;
}

constexpr double LETTER_K_INTERSECTION_POINT = -0.15 + 0.3 * (0.07 / 0.37);

void object::render(void) {
    int slices = 16;
    double radius = 0.4;

    double slice_angle = 2.0 * PI / slices;
    for (int i = 0; i < slices; i++) {
        double angle1 = i * slice_angle;
        double angle2 = (i + 1) * slice_angle;
        vect2 r1(radius * sin(angle1), radius * cos(angle1));
        vect2 r2(radius * sin(angle2), radius * cos(angle2));
        render_line(r + r1, r + r2, false);
    }

    // Kis jel kor kozepere:
    double length = 0.02;
    render_line(r + vect2(-length, -length), r + vect2(length, length), false);
    render_line(r + vect2(length, -length), r + vect2(-length, length), false);

    // Beleirjuk betut korbe:
    if (type == T_CEL) {
        render_line(r + vect2(-0.15, 0.3), r + vect2(-0.15, -0.3), false);
        render_line(r + vect2(-0.15, 0.3), r + vect2(0.15, 0.3), false);
        render_line(r + vect2(-0.15, -0.3), r + vect2(0.15, -0.3), false);
        render_line(r + vect2(-0.15, 0.0), r + vect2(0.1, 0.0), false);
        return;
    }
    if (type == T_KAJA) {
        render_line(r + vect2(-0.15, 0.3), r + vect2(-0.15, -0.3), false);
        render_line(r + vect2(-0.15, -0.3), r + vect2(0.15, -0.3), false);
        render_line(r + vect2(-0.15, 0.0), r + vect2(0.1, 0.0), false);
        return;
    }
    if (type == T_KEZDO) {
        render_line(r + vect2(0.15, 0.3), r + vect2(0.15, 0.0), false);
        render_line(r + vect2(-0.15, -0.3), r + vect2(-0.15, 0.0), false);

        render_line(r + vect2(-0.15, 0.3), r + vect2(0.15, 0.3), false);
        render_line(r + vect2(-0.15, -0.3), r + vect2(0.15, -0.3), false);
        render_line(r + vect2(-0.15, 0.0), r + vect2(0.15, 0.0), false);
        return;
    }
    if (type == T_HALALOS) {
        render_line(r + vect2(-0.15, 0.3), r + vect2(-0.15, -0.3), false);
        render_line(r + vect2(-0.15, 0.07), r + vect2(0.15, -0.3), false);
        render_line(r + vect2(LETTER_K_INTERSECTION_POINT, 0.0), r + vect2(0.15, 0.3), false);
        return;
    }
    internal_error("object::render illegal type");
}

object::object(FILE* h, int version) {
    if (fread(&r.x, 1, sizeof(r.x), h) != sizeof(r.x)) {
        internal_error("Failed to read object from file!");
    }
    if (fread(&r.y, 1, sizeof(r.y), h) != sizeof(r.y)) {
        internal_error("Failed to read object from file!");
    }
    if (fread(&type, 1, sizeof(type), h) != sizeof(type)) {
        internal_error("Failed to read object from file!");
    }

    property = 0;
    if (version >= 9) {
        if (fread(&property, 1, 4, h) != 4) {
            internal_error("Failed to read object from file!");
        }
    }
    animation = 0;
    if (version >= 11) {
        if (fread(&animation, 1, 4, h) != 4) {
            internal_error("Failed to read object from file!");
        }
    }
    if (animation < 0 || animation > 8) {
        internal_error("object::object invalid animation");
    }
}

void object::save(FILE* h) {
    if (fwrite(&r.x, 1, sizeof(r.x), h) != sizeof(r.x)) {
        internal_error("Failed to write object to file!");
    }
    if (fwrite(&r.y, 1, sizeof(r.y), h) != sizeof(r.y)) {
        internal_error("Failed to write object to file!");
    }
    if (fwrite(&type, 1, sizeof(type), h) != sizeof(type)) {
        internal_error("Failed to write object to file!");
    }
    if (fwrite(&property, 1, 4, h) != 4) {
        internal_error("Failed to write object to file!");
    }
    if (fwrite(&animation, 1, 4, h) != 4) {
        internal_error("Failed to write object to file!");
    }
}

double object::checksum(void) {
    double sum = 0;
    sum += r.x;
    sum += r.y;
    sum += type;
    return sum;
}
