
typedef pic8* ptrpic8;

class abc8 {
  private:
    ptrpic8* ppsprite;
    short* y_offset;
    int spacing;

  public:
    abc8(const char* filename);
    ~abc8(void);
    void write(pic8* dest, int x, int y, const char* text);
    void write_centered(pic8* dest, int x, int y, const char* text);
    int len(const char* text);
    void set_spacing(int new_spacing);
};
