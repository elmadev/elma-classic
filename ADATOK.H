

extern double Elszakadasisebhat, Belsosav, G;
extern double Talppontegybeolvadasitav;

extern double Ugroturelem;
extern double Vegenvaras;

extern double Drsugar, Drtang, Sr;

void initadatok(void);

extern double Fejsugar;

extern double Objektumsugar;

// Hajszalnyival kisebb kor.sugarnal, hogy kirajzolasnal ne logjon foldbe:
extern double Kerekrajzsugar;

// MCGA kirajzolasos:

extern double Arany, Aranyrec;

extern double Kord2x, Kord2y, Kord4x, Kord4y,
    Kord5y; // Ez az utolso vezeto helye

#define OBJEKTUMSUGAR_I (20)

extern int Viewzoom;

struct motorst {
    kor kor1, kor2, kor4;
    vekt2 fejr; // kor1-bol Kor12Fejr fuggveny allitja elo (leptet.h)!
    int hatra_f;
    int hatra_h;
    int gravirany; // 1 a normalis (0:UP 1:DOWN 2:LEFT 3:RIGHT)
    // double labalfa; Bicigli

    vekt2 vezetor, vezetov; // Motoros

    int kajaszam; // hanyat evett ez a motoros

    // Leptet ezeket is hasznalja:
    int voltfek;
    double dfek2, dfek4;

    int ugrasban1, ugrasban2;
    double ugras1kezd, ugras2kezd;
    double kezdoomega1, kezdoomega2;
};

extern motorst *Pmot1, *Pmot2;

// reorder.h-ban friend-kent mar deklaralva van:
// pgazhatra-ba beepiti betoltvet:
class recorder;
void berakrecbehosszat(recorder* prec);
// pgazhatra alapjan ellenorzi hosszt:
int jorechossza(recorder* prec);
