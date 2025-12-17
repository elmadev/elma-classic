#ifndef STATE_H
#define STATE_H

#define MAX_PLAYERS (50)
#define MAX_PLAYERNAME_LENGTH (14)

#define STATE_LEVEL_COUNT (90)

extern int INTERNAL_LEVEL_COUNT;

struct player {
    char name[MAX_PLAYERNAME_LENGTH + 2];
    char skipped[((STATE_LEVEL_COUNT / 4) + 1) * 4];
    int levels_completed;
    int selected_level; // -1 if selected external files
};

int get_player_index(const char* player_name);

#define MAX_TIMES (10)

typedef char player_name[MAX_PLAYERNAME_LENGTH + 1];

struct topten {
    int times_count;
    int times[MAX_TIMES];
    player_name names1[MAX_TIMES];
    player_name names2[MAX_TIMES];
};

struct topten_set {
    topten single, multi;
};

struct player_keys {
    int gas, brake, right_volt, left_volt, turn;
    int toggle_minimap, toggle_timer, toggle_visibility;
};

class state {
  public:
    topten_set toptens[STATE_LEVEL_COUNT];
    player players[MAX_PLAYERS];
    int player_count;
    player_name player1, player2;

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
    int key_increase_screen_size, key_decrease_screen_size;
    int key_screenshot;

    char editor_filename[20];
    char external_filename[20];

    state(const char* filename = NULL);
    ~state(void);

    void reload_toptens(void);

    void save(void);
    void write_stats_player_total_time(FILE* h, const char* player_name, int single);
    void write_stats_anonymous_total_time(FILE* h, int single, const char* text1, const char* text2,
                                          const char* text3);
    void write_stats(void);
    void reset_keys(void);
};

extern state* State;

#endif
