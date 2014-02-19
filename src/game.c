//
//  game.c
//
//
//  Created by Dirk Mika on 24.04.13.
//
//

#include <pebble.h>

#include "game.h"
#include "level.h"
#include "ball.h"
#include "powerup.h"
#include "paddle.h"
#include "missile.h"
#include "config.h"
#include "common.h"
#include "my_math.h"


// Because we can just save 256 at one, we need to split the savegame-data into two parts

typedef struct
{
    Level level;
    float paddle_x;
} SavegameData1;

typedef struct
{
    Ball balls[MAX_BALLS];
    PowerUp powerups[MAX_POWERUPS];
    Missile missiles[MAX_MISSILES];
    uint8_t num_balls_alive;
	uint8_t num_lives;
    int16_t score;
    uint16_t ball_speed;
    uint16_t ball_speed_normal;
    int16_t powerup_time_ball_slow;
    int16_t powerup_time_ball_fast;
    int16_t powerup_time_power_ball;
    int16_t powerup_time_missile_launcher;
    int16_t missile_launcher_launch_delay;
} SavegameData2;



// Private prototyps
void game_fork_ball(Game *game);
void game_hide_all_balls(Game *game);
void game_deactivate_all_powerups(Game *game);
void game_deactivate_all_missiles(Game *game);

void game_launch_missile(Game *game, GPoint point);


// Public functions
void game_init(Game *game)
{
    game->paddle = paddle_create();
    game_set_mode(game, GameModeAppStarted);
}

void game_deinit(Game* game)
{
    paddle_destroy(game->paddle);
}

void game_reset(Game *game)
{
    game->num_lives = GAME_NUM_START_LIVES;
    game->score = 0;
    game->level.nr = 0;
    game->ball_speed = BALL_SPEED_START;
    game->ball_speed_normal = BALL_SPEED_START;
}

void game_update(Game *game)
{
    register int oldScore = game->score / SCORE_AMOUNT_1UP;
    GRect paddle_rect = layer_get_frame(bitmap_layer_get_layer(game->paddle->layer));

    // Decrase PoperUpTimes
    if (game->powerup_time_ball_slow > 0)
    {
        game->powerup_time_ball_slow -= GAMELOOP_TIMER_INTERVALL;
        if (game->powerup_time_ball_slow <= 0)
        {
            game->ball_speed = game->ball_speed_normal;
        }
    }
    if (game->powerup_time_ball_fast > 0)
    {
        game->powerup_time_ball_fast -= GAMELOOP_TIMER_INTERVALL;
        if (game->powerup_time_ball_fast <= 0)
        {
            game->ball_speed = game->ball_speed_normal;
        }
    }
    if (game->powerup_time_power_ball > 0) game->powerup_time_power_ball -= GAMELOOP_TIMER_INTERVALL;
    if (game->powerup_time_missile_launcher > 0)
    {
        game->powerup_time_missile_launcher -= GAMELOOP_TIMER_INTERVALL;
        if (game->missile_launcher_launch_delay > 0)
        {
            game->missile_launcher_launch_delay -= GAMELOOP_TIMER_INTERVALL;
        }
        else
        {
            game_launch_missile(game, GPoint(paddle_rect.origin.x + (paddle_rect.size.w >> 1) - ((MISSILE_W - 1) >> 1), PADDLE_Y));
        }
    }
    
    // Move Paddle
    paddle_move(game->paddle);
    
    // Move PowerUps and check, if a PowerUp is catched
    for (register int i = 0; i < MAX_POWERUPS; i++)
    {
        PowerUp *currentPowerUp = &game->powerups[i];
        if (currentPowerUp->is_active)
        {
            powerup_move(currentPowerUp);
            
            GRect *powerup_frame = &GRect(currentPowerUp->x, (int16_t)(currentPowerUp->y + 0.5f), POWERUP_W, POWERUP_H);
            
            if (rectIntersect(&paddle_rect, powerup_frame))
            {
				currentPowerUp->is_active = false;
                
                switch (currentPowerUp->kind) {
                    case PowerUpKindAddBall:
                        game_fork_ball(game);
                        game->score += 2;
                        break;
                    case PowerUpKindSlowDown:
                        game->ball_speed = game->ball_speed_normal + BALL_SPEED_OFFSET_SLOW;
                        game->powerup_time_ball_slow = POWERUP_TIME_BALL_SLOW;
                        game->powerup_time_ball_fast = 0;
                        game->score += 1;
                        break;
                    case PowerUpKindSpeedUp:
                        game->ball_speed = game->ball_speed_normal + BALL_SPEED_OFFSET_FAST;
                        game->powerup_time_ball_fast = POWERUP_TIME_BALL_FAST;
                        game->powerup_time_ball_slow = 0;
                        game->score += 5;
                        break;
                    case PowerUpKindPowerBall:
                        game->powerup_time_power_ball = POWERUP_TIME_POWER_BALL;
                        game->score += 2;
                        break;
                    case PowerUpKindMissileLauncher:
                        if (game->powerup_time_missile_launcher <= 0)
                        {
                            // Force missile launch on next gameloop, when missile launcher wasn't active
                            game->missile_launcher_launch_delay = 0;
                        }
                        game->powerup_time_missile_launcher = POWERUP_TIME_MISSILE_LAUNCHER;
                        game->score += 1;
                        break;
                        
                    default:
                        break;
                }
				game->gameHandlers.scoreChanged(game);
            }
        }
    }
    
    // Move Missiles and check if block is hit
    for (register int i = 0; i < MAX_MISSILES; i++)
    {
        Missile *currentMissile = &game->missiles[i];
        
        if (currentMissile->is_active)
        {
            missile_move(currentMissile, game);
        }
    }
    
    // Move balls
    for (register int i = 0; i < MAX_BALLS; i++)
    {
		Ball *currentBall = &game->balls[i];

		// Is ball acive
        if (currentBall->is_alive)
        {
			// Yes, move ball around
            if (ball_move(currentBall, &paddle_rect, game))
            {
				// If ball_move returns true, the ball died
                game->num_balls_alive--;
				
				// Any ball left?
				if (game->num_balls_alive == 0)
				{
					// No, how bad. Lost a live.
                    game_set_mode(game, GameModeIdle);
					game->num_lives--;
                    game->ball_speed = game->ball_speed_normal;
                    game_deactivate_all_powerups(game);
                    game_deactivate_all_missiles(game);
					game->gameHandlers.lifeLost(game);
					
					// Any live left?
					if (game->num_lives == 0)
					{
						// No, ok that was it.
                        game_set_mode(game, GameModeFinished);
                        
						// If game is over, we don't need to check the reset of the balls or load the lext level
						return;
					}
				}
            }
        }
    }
    
    // Add a live every SCORE_AMOUNT_1UP points
    if (oldScore < (game->score / SCORE_AMOUNT_1UP))
    {
        game->num_lives++;
    }
    
    // Level finishes?
	if (game->mode != GameModeFinished)
	{
		if (game->level.numBlocksLeft == 0)
		{
			// Level cleared.
			game_hide_all_balls(game);
            game_deactivate_all_powerups(game);
            game_deactivate_all_missiles(game);
            game_set_mode(game, GameModeIdle);
			game->gameHandlers.levelFinished(game);
		}
	}
}

void game_set_mode(Game *game, GameMode mode)
{
    game->mode = mode;
    if (game->gameHandlers.modeChanged != NULL)
    {
        game->gameHandlers.modeChanged(game);
    }
}

bool game_lauch_glued_balls(Game *game)
{
	register bool is_launched = false;
	
	for (register int i = 0; i < MAX_BALLS; i++)
    {
		Ball *currentBall = &game->balls[i];
        if (currentBall->is_alive)
		{
			if (currentBall->is_glued)
			{
				is_launched = true;
				currentBall->is_glued = false;
			}
		}
	}
	return is_launched;
}

void game_handle_hit(Game *game, Block *block)
{
    if ((*block == BlockStateNormal) ||
        (*block == BlockStateDoubleHit2) ||
        ((*block == BlockStateDoubleHit1) && (game->powerup_time_power_ball > 0)))
    {
        *block = BlockStateNoBlock;

        game->level.numBlocksVisible--;
        game->level.numBlocksLeft--;
        game->score += 1;
        game->gameHandlers.scoreChanged(game);
        game->gameHandlers.blockHit(game);
    }
    else if (*block == BlockStateDoubleHit1)
    {
        *block = BlockStateDoubleHit2;
        game->gameHandlers.blockHit(game);
    }
}

// Drops a PowerUp by a chance of 1:7, if there is a free PowerUp-Slot
void game_drop_powerup(Game *game, GPoint point)
{
    static PowerUpProbabilityMap powerUpProbabilityMap = {
        25,     // PowerUpAddBall
        50,     // PowerUpKindSlowDown
        75,     // PowerUpKindSpeedUp
        85,     // PowerUpKindPowerBall
        100     // PowerUpKindMissileLauncher
    };
    
    // at an average of 7 blocks drop a PowerUp
    if ((rand() % 7) == 0)
    {
        // Find a free slot
        for (register int i = 0; i < MAX_POWERUPS; i++)
        {
	        PowerUp *currentPowerUp = &game->powerups[i];
            if (!currentPowerUp->is_active)
            {
                int probability = rand() % 100;
                for (register PowerUpKind powerUpKind = PowerUpKindAddBall; powerUpKind < NumPowerUpKind; powerUpKind++)
                {
                    if (probability < powerUpProbabilityMap[powerUpKind])
                    {
                        powerup_setup(currentPowerUp, powerUpKind, point);
                        return;
                    }
                }
            }
        }
    }
}

void game_reset_balls(Game *game)
{
    game_hide_all_balls(game);

    register int32_t angle = (rand() % ANGLE_45_DEGREE) + ANGLE_67_5_DEGREE;
    
    ball_setup(&game->balls[0],
               (BallPos){ BALL_START_POS.origin.x, BALL_START_POS.origin.y },
               (BallVector){
                   -(float)cos_lookup(angle) / (float)TRIG_MAX_RATIO,
                   -(float)sin_lookup(angle) / (float)TRIG_MAX_RATIO});
    ball_set_glued(&game->balls[0], true, PADDLE_HIDDEN_POS);
    game->num_balls_alive = 1;
}

void game_save(Game* game)
{
    SavegameData1 data1;
    SavegameData2 data2;
    
    data1.level = game->level;
    data1.paddle_x = game->paddle->x;
    
    for (int i = 0; i < MAX_BALLS; i++) data2.balls[i] = game->balls[i];
    for (int i = 0; i < MAX_POWERUPS; i++) data2.powerups[i] = game->powerups[i];
    for (int i = 0; i < MAX_MISSILES; i++) data2.missiles[i] = game->missiles[i];
    data2.num_balls_alive = game->num_balls_alive;
    data2.num_lives = game->num_lives;
    data2.score = game->score;
    data2.ball_speed = game->ball_speed;
    data2.ball_speed_normal = game->ball_speed_normal;
    data2.powerup_time_ball_slow = game->powerup_time_ball_slow;
    data2.powerup_time_ball_fast = game->powerup_time_ball_fast;
    data2.powerup_time_power_ball = game->powerup_time_power_ball;
    data2.powerup_time_missile_launcher = game->powerup_time_missile_launcher;
    data2.missile_launcher_launch_delay = game->missile_launcher_launch_delay;
    
    persist_write_data(110, &data1, sizeof(SavegameData1));
    persist_write_data(111, &data2, sizeof(SavegameData2));
}

bool game_load(Game* game)
{
    bool success = false;
    
    // Check, if saved gamedata exists
    if (persist_exists(110) && persist_exists(111))
    {
        // Check if size is what we need.
        if ((persist_get_size(110) == sizeof(SavegameData1)) &&
            (persist_get_size(111) == sizeof(SavegameData2)))
        {
            SavegameData1 data1;
            SavegameData2 data2;
            
            persist_read_data(110, &data1, sizeof(SavegameData1));
            persist_read_data(111, &data2, sizeof(SavegameData2));
            
            game->level = data1.level;
            game->paddle->x = data1.paddle_x;
            
            // This is a bit hacky, but I'm to lazy to refactor this atm.
            layer_set_frame(bitmap_layer_get_layer(game->paddle->layer), GRect(game->paddle->x + 0.5f, PADDLE_Y, PADDLE_W, PADDLE_H));
            
            for (int i = 0; i < MAX_BALLS; i++) game->balls[i] = data2.balls[i];
            for (int i = 0; i < MAX_POWERUPS; i++) game->powerups[i] = data2.powerups[i];
            for (int i = 0; i < MAX_MISSILES; i++) game->missiles[i] = data2.missiles[i];
            game->num_balls_alive = data2.num_balls_alive;
            game->num_lives = data2.num_lives;
            game->score = data2.score;
            game->ball_speed = data2.ball_speed;
            game->ball_speed_normal = data2.ball_speed_normal;
            game->powerup_time_ball_slow = data2.powerup_time_ball_slow;
            game->powerup_time_ball_fast = data2.powerup_time_ball_fast;
            game->powerup_time_power_ball = data2.powerup_time_power_ball;
            game->powerup_time_missile_launcher = data2.powerup_time_missile_launcher;
            game->missile_launcher_launch_delay = data2.missile_launcher_launch_delay;
            
            success = true;
        }
        else
        {
        }
    }
    return success;
}


// Private functions

void game_fork_ball(Game *game)
{
    register Ball *activeBall;
    register Ball *newBall;
    register int i;
    
    // Find a free ball
    for (i = 0; i < MAX_BALLS; i++)
    {
		newBall = &game->balls[i];
        if (!newBall->is_alive)
        {
            // Find an active ball
            for (i = 0; i < MAX_BALLS; i++)
            {
				activeBall = &game->balls[i];
                if (activeBall->is_alive)
                {
                    break;
                }
            }
            
            // Launch new Ball at the same position as one of the currently active balls.
            ball_setup(newBall, activeBall->position, (BallVector){ activeBall->vector.y, activeBall->vector.x });
            game->num_balls_alive += 1;
            
            break;
        }
    }
}

void game_hide_all_balls(Game *game)
{
	for (register int i = 0; i < MAX_BALLS; i++)
    {
        game->balls[i].is_alive = false;
    }
}

void game_deactivate_all_powerups(Game *game)
{
    for (register int i = 0; i < MAX_POWERUPS; i++)
    {
        game->powerups[i].is_active = false;
    }
    game->powerup_time_ball_slow = 0;
    game->powerup_time_ball_fast = 0;
    game->powerup_time_power_ball = 0;
    game->powerup_time_missile_launcher = 0;
}

void game_deactivate_all_missiles(Game *game)
{
    for (register int i = 0; i < MAX_MISSILES; i++)
    {
        game->missiles[i].is_active = false;
    }    
}

void game_launch_missile(Game *game, GPoint point)
{
    // find a free missile
    for (register int i = 0; i < MAX_MISSILES; i++)
    {
        Missile *currentMissile = &game->missiles[i];
        if (!currentMissile->is_active)
        {
            missile_setup(currentMissile, point);
            game->missile_launcher_launch_delay = MISSILE_LAUNCH_DELAY;
            return;
        }
    }
}


