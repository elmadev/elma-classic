#include "ALL.H"
#include "eol_settings.h"

int Hangmarmegy = 0;

void help(void) {
    szoveglista szl;

    int x1 = 90;
    int x2 = 220;
    int yo = 80;
    int dy = 32;
    // Korny->pabc_help->write( Korny->picbuffer, x1+60, yo,      "Help" );
    szl.addszoveg_kozep("Default controls:", 320, 20);

    szl.addszoveg("UP", x1, yo);
    szl.addszoveg("- Accelerate", x2, yo);

    szl.addszoveg("DOWN", x1, yo + dy);
    szl.addszoveg("- Block Wheels", x2, yo + dy);

    szl.addszoveg("LEFT", x1, yo + dy * 2);
    szl.addszoveg("- Rotate AntiClockwise", x2, yo + dy * 2);

    szl.addszoveg("RIGHT", x1, yo + dy * 3);
    szl.addszoveg("- Rotate Clockwise", x2, yo + dy * 3);

    szl.addszoveg("SPACE", x1, yo + dy * 4);
    szl.addszoveg("- Turn Around", x2, yo + dy * 4);

    szl.addszoveg("V", x1, yo + dy * 5);
    szl.addszoveg("- View Box Toggle", x2, yo + dy * 5);

    szl.addszoveg("T", x1, yo + dy * 6);
    szl.addszoveg("- Time Display Toggle", x2, yo + dy * 6);

    szl.addszoveg_kozep("After you have eaten all the fruits,", 320, yo + dy * 9);
    szl.addszoveg_kozep("touch the flower!", 320, yo + dy * 10);

    mk_emptychar();
    while (1) {
        if (mk_kbhit()) {
            int c = mk_getextchar();
            if (c == MK_ESC || c == MK_ENTER) {
                return;
            }
        }
        szl.kirajzol();
    }
}

// Egyben allithato:
void options(void) {
    int origsecondary = State->compatibility_mode;

    int kurrens = 0;
    while (1) {
        valaszt2 val;
        val.kur = kurrens;
        val.egykepen = 11;
        val.x0 = 72;
        val.x0_tab = 390;
        val.y0 = 77;
        val.dy = 36;
        val.escelheto = 1;
        strcpy(val.cim, "Options");

        int vantagis = 0;
        strcpy(Rubrikak[0], "Play mode:");
        if (State->single) {
            strcpy(Rubrikak_tab[0], "Single Player");
        } else {
            strcpy(Rubrikak_tab[0], "Multiplayer");
            vantagis = 1;
        }

        if (vantagis) {
            strcpy(Rubrikak[1], "Flag Tag:");
            if (State->flag_tag) {
                strcpy(Rubrikak_tab[1], "On");
            } else {
                strcpy(Rubrikak_tab[1], "Off");
            }
        }

        // Jatekos valasztas:
        strcpy(Rubrikak[1 + vantagis], "Player A:");
        strcpy(Rubrikak_tab[1 + vantagis], State->player1);

        strcpy(Rubrikak[2 + vantagis], "Player B:");
        strcpy(Rubrikak_tab[2 + vantagis], State->player2);

        strcpy(Rubrikak[3 + vantagis], "Sound:");
        if (State->sound_on) {
            strcpy(Rubrikak_tab[3 + vantagis], "Enabled");
        } else {
            strcpy(Rubrikak_tab[3 + vantagis], "Disabled");
        }

        strcpy(Rubrikak[4 + vantagis], "Sound Optimization:");
        if (State->compatibility_mode) {
            strcpy(Rubrikak_tab[4 + vantagis], "Compatibility");
        } else {
            strcpy(Rubrikak_tab[4 + vantagis], "Best Quality");
        }

        strcpy(Rubrikak[5 + vantagis], "Animated Menus:");
        if (State->animated_menus) {
            strcpy(Rubrikak_tab[5 + vantagis], "Yes");
        } else {
            strcpy(Rubrikak_tab[5 + vantagis], "No");
        }

        strcpy(Rubrikak[6 + vantagis], "Video Detail:");
        if (State->high_quality) {
            strcpy(Rubrikak_tab[6 + vantagis], "High");
        } else {
            strcpy(Rubrikak_tab[6 + vantagis], "Low");
        }

        strcpy(Rubrikak[7 + vantagis], "Animated Objects:");
        if (State->animated_objects) {
            strcpy(Rubrikak_tab[7 + vantagis], "Yes");
        } else {
            strcpy(Rubrikak_tab[7 + vantagis], "No");
        }

        strcpy(Rubrikak[8 + vantagis], "Swap Bikes:");
        if (State->player1_bike1) {
            strcpy(Rubrikak_tab[8 + vantagis], "No");
        } else {
            strcpy(Rubrikak_tab[8 + vantagis], "Yes");
        }

        strcpy(Rubrikak[9 + vantagis], "Customize Controls ...");
        strcpy(Rubrikak_tab[9 + vantagis], "");

        sprintf(Rubrikak[10 + vantagis], "Pics In Background:");
        if (EolSettings->pictures_in_background) {
            sprintf(Rubrikak_tab[10 + vantagis], "Yes");
        } else {
            sprintf(Rubrikak_tab[10 + vantagis], "No");
        }

        val.bead(11 + vantagis, 1);

        kurrens = val.valassz();

        if (kurrens < 0) {
            // State->save_aaa(); majd programbol valo kilepeskor kiirjuk
            if (origsecondary != State->compatibility_mode && Hangmarmegy) {
                // Kiirunk egy dialogust, hogy ujra kell inditani ehhez:
                szoveglista szl;
                szl.addszoveg_kozep("You modified the sound buffer", 320, 140);
                szl.addszoveg_kozep("optimization! As you already", 320, 190);
                szl.addszoveg_kozep("used the sound in this session,", 320, 240);
                szl.addszoveg_kozep("this will only take effect", 320, 290);
                szl.addszoveg_kozep("if you restart the game.", 320, 340);
                mk_emptychar();
                while (1) {
                    if (mk_kbhit()) {
                        int c = mk_getextchar();
                        if (c == MK_ESC || c == MK_ENTER) {
                            break;
                        }
                    }
                    szl.kirajzol();
                }
            }
            return;
        }

        if (kurrens == 0) {
            State->single = !State->single;
        }

        if (kurrens == 1 && vantagis) {
            State->flag_tag = !State->flag_tag;
        }

        if (vantagis) {
            kurrens -= 1;
        }

        if (kurrens == 1) {
            jatekosvalasztas(1, 1);
        }

        if (kurrens == 2) {
            jatekosvalasztas(0, 1);
        }

        if (kurrens == 3) {
            State->sound_on = !State->sound_on;
        }

        if (kurrens == 4) {
            State->compatibility_mode = !State->compatibility_mode;
        }

        if (kurrens == 5) {
            State->animated_menus = !State->animated_menus;
        }

        if (kurrens == 6) {
            State->high_quality = !State->high_quality;
            invalidate_ptop();
        }

        if (kurrens == 7) {
            State->animated_objects = !State->animated_objects;
        }

        if (kurrens == 8) {
            State->player1_bike1 = !State->player1_bike1;
        }

        if (kurrens == 9) {
            customize();
        }

        if (kurrens == 10) {
            EolSettings->pictures_in_background = !EolSettings->pictures_in_background;
            invalidate_level();
        }

        if (vantagis) { // Visszaalitjuk eredeti helyzetebe
            kurrens += 1;
        }
    }
}
