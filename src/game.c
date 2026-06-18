/**
 * @file game.c
 * @brief Implementation of the Game structure and its associated functions.
 *
 * Contains the core game logic, state machine, collision detection,
 * and rendering for the Breakout game.
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

/**
 * @struct _game
 * @brief Main game structure containing all game components and state.
 *
 * Holds the display, timer, event queue, paddle, ball, brick grid,
 * score, lives, and state machine variables.
 */
struct _game {
  ALLEGRO_DISPLAY *display;                 ///< Main display
  ALLEGRO_TIMER *timer;                     ///< Main game timer
  ALLEGRO_EVENT_QUEUE *event_queue;         ///< Event queue
  ALLEGRO_EVENT events;                     ///< Current event
  ALLEGRO_FONT *font;                       ///< Font for text rendering
  ALLEGRO_SAMPLE *snd_bounce;               ///< Paddle/wall bounce sound
  ALLEGRO_SAMPLE *snd_break;                ///< Brick break sound

  PADDLE *paddle;                           ///< Player paddle
  BALL *ball;                               ///< Game ball
  BRICK *bricks[BRICK_COLS][BRICK_ROWS];    ///< Brick grid

  GAME_STATE state;                         ///< Current game state
  bool done;                                ///< Exit flag
  bool draw;                                ///< Redraw needed flag

  int total_bricks;                         ///< Active bricks remaining
  int bricks_destroyed;                     ///< Total bricks destroyed
  int score;                                ///< Player's score
  int lives;                                ///< Remaining lives
  int title_timer;                          ///< Timer for title screen blink
  int dead_timer;                           ///< Timer between ball loss and respawn
  bool space_was_down;                      ///< Previous space key state
};

// Speed stage lookup tables
static const float _speed_stages[] = {BALL_SPEED_INIT, BALL_SPEED_1,
                                       BALL_SPEED_2, BALL_SPEED_3};
static const int   _speed_thresholds[] = {0, STAGE_1, STAGE_2, STAGE_3};

// Function Declarations

static STATUS game_reset_bricks(GAME *game);            /**< Destroys + recreates all bricks */
static STATUS game_reset_ball(GAME *game);              /**< Attaches ball to paddle */
static STATUS game_next_ball(GAME *game);               /**< Handles ball loss and life count */
static void   game_update_speed(GAME *game);            /**< Updates ball speed stage */
static STATUS game_update_play(GAME *game, ALLEGRO_KEYBOARD_STATE *key);  /**< Playing state logic */
static STATUS game_colisions(GAME *game);               /**< Ball vs brick collision */

static STATUS game_print_bricks(GAME *game);            /**< Renders all bricks */
static STATUS game_print_paddle(GAME *game);            /**< Renders the paddle */
static STATUS game_print_ball(GAME *game);              /**< Renders the ball */
static STATUS game_print_hud(GAME *game);               /**< Renders score and lives */
static STATUS game_print_title(GAME *game);             /**< Renders title screen */
static STATUS game_print_gameover(GAME *game);          /**< Renders game over screen */
static STATUS game_print_win(GAME *game);               /**< Renders win screen */
static STATUS game_print_dead(GAME *game);              /**< Renders ball lost overlay */

// =========================================================================
// Functions: Game Creation and Destruction
// =========================================================================

/**
 * @brief Creates and initializes a new game instance.
 *
 * Allocates memory for the GAME structure and sets all fields
 * to their default values.
 *
 * @return Pointer to the new GAME instance or NULL if allocation fails.
 */
GAME *game_create(void) {
  GAME *new_game = NULL;

  new_game = (GAME *)malloc(sizeof(GAME));
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

  for (int c = 0; c < BRICK_COLS; c++) {
    for (int r = 0; r < BRICK_ROWS; r++) {
      new_game->bricks[c][r] = NULL;
    }
  }

  new_game->state = STATE_TITLE;
  new_game->done = false;
  new_game->draw = false;
  new_game->total_bricks = 0;
  new_game->bricks_destroyed = 0;
  new_game->score = 0;
  new_game->lives = MAX_LIVES;
  new_game->title_timer = 0;
  new_game->dead_timer = 0;
  new_game->space_was_down = true;

  return new_game;
}

/**
 * @brief Destroys the game instance and frees all associated resources.
 *
 * Destroys bricks, ball, paddle, sounds, font, event queue, timer,
 * display, and finally the GAME struct itself.
 *
 * @param game Pointer to the GAME instance to destroy.
 */
void game_destroy(GAME *game) {
  if (!game) {
    return;
  }

  for (int c = 0; c < BRICK_COLS; c++) {
    for (int r = 0; r < BRICK_ROWS; r++) {
      if (game->bricks[c][r] != NULL) {
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
 * @brief Initializes the game: creates display, timer, queue, loads
 * resources, and creates the paddle, ball, and bricks.
 *
 * @param game Pointer to the GAME instance.
 * @return OK if initialization succeeds, ERROR if any setup fails.
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

  al_register_event_source(game->event_queue,
      al_get_timer_event_source(game->timer));
  al_register_event_source(game->event_queue,
      al_get_display_event_source(game->display));
  al_register_event_source(game->event_queue,
      al_get_keyboard_event_source());

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

ALLEGRO_DISPLAY *game_get_display(GAME *game) {
  if (!game) { return NULL; }
  return game->display;
}

ALLEGRO_TIMER *game_get_timer(GAME *game) {
  if (!game) { return NULL; }
  return game->timer;
}

ALLEGRO_EVENT_QUEUE *game_get_queue(GAME *game) {
  if (!game) { return NULL; }
  return game->event_queue;
}

ALLEGRO_EVENT *game_get_event(GAME *game) {
  if (!game) { return NULL; }
  return &game->events;
}

bool game_is_done(GAME *game) {
  if (!game) { return true; }
  return game->done;
}

// =========================================================================
// Functions: Brick and Ball Helpers
// =========================================================================

/**
 * @brief Destroys all current bricks and recreates the full 10x6 grid.
 *
 * @param game Pointer to the GAME instance.
 * @return OK if successful, ERROR on allocation failure.
 */
static STATUS game_reset_bricks(GAME *game) {
  if (!game) return ERROR;

  for (int c = 0; c < BRICK_COLS; c++) {
    for (int r = 0; r < BRICK_ROWS; r++) {
      if (game->bricks[c][r] != NULL) {
        brick_destroy(game->bricks[c][r]);
      }
      game->bricks[c][r] = brick_create(r, c);
      if (!game->bricks[c][r]) {
        return ERROR;
      }
    }
  }

  game->total_bricks = BRICK_TOTAL;
  game->bricks_destroyed = 0;

  return OK;
}

/**
 * @brief Attaches the ball to the paddle and resets the space key state.
 *
 * @param game Pointer to the GAME instance.
 * @return OK if successful, ERROR if game, ball, or paddle is NULL.
 */
static STATUS game_reset_ball(GAME *game) {
  if (!game || !game->ball || !game->paddle) return ERROR;

  ball_attach_to_paddle(game->ball, paddle_get_x(game->paddle));
  game->space_was_down = true;

  return OK;
}

/**
 * @brief Decrements lives after ball loss. Triggers game over or dead state.
 *
 * @param game Pointer to the GAME instance.
 * @return OK if successful, ERROR if game is NULL.
 */
static STATUS game_next_ball(GAME *game) {
  if (!game) return ERROR;

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
 * @brief Updates the ball speed stage based on bricks destroyed.
 *
 * Checks the current count against speed thresholds and sets the
 * ball speed to the matching stage.
 *
 * @param game Pointer to the GAME instance.
 */
static void game_update_speed(GAME *game) {
  if (!game) return;

  int level = 0;
  for (int s = 3; s >= 0; s--) {
    if (game->bricks_destroyed >= _speed_thresholds[s]) {
      level = s;
      break;
    }
  }

  ball_set_speed(game->ball, _speed_stages[level]);
}

// =========================================================================
// Functions: Game Update
// =========================================================================

/**
 * @brief Main update function for the game.
 *
 * Processes keyboard input and updates the state machine each frame.
 *
 * @param game Pointer to the GAME instance.
 * @param key Pointer to the keyboard state.
 * @return OK if update is successful, ERROR if game is NULL.
 */
STATUS game_update(GAME *game, ALLEGRO_KEYBOARD_STATE *key) {
  if (!game || !key) {
    return ERROR;
  }

  if (game->state != STATE_PLAY && al_key_down(key, ALLEGRO_KEY_ESCAPE)) {
    game->done = true;
    return OK;
  }

  switch (game->state) {
    case STATE_TITLE:
      game->title_timer++;
      if (al_key_down(key, ALLEGRO_KEY_ENTER)) {
        game->state = STATE_PLAY;
        game->score = 0;
        game->lives = MAX_LIVES;
        game_reset_bricks(game);
        game_reset_ball(game);
        paddle_set_x(game->paddle, PADDLE_INIT_X);
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
  }

  game->draw = true;
  return OK;
}

/**
 * @brief Updates the game during the PLAY state.
 *
 * Handles paddle movement, ball launch, ball physics, wall and paddle
 * bounces, ball loss, and brick collisions.
 *
 * @param game Pointer to the GAME instance.
 * @param key Pointer to the keyboard state.
 * @return OK if update is successful, ERROR on failure.
 */
static STATUS game_update_play(GAME *game, ALLEGRO_KEYBOARD_STATE *key) {
  if (!game || !key) return ERROR;

  if (al_key_down(key, ALLEGRO_KEY_LEFT)) {
    paddle_move_left(game->paddle);
  }
  if (al_key_down(key, ALLEGRO_KEY_RIGHT)) {
    paddle_move_right(game->paddle);
  }

  bool space_down = al_key_down(key, ALLEGRO_KEY_SPACE);
  if (ball_is_serving(game->ball) && space_down && !game->space_was_down) {
    ball_launch(game->ball);
  }
  game->space_was_down = space_down;

  float px = paddle_get_x(game->paddle);
  ball_move(game->ball, px);

  if (!ball_is_serving(game->ball)) {
    ball_bounce_wall(game->ball);

    if (ball_bounce_paddle(game->ball, px) == OK) {
      if (game->snd_bounce) {
        al_play_sample(game->snd_bounce, 0.4f, 0.0f, 1.2f,
                       ALLEGRO_PLAYMODE_ONCE, NULL);
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
 * @brief Checks for collisions between the ball and bricks.
 *
 * Iterates all alive bricks and tests AABB overlap against the ball.
 * On collision, destroys the brick, updates score and speed,
 * and bounces the ball on the correct axis.
 *
 * @param game Pointer to the GAME instance.
 * @return OK if successful, ERROR if game is NULL.
 */
static STATUS game_colisions(GAME *game) {
  if (!game) return ERROR;

  float bx = ball_get_x(game->ball);
  float by = ball_get_y(game->ball);
  float cx = bx + BALL_SIZE / 2;
  float cy = by + BALL_SIZE / 2;

  for (int c = 0; c < BRICK_COLS; c++) {
    for (int r = 0; r < BRICK_ROWS; r++) {
      BRICK *brick = game->bricks[c][r];
      if (!brick || !brick_is_alive(brick)) continue;

      float rx = brick_get_x(brick);
      float ry = brick_get_y(brick);
      int rw = brick_get_width(brick);
      int rh = brick_get_height(brick);

      if (bx < rx + rw && bx + BALL_SIZE > rx &&
          by < ry + rh && by + BALL_SIZE > ry) {

        brick_hit(brick);
        game->total_bricks--;
        game->bricks_destroyed++;
        game->score += brick_get_points(brick);

        game_update_speed(game);

        // Determine bounce axis from minimum overlap
        float ol = (float)fabs(cx - rx);
        float or = (float)fabs(rx + rw - cx);
        float ot = (float)fabs(cy - ry);
        float ob = (float)fabs(ry + rh - cy);
        float min_x = (ol < or) ? ol : or;
        float min_y = (ot < ob) ? ot : ob;

        if (min_x < min_y) {
          ball_bounce_x(game->ball);
        } else {
          ball_bounce_y(game->ball);
        }

        if (game->snd_break) {
          al_play_sample(game->snd_break, 0.5f, 0.0f, 1.0f,
                         ALLEGRO_PLAYMODE_ONCE, NULL);
        }

        if (game->total_bricks <= 0) {
          game->state = STATE_WIN;
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
 * @brief Main render function to draw all game elements.
 *
 * Clears the screen, draws the playfield border, bricks, paddle, ball,
 * HUD, and state-specific overlays.
 *
 * @param game Pointer to the GAME instance.
 * @return OK if rendering is successful, ERROR if game is NULL
 *         or draw flag is false.
 */
STATUS game_render(GAME *game) {
  if (!game || !game->draw) {
    return ERROR;
  }

  al_clear_to_color(al_map_rgb(10, 10, 30));
  al_draw_rectangle(8, 8, DISPLAY_WIDTH - 8, DISPLAY_HEIGHT - 8,
      al_map_rgb(255, 255, 255), 2);

  game_print_bricks(game);

  if (game->state != STATE_TITLE) {
    game_print_paddle(game);
  }

  if (!ball_is_serving(game->ball) || game->state == STATE_DEAD ||
      game->state == STATE_PLAY) {
    game_print_ball(game);
  }

  game_print_hud(game);

  if (game->state == STATE_TITLE)  game_print_title(game);
  if (game->state == STATE_OVER)   game_print_gameover(game);
  if (game->state == STATE_WIN)    game_print_win(game);
  if (game->state == STATE_DEAD)   game_print_dead(game);

  al_flip_display();
  game->draw = false;

  return OK;
}

static STATUS game_print_bricks(GAME *game) {
  if (!game) return ERROR;
  for (int c = 0; c < BRICK_COLS; c++)
    for (int r = 0; r < BRICK_ROWS; r++)
      brick_print(game->bricks[c][r]);
  return OK;
}

static STATUS game_print_paddle(GAME *game) {
  return paddle_print(game->paddle);
}

static STATUS game_print_ball(GAME *game) {
  return ball_print(game->ball);
}

static STATUS game_print_hud(GAME *game) {
  if (!game || !game->font) return ERROR;

  char buf[32];
  sprintf(buf, "SCORE: %d", game->score);
  al_draw_text(game->font, al_map_rgb(255, 255, 255),
               20, DISPLAY_HEIGHT - 30, 0, buf);

  sprintf(buf, "LIVES: %d", game->lives);
  al_draw_text(game->font, al_map_rgb(255, 255, 255),
               DISPLAY_WIDTH - 140, DISPLAY_HEIGHT - 30, 0, buf);

  return OK;
}

static STATUS game_print_title(GAME *game) {
  if (!game || !game->font) return ERROR;

  al_draw_filled_rectangle(130, 220, 470, 380,
                           al_map_rgba(0, 0, 0, 200));

  al_draw_text(game->font, al_map_rgb(0, 255, 0),
               DISPLAY_WIDTH / 2, 240, ALLEGRO_ALIGN_CENTER,
               "BREAKOUT");
  al_draw_text(game->font, al_map_rgb(255, 255, 255),
               DISPLAY_WIDTH / 2, 300, ALLEGRO_ALIGN_CENTER,
               "PRESS ENTER TO START");

  if ((game->title_timer / 30) % 2) {
    al_draw_text(game->font, al_map_rgb(255, 255, 0),
                 DISPLAY_WIDTH / 2, 330, ALLEGRO_ALIGN_CENTER,
                 "-> ENTER <-");
  }

  return OK;
}

static STATUS game_print_gameover(GAME *game) {
  if (!game || !game->font) return ERROR;

  char buf[32];
  al_draw_filled_rectangle(150, 240, 450, 360,
                           al_map_rgba(0, 0, 0, 200));

  al_draw_text(game->font, al_map_rgb(255, 0, 0),
               DISPLAY_WIDTH / 2, 270, ALLEGRO_ALIGN_CENTER,
               "GAME OVER");

  sprintf(buf, "SCORE: %d", game->score);
  al_draw_text(game->font, al_map_rgb(255, 255, 255),
               DISPLAY_WIDTH / 2, 310, ALLEGRO_ALIGN_CENTER, buf);

  return OK;
}

static STATUS game_print_win(GAME *game) {
  if (!game || !game->font) return ERROR;

  char buf[32];
  al_draw_filled_rectangle(150, 240, 450, 360,
                           al_map_rgba(0, 0, 0, 200));

  al_draw_text(game->font, al_map_rgb(0, 255, 0),
               DISPLAY_WIDTH / 2, 270, ALLEGRO_ALIGN_CENTER,
               "YOU WIN!");

  sprintf(buf, "SCORE: %d", game->score);
  al_draw_text(game->font, al_map_rgb(255, 255, 255),
               DISPLAY_WIDTH / 2, 310, ALLEGRO_ALIGN_CENTER, buf);

  return OK;
}

static STATUS game_print_dead(GAME *game) {
  if (!game || !game->font) return ERROR;

  al_draw_text(game->font, al_map_rgb(255, 0, 0),
               DISPLAY_WIDTH / 2, DISPLAY_HEIGHT / 2,
               ALLEGRO_ALIGN_CENTER, "BALL LOST!");

  return OK;
}
