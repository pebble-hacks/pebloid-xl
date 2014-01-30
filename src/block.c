//
//  block.c
//
//
//  Created by Dirk Mika on 23.04.13.
//
//

#include "block.h"
#include "common.h"

GBitmap* imgBlockNormal;
GBitmap* imgBlockSolid;
GBitmap* imgBlockDoubleHit1;
GBitmap* imgBlockDoubleHit2;

void block_app_init(void)
{
    imgBlockNormal = gbitmap_create_with_resource(RESOURCE_ID_IMG_BLOCK_NORMAL);
    imgBlockSolid = gbitmap_create_with_resource(RESOURCE_ID_IMG_BLOCK_SOLID);
    imgBlockDoubleHit1 = gbitmap_create_with_resource(RESOURCE_ID_IMG_BLOCK_DOUBLEHIT_1);
    imgBlockDoubleHit2 = gbitmap_create_with_resource(RESOURCE_ID_IMG_BLOCK_DOUBLEHIT_2);
}

void block_app_deinit(void)
{
    gbitmap_destroy(imgBlockNormal);
    gbitmap_destroy(imgBlockSolid);
    gbitmap_destroy(imgBlockDoubleHit1);
    gbitmap_destroy(imgBlockDoubleHit2);
}

GPoint block_coord_for_pixel_coord(int16_t x, int16_t y)
{
    return GPoint(x >> 4 /* x / BLOCK_W */, y / BLOCK_H);
}

