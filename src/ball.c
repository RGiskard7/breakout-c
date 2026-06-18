#include "ball.h"
#include "config.h"
#include <allegro5/allegro_primitives.h>
#include <stdlib.h>
#include <math.h>

struct _ball {
    float x, y;
    float vx, vy;
    float speed;
    int size;
    bool serving;
};

BALL *ball_create(void) {
    BALL *b = (BALL *)malloc(sizeof(BALL));
    if (!b) return NULL;
    b->x = 0;
    b->y = 0;
    b->vx = 0;
    b->vy = 0;
    b->speed = BALL_SPEED_INIT;
    b->size = BALL_SIZE;
    b->serving = true;
    return b;
}

void ball_destroy(BALL *b) {
    if (b) free(b);
}

float ball_get_x(BALL *b)     { return b ? b->x : 0; }
float ball_get_y(BALL *b)     { return b ? b->y : 0; }
float ball_get_speed(BALL *b) { return b ? b->speed : 0; }
bool  ball_is_serving(BALL *b){ return b ? b->serving : true; }

STATUS ball_set_x(BALL *b, float x) { if (!b) return ERROR; b->x = x; return OK; }
STATUS ball_set_y(BALL *b, float y) { if (!b) return ERROR; b->y = y; return OK; }
STATUS ball_set_speed(BALL *b, float speed) { if (!b) return ERROR; b->speed = speed; return OK; }
STATUS ball_set_serving(BALL *b, bool s) { if (!b) return ERROR; b->serving = s; return OK; }

STATUS ball_attach_to_paddle(BALL *b, float paddle_x) {
    if (!b) return ERROR;
    b->x = paddle_x + PADDLE_WIDTH / 2 - b->size / 2;
    b->y = PADDLE_Y - b->size - 2;
    b->vx = 0;
    b->vy = 0;
    b->speed = BALL_SPEED_INIT;
    b->serving = true;
    return OK;
}

STATUS ball_launch(BALL *b) {
    if (!b) return ERROR;
    b->vx = (float)(rand() % 5 - 2) * 1.3f;
    b->vy = -b->speed;
    b->serving = false;
    return OK;
}

STATUS ball_move(BALL *b, float paddle_x) {
    if (!b) return ERROR;
    if (b->serving) {
        b->x = paddle_x + PADDLE_WIDTH / 2 - b->size / 2;
        return OK;
    }
    b->x += b->vx;
    b->y += b->vy;
    return OK;
}

STATUS ball_bounce_wall(BALL *b) {
    if (!b) return ERROR;
    if (b->x < 10)          { b->x = 10; b->vx = -b->vx; }
    if (b->x > DISPLAY_WIDTH - 10 - b->size) { b->x = DISPLAY_WIDTH - 10 - b->size; b->vx = -b->vx; }
    if (b->y < 10)          { b->y = 10; b->vy = -b->vy; }
    return OK;
}

STATUS ball_bounce_paddle(BALL *b, float paddle_x) {
    if (!b) return ERROR;
    if (b->y + b->size >= PADDLE_Y && b->y <= PADDLE_Y + PADDLE_HEIGHT &&
        b->x + b->size > paddle_x && b->x < paddle_x + PADDLE_WIDTH && b->vy > 0) {
        b->y = PADDLE_Y - b->size;
        float hit = (b->x + b->size / 2 - paddle_x) / PADDLE_WIDTH;
        float ang = (hit - 0.5f) * BALL_MAX_ANGLE;
        float spd = (float)sqrt(b->vx * b->vx + b->vy * b->vy);
        b->vx = (float)sin(ang) * spd;
        b->vy = -(float)cos(ang) * spd;
        return OK;
    }
    return ERROR;
}

STATUS ball_bounce_x(BALL *b) {
    if (!b) return ERROR;
    b->vx = -b->vx;
    float spd = (float)sqrt(b->vx * b->vx + b->vy * b->vy);
    if (spd > 0) { b->vx = b->vx / spd * b->speed; b->vy = b->vy / spd * b->speed; }
    return OK;
}

STATUS ball_bounce_y(BALL *b) {
    if (!b) return ERROR;
    b->vy = -b->vy;
    float spd = (float)sqrt(b->vx * b->vx + b->vy * b->vy);
    if (spd > 0) { b->vx = b->vx / spd * b->speed; b->vy = b->vy / spd * b->speed; }
    return OK;
}

STATUS ball_render(BALL *b) {
    if (!b) return ERROR;
    al_draw_filled_circle(b->x + b->size / 2, b->y + b->size / 2,
        b->size / 2, al_map_rgb(255, 255, 255));
    return OK;
}
