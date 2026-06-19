/**
 * @file config.h
 * @brief Configuration file for the Breakout game.
 *
 * All geometry, colours and rules are matched to the original Atari 2600
 * Breakout. The layout is derived from the reference screenshot
 * (resources/Breakout2600.png, 1280x840) scaled down by 0.5 to a 640x420
 * window, so every element keeps its original proportions to the pixel.
 *
 * Author: RGiskard7
 * Date: 18/06/2026
 */

#ifndef CONFIG_H
#define CONFIG_H

// Display (0.5x of the 1280x840 reference, keeps the original 2600 aspect)
#define DISPLAY_WIDTH  640
#define DISPLAY_HEIGHT 420
#define FPS             60

// Resources
#define FONT_RSC       "resources/fonts/space_invaders.ttf"
#define SND_BOUNCE     "resources/sounds/shoot.wav"
#define SND_BREAK      "resources/sounds/invaderkilled.wav"

// Playfield / walls. The grey frame is an inverted "U": a full-width top bar
// plus both side walls. The score digits sit in the black strip above the bar.
#define WALL_THICKNESS   32
#define SCOREBOARD_H     64                       // ball ceiling = bottom of bar
#define TOPBAR_Y         32                       // grey top bar: [TOPBAR_Y, SCOREBOARD_H]
#define PLAYFIELD_LEFT   WALL_THICKNESS           // 32
#define PLAYFIELD_RIGHT  (DISPLAY_WIDTH - WALL_THICKNESS) // 608
#define PLAYFIELD_TOP    SCOREBOARD_H             // 64  (ball bounces here)

// Score band digit metrics (stretched Atari-style: wide columns, short rows)
#define SCORE_PW         16   // pixel width  (digit = 3 * PW = 48)
#define SCORE_PH          4   // pixel height (digit = 5 * PH = 20)
#define SCORE_Y           6   // top of the digits, inside the black strip
#define SCORE_X         136   // left edge of the 3-digit score group
#define BALL_NUM_X      400   // current ball-number digit (centre of the band)
#define PLAYER_NUM_X    544   // player-number digit (right of the band)

// Paddle
#define PADDLE_WIDTH        64
#define PADDLE_WIDTH_SMALL  32   // halves when the ball breaks to the ceiling
#define PADDLE_HEIGHT        8
#define PADDLE_Y           376
#define PADDLE_SPEED         6
#define PADDLE_INIT_X      ((DISPLAY_WIDTH - PADDLE_WIDTH) / 2)

// Ball
#define BALL_SIZE        8
#define BALL_MAX_ANGLE   2.2f   // max deflection (radians) at the paddle edge

// Ball speed tiers (px/frame @ 60 FPS). Authentic progression:
//   tier 0: serve   tier 1: after 4 hits   tier 2: after 12 hits
//   tier 3: first orange-row hit          tier 4: first red-row hit
#define BALL_SPD_0   3.0f
#define BALL_SPD_1   4.0f
#define BALL_SPD_2   5.0f
#define BALL_SPD_3   6.0f
#define BALL_SPD_4   7.0f
#define SPEEDUP_HITS_1   4
#define SPEEDUP_HITS_2  12

// Bricks: 6 rows x 18 columns, edge to edge (a full row is a solid band,
// holes appear as bricks are destroyed, exactly like the reference).
#define BRICK_COLS      18
#define BRICK_ROWS       6
#define BRICK_WIDTH     32   // 18 * 32 = 576 = playfield width
#define BRICK_HEIGHT    12
#define BRICK_GAP        0
#define BRICK_TOTAL     (BRICK_COLS * BRICK_ROWS)
#define BRICK_X0        WALL_THICKNESS   // 32
#define BRICK_Y0       112

// Two walls (screens) per game, like the original (perfect game = 864 pts)
#define MAX_SCREENS      2

// Scoring per row, top -> bottom (red/orange 7, yellow 4, green/blue 1).
// One screen = (7+7+4+4+1+1) * 18 = 432 points; two screens = 864.
#define PTS_ROW_0        7   // red
#define PTS_ROW_1        7   // orange
#define PTS_ROW_2        4   // tan
#define PTS_ROW_3        4   // olive/yellow
#define PTS_ROW_4        1   // green
#define PTS_ROW_5        1   // blue

// Row indices that trigger a speed-up the first time they are hit
#define ROW_RED          0
#define ROW_ORANGE       1

// Lives (balls per game)
#define MAX_LIVES        3

// Atari 2600 Breakout palette (sampled from the reference screenshot)
#define COLOR_WALL     al_map_rgb(142, 142, 142)
#define COLOR_PADDLE   al_map_rgb(200,  72,  72)
#define COLOR_BALL     al_map_rgb(200,  72,  72)
#define COLOR_SCORE    al_map_rgb(142, 142, 142)
#define COLOR_BG       al_map_rgb(0, 0, 0)

#define COLOR_ROW_0    al_map_rgb(200,  72,  72)  // red
#define COLOR_ROW_1    al_map_rgb(198, 108,  58)  // orange
#define COLOR_ROW_2    al_map_rgb(180, 122,  48)  // tan
#define COLOR_ROW_3    al_map_rgb(162, 162,  42)  // olive / yellow
#define COLOR_ROW_4    al_map_rgb( 72, 160,  72)  // green
#define COLOR_ROW_5    al_map_rgb( 66,  72, 200)  // blue

#endif /* CONFIG_H */
