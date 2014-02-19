//
//  common.h
//
//
//  Created by Dirk Mika on 19.04.13.
//
//

#pragma once

#include <stdbool.h>
#include <pebble.h>

//void itoa(int value, char *sp, int radix);
bool valueInRange(int value, int min, int max);
bool rectIntersect(GRect *r1, GRect *r2);
