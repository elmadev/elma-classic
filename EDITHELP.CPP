#include "EDITHELP.H"
#include "editor_dialog.h"

void editor_help() {
    dialog("GENERAL HELP",
           "On the left of the screen is a column of buttons. The buttons in the upper half are",
           "command buttons, that means they will take action at the moment you click on them with",
           "the left mouse button. The buttons in the lower half are the tool selection buttons.",
           "If you click on them with the left mouse button, you select a tool. You can use a tool",
           "in the working area of the screen. A one line help of the current tool is always "
           "displayed",
           "at the top of your screen.", "",
           "You can get help on the command and tool selection buttons by clicking on any of them",
           "with the right mouse button.", "",
           "Load the tutor1.lev file to take a first look on a level design.");
}

void editor_help_exit() {
    dialog("With the Exit button you can exit the editor.",
           "If there are any unsaved changes in the level file in the editor, a dialog will",
           "appear asking if you really want to exit without saving these changes.");
}

void editor_help_zoom_out() {
    dialog("With the Zoom Out button you can make larger the view window of the editor.",
           "In the upper right corner you can always see the actual zoom value. If this value is 1",
           "you cannot make the zoom window any larger.");
}

void editor_help_save() {
    dialog("With the Save button you can save the content of the editor to a level file.",
           "The name of the level file is always displayed at the top of the screen.",
           "If you haven't named your file, you will be prompted for a file name.",
           "In this case you must enter the name without an extension and a path, as the '.LEV'",
           "extension will be automatically added and you can save the file only in your current",
           "directory.");
}

void editor_help_save_as() {
    dialog("With the Save As button you can save the content of the editor to a level file",
           "with a new name.",
           "You will be prompted for a file name. You must enter the name without an extension and",
           "a path, as the '.LEV' extension will be automatically added and you can save the file",
           "only in your current directory.");
}

void editor_help_open() {
    dialog(
        "With the Open button you can open a level file in the editor.",
        "A dialog will appear with a list of the level files (all files with .LEV extension) in",
        "the current directory. You can pick the desired file with one mouse click on the name of",
        "the file or by pressing ENTER when the name of the file is highlighted.",
        "You can use the two long buttons at the top and the bottom to go up and down in the list.",
        "You can use the Up, Down, PageUp, PageDown and Esc keys also."
        "",
        "If you want to open a new file with a new name, you should choose the New button instead",
        "of the Open button, then use the Save As button to give a name to your file.");
}

void editor_help_new() {
    dialog("With the New button you can start to edit a new level file in the editor.");
}

void editor_help_save_and_play() {
    dialog(
        "With the Save and Play button you can try out the level that you are currently editing.",
        "You can press the P shortcut key also to try your level.",
        "The content of the editor will be saved to disk before you can play.");
}

void editor_help_zoom_fill() {
    dialog("With the Zoom Fill button you can make the view window fit the entire content of",
           "the editor.");
}

void editor_help_check_topology() {
    dialog("With the Check Topology button you can check the level you are editing for any",
           "basic errors. Before you want to try a level, you may want to check it first.",
           "When check finds the first error, it will display a dialog box with an error message,",
           "then it will zoom in onto the exact location of the error.",
           "Until you correct all errors you cannot play on the level,"
           "though you can save it to a file.");
}

void editor_help_properties() {
    dialog("With the Properties button you can set four level properties:",
           "the background texture,", "the foreground texture,",
           "the name of the level (not the file name) and the designer,", "the LGR file name.");
}

void editor_help_view_options() {
    dialog("With the View Options button you can chooses what type of things should be displayed",
           "and what type of things should be hidden in the editor window.");
}

void editor_help_move() {
    dialog(
        "With the Move tool you can move vertices, objects and pictures",
        "Click the left mouse button near a vertex or the center of an object to grab it. If you "
        "want",
        "to grab a picture (a rectangle in the editor), move the cursor to the upper-left vertex.",
        "Move the cursor to the new location you want to move it, and click the left mouse button.",
        "You can press ESC or click the right mouse button to cancel the operation.", "",
        "With the right mouse button you can set the properties of polygons, food-objects and",
        "pictures.", "You cannot set the properties of the Start, Exit and Killer objects.");
}

void editor_help_zoom_in() {
    dialog("With the Zoom In tool you can zoom the view window into any part of the window",
           "you see at the moment. Click the left mouse button to place the first corner of",
           "the window, then click again to place the second corner.",
           "You can press ESC or click the right mouse button to cancel the operation.");
}

void editor_help_create_vertex() {
    dialog(
        "With the Create Vertex tool you can create new vertices. If you click the left",
        "mouse button near an existing vertex, you will add vertices to an existing polygon.",
        "If you click far from any vertices, you will create the first vertex of a new polygon.",
        "If you want to add to an existing polygon, but find that when you click near a vertex",
        "you would like to add vertices to the opposite way from this point, you should press",
        "SPACE. Try this function to see exactly how it works. Similarly you can press ENTER to",
        "change the direction you are adding the vertices. Please take some moments to",
        "experiment with these two methods, because it is important to understand the way",
        "they work.", "You should press ESC or click the right mouse button when you have finished",
        "adding vertices. In this case the vertex currently grabbed will be deleted.", "",
        "To put GRASS to the top of the ground, you have to create separate grass polygons. You",
        "can change a normal polygon into a grass polygon by clicking near a vertex with the Move",
        "tool.",
        "A grass polygon always has an inactive line, the one which is longest in the x direction.",
        "The other lines determine the lower border of the grass (the upper border is determined",
        "by the normal polygons).");
}

void editor_help_delete_vertex() {
    dialog("With the Delete Vertex tool you can delete vertices. Just click the left mouse",
           "button near a vertex and it will disappear. You cannot delete any vertices of a",
           "polygon if it has only three vertices. To delete the entire polygon, use",
           "the Delete Polygon tool.");
}

void editor_help_delete_polygon() {
    dialog("With the Delete Polygon tool you can delete polygons. Just click the left mouse",
           "button near a vertex of a polygon and the polygon will disappear. You cannot",
           "delete all the polygons, at least one polygon must exist at any given time in",
           "the editor.");
}

void editor_help_create_food() {
    dialog("With the Create Food tool you can create new food objects. Just click the left",
           "mouse button to place a new food object. You can see an 'F' inside the food objects.",
           "", "To set the gravity property of a food object, use the move tool",
           "and click with the right mouse button!");
}

void editor_help_create_killer() {
    dialog(
        "With the Create Killer tool you can create new killer objects. Just click the left",
        "mouse button to place a new killer object. You can see a 'K' inside the killer objects.");
}

void editor_help_delete_object() {
    dialog(
        "With the Delete Object tool you can delete food and killer objects. Just click the left",
        "mouse button near the center of an object and it will disappear.",
        "You can only delete the food and killer objects, which have either an 'F' or 'K' inside.",
        "You cannot delete the start and exit objects, which has an 'S' or 'E' inside.");
}

void editor_help_create_sprite() {
    dialog(
        "With the Create Picture tool you can create new pictures. Just click the left",
        "mouse button where you want to place the selected picture.", "",
        "But first you must select a picture by clicking the right mouse button anywhere above",
        "the working area. A dialog will appear with three fields: normal picture, mask and",
        "texture. You can either select a normal picture, or select a mask with a texture.",
        "If you select a mask or a texture alone, you won't be able to put it into your level.",
        "In most cases you will select a normal picture and don't bother with Mask or Textures.",
        "",
        "To set the properties of a picture, use the Move tool and right-click near the left-top",
        "corner of a picture. The distance property determines which pictures hide the others.",
        "The clipping property has three states:",
        "S-Sky: Only those parts of the picture are drawn that are in the sky.",
        "G-Ground: Only those parts of the picture are drawn that are in the ground.",
        "U-Unclipped.");
}

void editor_help_delete_sprite() {
    dialog("With the Delete Picture tool you can delete pictures. Just click the left",
           "mouse button near the up-left corner of a picture and it will disappear.");
}
