#include "game/fps.h"
#include "platform/implementation.h"
#include <cmath>
#include <format>

namespace {

constexpr long long FIRST_UPDATE_INTERVAL = 50;
constexpr long long UPDATE_INTERVAL = 1000;

int frame_count;
int update_count;
double prev_fps;
double prev_ups;
long long prev_time;

} // namespace

namespace fps {

void reset() {
    frame_count = 0;
    update_count = 0;
    prev_fps = 0.0;
    prev_ups = 0.0;
    prev_time = get_milliseconds();
}

void physics_frame() { update_count++; }

void render_frame() {
    frame_count++;

    long long update_interval = UPDATE_INTERVAL;
    if (prev_fps == 0.0) {
        update_interval = FIRST_UPDATE_INTERVAL;
    }

    long long current_time = get_milliseconds();
    long long dt = current_time - prev_time;

    if (dt >= update_interval) {
        prev_fps = 1000.0 * frame_count / dt;
        prev_ups = 1000.0 * update_count / dt;
        frame_count = 0;
        update_count = 0;
        prev_time = current_time;
    }
}

std::string value(bool show_ups) {
    if (prev_fps == 0.0) {
        return "";
    }
    if (show_ups) {
        return std::format("{:.1f}/{:.1f}", prev_fps, prev_ups);
    }
    return std::format("{:.1f}", prev_fps);
}

} // namespace fps
