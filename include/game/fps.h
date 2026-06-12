#ifndef GAME_FPS_H
#define GAME_FPS_H

#include <string>

namespace fps {

void reset();
void physics_frame();
void render_frame();
inline std::string header(bool show_ups) { return show_ups ? "FPS/UPS" : "FPS"; }
std::string value(bool show_ups);

}; // namespace fps

#endif
