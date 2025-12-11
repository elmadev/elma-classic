
typedef struct {
    vekt2 r,
        // relozo,
        rmostani, v;
    double alfa, alfamostani, omega;
    double sug;
    double t;
} golyo;

void initgolyo(void);

extern int Nincsfal; // if true, balls can leave screen
void leptetgolyo(double stept);
// void ujraszamol( void );
// void idoreigazitmindet( void );

// filled by init, can read ball positions from here
extern int Ngolyok;
extern golyo* Golyok;
