//
//  paddle.c
//  
//
//  Created by Dirk Mika on 30.04.13.
//
//


#include "paddle.h"

GBitmap* imgPaddle = NULL;


// Public functions
Paddle* paddle_create(void)
{
    if (imgPaddle == NULL)
    {
        imgPaddle = gbitmap_create_with_resource(RESOURCE_ID_IMG_PADDLE);
    }
    Paddle* new_paddle = malloc(sizeof(Paddle));
    new_paddle->layer = bitmap_layer_create(PADDLE_START_POS);
    bitmap_layer_set_bitmap(new_paddle->layer, imgPaddle);
    paddle_reset(new_paddle);

    return new_paddle;
}

void paddle_reset(Paddle *paddle)
{
    paddle->x = PADDLE_X;
    paddle->vector = 0.0f;
}

void paddle_move(Paddle *paddle)
{
    if (paddle->vector != 0.0f)
    {
        float faktor = (GAMELOOP_TIMER_INTERVALL * PADDLE_BASE_SPEED / 1000.0);
        paddle->x += (paddle->vector * faktor);
        if (paddle->x < PADDLE_MIN_POS) paddle->x = PADDLE_MIN_POS;
        if (paddle->x > PADDLE_MAX_POS) paddle->x = PADDLE_MAX_POS;
        
        layer_set_frame(bitmap_layer_get_layer(paddle->layer), GRect(paddle->x + 0.5f, PADDLE_Y, PADDLE_W, PADDLE_H));
    }
}

void paddle_destroy(Paddle* paddle)
{
    if (imgPaddle)
    {
        gbitmap_destroy(imgPaddle);
    }
    bitmap_layer_destroy(paddle->layer);
    free(paddle);
}
