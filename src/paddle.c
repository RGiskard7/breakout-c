#include "paddle.h"
#include "config.h"
#include <allegro5/allegro_primitives.h>
#include <stdlib.h>

struct _paddle {
    float x;
    int width;
    int height;
};

PADDLE *paddle_create(void) {
    PADDLE *p = (PADDLE *)malloc(sizeof(PADDLE));
    if (!p) return NULL;
    p->x = PADDLE_INIT_X;
    p->width = PADDLE_WIDTH;
    p->height = PADDLE_HEIGHT;
    return p;
}

void paddle_destroy(PADDLE *p) {
    if (p) free(p);
}

STATUS paddle_set_x(PADDLE *p, float x) {
    if (!p) return ERROR;
    p->x = x;
    return OK;
}

float paddle_get_x(PADDLE *p) {
    return p ? p->x : 0;
}

int paddle_get_width(PADDLE *p) {
    return p ? p->width : 0;
}

int paddle_get_height(PADDLE *p) {
    return p ? p->height : 0;
}

void paddle_move_left(PADDLE *p) {
    if (!p) return;
    if (p->x - PADDLE_SPEED >= 10)
        p->x -= PADDLE_SPEED;
}

void paddle_move_right(PADDLE *p) {
    if (!p) return;
    if (p->x + p->width + PADDLE_SPEED <= DISPLAY_WIDTH - 10)
        p->x += PADDLE_SPEED;
}

STATUS paddle_render(PADDLE *p) {
    if (!p) return ERROR;
    al_draw_filled_rectangle(p->x, PADDLE_Y,
        p->x + p->width, PADDLE_Y + p->height,
        al_map_rgb(0, 200, 255));
    return OK;
}
