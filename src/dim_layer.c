//
//  dim_layer.c
//  Pebloid
//
//  Created by Dirk Mika on 12.05.13.
//
//

#include "dim_layer.h"


typedef struct {
    DimMode dim_mode;
} DimLayerData;


// Private

void dim_layer_update_proc(DimLayer* dim_layer, GContext *ctx)
{
    DimLayerData* data = layer_get_data(dim_layer);
    GRect bounds = layer_get_bounds(dim_layer);
    
    graphics_context_set_stroke_color(ctx, (data->dim_mode == DimModeBright ? GColorWhite : GColorBlack));
    for (register int16_t y = bounds.origin.y; y < bounds.origin.y + bounds.size.h; y++)
    {
        for (register int16_t x = bounds.origin.x + y % 2; x < bounds.origin.x + bounds.size.w; x += 2)
        {
            graphics_draw_pixel(ctx, GPoint(x, y));
        }
    }
}


// Public

DimLayer* dim_layer_create(GRect frame)
{
    DimLayer* new_dim_layer = layer_create_with_data(frame, sizeof(DimLayerData));
    layer_set_update_proc(new_dim_layer, dim_layer_update_proc);
    dim_layer_set_dim_mode(new_dim_layer, DimModeBright);
    
    return new_dim_layer;
}

void dim_layer_set_dim_mode(DimLayer *dim_layer, DimMode dim_mode)
{
    DimLayerData* data = layer_get_data(dim_layer);

    data->dim_mode = dim_mode;
    layer_mark_dirty(dim_layer);
}

void dim_layer_destroy(DimLayer* dim_layer)
{
    layer_destroy(dim_layer);
}

