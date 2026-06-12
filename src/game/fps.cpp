#include "game/fps.h"
#include "platform/implementation.h"

namespace {

constexpr long long FIRST_UPDATE_INTERVAL = 100;
constexpr long long UPDATE_INTERVAL = 1000;

int frame_count;
int update_count;
double prev_fps;
double prev_ups;
long long prev_time;

} // namespace

namespace fps {

double fps() { return prev_fps; }

double ups() { return prev_ups; }

void reset() {
    frame_count = 0;
    update_count = 0;
    prev_fps = 0.0;
    prev_ups = 0.0;
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
        frame_count = 0;
        update_count = 0;
        prev_time = current_time;
    }
}

void count_fps() {
    frame_count++;

    calculate();
}

void count_ups() { update_count++; }

} // namespace fps
