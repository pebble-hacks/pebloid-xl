//
//  powerup.h
//  
//
//  Created by Dirk Mika on 27.04.13.
//
//

#pragma once

#include <pebble.h>

#include "types.h"
#include "config.h"

extern GBitmap* imgPowerUpAddBall;
extern GBitmap* imgPowerUpSlowDown;
extern GBitmap* imgPowerUpSpeedUp;
extern GBitmap* imgPowerUpPowerBall;
extern GBitmap* imgPowerUpMissileLauncher;

void powerup_app_init(void);
void powerup_app_deinit(void);

void powerup_setup(PowerUp *powerup, PowerUpKind kind, GPoint point);
void powerup_move(PowerUp *powerup);
