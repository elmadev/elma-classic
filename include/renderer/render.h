#ifndef RENDER_H
#define RENDER_H

#include "physics/init.h"
#include "pic/lgr.h"
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

enum class GameLoop { Game, Replay, Render };
void render_game(double time, driver& driv1, driver& driv2, camera& current_camera, GameLoop loop);

void render_bike(pic8* dest, bool has_flag, vect2 bottomleft_corner, const motorst* mot,
                 const bike_metadata* metadata, const bike_pics* bike, const pic8* shirt);
#endif
