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

constexpr double Hal_eltol = -0.15 + 0.3 * (0.07 / 0.37);

void object::render(void) {
    int szegmensszam = 16;
    double sugar = 0.4;

    double elfordulas = 2.0 * PI / szegmensszam;
    for (int i = 0; i < szegmensszam; i++) {
        double alfa1 = i * elfordulas;
        double alfa2 = (i + 1) * elfordulas;
        vect2 r1(sugar * sin(alfa1), sugar * cos(alfa1));
        vect2 r2(sugar * sin(alfa2), sugar * cos(alfa2));
        render_line(r + r1, r + r2, false);
    }

    // Kis jel kor kozepere:
    double meret = 0.02;
    render_line(r + vect2(-meret, -meret), r + vect2(meret, meret), false);
    render_line(r + vect2(meret, -meret), r + vect2(-meret, meret), false);

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
        render_line(r + vect2(Hal_eltol, 0.0), r + vect2(0.15, 0.3), false);
        return;
    }
    internal_error("iugiffiif");
}

object::object(FILE* h, int version) {
    if (fread(&r.x, 1, sizeof(r.x), h) != sizeof(r.x)) {
        internal_error("Nem olvas file-bol kerek::kerek-ben!");
    }
    if (fread(&r.y, 1, sizeof(r.y), h) != sizeof(r.y)) {
        internal_error("Nem olvas file-bol kerek::kerek-ben!");
    }
    if (fread(&type, 1, sizeof(type), h) != sizeof(type)) {
        internal_error("Nem olvas file-bol kerek::kerek-ben!");
    }

    property = 0;
    if (version >= 9) {
        if (fread(&property, 1, 4, h) != 4) {
            internal_error("Nem olvas file-bol kerek::kerek-ben!");
        }
    }
    animation = 0;
    if (version >= 11) {
        if (fread(&animation, 1, 4, h) != 4) {
            internal_error("Nem olvas file-bol kerek::kerek-ben!");
        }
    }
    if (animation < 0 || animation > 8) {
        internal_error("65767yr");
    }
}

void object::save(FILE* h) {
    if (fwrite(&r.x, 1, sizeof(r.x), h) != sizeof(r.x)) {
        internal_error("Nem ir file-ba kerek::save-ben!");
    }
    if (fwrite(&r.y, 1, sizeof(r.y), h) != sizeof(r.y)) {
        internal_error("Nem ir file-ba kerek::save-ben!");
    }
    if (fwrite(&type, 1, sizeof(type), h) != sizeof(type)) {
        internal_error("Nem ir file-ba kerek::save-ben!");
    }
    if (fwrite(&property, 1, 4, h) != 4) {
        internal_error("Nem ir file-ba kerek::save-ben!");
    }
    if (fwrite(&animation, 1, 4, h) != 4) {
        internal_error("Nem ir file-ba kerek::save-ben!");
    }
}

double object::checksum(void) {
    double belyeg = 0;
    belyeg += r.x;
    belyeg += r.y;
    belyeg += type;
    return belyeg;
}
