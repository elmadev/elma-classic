#include "EDITTOOL.H"
#include "editor/canvas.h"
#include "editor/dialog.h"
#include "editor/window.h"
#include "EDITUJ.H"
#include "platform/implementation.h"
#include "polygon.h"
#include "level.h"
#include "lgr.h"
#include "M_PIC.H"
#include "main.h"
#include "menu/pic.h"
#include "pic8.h"
#include "sprite.h"

// int Kurzornegyzet = 0;

void draw_tooltip_help() {
    switch (Tool) {
    case T_MOVE:
        toolhelp(
            "Move the cursor near a vertex or an object center you want to move, and left click.");
        break;
    case T_ZOOMIN:
        toolhelp("Left click to place the first corner of the zoom in window.");
        break;
    case T_CREATE_VERT:
        toolhelp("If you left click near a vertex you will add to a polygon, otherwise you will "
                 "create a new polygon.");
        break;
    case T_DELETE_VERT:
        toolhelp("Left click near the vertex you want to delete.");
        break;
    case T_DELETE_POLY:
        toolhelp("Left click near any vertex of the polygon you want to delete.");
        break;
    case T_CREATE_FOOD:
        toolhelp("Left click to place a new Food object.");
        break;
    case T_CREATE_KILLER:
        toolhelp("Left click to place a new Killer object.");
        break;
    case T_DELETE_KEREK:
        toolhelp("Left click near the center of the object you want to delete.");
        break;
    case T_CREATE_SPRITE:
        toolhelp("Left click to place a new Picture. Right click chooses the picture.");
        break;
    case T_DELETE_SPRITE:
        toolhelp("Left click near the top-left corner of the picture you want to delete.");
        break;
    default:
        toolhelp("draw_tooltip_help unknown tool!");
    }
}

// MOVE MOVE MOVE MOVE MOVE MOVE MOVE MOVE MOVE MOVE MOVE MOVE
// MOVE MOVE MOVE MOVE MOVE MOVE MOVE MOVE MOVE MOVE MOVE MOVE
// MOVE MOVE MOVE MOVE MOVE MOVE MOVE MOVE MOVE MOVE MOVE MOVE
// MOVE MOVE MOVE MOVE MOVE MOVE MOVE MOVE MOVE MOVE MOVE MOVE

static double ToolMoveOriginalX = 0, ToolMoveOriginalY = 0;

void tool_move_leftclick(int mouse_x, int mouse_y) {
    if (Pgy || Pker || Psp) {
        if ((Pgy && Pker) || (Pgy && Psp) || (Pker && Psp)) {
            internal_error("tool_move_leftclick (Pgy && Pker) || (Pgy && Psp) || (Pker && Psp)!");
        }
        // Most fogjuk elengedni pontot:
        if (Pgy) {
            if (Pker) {
                internal_error("tool_move_leftclick (Pgy && Pker)");
            }
            Pgy->vertices[K] = pixel_to_meter(mouse_x, mouse_y);
            Pgy = NULL;
        } else {
            if (Pker) {
                Pker->r = pixel_to_meter(mouse_x, mouse_y);
                Pker = NULL;
            } else {
                Psp->r = pixel_to_meter(mouse_x, mouse_y);
                Psp = NULL;
            }
        }
        Valtozott = 1;
        draw_tooltip_help();
        invalidate();
    } else {
        // Most probalunk megfogni egy pontot:
        double x = pixel_to_meter_x(mouse_x);
        double y = pixel_to_meter_y(mouse_y);

        double distance_vertex, distance_object, distance_sprite;
        Pgy = Ptop->get_closest_vertex(x, y, &K, &distance_vertex);
        Pker = Ptop->get_closest_object(x, y, &distance_object);
        Psp = Ptop->get_closest_sprite(x, y, &distance_sprite);
        if (!Pgy && !Pker && !Psp) {
            return;
        }
        // Kiszurjuk hogy maximum egy maradjon nem NULL:
        if (Pgy && Pker) {
            if (distance_vertex < distance_object) {
                Pker = NULL;
            } else {
                Pgy = NULL;
            }
        }
        // Most mar csak egyik nem NULL Pgy es Pker kozul:
        if (Pgy && Psp) {
            if (distance_vertex < distance_sprite) {
                Psp = NULL;
            } else {
                Pgy = NULL;
            }
        }
        if (Pker && Psp) {
            if (distance_object < distance_sprite) {
                Psp = NULL;
            } else {
                Pker = NULL;
            }
        }
        // Kis ellenorzes:
        if ((Pgy && Pker) || (Pgy && Psp) || (Pker && Psp)) {
            internal_error("tool_move_leftclick: (Pgy && Pker) || (Pgy && Psp) || (Pker && Psp)!");
        }
        // Talaltunk egy pontot:
        if (Pgy) {
            toolhelp(
                "Move the vertex and left click to place it. Press ESC or right click to cancel.");
            ToolMoveOriginalX = Pgy->vertices[K].x;
            ToolMoveOriginalY = Pgy->vertices[K].y;
            mouse_x = meter_to_pixel_x(ToolMoveOriginalX);
            mouse_y = meter_to_pixel_y(ToolMoveOriginalY);
        } else {
            if (Pker) {
                toolhelp("Move the object and left click to place it. Press ESC or right click to "
                         "cancel.");
                ToolMoveOriginalX = Pker->r.x;
                ToolMoveOriginalY = Pker->r.y;
                mouse_x = meter_to_pixel_x(ToolMoveOriginalX);
                mouse_y = meter_to_pixel_y(ToolMoveOriginalY);
            } else {
                toolhelp("Move the picture and left click to place it. Press ESC or right click to "
                         "cancel.");
                ToolMoveOriginalX = Psp->r.x;
                ToolMoveOriginalY = Psp->r.y;
                mouse_x = meter_to_pixel_x(ToolMoveOriginalX);
                mouse_y = meter_to_pixel_y(ToolMoveOriginalY);
            }
        }
        set_mouse_position(mouse_x, mouse_y);
        push();
        Moux = mouse_x;
        Mouy = mouse_y;
        pop();
    }
}

// Gany, mar megfogott ponttal raugrik kozeli pontra:
void gany_move_jobb(int mouse_x, int mouse_y) {
    if (!Pgy) {
        internal_error("uirw");
    }
    // Meg van fogva egy pont, melyik van ehhez legkozelebb:
    double tav;
    double x = pixel_to_meter_x(mouse_x);
    double y = pixel_to_meter_y(mouse_y);
    int ujk = 0;
    polygon* pujgy = Ptop->get_closest_vertex(x, y, &ujk, &tav, Pgy);
    if (!pujgy) {
        return;
    }
    Pgy->vertices[K] = pujgy->vertices[ujk];
    Pgy = NULL;
    invalidate();
}

void tool_move_rightclick(int mouse_x, int mouse_y) {
    /*if( Pgy ) {
        gany_move_jobb( mouse_x, mouse_y );
        return;
    } */
    if (Pgy || Pker || Psp) {
        tool_move_esc();
        return;
    }
    // Nincs megfogva semmi. Ilyenkor attributumot lehet allitani:
    double distance_sprite = 0.0, distance_vertex = 0.0, distance_object = 0.0;
    double x = pixel_to_meter_x(mouse_x);
    double y = pixel_to_meter_y(mouse_y);
    sprite* spr = Ptop->get_closest_sprite(x, y, &distance_sprite);
    int unused = 0;
    polygon* poly = Ptop->get_closest_vertex(x, y, &unused, &distance_vertex);
    object* obj = Ptop->get_closest_object(x, y, &distance_object);

    if (!spr && !poly && !obj) {
        return;
    }

    // Csak egy mutatot hagyunk meg nem NULL-nak:
    if (spr && poly) {
        if (distance_sprite > distance_vertex) {
            spr = NULL;
        } else {
            poly = NULL;
        }
    }

    if (spr && obj) {
        if (distance_sprite > distance_object) {
            spr = NULL;
        } else {
            obj = NULL;
        }
    }

    if (poly && obj) {
        if (distance_vertex > distance_object) {
            poly = NULL;
        } else {
            obj = NULL;
        }
    }

    if ((poly && spr) || (poly && obj) || (obj && spr)) {
        internal_error("tool_move_rightclick (poly && spr) || (poly && obj) || (obj && spr)");
    }

    if (spr) {
        editor_window_sprite_properties(spr);
    }
    if (poly) {
        editor_window_polygon_properties(poly);
    }
    // Csak kaja-nak lehet megvaltoztatni tipusat:
    if (obj && obj->type == object::Type::Food) {
        editor_window_food_properties("Select food subtype!", &obj->property, &obj->animation);
    }
}

void tool_move_esc() {
    if (Pgy) {
        Pgy->vertices[K].x = ToolMoveOriginalX;
        Pgy->vertices[K].y = ToolMoveOriginalY;
        Pgy = NULL;
        invalidate();
    }
    if (Pker) {
        Pker->r.x = ToolMoveOriginalX;
        Pker->r.y = ToolMoveOriginalY;
        Pker = NULL;
        invalidate();
    }
    if (Psp) {
        Psp->r.x = ToolMoveOriginalX;
        Psp->r.y = ToolMoveOriginalY;
        Psp = NULL;
        invalidate();
    }
}

/*void t_move_space( void ) {
    Kurzornegyzet = !Kurzornegyzet;
    invalidate();
} */

void tool_move_mousemove(int mouse_x, int mouse_y) {
    // Ellenorzes:
    if (!Pgy && !Pker && !Psp) {
        internal_error("tool_move_mousemove !Pgy && !Pker && !Psp");
    }
    if ((Pgy && Pker) || (Pgy && Psp) || (Pker && Psp)) {
        internal_error("tool_move_mousemove: (Pgy && Pker) || (Pgy && Psp) || (Pker && Psp)!");
    }

    push();

    lockfrontbuffer_pic();
    if (Pgy) {
        // Letoroljuk oket:
        Pgy->render_one_line(K, Fel, false);
        Pgy->render_one_line(K, !Fel, false);
        // Uj koord:
        double x = pixel_to_meter_x(mouse_x);
        double y = pixel_to_meter_y(mouse_y);
        Pgy->set_vertex(K, x, y);
        // Visszarajzoljuk oket:
        Pgy->render_one_line(K, Fel, false);
        Pgy->render_one_line(K, !Fel, false);
    } else {
        if (Pker) {
            Pker->render();
            double x = pixel_to_meter_x(mouse_x);
            double y = pixel_to_meter_y(mouse_y);
            Pker->r.x = x;
            Pker->r.y = y;
            Pker->render();
        } else {
            Psp->render();
            double x = pixel_to_meter_x(mouse_x);
            double y = pixel_to_meter_y(mouse_y);
            Psp->r.x = x;
            Psp->r.y = y;
            Psp->render();
        }
    }
    unlockfrontbuffer_pic();

    Moux = mouse_x;
    Mouy = mouse_y;

    pop();
}

// CREATE_VERTEX CREATE_VERTEX CREATE_VERTEX CREATE_VERTEX
// CREATE_VERTEX CREATE_VERTEX CREATE_VERTEX CREATE_VERTEX
// CREATE_VERTEX CREATE_VERTEX CREATE_VERTEX CREATE_VERTEX
// CREATE_VERTEX CREATE_VERTEX CREATE_VERTEX CREATE_VERTEX

int CreatingPolygon = 0;
static int FirstVertexX = 0, FirstVertexY = 0;
static int MouseVertexX = 0, MouseVertexY = 0;

// Bal gomb nyomva:
void tool_create_vertex_leftclick(int mouse_x, int mouse_y) {
    Valtozott = 1;
    if (!Pgy && !CreatingPolygon) {
        // Most probalunk megfogni egy pontot:
        double x = pixel_to_meter_x(mouse_x);
        double y = pixel_to_meter_y(mouse_y);

        Pgy = Ptop->get_closest_vertex(x, y, &K);
        if (!Pgy) {
            // Egy uj poligon elso pontjat rakjuk le:
            // Eloszor megnezzuk, hogy van-e meg hely uj poligonnak:
            int poly_count = 0;
            for (int i = 0; i < MAX_POLYGONS; i++) {
                if (Ptop->polygons[i]) {
                    poly_count++;
                }
            }
            if (poly_count >= MAX_POLYGONS) {
                char tmp[100];
                sprintf(tmp,
                        "because you have already reached the maximum number of polygons (%d)!",
                        MAX_POLYGONS);
                dialog("You cannot create a new polygon", tmp);
                return;
            }

            toolhelp("Left click to place the second vertex. ESC or right click cancels.");
            CreatingPolygon = 1;
            FirstVertexX = MouseVertexX = mouse_x;
            FirstVertexY = MouseVertexY = mouse_y;
            return;
        } else {
            // Talaltunk egy mar meglevo poligonon egy pontot:
            toolhelp("Left click to place vertex. SPACE and ENTER swaps. ESC or right "
                     "click cancels.");
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
        toolhelp("Left click to place vertex. SPACE and ENTER swaps. ESC or right click "
                 "cancels.");
        Pgy->insert_vertex(K);
        invalidate();
        if (Fel) {
            K++;
        }
    } else {
        // Eddig egy pontja volt meg poligonnak, most mar valodi poligon-t
        // csinalunk belole:
        toolhelp("Left click to place point. SPACE and ENTER swaps. ESC or right click "
                 "cancels.");
        if (!CreatingPolygon) {
            internal_error("tool_create_vertex_leftclick !Pgy !CreatingPolygon");
        }
        for (int i = 0; i < MAX_POLYGONS; i++) {
            if (!Ptop->polygons[i]) {
                Pgy = Ptop->polygons[i] = new polygon;
                Pgy->vertex_count = 3;
                Pgy->vertices[0] = pixel_to_meter(FirstVertexX, FirstVertexY);
                Pgy->vertices[1] = pixel_to_meter(mouse_x, mouse_y);
                Pgy->vertices[2] = pixel_to_meter(mouse_x, mouse_y);
                K = 2;
                Fel = true;
                CreatingPolygon = 0;
                invalidate();
                return;
            }
        };
        internal_error("tool_create_vertex_leftclick no free polygon!");
    }
}

// Jobb gomb nyomva:
// Ha nem fogtunk pontot, akkor uj poligonnak veszi fel elso pontjat,
// Ha fogtuk pontot, akkor torli es elengedi:
void tool_create_vertex_esc() {
    if (!Pgy && !CreatingPolygon) {
        return; // Nincsen semmi dolgunk
    }

    draw_tooltip_help();
    if (Pgy) {
        // Pgy igaz, tehat egy pont meg van fogva, amit torlunk:
        if (Pgy->vertex_count <= 3) {
            // Egesz poligont toroljuk:
            // Eloszor egy kis ellenorzes:
            int poly_count = 0;
            for (int i = 0; i < MAX_POLYGONS; i++) {
                if (Ptop->polygons[i]) {
                    poly_count++;
                }
            }
            if (poly_count <= 1) {
                internal_error("tool_create_vertex_esc poly_count <= 1!");
            }

            int found = 0;
            for (int i = 0; i < MAX_POLYGONS; i++) {
                if (Ptop->polygons[i] == Pgy) {
                    // Megtalaltuk:
                    found = 1;
                    delete Pgy;
                    Pgy = NULL;
                    Ptop->polygons[i] = NULL;
                    K = 0;
                    invalidate();
                    break;
                }
            }
            if (!found) {
                internal_error("tool_create_vertex_esc !found!");
            }
        } else {
            // Csak a pontot toroljuk:
            Pgy->delete_vertex(K);
            Pgy = NULL;
            K = 0;
            invalidate();
        }
    } else {
        if (!CreatingPolygon) {
            internal_error("tool_create_vertex_esc !CreatingPolygon!");
        }
        CreatingPolygon = 0;
        invalidate();
    }
}

void tool_create_vertex_enter() {
    Fel = !Fel;
    if (Pgy && !CreatingPolygon) {
        invalidate();
    }
}

void tool_create_vertex_space() {
    if (!Pgy || CreatingPolygon) {
        return;
    }
    int next_index = K;
    if (Fel) {
        // Lejjebb maszunk egy pontot:
        next_index--;
        if (next_index < 0) {
            next_index = Pgy->vertex_count - 1;
        }
    } else {
        // Feljebb maszunk egy pontot:
        next_index++;
        if (next_index >= Pgy->vertex_count) {
            next_index = 0;
        }
    }
    Pgy->vertices[K] = Pgy->vertices[next_index];
    K = next_index;
    Moux = meter_to_pixel_x(Pgy->vertices[K].x);
    Mouy = meter_to_pixel_y(Pgy->vertices[K].y);
    set_mouse_position(Moux, Mouy);
    Fel = !Fel;
    invalidate();
}

void tool_create_vertex_mousemove(int mouse_x, int mouse_y) {
    if (!Pgy && !CreatingPolygon) {
        // Teljesen normalis kurzor mozgatas:
        internal_error("tool_create_vertex_mousemove invalid call!");
    }
    push();
    lockfrontbuffer_pic();
    if (Pgy) {
        // Letoroljuk oket:
        Pgy->render_one_line(K, Fel, true); // szaggatott
        Pgy->render_one_line(K, !Fel, false);
        // Uj koord:
        double x = pixel_to_meter_x(mouse_x);
        double y = pixel_to_meter_y(mouse_y);
        Pgy->set_vertex(K, x, y);
        // Visszarajzoljuk oket:
        Pgy->render_one_line(K, Fel, true); // szaggatott
        Pgy->render_one_line(K, !Fel, false);
        Moux = mouse_x;
        Mouy = mouse_y;
    } else {
        if (!CreatingPolygon) {
            internal_error("tool_create_vertex_mousemove !CreatingPolygon");
        }
        render_line(pixel_to_meter(FirstVertexX, FirstVertexY),
                    pixel_to_meter(MouseVertexX, MouseVertexY), false);
        Moux = MouseVertexX = mouse_x;
        Mouy = MouseVertexY = mouse_y;
        render_line(pixel_to_meter(FirstVertexX, FirstVertexY),
                    pixel_to_meter(MouseVertexX, MouseVertexY), false);
    }
    unlockfrontbuffer_pic();
    pop();
}

// DELETE DELETE DELETE DELETE DELETE DELETE DELETE DELETE
// DELETE DELETE DELETE DELETE DELETE DELETE DELETE DELETE
// DELETE DELETE DELETE DELETE DELETE DELETE DELETE DELETE
// DELETE DELETE DELETE DELETE DELETE DELETE DELETE DELETE

void tool_delete_vertex_leftclick(int mouse_x, int mouse_y) {
    if (Pgy) {
        internal_error("tool_delete_vertex_leftclick Pgy");
    }
    // Most probalunk megfogni egy pontot:
    double x = pixel_to_meter_x(mouse_x);
    double y = pixel_to_meter_y(mouse_y);

    int vertex_index = 0;
    polygon* poly = Ptop->get_closest_vertex(x, y, &vertex_index);
    if (!poly) {
        return;
    }
    // Talaltunk egy pontot, amit le kene torolni:
    if (poly->vertex_count < 3) {
        internal_error("tool_delete_vertex_leftclick poly->vertex_count < 3");
    }
    if (poly->vertex_count == 3) {
        // dialog( "You cannot delete this polygon, because every" );
        dialog("You cannot delete this vertex, because every polygon must",
               "have at least three vertices! If you want to delete the",
               "whole polygon, please select the DELETE POLYGON tool!");
        return;
    }
    poly->delete_vertex(vertex_index);
    Valtozott = 1;
    invalidate();
}

void tool_delete_polygon_leftclick(int mouse_x, int mouse_y) {
    if (Pgy) {
        internal_error("tool_delete_polygon_leftclick Pgy");
    }
    // Most probalunk megfogni egy pontot:
    double x = pixel_to_meter_x(mouse_x);
    double y = pixel_to_meter_y(mouse_y);

    int vertex_index = 0;
    polygon* poly = Ptop->get_closest_vertex(x, y, &vertex_index);
    if (!poly) {
        return;
    }
    // Talaltunk egy poligont, amit le kene torolni:
    int poly_count = 0;
    for (int i = 0; i < MAX_POLYGONS; i++) {
        if (Ptop->polygons[i]) {
            poly_count++;
        }
    }
    if (poly_count == 0) {
        internal_error("tool_delete_polygon_leftclick poly_count == 0");
    }
    if (poly_count == 1) {
        dialog("This is the only polygon, so you cannot delete it.");
        return;
    }
    for (int i = 0; i < MAX_POLYGONS; i++) {
        if (Ptop->polygons[i] == poly) {
            delete Ptop->polygons[i];
            Ptop->polygons[i] = NULL;
            Valtozott = 1;
            invalidate();
            return;
        }
    }
    internal_error("tool_delete_polygon_leftclick poly not found!");
}

// CREATE_KEREK CREATE_KEREK CREATE_KEREK CREATE_KEREK CREATE_KEREK
// CREATE_KEREK CREATE_KEREK CREATE_KEREK CREATE_KEREK CREATE_KEREK
// CREATE_KEREK CREATE_KEREK CREATE_KEREK CREATE_KEREK CREATE_KEREK
// CREATE_KEREK CREATE_KEREK CREATE_KEREK CREATE_KEREK CREATE_KEREK

// Legkozelebbi create food ezt fogja hasznalni:
object::Property DefaultFoodProperty = object::Property::None;
int DefaultFoodAnimation = 0;

void tool_create_food_rightclick() {
    editor_window_food_properties("Select Create_Food subtype!", &DefaultFoodProperty,
                                  &DefaultFoodAnimation);
}

// Food es killer is egyben:
void tool_create_object_leftclick(int mouse_x, int mouse_y, int is_food) {
    // Megszamoljuk eddigi objektumokat:
    int object_count = 0;
    for (int i = 0; i < MAX_OBJECTS; i++) {
        if (Ptop->objects[i]) {
            object_count++;
        }
    }
    if (object_count >= MAX_OBJECTS) {
        char tmp[100];
        sprintf(tmp, "You have already reached the maximum number of objects (%d)!",
                MAX_OBJECTS - 2);
        dialog(tmp);
        return;
    }

    for (int i = 0; i < MAX_OBJECTS; i++) {
        if (!Ptop->objects[i]) {
            double x = pixel_to_meter_x(mouse_x);
            double y = pixel_to_meter_y(mouse_y);
            object::Type type = object::Type::Food;
            if (!is_food) {
                type = object::Type::Killer;
            }
            Ptop->objects[i] = new object(x, y, type);
            if (is_food) {
                Ptop->objects[i]->property = DefaultFoodProperty;
                Ptop->objects[i]->animation = DefaultFoodAnimation;
            }
            invalidate();
            Valtozott = 1;
            return;
        }
    }
    internal_error("tool_create_object_leftclick no free object slot!");
}

// DELETE_KEREK DELETE_KEREK DELETE_KEREK DELETE_KEREK DELETE_KEREK
// DELETE_KEREK DELETE_KEREK DELETE_KEREK DELETE_KEREK DELETE_KEREK
// DELETE_KEREK DELETE_KEREK DELETE_KEREK DELETE_KEREK DELETE_KEREK
// DELETE_KEREK DELETE_KEREK DELETE_KEREK DELETE_KEREK DELETE_KEREK

void tool_delete_object_leftclick(int mouse_x, int mouse_y) {
    double x = pixel_to_meter_x(mouse_x);
    double y = pixel_to_meter_y(mouse_y);
    object* obj = Ptop->get_closest_object(x, y);
    if (!obj) {
        return;
    }
    for (int i = 0; i < MAX_OBJECTS; i++) {
        if (Ptop->objects[i] == obj) {
            if (obj->type == object::Type::Exit) {
                dialog("You cannot delete the Exit object!");
                return;
            }
            if (obj->type == object::Type::Start) {
                dialog("You cannot delete the Start object!");
                return;
            }
            delete obj;
            Ptop->objects[i] = NULL;
            Valtozott = 1;
            invalidate();
            return;
        }
    }
    internal_error("tool_delete_object_leftclick obj not found!");
}

// ZOOM ZOOM ZOOM ZOOM ZOOM ZOOM ZOOM ZOOM ZOOM ZOOM ZOOM ZOOM
// ZOOM ZOOM ZOOM ZOOM ZOOM ZOOM ZOOM ZOOM ZOOM ZOOM ZOOM ZOOM
// ZOOM ZOOM ZOOM ZOOM ZOOM ZOOM ZOOM ZOOM ZOOM ZOOM ZOOM ZOOM
// ZOOM ZOOM ZOOM ZOOM ZOOM ZOOM ZOOM ZOOM ZOOM ZOOM ZOOM ZOOM

int SelectingZoomInBox = 0; // Ha ez igaz, akkor nem szabad gombokat nyomkodni
static int ZoomInX1 = 0, ZoomInY1 = 0;
static int ZoomInX2 = 0, ZoomInY2 = 0;

void tool_zoom_in_leftclick(int mouse_x, int mouse_y) {
    if (SelectingZoomInBox) {
        // Megvan masodik pont is:
        draw_tooltip_help();
        zoom_in(ZoomInX1, ZoomInY1, mouse_x, mouse_y);
        SelectingZoomInBox = 0;
        invalidate();
    } else {
        // Most tesszuk le elso pontot:
        toolhelp("Left click to place the second corner of the zoom window. ESC or right click "
                 "cancels.");
        SelectingZoomInBox = 1;
        ZoomInX1 = ZoomInX2 = mouse_x;
        ZoomInY1 = ZoomInY2 = mouse_y;
    }
}

void tool_zoom_in_esc() {
    if (SelectingZoomInBox) {
        draw_tooltip_help();
        SelectingZoomInBox = 0;
        invalidate();
    }
}

static void draw_zoom_in_rectangle_pixel(int x, int y) {
    unsigned char palette_id = BufferMain->gpixel(x, y);
    palette_id += 128;
    BufferMain->ppixel(x, y, palette_id);
    ppixelfront(x, y, palette_id);
}

static void draw_zoom_in_rectangle(int x1, int y1, int x2, int y2) {
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
        draw_zoom_in_rectangle_pixel(x, y1);
        draw_zoom_in_rectangle_pixel(x, y2);
    }
    for (int y = y1; y <= y2; y++) {
        draw_zoom_in_rectangle_pixel(x1, y);
        draw_zoom_in_rectangle_pixel(x2, y);
    }
}

void tool_zoom_in_mousemove(int mouse_x, int mouse_y) {
    push();
    if (SelectingZoomInBox) {
        lockfrontbuffer_pic();
        draw_zoom_in_rectangle(ZoomInX1, ZoomInY1, ZoomInX2, ZoomInY2);
        ZoomInX2 = mouse_x;
        ZoomInY2 = mouse_y;
        Moux = mouse_x;
        Mouy = mouse_y;
        draw_zoom_in_rectangle(ZoomInX1, ZoomInY1, ZoomInX2, ZoomInY2);
        unlockfrontbuffer_pic();
    } else {
        // Teljesen normalis kurzor mozgatas:
        Moux = mouse_x;
        Mouy = mouse_y;
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
void tool_create_sprite_rightclick() {
    // Lgr-en vegzi muveleteit:
    if (!Lgr) {
        internal_error("tool_create_sprite_rightclick !Lgr");
    }
    if (Lgr->picture_count <= 0) {
        dialog("There are no pictures in the LGR file!");
        return;
    }
    editor_window_choose_sprite();
    /*if( Lgr->aktivindex >= Lgr->picture_count )
        internal_error( "[op0fijh" );
    kispritenev( Lgr->nevek[Lgr->aktivindex],
                 Lgr->tavolsagok[Lgr->aktivindex],
                 Lgr->hatarolok[Lgr->aktivindex] );*/
}

void tool_create_sprite_leftclick(int mouse_x, int mouse_y) {
    if (!Lgr) {
        internal_error("tool_create_sprite_leftclick !Lgr");
    }
    if (Lgr->picture_count <= 0) {
        dialog("There are no pictures in the LGR file!");
        return;
    }

    // Megszamoljuk eddigi objektumokat:
    int sprite_count = 0;
    for (int i = 0; i < MAX_SPRITES; i++) {
        if (Ptop->sprites[i]) {
            sprite_count++;
        }
    }
    if (sprite_count >= MAX_SPRITES) {
        char tmp[100];
        sprintf(tmp, "You have already reached the maximum number of pictures (%d)!", MAX_SPRITES);
        dialog(tmp);
        return;
    }

    if (!Lgr->editor_picture_name[0] &&
        !(Lgr->editor_texture_name[0] && Lgr->editor_mask_name[0])) {
        return; // Nincs kitoltve aktiv sprite
    }

    for (int i = 0; i < MAX_SPRITES; i++) {
        if (!Ptop->sprites[i]) {
            double x = pixel_to_meter_x(mouse_x);
            double y = pixel_to_meter_y(mouse_y);
            Ptop->sprites[i] = new sprite(x, y, Lgr->editor_picture_name, Lgr->editor_texture_name,
                                          Lgr->editor_mask_name);
            invalidate();
            Valtozott = 1;
            return;
        }
    }
    internal_error("tool_create_sprite_leftclick no free sprite slot!");
}

void tool_delete_sprite_leftclick(int mouse_x, int mouse_y) {
    double x = pixel_to_meter_x(mouse_x);
    double y = pixel_to_meter_y(mouse_y);
    sprite* spr = Ptop->get_closest_sprite(x, y);
    if (!spr) {
        return;
    }
    for (int i = 0; i < MAX_SPRITES; i++) {
        if (Ptop->sprites[i] == spr) {
            delete spr;
            Ptop->sprites[i] = NULL;
            Valtozott = 1;
            invalidate();
            return;
        }
    }
    internal_error("tool_delete_sprite_leftclick sprite not found!");
}
