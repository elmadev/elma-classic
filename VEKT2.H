
// in case of #define TEST, do additional tests

class vekt2 {
  public:
    double x, y;
    vekt2(void);
    vekt2(double, double);
    vekt2 operator+(vekt2);
    vekt2 operator-(vekt2);
    double operator*(vekt2);
    double operator%(vekt2); // vectorial product
    void normal(void);
    void forgatas(double alfa);
};

vekt2 operator*(double, vekt2);
vekt2 operator*(vekt2, double);
double abs(vekt2);
double absnegyzet(vekt2);
vekt2 egys(vekt2);
vekt2 forgatas90fokkal(vekt2);
vekt2 forgatasminusz90fokkal(vekt2);

// personal square root function:
double gyok(double x);

// different calculations:

// returns the intersection point of lines
// determined by the v1 vector with r1 origin and v2 vector with r2 origin
// calls hiba( char* ) in case of parallel lines
vekt2 intersection(vekt2 r1, vekt2 v1, vekt2 r2, vekt2 v2);
// calculates the distance of a point from a line segment (vsz = length of segment) 
double pontszakasztav(vekt2 rpont, vekt2 rsz, vekt2 vsz);
// calculates the distance of a point from a line
double pontegyenestav(vekt2 rpont, vekt2 rsz, vekt2 vsz);
// returns the intersection of 2 circles, r1 and r2 on the left side (sloppy):
vekt2 ketkormetszete(vekt2 r1, vekt2 r2, double l1, double l2);

// returns true if v1 vector with r1 origin and v2 vector with r2 origin intersect
int vektormetszes(vekt2 r1, vekt2 v1, vekt2 r2, vekt2 v2);
// this function doesn't consider intersections where the two vectors are barely touching(?)
int vektormetszes2(vekt2 r1, vekt2 v1, vekt2 r2, vekt2 v2);
// this function is more lenient on intersections
int vektormetszes3(vekt2 r1, vekt2 v1, vekt2 r2, vekt2 v2);

// returns true if a line intersects a circle
// of the two intersection points, it returns the first one when moving towards v:
int egyeneskormetszes(vekt2 r, vekt2 v, vekt2 r_kor, double sugar, vekt2* pmetszet);

extern vekt2 Vekt2i, Vekt2j, Vekt2k, Vekt2null;
extern double Pi;

// todo: write the following
// send message when dividing by 0 (inverted matrices)
// void hiba( char* message, char* text2 = NULL );
