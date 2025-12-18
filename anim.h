#ifndef ANIM_H
#define ANIM_H

class pic8;

constexpr int ANIM_MAX_FRAMES = 1000;
constexpr int ANIM_WIDTH = 40;

class anim {
  public:
    int frame_count;
    pic8* frames[ANIM_MAX_FRAMES];

    anim(pic8* source_sheet, const char* error_filename, double scale = 1.0);
    ~anim();
    pic8* get_frame_by_time(double time);
    pic8* get_frame_by_index(int index);

    void make_helmet_top();
};

#endif
