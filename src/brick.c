#include "brick.h"
#include "config.h"
#include <allegro5/allegro_primitives.h>
#include <stdlib.h>

static const ALLEGRO_COLOR colors[BRICK_ROWS] = {
    {1,0.3,0.3,1}, {1,0.3,0.3,1},
    {1,0.6,0,1},   {1,0.6,0,1},
    {1,0.85,0,1},  {1,0.85,0,1}
};

static const int points[BRICK_ROWS] = {
    PTS_ROW_0, PTS_ROW_1, PTS_ROW_2, PTS_ROW_3, PTS_ROW_4, PTS_ROW_5
};

struct _brick {
    int row, col;
    bool alive;
};

BRICK *brick_create(int row, int col) {
    BRICK *b = (BRICK *)malloc(sizeof(BRICK));
    if (!b) return NULL;
    b->row = row;
    b->col = col;
    b->alive = true;
    return b;
}

void brick_destroy(BRICK *b) {
    if (b) free(b);
}

bool brick_is_alive(BRICK *b) { return b ? b->alive : false; }

STATUS brick_hit(BRICK *b) {
    if (!b) return ERROR;
    b->alive = false;
    return OK;
}

int brick_get_row(BRICK *b)    { return b ? b->row : 0; }
int brick_get_col(BRICK *b)    { return b ? b->col : 0; }
int brick_get_points(BRICK *b) { return b ? points[b->row] : 0; }

float brick_get_x(BRICK *b) {
    return b ? BRICK_X0 + b->col * (BRICK_WIDTH + BRICK_GAP) : 0;
}
float brick_get_y(BRICK *b) {
    return b ? BRICK_Y0 + b->row * (BRICK_HEIGHT + BRICK_GAP) : 0;
}
int brick_get_width(BRICK *b)  { (void)b; return BRICK_WIDTH; }
int brick_get_height(BRICK *b) { (void)b; return BRICK_HEIGHT; }

void brick_render(BRICK *b) {
    if (!b || !b->alive) return;
    float x = brick_get_x(b);
    float y = brick_get_y(b);
    al_draw_filled_rectangle(x, y, x + BRICK_WIDTH, y + BRICK_HEIGHT, colors[b->row]);
    al_draw_rectangle(x, y, x + BRICK_WIDTH, y + BRICK_HEIGHT, al_map_rgb(40, 40, 40), 1);
}
