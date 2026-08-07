#include "game/fps.h"
#include "platform/implementation.h"
#include <format>

namespace {

constexpr long long FIRST_UPDATE_INTERVAL = 100;
constexpr long long UPDATE_INTERVAL = 1000;

int frame_count;
int update_count;
int render_count;
double prev_fps;
double prev_ups;
double prev_gpu;
long long prev_time;

std::string format_value(double value) {
    if (value == 0.0) {
        return "";
    }
    return std::format("{:.0f}", value);
}

} // namespace

namespace fps {

std::string format_fps() { return format_value(prev_fps); }

std::string format_ups() { return format_value(prev_ups); }

double gpu() { return prev_gpu; }

void reset() {
    frame_count = 0;
    update_count = 0;
    render_count = 0;
    prev_fps = 0.0;
    prev_ups = 0.0;
    prev_gpu = 0.0;
    prev_time = get_milliseconds();
}

static void calculate() {
    long long update_interval = UPDATE_INTERVAL;
    if (prev_fps == 0.0) {
        update_interval = FIRST_UPDATE_INTERVAL;
    }

    long long current_time = get_milliseconds();
    long long dt = current_time - prev_time;

    if (dt >= update_interval) {
        prev_fps = 1000.0 * frame_count / dt;
        prev_ups = 1000.0 * update_count / dt;
        prev_gpu = 1000.0 * render_count / dt;
        frame_count = 0;
        update_count = 0;
        render_count = 0;
        prev_time = current_time;
    }
}

void count_fps() { frame_count++; }

void count_ups() { update_count++; }

void count_gpu() {
    render_count++;

    calculate();
}

} // namespace fps
