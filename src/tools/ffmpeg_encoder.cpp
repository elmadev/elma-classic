#include "ffmpeg_encoder.h"
#include "pic/pic8.h"
#include "sound/engine.h"
#include <algorithm>
#include <format>

#ifdef _WIN32
#define popen _popen
#define pclose _pclose
constexpr const char PIPE_MODE[] = "wb";
#else
#include <csignal>
constexpr const char PIPE_MODE[] = "w";
#endif

namespace {

// Quote one argument for the shell popen() runs the command through
std::string quote(const std::string& argument) {
#ifdef _WIN32
    // cmd.exe passes the line on verbatim; these are the rules the CRT uses to
    // split it back into argv. Backslashes are only special in front of a quote.
    std::string quoted = "\"";
    size_t backslashes = 0;
    for (char c : argument) {
        if (c == '\\') {
            backslashes++;
            continue;
        }
        quoted.append(c == '"' ? backslashes * 2 + 1 : backslashes, '\\');
        backslashes = 0;
        quoted.push_back(c);
    }
    quoted.append(backslashes * 2, '\\');
    quoted.push_back('"');
    return quoted;
#else
    std::string quoted = "'";
    for (char c : argument) {
        if (c == '\'') {
            quoted += "'\\''";
        } else {
            quoted.push_back(c);
        }
    }
    quoted.push_back('\'');
    return quoted;
#endif
}

} // namespace

FILE* ffmpeg_start(const std::vector<std::string>& args) {
    std::string command = "ffmpeg";
    for (const std::string& argument : args) {
        command.push_back(' ');
        command += quote(argument);
    }

#ifndef _WIN32
    // A dead ffmpeg must fail our writes rather than kill us
    signal(SIGPIPE, SIG_IGN);
#endif

    return popen(command.c_str(), PIPE_MODE);
}

int ffmpeg_wait(FILE* pipe) {
    int status = pclose(pipe);
#ifndef _WIN32
    if (status == -1 || !WIFEXITED(status)) {
        return -1;
    }
    status = WEXITSTATUS(status);
#endif
    return status;
}

bool ffmpeg_not_found(int status) { return status == 127 || status == 9009; }

ffmpeg_encoder::ffmpeg_encoder(int width, int height, int fps, const std::string& video_path,
                               const std::vector<std::string>& extra_args,
                               const std::string& audio_path, int total_frames)
    : width(width),
      height(height),
      total_frames(total_frames) {
    std::vector<std::string> args = {
        "-y",
        "-hide_banner",
        "-loglevel",
        "error",
        "-f",
        "rawvideo",
        "-pixel_format",
        "rgb24",
        "-video_size",
        std::format("{}x{}", width, height),
        "-framerate",
        std::to_string(fps),
        "-i",
        "-",
        // Defaults first: ffmpeg lets a later occurrence of an output option win,
        // so anything the user passed after -- overrides these.
        "-c:v",
        "libx264",
        "-pix_fmt",
        "yuv420p",
        "-crf",
        "18",
    };
    args.insert(args.end(), extra_args.begin(), extra_args.end());
    args.push_back(video_path);

    ffmpeg = ffmpeg_start(args);
    if (!ffmpeg) {
        fail("Failed to start ffmpeg");
        return;
    }

    if (!audio_path.empty()) {
        audio = fopen(audio_path.c_str(), "wb");
        if (!audio) {
            fail("Failed to open temporary audio file: " + audio_path);
            return;
        }
    }

    rgb.resize((size_t)width * height * 3);
}

ffmpeg_encoder::~ffmpeg_encoder() {
    if (audio) {
        fclose(audio);
    }
    if (ffmpeg) {
        ffmpeg_wait(ffmpeg);
    }
}

void ffmpeg_encoder::fail(const std::string& message) {
    if (error_.empty()) {
        error_ = message;
    }
}

void ffmpeg_encoder::write_frame(pic8& frame, const unsigned char* palette) {
    if (!error_.empty()) {
        return;
    }

    if (frame.get_width() != width || frame.get_height() != height) {
        fail(std::format("Frame size changed from {}x{} to {}x{} mid-render", width, height,
                         frame.get_width(), frame.get_height()));
        return;
    }

    // The backbuffer is handed to us upside-down.
    frame.vertical_flip();
    unsigned char* out = rgb.data();
    for (int y = 0; y < height; y++) {
        const unsigned char* row = frame.get_row(y);
        for (int x = 0; x < width; x++) {
            const unsigned char* color = &palette[3 * (size_t)row[x]];
            *out++ = color[0];
            *out++ = color[1];
            *out++ = color[2];
        }
    }
    frame.vertical_flip();

    if (fwrite(rgb.data(), 1, rgb.size(), ffmpeg) != rgb.size()) {
        fail("ffmpeg closed the pipe before the render finished");
        return;
    }

    frames_written++;
    if (total_frames > 0) {
        // The total is estimated from the replay length, so keep it honest if
        // the render turns out to be a frame or two longer.
        total_frames = std::max(frames_written, total_frames);
        int percent = (int)((long long)frames_written * 100 / total_frames);
        printf("\rframe %d/%d (%d%%)", frames_written, total_frames, percent);
    } else {
        printf("\rframe %d", frames_written);
    }
    fflush(stdout);
}

void ffmpeg_encoder::write_audio(int sample_count) {
    if (!audio || !error_.empty() || sample_count <= 0) {
        return;
    }

    samples.resize((size_t)sample_count);
    sound_mixer(samples.data(), sample_count);

    if (fwrite(samples.data(), sizeof(short), (size_t)sample_count, audio) !=
        (size_t)sample_count) {
        fail("Failed to write to the temporary audio file");
    }
}

int ffmpeg_encoder::finish() {
    if (audio) {
        fclose(audio);
        audio = nullptr;
    }

    if (frames_written > 0) {
        printf("\n");
    }

    if (!ffmpeg) {
        return -1;
    }
    int status = ffmpeg_wait(ffmpeg);
    ffmpeg = nullptr;
    return status;
}
