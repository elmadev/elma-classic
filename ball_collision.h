#ifndef BALL_COLLISION_H
#define BALL_COLLISION_H

struct ball;

enum class WallId {
    Top = 0,
    Bottom = 1,
    Right = 2,
    Left = 3,
};

double ball_ball_collision_time(ball* ball1, ball* ball2);
void simulate_ball_ball_collision(ball* ball1, ball* ball2, double time);

double ball_wall_collision_time(ball* ball, WallId wall);
void simulate_ball_wall_collision(ball* ball, WallId wall, double time);

extern bool WallsDisabled;

#endif
