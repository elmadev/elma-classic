#include "DOBOZ.H"
#include "affine_pic.h"
#include "KIRAJZOL.H"
#include "pic8.h"
#include "vect2.h"
#include <cmath>

// Affinitassal kapcsolatos dolgok:
bool StretchEnabled = false;
static double StretchFactor = 1.0;
static vect2 StretchCenter = Vect2i;
static vect2 StretchAxis = Vect2i;
static double StretchMetersToPixelsX = 1.0;
static double StretchMetersToPixelsY = 1.0;

void draw_affine_pic_row(unsigned char transparency, unsigned char length, unsigned char* dest,
                         unsigned char* source, long source_x, long source_y, long source_dx,
                         long source_dy) {
    short* source_x_int = (short*)(&source_x);
    source_x_int++;
    short* source_y_int = (short*)(&source_y);
    source_y_int++;
    for (short x = 0; x < length; x++) {
        unsigned short fx = *source_x_int;
        unsigned short fy = *source_y_int;
        fy = (unsigned short)((fy << 8) + fx);

        unsigned char c = source[fy];
        if (c != transparency) {
            dest[x] = c;
        }

        source_x += source_dx;
        source_y += source_dy;
    }
}

// Ez a fuggveny kilogas eseten hivodik meg, plussz parametere xhossz:
void draw_affine_pic_row(unsigned char transparency, short length, unsigned char* dest,
                         unsigned char* source, long source_x, long source_y, long source_dx,
                         long source_dy, int length2) {
    short* source_x_int = (short*)(&source_x);
    source_x_int++;
    short* source_y_int = (short*)(&source_y);
    source_y_int++;
    for (short x = 0; x < length; x++) {
        if (x >= length2) {
            return;
        }
        unsigned short fx = *source_x_int;
        unsigned short fy = *source_y_int;
        fy = (unsigned short)((fy << 8) + fx);

        unsigned char c = source[fy];
        if (c != transparency) {
            dest[x] = c;
        }

        source_x += source_dx;
        source_y += source_dy;
    }
}

void set_stretch_parameters(vect2 bike_center, vect2 bike_i, double stretch,
                            double meters_to_pixels_x, double meters_to_pixels_y) {
    StretchCenter = bike_center;
    bike_i.normalize();
    StretchAxis = bike_i;
    StretchFactor = stretch;
    StretchMetersToPixelsX = meters_to_pixels_x;
    StretchMetersToPixelsY = meters_to_pixels_y;
}

// static unsigned char Egysor[400];

void draw_affine_pic(pic8* dest, affine_pic* aff, vect2 u, vect2 v, vect2 r) {
    unsigned char transparency = aff->transparency;

    // StretchEnabled:
    if (StretchEnabled) {
        // Kicsit atalakitja vektorokat:
        // Visszaalakitjuk 1:1-es aspect-ratio-ra r, u, v-t:
        r.x /= StretchMetersToPixelsX;
        r.y /= StretchMetersToPixelsY;
        u.x /= StretchMetersToPixelsX;
        u.y /= StretchMetersToPixelsY;
        v.x /= StretchMetersToPixelsX;
        v.y /= StretchMetersToPixelsY;

        // szamitas:
        double distance = (r - StretchCenter) * StretchAxis;
        vect2 delta = (distance * (1.0 - StretchFactor)) * StretchAxis;
        r = r - delta;

        distance = u * StretchAxis;
        delta = (distance * (1.0 - StretchFactor)) * StretchAxis;
        u = u - delta;

        distance = v * StretchAxis;
        delta = (distance * (1.0 - StretchFactor)) * StretchAxis;
        v = v - delta;

        // Visszaalakitjuk aranyx:aranyy-os aspect-ratio-ra r, u, v-t:
        r.x *= StretchMetersToPixelsX;
        r.y *= StretchMetersToPixelsY;
        u.x *= StretchMetersToPixelsX;
        u.y *= StretchMetersToPixelsY;
        v.x *= StretchMetersToPixelsX;
        v.y *= StretchMetersToPixelsY;
    }

    // Megnezi, hogy nem tul vizszintes-e:
    bool needs_rotation = false;
    bool positive_rotation_direction = 0;
    constexpr double MINIMUM_ROTATION = 0.005;
    if (u.x == 0.0) {
        needs_rotation = true;
    } else {
        if (fabs(u.y / u.x) < MINIMUM_ROTATION) {
            needs_rotation = true;
            if (u.y / u.x > 0) {
                positive_rotation_direction = true;
            }
        }
    }

    if (v.x == 0.0) {
        needs_rotation = true;
    } else {
        if (fabs(v.y / v.x) < MINIMUM_ROTATION) {
            needs_rotation = true;
            if (v.y / v.x > 0) {
                positive_rotation_direction = true;
            }
        }
    }

    if (needs_rotation) {
        if (positive_rotation_direction) {
            u.rotate(MINIMUM_ROTATION);
            v.rotate(MINIMUM_ROTATION);
        } else {
            u.rotate(-MINIMUM_ROTATION);
            v.rotate(-MINIMUM_ROTATION);
        }
    }
    // Most mar biztos, hogy egy kicsit ferde az egesz!
    // Kiveve Affin esetet, amikor azert ez lehetseges:
    if (StretchEnabled) {
        needs_rotation = true;
        while (needs_rotation) {
            needs_rotation = false;
            if (u.x == 0.0) {
                needs_rotation = true;
            }
            if (fabs(u.y / u.x) < MINIMUM_ROTATION) {
                needs_rotation = true;
            }
            if (u.y == 0.0) {
                needs_rotation = true;
            }
            if (fabs(u.x / u.y) < MINIMUM_ROTATION) {
                needs_rotation = true;
            }
            if (v.x == 0.0) {
                needs_rotation = true;
            }
            if (fabs(v.y / v.x) < MINIMUM_ROTATION) {
                needs_rotation = true;
            }
            if (v.y == 0.0) {
                needs_rotation = true;
            }
            if (fabs(v.x / v.y) < MINIMUM_ROTATION) {
                needs_rotation = true;
            }
            if (needs_rotation) {
                u.rotate(MINIMUM_ROTATION);
                v.rotate(MINIMUM_ROTATION);
            }
        }
    }

    // Inverz matrix kiszamitasa:
    vect2 u_pixel = u * (1.0 / (aff->width - 1));
    vect2 v_pixel = v * (1.0 / (aff->height - 1));
    double determinant_reciprocal = 1.0 / (u_pixel.x * v_pixel.y - v_pixel.x * u_pixel.y);
    vect2 inverse_i(v_pixel.y * determinant_reciprocal, -u_pixel.y * determinant_reciprocal);
    vect2 inverse_j(-v_pixel.x * determinant_reciprocal, u_pixel.x * determinant_reciprocal);

    // Most megnezzuk, hogy nem log-e ki a negy csucs kozul valamelyik
    // kepernyorol (negy csucs: r, r+u, r+v, r+u+v

    // X irany;
    double max_value = 0.0;
    double min_value = 0.0;
    if (u.x > 0) {
        if (v.x > 0) {
            // u poz, v poz:
            max_value = r.x + u.x + v.x;
            min_value = r.x;
        } else {
            // u poz, v neg:
            max_value = r.x + u.x;
            min_value = r.x + v.x;
        }
    } else {
        if (v.x > 0) {
            // u neg, v poz:
            max_value = r.x + v.x;
            min_value = r.x + u.x;
        } else {
            // u neg, v neg:
            max_value = r.x;
            min_value = r.x + u.x + v.x;
        }
    }
    bool possibly_out_of_bounds = false;
    if (max_value > Hatarx2) {
        possibly_out_of_bounds = true;
    }
    if (min_value < Hatarx1) {
        possibly_out_of_bounds = true;
    }

    // A legmagasabban fekvo pontot kell erre valasztani:
    vect2 apex;

    // Y irany;
    if (u.y > 0) {
        if (v.y > 0) {
            // u poz, v poz:
            apex = r + u + v;
            min_value = r.y;
        } else {
            // u poz, v neg:
            apex = r + u;
            min_value = r.y + v.y;
        }
    } else {
        if (v.y > 0) {
            // u neg, v poz:
            apex = r + v;
            min_value = r.y + u.y;
        } else {
            // u neg, v neg:
            apex = r;
            min_value = r.y + u.y + v.y;
        }
    }
    max_value = apex.y;
    if (max_value > Hatary2) {
        possibly_out_of_bounds = true;
    }
    if (min_value < Hatary1) {
        possibly_out_of_bounds = true;
    }

    // if( possibly_out_of_bounds )
    //	return;

    // Kivalasztjuk egesz kezdopontot:
    int x_left = (int)(apex.x);
    int y = (int)(apex.y);
    double apex_y = y;

    // Kiszamolja hatarolo egyenesek parametereit:
    long plane1_left, plane1_right;
    double plane1_slope = u.x / u.y;
    long plane2_left, plane2_right;
    double plane2_slope = v.x / v.y;
    if (StretchEnabled) {
        // u es v nem biztos, hogy merolegesek egymasra, ezert szamitas
        // bonyolultab:
        // Most u-ra meroleges eleket intezzuk el:
        // megkeressuk r magassagaban nem r-en atmeno u x koordjat:
        double comparison_x = r.x + v.x - (u.x / u.y) * v.y;
        if (r.x < comparison_x) {
            // u a ball es u+v a jobb!:
            plane1_left = (r.x + (apex_y - r.y) * plane1_slope) * 65536.0;
            plane1_right = (r.x + v.x + (apex_y - r.y - v.y) * plane1_slope) * 65536.0;
        } else {
            plane1_right = (r.x + (apex_y - r.y) * plane1_slope) * 65536.0;
            plane1_left = (r.x + v.x + (apex_y - r.y - v.y) * plane1_slope) * 65536.0;
        }
        // Most v-ra meroleges eleket intezzuk el:
        // megkeressuk r magassagaban nem r-en atmeno v x koordjat:
        comparison_x = r.x + u.x - (v.x / v.y) * u.y;
        if (r.x < comparison_x) {
            // v a ball es v+u a jobb!:
            plane2_left = (r.x + (apex_y - r.y) * plane2_slope) * 65536.0;
            plane2_right = (r.x + u.x + (apex_y - r.y - u.y) * plane2_slope) * 65536.0;
        } else {
            plane2_right = (r.x + (apex_y - r.y) * plane2_slope) * 65536.0;
            plane2_left = (r.x + u.x + (apex_y - r.y - u.y) * plane2_slope) * 65536.0;
        }
    } else {
        // u es v merolegesek egymasra, ezert szamitas gyors lehet:
        // Most u-ra meroleges eleket intezzuk el:
        if (v.x > 0) {
            // u a ball es u+v a jobb!:
            plane1_left = (r.x + (apex_y - r.y) * plane1_slope) * 65536.0;
            plane1_right = (r.x + v.x + (apex_y - r.y - v.y) * plane1_slope) * 65536.0;
        } else {
            plane1_right = (r.x + (apex_y - r.y) * plane1_slope) * 65536.0;
            plane1_left = (r.x + v.x + (apex_y - r.y - v.y) * plane1_slope) * 65536.0;
        }
        // Most v-ra meroleges eleket intezzuk el:
        if (u.x > 0) {
            // v a ball es v+u a jobb!:
            plane2_left = (r.x + (apex_y - r.y) * plane2_slope) * 65536.0;
            plane2_right = (r.x + u.x + (apex_y - r.y - u.y) * plane2_slope) * 65536.0;
        } else {
            plane2_right = (r.x + (apex_y - r.y) * plane2_slope) * 65536.0;
            plane2_left = (r.x + u.x + (apex_y - r.y - u.y) * plane2_slope) * 65536.0;
        }
    }

    long plane1_slope_fp = plane1_slope * 65536.0;
    long plane2_slope_fp = plane2_slope * 65536.0;

    // Most kiszamoljuk kis kepen futok elejet:
    vect2 diff = vect2(x_left, y) - r;
    vect2 affine_origin = vect2(0.5, 0.5) + diff.x * inverse_i + diff.y * inverse_j;
    long affine_x = affine_origin.x * 65536.0;
    long affine_y = affine_origin.y * 65536.0;

    long inverse_i_x_fp = inverse_i.x * 65536.0;
    long inverse_i_y_fp = inverse_i.y * 65536.0;
    long inverse_j_x_fp = inverse_j.x * 65536.0;
    long inverse_j_y_fp = inverse_j.y * 65536.0;

    short* plane1_left_int = (short*)(&plane1_left);
    plane1_left_int++;
    short* plane1_right_int = (short*)(&plane1_right);
    plane1_right_int++;
    short* plane2_left_int = (short*)(&plane2_left);
    plane2_left_int++;
    short* plane2_right_int = (short*)(&plane2_right);
    plane2_right_int++;

    if (possibly_out_of_bounds) {
        // Voltkilogas = 1; //Csak teszteleshez
        //  Nagy ciklus kezdetet veszi:
        while (true) {
            // Kiszamoljuk bal es jobb hatarokat ebben a sorban!
            int x1 = *plane1_left_int;
            int xtmp = *plane2_left_int;
            if (xtmp > x1) {
                x1 = xtmp;
            }
            x1++;
            int x2 = *plane1_right_int;
            xtmp = *plane2_right_int;
            if (xtmp < x2) {
                x2 = xtmp;
            }
            // x1 elso, x2 utolso hely, amit meg ki kell rakni!
            if (x1 <= x2 && y < Cysize) { // VALT
                // Most raallitjuk xbal-t x1-re:
                while (x_left > x1 && x_left > 0) { // VALT
                    x_left--;
                    affine_x -= inverse_i_x_fp;
                    affine_y -= inverse_i_y_fp;
                }
                while (x_left < x1 || x_left < 0) {
                    x_left++;
                    affine_x += inverse_i_x_fp;
                    affine_y += inverse_i_y_fp;
                }
                unsigned char* dest_target = dest->get_row(y);
                dest_target += x_left;
                int length_to_border = Cxsize - x_left;
                draw_affine_pic_row(transparency, short(x2 - x_left + 1), dest_target, aff->pixels,
                                    affine_x, affine_y, inverse_i_x_fp, inverse_i_y_fp,
                                    length_to_border);
            } else {
                if (x1 > x2 + 1) {
                    return;
                }
            }
            // Egy sorral lejjebb kell lepni!:
            y--;
            if (y < 0) { // VALT
                return;  // VALT
            }
            affine_x -= inverse_j_x_fp;
            affine_y -= inverse_j_y_fp;
            plane1_left -= plane1_slope_fp;
            plane1_right -= plane1_slope_fp;
            plane2_left -= plane2_slope_fp;
            plane2_right -= plane2_slope_fp;
        }
    } else {
        // Nagy ciklus kezdetet veszi:
        while (true) {
            // Kiszamoljuk bal es jobb hatarokat ebben a sorban!
            int x1 = *plane1_left_int;
            int xtmp = *plane2_left_int;
            if (xtmp > x1) {
                x1 = xtmp;
            }
            x1++;
            int x2 = *plane1_right_int;
            xtmp = *plane2_right_int;
            if (xtmp < x2) {
                x2 = xtmp;
            }
            // x1 elso, x2 utolso hely, amit meg ki kell rakni!
            if (x1 <= x2) {
                // Most raallitjuk xbal-t x1-re:
                while (x_left > x1) {
                    x_left--;
                    affine_x -= inverse_i_x_fp;
                    affine_y -= inverse_i_y_fp;
                }
                while (x_left < x1) {
                    x_left++;
                    affine_x += inverse_i_x_fp;
                    affine_y += inverse_i_y_fp;
                }
                unsigned char* dest_target = dest->get_row(y);
                dest_target += x_left;
                draw_affine_pic_row(transparency, short(x2 - x1 + 1), dest_target, aff->pixels,
                                    affine_x, affine_y, inverse_i_x_fp, inverse_i_y_fp);
            } else {
                if (x1 > x2 + 1) {
                    return;
                }
            }
            // Egy sorral lejjebb kell lepni!:
            y--;
            affine_x -= inverse_j_x_fp;
            affine_y -= inverse_j_y_fp;
            plane1_left -= plane1_slope_fp;
            plane1_right -= plane1_slope_fp;
            plane2_left -= plane2_slope_fp;
            plane2_right -= plane2_slope_fp;
        }
    }
}
