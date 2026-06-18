/**
 * @file ball.c
 * @brief Manages the creation, movement, physics, and rendering of the ball
 * in the Breakout game.
 *
 * The ball bounces off walls, the paddle (with angle control), and bricks.
 * Its speed increases in four stages as bricks are destroyed, matching
 * the original 1976 arcade behaviour.
 *
 * Author: RGiskard7
 * Date: 18/06/2026
 */

#include "ball.h"
#include "config.h"
#include <allegro5/allegro_primitives.h>
#include <stdlib.h>
#include <math.h>

/**
 * @struct _ball
 * @brief Structure representing the ball in the game.
 *
 * Holds the ball's position, velocity, speed stage, size,
 * and serving state (attached to paddle before launch).
 */
struct _ball {
    float x, y;       ///< Ball coordinates
    float vx, vy;     ///< Velocity components
    float speed;      ///< Current speed magnitude
    int size;         ///< Ball diameter in pixels
    bool serving;     ///< True when attached to paddle
};

/**
 * @brief Creates a new ball with default values.
 *
 * The ball starts serving (attached to paddle) with initial speed
 * from config.h.
 *
 * @return Pointer to the created BALL or NULL on allocation failure.
 */
BALL *ball_create(void) {
    BALL *new_ball = NULL;

    new_ball = (BALL *)malloc(sizeof(BALL));
    if (!new_ball) {
        return NULL;
    }

    new_ball->x = 0;
    new_ball->y = 0;
    new_ball->vx = 0;
    new_ball->vy = 0;
    new_ball->speed = BALL_SPEED_INIT;
    new_ball->size = BALL_SIZE;
    new_ball->serving = true;

    return new_ball;
}

/**
 * @brief Destroys the ball and frees allocated memory.
 *
 * @param ball Pointer to the BALL to destroy.
 */
void ball_destroy(BALL *ball) {
    if (ball != NULL) {
        free(ball);
    }
}

// Position getters

float ball_get_x(BALL *ball) {
    if (!ball) {
        return 0;
    }
    return ball->x;
}

float ball_get_y(BALL *ball) {
    if (!ball) {
        return 0;
    }
    return ball->y;
}

float ball_get_speed(BALL *ball) {
    if (!ball) {
        return 0;
    }
    return ball->speed;
}

bool ball_is_serving(BALL *ball) {
    if (!ball) {
        return true;
    }
    return ball->serving;
}

// Position setters

STATUS ball_set_x(BALL *ball, float x) {
    if (!ball) {
        return ERROR;
    }
    ball->x = x;
    return OK;
}

STATUS ball_set_y(BALL *ball, float y) {
    if (!ball) {
        return ERROR;
    }
    ball->y = y;
    return OK;
}

STATUS ball_set_speed(BALL *ball, float speed) {
    if (!ball) {
        return ERROR;
    }
    ball->speed = speed;
    return OK;
}

STATUS ball_set_serving(BALL *ball, bool serving) {
    if (!ball) {
        return ERROR;
    }
    ball->serving = serving;
    return OK;
}

// =========================================================================
// Functions: Movement and Physics
// =========================================================================

/**
 * @brief Attaches the ball to the paddle, ready to serve.
 *
 * Positions the ball centred on top of the paddle and resets
 * its velocity and speed to initial values.
 *
 * @param ball Pointer to the BALL.
 * @param paddle_x Current x-coordinate of the paddle.
 * @return OK on success, ERROR if ball is NULL.
 */
STATUS ball_attach_to_paddle(BALL *ball, float paddle_x) {
    if (!ball) {
        return ERROR;
    }

    ball->x = paddle_x + PADDLE_WIDTH / 2 - ball->size / 2;
    ball->y = PADDLE_Y - ball->size - 2;
    ball->vx = 0;
    ball->vy = 0;
    ball->speed = BALL_SPEED_INIT;
    ball->serving = true;

    return OK;
}

/**
 * @brief Launches the ball from the paddle with a random horizontal angle.
 *
 * @param ball Pointer to the BALL.
 * @return OK on success, ERROR if ball is NULL.
 */
STATUS ball_launch(BALL *ball) {
    if (!ball) {
        return ERROR;
    }

    ball->vx = (float)(rand() % 5 - 2) * 1.3f;
    ball->vy = -ball->speed;
    ball->serving = false;

    return OK;
}

/**
 * @brief Moves the ball by its velocity. If serving, follows the paddle.
 *
 * @param ball Pointer to the BALL.
 * @param paddle_x Current x-coordinate of the paddle.
 * @return OK on success, ERROR if ball is NULL.
 */
STATUS ball_move(BALL *ball, float paddle_x) {
    if (!ball) {
        return ERROR;
    }

    if (ball->serving) {
        ball->x = paddle_x + PADDLE_WIDTH / 2 - ball->size / 2;
        return OK;
    }

    ball->x += ball->vx;
    ball->y += ball->vy;

    return OK;
}

/**
 * @brief Bounces the ball off the screen walls (left, right, top).
 *
 * Clamps position to boundaries and reverses the corresponding
 * velocity component.
 *
 * @param ball Pointer to the BALL.
 * @return OK on success, ERROR if ball is NULL.
 */
STATUS ball_bounce_wall(BALL *ball) {
    if (!ball) {
        return ERROR;
    }

    if (ball->x < 10) {
        ball->x = 10;
        ball->vx = -ball->vx;
    }
    if (ball->x > DISPLAY_WIDTH - 10 - ball->size) {
        ball->x = DISPLAY_WIDTH - 10 - ball->size;
        ball->vx = -ball->vx;
    }
    if (ball->y < 10) {
        ball->y = 10;
        ball->vy = -ball->vy;
    }

    return OK;
}

/**
 * @brief Bounces the ball off the paddle with angle control.
 *
 * The horizontal hit position on the paddle determines the bounce angle.
 * Hitting the edges produces steeper angles.
 *
 * @param ball Pointer to the BALL.
 * @param paddle_x Current x-coordinate of the paddle.
 * @return OK if bounce occurred, ERROR otherwise.
 */
STATUS ball_bounce_paddle(BALL *ball, float paddle_x) {
    if (!ball) {
        return ERROR;
    }

    if (ball->y + ball->size >= PADDLE_Y &&
        ball->y <= PADDLE_Y + PADDLE_HEIGHT &&
        ball->x + ball->size > paddle_x &&
        ball->x < paddle_x + PADDLE_WIDTH &&
        ball->vy > 0) {

        ball->y = PADDLE_Y - ball->size;

        float hit = (ball->x + ball->size / 2 - paddle_x) / PADDLE_WIDTH;
        float angle = (hit - 0.5f) * BALL_MAX_ANGLE;
        float current_speed = (float)sqrt(ball->vx * ball->vx +
                                          ball->vy * ball->vy);
        ball->vx = (float)sin(angle) * current_speed;
        ball->vy = -(float)cos(angle) * current_speed;

        return OK;
    }

    return ERROR;
}

/**
 * @brief Bounces the ball horizontally (reverses vx).
 *
 * Normalises velocity to the current speed stage.
 *
 * @param ball Pointer to the BALL.
 * @return OK on success, ERROR if ball is NULL.
 */
STATUS ball_bounce_x(BALL *ball) {
    if (!ball) {
        return ERROR;
    }

    ball->vx = -ball->vx;

    float mag = (float)sqrt(ball->vx * ball->vx + ball->vy * ball->vy);
    if (mag > 0) {
        ball->vx = ball->vx / mag * ball->speed;
        ball->vy = ball->vy / mag * ball->speed;
    }

    return OK;
}

/**
 * @brief Bounces the ball vertically (reverses vy).
 *
 * Normalises velocity to the current speed stage.
 *
 * @param ball Pointer to the BALL.
 * @return OK on success, ERROR if ball is NULL.
 */
STATUS ball_bounce_y(BALL *ball) {
    if (!ball) {
        return ERROR;
    }

    ball->vy = -ball->vy;

    float mag = (float)sqrt(ball->vx * ball->vx + ball->vy * ball->vy);
    if (mag > 0) {
        ball->vx = ball->vx / mag * ball->speed;
        ball->vy = ball->vy / mag * ball->speed;
    }

    return OK;
}

// =========================================================================
// Functions: Rendering
// =========================================================================

/**
 * @brief Renders the ball as a filled white circle.
 *
 * @param ball Pointer to the BALL.
 * @return OK on success, ERROR if ball is NULL.
 */
STATUS ball_print(BALL *ball) {
    if (!ball) {
        return ERROR;
    }

    al_draw_filled_circle(ball->x + ball->size / 2,
                          ball->y + ball->size / 2,
                          ball->size / 2,
                          al_map_rgb(255, 255, 255));

    return OK;
}
