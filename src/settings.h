//
//  settings.h
//  
//
//  Created by Dirk Mika on 01.05.13.
//
//

#pragma once

#include <pebble.h>

extern const char *button_names_names[];

typedef enum {
    GameControlMoveLeft = 0,
    GameControlMoveRight,
    GameControlPlayPause
} GameControl;

typedef struct
{
    ButtonId button[3]; // 0 = move left, 1 = move right, 2 = play/pause
    bool has_saved_game;
    bool keep_backlight_on;
    bool is_sound_enabled;
} PebloidSettings;

void settings_load(PebloidSettings* settings);
void settings_edit(PebloidSettings* settings);
void settings_save(PebloidSettings* settings);

