

extern double Elszakadasisebhat, Belsosav, G;
extern double Talppontegybeolvadasitav;

extern double Ugroturelem;
extern double Vegenvaras;

extern double Dr, Sr;

void initadatok(void);

extern double Fejsugar;

extern double Objektumsugar;

// Hajszalnyival kisebb rigidbody.sugarnal, hogy kirajzolasnal ne logjon foldbe:
extern double Kerekrajzsugar;

// MCGA kirajzolasos:

extern double Arany, Aranyrec;

extern double Kord2x, Kord2y, Kord4x, Kord4y,
    Kord5y; // Ez az utolso vezeto helye

#define OBJEKTUMSUGAR_I (20)

extern int Viewzoom;

struct rigidbody {
    double rotation;
    double angular_velocity;
    double radius;
    double mass;
    double inertia; // Moment of inertia
    vekt2 r;
    vekt2 v;
};

struct motorst {
    rigidbody bike, left_wheel, right_wheel;
    vekt2 head_r; // bike-bol Kor12Fejr fuggveny allitja elo (leptet.h)!
    int flipped_bike;
    int flipped_camera;
    int gravity_direction; // 1 a normalis (0:UP 1:DOWN 2:LEFT 3:RIGHT)
    // double labalfa; Bicigli

    vekt2 body_r, body_v; // Motoros

    int apple_count; // hanyat evett ez a motoros

    // Leptet ezeket is hasznalja:
    int prev_brake;
    double left_wheel_brake_rotation, right_wheel_brake_rotation;

    int volting_right, volting_left;
    double right_volt_time, left_volt_time;
    double angular_velocity_pre_right_volt, angular_velocity_pre_left_volt;
};

extern motorst *Pmot1, *Pmot2;

// reorder.h-ban friend-kent mar deklaralva van:
// pgazhatra-ba beepiti betoltvet:
class recorder;
void berakrecbehosszat(recorder* prec);
// pgazhatra alapjan ellenorzi hosszt:
int jorechossza(recorder* prec);
