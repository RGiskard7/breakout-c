/**
 * @file paddle.c
 * @brief Manages the creation, movement, and rendering of the paddle
 * in the Breakout game.
 *
 * The paddle is the player-controlled element used to bounce the ball
 * toward the brick formation.
 *
 * Author: RGiskard7
 * Date: 18/06/2026
 */

#include "paddle.h"
#include "config.h"
#include <allegro5/allegro_primitives.h>
#include <stdlib.h>

/**
 * @struct _paddle
 * @brief Structure representing the player paddle.
 *
 * Holds the paddle's position and dimensions.
 */
struct _paddle {
    float x;           ///< Paddle x-coordinate
    int width;         ///< Paddle width in pixels
    int height;        ///< Paddle height in pixels
};

/**
 * @brief Creates a new paddle at the default starting position.
 *
 * Allocates memory for a PADDLE structure and initializes its
 * position and dimensions from config.h.
 *
 * @return Pointer to the created PADDLE or NULL on allocation failure.
 */
PADDLE *paddle_create(void) {
    PADDLE *new_paddle = NULL;

    new_paddle = (PADDLE *)malloc(sizeof(PADDLE));
    if (!new_paddle) {
        return NULL;
    }

    new_paddle->x = PADDLE_INIT_X;
    new_paddle->width = PADDLE_WIDTH;
    new_paddle->height = PADDLE_HEIGHT;

    return new_paddle;
}

/**
 * @brief Destroys the paddle and frees its allocated memory.
 *
 * @param paddle Pointer to the PADDLE to destroy.
 */
void paddle_destroy(PADDLE *paddle) {
    if (paddle != NULL) {
        free(paddle);
    }
}

/**
 * @brief Sets the x-coordinate of the paddle.
 *
 * @param paddle Pointer to the PADDLE.
 * @param x New x-coordinate.
 * @return OK on success, ERROR if paddle is NULL.
 */
STATUS paddle_set_x(PADDLE *paddle, float x) {
    if (!paddle) {
        return ERROR;
    }

    paddle->x = x;

    return OK;
}

/**
 * @brief Gets the current x-coordinate of the paddle.
 *
 * @param paddle Pointer to the PADDLE.
 * @return The x-coordinate, or 0 if paddle is NULL.
 */
float paddle_get_x(PADDLE *paddle) {
    if (!paddle) {
        return 0;
    }

    return paddle->x;
}

/**
 * @brief Gets the width of the paddle.
 *
 * @param paddle Pointer to the PADDLE.
 * @return The width, or 0 if paddle is NULL.
 */
int paddle_get_width(PADDLE *paddle) {
    if (!paddle) {
        return 0;
    }

    return paddle->width;
}

/**
 * @brief Gets the height of the paddle.
 *
 * @param paddle Pointer to the PADDLE.
 * @return The height, or 0 if paddle is NULL.
 */
int paddle_get_height(PADDLE *paddle) {
    if (!paddle) {
        return 0;
    }

    return paddle->height;
}

/**
 * @brief Moves the paddle to the left by PADDLE_SPEED, staying within bounds.
 *
 * @param paddle Pointer to the PADDLE.
 */
void paddle_move_left(PADDLE *paddle) {
    if (!paddle) {
        return;
    }

    if (paddle->x - PADDLE_SPEED >= 10) {
        paddle->x -= PADDLE_SPEED;
    }
}

/**
 * @brief Moves the paddle to the right by PADDLE_SPEED, staying within bounds.
 *
 * @param paddle Pointer to the PADDLE.
 */
void paddle_move_right(PADDLE *paddle) {
    if (!paddle) {
        return;
    }

    if (paddle->x + paddle->width + PADDLE_SPEED <= DISPLAY_WIDTH - 10) {
        paddle->x += PADDLE_SPEED;
    }
}

/**
 * @brief Renders the paddle as a filled rectangle on the screen.
 *
 * @param paddle Pointer to the PADDLE to render.
 * @return OK on success, ERROR if paddle is NULL.
 */
STATUS paddle_print(PADDLE *paddle) {
    if (!paddle) {
        return ERROR;
    }

    al_draw_filled_rectangle(paddle->x, PADDLE_Y,
        paddle->x + paddle->width, PADDLE_Y + paddle->height,
        al_map_rgb(0, 200, 255));

    return OK;
}
