#ifndef GAME_FPS_H
#define GAME_FPS_H

#include <string>

namespace fps {

void reset();

void count_fps();
void count_ups();
void count_gpu();

std::string format_fps();
std::string format_ups();
std::string format_gpu();

} // namespace fps

#endif
