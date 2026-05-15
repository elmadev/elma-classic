#include "editor/tool.h"
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

static bool delete_polygon(polygon* poly) {
    int poly_count = 0;
    for (int i = 0; i < MAX_POLYGONS; i++) {
        if (Level->polygons[i]) {
            poly_count++;
        }
    }
    if (poly_count == 0) {
        internal_error("delete_polygon poly_count == 0");
    }
    if (poly_count == 1) {
        return false;
    }
    for (int i = 0; i < MAX_POLYGONS; i++) {
        if (Level->polygons[i] == poly) {
            delete Level->polygons[i];
            Level->polygons[i] = nullptr;
            K = 0;
            Valtozott = 1;
            invalidate();
            return true;
        }
    }
    internal_error("delete_polygon poly not found!");
}

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

static double ToolMoveOriginalX;
static double ToolMoveOriginalY;

void tool_move_leftclick(int mouse_x, int mouse_y) {
    if (Pgy || Pker || Psp) {
        // We already are holding something in cursor - place it
        if ((Pgy && Pker) || (Pgy && Psp) || (Pker && Psp)) {
            internal_error("tool_move_leftclick (Pgy && Pker) || (Pgy && Psp) || (Pker && Psp)!");
        }
        if (Pgy) {
            Pgy->vertices[K] = pixel_to_meter(mouse_x, mouse_y);
            Pgy = nullptr;
        }
        if (Pker) {
            Pker->r = pixel_to_meter(mouse_x, mouse_y);
            Pker = nullptr;
        }
        if (Psp) {
            Psp->r = pixel_to_meter(mouse_x, mouse_y);
            Psp = nullptr;
        }
        Valtozott = 1;
        draw_tooltip_help();
        invalidate();
        return;
    }

    // We aren't holding anything. Grab the closest one within 10 pixels
    Level->get_closest_entity(mouse_x, mouse_y, &Pgy, &K, &Pker, &Psp);
    if (!Pgy && !Pker && !Psp) {
        return;
    }
    if (Pgy) {
        toolhelp("Move the vertex and left click to place it. Press ESC or right click to cancel.");
        ToolMoveOriginalX = Pgy->vertices[K].x;
        ToolMoveOriginalY = Pgy->vertices[K].y;
        mouse_x = meter_to_pixel_x(ToolMoveOriginalX);
        mouse_y = meter_to_pixel_y(ToolMoveOriginalY);
    }
    if (Pker) {
        toolhelp("Move the object and left click to place it. Press ESC or right click to "
                 "cancel.");
        ToolMoveOriginalX = Pker->r.x;
        ToolMoveOriginalY = Pker->r.y;
        mouse_x = meter_to_pixel_x(ToolMoveOriginalX);
        mouse_y = meter_to_pixel_y(ToolMoveOriginalY);
    }
    if (Psp) {
        toolhelp("Move the picture and left click to place it. Press ESC or right click to "
                 "cancel.");
        ToolMoveOriginalX = Psp->r.x;
        ToolMoveOriginalY = Psp->r.y;
        mouse_x = meter_to_pixel_x(ToolMoveOriginalX);
        mouse_y = meter_to_pixel_y(ToolMoveOriginalY);
    }

    set_mouse_position(mouse_x, mouse_y);
    push();
    Moux = mouse_x;
    Mouy = mouse_y;
    pop();
}

void tool_move_rightclick(int mouse_x, int mouse_y) {
    if (Pgy || Pker || Psp) {
        // Drop what we are holding
        tool_move_esc();
        return;
    }

    // Otherwise, open the properties dialog for the closest sprite/polygon/object
    sprite* spr;
    int unused;
    polygon* poly;
    object* obj;
    Level->get_closest_entity(mouse_x, mouse_y, &poly, &unused, &obj, &spr);
    if (spr) {
        editor_window_sprite_properties(spr);
    }
    if (poly) {
        editor_window_polygon_properties(poly);
    }
    if (obj && obj->type == object::Type::Food) {
        editor_window_food_properties("Select food subtype!", &obj->property, &obj->animation);
    }
}

void tool_move_esc() {
    // Drop what we are holding
    if (Pgy) {
        Pgy->vertices[K].x = ToolMoveOriginalX;
        Pgy->vertices[K].y = ToolMoveOriginalY;
        Pgy = nullptr;
        invalidate();
    }
    if (Pker) {
        Pker->r.x = ToolMoveOriginalX;
        Pker->r.y = ToolMoveOriginalY;
        Pker = nullptr;
        invalidate();
    }
    if (Psp) {
        Psp->r.x = ToolMoveOriginalX;
        Psp->r.y = ToolMoveOriginalY;
        Psp = nullptr;
        invalidate();
    }
}

void tool_move_mousemove(int mouse_x, int mouse_y) {
    if (!Pgy && !Pker && !Psp) {
        internal_error("tool_move_mousemove !Pgy && !Pker && !Psp");
    }
    if ((Pgy && Pker) || (Pgy && Psp) || (Pker && Psp)) {
        internal_error("tool_move_mousemove: (Pgy && Pker) || (Pgy && Psp) || (Pker && Psp)!");
    }

    push();
    lockfrontbuffer_pic();

    // Update the position of the item by redrawing it at the old position to erase,
    // then drawing it at the new position
    if (Pgy) {
        Pgy->render_one_line(K, Fel, false);
        Pgy->render_one_line(K, !Fel, false);
        double x = pixel_to_meter_x(mouse_x);
        double y = pixel_to_meter_y(mouse_y);
        Pgy->set_vertex(K, x, y);
        Pgy->render_one_line(K, Fel, false);
        Pgy->render_one_line(K, !Fel, false);
    }
    if (Pker) {
        Pker->render();
        Pker->r.x = pixel_to_meter_x(mouse_x);
        Pker->r.y = pixel_to_meter_y(mouse_y);
        Pker->render();
    }
    if (Psp) {
        Psp->render();
        Psp->r.x = pixel_to_meter_x(mouse_x);
        Psp->r.y = pixel_to_meter_y(mouse_y);
        Psp->render();
    }
    unlockfrontbuffer_pic();

    Moux = mouse_x;
    Mouy = mouse_y;

    pop();
}

bool CreatingPolygon = false;
static vect2 FirstVertex;
static vect2 MouseVertex;

void tool_create_vertex_leftclick(int mouse_x, int mouse_y) {
    Valtozott = 1;
    if (!Pgy && !CreatingPolygon) {
        // We aren't holding anything - find the closest vertex
        double x = pixel_to_meter_x(mouse_x);
        double y = pixel_to_meter_y(mouse_y);
        Pgy = Level->get_closest_vertex(x, y, &K);
        if (!Pgy) {
            // We didn't find a closest vertex. Create a new polygon
            int poly_count = 0;
            for (int i = 0; i < MAX_POLYGONS; i++) {
                if (Level->polygons[i]) {
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

            // Until the second point is placed, we don't have a polygon with 3 coordinates;
            // what we have is simply a line between 2 coordinates
            // Store this information temporarily and only create a real polygon once the 2nd
            // coordinate is placed.
            toolhelp("Left click to place the second vertex. ESC or right click cancels.");
            CreatingPolygon = true;
            FirstVertex = MouseVertex = vect2(x, y);
            return;
        }
        // We found the closest vertex
        toolhelp("Left click to place vertex. SPACE and ENTER swaps. ESC or right "
                 "click cancels.");
        push();
        Moux = meter_to_pixel_x(Pgy->vertices[K].x);
        Mouy = meter_to_pixel_y(Pgy->vertices[K].y);
        pop();
    }

    // We are holding a vertex
    if (Pgy) {
        // Create a new vertex
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
        return;
    }

    // We are creating a new polygon and this is the second point, so let's convert from
    // temporary coordinates to a real polygon
    toolhelp("Left click to place point. SPACE and ENTER swaps. ESC or right click "
             "cancels.");
    if (!CreatingPolygon) {
        internal_error("tool_create_vertex_leftclick !Pgy !CreatingPolygon");
    }
    for (int i = 0; i < MAX_POLYGONS; i++) {
        if (!Level->polygons[i]) {
            Pgy = Level->polygons[i] = new polygon;
            Pgy->vertex_count = 3;
            Pgy->vertices[0] = FirstVertex;
            Pgy->vertices[2] = Pgy->vertices[1] = pixel_to_meter(mouse_x, mouse_y);
            K = 2;
            Fel = true;
            CreatingPolygon = false;
            invalidate();
            return;
        }
    };
    internal_error("tool_create_vertex_leftclick no free polygon!");
}

void tool_create_vertex_esc() {
    if (!Pgy && !CreatingPolygon) {
        return;
    }

    draw_tooltip_help();
    if (Pgy) {
        // If we have a polygon
        if (Pgy->vertex_count <= 3) {
            // If we only have 3 points or less, delete the polygon
            // This should occur only when we are creating a new polygon from scratch,
            // and we've only placed 2 points so far
            if (!delete_polygon(Pgy)) {
                internal_error("tool_create_vertex_esc poly_count <= 1!");
            }
            Pgy = nullptr;
            return;
        }
        // Delete the floating vertex at the mouse position
        Pgy->delete_vertex(K);
        Pgy = nullptr;
        K = 0;
        invalidate();
        return;
    }

    // If we were creating a new polygon and only have a line using temporary variables,
    // then delete this line
    if (!CreatingPolygon) {
        internal_error("tool_create_vertex_esc !CreatingPolygon!");
    }
    CreatingPolygon = false;
    invalidate();
}

void tool_create_vertex_enter() {
    // Swap the direction of vertex creation
    Fel = !Fel;
    if (Pgy && !CreatingPolygon) {
        invalidate();
    }
}

void tool_create_vertex_space() {
    // Move the floating vertex over by 1 to a new place in the polygon
    if (!Pgy || CreatingPolygon) {
        return;
    }
    int next_index = K;
    if (Fel) {
        next_index--;
        if (next_index < 0) {
            next_index = Pgy->vertex_count - 1;
        }
    } else {
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
        internal_error("tool_create_vertex_mousemove invalid call!");
    }
    push();
    lockfrontbuffer_pic();
    // Erase existing line by drawing over it, then draw new line
    double x = pixel_to_meter_x(mouse_x);
    double y = pixel_to_meter_y(mouse_y);
    if (Pgy) {
        Pgy->render_one_line(K, Fel, true);
        Pgy->render_one_line(K, !Fel, false);
        Pgy->set_vertex(K, x, y);
        Pgy->render_one_line(K, Fel, true);
        Pgy->render_one_line(K, !Fel, false);
    } else {
        if (!CreatingPolygon) {
            internal_error("tool_create_vertex_mousemove !CreatingPolygon");
        }
        render_line(FirstVertex, MouseVertex, false);
        MouseVertex = vect2(x, y);
        render_line(FirstVertex, MouseVertex, false);
    }
    Moux = mouse_x;
    Mouy = mouse_y;
    unlockfrontbuffer_pic();
    pop();
}

void tool_delete_vertex_leftclick(int mouse_x, int mouse_y) {
    if (Pgy) {
        internal_error("tool_delete_vertex_leftclick Pgy");
    }

    double x = pixel_to_meter_x(mouse_x);
    double y = pixel_to_meter_y(mouse_y);
    int vertex_index = 0;
    polygon* poly = Level->get_closest_vertex(x, y, &vertex_index);

    if (!poly) {
        return;
    }
    if (poly->vertex_count < 3) {
        internal_error("tool_delete_vertex_leftclick poly->vertex_count < 3");
    }
    if (poly->vertex_count == 3) {
        if (!delete_polygon(poly)) {
            dialog("This is the only polygon, so you cannot delete it.");
        }
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

    double x = pixel_to_meter_x(mouse_x);
    double y = pixel_to_meter_y(mouse_y);
    int vertex_index = 0;
    polygon* poly = Level->get_closest_vertex(x, y, &vertex_index);

    if (!poly) {
        return;
    }

    if (!delete_polygon(poly)) {
        dialog("This is the only polygon, so you cannot delete it.");
    }
}

object::Property DefaultFoodProperty = object::Property::None;
int DefaultFoodAnimation = 0;

void tool_create_food_rightclick() {
    editor_window_food_properties("Select the default subtype!", &DefaultFoodProperty,
                                  &DefaultFoodAnimation);
}

void tool_create_object_leftclick(int mouse_x, int mouse_y, bool is_food) {
    int object_count = 0;
    for (int i = 0; i < MAX_OBJECTS; i++) {
        if (Level->objects[i]) {
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
        if (!Level->objects[i]) {
            double x = pixel_to_meter_x(mouse_x);
            double y = pixel_to_meter_y(mouse_y);
            object::Type type = object::Type::Food;
            if (!is_food) {
                type = object::Type::Killer;
            }
            Level->objects[i] = new object(x, y, type);
            if (is_food) {
                Level->objects[i]->property = DefaultFoodProperty;
                Level->objects[i]->animation = DefaultFoodAnimation;
            }
            invalidate();
            Valtozott = 1;
            return;
        }
    }
    internal_error("tool_create_object_leftclick no free object slot!");
}

void tool_delete_object_leftclick(int mouse_x, int mouse_y) {
    double x = pixel_to_meter_x(mouse_x);
    double y = pixel_to_meter_y(mouse_y);
    object* obj = Level->get_closest_object(x, y);
    if (!obj) {
        return;
    }
    for (int i = 0; i < MAX_OBJECTS; i++) {
        if (Level->objects[i] == obj) {
            if (obj->type == object::Type::Exit) {
                dialog("You cannot delete the Exit object!");
                return;
            }
            if (obj->type == object::Type::Start) {
                dialog("You cannot delete the Start object!");
                return;
            }
            delete obj;
            Level->objects[i] = nullptr;
            Valtozott = 1;
            invalidate();
            return;
        }
    }
    internal_error("tool_delete_object_leftclick obj not found!");
}

bool SelectingZoomInBox = false;
static int ZoomInX1;
static int ZoomInY1;
static int ZoomInX2;
static int ZoomInY2;

void tool_zoom_in_leftclick(int mouse_x, int mouse_y) {
    if (SelectingZoomInBox) {
        // Second click
        draw_tooltip_help();
        zoom_in(ZoomInX1, ZoomInY1, mouse_x, mouse_y);
        SelectingZoomInBox = false;
        invalidate();
    } else {
        // First click
        toolhelp("Left click to place the second corner of the zoom window. ESC or right click "
                 "cancels.");
        SelectingZoomInBox = true;
        ZoomInX1 = ZoomInX2 = mouse_x;
        ZoomInY1 = ZoomInY2 = mouse_y;
    }
}

void tool_zoom_in_esc() {
    if (SelectingZoomInBox) {
        draw_tooltip_help();
        SelectingZoomInBox = false;
        invalidate();
    }
}

// Draw a single pixel in the editor to draw the zoom in box, by inverting the palette id
static void draw_zoom_in_rectangle_pixel(int x, int y) {
    unsigned char palette_id = BufferMain->gpixel(x, y);
    palette_id += 128;
    BufferMain->ppixel(x, y, palette_id);
    ppixelfront(x, y, palette_id);
}

// Draw the zoom in rectangle
static void draw_zoom_in_rectangle(int x1, int y1, int x2, int y2) {
    if (x2 < x1) {
        std::swap(x1, x2);
    }
    if (y2 < y1) {
        std::swap(y1, y2);
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
        // Draw the zoom in box only if we clicked to select the topleft corner
        lockfrontbuffer_pic();
        draw_zoom_in_rectangle(ZoomInX1, ZoomInY1, ZoomInX2, ZoomInY2);
        ZoomInX2 = mouse_x;
        ZoomInY2 = mouse_y;
        Moux = mouse_x;
        Mouy = mouse_y;
        draw_zoom_in_rectangle(ZoomInX1, ZoomInY1, ZoomInX2, ZoomInY2);
        unlockfrontbuffer_pic();
    } else {
        Moux = mouse_x;
        Mouy = mouse_y;
    }
    pop();
}

void tool_create_sprite_rightclick() {
    if (!Lgr) {
        internal_error("tool_create_sprite_rightclick !Lgr");
    }
    if (Lgr->picture_count <= 0) {
        dialog("There are no pictures in the LGR file!");
        return;
    }
    editor_window_choose_sprite();
}

void tool_create_sprite_leftclick(int mouse_x, int mouse_y) {
    if (!Lgr) {
        internal_error("tool_create_sprite_leftclick !Lgr");
    }
    if (Lgr->picture_count <= 0) {
        dialog("There are no pictures in the LGR file!");
        return;
    }

    int sprite_count = 0;
    for (int i = 0; i < MAX_SPRITES; i++) {
        if (Level->sprites[i]) {
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
        // No sprite selected
        return;
    }

    for (int i = 0; i < MAX_SPRITES; i++) {
        if (!Level->sprites[i]) {
            double x = pixel_to_meter_x(mouse_x);
            double y = pixel_to_meter_y(mouse_y);
            Level->sprites[i] = new sprite(x, y, Lgr->editor_picture_name, Lgr->editor_texture_name,
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
    sprite* spr = Level->get_closest_sprite(x, y);
    if (!spr) {
        return;
    }
    for (int i = 0; i < MAX_SPRITES; i++) {
        if (Level->sprites[i] == spr) {
            delete spr;
            Level->sprites[i] = nullptr;
            Valtozott = 1;
            invalidate();
            return;
        }
    }
    internal_error("tool_delete_sprite_leftclick sprite not found!");
}
