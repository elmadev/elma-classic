
enum {
    WAV_UTODES = 1,
    WAV_TORES,
    WAV_SIKER,
    WAV_EVES,
    WAV_FORDULAS,
    WAV_UGRAS1,
    WAV_UGRAS2,
    WAV_ALAP,
    WAV_GAZ,
    WAV_DORZSOL,
    WAV_TURA
};

// Csak program elejen allithato:
// Ha hamis, semmi directsound fv.-t nem hiv:
extern int Hangenabled;
// Ha igaz, callback fuggveny 0-kat ir bufferbe:
extern int Mute;

void starthanghigh(void);
void stophanghigh(void);

// Elinditja alapjarat hangot:
void startmotor(int mot1);
// Leallitja motor hangjat (ha meghalt):
void stopmotor(int mot1);
// frekvencia 1.0-tol 2.0-ig valtozik csak:
void setmotor(int mot1, double frekvencia, int gaz);

void setsurlodas(double ero);

void startwave(int wavazonosito, double hangero);
int vanmegwav(void);

void hangosdelay(long t);
