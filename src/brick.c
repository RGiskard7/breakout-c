/**
 * @file brick.c
 * @brief Manages the creation, destruction, and rendering of bricks
 * in the Breakout game.
 *
 * Bricks are arranged in a 10x6 grid. Each row has a distinct colour
 * and point value. Bricks are destroyed on a single ball hit.
 *
 * Author: RGiskard7
 * Date: 18/06/2026
 */

#include "brick.h"
#include "config.h"
#include <allegro5/allegro_primitives.h>
#include <stdlib.h>

///< Brick colours per row (top to bottom): red, orange, yellow (two rows each)
static const ALLEGRO_COLOR _brick_colors[BRICK_ROWS] = {
    {1.0f, 0.3f, 0.3f, 1.0f},
    {1.0f, 0.3f, 0.3f, 1.0f},
    {1.0f, 0.6f, 0.0f, 1.0f},
    {1.0f, 0.6f, 0.0f, 1.0f},
    {1.0f, 0.85f, 0.0f, 1.0f},
    {1.0f, 0.85f, 0.0f, 1.0f}
};

///< Point values per row
static const int _brick_points[BRICK_ROWS] = {
    PTS_ROW_0, PTS_ROW_1, PTS_ROW_2, PTS_ROW_3, PTS_ROW_4, PTS_ROW_5
};

/**
 * @struct _brick
 * @brief Structure representing a single brick.
 *
 * Stores the brick's grid position and alive/dead state.
 * Position on screen is calculated from row and column indices.
 */
struct _brick {
    int row;         ///< Row index (0 = top)
    int col;         ///< Column index (0 = left)
    bool alive;      ///< True if not yet destroyed
};

/**
 * @brief Creates a new brick at the given grid coordinates.
 *
 * @param row Row index (determines colour and points).
 * @param col Column index (determines horizontal position).
 * @return Pointer to the created BRICK or NULL on allocation failure.
 */
BRICK *brick_create(int row, int col) {
    BRICK *new_brick = NULL;

    new_brick = (BRICK *)malloc(sizeof(BRICK));
    if (!new_brick) {
        return NULL;
    }

    new_brick->row = row;
    new_brick->col = col;
    new_brick->alive = true;

    return new_brick;
}

/**
 * @brief Destroys the brick and frees allocated memory.
 *
 * @param brick Pointer to the BRICK to destroy.
 */
void brick_destroy(BRICK *brick) {
    if (brick != NULL) {
        free(brick);
    }
}

/**
 * @brief Checks if the brick is still alive.
 *
 * @param brick Pointer to the BRICK.
 * @return true if alive, false otherwise.
 */
bool brick_is_alive(BRICK *brick) {
    if (!brick) {
        return false;
    }
    return brick->alive;
}

/**
 * @brief Marks the brick as destroyed.
 *
 * @param brick Pointer to the BRICK.
 * @return OK on success, ERROR if brick is NULL.
 */
STATUS brick_hit(BRICK *brick) {
    if (!brick) {
        return ERROR;
    }
    brick->alive = false;
    return OK;
}

/**
 * @brief Gets the brick's row index.
 *
 * @param brick Pointer to the BRICK.
 * @return Row index, or 0 if brick is NULL.
 */
int brick_get_row(BRICK *brick) {
    if (!brick) {
        return 0;
    }
    return brick->row;
}

/**
 * @brief Gets the brick's column index.
 *
 * @param brick Pointer to the BRICK.
 * @return Column index, or 0 if brick is NULL.
 */
int brick_get_col(BRICK *brick) {
    if (!brick) {
        return 0;
    }
    return brick->col;
}

/**
 * @brief Gets the point value for this brick.
 *
 * Points are determined by the brick's row.
 *
 * @param brick Pointer to the BRICK.
 * @return Point value, or 0 if brick is NULL.
 */
int brick_get_points(BRICK *brick) {
    if (!brick) {
        return 0;
    }
    return _brick_points[brick->row];
}

/**
 * @brief Gets the screen x-coordinate of the brick.
 *
 * Calculated from the brick's column index and BRICK_WIDTH + BRICK_GAP.
 *
 * @param brick Pointer to the BRICK.
 * @return X-coordinate, or 0 if brick is NULL.
 */
float brick_get_x(BRICK *brick) {
    if (!brick) {
        return 0;
    }
    return BRICK_X0 + brick->col * (BRICK_WIDTH + BRICK_GAP);
}

/**
 * @brief Gets the screen y-coordinate of the brick.
 *
 * Calculated from the brick's row index and BRICK_HEIGHT + BRICK_GAP.
 *
 * @param brick Pointer to the BRICK.
 * @return Y-coordinate, or 0 if brick is NULL.
 */
float brick_get_y(BRICK *brick) {
    if (!brick) {
        return 0;
    }
    return BRICK_Y0 + brick->row * (BRICK_HEIGHT + BRICK_GAP);
}

/**
 * @brief Gets the brick width.
 *
 * @param brick Pointer to the BRICK (unused, all bricks same size).
 * @return BRICK_WIDTH constant.
 */
int brick_get_width(BRICK *brick) {
    (void)brick;
    return BRICK_WIDTH;
}

/**
 * @brief Gets the brick height.
 *
 * @param brick Pointer to the BRICK (unused, all bricks same size).
 * @return BRICK_HEIGHT constant.
 */
int brick_get_height(BRICK *brick) {
    (void)brick;
    return BRICK_HEIGHT;
}

/**
 * @brief Renders the brick as a filled rectangle with its row colour.
 *
 * Inactive (destroyed) bricks are not drawn.
 *
 * @param brick Pointer to the BRICK.
 */
void brick_print(BRICK *brick) {
    if (!brick || !brick->alive) {
        return;
    }

    float x = brick_get_x(brick);
    float y = brick_get_y(brick);

    al_draw_filled_rectangle(x, y, x + BRICK_WIDTH, y + BRICK_HEIGHT,
                             _brick_colors[brick->row]);
    al_draw_rectangle(x, y, x + BRICK_WIDTH, y + BRICK_HEIGHT,
                      al_map_rgb(40, 40, 40), 1);
}
