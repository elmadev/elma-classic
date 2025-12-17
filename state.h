#ifndef STATE_H
#define STATE_H

#include "M_K.H"

constexpr int MAX_PLAYERS = 50;
constexpr int MAX_PLAYERNAME_LENGTH = 14;
constexpr int STATE_LEVEL_COUNT = 90;
constexpr int INTERNAL_LEVEL_COUNT = 55;
constexpr int MAX_TIMES = 10;

struct player {
    char name[MAX_PLAYERNAME_LENGTH + 2];
    char skipped[((STATE_LEVEL_COUNT / 4) + 1) * 4];
    int levels_completed;
    int selected_level; // -1 if selected external files
};

typedef char player_name[MAX_PLAYERNAME_LENGTH + 1];

struct topten {
    int times_count;
    int times[MAX_TIMES];
    player_name names1[MAX_TIMES];
    player_name names2[MAX_TIMES];
};

struct topten_set {
    topten single;
    topten multi;
};

struct player_keys {
    DIK_Code gas;
    DIK_Code brake;
    DIK_Code right_volt;
    DIK_Code left_volt;
    DIK_Code turn;
    DIK_Code toggle_minimap;
    DIK_Code toggle_timer;
    DIK_Code toggle_visibility; // Toggle Player
};

class state {
  public:
    topten_set toptens[STATE_LEVEL_COUNT];
    player players[MAX_PLAYERS];
    int player_count;
    player_name player1;
    player_name player2;

    int sound_on;
    int compatibility_mode;
    int single;
    int flag_tag;
    int player1_bike1; // 1 if player 1 uses Q1BIKE
    int high_quality;
    int animated_objects;
    int animated_menus;

    player_keys keys1;
    player_keys keys2;
    DIK_Code key_increase_screen_size;
    DIK_Code key_decrease_screen_size;
    DIK_Code key_screenshot;

    char editor_filename[20];
    char external_filename[20];

    state(const char* filename = nullptr);
    void reload_toptens();
    void save();
    void write_stats();
    void reset_keys();
    player* get_player(const char* player_name);

  private:
    void write_stats_player_total_time(FILE* h, const char* player_name, bool single);
    void write_stats_anonymous_total_time(FILE* h, bool single, const char* text1,
                                          const char* text2, const char* text3);
};

extern state* State;

#endif
