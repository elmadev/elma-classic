#include "EDITTOOL.H"
#include "editor_canvas.h"
#include "editor_dialog.h"
#include "EDITTOLT.H"
#include "EDITUJ.H"
#include "platform_impl.h"
#include "polygon.h"
#include "level.h"
#include "lgr.h"
#include "M_PIC.H"
#include "main.h"
#include "menu_pic.h"
#include "pic8.h"
#include "sprite.h"

// int Kurzornegyzet = 0;

void alaphelp(void) {
    switch (Tool) {
    case T_MOVE:
        toolhelp("Move the cursor near a vertex or an object center you want to move, and click "
                 "left button.");
        break;
    case T_ZOOMIN:
        toolhelp("Click the left button to place the first corner of zoom window.");
        break;
    case T_CREATE_VERT:
        toolhelp("If you click left button near a vertex you will add to a polygon, else create a "
                 "new polygon.");
        break;
    case T_DELETE_VERT:
        toolhelp("Click the left button near the vertex you want to delete.");
        break;
    case T_DELETE_POLY:
        toolhelp("Click the left button near any vertex of the polygon you want to delete.");
        break;
    case T_CREATE_FOOD:
        toolhelp("Click the left button to place a new Food object.");
        break;
    case T_CREATE_KILLER:
        toolhelp("Click the left button to place a new Killer object.");
        break;
    case T_DELETE_KEREK:
        toolhelp("Click the left button near the center of the object you want to delete.");
        break;
    case T_CREATE_SPRITE:
        toolhelp("Click the left button to place a new Picture. Right button chooses picture.");
        break;
    case T_DELETE_SPRITE:
        toolhelp("Click the left button near the left-up corner of the picture you want to delete. "
                 "Right chooses.");
        break;
    default:
        toolhelp("Ehhez a tool-hoz meg nincs alaphelp!");
    }
}

// MOVE MOVE MOVE MOVE MOVE MOVE MOVE MOVE MOVE MOVE MOVE MOVE
// MOVE MOVE MOVE MOVE MOVE MOVE MOVE MOVE MOVE MOVE MOVE MOVE
// MOVE MOVE MOVE MOVE MOVE MOVE MOVE MOVE MOVE MOVE MOVE MOVE
// MOVE MOVE MOVE MOVE MOVE MOVE MOVE MOVE MOVE MOVE MOVE MOVE

static double Moveorigx = 0, Moveorigy = 0;

void t_move_nyomva(int mx, int my) {
    if (Pgy || Pker || Psp) {
        if ((Pgy && Pker) || (Pgy && Psp) || (Pker && Psp)) {
            internal_error("t_move_nyomva-ban (Pgy && Pker) || (Pgy && Psp) || (Pker && Psp)!");
        }
        // Most fogjuk elengedni pontot:
        if (Pgy) {
            if (Pker) {
                internal_error("iuofiuceyhbuc");
            }
            Pgy->vertices[K] = pixel_to_meter(mx, my);
            Pgy = NULL;
        } else {
            if (Pker) {
                Pker->r = pixel_to_meter(mx, my);
                Pker = NULL;
            } else {
                Psp->r = pixel_to_meter(mx, my);
                Psp = NULL;
            }
        }
        Valtozott = 1;
        alaphelp();
        invalidate();
    } else {
        // Most probalunk megfogni egy pontot:
        double x = pixel_to_meter_x(mx);
        double y = pixel_to_meter_y(my);

        double tavpont, tavkerek, tavsprite;
        Pgy = Ptop->get_closest_vertex(x, y, &K, &tavpont);
        Pker = Ptop->get_closest_object(x, y, &tavkerek);
        Psp = Ptop->get_closest_sprite(x, y, &tavsprite);
        if (!Pgy && !Pker && !Psp) {
            return;
        }
        // Kiszurjuk hogy maximum egy maradjon nem NULL:
        if (Pgy && Pker) {
            if (tavpont < tavkerek) {
                Pker = NULL;
            } else {
                Pgy = NULL;
            }
        }
        // Most mar csak egyik nem NULL Pgy es Pker kozul:
        if (Pgy && Psp) {
            if (tavpont < tavsprite) {
                Psp = NULL;
            } else {
                Pgy = NULL;
            }
        }
        if (Pker && Psp) {
            if (tavkerek < tavsprite) {
                Psp = NULL;
            } else {
                Pker = NULL;
            }
        }
        // Kis ellenorzes:
        if ((Pgy && Pker) || (Pgy && Psp) || (Pker && Psp)) {
            internal_error("t_move_nyomva-ban legtav utan:",
                           "(Pgy && Pker) || (Pgy && Psp) || (Pker && Psp)!");
        }
        // Talaltunk egy pontot:
        if (Pgy) {
            toolhelp("Move the vertex and press the left button to place it. Press ESC or right "
                     "button to cancel.");
            Moveorigx = Pgy->vertices[K].x;
            Moveorigy = Pgy->vertices[K].y;
            mx = meter_to_pixel_x(Moveorigx);
            my = meter_to_pixel_y(Moveorigy);
        } else {
            if (Pker) {
                toolhelp("Move the object and press the left button to place it. Press ESC or "
                         "right button to cancel.");
                Moveorigx = Pker->r.x;
                Moveorigy = Pker->r.y;
                mx = meter_to_pixel_x(Moveorigx);
                my = meter_to_pixel_y(Moveorigy);
            } else {
                toolhelp("Move the picture and press the left button to place it. Press ESC or "
                         "right button to cancel.");
                Moveorigx = Psp->r.x;
                Moveorigy = Psp->r.y;
                mx = meter_to_pixel_x(Moveorigx);
                my = meter_to_pixel_y(Moveorigy);
            }
        }
        set_mouse_position(mx, my);
        push();
        Moux = mx;
        Mouy = my;
        pop();
    }
}

// Gany, mar megfogott ponttal raugrik kozeli pontra:
void gany_move_jobb(int mx, int my) {
    if (!Pgy) {
        internal_error("uirw");
    }
    // Meg van fogva egy pont, melyik van ehhez legkozelebb:
    double tav;
    double x = pixel_to_meter_x(mx);
    double y = pixel_to_meter_y(my);
    int ujk = 0;
    polygon* pujgy = Ptop->get_closest_vertex(x, y, &ujk, &tav, Pgy);
    if (!pujgy) {
        return;
    }
    Pgy->vertices[K] = pujgy->vertices[ujk];
    Pgy = NULL;
    invalidate();
}

void t_move_nyomva_jobb(int mx, int my) {
    /*if( Pgy ) {
        gany_move_jobb( mx, my );
        return;
    } */
    if (Pgy || Pker || Psp) {
        t_move_esc();
        return;
    }
    // Nincs megfogva semmi. Ilyenkor attributumot lehet allitani:
    double tavsp = 0.0, tavgy = 0.0, tavker = 0.0;
    double x = pixel_to_meter_x(mx);
    double y = pixel_to_meter_y(my);
    sprite* psp = Ptop->get_closest_sprite(x, y, &tavsp);
    int semmi = 0;
    polygon* pgy = Ptop->get_closest_vertex(x, y, &semmi, &tavgy);
    object* pk = Ptop->get_closest_object(x, y, &tavker);

    if (!psp && !pgy && !pk) {
        return;
    }

    // Csak egy mutatot hagyunk meg nem NULL-nak:
    if (psp && pgy) {
        if (tavsp > tavgy) {
            psp = NULL;
        } else {
            pgy = NULL;
        }
    }

    if (psp && pk) {
        if (tavsp > tavker) {
            psp = NULL;
        } else {
            pk = NULL;
        }
    }

    if (pgy && pk) {
        if (tavgy > tavker) {
            pgy = NULL;
        } else {
            pk = NULL;
        }
    }

    if ((pgy && psp) || (pgy && pk) || (pk && psp)) {
        internal_error("ycggyuegcfuye54354");
    }

    if (psp) {
        setspritetav(psp);
    }
    if (pgy) {
        set_gyuru_attributes(pgy);
    }
    // Csak kaja-nak lehet megvaltoztatni tipusat:
    if (pk && pk->type == object::Type::Food) {
        set_kerek_tipus("Select food subtype!", &pk->property, &pk->animation);
    }
}

void t_move_esc(void) {
    if (Pgy) {
        Pgy->vertices[K].x = Moveorigx;
        Pgy->vertices[K].y = Moveorigy;
        Pgy = NULL;
        invalidate();
    }
    if (Pker) {
        Pker->r.x = Moveorigx;
        Pker->r.y = Moveorigy;
        Pker = NULL;
        invalidate();
    }
    if (Psp) {
        Psp->r.x = Moveorigx;
        Psp->r.y = Moveorigy;
        Psp = NULL;
        invalidate();
    }
}

/*void t_move_space( void ) {
    Kurzornegyzet = !Kurzornegyzet;
    invalidate();
} */

void t_move_mmove(int mx, int my) {
    // Ellenorzes:
    if (!Pgy && !Pker && !Psp) {
        internal_error("t_move_mmove-ban !Pgy && !Pker!");
    }
    if ((Pgy && Pker) || (Pgy && Psp) || (Pker && Psp)) {
        internal_error("t_move_mmove-ban:", "(Pgy && Pker) || (Pgy && Psp) || (Pker && Psp)!");
    }

    push();

    lockfrontbuffer_pic();
    if (Pgy) {
        // Letoroljuk oket:
        Pgy->render_one_line(K, Fel, false);
        Pgy->render_one_line(K, !Fel, false);
        // Uj koord:
        double dx = pixel_to_meter_x(mx);
        double dy = pixel_to_meter_y(my);
        Pgy->set_vertex(K, dx, dy);
        // Visszarajzoljuk oket:
        Pgy->render_one_line(K, Fel, false);
        Pgy->render_one_line(K, !Fel, false);
    } else {
        if (Pker) {
            Pker->render();
            double dx = pixel_to_meter_x(mx);
            double dy = pixel_to_meter_y(my);
            Pker->r.x = dx;
            Pker->r.y = dy;
            Pker->render();
        } else {
            Psp->render();
            double dx = pixel_to_meter_x(mx);
            double dy = pixel_to_meter_y(my);
            Psp->r.x = dx;
            Psp->r.y = dy;
            Psp->render();
        }
    }
    unlockfrontbuffer_pic();

    Moux = mx;
    Mouy = my;

    pop();
}

// CREATE_VERTEX CREATE_VERTEX CREATE_VERTEX CREATE_VERTEX
// CREATE_VERTEX CREATE_VERTEX CREATE_VERTEX CREATE_VERTEX
// CREATE_VERTEX CREATE_VERTEX CREATE_VERTEX CREATE_VERTEX
// CREATE_VERTEX CREATE_VERTEX CREATE_VERTEX CREATE_VERTEX

int Egypont = 0;
static int Epontx = 0, Eponty = 0;
static int Kpontx = 0, Kponty = 0;

// Bal gomb nyomva:
void t_create_vert_nyomva(int mx, int my) {
    Valtozott = 1;
    if (!Pgy && !Egypont) {
        // Most probalunk megfogni egy pontot:
        double x = pixel_to_meter_x(mx);
        double y = pixel_to_meter_y(my);

        Pgy = Ptop->get_closest_vertex(x, y, &K);
        if (!Pgy) {
            // Egy uj poligon elso pontjat rakjuk le:
            // Eloszor megnezzuk, hogy van-e meg hely uj poligonnak:
            int gyuruszam = 0;
            for (int i = 0; i < MAX_POLYGONS; i++) {
                if (Ptop->polygons[i]) {
                    gyuruszam++;
                }
            }
            if (gyuruszam >= MAX_POLYGONS) {
                dialog("You cannot create a new polygon",
                       "because you have already reached the maximum number of polygons (300)!");
                return;
            }

            toolhelp(
                "Click the left button to place the second point. ESC or right button cancels.");
            Egypont = 1;
            Epontx = Kpontx = mx;
            Eponty = Kponty = my;
            return;
        } else {
            // Talaltunk egy mar meglevo poligonon egy pontot:
            toolhelp("Click the left button to place point. SPACE and ENTER swaps. ESC or right "
                     "button cancels.");
            push();
            Moux = meter_to_pixel_x(Pgy->vertices[K].x);
            Mouy = meter_to_pixel_y(Pgy->vertices[K].y);
            pop();
        }
    }

    if (Pgy) {
        // Most fogunk letenni uj pontot:
        if (Pgy->vertex_count >= MAX_VERTICES) {
            char tmp[100];
            sprintf(tmp, "already reached the maximum number of vertices in this level! (%d)",
                    MAX_VERTICES);
            dialog("You cannot create a new vertex, because you have", tmp);
            return;
        }
        toolhelp("Click the left button to place point. SPACE and ENTER swaps. ESC or right button "
                 "cancels.");
        Pgy->insert_vertex(K);
        invalidate();
        if (Fel) {
            K++;
        }
    } else {
        // Eddig egy pontja volt meg poligonnak, most mar valodi poligon-t
        // csinalunk belole:
        toolhelp("Click the left button to place point. SPACE and ENTER swaps. ESC or right button "
                 "cancels.");
        if (!Egypont) {
            internal_error("!Egypont, pedig annak kene lennie (ffdsgiu)!");
        }
        for (int i = 0; i < MAX_POLYGONS; i++) {
            if (!Ptop->polygons[i]) {
                Pgy = Ptop->polygons[i] = new polygon;
                Pgy->vertex_count = 3;
                Pgy->vertices[0] = pixel_to_meter(Epontx, Eponty);
                Pgy->vertices[1] = pixel_to_meter(mx, my);
                Pgy->vertices[2] = pixel_to_meter(mx, my);
                K = 2;
                Fel = true;
                Egypont = 0;
                invalidate();
                return;
            }
        };
        internal_error("Tul sok polygon (iycriyfd)!");
    }
}

// Jobb gomb nyomva:
// Ha nem fogtunk pontot, akkor uj poligonnak veszi fel elso pontjat,
// Ha fogtuk pontot, akkor torli es elengedi:
void t_create_vert_esc(void) {
    if (!Pgy && !Egypont) {
        return; // Nincsen semmi dolgunk
    }

    alaphelp();
    if (Pgy) {
        // Pgy igaz, tehat egy pont meg van fogva, amit torlunk:
        if (Pgy->vertex_count <= 3) {
            // Egesz poligont toroljuk:
            // Eloszor egy kis ellenorzes:
            int gyuruszam = 0;
            for (int i = 0; i < MAX_POLYGONS; i++) {
                if (Ptop->polygons[i]) {
                    gyuruszam++;
                }
            }
            if (gyuruszam <= 1) {
                internal_error("t_create_vert_nyomva_esc-ban gyuruszam <= 1 (7pq4)!");
            }

            int talalt = 0;
            for (int i = 0; i < MAX_POLYGONS; i++) {
                if (Ptop->polygons[i] == Pgy) {
                    // Megtalaltuk:
                    talalt = 1;
                    delete Pgy;
                    Pgy = NULL;
                    Ptop->polygons[i] = NULL;
                    K = 0;
                    invalidate();
                    break;
                }
            }
            if (!talalt) {
                internal_error("Gyuru torlesekor !talalt!");
            }
        } else {
            // Csak a pontot toroljuk:
            Pgy->delete_vertex(K);
            Pgy = NULL;
            K = 0;
            invalidate();
        }
    } else {
        if (!Egypont) {
            internal_error("!Egypont (jygfeuye)!");
        }
        Egypont = 0;
        invalidate();
    }
}

void t_create_vert_enter(void) {
    Fel = !Fel;
    if (Pgy && !Egypont) {
        invalidate();
    }
}

void t_create_vert_space(void) {
    if (!Pgy || Egypont) {
        return;
    }
    int kkov = K;
    if (Fel) {
        // Lejjebb maszunk egy pontot:
        kkov--;
        if (kkov < 0) {
            kkov = Pgy->vertex_count - 1;
        }
    } else {
        // Feljebb maszunk egy pontot:
        kkov++;
        if (kkov >= Pgy->vertex_count) {
            kkov = 0;
        }
    }
    Pgy->vertices[K] = Pgy->vertices[kkov];
    K = kkov;
    Moux = meter_to_pixel_x(Pgy->vertices[K].x);
    Mouy = meter_to_pixel_y(Pgy->vertices[K].y);
    set_mouse_position(Moux, Mouy);
    Fel = !Fel;
    invalidate();
}

void t_create_vert_mmove(int mx, int my) {
    if (!Pgy && !Egypont) {
        // Teljesen normalis kurzor mozgatas:
        internal_error("Ezt most edituj.cpp-nek kellene csinalnia!");
    }
    push();
    lockfrontbuffer_pic();
    if (Pgy) {
        // Letoroljuk oket:
        Pgy->render_one_line(K, Fel, true); // szaggatott
        Pgy->render_one_line(K, !Fel, false);
        // Uj koord:
        double dx = pixel_to_meter_x(mx);
        double dy = pixel_to_meter_y(my);
        Pgy->set_vertex(K, dx, dy);
        // Visszarajzoljuk oket:
        Pgy->render_one_line(K, Fel, true); // szaggatott
        Pgy->render_one_line(K, !Fel, false);
        Moux = mx;
        Mouy = my;
    } else {
        if (!Egypont) {
            internal_error("!Egypont (yifeye)");
        }
        render_line(pixel_to_meter(Epontx, Eponty), pixel_to_meter(Kpontx, Kponty), false);
        Moux = Kpontx = mx;
        Mouy = Kponty = my;
        render_line(pixel_to_meter(Epontx, Eponty), pixel_to_meter(Kpontx, Kponty), false);
    }
    unlockfrontbuffer_pic();
    pop();
}

// DELETE DELETE DELETE DELETE DELETE DELETE DELETE DELETE
// DELETE DELETE DELETE DELETE DELETE DELETE DELETE DELETE
// DELETE DELETE DELETE DELETE DELETE DELETE DELETE DELETE
// DELETE DELETE DELETE DELETE DELETE DELETE DELETE DELETE

void t_delete_vert_nyomva(int mx, int my) {
    if (Pgy) {
        internal_error("yffwiye");
    }
    // Most probalunk megfogni egy pontot:
    double x = pixel_to_meter_x(mx);
    double y = pixel_to_meter_y(my);

    int k = 0;
    polygon* pgy = Ptop->get_closest_vertex(x, y, &k);
    if (!pgy) {
        return;
    }
    // Talaltunk egy pontot, amit le kene torolni:
    if (pgy->vertex_count < 3) {
        internal_error("yigfdsyhifeiu");
    }
    if (pgy->vertex_count == 3) {
        // dialog( "You cannot delete this polygon, because every" );
        dialog("You cannot delete this vertex, because every polygon must",
               "have at least three vertices! If you want to delete the",
               "whole polygon, please select the DEL POLYGON tool!");
        return;
    }
    pgy->delete_vertex(k);
    Valtozott = 1;
    invalidate();
}

void t_delete_poly_nyomva(int mx, int my) {
    if (Pgy) {
        internal_error("oiutrgiuot");
    }
    // Most probalunk megfogni egy pontot:
    double x = pixel_to_meter_x(mx);
    double y = pixel_to_meter_y(my);

    int k = 0;
    polygon* pgy = Ptop->get_closest_vertex(x, y, &k);
    if (!pgy) {
        return;
    }
    // Talaltunk egy poligont, amit le kene torolni:
    int szam = 0;
    for (int i = 0; i < MAX_POLYGONS; i++) {
        if (Ptop->polygons[i]) {
            szam++;
        }
    }
    if (szam == 0) {
        internal_error("oufeifwe");
    }
    if (szam == 1) {
        dialog("This is the only polygon, so you cannot delete it.");
        return;
    }
    for (int i = 0; i < MAX_POLYGONS; i++) {
        if (Ptop->polygons[i] == pgy) {
            delete Ptop->polygons[i];
            Ptop->polygons[i] = NULL;
            Valtozott = 1;
            invalidate();
            return;
        }
    }
    internal_error("fdsiyfdfei");
}

// CREATE_KEREK CREATE_KEREK CREATE_KEREK CREATE_KEREK CREATE_KEREK
// CREATE_KEREK CREATE_KEREK CREATE_KEREK CREATE_KEREK CREATE_KEREK
// CREATE_KEREK CREATE_KEREK CREATE_KEREK CREATE_KEREK CREATE_KEREK
// CREATE_KEREK CREATE_KEREK CREATE_KEREK CREATE_KEREK CREATE_KEREK

// Legkozelebbi create food ezt fogja hasznalni:
object::Property Food_kajatipus = object::Property::None;
int Food_foodsorszam = 0;

void t_create_food_nyomva_jobb(void) {
    set_kerek_tipus("Select Create_Food subtype!", &Food_kajatipus, &Food_foodsorszam);
}

// Food es killer is egyben:
void t_create_kerek_nyomva(int mx, int my, int food) {
    // Megszamoljuk eddigi objektumokat:
    int objszam = 0;
    for (int i = 0; i < MAX_OBJECTS; i++) {
        if (Ptop->objects[i]) {
            objszam++;
        }
    }
    if (objszam >= MAX_OBJECTS) {
        dialog("You have already reached the maximum number of objects (50)!");
        return;
    }

    for (int i = 0; i < MAX_OBJECTS; i++) {
        if (!Ptop->objects[i]) {
            double x = pixel_to_meter_x(mx);
            double y = pixel_to_meter_y(my);
            object::Type tipus = object::Type::Food;
            if (!food) {
                tipus = object::Type::Killer;
            }
            Ptop->objects[i] = new object(x, y, tipus);
            if (food) {
                Ptop->objects[i]->property = Food_kajatipus;
                Ptop->objects[i]->animation = Food_foodsorszam;
            }
            invalidate();
            Valtozott = 1;
            return;
        }
    }
    internal_error("yer8yryfryfr!");
}

// DELETE_KEREK DELETE_KEREK DELETE_KEREK DELETE_KEREK DELETE_KEREK
// DELETE_KEREK DELETE_KEREK DELETE_KEREK DELETE_KEREK DELETE_KEREK
// DELETE_KEREK DELETE_KEREK DELETE_KEREK DELETE_KEREK DELETE_KEREK
// DELETE_KEREK DELETE_KEREK DELETE_KEREK DELETE_KEREK DELETE_KEREK

void t_delete_kerek_nyomva(int mx, int my) {
    double x = pixel_to_meter_x(mx);
    double y = pixel_to_meter_y(my);
    object* pker = Ptop->get_closest_object(x, y);
    if (!pker) {
        return;
    }
    for (int i = 0; i < MAX_OBJECTS; i++) {
        if (Ptop->objects[i] == pker) {
            if (pker->type == object::Type::Exit) {
                dialog("You cannot delete the Exit object!");
                return;
            }
            if (pker->type == object::Type::Start) {
                dialog("You cannot delete the Start object!");
                return;
            }
            delete pker;
            Ptop->objects[i] = NULL;
            Valtozott = 1;
            invalidate();
            return;
        }
    }
    internal_error("iufewifewiyfweiyhfewiyh");
}

// ZOOM ZOOM ZOOM ZOOM ZOOM ZOOM ZOOM ZOOM ZOOM ZOOM ZOOM ZOOM
// ZOOM ZOOM ZOOM ZOOM ZOOM ZOOM ZOOM ZOOM ZOOM ZOOM ZOOM ZOOM
// ZOOM ZOOM ZOOM ZOOM ZOOM ZOOM ZOOM ZOOM ZOOM ZOOM ZOOM ZOOM
// ZOOM ZOOM ZOOM ZOOM ZOOM ZOOM ZOOM ZOOM ZOOM ZOOM ZOOM ZOOM

int Zoomfogva = 0; // Ha ez igaz, akkor nem szabad gombokat nyomkodni
static int Zoomx1 = 0, Zoomy1 = 0;
static int Zoomx2 = 0, Zoomy2 = 0;

void t_zoomin_nyomva(int x, int y) {
    if (Zoomfogva) {
        // Megvan masodik pont is:
        alaphelp();
        zoom_in(Zoomx1, Zoomy1, x, y);
        Zoomfogva = 0;
        invalidate();
    } else {
        // Most tesszuk le elso pontot:
        toolhelp("Click left button to place the second corner of zoom window. ESC or right button "
                 "cancels.");
        Zoomfogva = 1;
        Zoomx1 = Zoomx2 = x;
        Zoomy1 = Zoomy2 = y;
    }
}

void t_zoomin_esc(void) {
    if (Zoomfogva) {
        alaphelp();
        Zoomfogva = 0;
        invalidate();
    }
}

static void zoompont(int x, int y) {
    unsigned char szin = BufferMain->gpixel(x, y);
    szin += 128;
    BufferMain->ppixel(x, y, szin);
    ppixelfront(x, y, szin);
}

static void kikeret(int x1, int y1, int x2, int y2) {
    if (x2 < x1) {
        int tmp = x1;
        x1 = x2;
        x2 = tmp;
    }
    if (y2 < y1) {
        int tmp = y1;
        y1 = y2;
        y2 = tmp;
    }
    for (int x = x1; x <= x2; x++) {
        zoompont(x, y1);
        zoompont(x, y2);
    }
    for (int y = y1; y <= y2; y++) {
        zoompont(x1, y);
        zoompont(x2, y);
    }
}

void t_zoomin_mmove(int x, int y) {
    push();
    if (Zoomfogva) {
        lockfrontbuffer_pic();
        kikeret(Zoomx1, Zoomy1, Zoomx2, Zoomy2);
        Zoomx2 = x;
        Zoomy2 = y;
        Moux = x;
        Mouy = y;
        kikeret(Zoomx1, Zoomy1, Zoomx2, Zoomy2);
        unlockfrontbuffer_pic();
    } else {
        // Teljesen normalis kurzor mozgatas:
        Moux = x;
        Mouy = y;
    }
    pop();
}

// SPRITE SPRITE SPRITE SPRITE SPRITE SPRITE SPRITE SPRITE SPRITE SPRITE
// SPRITE SPRITE SPRITE SPRITE SPRITE SPRITE SPRITE SPRITE SPRITE SPRITE
// SPRITE SPRITE SPRITE SPRITE SPRITE SPRITE SPRITE SPRITE SPRITE SPRITE
// SPRITE SPRITE SPRITE SPRITE SPRITE SPRITE SPRITE SPRITE SPRITE SPRITE
// SPRITE SPRITE SPRITE SPRITE SPRITE SPRITE SPRITE SPRITE SPRITE SPRITE
// SPRITE SPRITE SPRITE SPRITE SPRITE SPRITE SPRITE SPRITE SPRITE SPRITE

// Elougrik dialogus sprite valasztasra:
void t_create_sprite_nyomva_jobb(void) {
    // Lgr-en vegzi muveleteit:
    if (!Lgr) {
        internal_error("grtyy");
    }
    if (Lgr->picture_count <= 0) {
        dialog("There are not any pictures in the LGR file!");
        return;
    }
    tolt_pickasprite();
    /*if( Lgr->aktivindex >= Lgr->picture_count )
        internal_error( "[op0fijh" );
    kispritenev( Lgr->nevek[Lgr->aktivindex],
                 Lgr->tavolsagok[Lgr->aktivindex],
                 Lgr->hatarolok[Lgr->aktivindex] );*/
}

void t_create_sprite_nyomva(int mx, int my) {
    if (!Lgr) {
        internal_error("987g45");
    }
    if (Lgr->picture_count <= 0) {
        dialog("There are not any pictures in the LGR file!");
        return;
    }

    // Megszamoljuk eddigi objektumokat:
    int spriteszam = 0;
    for (int i = 0; i < MAX_SPRITES; i++) {
        if (Ptop->sprites[i]) {
            spriteszam++;
        }
    }
    if (spriteszam >= MAX_SPRITES) {
        dialog("You have already reached the maximum number of pictures (5000)!");
        return;
    }

    if (!Lgr->editor_picture_name[0] &&
        !(Lgr->editor_texture_name[0] && Lgr->editor_mask_name[0])) {
        return; // Nincs kitoltve aktiv sprite
    }

    for (int i = 0; i < MAX_SPRITES; i++) {
        if (!Ptop->sprites[i]) {
            double x = pixel_to_meter_x(mx);
            double y = pixel_to_meter_y(my);
            Ptop->sprites[i] = new sprite(x, y, Lgr->editor_picture_name, Lgr->editor_texture_name,
                                          Lgr->editor_mask_name);
            invalidate();
            Valtozott = 1;
            return;
        }
    }
    internal_error("oihhgffc!");
}

void t_delete_sprite_nyomva(int mx, int my) {
    double x = pixel_to_meter_x(mx);
    double y = pixel_to_meter_y(my);
    sprite* psp = Ptop->get_closest_sprite(x, y);
    if (!psp) {
        return;
    }
    for (int i = 0; i < MAX_SPRITES; i++) {
        if (Ptop->sprites[i] == psp) {
            delete psp;
            Ptop->sprites[i] = NULL;
            Valtozott = 1;
            invalidate();
            return;
        }
    }
    internal_error("uytyttyrtrtdf");
}
