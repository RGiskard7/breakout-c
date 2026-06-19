/**
 * @file game.c
 * @brief Core game logic: state machine, collisions and rendering.
 *
 * Recreates the Atari 2600 Breakout layout and rules: grey side walls plus a
 * full-width top bar, a top score band drawn with blocky digits, six solid
 * brick rows, a red paddle that halves after the ball breaks to the ceiling,
 * the 4/12-hit + orange/red speed progression and two screens of bricks.
 *
 * Author: RGiskard7
 * Date: 18/06/2026
 */

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
    ALLEGRO_DISPLAY *display;           ///< Game window
    ALLEGRO_TIMER *timer;               ///< Frame timer (FPS)
    ALLEGRO_EVENT_QUEUE *event_queue;   ///< Event queue
    ALLEGRO_EVENT events;               ///< Last event polled
    ALLEGRO_FONT *font;                 ///< TTF font for overlay text
    ALLEGRO_SAMPLE *snd_bounce;         ///< Paddle bounce sound
    ALLEGRO_SAMPLE *snd_break;          ///< Brick break sound

    PADDLE *paddle;                     ///< Player paddle
    BALL *ball;                         ///< Ball in play
    BRICK *bricks[BRICK_COLS][BRICK_ROWS]; ///< Brick wall grid

    GAME_STATE state;                   ///< Current state machine state
    bool done;                          ///< True when the main loop must exit
    bool draw;                          ///< True when a redraw is pending

    int total_bricks;                   ///< Bricks still standing on this screen
    int score;                          ///< Player score
    int lives;                          ///< Balls remaining
    int screen;                         ///< Current wall (1 .. MAX_SCREENS)

    int hits;                           ///< Bricks hit on this screen (speed-up)
    bool hit_orange;                    ///< Orange row reached at least once
    bool hit_red;                       ///< Red row reached at least once
    bool paddle_small;                  ///< Paddle already halved on this screen

    int title_timer;                    ///< Counter for the blinking title text
    int dead_timer;                     ///< Pause counter after losing a ball
    bool space_was_down;                ///< Edge detection for the serve key
};

/**
 * @brief Blocky 3x5 digit font for the score band (Atari-style scoreboard).
 *
 * Each entry is five rows of three bits (bit 2 = leftmost column). "1" is a
 * plain vertical bar, matching the original TIA score font.
 */
static const unsigned char DIGITS[10][5] = {
    {0x7, 0x5, 0x5, 0x5, 0x7}, // 0
    {0x2, 0x2, 0x2, 0x2, 0x2}, // 1
    {0x7, 0x1, 0x7, 0x4, 0x7}, // 2
    {0x7, 0x1, 0x7, 0x1, 0x7}, // 3
    {0x5, 0x5, 0x7, 0x1, 0x1}, // 4
    {0x7, 0x4, 0x7, 0x1, 0x7}, // 5
    {0x7, 0x4, 0x7, 0x5, 0x7}, // 6
    {0x7, 0x1, 0x2, 0x2, 0x2}, // 7
    {0x7, 0x5, 0x7, 0x5, 0x7}, // 8
    {0x7, 0x5, 0x7, 0x1, 0x7}  // 9
};

/**
 * @brief Ball speed for each of the five progression tiers.
 */
static const float _speed_tiers[5] = {
    BALL_SPD_0, BALL_SPD_1, BALL_SPD_2, BALL_SPD_3, BALL_SPD_4
};

// Private function declarations

static STATUS game_reset_bricks(GAME *game);
static STATUS game_reset_ball(GAME *game);
static STATUS game_reset_screen(GAME *game);
static STATUS game_new_game(GAME *game);
static STATUS game_next_ball(GAME *game);
static void   game_update_speed(GAME *game);
static STATUS game_update_play(GAME *game, ALLEGRO_KEYBOARD_STATE *key);
static STATUS game_colisions(GAME *game);

static void   game_draw_digits(const char *str, int x, int y, int pw, int ph);
static STATUS game_print_walls(GAME *game);
static STATUS game_print_bricks(GAME *game);
static STATUS game_print_hud(GAME *game);
static STATUS game_print_overlay(GAME *game);

// =========================================================================
// Functions: Game Creation and Destruction
// =========================================================================

/**
 * @brief Allocates a game and sets every field to a safe default.
 *
 * @return Pointer to the created game or NULL if allocation fails.
 */
GAME *game_create(void) {
    GAME *new_game = NULL;
    int c = 0;
    int r = 0;

    new_game = (GAME *) malloc(sizeof (GAME));
    if (!new_game) {
        return NULL;
    }

    new_game->display = NULL;
    new_game->timer = NULL;
    new_game->event_queue = NULL;
    new_game->font = NULL;
    new_game->snd_bounce = NULL;
    new_game->snd_break = NULL;
    new_game->paddle = NULL;
    new_game->ball = NULL;

    for (c = 0; c < BRICK_COLS; c++) {
        for (r = 0; r < BRICK_ROWS; r++) {
            new_game->bricks[c][r] = NULL;
        }
    }

    new_game->state = STATE_TITLE;
    new_game->done = false;
    new_game->draw = false;

    new_game->total_bricks = 0;
    new_game->score = 0;
    new_game->lives = MAX_LIVES;
    new_game->screen = 1;

    new_game->hits = 0;
    new_game->hit_orange = false;
    new_game->hit_red = false;
    new_game->paddle_small = false;

    new_game->title_timer = 0;
    new_game->dead_timer = 0;
    new_game->space_was_down = true;

    return new_game;
}

/**
 * @brief Destroys a game and every resource it owns.
 *
 * @param game Pointer to the game to destroy.
 */
void game_destroy(GAME *game) {
    int c = 0;
    int r = 0;

    if (!game) {
        return;
    }

    for (c = 0; c < BRICK_COLS; c++) {
        for (r = 0; r < BRICK_ROWS; r++) {
            if (game->bricks[c][r]) {
                brick_destroy(game->bricks[c][r]);
                game->bricks[c][r] = NULL;
            }
        }
    }

    if (game->ball) {
        ball_destroy(game->ball);
        game->ball = NULL;
    }
    if (game->paddle) {
        paddle_destroy(game->paddle);
        game->paddle = NULL;
    }
    if (game->snd_break) {
        al_destroy_sample(game->snd_break);
        game->snd_break = NULL;
    }
    if (game->snd_bounce) {
        al_destroy_sample(game->snd_bounce);
        game->snd_bounce = NULL;
    }
    if (game->font) {
        al_destroy_font(game->font);
        game->font = NULL;
    }
    if (game->event_queue) {
        al_destroy_event_queue(game->event_queue);
        game->event_queue = NULL;
    }
    if (game->timer) {
        al_destroy_timer(game->timer);
        game->timer = NULL;
    }
    if (game->display) {
        al_destroy_display(game->display);
        game->display = NULL;
    }

    free(game);
}

// =========================================================================
// Functions: Game Initialization
// =========================================================================

/**
 * @brief Initializes the window, timer, queue, resources and entities.
 *
 * @param game Pointer to the game.
 * @return STATUS code (OK on success, ERROR on any failure).
 */
STATUS game_init(GAME *game) {
    if (!game) {
        return ERROR;
    }

    game->timer = al_create_timer(1.0 / FPS);
    if (!game->timer) {
        return ERROR;
    }

    game->display = al_create_display(DISPLAY_WIDTH, DISPLAY_HEIGHT);
    if (!game->display) {
        return ERROR;
    }
    al_set_window_title(game->display, "Breakout");

    game->event_queue = al_create_event_queue();
    if (!game->event_queue) {
        return ERROR;
    }

    al_register_event_source(game->event_queue, al_get_timer_event_source(game->timer));
    al_register_event_source(game->event_queue, al_get_display_event_source(game->display));
    al_register_event_source(game->event_queue, al_get_keyboard_event_source());

    // la fuente y los sonidos son opcionales: si faltan, el juego sigue
    game->font = al_load_ttf_font(FONT_RSC, 18, 0);
    game->snd_bounce = al_load_sample(SND_BOUNCE);
    game->snd_break = al_load_sample(SND_BREAK);

    game->paddle = paddle_create();
    if (!game->paddle) {
        return ERROR;
    }

    game->ball = ball_create();
    if (!game->ball) {
        return ERROR;
    }

    if (game_reset_bricks(game) == ERROR) {
        return ERROR;
    }

    return OK;
}

// =========================================================================
// Functions: Accessors
// =========================================================================

/**
 * @brief Retrieves the game display.
 *
 * @param game Pointer to the game.
 * @return Display pointer, or NULL if game is NULL.
 */
ALLEGRO_DISPLAY *game_get_display(GAME *game) {
    if (!game) {
        return NULL;
    }

    return game->display;
}

/**
 * @brief Retrieves the game timer.
 *
 * @param game Pointer to the game.
 * @return Timer pointer, or NULL if game is NULL.
 */
ALLEGRO_TIMER *game_get_timer(GAME *game) {
    if (!game) {
        return NULL;
    }

    return game->timer;
}

/**
 * @brief Retrieves the game event queue.
 *
 * @param game Pointer to the game.
 * @return Event queue pointer, or NULL if game is NULL.
 */
ALLEGRO_EVENT_QUEUE *game_get_queue(GAME *game) {
    if (!game) {
        return NULL;
    }

    return game->event_queue;
}

/**
 * @brief Retrieves the address of the game's event slot.
 *
 * @param game Pointer to the game.
 * @return Event pointer, or NULL if game is NULL.
 */
ALLEGRO_EVENT *game_get_event(GAME *game) {
    if (!game) {
        return NULL;
    }

    return &game->events;
}

/**
 * @brief Tells whether the main loop should stop.
 *
 * @param game Pointer to the game.
 * @return True if done, true also if game is NULL.
 */
bool game_is_done(GAME *game) {
    if (!game) {
        return true;
    }

    return game->done;
}

// =========================================================================
// Functions: Helpers
// =========================================================================

/**
 * @brief Rebuilds the whole brick wall, all bricks alive.
 *
 * @param game Pointer to the game.
 * @return STATUS code (OK on success, ERROR on allocation failure).
 */
static STATUS game_reset_bricks(GAME *game) {
    int c = 0;
    int r = 0;

    if (!game) {
        return ERROR;
    }

    for (c = 0; c < BRICK_COLS; c++) {
        for (r = 0; r < BRICK_ROWS; r++) {
            if (game->bricks[c][r]) {
                brick_destroy(game->bricks[c][r]);
            }
            game->bricks[c][r] = brick_create(r, c);
            if (!game->bricks[c][r]) {
                return ERROR;
            }
        }
    }

    game->total_bricks = BRICK_TOTAL;

    return OK;
}

/**
 * @brief Places the ball on the paddle and applies the current speed tier.
 *
 * @param game Pointer to the game.
 * @return STATUS code (OK on success, ERROR if a needed entity is NULL).
 */
static STATUS game_reset_ball(GAME *game) {
    if (!game || !game->ball || !game->paddle) {
        return ERROR;
    }

    ball_attach_to_paddle(game->ball, paddle_get_x(game->paddle),
        paddle_get_width(game->paddle));
    game_update_speed(game);
    game->space_was_down = true;

    return OK;
}

/**
 * @brief Rebuilds the wall and clears the per-screen progression.
 *
 * Resets the hit counters, restores the full paddle and re-serves the ball.
 *
 * @param game Pointer to the game.
 * @return STATUS code (OK on success, ERROR on failure).
 */
static STATUS game_reset_screen(GAME *game) {
    if (!game) {
        return ERROR;
    }

    game->hits = 0;
    game->hit_orange = false;
    game->hit_red = false;
    game->paddle_small = false;

    paddle_set_width(game->paddle, PADDLE_WIDTH);
    paddle_set_x(game->paddle, PADDLE_INIT_X);

    if (game_reset_bricks(game) == ERROR) {
        return ERROR;
    }

    return game_reset_ball(game);
}

/**
 * @brief Starts a brand new game (score, lives and first screen).
 *
 * @param game Pointer to the game.
 * @return STATUS code (OK on success, ERROR on failure).
 */
static STATUS game_new_game(GAME *game) {
    if (!game) {
        return ERROR;
    }

    game->score = 0;
    game->lives = MAX_LIVES;
    game->screen = 1;

    return game_reset_screen(game);
}

/**
 * @brief Consumes one ball and moves to the dead or game-over state.
 *
 * @param game Pointer to the game.
 * @return STATUS code (OK on success, ERROR if game is NULL).
 */
static STATUS game_next_ball(GAME *game) {
    if (!game) {
        return ERROR;
    }

    game->lives--;
    if (game->lives <= 0) {
        game->state = STATE_OVER;
    } else {
        game->state = STATE_DEAD;
        game->dead_timer = 0;
    }

    return OK;
}

/**
 * @brief Updates the ball speed from the current per-screen progression.
 *
 * Tiers: 4 hits, 12 hits, first orange-row hit and first red-row hit.
 *
 * @param game Pointer to the game.
 */
static void game_update_speed(GAME *game) {
    int tier = 0;

    if (!game) {
        return;
    }

    if (game->hits >= SPEEDUP_HITS_1) {
        tier = 1;
    }
    if (game->hits >= SPEEDUP_HITS_2) {
        tier = 2;
    }
    if (game->hit_orange && tier < 3) {
        tier = 3;
    }
    if (game->hit_red) {
        tier = 4;
    }

    ball_set_speed(game->ball, _speed_tiers[tier]);
}

// =========================================================================
// Functions: Game Update
// =========================================================================

/**
 * @brief Advances the game one frame according to the current state.
 *
 * @param game Pointer to the game.
 * @param key Current keyboard state.
 * @return STATUS code (OK on success, ERROR on bad arguments).
 */
STATUS game_update(GAME *game, ALLEGRO_KEYBOARD_STATE *key) {
    if (!game || !key) {
        return ERROR;
    }

    // ESC sale del juego salvo durante la partida
    if (game->state != STATE_PLAY && al_key_down(key, ALLEGRO_KEY_ESCAPE)) {
        game->done = true;
        return OK;
    }

    switch (game->state) {
        case STATE_TITLE:
            game->title_timer++;
            if (al_key_down(key, ALLEGRO_KEY_ENTER)) {
                game_new_game(game);
                game->state = STATE_PLAY;
            }
            break;

        case STATE_PLAY:
            game_update_play(game, key);
            break;

        case STATE_DEAD:
            game->dead_timer++;
            if (game->dead_timer > 50) {
                paddle_set_x(game->paddle, PADDLE_INIT_X);
                game_reset_ball(game);
                game->state = STATE_PLAY;
            }
            break;

        case STATE_OVER:
        case STATE_WIN:
            if (al_key_down(key, ALLEGRO_KEY_ENTER)) {
                game->state = STATE_TITLE;
            }
            break;

        default:
            break;
    }

    game->draw = true;

    return OK;
}

/**
 * @brief Handles input, ball movement and collisions during play.
 *
 * @param game Pointer to the game.
 * @param key Current keyboard state.
 * @return STATUS code (OK on success, ERROR on bad arguments).
 */
static STATUS game_update_play(GAME *game, ALLEGRO_KEYBOARD_STATE *key) {
    float px = 0.0f;
    int pw = 0;
    bool sp = false;

    if (!game || !key) {
        return ERROR;
    }

    if (al_key_down(key, ALLEGRO_KEY_LEFT)) {
        paddle_move_left(game->paddle);
    }
    if (al_key_down(key, ALLEGRO_KEY_RIGHT)) {
        paddle_move_right(game->paddle);
    }

    px = paddle_get_x(game->paddle);
    pw = paddle_get_width(game->paddle);

    // saque con SPACE (deteccion de flanco para un solo disparo)
    sp = al_key_down(key, ALLEGRO_KEY_SPACE);
    if (ball_is_serving(game->ball) && sp && !game->space_was_down) {
        ball_launch(game->ball);
    }
    game->space_was_down = sp;

    ball_move(game->ball, px, pw);

    if (!ball_is_serving(game->ball)) {
        ball_bounce_wall(game->ball);

        // romper hasta el techo reduce la pala a la mitad en esta pantalla
        if (ball_take_ceiling_hit(game->ball) && !game->paddle_small) {
            game->paddle_small = true;
            paddle_set_width(game->paddle, PADDLE_WIDTH_SMALL);
        }

        if (ball_bounce_paddle(game->ball, px, paddle_get_width(game->paddle)) == OK) {
            if (game->snd_bounce) {
                al_play_sample(game->snd_bounce, 0.4f, 0, 1.2f, ALLEGRO_PLAYMODE_ONCE, NULL);
            }
        }

        if (ball_get_y(game->ball) > DISPLAY_HEIGHT) {
            game_next_ball(game);
            return OK;
        }

        if (game_colisions(game) == ERROR) {
            return ERROR;
        }
    }

    return OK;
}

/**
 * @brief Detects and resolves a ball-brick collision (one brick per frame).
 *
 * On a hit it scores the brick, updates the speed progression, bounces the
 * ball off the nearest face and, when the wall is cleared, advances to the
 * next screen or wins the game.
 *
 * @param game Pointer to the game.
 * @return STATUS code (OK on success, ERROR if game is NULL).
 */
static STATUS game_colisions(GAME *game) {
    BRICK *brick = NULL;
    float bx = 0.0f;
    float by = 0.0f;
    float cx = 0.0f;
    float cy = 0.0f;
    float rx = 0.0f;
    float ry = 0.0f;
    int rw = 0;
    int rh = 0;
    float ol = 0.0f;
    float orr = 0.0f;
    float ot = 0.0f;
    float ob = 0.0f;
    float min_x = 0.0f;
    float min_y = 0.0f;
    int c = 0;
    int r = 0;

    if (!game) {
        return ERROR;
    }

    bx = ball_get_x(game->ball);
    by = ball_get_y(game->ball);
    cx = bx + BALL_SIZE / 2.0f;
    cy = by + BALL_SIZE / 2.0f;

    for (c = 0; c < BRICK_COLS; c++) {
        for (r = 0; r < BRICK_ROWS; r++) {
            brick = game->bricks[c][r];
            if (!brick || !brick_is_alive(brick)) {
                continue;
            }

            rx = brick_get_x(brick);
            ry = brick_get_y(brick);
            rw = brick_get_width(brick);
            rh = brick_get_height(brick);

            if (bx < rx + rw && bx + BALL_SIZE > rx &&
                by < ry + rh && by + BALL_SIZE > ry) {

                brick_hit(brick);
                game->total_bricks--;
                game->score += brick_get_points(brick);

                // progresion de velocidad: cuenta golpes y filas naranja/roja
                game->hits++;
                if (r == ROW_ORANGE) {
                    game->hit_orange = true;
                }
                if (r == ROW_RED) {
                    game->hit_red = true;
                }
                game_update_speed(game);

                // se rebota por la cara mas cercana del ladrillo
                ol = (float) fabs(cx - rx);
                orr = (float) fabs(rx + rw - cx);
                ot = (float) fabs(cy - ry);
                ob = (float) fabs(ry + rh - cy);
                min_x = (ol < orr) ? ol : orr;
                min_y = (ot < ob) ? ot : ob;
                if (min_x < min_y) {
                    ball_bounce_x(game->ball);
                } else {
                    ball_bounce_y(game->ball);
                }

                if (game->snd_break) {
                    al_play_sample(game->snd_break, 0.5f, 0, 1.0f, ALLEGRO_PLAYMODE_ONCE, NULL);
                }

                if (game->total_bricks <= 0) {
                    if (game->screen < MAX_SCREENS) {
                        game->screen++;
                        game_reset_screen(game);
                    } else {
                        game->state = STATE_WIN;
                    }
                }

                return OK;
            }
        }
    }

    return OK;
}

// =========================================================================
// Functions: Rendering
// =========================================================================

/**
 * @brief Renders the current frame and flips the display.
 *
 * @param game Pointer to the game.
 * @return STATUS code (OK on success, ERROR if nothing to draw).
 */
STATUS game_render(GAME *game) {
    if (!game || !game->draw) {
        return ERROR;
    }

    al_clear_to_color(COLOR_BG);

    game_print_walls(game);
    game_print_bricks(game);

    if (game->state != STATE_TITLE) {
        paddle_print(game->paddle);
    }
    if (game->state == STATE_PLAY || game->state == STATE_DEAD) {
        ball_print(game->ball);
    }

    game_print_hud(game);
    game_print_overlay(game);

    al_flip_display();
    game->draw = false;

    return OK;
}

/**
 * @brief Draws a string of digits with the stretched blocky font (grey).
 *
 * @param str Null-terminated string of digit characters.
 * @param x Left x-coordinate of the first glyph.
 * @param y Top y-coordinate.
 * @param pw Pixel width of each font cell.
 * @param ph Pixel height of each font cell.
 */
static void game_draw_digits(const char *str, int x, int y, int pw, int ph) {
    int i = 0;
    int d = 0;
    int row = 0;
    int col = 0;

    for (i = 0; str[i]; i++) {
        d = str[i] - '0';
        if (d < 0 || d > 9) {
            x += 4 * pw;
            continue;
        }

        for (row = 0; row < 5; row++) {
            for (col = 0; col < 3; col++) {
                if (DIGITS[d][row] & (0x4 >> col)) {
                    al_draw_filled_rectangle(
                        x + col * pw, y + row * ph,
                        x + col * pw + pw, y + row * ph + ph, COLOR_SCORE);
                }
            }
        }

        x += 4 * pw; // glifo de 3 columnas + 1 de separacion
    }
}

/**
 * @brief Draws the grey frame: full-width top bar plus both side walls.
 *
 * @param game Pointer to the game.
 * @return STATUS code (OK on success, ERROR if game is NULL).
 */
static STATUS game_print_walls(GAME *game) {
    if (!game) {
        return ERROR;
    }

    al_draw_filled_rectangle(0, TOPBAR_Y, DISPLAY_WIDTH, SCOREBOARD_H, COLOR_WALL);
    al_draw_filled_rectangle(0, SCOREBOARD_H, WALL_THICKNESS, DISPLAY_HEIGHT, COLOR_WALL);
    al_draw_filled_rectangle(DISPLAY_WIDTH - WALL_THICKNESS, SCOREBOARD_H,
        DISPLAY_WIDTH, DISPLAY_HEIGHT, COLOR_WALL);

    return OK;
}

/**
 * @brief Draws every brick in the wall.
 *
 * @param game Pointer to the game.
 * @return STATUS code (OK on success, ERROR if game is NULL).
 */
static STATUS game_print_bricks(GAME *game) {
    int c = 0;
    int r = 0;

    if (!game) {
        return ERROR;
    }

    for (c = 0; c < BRICK_COLS; c++) {
        for (r = 0; r < BRICK_ROWS; r++) {
            brick_print(game->bricks[c][r]);
        }
    }

    return OK;
}

/**
 * @brief Draws the score band: score, current ball number and player number.
 *
 * Mirrors the original Atari 2600 layout: 3-digit score on the left, the ball
 * being played in the centre and the player number on the right.
 *
 * @param game Pointer to the game.
 * @return STATUS code (OK on success, ERROR if game is NULL).
 */
static STATUS game_print_hud(GAME *game) {
    char buf[8];
    int ball = 0;

    if (!game) {
        return ERROR;
    }

    sprintf(buf, "%03d", game->score % 1000);
    game_draw_digits(buf, SCORE_X, SCORE_Y, SCORE_PW, SCORE_PH);

    // numero de la bola en juego (1 .. MAX_LIVES)
    ball = MAX_LIVES - game->lives + 1;
    if (ball < 1) {
        ball = 1;
    }
    if (ball > 9) {
        ball = 9;
    }
    sprintf(buf, "%d", ball);
    game_draw_digits(buf, BALL_NUM_X, SCORE_Y, SCORE_PW, SCORE_PH);

    // numero de jugador (siempre 1 en esta version de un jugador)
    game_draw_digits("1", PLAYER_NUM_X, SCORE_Y, SCORE_PW, SCORE_PH);

    return OK;
}

/**
 * @brief Draws the state-dependent overlay text (title, win, over, dead).
 *
 * @param game Pointer to the game.
 * @return STATUS code (OK on success, ERROR if game or font is NULL).
 */
static STATUS game_print_overlay(GAME *game) {
    char buf[32];

    if (!game || !game->font) {
        return ERROR;
    }

    switch (game->state) {
        case STATE_TITLE:
            al_draw_text(game->font, al_map_rgb(255, 255, 255), DISPLAY_WIDTH / 2, 230,
                ALLEGRO_ALIGN_CENTER, "BREAKOUT");
            al_draw_text(game->font, al_map_rgb(200, 200, 200), DISPLAY_WIDTH / 2, 270,
                ALLEGRO_ALIGN_CENTER, "LEFT / RIGHT to move, SPACE to serve");
            if ((game->title_timer / 30) % 2) {
                al_draw_text(game->font, al_map_rgb(255, 255, 0), DISPLAY_WIDTH / 2, 310,
                    ALLEGRO_ALIGN_CENTER, "PRESS ENTER");
            }
            break;

        case STATE_OVER:
            al_draw_text(game->font, al_map_rgb(255, 0, 0), DISPLAY_WIDTH / 2, 190,
                ALLEGRO_ALIGN_CENTER, "GAME OVER");
            sprintf(buf, "SCORE %d", game->score);
            al_draw_text(game->font, al_map_rgb(255, 255, 255), DISPLAY_WIDTH / 2, 225,
                ALLEGRO_ALIGN_CENTER, buf);
            al_draw_text(game->font, al_map_rgb(200, 200, 200), DISPLAY_WIDTH / 2, 255,
                ALLEGRO_ALIGN_CENTER, "ENTER to continue");
            break;

        case STATE_WIN:
            al_draw_text(game->font, al_map_rgb(0, 255, 0), DISPLAY_WIDTH / 2, 190,
                ALLEGRO_ALIGN_CENTER, "YOU WIN!");
            sprintf(buf, "SCORE %d", game->score);
            al_draw_text(game->font, al_map_rgb(255, 255, 255), DISPLAY_WIDTH / 2, 225,
                ALLEGRO_ALIGN_CENTER, buf);
            al_draw_text(game->font, al_map_rgb(200, 200, 200), DISPLAY_WIDTH / 2, 255,
                ALLEGRO_ALIGN_CENTER, "ENTER to continue");
            break;

        case STATE_DEAD:
            al_draw_text(game->font, al_map_rgb(255, 0, 0), DISPLAY_WIDTH / 2, 250,
                ALLEGRO_ALIGN_CENTER, "BALL LOST");
            break;

        default:
            break;
    }

    return OK;
}
