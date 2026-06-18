#ifndef BALL_H
#define BALL_H

#include <allegro5/allegro.h>
#include "types.h"

typedef struct _ball BALL;

BALL  *ball_create(void);
void   ball_destroy(BALL *b);

float  ball_get_x(BALL *b);
float  ball_get_y(BALL *b);
float  ball_get_speed(BALL *b);
bool   ball_is_serving(BALL *b);

STATUS ball_set_x(BALL *b, float x);
STATUS ball_set_y(BALL *b, float y);
STATUS ball_set_speed(BALL *b, float speed);
STATUS ball_set_serving(BALL *b, bool serving);

STATUS ball_attach_to_paddle(BALL *b, float paddle_x);
STATUS ball_launch(BALL *b);
STATUS ball_move(BALL *b, float paddle_x);
STATUS ball_bounce_wall(BALL *b);
STATUS ball_bounce_paddle(BALL *b, float paddle_x);
STATUS ball_bounce_x(BALL *b);
STATUS ball_bounce_y(BALL *b);

STATUS ball_render(BALL *b);

#endif
