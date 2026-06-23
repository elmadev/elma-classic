#include "physics/pacer.h"
#include "eol/settings.h"
#include "eol/status_messages.h"
#include "game/fps.h"
#include "main.h"
#include "platform/implementation.h"
#include <format>

namespace pacer {

namespace {

bool FpsLimitEnabled = false;
int FpsLimit = 0;

// In physics units of time
double time = 0.0;
double target_time = 0.0;

// In milliseconds
long long start_time = 0;
long long real_frame_count = 0;

} // namespace

void request_fps_limit(bool enabled, int limit) {
    bool enabled_changed =
        enabled != FpsLimitEnabled || enabled != EolSettings->fps_limit_enabled();
    if (!enabled) {
        if (enabled_changed) {
            StatusMessages->add("Turning FPS limiter off when the next run starts");
        } else {
            StatusMessages->add("FPS limiter is already off");
        }
    } else {
        bool limit_changed = limit != FpsLimit || limit != EolSettings->fps_limit();
        if (enabled_changed || limit_changed) {
            StatusMessages->add(
                std::format("Setting FPS limiter to {} when the next run starts", limit));
        } else {
            StatusMessages->add(std::format("FPS is already limited to {}", limit));
        }
    }
    EolSettings->set_fps_limit_enabled(enabled);
    EolSettings->set_fps_limit(limit);
}

void reset() {
    time = 0.0;
    target_time = 0.0;

    start_time = get_milliseconds();
    real_frame_count = 0;

    FpsLimitEnabled = EolSettings->fps_limit_enabled();
    FpsLimit = EolSettings->fps_limit();
}

void new_frame() {
    long long now = get_milliseconds();
    long long elapsed = now - start_time;

    if (FpsLimitEnabled) {
        // In milliunits (a value of 1000 corresponds to one allowed frame)
        long long max_allowed_frames = elapsed * FpsLimit;

        if (real_frame_count * 1000LL > max_allowed_frames) {
            // Skip current frame
            return;
        }
    }

    real_frame_count++;
    fps::count_fps();

    target_time = elapsed * MILLISECONDS_TO_PHYS_TIME;
    target_time = std::max(0.000001, target_time);
}

bool subframe(double* out_dt) {
    double dt = target_time - time;
    if (0.000001 > dt) {
        return false;
    }
    dt = std::min(dt, PHYS_MAX_TIMESTEP);
    *out_dt = dt;
    time += dt;
    return true;
}

} // namespace pacer
