#include "ALL.H"
#include "eol_settings.h"

int Hangmarmegy = 0;

void help(void) {
    menu_pic szl;

    int x1 = 90;
    int x2 = 220;
    int yo = 80;
    int dy = 32;
    // Korny->pabc_help->write( Korny->picbuffer, x1+60, yo,      "Help" );
    szl.add_line_centered("Default controls:", 320, 20);

    szl.add_line("UP", x1, yo);
    szl.add_line("- Accelerate", x2, yo);

    szl.add_line("DOWN", x1, yo + dy);
    szl.add_line("- Block Wheels", x2, yo + dy);

    szl.add_line("LEFT", x1, yo + dy * 2);
    szl.add_line("- Rotate AntiClockwise", x2, yo + dy * 2);

    szl.add_line("RIGHT", x1, yo + dy * 3);
    szl.add_line("- Rotate Clockwise", x2, yo + dy * 3);

    szl.add_line("SPACE", x1, yo + dy * 4);
    szl.add_line("- Turn Around", x2, yo + dy * 4);

    szl.add_line("V", x1, yo + dy * 5);
    szl.add_line("- View Box Toggle", x2, yo + dy * 5);

    szl.add_line("T", x1, yo + dy * 6);
    szl.add_line("- Time Display Toggle", x2, yo + dy * 6);

    szl.add_line_centered("After you have eaten all the fruits,", 320, yo + dy * 9);
    szl.add_line_centered("touch the flower!", 320, yo + dy * 10);

    mk_emptychar();
    while (1) {
        if (mk_kbhit()) {
            int c = mk_getextchar();
            if (c == MK_ESC || c == MK_ENTER) {
                return;
            }
        }
        szl.render();
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

        strcpy(Rubrikak[10 + vantagis], "Pics In Background:");
        if (EolSettings->pictures_in_background) {
            strcpy(Rubrikak_tab[10 + vantagis], "Yes");
        } else {
            strcpy(Rubrikak_tab[10 + vantagis], "No");
        }

        strcpy(Rubrikak[11 + vantagis], "Centered Camera:");
        if (EolSettings->center_camera) {
            strcpy(Rubrikak_tab[11 + vantagis], "Yes");
        } else {
            strcpy(Rubrikak_tab[11 + vantagis], "No");
        }

        strcpy(Rubrikak[12 + vantagis], "Centered Minimap:");
        if (EolSettings->center_map) {
            strcpy(Rubrikak_tab[12 + vantagis], "Yes");
        } else {
            strcpy(Rubrikak_tab[12 + vantagis], "No");
        }

        val.bead(13 + vantagis, 1);

        kurrens = val.valassz();

        if (kurrens < 0) {
            // State->save_aaa(); majd programbol valo kilepeskor kiirjuk
            if (origsecondary != State->compatibility_mode && Hangmarmegy) {
                // Kiirunk egy dialogust, hogy ujra kell inditani ehhez:
                menu_pic szl;
                szl.add_line_centered("You modified the sound buffer", 320, 140);
                szl.add_line_centered("optimization! As you already", 320, 190);
                szl.add_line_centered("used the sound in this session,", 320, 240);
                szl.add_line_centered("this will only take effect", 320, 290);
                szl.add_line_centered("if you restart the game.", 320, 340);
                mk_emptychar();
                while (1) {
                    if (mk_kbhit()) {
                        int c = mk_getextchar();
                        if (c == MK_ESC || c == MK_ENTER) {
                            break;
                        }
                    }
                    szl.render();
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

        if (kurrens == 11) {
            EolSettings->center_camera = !EolSettings->center_camera;
        }

        if (kurrens == 12) {
            EolSettings->center_map = !EolSettings->center_map;
        }

        if (vantagis) { // Visszaalitjuk eredeti helyzetebe
            kurrens += 1;
        }
    }
}
