//
//  types.h
//
//
//  Created by Dirk Mika on 24.04.13.
//
//

#pragma once

#include <pebble.h>
#include "config.h"


// Block
typedef enum {
    BlockStateNoBlock = 0,
    BlockStateNormal,
    BlockStateUndestroyable,
    BlockStateDoubleHit1,
    BlockStateDoubleHit2
} Block;


// Level
typedef struct {
    uint8_t nr;
	uint8_t numBlocksVisible;
	uint8_t numBlocksLeft;
    Block blocks[LEVEL_NUM_ROWS][LEVEL_NUM_COLS];
} Level;


// Ball
typedef struct
{
    float x;
    float y;
} BallPos;

typedef struct
{
    float x;
    float y;
} BallVector;

typedef struct
{
    BallPos position;
    BallVector vector;
    bool is_alive;
	bool is_glued;
	GSize gluedPaddleOffset;
} Ball;

// Paddle
typedef enum {
    PaddleSizeNormal = 0,
    PaddleSizeLarge,
    PaddleSizeSmall
} PaddleSize;

typedef struct
{
    BitmapLayer* layer;
    float x;
    float vector;
} Paddle;

// PowerUps
typedef enum {
    PowerUpKindAddBall = 0,
    PowerUpKindSlowDown,
    PowerUpKindSpeedUp,
    PowerUpKindPowerBall,
    PowerUpKindMissileLauncher,
    NumPowerUpKind
} PowerUpKind;

typedef int PowerUpProbabilityMap[NumPowerUpKind];

typedef struct
{
    int16_t x;
    float y;
    PowerUpKind kind;
    bool is_active;
} PowerUp;

// Missiles (Missile-Launcher PowerUp)
typedef struct
{
    int16_t x;
    float y;
    bool is_active;
} Missile;

// Game
typedef enum {
    GameModeAppStarted = 0,
	GameModeFinished,	// set when game over
	GameModeRunning,	// Game is running
	GameModePause,		// Player has pressed pause
	GameModeIdle		// Game is idle. Set after a ball is lost or level has finished. Give the main-controller the chance to animate some stuff
                        // before the game continues to run.
} GameMode;

struct Game;

typedef void (*GameLifeLostHandler)(struct Game *game);
typedef void (*GameScoreChangedHandler)(struct Game *game);
typedef void (*GameLevelFinishedHandler)(struct Game *game);
typedef void (*GameModeChangedHandler)(struct Game *game);
typedef void (*GameBlockHitHandler)(struct Game *game);

typedef struct GameHandlers
{
	GameLifeLostHandler lifeLost;
	GameScoreChangedHandler scoreChanged;
	GameLevelFinishedHandler levelFinished;
    GameModeChangedHandler modeChanged;
    GameBlockHitHandler blockHit;
} GameHandlers;

typedef struct Game
{
    Level level;
    Ball balls[MAX_BALLS];
    PowerUp powerups[MAX_POWERUPS];
    Missile missiles[MAX_MISSILES];
    Paddle* paddle;
    uint8_t num_balls_alive;
	uint8_t num_lives;
    int16_t score;
    uint16_t ball_speed;
    uint16_t ball_speed_normal;
	GameMode mode;
	GameHandlers gameHandlers;
    int16_t powerup_time_ball_slow;                 // Remaining PowerUp time for slow ball
    int16_t powerup_time_ball_fast;                 // Remaining PowerUp time for fast ball
    int16_t powerup_time_power_ball;                // Remaining PowerUp time for power ball
    int16_t powerup_time_missile_launcher;          // Remaining PowerUp time for missile launcher
    int16_t missile_launcher_launch_delay;          // delay between two missile launches
} Game;

typedef void (*AnimationStepPerformer)(uint8_t step);

typedef struct
{
    AnimationStepPerformer performStep;
    uint8_t step;
} AnimationStep;


typedef enum {
    ShowStandByPaddleNone = 0,
    ShowStandByPaddleAll,
    ShowStandByPaddleAllButLast
} ShowStandByPaddle;

