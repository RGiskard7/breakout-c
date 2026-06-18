#ifndef PADDLE_H
#define PADDLE_H

#include <allegro5/allegro.h>
#include "types.h"

typedef struct _paddle PADDLE;

PADDLE *paddle_create(void);
void    paddle_destroy(PADDLE *p);

STATUS  paddle_set_x(PADDLE *p, float x);
float   paddle_get_x(PADDLE *p);
int     paddle_get_width(PADDLE *p);
int     paddle_get_height(PADDLE *p);

void    paddle_move_left(PADDLE *p);
void    paddle_move_right(PADDLE *p);
STATUS  paddle_render(PADDLE *p);

#endif
