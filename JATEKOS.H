#ifndef JATEKOS_H
#define JATEKOS_H

// return false if Esc out of the menu
bool menu_player_create(bool change_player1);

// return false if Esc out of the menu. Return true if player selected or deleted
bool menu_player_choose(bool change_player1, bool allow_delete);

#endif
