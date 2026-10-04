#ifndef GAME_VIDEO_EXPORT_H
#define GAME_VIDEO_EXPORT_H

class pic8;

// Sink for frames produced while VideoRecordingMode is set. When no encoder is
// installed, frames are written as individual .pcx files instead.
class video_encoder {
  public:
    virtual ~video_encoder() = default;

    // The frame is the paletted backbuffer, upside-down, as it is handed to
    // handle_screenshot(). The palette is 256 RGB triplets.
    virtual void write_frame(pic8& frame, const unsigned char* palette) = 0;

    // Pull sample_count mono samples from the sound mixer and write them.
    virtual void write_audio(int sample_count) = 0;

    // True once the encoder can no longer accept frames, so that rendering can
    // stop instead of running the replay out for nothing.
    virtual bool failed() const = 0;
};

extern video_encoder* VideoEncoder;

#endif
