#include "game.h"
#include "config.h"
#include "paddle.h"
#include "ball.h"
#include "brick.h"

#include <allegro5/allegro_primitives.h>
#include <allegro5/allegro_font.h>
#include <allegro5/allegro_ttf.h>
#include <allegro5/allegro_audio.h>
#include <allegro5/allegro_acodec.h>
#include <stdio.h>
#include <stdlib.h>
#include <math.h>

struct _game {
    ALLEGRO_DISPLAY *display;
    ALLEGRO_TIMER *timer;
    ALLEGRO_EVENT_QUEUE *queue;
    ALLEGRO_EVENT event;
    ALLEGRO_FONT *font;
    ALLEGRO_SAMPLE *snd_bounce;
    ALLEGRO_SAMPLE *snd_break;

    PADDLE *paddle;
    BALL *ball;
    BRICK *bricks[BRICK_COLS][BRICK_ROWS];

    GAME_STATE state;
    bool done;
    bool draw;

    int total_bricks;
    int bricks_destroyed;
    int score;
    int lives;
    int title_timer;
    int dead_timer;
    bool space_was;
};

static const float speed_stages[] = {BALL_SPEED_INIT, BALL_SPEED_1, BALL_SPEED_2, BALL_SPEED_3};
static const int   speed_thresh[]  = {0, STAGE_1, STAGE_2, STAGE_3};

// --- Function declarations ---

static STATUS game_reset_bricks(GAME *g);
static STATUS game_reset_ball(GAME *g);
static STATUS game_next_ball(GAME *g);
static void   game_update_speed(GAME *g);
static STATUS game_update_play(GAME *g, ALLEGRO_KEYBOARD_STATE *key);
static STATUS game_colisions(GAME *g);

static STATUS game_print_bricks(GAME *g);
static STATUS game_print_paddle(GAME *g);
static STATUS game_print_ball(GAME *g);
static STATUS game_print_hud(GAME *g);
static STATUS game_print_title(GAME *g);
static STATUS game_print_gameover(GAME *g);
static STATUS game_print_win(GAME *g);
static STATUS game_print_dead(GAME *g);

// --- Create / Destroy / Init ---

GAME *game_create(void) {
    GAME *g = (GAME *)malloc(sizeof(GAME));
    if (!g) return NULL;

    g->display = NULL;
    g->timer = NULL;
    g->queue = NULL;
    g->font = NULL;
    g->snd_bounce = NULL;
    g->snd_break = NULL;
    g->paddle = NULL;
    g->ball = NULL;

    for (int c = 0; c < BRICK_COLS; c++)
        for (int r = 0; r < BRICK_ROWS; r++)
            g->bricks[c][r] = NULL;

    g->state = STATE_TITLE;
    g->done = false;
    g->draw = false;
    g->total_bricks = 0;
    g->bricks_destroyed = 0;
    g->score = 0;
    g->lives = MAX_LIVES;
    g->title_timer = 0;
    g->dead_timer = 0;
    g->space_was = true;

    return g;
}

STATUS game_init(GAME *g) {
    if (!g) return ERROR;

    g->timer = al_create_timer(1.0 / FPS);
    if (!g->timer) return ERROR;

    g->display = al_create_display(DISPLAY_WIDTH, DISPLAY_HEIGHT);
    if (!g->display) return ERROR;
    al_set_window_title(g->display, "Breakout");

    g->queue = al_create_event_queue();
    if (!g->queue) return ERROR;

    al_register_event_source(g->queue, al_get_timer_event_source(g->timer));
    al_register_event_source(g->queue, al_get_display_event_source(g->display));
    al_register_event_source(g->queue, al_get_keyboard_event_source());

    g->font = al_load_ttf_font(FONT_RSC, 18, 0);
    g->snd_bounce = al_load_sample(SND_BOUNCE);
    g->snd_break  = al_load_sample(SND_BREAK);

    g->paddle = paddle_create();
    if (!g->paddle) return ERROR;

    g->ball = ball_create();
    if (!g->ball) return ERROR;

    if (game_reset_bricks(g) == ERROR) return ERROR;

    return OK;
}

void game_destroy(GAME *g) {
    if (!g) return;

    for (int c = 0; c < BRICK_COLS; c++)
        for (int r = 0; r < BRICK_ROWS; r++)
            if (g->bricks[c][r]) brick_destroy(g->bricks[c][r]);

    if (g->ball) ball_destroy(g->ball);
    if (g->paddle) paddle_destroy(g->paddle);
    if (g->snd_break) al_destroy_sample(g->snd_break);
    if (g->snd_bounce) al_destroy_sample(g->snd_bounce);
    if (g->font) al_destroy_font(g->font);
    if (g->queue) al_destroy_event_queue(g->queue);
    if (g->timer) al_destroy_timer(g->timer);
    if (g->display) al_destroy_display(g->display);
    free(g);
}

ALLEGRO_DISPLAY     *game_get_display(GAME *g) { return g ? g->display : NULL; }
ALLEGRO_TIMER       *game_get_timer(GAME *g)   { return g ? g->timer : NULL; }
ALLEGRO_EVENT_QUEUE *game_get_queue(GAME *g)    { return g ? g->queue : NULL; }
ALLEGRO_EVENT       *game_get_event(GAME *g)    { return g ? &g->event : NULL; }
bool                 game_is_done(GAME *g)      { return g ? g->done : true; }

// --- Reset helpers ---

static STATUS game_reset_bricks(GAME *g) {
    if (!g) return ERROR;
    for (int c = 0; c < BRICK_COLS; c++) {
        for (int r = 0; r < BRICK_ROWS; r++) {
            if (g->bricks[c][r]) brick_destroy(g->bricks[c][r]);
            g->bricks[c][r] = brick_create(r, c);
            if (!g->bricks[c][r]) return ERROR;
        }
    }
    g->total_bricks = BRICK_TOTAL;
    g->bricks_destroyed = 0;
    return OK;
}

static STATUS game_reset_ball(GAME *g) {
    if (!g || !g->ball || !g->paddle) return ERROR;
    ball_attach_to_paddle(g->ball, paddle_get_x(g->paddle));
    g->space_was = true;
    return OK;
}

static STATUS game_next_ball(GAME *g) {
    if (!g) return ERROR;
    g->lives--;
    if (g->lives <= 0) {
        g->state = STATE_OVER;
        return OK;
    }
    g->state = STATE_DEAD;
    g->dead_timer = 0;
    return OK;
}

static void game_update_speed(GAME *g) {
    if (!g) return;
    int lvl = 0;
    for (int s = 3; s >= 0; s--)
        if (g->bricks_destroyed >= speed_thresh[s]) { lvl = s; break; }
    ball_set_speed(g->ball, speed_stages[lvl]);
}

// --- Update ---

STATUS game_update(GAME *g, ALLEGRO_KEYBOARD_STATE *key) {
    if (!g || !key) return ERROR;

    if (g->state != STATE_PLAY && al_key_down(key, ALLEGRO_KEY_ESCAPE)) {
        g->done = true;
        return OK;
    }

    switch (g->state) {
    case STATE_TITLE:
        g->title_timer++;
        if (al_key_down(key, ALLEGRO_KEY_ENTER)) {
            g->state = STATE_PLAY;
            g->score = 0;
            g->lives = MAX_LIVES;
            game_reset_bricks(g);
            game_reset_ball(g);
            paddle_set_x(g->paddle, PADDLE_INIT_X);
        }
        break;

    case STATE_PLAY:
        game_update_play(g, key);
        break;

    case STATE_DEAD:
        if (++g->dead_timer > 50) {
            paddle_set_x(g->paddle, PADDLE_INIT_X);
            game_reset_ball(g);
            g->state = STATE_PLAY;
        }
        break;

    case STATE_OVER:
    case STATE_WIN:
        if (al_key_down(key, ALLEGRO_KEY_ENTER))
            g->state = STATE_TITLE;
        break;
    }

    g->draw = true;
    return OK;
}

static STATUS game_update_play(GAME *g, ALLEGRO_KEYBOARD_STATE *key) {
    if (!g || !key) return ERROR;

    if (al_key_down(key, ALLEGRO_KEY_LEFT))
        paddle_move_left(g->paddle);
    if (al_key_down(key, ALLEGRO_KEY_RIGHT))
        paddle_move_right(g->paddle);

    bool sp = al_key_down(key, ALLEGRO_KEY_SPACE);
    if (ball_is_serving(g->ball) && sp && !g->space_was)
        ball_launch(g->ball);
    g->space_was = sp;

    float px = paddle_get_x(g->paddle);
    ball_move(g->ball, px);

    if (!ball_is_serving(g->ball)) {
        ball_bounce_wall(g->ball);

        if (ball_bounce_paddle(g->ball, px) == OK)
            if (g->snd_bounce) al_play_sample(g->snd_bounce, 0.4f, 0, 1.2f, ALLEGRO_PLAYMODE_ONCE, NULL);

        if (ball_get_y(g->ball) > DISPLAY_HEIGHT) {
            game_next_ball(g);
            return OK;
        }

        if (game_colisions(g) == ERROR) return ERROR;
    }

    return OK;
}

static STATUS game_colisions(GAME *g) {
    if (!g) return ERROR;

    float bx = ball_get_x(g->ball);
    float by = ball_get_y(g->ball);
    int sz = BALL_SIZE;
    float cx = bx + sz / 2, cy = by + sz / 2;

    for (int c = 0; c < BRICK_COLS; c++) {
        for (int r = 0; r < BRICK_ROWS; r++) {
            BRICK *br = g->bricks[c][r];
            if (!br || !brick_is_alive(br)) continue;

            float rx = brick_get_x(br);
            float ry = brick_get_y(br);
            int rw = brick_get_width(br);
            int rh = brick_get_height(br);

            if (bx < rx + rw && bx + sz > rx && by < ry + rh && by + sz > ry) {
                brick_hit(br);
                g->total_bricks--;
                g->bricks_destroyed++;
                g->score += brick_get_points(br);

                game_update_speed(g);

                // Determine bounce axis: compare overlaps on each side
                float ol = fabs(cx - rx);
                float or = fabs(rx + rw - cx);
                float ot = fabs(cy - ry);
                float ob = fabs(ry + rh - cy);
                float min_x = (ol < or) ? ol : or;
                float min_y = (ot < ob) ? ot : ob;

                if (min_x < min_y)
                    ball_bounce_x(g->ball);
                else
                    ball_bounce_y(g->ball);

                if (g->snd_break) al_play_sample(g->snd_break, 0.5f, 0, 1.0f, ALLEGRO_PLAYMODE_ONCE, NULL);

                if (g->total_bricks <= 0) g->state = STATE_WIN;
                return OK;
            }
        }
    }
    return OK;
}

// --- Render ---

STATUS game_render(GAME *g) {
    if (!g || !g->draw) return ERROR;

    al_clear_to_color(al_map_rgb(10, 10, 30));
    al_draw_rectangle(8, 8, DISPLAY_WIDTH - 8, DISPLAY_HEIGHT - 8,
        al_map_rgb(255, 255, 255), 2);

    game_print_bricks(g);
    if (g->state != STATE_TITLE) game_print_paddle(g);
    if (!ball_is_serving(g->ball) || g->state == STATE_DEAD || g->state == STATE_PLAY)
        game_print_ball(g);
    game_print_hud(g);

    if (g->state == STATE_TITLE)  game_print_title(g);
    if (g->state == STATE_OVER)   game_print_gameover(g);
    if (g->state == STATE_WIN)    game_print_win(g);
    if (g->state == STATE_DEAD)   game_print_dead(g);

    al_flip_display();
    g->draw = false;
    return OK;
}

static STATUS game_print_bricks(GAME *g) {
    if (!g) return ERROR;
    for (int c = 0; c < BRICK_COLS; c++)
        for (int r = 0; r < BRICK_ROWS; r++)
            brick_render(g->bricks[c][r]);
    return OK;
}

static STATUS game_print_paddle(GAME *g) {
    return paddle_render(g->paddle);
}

static STATUS game_print_ball(GAME *g) {
    return ball_render(g->ball);
}

static STATUS game_print_hud(GAME *g) {
    if (!g || !g->font) return OK;
    char buf[32];
    sprintf(buf, "SCORE: %d", g->score);
    al_draw_text(g->font, al_map_rgb(255, 255, 255), 20, DISPLAY_HEIGHT - 30, 0, buf);
    sprintf(buf, "LIVES: %d", g->lives);
    al_draw_text(g->font, al_map_rgb(255, 255, 255), DISPLAY_WIDTH - 140, DISPLAY_HEIGHT - 30, 0, buf);
    return OK;
}

static STATUS game_print_title(GAME *g) {
    if (!g || !g->font) return OK;
    al_draw_filled_rectangle(130, 220, 470, 380, al_map_rgba(0, 0, 0, 200));
    al_draw_text(g->font, al_map_rgb(0, 255, 0), DISPLAY_WIDTH / 2, 240, ALLEGRO_ALIGN_CENTER, "BREAKOUT");
    al_draw_text(g->font, al_map_rgb(255, 255, 255), DISPLAY_WIDTH / 2, 300, ALLEGRO_ALIGN_CENTER, "PRESS ENTER TO START");
    if ((g->title_timer / 30) % 2)
        al_draw_text(g->font, al_map_rgb(255, 255, 0), DISPLAY_WIDTH / 2, 330, ALLEGRO_ALIGN_CENTER, "-> ENTER <-");
    return OK;
}

static STATUS game_print_gameover(GAME *g) {
    if (!g || !g->font) return OK;
    char buf[32];
    al_draw_filled_rectangle(150, 240, 450, 360, al_map_rgba(0, 0, 0, 200));
    al_draw_text(g->font, al_map_rgb(255, 0, 0), DISPLAY_WIDTH / 2, 270, ALLEGRO_ALIGN_CENTER, "GAME OVER");
    sprintf(buf, "SCORE: %d", g->score);
    al_draw_text(g->font, al_map_rgb(255, 255, 255), DISPLAY_WIDTH / 2, 310, ALLEGRO_ALIGN_CENTER, buf);
    return OK;
}

static STATUS game_print_win(GAME *g) {
    if (!g || !g->font) return OK;
    char buf[32];
    al_draw_filled_rectangle(150, 240, 450, 360, al_map_rgba(0, 0, 0, 200));
    al_draw_text(g->font, al_map_rgb(0, 255, 0), DISPLAY_WIDTH / 2, 270, ALLEGRO_ALIGN_CENTER, "YOU WIN!");
    sprintf(buf, "SCORE: %d", g->score);
    al_draw_text(g->font, al_map_rgb(255, 255, 255), DISPLAY_WIDTH / 2, 310, ALLEGRO_ALIGN_CENTER, buf);
    return OK;
}

static STATUS game_print_dead(GAME *g) {
    if (!g || !g->font) return OK;
    al_draw_text(g->font, al_map_rgb(255, 0, 0), DISPLAY_WIDTH / 2, DISPLAY_HEIGHT / 2, ALLEGRO_ALIGN_CENTER, "BALL LOST!");
    return OK;
}
