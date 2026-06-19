/**
 * @file brick.h
 * @brief Declaration of the Brick structure and its associated functions.
 *
 * Bricks form the destructible wall at the top of the screen.
 * Each brick belongs to a specific row (colour and points) and
 * is destroyed after a single hit.
 *
 * Author: RGiskard7
 * Date: 18/06/2026
 */

#ifndef BRICK_H
#define BRICK_H

#include <allegro5/allegro.h>
#include "types.h"

typedef struct _brick BRICK;

/**
 * @brief Creates a new brick at the given grid position.
 *
 * @param row Row index (0 = top, determines colour and points).
 * @param col Column index (determines x position).
 * @return Pointer to the created BRICK or NULL on allocation failure.
 */
BRICK *brick_create(int row, int col);

/**
 * @brief Destroys the brick and frees allocated memory.
 *
 * @param brick Pointer to the BRICK to destroy.
 */
void brick_destroy(BRICK *brick);

// State
bool   brick_is_alive(BRICK *brick);
STATUS brick_hit(BRICK *brick);
STATUS brick_revive(BRICK *brick);

// Properties
int    brick_get_row(BRICK *brick);
int    brick_get_col(BRICK *brick);
int    brick_get_points(BRICK *brick);

// Position and dimensions
float  brick_get_x(BRICK *brick);
float  brick_get_y(BRICK *brick);
int    brick_get_width(BRICK *brick);
int    brick_get_height(BRICK *brick);

// Rendering
void brick_print(BRICK *brick);

#endif /* BRICK_H */
