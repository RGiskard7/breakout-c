/**
 * @file brick.c
 * @brief Implementation of the Brick structure and associated functions.
 *
 * Bricks form the destructible wall at the top of the screen. Each brick belongs
 * to a row that determines its colour and point value, and is destroyed after a
 * single hit. Rows are drawn edge to edge so a full row looks like a solid
 * coloured band and destroyed bricks leave black holes, just like the original.
 *
 * Author: RGiskard7
 * Date: 18/06/2026
 */

#include "brick.h"
#include "config.h"

#include <allegro5/allegro_primitives.h>
#include <stdlib.h>

/**
 * @brief Point value of every brick row, top to bottom.
 *
 * Red/orange = 7, yellow rows = 4, green/blue = 1 (Atari 2600 scoring).
 */
static const int _points[BRICK_ROWS] = {
    PTS_ROW_0, PTS_ROW_1, PTS_ROW_2, PTS_ROW_3, PTS_ROW_4, PTS_ROW_5
};

struct _brick {
    int row;        ///< Row index (0 = top, sets colour and points)
    int col;        ///< Column index (sets x position)
    bool alive;     ///< False once the brick has been destroyed
};

/**
 * @brief Returns the authentic Atari 2600 colour of a brick row.
 *
 * @param row Row index (0 = top).
 * @return Allegro colour for that row.
 */
static ALLEGRO_COLOR brick_row_color(int row) {
    switch (row) {
        case 0:
            return COLOR_ROW_0;
        case 1:
            return COLOR_ROW_1;
        case 2:
            return COLOR_ROW_2;
        case 3:
            return COLOR_ROW_3;
        case 4:
            return COLOR_ROW_4;
        default:
            return COLOR_ROW_5;
    }
}

/**
 * @brief Creates a brick at the given grid position.
 *
 * @param row Row index (0 = top, determines colour and points).
 * @param col Column index (determines x position).
 * @return Pointer to the created brick or NULL if allocation fails.
 */
BRICK *brick_create(int row, int col) {
    BRICK *new_brick = NULL;

    new_brick = (BRICK *) malloc(sizeof (BRICK));
    if (!new_brick) {
        return NULL;
    }

    new_brick->row = row;
    new_brick->col = col;
    new_brick->alive = true;

    return new_brick;
}

/**
 * @brief Destroys a brick and frees its memory.
 *
 * @param brick Pointer to the brick to destroy.
 */
void brick_destroy(BRICK *brick) {
    if (brick != NULL) {
        free(brick);
    }
}

/**
 * @brief Tells whether the brick is still standing.
 *
 * @param brick Pointer to the brick.
 * @return True if alive, false if destroyed or NULL.
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
 * @param brick Pointer to the brick.
 * @return STATUS code (OK on success, ERROR if brick is NULL).
 */
STATUS brick_hit(BRICK *brick) {
    if (!brick) {
        return ERROR;
    }

    brick->alive = false;

    return OK;
}

/**
 * @brief Brings the brick back to life (used when rebuilding the wall).
 *
 * @param brick Pointer to the brick.
 * @return STATUS code (OK on success, ERROR if brick is NULL).
 */
STATUS brick_revive(BRICK *brick) {
    if (!brick) {
        return ERROR;
    }

    brick->alive = true;

    return OK;
}

/**
 * @brief Retrieves the row index of the brick.
 *
 * @param brick Pointer to the brick.
 * @return Row index, or 0 if brick is NULL.
 */
int brick_get_row(BRICK *brick) {
    if (!brick) {
        return 0;
    }

    return brick->row;
}

/**
 * @brief Retrieves the column index of the brick.
 *
 * @param brick Pointer to the brick.
 * @return Column index, or 0 if brick is NULL.
 */
int brick_get_col(BRICK *brick) {
    if (!brick) {
        return 0;
    }

    return brick->col;
}

/**
 * @brief Retrieves the point value of the brick.
 *
 * @param brick Pointer to the brick.
 * @return Points awarded for this brick, or 0 if brick is NULL.
 */
int brick_get_points(BRICK *brick) {
    if (!brick) {
        return 0;
    }

    return _points[brick->row];
}

/**
 * @brief Retrieves the x-coordinate of the brick's left edge.
 *
 * @param brick Pointer to the brick.
 * @return X-coordinate, or 0 if brick is NULL.
 */
float brick_get_x(BRICK *brick) {
    if (!brick) {
        return 0.0f;
    }

    return BRICK_X0 + brick->col * (BRICK_WIDTH + BRICK_GAP);
}

/**
 * @brief Retrieves the y-coordinate of the brick's top edge.
 *
 * @param brick Pointer to the brick.
 * @return Y-coordinate, or 0 if brick is NULL.
 */
float brick_get_y(BRICK *brick) {
    if (!brick) {
        return 0.0f;
    }

    return BRICK_Y0 + brick->row * (BRICK_HEIGHT + BRICK_GAP);
}

/**
 * @brief Retrieves the width of the brick.
 *
 * @param brick Pointer to the brick.
 * @return Brick width in pixels.
 */
int brick_get_width(BRICK *brick) {
    (void) brick;

    return BRICK_WIDTH;
}

/**
 * @brief Retrieves the height of the brick.
 *
 * @param brick Pointer to the brick.
 * @return Brick height in pixels.
 */
int brick_get_height(BRICK *brick) {
    (void) brick;

    return BRICK_HEIGHT;
}

/**
 * @brief Draws the brick on the screen if it is still alive.
 *
 * @param brick Pointer to the brick.
 */
void brick_print(BRICK *brick) {
    float x = 0.0f;
    float y = 0.0f;

    if (!brick || !brick->alive) {
        return;
    }

    x = brick_get_x(brick);
    y = brick_get_y(brick);

    al_draw_filled_rectangle(x, y, x + BRICK_WIDTH, y + BRICK_HEIGHT,
        brick_row_color(brick->row));
}
