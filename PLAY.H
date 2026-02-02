#ifndef PLAY_H
#define PLAY_H

void menu_play();

void replay_from_file(const char* filename);

// Pass either the internal_index or the external_filename
// Update top ten and show finish message
// time_message examples:
//      "You Failed to Finish! (A/B died first)"
//       "(A:/B:/null) 00:14:00 Best Time!/You Made the Top Ten!"
// Update top ten and save external level file but don't save state.dat file
void update_top_ten(int time, char* time_message, int internal_index,
                    const char* external_filename);

// Pass either the internal_index or the external_filename
// Returns selection:
// 0 = Esc
// 1 = Play Again
// 2 = Play Next
// 3 = Skip
// Save Play -> submenu
// 4 = Best Times
int menu_level(int internal_index, bool nav_on_play_next, const char* time_message,
               const char* external_filename);

void loading_screen();

#endif
