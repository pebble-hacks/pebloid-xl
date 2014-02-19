//
//  dim_layer.h
//  Pebloid
//
//  Created by Dirk Mika on 12.05.13.
//
//

#pragma once

#include <pebble.h>

typedef enum
{
    DimModeBright = 0,
    DimModeDark
} DimMode;

typedef Layer DimLayer;

DimLayer* dim_layer_create(GRect frame);
void dim_layer_set_dim_mode(DimLayer *dim_layer, DimMode dim_mode);
void dim_layer_destroy(DimLayer* dim_layer);
