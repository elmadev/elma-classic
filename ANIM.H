
#define MAXANIMFRAME (1000)
extern const int Animsize;

class anim {
  public:
    int frameszam;
    pic8* ppictomb[MAXANIMFRAME];

    // Meg is forditja kepet:
    anim(pic8* nagykep, char* uzenet_filenev);
    ~anim(void);
    pic8* getframe(double ido);
    pic8* getframebyindex(int index);

    // Menu sisakjanal egy kis piros vonalat odahuz fole:
    void korrigal(void);
};
