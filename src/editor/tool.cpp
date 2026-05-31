#include "editor/tool.h"
#include "editor/canvas.h"
#include "editor/dialog.h"
#include "editor/editor.h"
#include "editor/window.h"
#include "level/level.h"
#include "level/polygon.h"
#include "level/sprite.h"
#include "main.h"
#include "menu/pic.h"
#include "pic/lgr.h"
#include "pic/pic8.h"
#include "pic/surface.h"
#include "platform/implementation.h"

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
            SelectedVertexIndex = 0;
            LevelChanged = true;
            invalidate_editor_level();
            return true;
        }
    }
    internal_error("delete_polygon poly not found!");
}

static double ToolMoveOriginalX;
static double ToolMoveOriginalY;

void tool_move_leftclick(int mouse_x, int mouse_y) {
    if (SelectedPolygon || SelectedObject || SelectedSprite) {
        // We already are holding something in cursor - place it
        if ((SelectedPolygon && SelectedObject) || (SelectedPolygon && SelectedSprite) ||
            (SelectedObject && SelectedSprite)) {
            internal_error(
                "tool_move_leftclick (SelectedPolygon && SelectedObject) || (SelectedPolygon && "
                "SelectedSprite) || (SelectedObject && SelectedSprite)!");
        }
        if (SelectedPolygon) {
            SelectedPolygon->vertices[SelectedVertexIndex] = pixel_to_meter(mouse_x, mouse_y);
            SelectedPolygon = nullptr;
        }
        if (SelectedObject) {
            SelectedObject->r = pixel_to_meter(mouse_x, mouse_y);
            SelectedObject = nullptr;
        }
        if (SelectedSprite) {
            SelectedSprite->r = pixel_to_meter(mouse_x, mouse_y);
            SelectedSprite = nullptr;
        }
        LevelChanged = true;
        draw_tooltip_help();
        invalidate_editor_level();
        return;
    }

    // We aren't holding anything. Grab the closest one within 10 pixels
    Level->get_closest_entity(mouse_x, mouse_y, &SelectedPolygon, &SelectedVertexIndex,
                              &SelectedObject, &SelectedSprite);
    if (!SelectedPolygon && !SelectedObject && !SelectedSprite) {
        return;
    }
    if (SelectedPolygon) {
        draw_tooltip(
            "Move the vertex and left click to place it. Press ESC or right click to cancel.");
        ToolMoveOriginalX = SelectedPolygon->vertices[SelectedVertexIndex].x;
        ToolMoveOriginalY = SelectedPolygon->vertices[SelectedVertexIndex].y;
        mouse_x = meter_to_pixel_x(ToolMoveOriginalX);
        mouse_y = meter_to_pixel_y(ToolMoveOriginalY);
    }
    if (SelectedObject) {
        draw_tooltip("Move the object and left click to place it. Press ESC or right click to "
                     "cancel.");
        ToolMoveOriginalX = SelectedObject->r.x;
        ToolMoveOriginalY = SelectedObject->r.y;
        mouse_x = meter_to_pixel_x(ToolMoveOriginalX);
        mouse_y = meter_to_pixel_y(ToolMoveOriginalY);
    }
    if (SelectedSprite) {
        draw_tooltip("Move the picture and left click to place it. Press ESC or right click to "
                     "cancel.");
        ToolMoveOriginalX = SelectedSprite->r.x;
        ToolMoveOriginalY = SelectedSprite->r.y;
        mouse_x = meter_to_pixel_x(ToolMoveOriginalX);
        mouse_y = meter_to_pixel_y(ToolMoveOriginalY);
    }

    set_mouse_position(mouse_x, mouse_y);
    MouseX = mouse_x;
    MouseY = mouse_y;
}

void tool_move_rightclick(int mouse_x, int mouse_y) {
    if (SelectedPolygon || SelectedObject || SelectedSprite) {
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
    if (SelectedPolygon) {
        SelectedPolygon->vertices[SelectedVertexIndex].x = ToolMoveOriginalX;
        SelectedPolygon->vertices[SelectedVertexIndex].y = ToolMoveOriginalY;
        SelectedPolygon = nullptr;
        invalidate_editor_level();
    }
    if (SelectedObject) {
        SelectedObject->r.x = ToolMoveOriginalX;
        SelectedObject->r.y = ToolMoveOriginalY;
        SelectedObject = nullptr;
        invalidate_editor_level();
    }
    if (SelectedSprite) {
        SelectedSprite->r.x = ToolMoveOriginalX;
        SelectedSprite->r.y = ToolMoveOriginalY;
        SelectedSprite = nullptr;
        invalidate_editor_level();
    }
}

void tool_move_mousemove(int mouse_x, int mouse_y) {
    if (!SelectedPolygon && !SelectedObject && !SelectedSprite) {
        internal_error(
            "tool_move_mousemove !SelectedPolygon && !SelectedObject && !SelectedSprite");
    }
    if ((SelectedPolygon && SelectedObject) || (SelectedPolygon && SelectedSprite) ||
        (SelectedObject && SelectedSprite)) {
        internal_error("tool_move_mousemove: (SelectedPolygon && SelectedObject) || "
                       "(SelectedPolygon && SelectedSprite) "
                       "|| (SelectedObject && SelectedSprite)!");
    }

    // Update the position of the item by redrawing it at the old position to erase,
    // then drawing it at the new position
    if (SelectedPolygon) {
        SelectedPolygon->render_one_line(SelectedVertexIndex, CreateVertexDirection, false);
        SelectedPolygon->render_one_line(SelectedVertexIndex, !CreateVertexDirection, false);
        double x = pixel_to_meter_x(mouse_x);
        double y = pixel_to_meter_y(mouse_y);
        SelectedPolygon->set_vertex(SelectedVertexIndex, x, y);
        SelectedPolygon->render_one_line(SelectedVertexIndex, CreateVertexDirection, false);
        SelectedPolygon->render_one_line(SelectedVertexIndex, !CreateVertexDirection, false);
    }
    if (SelectedObject) {
        SelectedObject->render();
        SelectedObject->r.x = pixel_to_meter_x(mouse_x);
        SelectedObject->r.y = pixel_to_meter_y(mouse_y);
        SelectedObject->render();
    }
    if (SelectedSprite) {
        SelectedSprite->render();
        SelectedSprite->r.x = pixel_to_meter_x(mouse_x);
        SelectedSprite->r.y = pixel_to_meter_y(mouse_y);
        SelectedSprite->render();
    }
}

bool CreatingPolygon = false;
static vect2 FirstVertex;
static vect2 MouseVertex;

void tool_create_vertex_leftclick(int mouse_x, int mouse_y) {
    LevelChanged = true;
    if (!SelectedPolygon && !CreatingPolygon) {
        // We aren't holding anything - find the closest vertex
        double x = pixel_to_meter_x(mouse_x);
        double y = pixel_to_meter_y(mouse_y);
        SelectedPolygon = Level->get_closest_vertex(x, y, &SelectedVertexIndex);
        if (!SelectedPolygon) {
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
            draw_tooltip("Left click to place the second vertex. ESC or right click cancels.");
            CreatingPolygon = true;
            FirstVertex = MouseVertex = vect2(x, y);
            return;
        }
        // We found the closest vertex
        draw_tooltip("Left click to place vertex. SPACE and ENTER swaps. ESC or right "
                     "click cancels.");
    }

    // We are holding a vertex
    if (SelectedPolygon) {
        // Create a new vertex
        if (SelectedPolygon->vertex_count >= MAX_VERTICES) {
            char tmp[100];
            sprintf(tmp, "already reached the maximum number of vertices in this level! (%d)",
                    MAX_VERTICES);
            dialog("You cannot create a new vertex, because you have", tmp);
            return;
        }
        draw_tooltip("Left click to place vertex. SPACE and ENTER swaps. ESC or right click "
                     "cancels.");
        SelectedPolygon->insert_vertex(SelectedVertexIndex);
        invalidate_editor_level();
        if (CreateVertexDirection) {
            SelectedVertexIndex++;
        }
        return;
    }

    // We are creating a new polygon and this is the second point, so let's convert from
    // temporary coordinates to a real polygon
    draw_tooltip("Left click to place point. SPACE and ENTER swaps. ESC or right click "
                 "cancels.");
    if (!CreatingPolygon) {
        internal_error("tool_create_vertex_leftclick !SelectedPolygon !CreatingPolygon");
    }
    for (int i = 0; i < MAX_POLYGONS; i++) {
        if (!Level->polygons[i]) {
            SelectedPolygon = Level->polygons[i] = new polygon;
            SelectedPolygon->vertex_count = 3;
            SelectedPolygon->vertices[0] = FirstVertex;
            SelectedPolygon->vertices[2] = SelectedPolygon->vertices[1] =
                pixel_to_meter(mouse_x, mouse_y);
            SelectedVertexIndex = 2;
            CreateVertexDirection = true;
            CreatingPolygon = false;
            invalidate_editor_level();
            return;
        }
    };
    internal_error("tool_create_vertex_leftclick no free polygon!");
}

void tool_create_vertex_esc() {
    if (!SelectedPolygon && !CreatingPolygon) {
        return;
    }

    draw_tooltip_help();
    if (SelectedPolygon) {
        // If we have a polygon
        if (SelectedPolygon->vertex_count <= 3) {
            // If we only have 3 points or less, delete the polygon
            // This should occur only when we are creating a new polygon from scratch,
            // and we've only placed 2 points so far
            if (!delete_polygon(SelectedPolygon)) {
                internal_error("tool_create_vertex_esc poly_count <= 1!");
            }
            SelectedPolygon = nullptr;
            return;
        }
        // Delete the floating vertex at the mouse position
        SelectedPolygon->delete_vertex(SelectedVertexIndex);
        SelectedPolygon = nullptr;
        SelectedVertexIndex = 0;
        invalidate_editor_level();
        return;
    }

    // If we were creating a new polygon and only have a line using temporary variables,
    // then delete this line
    if (!CreatingPolygon) {
        internal_error("tool_create_vertex_esc !CreatingPolygon!");
    }
    CreatingPolygon = false;
    invalidate_editor_level();
}

void tool_create_vertex_enter() {
    // Swap the direction of vertex creation
    CreateVertexDirection = !CreateVertexDirection;
    if (SelectedPolygon && !CreatingPolygon) {
        invalidate_editor_level();
    }
}

void tool_create_vertex_space() {
    // Move the floating vertex over by 1 to a new place in the polygon
    if (!SelectedPolygon || CreatingPolygon) {
        return;
    }
    int next_index = SelectedVertexIndex;
    if (CreateVertexDirection) {
        next_index--;
        if (next_index < 0) {
            next_index = SelectedPolygon->vertex_count - 1;
        }
    } else {
        next_index++;
        if (next_index >= SelectedPolygon->vertex_count) {
            next_index = 0;
        }
    }
    SelectedPolygon->vertices[SelectedVertexIndex] = SelectedPolygon->vertices[next_index];
    SelectedVertexIndex = next_index;
    MouseX = meter_to_pixel_x(SelectedPolygon->vertices[SelectedVertexIndex].x);
    MouseY = meter_to_pixel_y(SelectedPolygon->vertices[SelectedVertexIndex].y);
    set_mouse_position(MouseX, MouseY);
    CreateVertexDirection = !CreateVertexDirection;
    invalidate_editor_level();
}

void tool_create_vertex_mousemove(int mouse_x, int mouse_y) {
    if (!SelectedPolygon && !CreatingPolygon) {
        internal_error("tool_create_vertex_mousemove invalid call!");
    }

    // Erase existing line by drawing over it, then draw new line
    double x = pixel_to_meter_x(mouse_x);
    double y = pixel_to_meter_y(mouse_y);
    if (SelectedPolygon) {
        SelectedPolygon->render_one_line(SelectedVertexIndex, CreateVertexDirection, true);
        SelectedPolygon->render_one_line(SelectedVertexIndex, !CreateVertexDirection, false);
        SelectedPolygon->set_vertex(SelectedVertexIndex, x, y);
        SelectedPolygon->render_one_line(SelectedVertexIndex, CreateVertexDirection, true);
        SelectedPolygon->render_one_line(SelectedVertexIndex, !CreateVertexDirection, false);
    } else {
        if (!CreatingPolygon) {
            internal_error("tool_create_vertex_mousemove !CreatingPolygon");
        }
        render_line(FirstVertex, MouseVertex, false);
        MouseVertex = vect2(x, y);
        render_line(FirstVertex, MouseVertex, false);
    }
}

void tool_delete_vertex_leftclick(int mouse_x, int mouse_y) {
    if (SelectedPolygon) {
        internal_error("tool_delete_vertex_leftclick SelectedPolygon");
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
    LevelChanged = true;
    invalidate_editor_level();
}

void tool_delete_polygon_leftclick(int mouse_x, int mouse_y) {
    if (SelectedPolygon) {
        internal_error("tool_delete_polygon_leftclick SelectedPolygon");
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
            invalidate_editor_level();
            LevelChanged = true;
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
            LevelChanged = true;
            invalidate_editor_level();
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
        invalidate_editor_level();
    } else {
        // First click
        draw_tooltip("Left click to place the second corner of the zoom window. ESC or right click "
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
        invalidate_editor_level();
    }
}

// Draw a single pixel in the editor to draw the zoom in box, by inverting the palette id
static void draw_zoom_in_rectangle_pixel(int x, int y) {
    unsigned char palette_id = BufferMain->gpixel(x, y);
    palette_id += 128;
    BufferMain->ppixel(x, y, palette_id);
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
    if (SelectingZoomInBox) {
        // Draw the zoom in box only if we clicked to select the topleft corner
        draw_zoom_in_rectangle(ZoomInX1, ZoomInY1, ZoomInX2, ZoomInY2);
        ZoomInX2 = mouse_x;
        ZoomInY2 = mouse_y;
        draw_zoom_in_rectangle(ZoomInX1, ZoomInY1, ZoomInX2, ZoomInY2);
    }
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
            invalidate_editor_level();
            LevelChanged = true;
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
            LevelChanged = true;
            invalidate_editor_level();
            return;
        }
    }
    internal_error("tool_delete_sprite_leftclick sprite not found!");
}
