//
//  block.h
//
//
//  Created by Dirk Mika on 23.04.13.
//
//

#pragma once

#include <pebble.h>

#include "types.h"
#include "config.h"

extern GBitmap* imgBlockNormal;
extern GBitmap* imgBlockSolid;
extern GBitmap* imgBlockDoubleHit1;
extern GBitmap* imgBlockDoubleHit2;

void block_app_init(void);
void block_app_deinit(void);
GPoint block_coord_for_pixel_coord(int16_t x, int16_t y);
