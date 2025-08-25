#pragma once
#include <allegro5/allegro.h>
#include <allegro5/allegro_color.h>
#include <stdbool.h>

typedef struct DISPLAY_OPTIONS
{
	uint8_t scale;
	ALLEGRO_COLOR color_on;
	ALLEGRO_COLOR color_off;
}DISPLAY_OPTIONS;

typedef struct INPUT_KEY
{
	uint8_t value;
	uint8_t keycode;
}INPUT_KEY;

typedef struct MACHINE MACHINE;

bool start_allegro();
bool end_allegro();
MACHINE* create_machine(DISPLAY_OPTIONS display_options);
void delete_machine(MACHINE* machine);
void set_font(MACHINE* machine, int8_t* font);
INPUT_KEY** create_default_keypad();
void set_keypad(MACHINE* machine, INPUT_KEY** keypad);
void load_program(MACHINE* machine, const char* file_name);
void run_program(MACHINE* machine);