//
//  ball.h
//
//
//  Created by Dirk Mika on 22.04.13.
//
//

#pragma once

#include <pebble.h>

#include "config.h"
#include "types.h"

extern GBitmap* imgBallNormal;

void ball_app_init(void);
void ball_app_deinit(void);

void ball_setup(Ball *ball, BallPos ballPos, BallVector ballVector);
void ball_set_glued(Ball *ball, bool glued, GRect paddle_rect);
bool ball_move(Ball *ball, GRect *paddle_rect, Game *game); // returns true if ball died
GRect ball_get_frame(Ball *ball);
