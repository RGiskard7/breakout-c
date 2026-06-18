#ifndef BRICK_H
#define BRICK_H

#include <allegro5/allegro.h>
#include "types.h"

typedef struct _brick BRICK;

BRICK *brick_create(int row, int col);
void   brick_destroy(BRICK *b);

bool   brick_is_alive(BRICK *b);
STATUS brick_hit(BRICK *b);
int    brick_get_row(BRICK *b);
int    brick_get_col(BRICK *b);
int    brick_get_points(BRICK *b);

float  brick_get_x(BRICK *b);
float  brick_get_y(BRICK *b);
int    brick_get_width(BRICK *b);
int    brick_get_height(BRICK *b);

void   brick_render(BRICK *b);

#endif
