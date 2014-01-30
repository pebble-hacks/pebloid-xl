//
//  game.h
//
//
//  Created by Dirk Mika on 24.04.13.
//
//

#pragma once

#include "types.h"


void game_init(Game *game);
void game_deinit(Game* game);
void game_reset(Game *game);
void game_update(Game *game);
void game_set_mode(Game *game, GameMode mode);
bool game_lauch_glued_balls(Game *game);
void game_handle_hit(Game *game, Block *block);
void game_drop_powerup(Game *game, GPoint point);
void game_reset_balls(Game *game);
void game_save(Game* game);
bool game_load(Game* game);
