#ifndef ABC8_H
#define ABC8_H

class pic8;
typedef pic8* ptrpic8;

extern bool ErrorOnMissingCodepoint;

class abc8 {
  private:
    ptrpic8* ppsprite;
    short* y_offset;
    int spacing;

  public:
    abc8(const char* filename);
    ~abc8();
    void write(pic8* dest, int x, int y, const char* text);
    void write_centered(pic8* dest, int x, int y, const char* text);
    int len(const char* text);
    void set_spacing(int new_spacing);
};

#endif
