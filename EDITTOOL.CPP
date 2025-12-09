#include "all.h"

// int Kurzornegyzet = 0;

void alaphelp(void) {
    switch (Tool) {
    case T_MOVE:
        toolhelp("Move the cursor near a vertex or an object center you want to move, and click "
                 "left button.");
        break;
    case T_ZOOMIN:
        toolhelp("Click the left button to place the first corner of zoom window.");
        break;
    case T_CREATE_VERT:
        toolhelp("If you click left button near a vertex you will add to a polygon, else create a "
                 "new polygon.");
        break;
    case T_DELETE_VERT:
        toolhelp("Click the left button near the vertex you want to delete.");
        break;
    case T_DELETE_POLY:
        toolhelp("Click the left button near any vertex of the polygon you want to delete.");
        break;
    case T_CREATE_FOOD:
        toolhelp("Click the left button to place a new Food object.");
        break;
    case T_CREATE_KILLER:
        toolhelp("Click the left button to place a new Killer object.");
        break;
    case T_DELETE_KEREK:
        toolhelp("Click the left button near the center of the object you want to delete.");
        break;
    case T_CREATE_SPRITE:
        toolhelp("Click the left button to place a new Picture. Right button chooses picture.");
        break;
    case T_DELETE_SPRITE:
        toolhelp("Click the left button near the left-up corner of the picture you want to delete. "
                 "Right chooses.");
        break;
    default:
        toolhelp("Ehhez a tool-hoz meg nincs alaphelp!");
    }
}

// MOVE MOVE MOVE MOVE MOVE MOVE MOVE MOVE MOVE MOVE MOVE MOVE
// MOVE MOVE MOVE MOVE MOVE MOVE MOVE MOVE MOVE MOVE MOVE MOVE
// MOVE MOVE MOVE MOVE MOVE MOVE MOVE MOVE MOVE MOVE MOVE MOVE
// MOVE MOVE MOVE MOVE MOVE MOVE MOVE MOVE MOVE MOVE MOVE MOVE

static double Moveorigx = 0, Moveorigy = 0;

void t_move_nyomva(int mx, int my) {
    if (Pgy || Pker || Psp) {
        if ((Pgy && Pker) || (Pgy && Psp) || (Pker && Psp)) {
            hiba("t_move_nyomva-ban (Pgy && Pker) || (Pgy && Psp) || (Pker && Psp)!");
        }
        // Most fogjuk elengedni pontot:
        if (Pgy) {
            if (Pker) {
                hiba("iuofiuceyhbuc");
            }
            Pgy->ponttomb[K] = konv(mx, my);
            Pgy = NULL;
        } else {
            if (Pker) {
                Pker->r = konv(mx, my);
                Pker = NULL;
            } else {
                Psp->r = konv(mx, my);
                Psp = NULL;
            }
        }
        Valtozott = 1;
        alaphelp();
        invalidate();
    } else {
        // Most probalunk megfogni egy pontot:
        double x = konvx_i2d(mx);
        double y = konvy_i2d(my);

        double tavpont, tavkerek, tavsprite;
        Pgy = Ptop->legkozpont(x, y, &K, &tavpont);
        Pker = Ptop->legkozkerek(x, y, &tavkerek);
        Psp = Ptop->legkozsprite(x, y, &tavsprite);
        if (!Pgy && !Pker && !Psp) {
            return;
        }
        // Kiszurjuk hogy maximum egy maradjon nem NULL:
        if (Pgy && Pker) {
            if (tavpont < tavkerek) {
                Pker = NULL;
            } else {
                Pgy = NULL;
            }
        }
        // Most mar csak egyik nem NULL Pgy es Pker kozul:
        if (Pgy && Psp) {
            if (tavpont < tavsprite) {
                Psp = NULL;
            } else {
                Pgy = NULL;
            }
        }
        if (Pker && Psp) {
            if (tavkerek < tavsprite) {
                Psp = NULL;
            } else {
                Pker = NULL;
            }
        }
        // Kis ellenorzes:
        if ((Pgy && Pker) || (Pgy && Psp) || (Pker && Psp)) {
            hiba("t_move_nyomva-ban legtav utan:",
                 "(Pgy && Pker) || (Pgy && Psp) || (Pker && Psp)!");
        }
        // Talaltunk egy pontot:
        if (Pgy) {
            toolhelp("Move the vertex and press the left button to place it. Press ESC or right "
                     "button to cancel.");
            Moveorigx = Pgy->ponttomb[K].x;
            Moveorigy = Pgy->ponttomb[K].y;
            mx = konvx_d2i(Moveorigx);
            my = konvy_d2i(Moveorigy);
        } else {
            if (Pker) {
                toolhelp("Move the object and press the left button to place it. Press ESC or "
                         "right button to cancel.");
                Moveorigx = Pker->r.x;
                Moveorigy = Pker->r.y;
                mx = konvx_d2i(Moveorigx);
                my = konvy_d2i(Moveorigy);
            } else {
                toolhelp("Move the picture and press the left button to place it. Press ESC or "
                         "right button to cancel.");
                Moveorigx = Psp->r.x;
                Moveorigy = Psp->r.y;
                mx = konvx_d2i(Moveorigx);
                my = konvy_d2i(Moveorigy);
            }
        }
        setmou(mx, my);
        push();
        Moux = mx;
        Mouy = my;
        pop();
    }
}

// Gany, mar megfogott ponttal raugrik kozeli pontra:
void gany_move_jobb(int mx, int my) {
    if (!Pgy) {
        hiba("uirw");
    }
    // Meg van fogva egy pont, melyik van ehhez legkozelebb:
    double tav;
    double x = konvx_i2d(mx);
    double y = konvy_i2d(my);
    int ujk = 0;
    gyuru* pujgy = Ptop->legkozpont(x, y, &ujk, &tav, Pgy);
    if (!pujgy) {
        return;
    }
    Pgy->ponttomb[K] = pujgy->ponttomb[ujk];
    Pgy = NULL;
    invalidate();
}

void t_move_nyomva_jobb(int mx, int my) {
    /*if( Pgy ) {
        gany_move_jobb( mx, my );
        return;
    } */
    if (Pgy || Pker || Psp) {
        t_move_esc();
        return;
    }
    // Nincs megfogva semmi. Ilyenkor attributumot lehet allitani:
    double tavsp = 0.0, tavgy = 0.0, tavker = 0.0;
    double x = konvx_i2d(mx);
    double y = konvy_i2d(my);
    sprite* psp = Ptop->legkozsprite(x, y, &tavsp);
    int semmi = 0;
    gyuru* pgy = Ptop->legkozpont(x, y, &semmi, &tavgy);
    kerek* pk = Ptop->legkozkerek(x, y, &tavker);

    if (!psp && !pgy && !pk) {
        return;
    }

    // Csak egy mutatot hagyunk meg nem NULL-nak:
    if (psp && pgy) {
        if (tavsp > tavgy) {
            psp = NULL;
        } else {
            pgy = NULL;
        }
    }

    if (psp && pk) {
        if (tavsp > tavker) {
            psp = NULL;
        } else {
            pk = NULL;
        }
    }

    if (pgy && pk) {
        if (tavgy > tavker) {
            pgy = NULL;
        } else {
            pk = NULL;
        }
    }

    if ((pgy && psp) || (pgy && pk) || (pk && psp)) {
        hiba("ycggyuegcfuye54354");
    }

    if (psp) {
        setspritetav(psp);
    }
    if (pgy) {
        set_gyuru_attributes(pgy);
    }
    // Csak kaja-nak lehet megvaltoztatni tipusat:
    if (pk && pk->tipus == T_KAJA) {
        set_kerek_tipus("Select food subtype!", &pk->kajatipus, &pk->foodsorszam);
    }
}

void t_move_esc(void) {
    if (Pgy) {
        Pgy->ponttomb[K].x = Moveorigx;
        Pgy->ponttomb[K].y = Moveorigy;
        Pgy = NULL;
        invalidate();
    }
    if (Pker) {
        Pker->r.x = Moveorigx;
        Pker->r.y = Moveorigy;
        Pker = NULL;
        invalidate();
    }
    if (Psp) {
        Psp->r.x = Moveorigx;
        Psp->r.y = Moveorigy;
        Psp = NULL;
        invalidate();
    }
}

/*void t_move_space( void ) {
    Kurzornegyzet = !Kurzornegyzet;
    invalidate();
} */

void t_move_mmove(int mx, int my) {
    // Ellenorzes:
    if (!Pgy && !Pker && !Psp) {
        hiba("t_move_mmove-ban !Pgy && !Pker!");
    }
    if ((Pgy && Pker) || (Pgy && Psp) || (Pker && Psp)) {
        hiba("t_move_mmove-ban:", "(Pgy && Pker) || (Pgy && Psp) || (Pker && Psp)!");
    }

    push();

    lockfrontbuffer_pic();
    if (Pgy) {
        // Letoroljuk oket:
        Pgy->kirajz(K, Fel, 0);
        Pgy->kirajz(K, !Fel, 0);
        // Uj koord:
        double dx = konvx_i2d(mx);
        double dy = konvy_i2d(my);
        Pgy->ujkoord(K, dx, dy);
        // Visszarajzoljuk oket:
        Pgy->kirajz(K, Fel, 0);
        Pgy->kirajz(K, !Fel, 0);
    } else {
        if (Pker) {
            Pker->kirajzol();
            double dx = konvx_i2d(mx);
            double dy = konvy_i2d(my);
            Pker->r.x = dx;
            Pker->r.y = dy;
            Pker->kirajzol();
        } else {
            Psp->kirajzol();
            double dx = konvx_i2d(mx);
            double dy = konvy_i2d(my);
            Psp->r.x = dx;
            Psp->r.y = dy;
            Psp->kirajzol();
        }
    }
    unlockfrontbuffer_pic();

    Moux = mx;
    Mouy = my;

    pop();
}

// CREATE_VERTEX CREATE_VERTEX CREATE_VERTEX CREATE_VERTEX
// CREATE_VERTEX CREATE_VERTEX CREATE_VERTEX CREATE_VERTEX
// CREATE_VERTEX CREATE_VERTEX CREATE_VERTEX CREATE_VERTEX
// CREATE_VERTEX CREATE_VERTEX CREATE_VERTEX CREATE_VERTEX

int Egypont = 0;
static int Epontx = 0, Eponty = 0;
static int Kpontx = 0, Kponty = 0;

// Bal gomb nyomva:
void t_create_vert_nyomva(int mx, int my) {
    Valtozott = 1;
    if (!Pgy && !Egypont) {
        // Most probalunk megfogni egy pontot:
        double x = konvx_i2d(mx);
        double y = konvy_i2d(my);

        Pgy = Ptop->legkozpont(x, y, &K);
        if (!Pgy) {
            // Egy uj poligon elso pontjat rakjuk le:
            // Eloszor megnezzuk, hogy van-e meg hely uj poligonnak:
            int gyuruszam = 0;
            for (int i = 0; i < MAXGYURU; i++) {
                if (Ptop->ptomb[i]) {
                    gyuruszam++;
                }
            }
            if (gyuruszam >= MAXGYURU) {
                dialog("You cannot create a new polygon",
                       "because you have already reached the maximum number of polygons (300)!");
                return;
            }

            toolhelp(
                "Click the left button to place the second point. ESC or right button cancels.");
            Egypont = 1;
            Epontx = Kpontx = mx;
            Eponty = Kponty = my;
            return;
        } else {
            // Talaltunk egy mar meglevo poligonon egy pontot:
            toolhelp("Click the left button to place point. SPACE and ENTER swaps. ESC or right "
                     "button cancels.");
            push();
            Moux = konvx_d2i(Pgy->ponttomb[K].x);
            Mouy = konvy_d2i(Pgy->ponttomb[K].y);
            pop();
        }
    }

    if (Pgy) {
        // Most fogunk letenni uj pontot:
        if (Pgy->getpontszam() >= MAXPONT) {
            dialog("You cannot create a new vertex, because you have",
                   "already reached the maximum number of vertices in this polygon (1000)!");
            return;
        }
        toolhelp("Click the left button to place point. SPACE and ENTER swaps. ESC or right button "
                 "cancels.");
        Pgy->beszur(K);
        invalidate();
        if (Fel) {
            K++;
        }
    } else {
        // Eddig egy pontja volt meg poligonnak, most mar valodi poligon-t
        // csinalunk belole:
        toolhelp("Click the left button to place point. SPACE and ENTER swaps. ESC or right button "
                 "cancels.");
        if (!Egypont) {
            hiba("!Egypont, pedig annak kene lennie (ffdsgiu)!");
        }
        for (int i = 0; i < MAXGYURU; i++) {
            if (!Ptop->ptomb[i]) {
                Pgy = Ptop->ptomb[i] = new gyuru;
                Pgy->pontszam = 3;
                Pgy->ponttomb[0] = konv(Epontx, Eponty);
                Pgy->ponttomb[1] = konv(mx, my);
                Pgy->ponttomb[2] = konv(mx, my);
                K = 2;
                Fel = 1;
                Egypont = 0;
                invalidate();
                return;
            }
        };
        hiba("Tul sok gyuru (iycriyfd)!");
    }
}

// Jobb gomb nyomva:
// Ha nem fogtunk pontot, akkor uj poligonnak veszi fel elso pontjat,
// Ha fogtuk pontot, akkor torli es elengedi:
void t_create_vert_esc(void) {
    if (!Pgy && !Egypont) {
        return; // Nincsen semmi dolgunk
    }

    alaphelp();
    if (Pgy) {
        // Pgy igaz, tehat egy pont meg van fogva, amit torlunk:
        if (Pgy->getpontszam() <= 3) {
            // Egesz poligont toroljuk:
            // Eloszor egy kis ellenorzes:
            int gyuruszam = 0;
            for (int i = 0; i < MAXGYURU; i++) {
                if (Ptop->ptomb[i]) {
                    gyuruszam++;
                }
            }
            if (gyuruszam <= 1) {
                hiba("t_create_vert_nyomva_esc-ban gyuruszam <= 1 (7pq4)!");
            }

            int talalt = 0;
            for (int i = 0; i < MAXGYURU; i++) {
                if (Ptop->ptomb[i] == Pgy) {
                    // Megtalaltuk:
                    talalt = 1;
                    delete Pgy;
                    Pgy = NULL;
                    Ptop->ptomb[i] = NULL;
                    K = 0;
                    invalidate();
                    break;
                }
            }
            if (!talalt) {
                hiba("Gyuru torlesekor !talalt!");
            }
        } else {
            // Csak a pontot toroljuk:
            Pgy->torol(K);
            Pgy = NULL;
            K = 0;
            invalidate();
        }
    } else {
        if (!Egypont) {
            hiba("!Egypont (jygfeuye)!");
        }
        Egypont = 0;
        invalidate();
    }
}

void t_create_vert_enter(void) {
    Fel = !Fel;
    if (Pgy && !Egypont) {
        invalidate();
    }
}

void t_create_vert_space(void) {
    if (!Pgy || Egypont) {
        return;
    }
    int kkov = K;
    if (Fel) {
        // Lejjebb maszunk egy pontot:
        kkov--;
        if (kkov < 0) {
            kkov = Pgy->pontszam - 1;
        }
    } else {
        // Feljebb maszunk egy pontot:
        kkov++;
        if (kkov >= Pgy->pontszam) {
            kkov = 0;
        }
    }
    Pgy->ponttomb[K] = Pgy->ponttomb[kkov];
    K = kkov;
    Moux = konvx(Pgy->ponttomb[K]);
    Mouy = konvy(Pgy->ponttomb[K]);
    setmou(Moux, Mouy);
    Fel = !Fel;
    invalidate();
}

void t_create_vert_mmove(int mx, int my) {
    if (!Pgy && !Egypont) {
        // Teljesen normalis kurzor mozgatas:
        hiba("Ezt most edituj.cpp-nek kellene csinalnia!");
    }
    push();
    lockfrontbuffer_pic();
    if (Pgy) {
        // Letoroljuk oket:
        Pgy->kirajz(K, Fel, 1); // szaggatott
        Pgy->kirajz(K, !Fel, 0);
        // Uj koord:
        double dx = konvx_i2d(mx);
        double dy = konvy_i2d(my);
        Pgy->ujkoord(K, dx, dy);
        // Visszarajzoljuk oket:
        Pgy->kirajz(K, Fel, 1); // szaggatott
        Pgy->kirajz(K, !Fel, 0);
        Moux = mx;
        Mouy = my;
    } else {
        if (!Egypont) {
            hiba("!Egypont (yifeye)");
        }
        vonalrajz(konv(Epontx, Eponty), konv(Kpontx, Kponty), 0);
        Moux = Kpontx = mx;
        Mouy = Kponty = my;
        vonalrajz(konv(Epontx, Eponty), konv(Kpontx, Kponty), 0);
    }
    unlockfrontbuffer_pic();
    pop();
}

// DELETE DELETE DELETE DELETE DELETE DELETE DELETE DELETE
// DELETE DELETE DELETE DELETE DELETE DELETE DELETE DELETE
// DELETE DELETE DELETE DELETE DELETE DELETE DELETE DELETE
// DELETE DELETE DELETE DELETE DELETE DELETE DELETE DELETE

void t_delete_vert_nyomva(int mx, int my) {
    if (Pgy) {
        hiba("yffwiye");
    }
    // Most probalunk megfogni egy pontot:
    double x = konvx_i2d(mx);
    double y = konvy_i2d(my);

    int k = 0;
    gyuru* pgy = Ptop->legkozpont(x, y, &k);
    if (!pgy) {
        return;
    }
    // Talaltunk egy pontot, amit le kene torolni:
    if (pgy->pontszam < 3) {
        hiba("yigfdsyhifeiu");
    }
    if (pgy->pontszam == 3) {
        // dialog( "You cannot delete this polygon, because every" );
        dialog("You cannot delete this vertex, because every polygon must",
               "have at least three vertices! If you want to delete the",
               "whole polygon, please select the DEL POLYGON tool!");
        return;
    }
    pgy->torol(k);
    Valtozott = 1;
    invalidate();
}

void t_delete_poly_nyomva(int mx, int my) {
    if (Pgy) {
        hiba("oiutrgiuot");
    }
    // Most probalunk megfogni egy pontot:
    double x = konvx_i2d(mx);
    double y = konvy_i2d(my);

    int k = 0;
    gyuru* pgy = Ptop->legkozpont(x, y, &k);
    if (!pgy) {
        return;
    }
    // Talaltunk egy poligont, amit le kene torolni:
    int szam = 0;
    for (int i = 0; i < MAXGYURU; i++) {
        if (Ptop->ptomb[i]) {
            szam++;
        }
    }
    if (szam == 0) {
        hiba("oufeifwe");
    }
    if (szam == 1) {
        dialog("This is the only polygon, so you cannot delete it.");
        return;
    }
    for (int i = 0; i < MAXGYURU; i++) {
        if (Ptop->ptomb[i] == pgy) {
            delete Ptop->ptomb[i];
            Ptop->ptomb[i] = NULL;
            Valtozott = 1;
            invalidate();
            return;
        }
    }
    hiba("fdsiyfdfei");
}

// CREATE_KEREK CREATE_KEREK CREATE_KEREK CREATE_KEREK CREATE_KEREK
// CREATE_KEREK CREATE_KEREK CREATE_KEREK CREATE_KEREK CREATE_KEREK
// CREATE_KEREK CREATE_KEREK CREATE_KEREK CREATE_KEREK CREATE_KEREK
// CREATE_KEREK CREATE_KEREK CREATE_KEREK CREATE_KEREK CREATE_KEREK

// Legkozelebbi create food ezt fogja hasznalni:
int Food_kajatipus = 0, Food_foodsorszam = 0;

void t_create_food_nyomva_jobb(void) {
    set_kerek_tipus("Select Create_Food subtype!", &Food_kajatipus, &Food_foodsorszam);
}

// Food es killer is egyben:
void t_create_kerek_nyomva(int mx, int my, int food) {
    // Megszamoljuk eddigi objektumokat:
    int objszam = 0;
    for (int i = 0; i < MAXKEREK; i++) {
        if (Ptop->kerektomb[i]) {
            objszam++;
        }
    }
    if (objszam >= MAXKEREK) {
        dialog("You have already reached the maximum number of objects (50)!");
        return;
    }

    for (int i = 0; i < MAXKEREK; i++) {
        if (!Ptop->kerektomb[i]) {
            double x = konvx_i2d(mx);
            double y = konvy_i2d(my);
            int tipus = T_KAJA;
            if (!food) {
                tipus = T_HALALOS;
            }
            Ptop->kerektomb[i] = new kerek(x, y, tipus);
            if (food) {
                Ptop->kerektomb[i]->kajatipus = Food_kajatipus;
                Ptop->kerektomb[i]->foodsorszam = Food_foodsorszam;
            }
            invalidate();
            Valtozott = 1;
            return;
        }
    }
    hiba("yer8yryfryfr!");
}

// DELETE_KEREK DELETE_KEREK DELETE_KEREK DELETE_KEREK DELETE_KEREK
// DELETE_KEREK DELETE_KEREK DELETE_KEREK DELETE_KEREK DELETE_KEREK
// DELETE_KEREK DELETE_KEREK DELETE_KEREK DELETE_KEREK DELETE_KEREK
// DELETE_KEREK DELETE_KEREK DELETE_KEREK DELETE_KEREK DELETE_KEREK

void t_delete_kerek_nyomva(int mx, int my) {
    double x = konvx_i2d(mx);
    double y = konvy_i2d(my);
    kerek* pker = Ptop->legkozkerek(x, y);
    if (!pker) {
        return;
    }
    for (int i = 0; i < MAXKEREK; i++) {
        if (Ptop->kerektomb[i] == pker) {
            if (pker->tipus == T_CEL) { // T_KEZDO
                dialog("You cannot delete the Exit object!");
                return;
            }
            if (pker->tipus == T_KEZDO) {
                dialog("You cannot delete the Start object!");
                return;
            }
            delete pker;
            Ptop->kerektomb[i] = NULL;
            Valtozott = 1;
            invalidate();
            return;
        }
    }
    hiba("iufewifewiyfweiyhfewiyh");
}

// ZOOM ZOOM ZOOM ZOOM ZOOM ZOOM ZOOM ZOOM ZOOM ZOOM ZOOM ZOOM
// ZOOM ZOOM ZOOM ZOOM ZOOM ZOOM ZOOM ZOOM ZOOM ZOOM ZOOM ZOOM
// ZOOM ZOOM ZOOM ZOOM ZOOM ZOOM ZOOM ZOOM ZOOM ZOOM ZOOM ZOOM
// ZOOM ZOOM ZOOM ZOOM ZOOM ZOOM ZOOM ZOOM ZOOM ZOOM ZOOM ZOOM

int Zoomfogva = 0; // Ha ez igaz, akkor nem szabad gombokat nyomkodni
static int Zoomx1 = 0, Zoomy1 = 0;
static int Zoomx2 = 0, Zoomy2 = 0;

void t_zoomin_nyomva(int x, int y) {
    if (Zoomfogva) {
        // Megvan masodik pont is:
        alaphelp();
        zoomin(Zoomx1, Zoomy1, x, y);
        Zoomfogva = 0;
        invalidate();
    } else {
        // Most tesszuk le elso pontot:
        toolhelp("Click left button to place the second corner of zoom window. ESC or right button "
                 "cancels.");
        Zoomfogva = 1;
        Zoomx1 = Zoomx2 = x;
        Zoomy1 = Zoomy2 = y;
    }
}

void t_zoomin_esc(void) {
    if (Zoomfogva) {
        alaphelp();
        Zoomfogva = 0;
        invalidate();
    }
}

static void zoompont(int x, int y) {
    unsigned char szin = Buffsima->gpixel(x, y);
    szin += 128;
    Buffsima->ppixel(x, y, szin);
    ppixelfront(x, y, szin);
}

static void kikeret(int x1, int y1, int x2, int y2) {
    if (x2 < x1) {
        int tmp = x1;
        x1 = x2;
        x2 = tmp;
    }
    if (y2 < y1) {
        int tmp = y1;
        y1 = y2;
        y2 = tmp;
    }
    for (int x = x1; x <= x2; x++) {
        zoompont(x, y1);
        zoompont(x, y2);
    }
    for (int y = y1; y <= y2; y++) {
        zoompont(x1, y);
        zoompont(x2, y);
    }
}

void t_zoomin_mmove(int x, int y) {
    push();
    if (Zoomfogva) {
        lockfrontbuffer_pic();
        kikeret(Zoomx1, Zoomy1, Zoomx2, Zoomy2);
        Zoomx2 = x;
        Zoomy2 = y;
        Moux = x;
        Mouy = y;
        kikeret(Zoomx1, Zoomy1, Zoomx2, Zoomy2);
        unlockfrontbuffer_pic();
    } else {
        // Teljesen normalis kurzor mozgatas:
        Moux = x;
        Mouy = y;
    }
    pop();
}

// SPRITE SPRITE SPRITE SPRITE SPRITE SPRITE SPRITE SPRITE SPRITE SPRITE
// SPRITE SPRITE SPRITE SPRITE SPRITE SPRITE SPRITE SPRITE SPRITE SPRITE
// SPRITE SPRITE SPRITE SPRITE SPRITE SPRITE SPRITE SPRITE SPRITE SPRITE
// SPRITE SPRITE SPRITE SPRITE SPRITE SPRITE SPRITE SPRITE SPRITE SPRITE
// SPRITE SPRITE SPRITE SPRITE SPRITE SPRITE SPRITE SPRITE SPRITE SPRITE
// SPRITE SPRITE SPRITE SPRITE SPRITE SPRITE SPRITE SPRITE SPRITE SPRITE

// Elougrik dialogus sprite valasztasra:
void t_create_sprite_nyomva_jobb(void) {
    // Plgr-en vegzi muveleteit:
    if (!Plgr) {
        hiba("grtyy");
    }
    if (Plgr->kepszam <= 0) {
        dialog("There are not any pictures in the LGR file!");
        return;
    }
    tolt_pickasprite();
    /*if( Plgr->aktivindex >= Plgr->kepszam )
        hiba( "[op0fijh" );
    kispritenev( Plgr->nevek[Plgr->aktivindex],
                 Plgr->tavolsagok[Plgr->aktivindex],
                 Plgr->hatarolok[Plgr->aktivindex] );*/
}

void t_create_sprite_nyomva(int mx, int my) {
    if (!Plgr) {
        hiba("987g45");
    }
    if (Plgr->kepszam <= 0) {
        dialog("There are not any pictures in the LGR file!");
        return;
    }

    // Megszamoljuk eddigi objektumokat:
    int spriteszam = 0;
    for (int i = 0; i < MAXSPRITE; i++) {
        if (Ptop->spritetomb[i]) {
            spriteszam++;
        }
    }
    if (spriteszam >= MAXSPRITE) {
        dialog("You have already reached the maximum number of pictures (5000)!");
        return;
    }

    if (!Plgr->aktiv_kepnev[0] && !(Plgr->aktiv_texturanev[0] && Plgr->aktiv_maszknev[0])) {
        return; // Nincs kitoltve aktiv sprite
    }

    for (int i = 0; i < MAXSPRITE; i++) {
        if (!Ptop->spritetomb[i]) {
            double x = konvx_i2d(mx);
            double y = konvy_i2d(my);
            Ptop->spritetomb[i] =
                new sprite(x, y, Plgr->aktiv_kepnev, Plgr->aktiv_texturanev, Plgr->aktiv_maszknev);
            invalidate();
            Valtozott = 1;
            return;
        }
    }
    hiba("oihhgffc!");
}

void t_delete_sprite_nyomva(int mx, int my) {
    double x = konvx_i2d(mx);
    double y = konvy_i2d(my);
    sprite* psp = Ptop->legkozsprite(x, y);
    if (!psp) {
        return;
    }
    for (int i = 0; i < MAXSPRITE; i++) {
        if (Ptop->spritetomb[i] == psp) {
            delete psp;
            Ptop->spritetomb[i] = NULL;
            Valtozott = 1;
            invalidate();
            return;
        }
    }
    hiba("uytyttyrtrtdf");
}
