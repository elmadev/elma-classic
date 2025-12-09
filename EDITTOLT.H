

void tolt_open(void);
// Ha sikeresen mentettek, akkor igazak:
int tolt_save_as(int lokkolhato);
int tolt_save(int lokkolhato);
void tolt_new(void);

void change_level_properties(void);

// Editfilenev alapjan megprobal josolni, majd oda teszi eredmenyt:
void kivalasztkezdetit(void);

// Kivalaszt LGR listabol egy sprite-ot:
void tolt_pickasprite(void);

// Siker eseten igaz:
int tolt_picklgrfile(char* lgrnev); // lgrnevbe masol max 8 chart

void setspritetav(sprite* psp);

void set_kerek_tipus(char* fejlec, int* pkajatipus, int* pfoodsorszam);

extern int Rajzolpoligon;
extern int Rajzolkoveto;
extern int Rajzolkepek;

void setviewoptions(void);
