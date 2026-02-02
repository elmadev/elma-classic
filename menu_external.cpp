#include "menu_external.h"
#include "menu_nav.h"
#include "fs_utils.h"
#include "LEJATSZO.H"
#include "LOAD.H"
#include "main.h"
#include "menu_nav.h"
#include "menu_pic.h"
#include "platform_impl.h"
#include "platform_utils.h"
#include "PLAY.H"
#include "state.h"
#include <cstring>
#include <algorithm>
#include <string>
#include <vector>

void menu_external_levels() {
    finame filename;
    std::vector<std::string> lev_names;
    bool done = find_first("lev/*.lev", filename);
    int count = 0;
    while (!done) {
        lev_names.push_back(filename);

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

    int i = 0;
    std::sort(lev_names.begin(), lev_names.end(),
              [](std::string& a, std::string& b) { return strcmpi(a.c_str(), b.c_str()) < 0; });
    for (std::string& name : lev_names) {
        strcpy(NavEntriesLeft[i++], name.c_str());
    }

    int previous_index = 0;
    for (int i = 0; i < count; i++) {
        char tmp[50];
        strcpy(tmp, NavEntriesLeft[i]);
        if (strcmpi(tmp, State->external_filename) == 0) {
            previous_index = i;
        }
    }

    menu_nav val;
    val.search_pattern = SearchPattern::Sorted;
    val.selected_index = previous_index;
    strcpy(val.title, "Select External File!");

    val.setup(count);

    while (true) {
        int choice = val.navigate();
        if (choice < 0) {
            return;
        }

        char filename[20];
        nav_entry* entry = val.entry_left(choice);
        if (strlen(*entry) > 15) {
            internal_error("menu_external_levels() strlen(entry) > 15!");
        }
        strcpy(filename, *entry);

        strcpy(State->external_filename, filename);

        while (true) {
            loading_screen();
            if (!floadlevel_p(filename)) {
                break;
            }
            Rec1->erase(filename);
            Rec2->erase(filename);
            int time = lejatszo(filename, F1Pressed ? CameraMode::MapViewer : CameraMode::Normal);
            MenuPalette->set();
            char finish_msg[100] = "";
            update_top_ten(time, finish_msg, 0, filename);
            if (!menu_level(0, 0, finish_msg, filename)) {
                Rec1->erase(filename);
                Rec2->erase(filename);
                break;
            }
        }
    }
}
