//
//  settings.c
//  
//
//  Created by Dirk Mika on 01.05.13.
//
//


#include "settings.h"
#include "common.h"


static Window* window;
static SimpleMenuLayer* menu_layer;
static SimpleMenuSection menu_sections[3];
static SimpleMenuItem menu_section0_items[3];  // Control...
static SimpleMenuItem menu_section1_items[2];  // Misc...
static SimpleMenuItem menu_section2_items[1];  // About...

const char *button_names_names[] = { "Back", "up", "select", "down" };

GBitmap* menu_icon_0_0;
GBitmap* menu_icon_0_1;
GBitmap* menu_icon_0_2;
GBitmap* menu_icon_1_0;
GBitmap* menu_icon_1_1;
GBitmap* menu_icon_2_0;


// Prototypes

void settings_to_menu();
void menu_layer_section0_select_callback(int index, void *context);
void menu_layer_section1_select_callback(int index, void *context);


// Private

void settings_to_menu(PebloidSettings* settings)
{
    static char subtitles[3][16];
    
    for (register int i = 0; i < 3; i++)
    {
        snprintf(subtitles[i], sizeof(subtitles[i]), "Button '%s'", button_names_names[settings->button[i]]);
        menu_section0_items[i].subtitle = subtitles[i];
    }
}

void menu_layer_section0_select_callback(int index, void *context)
{
    PebloidSettings* settings = (PebloidSettings*)context;
    
    register ButtonId oldValue = settings->button[index];
    settings->button[index] = settings->button[index] + 1;
    if (settings->button[index] == NUM_BUTTONS)
    {
        settings->button[index] = BUTTON_ID_UP;
    }

    register int otherButton1 = (index + 1) % 3;
    register int otherButton2 = (index + 2) % 3;
    
    if (settings->button[otherButton1] == settings->button[index])
    {
        settings->button[otherButton1] = oldValue;
    }
    if (settings->button[otherButton2] == settings->button[index])
    {
        settings->button[otherButton2] = oldValue;
    }

    settings_to_menu(settings);
    menu_layer_reload_data((MenuLayer*)menu_layer);
}

void menu_layer_section1_select_callback(int index, void *context)
{
    PebloidSettings* settings = (PebloidSettings*)context;

    switch (index) {
        case 0:
            settings->keep_backlight_on = !settings->keep_backlight_on;
            menu_section1_items[0].subtitle = (settings->keep_backlight_on ? "Always on" : "Default");
            break;
        case 1:
            settings->is_sound_enabled = !settings->is_sound_enabled;
            menu_section1_items[1].subtitle = (settings->is_sound_enabled ? "On" : "Off");
            break;
            
        default:
            break;
    }
    menu_layer_reload_data((MenuLayer*)menu_layer);
}

void handle_appear(Window *window)
{
    scroll_layer_set_frame((ScrollLayer*)menu_layer, layer_get_bounds(window_get_root_layer(window)));
}

void handle_unload(Window *window)
{
    gbitmap_destroy(menu_icon_0_0);
    gbitmap_destroy(menu_icon_0_1);
    gbitmap_destroy(menu_icon_0_2);
    gbitmap_destroy(menu_icon_1_0);
    gbitmap_destroy(menu_icon_1_1);
    gbitmap_destroy(menu_icon_2_0);
}

void init_settings_windws(PebloidSettings* settings)
{
    window = window_create();
    window_set_background_color(window, GColorWhite);
    window_set_window_handlers(window, (WindowHandlers) {
        .appear = (WindowHandler)handle_appear,
        .unload = (WindowHandler)handle_unload
    });
    
    menu_icon_0_0 = gbitmap_create_with_resource(RESOURCE_ID_IMG_SETTINGS_LEFT);
    menu_icon_0_1 = gbitmap_create_with_resource(RESOURCE_ID_IMG_SETTINGS_RIGHT);
    menu_icon_0_2 = gbitmap_create_with_resource(RESOURCE_ID_IMG_SETTINGS_PLAY_PAUSE);
    menu_icon_1_0 = gbitmap_create_with_resource(RESOURCE_ID_IMG_SETTINGS_BACKLIGHT);
    menu_icon_1_1 = gbitmap_create_with_resource(RESOURCE_ID_IMG_SETTINGS_SOUND);
    menu_icon_2_0 = gbitmap_create_with_resource(RESOURCE_ID_IMAGE_MENU_ICON);
    
    // Section "Control..."
    menu_section0_items[0] = (SimpleMenuItem) {
        .title = "Move left",
        .icon = menu_icon_0_0,
        .callback = &menu_layer_section0_select_callback
    };
    menu_section0_items[1] = (SimpleMenuItem) {
        .title = "Move right",
        .icon = menu_icon_0_1,
        .callback = &menu_layer_section0_select_callback
    };
    menu_section0_items[2] = (SimpleMenuItem) {
        .title = "Play / Pause",
        .icon = menu_icon_0_2,
        .callback = &menu_layer_section0_select_callback
    };
    // Header
    menu_sections[0] = (SimpleMenuSection) {
        .title = "Control...",
        .items = menu_section0_items,
        .num_items = ARRAY_LENGTH(menu_section0_items)
    };
    
    // Section "Misc..."
    menu_section1_items[0] = (SimpleMenuItem) {
        .title = "Backlight",
        .subtitle = (settings->keep_backlight_on ? "Always on" : "Default"),
        .icon = menu_icon_1_0,
        .callback = menu_layer_section1_select_callback
    };
    menu_section1_items[1] = (SimpleMenuItem) {
        .title = "'Sound'",
        .subtitle = (settings->is_sound_enabled ? "On" : "Off"),
        .icon = menu_icon_1_1,
        .callback = menu_layer_section1_select_callback
    };
    // Header
    menu_sections[1] = (SimpleMenuSection) {
        .title = "Misc...",
        .items = menu_section1_items,
        .num_items = ARRAY_LENGTH(menu_section1_items)
    };
    
    // Section "About..."
    menu_section2_items[0] = (SimpleMenuItem) {
        .title = "Pebloid V3.0",
        .subtitle = "(C)2013 Dirk Mika",
        .icon = menu_icon_2_0,
        .callback = NULL
    };
    // Header
    menu_sections[2] = (SimpleMenuSection) {
        .title = "About...",
        .items = menu_section2_items,
        .num_items = ARRAY_LENGTH(menu_section2_items)
    };
    
    menu_layer = simple_menu_layer_create(layer_get_bounds(window_get_root_layer(window)), window, menu_sections, 3, settings);
    layer_add_child(window_get_root_layer(window), simple_menu_layer_get_layer(menu_layer));
}


// Public

void settings_load(PebloidSettings* settings)
{
    PebloidSettings defaults = { {BUTTON_ID_UP, BUTTON_ID_DOWN, BUTTON_ID_SELECT},
                                 false,                                             // has_saved_game
                                 false,                                             // keep_backlight_on
                                 false                                              // is_sound_enabled
    };
    
    *settings = defaults;
    
    if (persist_exists(123))
    {
        size_t len = persist_get_size(123);
        persist_read_data(123, settings, len);
    }
}

void settings_edit(PebloidSettings* settings)
{
    init_settings_windws(settings);
    settings_to_menu(settings);
    window_stack_push(window, true);
}

void settings_save(PebloidSettings* settings)
{
    persist_write_data(123, settings, sizeof(PebloidSettings));
}




