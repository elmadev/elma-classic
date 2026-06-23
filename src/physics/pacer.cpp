#include "physics/pacer.h"
#include "eol/settings.h"
#include "eol/status_messages.h"
#include "game/fps.h"
#include "main.h"
#include "platform/implementation.h"
#include <format>

namespace pacer {

namespace {

constexpr long long LIMITER_TIMEOUT_MS = 33;

bool FpsLimitEnabled = false;
int FpsLimit = 0;
bool FpsBoost = false;

// In physics units of time
double time = 0.0;
double target_time = 0.0;

// In milliseconds
long long start_time = 0;
long long last_real_frame_time = 0;
long long real_frame_count = 0;

} // namespace

std::string format_fps_limit() {
    bool enabled = pacer::FpsLimitEnabled;
    int limit = enabled ? pacer::FpsLimit : 0;
    bool next_enabled = EolSettings->fps_limit_enabled();
    int next_limit = next_enabled ? EolSettings->fps_limit() : 0;
    auto format_limit = [](int limit) -> std::string {
        if (limit == 0) {
            return "off";
        }
        return std::to_string(limit);
    };
    if (enabled || next_enabled) {
        if (limit != next_limit) {
            return std::format(" ({} -> {})", format_limit(limit), format_limit(next_limit));
        }
        return std::format(" ({})", format_limit(limit));
    }
    return "";
}

void request_fps_limit(bool enabled, int limit) {
    if (!EolSettings->show_fps()) {
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
    }
    EolSettings->set_fps_limit_enabled(enabled);
    EolSettings->set_fps_limit(limit);
}

void reset() {
    time = 0.0;
    target_time = 0.0;

    start_time = get_milliseconds();
    last_real_frame_time = start_time;
    real_frame_count = 0;

    FpsLimitEnabled = EolSettings->fps_limit_enabled();
    FpsLimit = EolSettings->fps_limit();
    FpsBoost = EolSettings->fps_boost();
}

void new_frame() {
    long long now = get_milliseconds();
    long long elapsed = now - start_time;

    if (FpsLimitEnabled) {
        // In milliunits (a value of 1000 corresponds to one allowed frame)
        long long max_allowed_frames = elapsed * FpsLimit;

        // eol-client forces a real frame at least every 33 ms even when over the limit
        if (real_frame_count * 1000LL > max_allowed_frames &&
            now - last_real_frame_time <= LIMITER_TIMEOUT_MS) {
            // Skip current frame
            return;
        }
    }

    real_frame_count++;
    last_real_frame_time = now;
    fps::count_fps();

    target_time = std::max(elapsed * MILLISECONDS_TO_PHYS_TIME, 0.000001);
}

bool subframe(double* out_dt) {
    double dt = target_time - time;
    if (0.000001 > dt) {
        return false;
    }

    // Minimum UPS is 79.4, or 1000.0 if booster is enabled
    const bool boost = FpsBoost && !FpsLimitEnabled;
    const double max_timestep = boost ? MILLISECONDS_TO_PHYS_TIME : PHYS_MAX_TIMESTEP;
    dt = std::min(dt, max_timestep);
    *out_dt = dt;
    time += dt;
    return true;
}

} // namespace pacer
