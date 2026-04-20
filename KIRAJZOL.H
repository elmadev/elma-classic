#ifndef KIRAJZOL_H
#define KIRAJZOL_H

class pic8;
struct valtozok;
struct camera;

extern int Voltkilogas; // Csak tesztelesre

// Boundary of the screen with a slight buffer - used to render affine pics
extern double AffinePicScreenLeft, AffinePicScreenRight, AffinePicScreenBottom, AffinePicScreenTop;

extern int GameViewWidth, GameViewHeight;

extern int GameBackgroundRender;
extern int GAME_BACKGROUND_RENDER_SET_VALUE;

void init_renderer();

void increase_view_size();
void decrease_view_size();

void render_game(double time, valtozok* metadata1, valtozok* metadata2, int show_minimap1,
                 int show_timer1, int show_minimap2, int show_timer2, camera& current_camera);

#endif
