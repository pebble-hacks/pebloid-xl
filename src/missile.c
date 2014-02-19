//
//  missile.c
//  Pebloid
//
//  Created by Dirk Mika on 10.05.13.
//
//


#include "missile.h"
#include "types.h"
#include "block.h"
#include "game.h"
#include "config.h"
#include "common.h"

GBitmap* imgMissile;


void missile_app_init(void)
{
    imgMissile = gbitmap_create_with_resource(RESOURCE_ID_IMG_MISSILE);
}

void missile_app_deinit(void)
{
    gbitmap_destroy(imgMissile);
}

void missile_setup(Missile *missile, GPoint point)
{
    missile->is_active = true;
    missile->x = point.x;
    missile->y = point.y;
}

void missile_move(Missile *missile, Game *game)
{
    missile->y -= (GAMELOOP_TIMER_INTERVALL * MISSILE_BASE_SPEED / 1000.0);
    if (missile->y < 0)
    {
		missile->is_active = false;
    }
    else
    {
        GPoint missileTop = block_coord_for_pixel_coord(missile->x + ((MISSILE_W - 1) >> 1), (int16_t)(missile->y + 0.5f));
        register int16_t row = missileTop.y;
        register int16_t col = missileTop.x;

        if (row < LEVEL_NUM_ROWS)
        {
            Block *currentBlock = &game->level.blocks[row][col];
            
            if (*currentBlock != BlockStateNoBlock)
            {
                game_handle_hit(game, currentBlock);
                if (*currentBlock != BlockStateUndestroyable)
                {
                    game_drop_powerup(game, GPoint(col * BLOCK_W, row * BLOCK_H));
                }
                missile->is_active = false;
            }
        }
    }
}

GRect missile_get_frame(Missile *missile)
{
    return GRect(missile->x, (int16_t)(missile->y + 0.5f), MISSILE_W, MISSILE_H);
}


