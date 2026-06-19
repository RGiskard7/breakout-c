/**
 * @file paddle.c
 * @brief Implementation of the Paddle structure and associated functions.
 *
 * The paddle is the player-controlled bar at the bottom of the screen. It is a
 * solid red bar (Atari 2600 colour) and halves its width for the rest of a
 * screen once the ball breaks through to the ceiling.
 *
 * Author: RGiskard7
 * Date: 18/06/2026
 */

#include "paddle.h"
#include "config.h"

#include <allegro5/allegro_primitives.h>
#include <stdlib.h>

struct _paddle {
    float x;        ///< Left x-coordinate of the paddle
    int width;      ///< Current width (full or halved)
    int height;     ///< Paddle height
};

/**
 * @brief Creates a paddle centred at the bottom of the playfield.
 *
 * @return Pointer to the created paddle or NULL if allocation fails.
 */
PADDLE *paddle_create(void) {
    PADDLE *new_paddle = NULL;

    new_paddle = (PADDLE *) malloc(sizeof (PADDLE));
    if (!new_paddle) {
        return NULL;
    }

    new_paddle->x = PADDLE_INIT_X;
    new_paddle->width = PADDLE_WIDTH;
    new_paddle->height = PADDLE_HEIGHT;

    return new_paddle;
}

/**
 * @brief Destroys a paddle and frees its memory.
 *
 * @param paddle Pointer to the paddle to destroy.
 */
void paddle_destroy(PADDLE *paddle) {
    if (paddle != NULL) {
        free(paddle);
    }
}

/**
 * @brief Sets the x-coordinate of the paddle.
 *
 * @param paddle Pointer to the paddle.
 * @param x New x-coordinate.
 * @return STATUS code (OK on success, ERROR if paddle is NULL).
 */
STATUS paddle_set_x(PADDLE *paddle, float x) {
    if (!paddle) {
        return ERROR;
    }

    paddle->x = x;

    return OK;
}

/**
 * @brief Retrieves the x-coordinate of the paddle.
 *
 * @param paddle Pointer to the paddle.
 * @return X-coordinate, or 0 if paddle is NULL.
 */
float paddle_get_x(PADDLE *paddle) {
    if (!paddle) {
        return 0.0f;
    }

    return paddle->x;
}

/**
 * @brief Retrieves the width of the paddle.
 *
 * @param paddle Pointer to the paddle.
 * @return Width, or 0 if paddle is NULL.
 */
int paddle_get_width(PADDLE *paddle) {
    if (!paddle) {
        return 0;
    }

    return paddle->width;
}

/**
 * @brief Retrieves the height of the paddle.
 *
 * @param paddle Pointer to the paddle.
 * @return Height, or 0 if paddle is NULL.
 */
int paddle_get_height(PADDLE *paddle) {
    if (!paddle) {
        return 0;
    }

    return paddle->height;
}

/**
 * @brief Sets the paddle width (used to halve it after a ceiling break).
 *
 * The paddle stays centred on itself and is clamped inside the side walls.
 *
 * @param paddle Pointer to the paddle.
 * @param width New width in pixels.
 * @return STATUS code (OK on success, ERROR if paddle is NULL).
 */
STATUS paddle_set_width(PADDLE *paddle, int width) {
    if (!paddle) {
        return ERROR;
    }

    // se reposiciona para mantener el centro al cambiar de tamano
    paddle->x += (paddle->width - width) / 2.0f;
    paddle->width = width;

    if (paddle->x < PLAYFIELD_LEFT) {
        paddle->x = PLAYFIELD_LEFT;
    }
    if (paddle->x + width > PLAYFIELD_RIGHT) {
        paddle->x = PLAYFIELD_RIGHT - width;
    }

    return OK;
}

/**
 * @brief Moves the paddle to the left, bounded by the side wall.
 *
 * @param paddle Pointer to the paddle.
 */
void paddle_move_left(PADDLE *paddle) {
    if (!paddle) {
        return;
    }

    if (paddle->x - PADDLE_SPEED >= PLAYFIELD_LEFT) {
        paddle->x -= PADDLE_SPEED;
    } else {
        paddle->x = PLAYFIELD_LEFT;
    }
}

/**
 * @brief Moves the paddle to the right, bounded by the side wall.
 *
 * @param paddle Pointer to the paddle.
 */
void paddle_move_right(PADDLE *paddle) {
    if (!paddle) {
        return;
    }

    if (paddle->x + paddle->width + PADDLE_SPEED <= PLAYFIELD_RIGHT) {
        paddle->x += PADDLE_SPEED;
    } else {
        paddle->x = PLAYFIELD_RIGHT - paddle->width;
    }
}

/**
 * @brief Draws the paddle on the screen.
 *
 * @param paddle Pointer to the paddle.
 * @return STATUS code (OK on success, ERROR if paddle is NULL).
 */
STATUS paddle_print(PADDLE *paddle) {
    if (!paddle) {
        return ERROR;
    }

    al_draw_filled_rectangle(paddle->x, PADDLE_Y,
        paddle->x + paddle->width, PADDLE_Y + paddle->height, COLOR_PADDLE);

    return OK;
}
