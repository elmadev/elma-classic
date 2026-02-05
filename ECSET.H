#ifndef ECSET_H
#define ECSET_H

#include "vect2.h"

class ecset;
class grass;
struct canvas_chunk_node;
class pic8;
class sprite;
struct segment;

// Egy iterator class (inkabb cache) ecsethez (foltoz hasznalja oket):
class node_finder {
    ecset* source;
    int current_x, current_y;
    canvas_chunk_node* current_node;

  public:
    node_finder(ecset* src);
    canvas_chunk_node* get_chunk(int x, int y);
};

// Elso par strukturat be lehetne majd vinni cpp-be:

#define PixelsDefaultForeground (1)
#define PixelsDefaultBackground (0)
#define PixelsTransparent (2)

struct canvas_chunk_node {
    canvas_chunk_node* next;
    int width;
    int distance;
    unsigned char* pixels;
};

#define CHUNK_NODE_BLOCK_LENGTH (10000)

struct canvas_chunk_node_array {
    canvas_chunk_node nodes[CHUNK_NODE_BLOCK_LENGTH];
    canvas_chunk_node_array* next;
};

#define CANVAS_MAX_HEIGHT (12000)

struct canvas_chunk {
    int width;
    unsigned char* pixels;
};

void create_canvases(void);

class ecset {
    friend void create_canvases(void);
    friend void segedfv(void);
    friend canvas_chunk_node* node_finder::get_chunk(int x, int y);

    // madarab allokalas:
    canvas_chunk_node_array* node_array;
    canvas_chunk_node_array* node_array_last;
    int node_array_index;
    canvas_chunk_node* newmdarab(void);
    void deletemdarabok(void);

    canvas_chunk* chunk_array;

    int is_minimap;
    vect2 origin;
    double width, height;
    int pixel_width, pixel_height;
    canvas_chunk_node* rows_linked[CANVAS_MAX_HEIGHT];
    canvas_chunk* rows[CANVAS_MAX_HEIGHT];
    // Kulon van ket jatekosnak:
    int rows_x1[CANVAS_MAX_HEIGHT];                  // Eppen hol all mutatohoz tartozo x
    canvas_chunk* rows_position1[CANVAS_MAX_HEIGHT]; // Melyik darab kurrens
    int rows_x2[CANVAS_MAX_HEIGHT];                  // Eppen hol all mutatohoz tartozo x
    canvas_chunk* rows_position2[CANVAS_MAX_HEIGHT]; // Melyik darab kurrens

    void getorigoandsize(void);
    void addszakasz(segment* psz);
    // Rovid folddarabokat atalakitja konkret mutatova:
    void foldmutatocsere(void);
    // Egymas utani azonos maszkokat es ureseket lecsereli egy hosszabbra:
    void duplaeliminacio(void);
    void textura2mutato(void);
    void mutatotlanit(void);

    // Ptop kerek-jeinek egesz koordinatait kitolti:
    void kitoltfoodkoordokat(void);

    void levon10000t(void);
    canvas_chunk_node* beepitegymadarabnyit(canvas_chunk_node* pmd, unsigned char* ujpixelek,
                                            int x1, int ujx1, int ujx2, int ujtavolsag,
                                            canvas_chunk_node* vege, int* pujavege, int fazis);
    void addbytesor(unsigned char* ujpixelek, int ujtavolsag, int ujx1, int ujx2, int y, int fazis);
    void addegymaszk(sprite* psp, int index_t, int index_m, int fazis);
    void addspriteok(int fazis);
    void addobjektumok(void); // Ez most nem view eseten visszter

    void kovetotextura(grass* pkov, int index, int xo, int yo, int fazis);
    void kiegykovetokep(grass* pkov, int index, int xo, int yo, int fazis);
    void kikovetokepek(grass* pkov, int* ytomb, int hossz, int x0, int fazis);
    void addkovetok(int fazis);

    void kiegysor_A(unsigned char* pc, int ye);
    void kiegysor_B(unsigned char* pc, int ye);
    void kiegysor_FF(unsigned char* pc, int ye); // fekete-feher

    void foltoz(void);

    ecset(int view);    // Ptop es Segments alapjan
    ecset(ecset* pold); // Ures sortomboket hoz benne letre
    ~ecset(void);

  public:
    void getbalalso_int(vect2 balalso, int* px, int* py);
    void kitesz(int ajatekos, pic8* ppic, vect2 balalso, int x1, int y1, int x2, int y2);
    void kiteszview(int ajatekos, pic8* ppic, vect2 balalso, int x1, int y1, int x2, int y2);

    // void egetkeszit( void );
};

extern ecset* CanvasBack;
extern ecset* CanvasFront;
extern ecset* CanvasMinimap;

// create_canvases ide osszegzi osszes lefoglalt szakaszt:
extern int Osszegszam;

#endif
