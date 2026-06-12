#ifndef RENDER_H
#define RENDER_H

class pic8;
struct bike_metadata;
struct camera;
struct driver;

// Boundary of the screen with a slight buffer - used to render affine pics
extern double AffinePicScreenLeft, AffinePicScreenRight, AffinePicScreenBottom, AffinePicScreenTop;

extern int GameViewWidth, GameViewHeight;

void init_renderer();
void reset_game_background();

void increase_view_size();
void decrease_view_size();

void render_game(double time, driver& driv1, driver& driv2, camera& current_camera);

#endif
