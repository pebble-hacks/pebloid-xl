//
//  missile.h
//  Pebloid
//
//  Created by Dirk Mika on 10.05.13.
//
//

#pragma once

#include <pebble.h>

#include "types.h"
#include "config.h"

extern GBitmap* imgMissile;

void missile_app_init(void);
void missile_app_deinit(void);

void missile_setup(Missile *missile, GPoint point);
void missile_move(Missile *missile, Game *game);
GRect missile_get_frame(Missile *missile);