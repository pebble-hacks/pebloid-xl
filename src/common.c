//
//  common.c
//
//
//  Created by Dirk Mika on 19.04.13.
//
//

#include "common.h"


bool valueInRange(int value, int min, int max)
{
    return (value >= min) && (value <= max);
}

bool rectIntersect(GRect *r1, GRect *r2)
{
    register bool xOverlap = valueInRange(r1->origin.x, r2->origin.x, r2->origin.x + r2->size.w - 1) ||
    valueInRange(r2->origin.x, r1->origin.x, r1->origin.x + r1->size.w - 1);
    
    register bool yOverlap = valueInRange(r1->origin.y, r2->origin.y, r2->origin.y + r2->size.h - 1) ||
    valueInRange(r2->origin.y, r1->origin.y, r1->origin.y + r1->size.h - 1);
    
    return xOverlap && yOverlap;
}
