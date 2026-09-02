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

// Defined in render.cpp
extern int MinimapWidth;
extern int MinimapHeight;
extern int MinimapX;
extern int MinimapDx;

void render_minimap(bool player1, pic8* pic, double camera_turn_phase, vect2 bike_center,
                    motorst* other_motor);
void render_bike(pic8* pic, bool has_flag, vect2 bottomleft_corner, const motorst* mot,
                 const bike_metadata* metadata, const bike_pics* bike, const pic8* shirt);

#endif
