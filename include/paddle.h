/**
 * @file paddle.h
 * @brief Declaration of the Paddle structure and its associated functions.
 *
 * The paddle represents the player-controlled bar at the bottom of the screen.
 * It moves horizontally within the game boundaries.
 *
 * Author: RGiskard7
 * Date: 18/06/2026
 */

#ifndef PADDLE_H
#define PADDLE_H

#include <allegro5/allegro.h>
#include "types.h"

typedef struct _paddle PADDLE;

/**
 * @brief Creates a new paddle at the default starting position.
 *
 * @return Pointer to the created PADDLE or NULL on allocation failure.
 */
PADDLE *paddle_create(void);

/**
 * @brief Destroys the paddle and frees allocated resources.
 *
 * @param paddle Pointer to the PADDLE to destroy.
 */
void paddle_destroy(PADDLE *paddle);

/**
 * @brief Sets the x-coordinate of the paddle.
 *
 * @param paddle Pointer to the PADDLE.
 * @param x New x-coordinate.
 * @return OK on success, ERROR if paddle is NULL.
 */
STATUS paddle_set_x(PADDLE *paddle, float x);

/**
 * @brief Gets the current x-coordinate of the paddle.
 *
 * @param paddle Pointer to the PADDLE.
 * @return The x-coordinate, or 0 if paddle is NULL.
 */
float paddle_get_x(PADDLE *paddle);

/**
 * @brief Gets the width of the paddle.
 *
 * @param paddle Pointer to the PADDLE.
 * @return The width, or 0 if paddle is NULL.
 */
int paddle_get_width(PADDLE *paddle);

/**
 * @brief Gets the height of the paddle.
 *
 * @param paddle Pointer to the PADDLE.
 * @return The height, or 0 if paddle is NULL.
 */
int paddle_get_height(PADDLE *paddle);

/**
 * @brief Moves the paddle to the left, bounded by screen edge.
 *
 * @param paddle Pointer to the PADDLE.
 */
void paddle_move_left(PADDLE *paddle);

/**
 * @brief Moves the paddle to the right, bounded by screen edge.
 *
 * @param paddle Pointer to the PADDLE.
 */
void paddle_move_right(PADDLE *paddle);

/**
 * @brief Renders the paddle on the screen.
 *
 * @param paddle Pointer to the PADDLE.
 * @return OK on success, ERROR if paddle is NULL.
 */
STATUS paddle_print(PADDLE *paddle);

#endif /* PADDLE_H */
