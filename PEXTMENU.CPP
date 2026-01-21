#include "fs_utils.h"
#include "LEJATSZO.H"
#include "LOAD.H"
#include "main.h"
#include "menu_nav.h"
#include "menu_pic.h"
#include "platform_impl.h"
#include "platform_utils.h"
#include "PLAY.H"
#include "TOPOL.H"
#include <cstring>

int abcbenelobb(const char* text1, const char* text2);

static void sort_external_levels(int count) {
    for (int i = 0; i < count + 3; i++) {
        for (int j = 0; j < count - 1; j++) {
            char* name1 = NavEntriesLeft[j];
            char* name2 = NavEntriesLeft[j + 1];
            if (abcbenelobb(name2, name1) > 0) {
                char tmp[20];
                strcpy(tmp, name1);
                strcpy(name1, name2);
                strcpy(name2, tmp);
            }
        }
    }
}

void menu_external_levels() {
    finame filename;
    bool done = find_first("lev/*.lev", filename);
    int count = 0;
    while (!done) {
        strcpy(NavEntriesLeft[count], filename);

        done = find_next(filename);
        count++;
        if (count >= NavEntriesLeftMaxLength - 4) {
            done = true;
        }
    }

    find_close();

    if (count <= 0) {
        return;
    }

    sort_external_levels(count);

    int previous_index = 0;
    for (int i = 0; i < count; i++) {
        char tmp[50];
        strcpy(tmp, NavEntriesLeft[i]);
        if (strcmpi(tmp, State->external_filename) == 0) {
            previous_index = i;
        }
    }

    menu_nav val;
    val.selected_index = previous_index;
    strcpy(val.title, "Select External File!");

    val.setup(count);

    while (true) {
        int choice = val.navigate();
        if (choice < 0) {
            return;
        }

        char filename[20];
        if (strlen(val.entries_left[choice]) > 15) {
            internal_error("menu_external_levels() choice > 15!");
        }
        strcpy(filename, val.entries_left[choice]);

        strcpy(State->external_filename, filename);

        while (true) {
            kiirloading();
            if (!floadlevel_p(filename)) {
                break;
            }
            Prec1->erase(filename);
            Prec2->erase(filename);
            int time = lejatszo(filename);
            MenuPalette->set();
            char finish_msg[100] = "";
            idoelintezes(time, finish_msg, 0, filename);
            if (!afterplay(0, 0, finish_msg, filename)) {
                Prec1->erase(filename);
                Prec2->erase(filename);
                break;
            }
        }
    }
}
