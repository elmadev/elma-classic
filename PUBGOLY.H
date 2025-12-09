
typedef struct {
    vekt2 r,
        // relozo,
        rmostani, v;
    double alfa, alfamostani, omega;
    double sug;
    double t;
} golyo;

void initgolyo(void);

extern int Nincsfal; // Ha ez igaz, golyok kimehetnek falon kivulre
void leptetgolyo(double stept);
// void ujraszamol( void );
// void idoreigazitmindet( void );

// Ezt init tolti ki, innen lehet kiolvasni hol vannak golyok:
extern int Ngolyok;
extern golyo* Golyok;
