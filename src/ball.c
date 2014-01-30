//
//  ball.c
//
//
//  Created by Dirk Mika on 22.04.13.
//
//

#include "ball.h"
#include "common.h"
#include "game.h"
#include "block.h"
#include "my_math.h"


// Private type

typedef enum
{
    BlockSideNone = 0,
    BlockSideLeft,
    BlockSideBottom,
    BlockSideRight,
    BlockSideTop
} BlockSide;


// Forward declaration
BlockSide calc_hit_side_of_block(Ball *ball, GRect *blockFrame, GRect *ballFrame, float faktor);
void ball_check_and_handle_hit(Ball *ball, GRect *paddle_rect, Game *game, float faktor);
int fillCoordsToCheck(GPoint coordToCheck[3], GPoint p1, GPoint p2, GPoint p3);


// Private var
GBitmap* imgBallNormal;


// Public functions
void ball_app_init(void)
{
    imgBallNormal = gbitmap_create_with_resource(RESOURCE_ID_IMG_BALL);
}

void ball_app_deinit(void)
{
    gbitmap_destroy(imgBallNormal);
}


void ball_setup(Ball *ball, BallPos ballPos, BallVector ballVector)
{
    ball->is_alive = true;
    ball->position = ballPos;
    ball->vector = ballVector;
}

void ball_set_glued(Ball *ball, bool glued, GRect paddle_rect)
{
	ball->is_glued = glued;
	ball->gluedPaddleOffset = GSize((int16_t)(ball->position.x + 0.5f) - paddle_rect.origin.x,
									(int16_t)(ball->position.y + 0.5f) - paddle_rect.origin.y);
}

bool ball_move(Ball *ball, GRect *paddle_rect, Game *game)
{
    if (ball->is_alive)
    {
        // Calculate new position
		if (ball->is_glued)
		{
			// Keep ball's offset to paddle constant
            ball->position = (BallPos){ paddle_rect->origin.x + ball->gluedPaddleOffset.w,
                paddle_rect->origin.y + ball->gluedPaddleOffset.h};
		}
		else
		{
        	float faktor = (GAMELOOP_TIMER_INTERVALL * game->ball_speed / 1000.0);
        	ball->position.x += (ball->vector.x * faktor);
        	ball->position.y += (ball->vector.y * faktor);
            
        	ball_check_and_handle_hit(ball, paddle_rect, game, faktor);
		}
    }
    return !ball->is_alive;
}

GRect ball_get_frame(Ball *ball)
{
    return GRect(ball->position.x + 0.5f, ball->position.y + 0.5f, BALL_SIZE, BALL_SIZE);
}


// private functions

void ball_check_and_handle_hit(Ball *ball, GRect *paddle_rect, Game *game, float faktor)
{
	float position;
	register int32_t angle;
	register bool didReflect;
    GRect ballFrame = ball_get_frame(ball);
	
	// Ball hit the ground?
	if (ballFrame.origin.y > SCREEN_H)
    {
        ball->is_alive = false;
    }
	else
	{
		do {
			didReflect = false;
			
			//Ball hit the left wall?
			if (ballFrame.origin.x < 0)
			{
				ball->vector.x = -ball->vector.x;
				ball->position.x = -ball->position.x;
				didReflect = true;
			}
			// Ball hit the right wall?
			else if (ballFrame.origin.x > (SCREEN_W - BALL_SIZE))
			{
				ball->vector.x = -ball->vector.x;
				ball->position.x = (float)((SCREEN_W - BALL_SIZE) << 1) - ball->position.x;
				didReflect = true;
			}
			
			// Ball hit the ceiling?
			if (ballFrame.origin.y < 0)
			{
				ball->vector.y = -ball->vector.y;
				ball->position.y = -ball->position.y;
				didReflect = true;
			}
			// Ball hit the paddle?
			else if (rectIntersect(&ballFrame, paddle_rect))
			{
				switch (calc_hit_side_of_block(ball, paddle_rect, &ballFrame, faktor))
				{
					case BlockSideTop:
						position = (float)(ball->position.x + BALL_SIZE - game->paddle->x) / (float)(PADDLE_W + BALL_SIZE);
						angle = (position * ANGLE_160_DEGREE + ANGLE_10_DEGREE);
						ball->vector = (BallVector){
							-(float)cos_lookup(angle) / (float)TRIG_MAX_RATIO,
							-(float)sin_lookup(angle) / (float)TRIG_MAX_RATIO};
						ball->position.y = (float)((paddle_rect->origin.y - BALL_SIZE) << 1) - ball->position.y;
						didReflect = true;
						break;
					case BlockSideLeft:
						ball->vector.x = -ball->vector.x;
						ball->position.x = (float)((paddle_rect->origin.x - BALL_SIZE) << 1) - ball->position.x;
						didReflect = true;
						break;
					case BlockSideRight:
						ball->vector.x = -ball->vector.x;
						ball->position.x = (float)((paddle_rect->origin.x + paddle_rect->size.w) << 1) - ball->position.x;
						didReflect = true;
						break;
					default:
						break;
				}
			}
			
			if (didReflect)
			{
				ballFrame = ball_get_frame(ball);
			}
		} while (didReflect);
        
		do {
			didReflect = false;
            
			// There are at max four blocks to check (one at each corner of the ball's rect)
            GPoint LO =    block_coord_for_pixel_coord(ballFrame.origin.x,                 ballFrame.origin.y);                    // LO
            GPoint RO =    block_coord_for_pixel_coord(ballFrame.origin.x + BALL_SIZE - 1, ballFrame.origin.y);                    // RO
            GPoint LU =    block_coord_for_pixel_coord(ballFrame.origin.x,                 ballFrame.origin.y + BALL_SIZE - 1);    // LU
            GPoint RU =    block_coord_for_pixel_coord(ballFrame.origin.x + BALL_SIZE - 1, ballFrame.origin.y + BALL_SIZE - 1);     // RU
            
            GPoint coordToCheck[3];
            register int numCoords;
			register bool runter = ball->vector.y > 0;
            
			// We just check three of those four depening on the direction the ball is going
			// We also define the order of hit-checking depenig on the direction
			if (ball->vector.x > SQRT05)
			{
				if (runter)
				{
                    numCoords = fillCoordsToCheck(coordToCheck, RO, LU, RU);
				}
				else
				{
                    numCoords = fillCoordsToCheck(coordToCheck, RU, LO, RO);
				}
			}
			else if (ball->vector.x > 0)
			{
				if (runter)
				{
                    numCoords = fillCoordsToCheck(coordToCheck, LU, RO, RU);
				}
				else
				{
                    numCoords = fillCoordsToCheck(coordToCheck, LO, RU, RO);
				}
			}
			else if (ball->vector.x > -SQRT05)
			{
				if (runter)
				{
                    numCoords = fillCoordsToCheck(coordToCheck, RU, LO, LU);
				}
				else
				{
                    numCoords = fillCoordsToCheck(coordToCheck, RO, LU, LO);
				}
			}
			else /* if (ball->vector.x < -SQRT05) */
			{
				if (runter)
				{
                    numCoords = fillCoordsToCheck(coordToCheck, LO, RU, LU);
				}
				else
				{
                    numCoords = fillCoordsToCheck(coordToCheck, LU, RO, LO);
				}
			}
            
			for (register int i = 0; i < numCoords; i++)
			{
                register int16_t row = coordToCheck[i].y;
                register int16_t col = coordToCheck[i].x;
                
                if (row < LEVEL_NUM_ROWS)
                {
                    Block *currentBlock = &game->level.blocks[row][col];
                    
                    if (*currentBlock != BlockStateNoBlock)
                    {
                        GRect *blockFrame = &GRect(col * BLOCK_W, row * BLOCK_H, BLOCK_W, BLOCK_H);
                        
                        game_handle_hit(game, currentBlock);
                        game_drop_powerup(game, blockFrame->origin);
                        
                        // If PowerUp POWER_BALL is active, don't reflect ball at blocks (only at non-destroyable blocks)
                        if (game->powerup_time_power_ball <= 0 || (*currentBlock == BlockStateUndestroyable))
                        {
                            BlockSide side = calc_hit_side_of_block(ball, blockFrame, &ballFrame, faktor);
                            
                            // Now check, which side was hit, to reflect the ball in the opposite direction
                            switch (side) {
                                case BlockSideLeft:
                                    ball->vector.x = -ball->vector.x;
                                    ball->position.x = (float)((blockFrame->origin.x - BALL_SIZE) << 1) - ball->position.x;
                                    didReflect = true;
                                    break;
                                case BlockSideBottom:
                                    ball->vector.y = -ball->vector.y;
                                    ball->position.y = (float)((blockFrame->origin.y + blockFrame->size.h) << 1) - ball->position.y;
                                    didReflect = true;
                                    break;
                                case BlockSideRight:
                                    ball->vector.x = -ball->vector.x;
                                    ball->position.x = (float)((blockFrame->origin.x + blockFrame->size.w) << 1) - ball->position.x;
                                    didReflect = true;
                                    break;
                                case BlockSideTop:
                                    ball->vector.y = -ball->vector.y;
                                    ball->position.y = (float)((blockFrame->origin.y - BALL_SIZE) << 1) - ball->position.y;
                                    didReflect = true;
                                    break;
                                default:
                                    break;
                            }
                        }

                        // We never hit more than one block at once
                        break;
                    }
                }
            }
            if (didReflect)
            {
                ballFrame = ball_get_frame(ball);
            }
        } while (didReflect);
    }
}

BlockSide calc_hit_side_of_block(Ball *ball, GRect *blockFrame, GRect *ballFrame, float faktor)
{
    GPoint oldPoint = GPoint(ball->position.x - (ball->vector.x * faktor) + 0.5f,
                             ball->position.y - (ball->vector.y * faktor) + 0.5f);
    
    if (oldPoint.y + BALL_SIZE - 1 < blockFrame->origin.y)
    {
        return BlockSideTop;
    }
    else if (oldPoint.y > blockFrame->origin.y + blockFrame->size.h - 1)
    {
        return BlockSideBottom;
    }
    else if (oldPoint.x + BALL_SIZE - 1 < blockFrame->origin.x)
    {
        return BlockSideLeft;
    }
    else if (oldPoint.x > blockFrame->origin.x + blockFrame->size.w - 1)
    {
        return BlockSideRight;
    }
    return BlockSideNone;
}

int fillCoordsToCheck(GPoint coordToCheck[3], GPoint p1, GPoint p2, GPoint p3)
{
    register int numCoords = 1;
    
    coordToCheck[0] = p1;
    if  (p2.x != p1.x || p2.y != p1.y) coordToCheck[numCoords++] = p2;
    if ((p3.x != p1.x || p3.y != p1.y) && (p3.x != p2.x || p3.y != p2.y)) coordToCheck[numCoords++] = p3;
    return numCoords;
}



