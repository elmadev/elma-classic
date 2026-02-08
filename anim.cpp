#include "anim.h"
#include "main.h"
#include "KIRAJZOL.H"
#include "pic8.h"
#include <cstring>
#include <format>

anim::anim(pic8* source_sheet, const char* error_filename, int target_height, double zoom) {
    frame_count = 0;
    std::memset(frames, 0, sizeof(frames));

    // LGR13 format: target object height must always be ANIM_WIDTH
    if (target_height != ANIM_WIDTH) {
        external_error(std::format("Object picture height must be {}", ANIM_WIDTH).c_str(),
                       error_filename);
    }

    // Get total number of animation frames
    int source_height = source_sheet->get_height();
    int source_width = source_sheet->get_width();
    if (source_width % source_height) {
        external_error("Object picture width must be a multiple of its height", error_filename);
    }
    frame_count = source_width / source_height;
    if (frame_count < 0) {
        internal_error("anim::anim frame_count < 0");
    }
    if (frame_count > ANIM_MAX_FRAMES) {
        external_error(
            std::format("Too many frames in picture! Max frame is {}!", ANIM_MAX_FRAMES).c_str(),
            error_filename);
    }

    // Split the source picture into individual frames
    unsigned char transparency = source_sheet->gpixel(0, 0);
    for (int i = 0; i < frame_count; i++) {
        frames[i] = new pic8(source_height, source_height);
        blit8(frames[i], source_sheet, -source_height * i, 0);
        frames[i] = pic8::resize(frames[i], (int)(target_height * zoom));
        forditkepet(frames[i]);
        frames[i]->add_transparency(transparency);
    }
}

anim::~anim() {
    frame_count = 0;
    for (int i = 0; i < ANIM_MAX_FRAMES; i++) {
        if (frames[i]) {
            delete frames[i];
            frames[i] = nullptr;
        }
    }
}

constexpr double FrameTimestep = 0.014;

// Input time is milliseconds*STOPWATCH_MULTIPLIER*0.0024
// Therefore framerate is 31.2 fps?
pic8* anim::get_frame_by_time(double time) {
    int step = (int)(time / FrameTimestep);
    int i = step % frame_count;
    return frames[i];
}

pic8* anim::get_frame_by_index(int index) {
    if (index < 0 || index >= frame_count) {
        internal_error("get_frame_by_index out of range");
    }
    return frames[index];
}

// The red helmet is actually 41 pixels high, but the class only makes 40x40 images.
// Let's add the top row of the helmet to each frame
void anim::make_helmet_top() {
    for (int i = 0; i < frame_count; i++) {
        // Paste the old picture into a new pic of 40x41
        pic8* new_frame = new pic8(ANIM_WIDTH, ANIM_WIDTH + 1);
        unsigned char transparency = frames[i]->gpixel(0, 0);
        new_frame->fill_box(transparency);
        blit8(new_frame, frames[i], 0, 1);

        // Find the first solid pixel in row 1 located at (x1, 1)
        // Get the color at (x1 + 3, 1)
        // Copy this color in row 0 from x1 + 4 to the right border
        for (int x1 = 0; x1 < ANIM_WIDTH; x1++) {
            if (new_frame->gpixel(x1, 1) != transparency) {
                for (int x2 = x1 + 4; x2 < ANIM_WIDTH; x2++) {
                    new_frame->ppixel(x2, 0, new_frame->gpixel(x1 + 3, 1));
                }
                break;
            }
        }
        // Lastly, trim 3 pixels off the right side of row 0
        for (int x1 = ANIM_WIDTH - 1; x1 > 0; x1--) {
            if (new_frame->gpixel(x1, 1) != transparency) {
                for (int x2 = x1 - 3; x2 < ANIM_WIDTH; x2++) {
                    new_frame->ppixel(x2, 0, transparency);
                }
                break;
            }
        }

        // Regenerate transparency data
        new_frame->add_transparency();

        delete frames[i];
        frames[i] = new_frame;
    }
}
