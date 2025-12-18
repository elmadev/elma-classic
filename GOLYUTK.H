#ifndef GOLYUTK_H
#define GOLYUTK_H

enum { Wall_Top = 0, Wall_Bottom, Wall_Right, Wall_Left };

double ball_ball_collision_time(ball* ball1, ball* ball2);
void simulate_ball_ball_collision(ball* ball1, ball* ball2, double time);

double ball_wall_collision_time(ball* ball, int wall);
void simulate_ball_wall_collision(ball* ball, int wall, double time);

void update_keyframe(ball* m, double t);

extern int WallsDisabled;

#endif
