/**
 * @file ball.h
 * @brief Declaration of the Ball structure and its associated functions.
 *
 * The ball bounces off walls, the paddle, and bricks. Its speed increases
 * in stages as more bricks are destroyed.
 *
 * Author: RGiskard7
 * Date: 18/06/2026
 */

#ifndef BALL_H
#define BALL_H

#include <allegro5/allegro.h>
#include "types.h"

typedef struct _ball BALL;

/**
 * @brief Creates a new ball instance.
 *
 * @return Pointer to the created BALL or NULL on allocation failure.
 */
BALL *ball_create(void);

/**
 * @brief Destroys the ball and frees allocated resources.
 *
 * @param ball Pointer to the BALL to destroy.
 */
void ball_destroy(BALL *ball);

// Position getters
float ball_get_x(BALL *ball);
float ball_get_y(BALL *ball);
float ball_get_speed(BALL *ball);
bool  ball_is_serving(BALL *ball);

/**
 * @brief Returns and clears the "ball hit the ceiling" flag.
 *
 * @return true once after each ceiling bounce, false otherwise.
 */
bool  ball_take_ceiling_hit(BALL *ball);

// Position setters
STATUS ball_set_x(BALL *ball, float x);
STATUS ball_set_y(BALL *ball, float y);
STATUS ball_set_speed(BALL *ball, float speed);
STATUS ball_set_serving(BALL *ball, bool serving);

// Movement and physics
STATUS ball_attach_to_paddle(BALL *ball, float paddle_x, int paddle_w);
STATUS ball_launch(BALL *ball);
STATUS ball_move(BALL *ball, float paddle_x, int paddle_w);
STATUS ball_bounce_wall(BALL *ball);
STATUS ball_bounce_paddle(BALL *ball, float paddle_x, int paddle_w);
STATUS ball_bounce_x(BALL *ball);
STATUS ball_bounce_y(BALL *ball);

// Rendering
STATUS ball_print(BALL *ball);

#endif /* BALL_H */
