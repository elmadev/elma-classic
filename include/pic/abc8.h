#ifndef ABC8_H
#define ABC8_H

class pic8;
typedef pic8* ptrpic8;

class abc8 {
  private:
    ptrpic8* ppsprite;
    short* y_offset;
    int spacing;
    int line_height_;

  public:
    abc8(const char* filename);
    ~abc8();
    void write(pic8* dest, int x, int y, const char* text);
    void write_centered(pic8* dest, int x, int y, const char* text);
    void write_right_align(pic8* dest, int x, int y, const char* text);
    int len(const char* text);
    void set_spacing(int new_spacing);
    bool has_char(unsigned char c) const;
#ifdef DEBUG
    int line_height() const;
#else
    int line_height() const { return line_height_; }
#endif
    void set_line_height(int height) { line_height_ = height; }
};

#endif
