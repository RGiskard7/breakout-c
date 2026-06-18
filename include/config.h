/**
 * @file config.h
 * @brief Configuration file for defining constants and resources in the
 * Breakout game.
 *
 * This file contains screen dimensions, gameplay parameters, resource paths,
 * and all tunable constants referenced throughout the codebase.
 *
 * Author: RGiskard7
 * Date: 18/06/2026
 */

#ifndef CONFIG_H
#define CONFIG_H

// Display Configuration
#define DISPLAY_WIDTH  600
#define DISPLAY_HEIGHT 600
#define FPS             60

// Resource Paths
#define FONT_RSC       "resources/fonts/space_invaders.ttf"
#define SND_BOUNCE     "resources/sounds/shoot.wav"
#define SND_BREAK      "resources/sounds/invaderkilled.wav"

// PADDLE CONFIGURATION
#define PADDLE_WIDTH    80
#define PADDLE_HEIGHT   14
#define PADDLE_Y       530
#define PADDLE_SPEED     8
#define PADDLE_INIT_X  (DISPLAY_WIDTH / 2 - PADDLE_WIDTH / 2)

// BALL CONFIGURATION
#define BALL_SIZE        10
#define BALL_SPEED_INIT  4.5f
#define BALL_SPEED_1     6.0f
#define BALL_SPEED_2     8.0f
#define BALL_SPEED_3    10.5f
#define BALL_MAX_ANGLE    2.5f

// BRICKS CONFIGURATION
#define BRICK_COLS       10
#define BRICK_ROWS        6
#define BRICK_WIDTH      52
#define BRICK_HEIGHT     18
#define BRICK_GAP         4
#define BRICK_X0         22
#define BRICK_Y0         80
#define BRICK_TOTAL      (BRICK_COLS * BRICK_ROWS)

// Speed stages (bricks destroyed to advance to next stage)
#define STAGE_1           8
#define STAGE_2          20
#define STAGE_3          38

// Scoring per row (top to bottom)
#define PTS_ROW_0         7
#define PTS_ROW_1         7
#define PTS_ROW_2         5
#define PTS_ROW_3         5
#define PTS_ROW_4         3
#define PTS_ROW_5         3

// Lives
#define MAX_LIVES         3

#endif /* CONFIG_H */
