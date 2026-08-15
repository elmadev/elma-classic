#include "eol/kuski.h"
#include <algorithm>
#define _USE_MATH_DEFINES
#include <cmath>

#ifndef M_PI
#define M_PI 3.141592653589793
#endif

void kuski::clear_apple_data() {
    for (bool& taken : apples_taken) {
        taken = false;
    }
}

static uint32_t mask24(uint32_t val) { return val & 0xFFFFFF; }

// val1 and val2 are monotonically increasing values that wrap around at 2^bits
// compare these two values, handling the possibility of this wraparound:
// -1: val1 < val2, 0: val1 == val2, 1: val1 > val2
static int compare_wrapping(uint32_t val1, uint32_t val2, int bits) {
    const uint32_t max_diff = 1u << (bits - 1);

    if ((val1 < val2 && val2 - val1 < max_diff) || (val1 > val2 && val1 - val2 >= max_diff)) {
        return -1;
    }
    if ((val1 < val2 && val2 - val1 >= max_diff) || (val1 > val2 && val1 - val2 < max_diff)) {
        return 1;
    }
    return 0;
}

// Interpolate along the shorter arc.
// alpha is the interpolation value between 0.0 and 1.0 from angle1 to angle2
static double interpolate_angle(double alpha, double angle1, double angle2) {
    if (angle1 - angle2 > M_PI) {
        return (1 - alpha) * (angle1 - 2 * M_PI) + alpha * angle2;
    }
    if (angle2 - angle1 > M_PI) {
        return (1 - alpha) * angle1 + alpha * (angle2 - 2 * M_PI);
    }
    return (1 - alpha) * angle1 + alpha * angle2;
}

// Interpolating wheel/head positions linearly makes them drift off a rotating
// bike, so go through polar coordinates around the bike center instead.
static vect2 interpolate_around_center(const vect2& p1, const vect2& p2, double alpha,
                                       const vect2& center1, const vect2& center2) {
    const double radius1 = (p1 - center1).length();
    const double radius2 = (p2 - center2).length();

    double a1 = 0.0;
    if (radius1 > 0.0) {
        a1 = acos((p1.x - center1.x) / radius1);
        if (p1.y - center1.y < 0) {
            a1 = 2 * M_PI - a1;
        }
    }

    double a2 = 0.0;
    if (radius2 > 0.0) {
        a2 = acos((p2.x - center2.x) / radius2);
        if (p2.y - center2.y < 0) {
            a2 = 2 * M_PI - a2;
        }
    }

    const double a = interpolate_angle(alpha, a1, a2);
    const double radius = (1 - alpha) * radius1 + alpha * radius2;
    const vect2 center = (1 - alpha) * center1 + alpha * center2;
    return center + vect2(radius * cos(a), radius * sin(a));
}

// Only blend while the discrete state behind the phase (volt direction, bike
// flip) matches; otherwise snap. Also keep the result off exactly 0.5, which
// sits right on sign flips in render_bike (the wheel z-order, for one).
static double interpolate_anim_phase(double alpha, double phase1, double phase2,
                                     bool discrete_matches) {
    double phase =
        discrete_matches ? (1 - alpha) * phase1 + alpha * phase2 : (alpha < 0.5 ? phase1 : phase2);
    if (fabs(phase - 0.5) < 1.0 / 256) {
        phase = 0.5 + 1.0 / 256;
    }
    return phase;
}

static spy_data interpolate_pose(const spy_data& d1, const spy_data& d2, double alpha) {
    spy_data out = d1;

    out.mot.bike.r = (1 - alpha) * d1.mot.bike.r + alpha * d2.mot.bike.r;
    out.mot.bike.rotation = interpolate_angle(alpha, d1.mot.bike.rotation, d2.mot.bike.rotation);

    out.mot.left_wheel.r = interpolate_around_center(d1.mot.left_wheel.r, d2.mot.left_wheel.r,
                                                     alpha, d1.mot.bike.r, d2.mot.bike.r);
    out.mot.left_wheel.rotation =
        interpolate_angle(alpha, d1.mot.left_wheel.rotation, d2.mot.left_wheel.rotation);
    out.mot.right_wheel.r = interpolate_around_center(d1.mot.right_wheel.r, d2.mot.right_wheel.r,
                                                      alpha, d1.mot.bike.r, d2.mot.bike.r);
    out.mot.right_wheel.rotation =
        interpolate_angle(alpha, d1.mot.right_wheel.rotation, d2.mot.right_wheel.rotation);
    out.mot.head_r = interpolate_around_center(d1.mot.head_r, d2.mot.head_r, alpha, d1.mot.bike.r,
                                               d2.mot.bike.r);
    out.mot.body_r = interpolate_around_center(d1.mot.body_r, d2.mot.body_r, alpha, d1.mot.bike.r,
                                               d2.mot.bike.r);

    out.mot.flipped_bike = alpha < 0.5 ? d1.mot.flipped_bike : d2.mot.flipped_bike;
    out.metadata.volt_is_right =
        alpha < 0.5 ? d1.metadata.volt_is_right : d2.metadata.volt_is_right;

    out.metadata.arm_position =
        interpolate_anim_phase(alpha, d1.metadata.arm_position, d2.metadata.arm_position,
                               d1.metadata.volt_is_right == d2.metadata.volt_is_right);
    out.metadata.bike_turning.turn_phase = interpolate_anim_phase(
        alpha, d1.metadata.bike_turning.turn_phase, d2.metadata.bike_turning.turn_phase,
        d1.mot.flipped_bike == d2.mot.flipped_bike);

    return out;
}

// Returns the position just after the newest non-stop frame at or before
// time, or frames.end() if there is none. offset shifts each frame's
// time onto the same clock as time.
std::deque<spy_playback::spy_frame>::iterator spy_playback::frame_after_time(uint32_t time,
                                                                             uint32_t offset) {
    for (auto pos = frames.begin(); pos != frames.end(); ++pos) {
        if (pos->stop) {
            return pos;
        }
        if (compare_wrapping(time, mask24(pos->data.time + offset), 24) == -1) {
            return pos;
        }
    }
    return frames.end();
}

const struct spy_data* spy_playback::spy_data() const { return data ? &*data : nullptr; }

void spy_playback::add(const struct spy_data& sd, int min_spy_frames) {
    if (min_spy_frames == 0) {
        return;
    }

    if (!frames.empty()) {
        const spy_frame& newest = frames.back();

        // Stale or reordered packet from an earlier run.
        if (compare_wrapping(sd.run_id, newest.data.run_id, 8) == -1) {
            return;
        }

        // The run (re)started with no stop in between.
        // Rebuffer to ensure we fill min_spy_frames before
        // playback starts; the shown pose stays up meanwhile.
        const bool new_run = sd.run_id != newest.data.run_id;
        const bool run_clock_toggled = !newest.stop && (newest.data.time == 0) != (sd.time == 0);
        if (new_run || run_clock_toggled) {
            rebuffer();
        }
    }

    auto pos = frame_after_time(sd.time, 0);
    frames.insert(pos, spy_frame{.data = sd});
}

void spy_playback::stop() {
    if (frames.empty()) {
        clear();
        return;
    }

    spy_frame stop{.data = frames.back().data, .stop = true};
    stop.data.time = 0;
    frames.push_back(stop);
}

void spy_playback::rebuffer() {
    frames.clear();
    running = false;
    time_offset = 0;
}

void spy_playback::clear() {
    rebuffer();
    data.reset();
}

void spy_playback::update(uint32_t now_ms, int min_spy_frames) {
    uint32_t now = mask24(now_ms);

    if (!frames.empty() && frames.back().stop && !running) {
        // Stopped before playback even started
        clear();
        return;
    }

    // A kuski waiting at the start during countdown battles keeps streaming
    // frames stamped with time 0. There's nothing to interpolate;
    // just show the newest pose with the run clock at zero.
    if (!frames.empty() && !frames.back().stop && frames.back().data.time == 0) {
        data = frames.back().data;
        frames.erase(frames.begin(), frames.end() - 1);
        return;
    }

    if (running) {
        // Advance playback: everything before the frame now falls on is spent.
        auto pos = frame_after_time(now, time_offset);
        if (pos == frames.begin()) {
            // Even the oldest frame is ahead of the playback clock.
            // This should never happen under normal situations; it takes
            // a stall long enough (hours) to flip the wrapping compares.
            clear();
            return;
        }
        frames.erase(frames.begin(), pos - 1);
    } else if (frames.size() >= static_cast<size_t>(min_spy_frames)) {
        // Enough buffered so we can start playing, clocked from the oldest frame.
        time_offset = mask24(now - frames.front().data.time);
        running = true;
    }

    if (!running) {
        return;
    }

    const spy_frame& frame1 = frames.front();
    const spy_frame& frame2 = frames.size() > 1 ? frames[1] : frame1;

    // Playback hit a stop marker, or frame2 is so old that fresh frames
    // should long since have arrived. The run is either over or the feed
    // died; hide the kuski.
    constexpr uint32_t STALE_FRAME_MS = 3000;
    if (frame1.stop || frame2.stop ||
        compare_wrapping(now, mask24(frame2.data.time + time_offset + STALE_FRAME_MS), 24) == 1) {
        clear();
        return;
    }

    // Calculating `alpha` needs non-wrapped time values.
    const uint32_t time1 = mask24(frame1.data.time + time_offset);
    uint32_t time2 = mask24(frame2.data.time + time_offset);
    if (time2 < time1) {
        // `frame2` comes after `frame1`, so a lower time value means the timer wrapped around, so
        // unwrap it.
        time2 += 1 << 24;
    }
    if (now < time1) {
        // `frame1` is at or before now, so a lower `now` time value means the
        // timer wrapped around, so unwrap it.
        now += 1 << 24;
    }

    double alpha = 0.0;
    if (frame2.data.time != frame1.data.time) {
        alpha = (now - time1) / (double)(time2 - time1);
    }
    alpha = std::clamp(alpha, 0.0, 1.0);

    struct spy_data out = interpolate_pose(frame1.data, frame2.data, alpha);
    out.time = mask24(now - time_offset);
    data = out;
}
