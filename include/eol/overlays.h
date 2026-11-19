#ifndef EOL_OVERLAYS_H
#define EOL_OVERLAYS_H

struct clickable;
struct game_mouse;
class pic8;

namespace overlays {

extern bool Editor;

void render(pic8& screen);
void get_closest(const game_mouse& pos, int& dist, clickable*& closest);

} // namespace overlays

#endif
