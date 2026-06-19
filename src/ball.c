/**
 * @file ball.c
 * @brief Implementation of the Ball structure and associated functions.
 *
 * The ball bounces off the grey walls, the ceiling, the paddle and the bricks.
 * Its speed is set in stages by the game logic (after 4 hits, after 12 hits and
 * upon reaching the orange and red rows), matching the original Atari 2600
 * Breakout progression.
 *
 * Author: RGiskard7
 * Date: 18/06/2026
 */

#include "ball.h"
#include "config.h"

#include <allegro5/allegro_primitives.h>
#include <stdlib.h>
#include <math.h>

struct _ball {
    float x, y;            ///< Ball's x and y coordinates (top-left corner)
    float vx, vy;          ///< Velocity components
    float speed;           ///< Current speed magnitude (px/frame)
    int size;              ///< Side length of the square ball
    bool serving;          ///< True while the ball rests on the paddle
    bool hit_ceiling;      ///< Raised on a ceiling bounce, consumed by the game
};

/**
 * @brief Creates a ball resting on the paddle, ready to be served.
 *
 * @return Pointer to the created ball or NULL if allocation fails.
 */
BALL *ball_create(void) {
    BALL *new_ball = NULL;

    new_ball = (BALL *) malloc(sizeof (BALL));
    if (!new_ball) {
        return NULL;
    }

    new_ball->x = 0;
    new_ball->y = 0;
    new_ball->vx = 0;
    new_ball->vy = 0;

    new_ball->speed = BALL_SPD_0;
    new_ball->size = BALL_SIZE;

    new_ball->serving = true;
    new_ball->hit_ceiling = false;

    return new_ball;
}

/**
 * @brief Destroys a ball and frees its memory.
 *
 * @param ball Pointer to the ball to destroy.
 */
void ball_destroy(BALL *ball) {
    if (ball != NULL) {
        free(ball);
    }
}

/**
 * @brief Retrieves the x-coordinate of the ball.
 *
 * @param ball Pointer to the ball.
 * @return X-coordinate, or 0 if ball is NULL.
 */
float ball_get_x(BALL *ball) {
    if (!ball) {
        return 0.0f;
    }

    return ball->x;
}

/**
 * @brief Retrieves the y-coordinate of the ball.
 *
 * @param ball Pointer to the ball.
 * @return Y-coordinate, or 0 if ball is NULL.
 */
float ball_get_y(BALL *ball) {
    if (!ball) {
        return 0.0f;
    }

    return ball->y;
}

/**
 * @brief Retrieves the current speed of the ball.
 *
 * @param ball Pointer to the ball.
 * @return Speed magnitude, or 0 if ball is NULL.
 */
float ball_get_speed(BALL *ball) {
    if (!ball) {
        return 0.0f;
    }

    return ball->speed;
}

/**
 * @brief Tells whether the ball is waiting to be served on the paddle.
 *
 * @param ball Pointer to the ball.
 * @return True if serving, true also if ball is NULL (nothing to move).
 */
bool ball_is_serving(BALL *ball) {
    if (!ball) {
        return true;
    }

    return ball->serving;
}

/**
 * @brief Returns and clears the "ball hit the ceiling" flag.
 *
 * Used by the game logic to halve the paddle the first time the ball
 * breaks through to the upper wall.
 *
 * @param ball Pointer to the ball.
 * @return True once after each ceiling bounce, false otherwise.
 */
bool ball_take_ceiling_hit(BALL *ball) {
    if (!ball || !ball->hit_ceiling) {
        return false;
    }

    ball->hit_ceiling = false;

    return true;
}

/**
 * @brief Sets the x-coordinate of the ball.
 *
 * @param ball Pointer to the ball.
 * @param x New x-coordinate.
 * @return STATUS code (OK on success, ERROR if ball is NULL).
 */
STATUS ball_set_x(BALL *ball, float x) {
    if (!ball) {
        return ERROR;
    }

    ball->x = x;

    return OK;
}

/**
 * @brief Sets the y-coordinate of the ball.
 *
 * @param ball Pointer to the ball.
 * @param y New y-coordinate.
 * @return STATUS code (OK on success, ERROR if ball is NULL).
 */
STATUS ball_set_y(BALL *ball, float y) {
    if (!ball) {
        return ERROR;
    }

    ball->y = y;

    return OK;
}

/**
 * @brief Sets the ball speed, keeping its current direction.
 *
 * The velocity vector is rescaled to the new magnitude so a mid-flight
 * speed change does not alter the trajectory.
 *
 * @param ball Pointer to the ball.
 * @param speed New speed magnitude.
 * @return STATUS code (OK on success, ERROR if ball is NULL).
 */
STATUS ball_set_speed(BALL *ball, float speed) {
    float mag = 0.0f;

    if (!ball) {
        return ERROR;
    }

    ball->speed = speed;

    mag = (float) sqrt(ball->vx * ball->vx + ball->vy * ball->vy);
    if (mag > 0) {
        ball->vx = ball->vx / mag * speed;
        ball->vy = ball->vy / mag * speed;
    }

    return OK;
}

/**
 * @brief Sets the serving state of the ball.
 *
 * @param ball Pointer to the ball.
 * @param serving New serving state.
 * @return STATUS code (OK on success, ERROR if ball is NULL).
 */
STATUS ball_set_serving(BALL *ball, bool serving) {
    if (!ball) {
        return ERROR;
    }

    ball->serving = serving;

    return OK;
}

/**
 * @brief Places the ball on top of the paddle, ready to be served.
 *
 * @param ball Pointer to the ball.
 * @param paddle_x Left x-coordinate of the paddle.
 * @param paddle_w Current paddle width.
 * @return STATUS code (OK on success, ERROR if ball is NULL).
 */
STATUS ball_attach_to_paddle(BALL *ball, float paddle_x, int paddle_w) {
    if (!ball) {
        return ERROR;
    }

    ball->x = paddle_x + paddle_w / 2.0f - ball->size / 2.0f;
    ball->y = PADDLE_Y - ball->size - 2;

    ball->vx = 0;
    ball->vy = 0;

    ball->serving = true;
    ball->hit_ceiling = false;

    return OK;
}

/**
 * @brief Launches the ball upward at a shallow random angle.
 *
 * @param ball Pointer to the ball.
 * @return STATUS code (OK on success, ERROR if ball is NULL).
 */
STATUS ball_launch(BALL *ball) {
    float angle = 0.0f;

    if (!ball) {
        return ERROR;
    }

    // angulo de saque aleatorio, hacia arriba a velocidad completa
    angle = ((rand() % 1000) / 1000.0f - 0.5f) * BALL_MAX_ANGLE;

    ball->vx = (float) sin(angle) * ball->speed;
    ball->vy = -(float) cos(angle) * ball->speed;

    ball->serving = false;

    return OK;
}

/**
 * @brief Advances the ball one frame, or keeps it on the paddle when serving.
 *
 * @param ball Pointer to the ball.
 * @param paddle_x Left x-coordinate of the paddle.
 * @param paddle_w Current paddle width.
 * @return STATUS code (OK on success, ERROR if ball is NULL).
 */
STATUS ball_move(BALL *ball, float paddle_x, int paddle_w) {
    if (!ball) {
        return ERROR;
    }

    if (ball->serving) {
        ball->x = paddle_x + paddle_w / 2.0f - ball->size / 2.0f;
        return OK;
    }

    ball->x += ball->vx;
    ball->y += ball->vy;

    return OK;
}

/**
 * @brief Bounces the ball off the side walls and the ceiling.
 *
 * @param ball Pointer to the ball.
 * @return STATUS code (OK on success, ERROR if ball is NULL).
 */
STATUS ball_bounce_wall(BALL *ball) {
    if (!ball) {
        return ERROR;
    }

    if (ball->x < PLAYFIELD_LEFT) {
        ball->x = PLAYFIELD_LEFT;
        ball->vx = -ball->vx;
    }

    if (ball->x > PLAYFIELD_RIGHT - ball->size) {
        ball->x = PLAYFIELD_RIGHT - ball->size;
        ball->vx = -ball->vx;
    }

    if (ball->y < PLAYFIELD_TOP) {
        ball->y = PLAYFIELD_TOP;
        ball->vy = -ball->vy;
        ball->hit_ceiling = true;
    }

    return OK;
}

/**
 * @brief Bounces the ball off the paddle, deflecting by the impact point.
 *
 * Hitting the centre sends the ball straight up; the edges give the widest
 * angle. Returns ERROR when there is no contact this frame.
 *
 * @param ball Pointer to the ball.
 * @param paddle_x Left x-coordinate of the paddle.
 * @param paddle_w Current paddle width.
 * @return OK if the ball bounced, ERROR otherwise.
 */
STATUS ball_bounce_paddle(BALL *ball, float paddle_x, int paddle_w) {
    float hit = 0.0f;
    float angle = 0.0f;

    if (!ball) {
        return ERROR;
    }

    if (ball->y + ball->size >= PADDLE_Y &&
        ball->y <= PADDLE_Y + PADDLE_HEIGHT &&
        ball->x + ball->size > paddle_x &&
        ball->x < paddle_x + paddle_w &&
        ball->vy > 0) {

        ball->y = PADDLE_Y - ball->size;

        // posicion relativa del impacto (0 = izquierda, 1 = derecha)
        hit = (ball->x + ball->size / 2.0f - paddle_x) / paddle_w;
        if (hit < 0) {
            hit = 0;
        }
        if (hit > 1) {
            hit = 1;
        }

        angle = (hit - 0.5f) * BALL_MAX_ANGLE;
        ball->vx = (float) sin(angle) * ball->speed;
        ball->vy = -(float) cos(angle) * ball->speed;

        return OK;
    }

    return ERROR;
}

/**
 * @brief Reverses the horizontal velocity, keeping the speed magnitude.
 *
 * @param ball Pointer to the ball.
 * @return STATUS code (OK on success, ERROR if ball is NULL).
 */
STATUS ball_bounce_x(BALL *ball) {
    float mag = 0.0f;

    if (!ball) {
        return ERROR;
    }

    ball->vx = -ball->vx;

    mag = (float) sqrt(ball->vx * ball->vx + ball->vy * ball->vy);
    if (mag > 0) {
        ball->vx = ball->vx / mag * ball->speed;
        ball->vy = ball->vy / mag * ball->speed;
    }

    return OK;
}

/**
 * @brief Reverses the vertical velocity, keeping the speed magnitude.
 *
 * @param ball Pointer to the ball.
 * @return STATUS code (OK on success, ERROR if ball is NULL).
 */
STATUS ball_bounce_y(BALL *ball) {
    float mag = 0.0f;

    if (!ball) {
        return ERROR;
    }

    ball->vy = -ball->vy;

    mag = (float) sqrt(ball->vx * ball->vx + ball->vy * ball->vy);
    if (mag > 0) {
        ball->vx = ball->vx / mag * ball->speed;
        ball->vy = ball->vy / mag * ball->speed;
    }

    return OK;
}

/**
 * @brief Draws the ball on the screen.
 *
 * @param ball Pointer to the ball.
 * @return STATUS code (OK on success, ERROR if ball is NULL).
 */
STATUS ball_print(BALL *ball) {
    if (!ball) {
        return ERROR;
    }

    al_draw_filled_rectangle(ball->x, ball->y,
        ball->x + ball->size, ball->y + ball->size, COLOR_BALL);

    return OK;
}
