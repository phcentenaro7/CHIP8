#pragma once
#include <allegro5/allegro.h>
#include <allegro5/allegro_font.h>
#include "machine.h"

typedef enum DEBUG_KEY_INDEX
{
	DEBUG_ENABLED,
	DEBUG_STEP_BY_STEP,
	DEBUG_NEXT_STEP,
	DEBUG_SHOW_STATE,
	NUM_DEBUG_OPTIONS
}DEBUG_KEY_INDEX;

typedef struct DEBUG_SETTINGS
{
	bool options[NUM_DEBUG_OPTIONS];
	uint8_t keys[NUM_DEBUG_OPTIONS];
	ALLEGRO_COLOR text_color;
	ALLEGRO_FONT* text_font;
	uint8_t font_size;
	uint16_t display_width;
	uint16_t display_height;
}DEBUG_SETTINGS;

typedef struct DEBUG DEBUG;

DEBUG* create_debug(MACHINE* machine);
void delete_debug(DEBUG* debug);
void start_debug_thread(DEBUG* debug);