#include "physics/pacer.h"
#include "eol/settings.h"
#include "eol/status_messages.h"
#include "game/fps.h"
#include "log.h"
#include "util/util.h"
#include <format>
#include <random>

namespace pacer {

// TEST PARAMS TEST PARAMS TEST PARAMS

// Run the test by setting !fps 30

// Proof that the test passes: At 33, the test never fails.
// Proof that the test is valid: At 32, the test fails because the limiter kicks in
constexpr long long LIMITER_TIMEOUT_MS = 33;

// At 0.3, normal_distribution generates a number most often between 0.0 and 1.0
constexpr double NORMAL_DISTRIBUTION_OMEGA = 0.3;

// Multiplier for OMEGA
// At 0.3 and 60.0, frame lengths will be usually between 0-60 ms
// (i.e. simulate a GPU rate of 16-10000 frames per second)
constexpr double MILLISECONDS_MULTIPLIER = 60.0;

// Test the game at a new GPU framerate distribution every 25 frames
constexpr int GPU_FRAME_RANDOMIZER_INTERVAL = 25;

long long get_milliseconds_rand() {
    // Normal distribution random number generator
    static std::random_device device{};
    static std::mt19937 generator{device()};
    static std::normal_distribution<double> dist{0.0, NORMAL_DISTRIBUTION_OMEGA};

    std::mt19937* generator_ptr = &generator;
    std::normal_distribution<double>* dist_ptr = &dist;
    auto normal_rand = [&dist_ptr, &generator_ptr]() {
        double val = std::abs(dist_ptr->operator()(*generator_ptr));
        return static_cast<int>(val * MILLISECONDS_MULTIPLIER);
    };

    // Main function
    static int count = 0;
    static int min = 0;
    static int max = 1;
    static int range = 1;
    static int elapsed = 0;
    count++;

    // Run a new test every X frames by choosing a random GPU framerate
    if (count > GPU_FRAME_RANDOMIZER_INTERVAL) {
        count = 0;
        min = normal_rand();
        max = normal_rand();
        if (max < min) {
            std::swap(min, max);
        }
        range = max - min + 1;
        LOG_DEBUG("RANGE: {}-{} ms; GPU rate: {:.0f}", min, max, 2000.0 / (min + max));
    }

    // Update time with random GPU framerate
    elapsed += min + util::random::range(range);
    return elapsed;
}

namespace {

bool FpsLimitEnabled = false;
int FpsLimit = 0;

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

    start_time = get_milliseconds_rand();
    last_real_frame_time = start_time;
    real_frame_count = 0;

    FpsLimitEnabled = EolSettings->fps_limit_enabled();
    FpsLimit = EolSettings->fps_limit();
}

void new_frame() {
    long long now = get_milliseconds_rand();
    long long elapsed = now - start_time;

    if (FpsLimitEnabled) {
        // In milliunits (a value of 1000 corresponds to one allowed frame)
        long long max_allowed_frames = elapsed * FpsLimit;

        if (real_frame_count * 1000LL > max_allowed_frames) {
            // Skip current frame
            if (now - last_real_frame_time > LIMITER_TIMEOUT_MS) {
                internal_error("FPS limiter test fail");
            }
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
    dt = std::min(dt, PHYS_MAX_TIMESTEP);
    *out_dt = dt;
    time += dt;
    return true;
}

} // namespace pacer
