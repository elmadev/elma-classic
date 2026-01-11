#include "object.h"
#include "editor_canvas.h"
#include "main.h"
#include "platform_utils.h"
#include <cmath>

// KEREK KEREK KEREK KEREK KEREK KEREK KEREK KEREK KEREK2
// KEREK KEREK KEREK KEREK KEREK KEREK KEREK KEREK KEREK
// KEREK KEREK KEREK KEREK KEREK KEREK KEREK KEREK KEREK
// KEREK KEREK KEREK KEREK KEREK KEREK KEREK KEREK KEREK

kerek::kerek(double x, double y, int tipusp) {
    r.x = x;
    r.y = y;
    tipus = tipusp;
    kajatipus = 0; // Ra kell movolni hogy kaja lehessen
    foodsorszam = 0;
}

constexpr double Hal_eltol = -0.15 + 0.3 * (0.07 / 0.37);

void kerek::kirajzol(void) {
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
    if (tipus == T_CEL) {
        render_line(r + vect2(-0.15, 0.3), r + vect2(-0.15, -0.3), false);
        render_line(r + vect2(-0.15, 0.3), r + vect2(0.15, 0.3), false);
        render_line(r + vect2(-0.15, -0.3), r + vect2(0.15, -0.3), false);
        render_line(r + vect2(-0.15, 0.0), r + vect2(0.1, 0.0), false);
        return;
    }
    if (tipus == T_KAJA) {
        render_line(r + vect2(-0.15, 0.3), r + vect2(-0.15, -0.3), false);
        render_line(r + vect2(-0.15, -0.3), r + vect2(0.15, -0.3), false);
        render_line(r + vect2(-0.15, 0.0), r + vect2(0.1, 0.0), false);
        return;
    }
    if (tipus == T_KEZDO) {
        render_line(r + vect2(0.15, 0.3), r + vect2(0.15, 0.0), false);
        render_line(r + vect2(-0.15, -0.3), r + vect2(-0.15, 0.0), false);

        render_line(r + vect2(-0.15, 0.3), r + vect2(0.15, 0.3), false);
        render_line(r + vect2(-0.15, -0.3), r + vect2(0.15, -0.3), false);
        render_line(r + vect2(-0.15, 0.0), r + vect2(0.15, 0.0), false);
        return;
    }
    if (tipus == T_HALALOS) {
        render_line(r + vect2(-0.15, 0.3), r + vect2(-0.15, -0.3), false);
        render_line(r + vect2(-0.15, 0.07), r + vect2(0.15, -0.3), false);
        render_line(r + vect2(Hal_eltol, 0.0), r + vect2(0.15, 0.3), false);
        return;
    }
    internal_error("iugiffiif");
}

kerek::kerek(FILE* h, int verzio) {
    if (fread(&r.x, 1, sizeof(r.x), h) != sizeof(r.x)) {
        internal_error("Nem olvas file-bol kerek::kerek-ben!");
    }
    if (fread(&r.y, 1, sizeof(r.y), h) != sizeof(r.y)) {
        internal_error("Nem olvas file-bol kerek::kerek-ben!");
    }
    if (fread(&tipus, 1, sizeof(tipus), h) != sizeof(tipus)) {
        internal_error("Nem olvas file-bol kerek::kerek-ben!");
    }

    kajatipus = 0;
    if (verzio >= 9) {
        if (fread(&kajatipus, 1, 4, h) != 4) {
            internal_error("Nem olvas file-bol kerek::kerek-ben!");
        }
    }
    foodsorszam = 0;
    if (verzio >= 11) {
        if (fread(&foodsorszam, 1, 4, h) != 4) {
            internal_error("Nem olvas file-bol kerek::kerek-ben!");
        }
    }
    if (foodsorszam < 0 || foodsorszam > 8) {
        internal_error("65767yr");
    }
}

void kerek::save(FILE* h) {
    if (fwrite(&r.x, 1, sizeof(r.x), h) != sizeof(r.x)) {
        internal_error("Nem ir file-ba kerek::save-ben!");
    }
    if (fwrite(&r.y, 1, sizeof(r.y), h) != sizeof(r.y)) {
        internal_error("Nem ir file-ba kerek::save-ben!");
    }
    if (fwrite(&tipus, 1, sizeof(tipus), h) != sizeof(tipus)) {
        internal_error("Nem ir file-ba kerek::save-ben!");
    }
    if (fwrite(&kajatipus, 1, 4, h) != 4) {
        internal_error("Nem ir file-ba kerek::save-ben!");
    }
    if (fwrite(&foodsorszam, 1, 4, h) != 4) {
        internal_error("Nem ir file-ba kerek::save-ben!");
    }
}

double kerek::belyegszamitas(void) {
    double belyeg = 0;
    belyeg += r.x;
    belyeg += r.y;
    belyeg += tipus;
    return belyeg;
}
