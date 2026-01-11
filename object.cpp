#include "object.h"
#include "editor_canvas.h"
#include "main.h"
#include "platform_utils.h"
#include <cmath>

object::object(double x, double y, Type typ) {
    r.x = x;
    r.y = y;
    type = typ;
    property = Property::None;
    animation = 0;
}

void object::render() {
    // Draw a circle approximated by 16 slices
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

    // Draw a small X in the middle
    double length = 0.02;
    render_line(r + vect2(-length, -length), r + vect2(length, length), false);
    render_line(r + vect2(length, -length), r + vect2(-length, length), false);

    // Draw the letter
    if (type == Type::Exit) {
        // E
        render_line(r + vect2(-0.15, 0.3), r + vect2(-0.15, -0.3), false);
        render_line(r + vect2(-0.15, 0.3), r + vect2(0.15, 0.3), false);
        render_line(r + vect2(-0.15, -0.3), r + vect2(0.15, -0.3), false);
        render_line(r + vect2(-0.15, 0.0), r + vect2(0.1, 0.0), false);
        return;
    }
    if (type == Type::Food) {
        // A
        render_line(r + vect2(-0.15, 0.3), r + vect2(-0.15, -0.3), false);
        render_line(r + vect2(-0.15, -0.3), r + vect2(0.15, -0.3), false);
        render_line(r + vect2(-0.15, 0.0), r + vect2(0.1, 0.0), false);
        return;
    }
    if (type == Type::Start) {
        // S
        render_line(r + vect2(0.15, 0.3), r + vect2(0.15, 0.0), false);
        render_line(r + vect2(-0.15, -0.3), r + vect2(-0.15, 0.0), false);
        render_line(r + vect2(-0.15, 0.3), r + vect2(0.15, 0.3), false);
        render_line(r + vect2(-0.15, -0.3), r + vect2(0.15, -0.3), false);
        render_line(r + vect2(-0.15, 0.0), r + vect2(0.15, 0.0), false);
        return;
    }
    if (type == Type::Killer) {
        // K
        render_line(r + vect2(-0.15, 0.3), r + vect2(-0.15, -0.3), false);
        render_line(r + vect2(-0.15, 0.07), r + vect2(0.15, -0.3), false);
        // Bottom-right leg:
        constexpr double LETTER_K_INTERSECTION_POINT = -0.15 + 0.3 * (0.07 / 0.37);
        render_line(r + vect2(LETTER_K_INTERSECTION_POINT, 0.0), r + vect2(0.15, 0.3), false);
        return;
    }
    internal_error("object::render illegal type");
}

object::object(FILE* h, int version) {
    if (fread(&r.x, 1, sizeof(r.x), h) != 8) {
        internal_error("Failed to read object from file!");
    }
    if (fread(&r.y, 1, sizeof(r.y), h) != 8) {
        internal_error("Failed to read object from file!");
    }
    if (fread(&type, 1, sizeof(type), h) != 4) {
        internal_error("Failed to read object from file!");
    }
    if (version == 6) {
        property = Property::None;
        animation = 0;
    } else if (version == 14) {
        if (fread(&property, 1, 4, h) != 4) {
            internal_error("Failed to read object from file!");
        }
        if (fread(&animation, 1, 4, h) != 4) {
            internal_error("Failed to read object from file!");
        }
    } else {
        internal_error("object::object unknown version!");
    }
    if (animation < 0 || animation > 8) {
        internal_error("object::object invalid animation");
    }
}

void object::save(FILE* h) {
    if (fwrite(&r.x, 1, sizeof(r.x), h) != 8) {
        internal_error("Failed to write object to file!");
    }
    if (fwrite(&r.y, 1, sizeof(r.y), h) != 8) {
        internal_error("Failed to write object to file!");
    }
    if (fwrite(&type, 1, sizeof(type), h) != 4) {
        internal_error("Failed to write object to file!");
    }
    if (fwrite(&property, 1, 4, h) != 4) {
        internal_error("Failed to write object to file!");
    }
    if (fwrite(&animation, 1, 4, h) != 4) {
        internal_error("Failed to write object to file!");
    }
}

double object::checksum() {
    double sum = 0;
    sum += r.x;
    sum += r.y;
    sum += (int)type;
    return sum;
}
