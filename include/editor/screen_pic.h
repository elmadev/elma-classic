#ifndef SCREEN_PIC_H
#define SCREEN_PIC_H

#include <memory>

class pic8;

class screen_pic {
  public:
    enum class Mode {
        OutsideEditor,
        EditorGui,
        EditorCanvas,
    };

    screen_pic(pic8* initial, Mode mode);
    void blit_to_screen();
    pic8* pic() const { return pic_.get(); };
    void reset();

  private:
    static constexpr int MIN_WIDTH = 640;
    static constexpr int MIN_HEIGHT = 480;

    pic8* initial_;
    std::unique_ptr<pic8> pic_;
    Mode mode_;
};

#endif
