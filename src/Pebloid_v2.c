
#include <pebble.h>

#include "config.h"
#include "ball.h"

// Some libs and utils
#include "my_math.h"
#include "common.h"
#include "types.h"
#include "game.h"
#include "level.h"
#include "block.h"
#include "powerup.h"
#include "missile.h"
#include "paddle.h"
#include "settings.h"
#include "dim_layer.h"

#define ACCEL_MEMORY 5 // seconds

// Main-Window
Window* window;

// Layers of main-Window
Layer* gameLayer;
DimLayer* dimLayer;
TextLayer* infoTextLayer;
TextLayer* hintTextLayer;
Layer* bottomInfoLayer;

// Fonts
GFont info_font;
GFont hint_font;

// Misc
int frames_per_second;

ShowStandByPaddle showStandbyPaddles;
static char infoText[19];
static char hintText[30];
char *format_game_hint = "%s\n\n\nPress '%s'";
PebloidSettings pebloidSettings;
// TODO make size depend on update rate
#define ACCEL_SIZE 10
static int accel_history[ACCEL_SIZE];
static int accel_pos;
static int accel_mid;
#define ACCEL_MAX 100
#define ACCEL_MIN 5

// All gamedata
Game game;


// prototype
void animationSequenceMainCallback(struct Animation *animation, bool finished, void *context);
void updateLevelAndScore();
void updateHintText();
void handleTimer(void *data);
void handleTick(struct tm *tick_time, TimeUnits units_changed);

// Animation helper
void animateLayerToFrame(struct Layer *layer,
						 GRect *to_frame,
						 uint32_t delay_ms,
						 uint32_t duration_ms,
						 AnimationCurve curve,
						 AnimationHandlers *callbacks,
                         void *context)
{
	PropertyAnimation *property_animation = property_animation_create_layer_frame(layer,
                                                                                  NULL, // always animate from current frame
                                                                                  to_frame);
	animation_set_delay(&property_animation->animation, delay_ms);
	animation_set_duration(&property_animation->animation, duration_ms);
	animation_set_curve(&property_animation->animation, curve);
	if (callbacks != NULL)
	{
		animation_set_handlers(&property_animation->animation,
							   *callbacks,
							   context);
	}
	animation_schedule(&property_animation->animation);
}


//
// Animations
//
void newPaddleAnimation(uint8_t step)
{
    static AnimationStep nextStep;
    GRect frame;

    nextStep = (AnimationStep) {
        .performStep = &newPaddleAnimation,
        .step = step + 1
    };
    switch (step) {
        case 0:
            // Move paddle down out of game-area
        {
            Layer* paddle_layer = bitmap_layer_get_layer(game.paddle->layer);
            frame = layer_get_frame(paddle_layer);
            frame.origin.y = SCREEN_H;
            animateLayerToFrame(paddle_layer,
                                &frame,
                                0,
                                ANIM_DUR_PADDLE_DOWN,
                                AnimationCurveEaseIn,
                                &(AnimationHandlers) {
                                    .stopped = &animationSequenceMainCallback
                                },
                                &nextStep);
        }
            break;
        case 1:
            showStandbyPaddles = ShowStandByPaddleAll;
            
            // Move the entire game-area to the right, let the standbypaddles become visible
            animateLayerToFrame(gameLayer,
                                &GRect(PADDLE_W, 0, SCREEN_W, SCREEN_H),
                                ANIM_DELAY,
                                ANIM_DUR_SHOW_STANDBY_PADDLE,
                                AnimationCurveEaseOut,
                                &(AnimationHandlers) {
                                    .stopped = &animationSequenceMainCallback
                                },
                                &nextStep);
            break;
        case 2:
            // Move Paddle to the lowest StandbyPaddle
        {
            Layer* paddle_layer = bitmap_layer_get_layer(game.paddle->layer);
            layer_set_frame(paddle_layer, GRect(-PADDLE_W, PADDLE_Y, PADDLE_W, PADDLE_H));
            
            // Hide the lowest paddel
            showStandbyPaddles = ShowStandByPaddleAllButLast;

            animateLayerToFrame(paddle_layer,
                                &PADDLE_START_POS,
                                ANIM_DELAY,
                                ANIM_DUR_MOVE_NEXT_PADDLE_IN,
                                AnimationCurveEaseInOut,
                                &(AnimationHandlers) {
                                    .stopped = &animationSequenceMainCallback
                                },
                                &nextStep);
        }
            break;
        case 3:
            // Move the gamearea back in place
            animateLayerToFrame(gameLayer,
                                &GRect(0, 0, SCREEN_W, SCREEN_H),
                                ANIM_DELAY,
                                ANIM_DUR_HIDE_STANDBY_PADDLE,
                                AnimationCurveEaseOut,
                                &(AnimationHandlers) {
                                    .stopped = &animationSequenceMainCallback
                                },
                                &nextStep);
            break;
        case 4:
            showStandbyPaddles = ShowStandByPaddleNone;
			game_reset_balls(&game);
            paddle_reset(game.paddle);
            game_set_mode(&game, GameModeRunning);
            break;
            
        default:
            break;
    }
}

void nextLevelAnimation(uint8_t step)
{
    static AnimationStep nextStep;
    GRect frame;
    
    nextStep = (AnimationStep) {
        .performStep = &nextLevelAnimation,
        .step = step + 1
    };
    switch (step) {
        case 0:
            // Move paddle down out of game-area
            if (game.level.numBlocksVisible <= 0)
			{
                // If no more blocks are visible, we skip the step where the game-area is moved out of screen (step 1)
				nextStep.step++;
			}
            Layer* paddle_layer = bitmap_layer_get_layer(game.paddle->layer);
			frame = layer_get_frame(paddle_layer);
            frame.origin.y = SCREEN_H;
            animateLayerToFrame(paddle_layer,
                                &frame,
                                0,
                                500,
                                AnimationCurveEaseIn,
                                &(AnimationHandlers) {
                                    .stopped = &animationSequenceMainCallback
                                },
                                &nextStep);
            break;
		case 1:
			// Move game-area out of screen (just, if some visible blocks are left)
            frame = GRect(-SCREEN_W, 0, SCREEN_W, SCREEN_H);
            animateLayerToFrame(gameLayer,
                                &frame,
                                100,
                                1000,
                                AnimationCurveEaseIn,
                                &(AnimationHandlers) {
                                    .stopped = &animationSequenceMainCallback
                                },
                                &nextStep);
			break;
        case 2:
            // Move game-area a complete screen to the right, so we can animate it back in
            frame = GRect(SCREEN_W, 0, SCREEN_W, SCREEN_H);
            layer_set_frame(gameLayer, frame);
            
            // Load next level
            if (game.level.nr == NUM_LEVELS)
            {
                // Increase ball-speed upon finishing last level
                game.ball_speed_normal += BALL_SPEED_OFFSET_ALL_FINISHED;
                level_load(&game.level, 1);
            }
            else
            {
                level_load(&game.level, game.level.nr + 1);
            }
            
            updateLevelAndScore();

            frame.origin.x = 0 ;
            animateLayerToFrame(gameLayer,
                                &frame,
                                100,
                                1000,
                                AnimationCurveEaseOut,
                                &(AnimationHandlers) {
                                    .stopped = &animationSequenceMainCallback
                                },
                                &nextStep);

            break;
        case 3:
        {
            Layer* paddle_layer = bitmap_layer_get_layer(game.paddle->layer);
            layer_set_frame(paddle_layer, PADDLE_HIDDEN_POS);

            game_reset_balls(&game);
            
            paddle_reset(game.paddle);
            game_set_mode(&game, GameModeRunning);

            animateLayerToFrame(paddle_layer,
                                &PADDLE_START_POS,
                                100,
                                700,
                                AnimationCurveLinear,
                                &(AnimationHandlers) {
                                    .stopped = &animationSequenceMainCallback
                                },
                                &nextStep);
        }
            break;
        case 4:
            break;
    }
}

void gameFinishedAnimation(uint8_t step)
{
    static AnimationStep nextStep;
    
    nextStep = (AnimationStep) {
        .performStep = &gameFinishedAnimation,
        .step = step + 1
    };
    switch (step) {
        case 0:
            // Place Hint-layer above the screen
            layer_set_frame(text_layer_get_layer(hintTextLayer), HINT_TEXT_ABOVE_RECT);
            
            // set text and make layer visible
            updateHintText();
            layer_set_hidden(text_layer_get_layer(hintTextLayer), false);

            // Move the entire game-area down out of the screen
            animateLayerToFrame(gameLayer,
                                &GRect(0, SCREEN_H, SCREEN_W, SCREEN_H),
                                0,
                                750,
                                AnimationCurveEaseIn,
                                &(AnimationHandlers) {
                                    .stopped = &animationSequenceMainCallback
                                },
                                &nextStep);
            
            // Move hint-layer down
            animateLayerToFrame(text_layer_get_layer(hintTextLayer),
                                &HINT_TEXT_RECT,
                                0,
                                750,
                                AnimationCurveEaseIn,
                                NULL,
                                NULL);
            break;
        case 1:
            break;
    }
}



//
// Animation Callbacks
//
void animationSequenceMainCallback(struct Animation *animation, bool finished, void *context)
{
    AnimationStep *nextStep = (AnimationStep *)(context);

    // Call next step in animation sequnce
    nextStep->performStep(nextStep->step);
    
    // cleanup animation
    animation_destroy(animation);
}


//
// Game handler
//
void gameLifeLostHandler(Game *game)
{
    static const uint32_t const segments[] = { 30, 30, 30, 30, 50 };
    static VibePattern pat = {
        .durations = segments,
        .num_segments = ARRAY_LENGTH(segments),
    };
    
    if (pebloidSettings.is_sound_enabled)
    {
        vibes_enqueue_custom_pattern(pat);
    }

    if (game->num_lives > 0)
    {
        newPaddleAnimation(0);
    }
}

void gameScoreChangedHandler(Game *game)
{
#ifndef SHOW_FPS
    updateLevelAndScore();
#endif
}

void gameLevelFinishedHandler(Game *game)
{
    nextLevelAnimation(0);
}

void gameModeChangedHandler(Game *game)
{
    // Adjust UI according to gamemode
    switch (game->mode) {
        case GameModeAppStarted:
            break;
        case GameModeFinished:
            gameFinishedAnimation(0);
            break;
        case GameModeIdle:
            break;
        case GameModePause:
            updateHintText();
            layer_set_hidden(text_layer_get_layer(hintTextLayer), false);
            layer_set_hidden(dimLayer, false);
            break;
        case GameModeRunning:
            layer_set_hidden(text_layer_get_layer(hintTextLayer), true);
            layer_set_hidden(dimLayer, true);
            app_timer_register(GAMELOOP_TIMER_INTERVALL, handleTimer, NULL);
            if (pebloidSettings.keep_backlight_on)
            {
              light_enable(true);
            }
            else
            {
              light_enable(false);
            }
            break;
            
        default:
            break;
    }
}

void gameBlockHitHandler(Game *game)
{
    static const uint32_t const segments[] = { 10, 10, 10 };
    static VibePattern pat = {
        .durations = segments,
        .num_segments = ARRAY_LENGTH(segments),
    };
    
    if (pebloidSettings.is_sound_enabled)
    {
        vibes_enqueue_custom_pattern(pat);
    }
}



//
// Misc
//
void updateLevelAndScore()
{
    snprintf(infoText, sizeof(infoText), "Score:%04d  Lvl:%02d", game.score, game.level.nr);
    text_layer_set_text(infoTextLayer, infoText);
}

void updateHintText()
{
    char *text = "";
    
    if (game.mode == GameModePause)
    {
        text = "Game paused";
    }
    else if (game.mode == GameModeFinished)
    {
        text = "Game over";
    }
    else if (game.mode == GameModeAppStarted)
    {
        text = "Pebloid";
    }
    snprintf(hintText, sizeof(hintText), format_game_hint, text, button_names_names[pebloidSettings.button[GameControlPlayPause]]);
    text_layer_set_text(hintTextLayer, hintText);
}

void tap_handler(AccelAxisType axis, int32_t dir) {
  game_lauch_glued_balls(&game);
}

void accel_handler(AccelData* accel_data, uint32_t samples_available) {
  AccelData avg;
  uint32_t x = 0;
  for(uint32_t i = 0; i < samples_available; i++) {
	  x += accel_data[i].x;
  }
  x /= samples_available;
  avg.x = (int)x;

  accel_history[accel_pos++] = avg.x;
  if(accel_pos > ACCEL_SIZE)
    accel_pos = 0;
  // Take average of history, if ACCEL_SIZE grows then should not compute from scratch each time
  int tmp_accel_mid = 0;
  for(int i = 0; i < ACCEL_SIZE; i++)
    tmp_accel_mid += accel_history[i];
  accel_mid = tmp_accel_mid / ACCEL_SIZE;
  if(avg.x < accel_mid + ACCEL_MIN && avg.x > accel_mid - ACCEL_MIN)
    avg.x = accel_mid;
  if(avg.x > accel_mid + ACCEL_MAX)
    avg.x = accel_mid + ACCEL_MAX;
  else if(avg.x < accel_mid - ACCEL_MAX)
    avg.x = accel_mid - ACCEL_MAX;
  //int err = avg.x - prev_accel - d_accel; 
  //int acc = avg.x + 4 * err / 5;
  //d_accel = d_accel + err;
  //prev_accel = acc;
  float f = (avg.x - accel_mid) / (float)ACCEL_MAX;
  game.paddle->vector = f;
}
//
// Button handler
//
void pressed_handler(ClickRecognizerRef recognizer, void *context)
{
    
}

void released_handler(ClickRecognizerRef recognizer, void *context)
{
   
}

void single_click_handler(ClickRecognizerRef recognizer, void *context)
{
    ButtonId button = click_recognizer_get_button_id(recognizer);
    
    if (button == pebloidSettings.button[GameControlPlayPause])
    {
        if (game.mode == GameModeRunning)
        {
            if (!game_lauch_glued_balls(&game))
            {
                game_set_mode(&game, GameModePause);
            }
        }
        else if (game.mode == GameModeIdle)
        {
        }
        else if (game.mode == GameModePause)
        {
            game_set_mode(&game, GameModeRunning);
        }
        else if ((game.mode == GameModeFinished) || (game.mode == GameModeAppStarted))
        {
            layer_set_frame(gameLayer, GRect(0, 0, SCREEN_W, SCREEN_H));
            layer_set_hidden(text_layer_get_layer(hintTextLayer), true);
            
            // in nextLevelAnimation the n+1th level is loaded and the gameloop is started.
            game_reset(&game);
            game.level.nr = 0;
            nextLevelAnimation(0);
        }
    }
}

void select_long_click_handler(ClickRecognizerRef recognizer, void *context)
{
    ButtonId button = click_recognizer_get_button_id(recognizer);
    
    if (button == BUTTON_ID_SELECT)
    {
        if (game.mode != GameModeRunning)
        {
            settings_edit(&pebloidSettings);
        }
#ifdef DEBUG_SKIP_LEVEL
        if (game.mode == GameModeRunning)
        {
            game.level.numBlocksLeft = 0;
        }
#endif
    }
}

void select_long_click_release_handler(ClickRecognizerRef recognizer, Window *window)
{
    (void)recognizer;
    (void)window;
}

void click_config_provider(void *context)
{
    window_single_click_subscribe(BUTTON_ID_SELECT, single_click_handler);
    window_long_click_subscribe(BUTTON_ID_SELECT, 0, select_long_click_handler, NULL);
    window_raw_click_subscribe(BUTTON_ID_SELECT, pressed_handler, released_handler, NULL);

    window_single_click_subscribe(BUTTON_ID_UP, single_click_handler);
    
    window_single_click_subscribe(BUTTON_ID_DOWN, single_click_handler);
}


//
// gameLayer updateProc
//
void gameLayerUpdateProc(struct Layer *layer, GContext *ctx)
{
    int16_t x, y;
    GBitmap *bitmap;

    // Draw Blocks
    y = 2 * BLOCK_H + 1;
    for (register int row = 2; row < LEVEL_NUM_ROWS; row++)
    {
        x = 1;
        for (register int col = 0; col < LEVEL_NUM_COLS; col++)
        {
            Block currentBlock = game.level.blocks[row][col];
            if (currentBlock != BlockStateNoBlock)
            {
                switch (currentBlock) {
                    case BlockStateNormal:
                        bitmap = imgBlockNormal;
                        break;
                    case BlockStateUndestroyable:
                        bitmap = imgBlockSolid;
                        break;
                    case BlockStateDoubleHit1:
                        bitmap = imgBlockDoubleHit1;
                        break;
                    case BlockStateDoubleHit2:
                        bitmap = imgBlockDoubleHit2;
                        break;
                    default:
                        bitmap = NULL;
                        break;
                }
                graphics_draw_bitmap_in_rect(ctx, bitmap, GRect(x, y, BLOCK_W - 2, BLOCK_H - 2));
            }
            
            x += BLOCK_W;
        }
        
        y += BLOCK_H;
    }
    
    // Draw PowerUps
    for (register int i = 0; i < MAX_POWERUPS; i++)
    {
        PowerUp *currentPowerUp = &game.powerups[i];
        if (currentPowerUp->is_active)
        {
            switch (currentPowerUp->kind) {
                case PowerUpKindAddBall:
                    bitmap = imgPowerUpAddBall;
                    break;
                case PowerUpKindSlowDown:
                    bitmap = imgPowerUpSlowDown;
                    break;
                case PowerUpKindSpeedUp:
                    bitmap = imgPowerUpSpeedUp;
                    break;
                case PowerUpKindPowerBall:
                    bitmap = imgPowerUpPowerBall;
                    break;
                case PowerUpKindMissileLauncher:
                    bitmap = imgPowerUpMissileLauncher;
                    break;
                default:
                    bitmap = NULL;
                    break;
            }
            graphics_draw_bitmap_in_rect(ctx, bitmap, GRect(currentPowerUp->x, (int16_t)(currentPowerUp->y + 0.5f), POWERUP_W, POWERUP_H));
        }
    }

    // Draw Missiles
    for (register int i = 0; i < MAX_MISSILES; i++)
    {
        Missile *currentMissile = &game.missiles[i];
        if (currentMissile->is_active)
        {
            graphics_draw_bitmap_in_rect(ctx, imgMissile, missile_get_frame(currentMissile));
        }
    }
    
    // Draw Balls
    for (register int i = 0; i < MAX_BALLS; i++)
    {
        Ball *currentBall = &game.balls[i];
        if (currentBall->is_alive)
        {
            graphics_draw_bitmap_in_rect(ctx, imgBallNormal, ball_get_frame(currentBall));
        }
    }
    
    // Draw StandbyPaddles if needed
    if (showStandbyPaddles != ShowStandByPaddleNone)
    {
        for (register int i = (showStandbyPaddles == ShowStandByPaddleAll ? 0 : 1); i < game.num_lives; i++)
        {
            graphics_draw_bitmap_in_rect(ctx, imgPaddle, GRect(-PADDLE_W, PADDLE_Y - i * (PADDLE_H + 4), PADDLE_W, PADDLE_H));
        }
    }
#ifdef SHOW_FPS
    frames_per_second++;
#endif
}

void bottomInfoLayerUpdateProc(struct Layer *layer, GContext *ctx)
{
    graphics_context_set_fill_color(ctx, GColorWhite);
    graphics_fill_rect(ctx, layer_get_bounds(layer), 0, GCornerNone);

    graphics_context_set_stroke_color(ctx, GColorBlack);
    graphics_draw_line(ctx, GPoint(0,0), GPoint(SCREEN_W, 0));
    
    graphics_context_set_fill_color(ctx, GColorBlack);
    if (game.powerup_time_ball_slow > 0)
    {
        graphics_draw_bitmap_in_rect(ctx, imgPowerUpSlowDown, GRect(4, 4, POWERUP_W, POWERUP_H));
        graphics_fill_rect(ctx, GRect(22, 4, game.powerup_time_ball_slow * POWERUP_BAR_W / POWERUP_TIME_BALL_SLOW, POWERUP_H), 0, GCornerNone);
    }
    if (game.powerup_time_ball_fast > 0)
    {
        graphics_draw_bitmap_in_rect(ctx, imgPowerUpSpeedUp, GRect(4, 4, POWERUP_W, POWERUP_H));
        graphics_fill_rect(ctx, GRect(22, 4, game.powerup_time_ball_fast * POWERUP_BAR_W / POWERUP_TIME_BALL_FAST, POWERUP_H), 0, GCornerNone);
    }
    if (game.powerup_time_power_ball > 0)
    {
        graphics_draw_bitmap_in_rect(ctx, imgPowerUpPowerBall, GRect(46, 4, POWERUP_W, POWERUP_H));
        graphics_fill_rect(ctx, GRect(64, 4, game.powerup_time_power_ball * POWERUP_BAR_W/ POWERUP_TIME_POWER_BALL, POWERUP_H), 0, GCornerNone);
    }
    if (game.powerup_time_missile_launcher > 0)
    {
        graphics_draw_bitmap_in_rect(ctx, imgPowerUpMissileLauncher, GRect(88, 4, POWERUP_W, POWERUP_H));
        graphics_fill_rect(ctx, GRect(106, 4, game.powerup_time_missile_launcher * POWERUP_BAR_W/ POWERUP_TIME_MISSILE_LAUNCHER, POWERUP_H), 0, GCornerNone);
    }
}


//
// App focus handler

void app_focus_handler(bool in_focus)
{
    if (!in_focus)
    {
        if (game.mode == GameModeRunning)
        {
            game_set_mode(&game, GameModePause);
        }
    }
}


//
// Window handler
//
void main_window_handle_appear(struct Window *window)
{
    updateHintText();
}


// App handler
void handle_init(void)
{
    // Einstellungen laden
    settings_load(&pebloidSettings);
    pebloidSettings.keep_backlight_on = true;

    // Fonts
    info_font = fonts_load_custom_font(resource_get_handle(RESOURCE_ID_FONT_INFO_8));
    hint_font = fonts_load_custom_font(resource_get_handle(RESOURCE_ID_FONT_HINT_16));
    
	// Mainwindow
    window = window_create();
    window_set_fullscreen(window, true);
	window_set_click_config_provider(window, click_config_provider);
    window_set_window_handlers(window, (WindowHandlers) {
        .appear = (WindowHandler)main_window_handle_appear,
    });

    accel_data_service_subscribe(5, accel_handler);
    accel_tap_service_subscribe(tap_handler);
    Layer* window_layer = window_get_root_layer(window);
	
    // Textlayer info
    infoTextLayer = text_layer_create(INFO_TEXT_RECT);
    text_layer_set_font(infoTextLayer, info_font);
    text_layer_set_text(infoTextLayer, "(c)2013 Dirk Mika");
	layer_add_child(window_layer, text_layer_get_layer(infoTextLayer));
    
	// Gamelayer
    gameLayer = layer_create(layer_get_frame(window_layer));
    layer_set_clips(gameLayer, false);
    layer_set_update_proc(gameLayer, &gameLayerUpdateProc);
	layer_add_child(window_layer, gameLayer);
    
    // dimLayer
    dimLayer = dim_layer_create(layer_get_frame(window_layer));
    layer_add_child(window_layer, dimLayer);
    layer_set_hidden(dimLayer, true);
    
    // Bottom InfoLayer
    bottomInfoLayer = layer_create(BOTTOM_INFO_TEXT_RECT);
    layer_set_update_proc(bottomInfoLayer, &bottomInfoLayerUpdateProc);
	layer_add_child(window_layer, bottomInfoLayer);
    
    // hintTextLayer
    hintTextLayer = text_layer_create(HINT_TEXT_RECT);
    text_layer_set_text_color(hintTextLayer, GColorBlack);
    text_layer_set_background_color(hintTextLayer, GColorClear);
    text_layer_set_font(hintTextLayer, hint_font);
    text_layer_set_text_alignment(hintTextLayer, GTextAlignmentCenter);
	layer_add_child(window_layer, text_layer_get_layer(hintTextLayer));
    
    // Init App-Specific stuff of some 'classes' (needs deinited!)
    ball_app_init();
    block_app_init();
    powerup_app_init();
    missile_app_init();
    
    // Init-Gamedata
    game_init(&game);
	game.gameHandlers = (GameHandlers){
		.lifeLost = gameLifeLostHandler,
		.levelFinished = gameLevelFinishedHandler,
        .scoreChanged = gameScoreChangedHandler,
        .modeChanged = gameModeChangedHandler,
        .blockHit = gameBlockHitHandler
	};
    if (pebloidSettings.has_saved_game)
    {
        if (game_load(&game))
        {
            game_set_mode(&game, GameModePause);
        }
    }
    
    // Paddle
	layer_add_child(gameLayer, bitmap_layer_get_layer(game.paddle->layer));

	// Show window
	window_stack_push(window, true);
    
    // Register for app focus events
    app_focus_service_subscribe(app_focus_handler);
    
    // Init random-generator
    srand(time(NULL));
    
#ifdef SHOW_FPS
    tick_timer_service_subscribe(SECOND_UNIT, handleTick);
#endif
}

void handle_deinit(void)
{
#ifdef SHOW_FPS
    tick_timer_service_unsubscribe();
#endif

    app_focus_service_unsubscribe();
    
    // Save game if active
    if (game.mode == GameModePause || game.mode == GameModeRunning || game.mode == GameModeIdle)
    {
        game_save(&game);
        pebloidSettings.has_saved_game = true;
    }
    else
    {
        pebloidSettings.has_saved_game = false;
    }

    // Save settings
    settings_save(&pebloidSettings);
    
    text_layer_destroy(hintTextLayer);
    
    missile_app_deinit();
    powerup_app_deinit();
    block_app_deinit();
    ball_app_deinit();

    layer_destroy(bottomInfoLayer);
    
    dim_layer_destroy(dimLayer);
    
    layer_destroy(gameLayer);
    
    game_deinit(&game);
    
    text_layer_destroy(infoTextLayer);
    
    window_destroy(window);
    
	fonts_unload_custom_font(info_font);
	fonts_unload_custom_font(hint_font);
}

void handleTimer(void *data)
{
    if (game.mode == GameModeRunning)
    {
        // First, queue the timer-event
        app_timer_register(GAMELOOP_TIMER_INTERVALL, handleTimer, NULL);
        
        // Update gamelogic
        game_update(&game);
        
        // Force redraw of gamelayer
        layer_mark_dirty(gameLayer);
    }
}

#ifdef SHOW_FPS
void handleTick(struct tm *tick_time, TimeUnits units_changed)
{
    snprintf(infoText, sizeof(infoText), "%03d", frames_per_second);
    text_layer_set_text(infoTextLayer, infoText);
    frames_per_second = 0;
}
#endif


int main(void)
{
    handle_init();
    app_event_loop();
    handle_deinit();
}


