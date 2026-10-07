#ifndef RENDERER_RENDER_H
#define RENDERER_RENDER_H

#include <vector>

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
void render_game(double time, const std::vector<driver*>& drivers, camera& current_camera,
                 GameLoop loop);

void level_to_bmp(const char* filename);

#endif
