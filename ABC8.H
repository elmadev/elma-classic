
typedef pic8* ptrpic8;

class abc8 {
  private:
    ptrpic8* ppsprite;
    short* y_offset;
    int default_spacing;

  public:
    abc8(char* filename);
    ~abc8(void);
    void write(pic8* dest, int x, int y, char* text);
    void write_centered(pic8* dest, int x, int y, char* text);
    int len(char* text);
    void set_spacing(int spacing);
};
