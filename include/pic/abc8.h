#ifndef ABC8_H
#define ABC8_H

class pic8;

class abc8 {
  private:
    static constexpr int MAX_CODEPOINTS = 256;

    pic8* sprites[MAX_CODEPOINTS];
    short y_offset[MAX_CODEPOINTS];
    int spacing;
    int line_height_;

  public:
    abc8(const char* filename, int spacing, int line_height);
    ~abc8();
    void write(pic8* dest, int x, int y, const char* text);
    void write_centered(pic8* dest, int x, int y, const char* text);
    void write_right_align(pic8* dest, int x, int y, const char* text);
    int len(const char* text);
    bool has_char(unsigned char c) const;
    int line_height() const { return line_height_; }
};

#endif
