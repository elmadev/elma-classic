#ifndef GAME_FPS_H
#define GAME_FPS_H

#include <string>

namespace fps {

void reset();

void count_fps();
void count_ups();

void update();

std::string format_fps();
std::string format_ups();

} // namespace fps

#endif
