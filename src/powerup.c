//
//  powerup.c
//  
//
//  Created by Dirk Mika on 27.04.13.
//
//

#include "powerup.h"
#include "types.h"
#include "config.h"
#include "common.h"

GBitmap* imgPowerUpAddBall;
GBitmap* imgPowerUpSlowDown;
GBitmap* imgPowerUpSpeedUp;
GBitmap* imgPowerUpPowerBall;
GBitmap* imgPowerUpMissileLauncher;


void powerup_app_init(void)
{
    imgPowerUpAddBall = gbitmap_create_with_resource(RESOURCE_ID_IMG_POWERUP_ADD_BALL);
    imgPowerUpSlowDown = gbitmap_create_with_resource(RESOURCE_ID_IMG_POWERUP_SLOW_BALL);
    imgPowerUpSpeedUp = gbitmap_create_with_resource(RESOURCE_ID_IMG_POWERUP_SPEED_UP);
    imgPowerUpPowerBall = gbitmap_create_with_resource(RESOURCE_ID_IMG_POWERUP_POWER_BALL);
    imgPowerUpMissileLauncher = gbitmap_create_with_resource(RESOURCE_ID_IMG_POWERUP_MISSILE_LAUNCHER);
}

void powerup_app_deinit(void)
{
    gbitmap_destroy(imgPowerUpAddBall);
    gbitmap_destroy(imgPowerUpSlowDown);
    gbitmap_destroy(imgPowerUpSpeedUp);
    gbitmap_destroy(imgPowerUpPowerBall);
    gbitmap_destroy(imgPowerUpMissileLauncher);
}


void powerup_setup(PowerUp *powerup, PowerUpKind kind, GPoint point)
{
    powerup->kind = kind;
    powerup->is_active = true;
    powerup->x = point.x;
    powerup->y = point.y;
}

void powerup_move(PowerUp *powerup)
{
    powerup->y += (GAMELOOP_TIMER_INTERVALL * POWERUP_BASE_SPEED / 1000.0);
    if (powerup->y > SCREEN_H)
    {
		powerup->is_active = false;
    }
}

