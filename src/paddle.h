//
//  paddle.h
//  
//
//  Created by Dirk Mika on 30.04.13.
//
//

#include <pebble.h>

#include "config.h"
#include "types.h"

extern GBitmap* imgPaddle;

Paddle* paddle_create(void);
void paddle_reset(Paddle *paddle);
void paddle_move(Paddle *paddle);
void paddle_destroy(Paddle* paddle);
