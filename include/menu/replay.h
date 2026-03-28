#ifndef REPLAY_MENU_H
#define REPLAY_MENU_H

#include <string>

void menu_replay_all();
void menu_merge_replays();
void menu_replay_level(int level_id);
void menu_merge_level(int level_id, const std::string& merge_file);

#endif
