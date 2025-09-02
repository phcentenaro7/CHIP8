#include <allegro5/allegro.h>
#include <allegro5/allegro_primitives.h>
#include <allegro5/allegro_image.h>
#include <allegro5/allegro_audio.h>
#include <allegro5/allegro_acodec.h>
#include <allegro5/allegro_font.h>
#include <allegro5/allegro_ttf.h>
#include <allegro5/allegro_native_dialog.h>
#include <assert.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include "machine.h"
#include "struct_machine.h"
#include "struct_debug.h"
#include "opcodes.h"
#include <stdio.h>
#include <stdbool.h>
#include <time.h>

static void prepare_display(MACHINE* machine, DISPLAY_OPTIONS display_options);
static void prepare_bitmaps(MACHINE* machine, DISPLAY_OPTIONS display_options);
static void prepare_timers(MACHINE* machine);
static void prepare_audio(MACHINE* machine);
static void prepare_event_queue(MACHINE* machine);
static void update_display(MACHINE* machine);
static void update_counters(MACHINE* machine);
static void handle_timer_events(MACHINE* machine, ALLEGRO_EVENT event);
static void handle_keypad_events(MACHINE* machine, ALLEGRO_EVENT event);
static void handle_display_events(MACHINE* machine, ALLEGRO_EVENT event);

bool start_allegro()
{
	al_init();
	al_install_keyboard();
	al_install_audio();
	al_init_acodec_addon();
	al_init_primitives_addon();
	al_init_image_addon();
	al_init_font_addon();
	al_init_ttf_addon();
	al_init_native_dialog_addon();
	return true;
}

bool end_allegro()
{
	/*al_uninstall_keyboard();
	al_uninstall_audio();
	al_shutdown_primitives_addon();
	al_shutdown_image_addon();
	al_shutdown_font_addon();
	al_shutdown_ttf_addon();
	al_shutdown_native_dialog_addon();*/
	return true;
}

static void prepare_display(MACHINE* machine, DISPLAY_OPTIONS display_options)
{
	machine->display_options = display_options;
	machine->display = al_create_display(DEFAULT_DISPLAY_WIDTH * display_options.scale, DEFAULT_DISPLAY_HEIGHT * display_options.scale);
	assert(machine->display);
	al_set_window_title(machine->display, "C8 - CHIP8 Emulator");
}

static void prepare_bitmaps(MACHINE* machine, DISPLAY_OPTIONS display_options)
{
	machine->pixel_on = al_create_bitmap(display_options.scale, display_options.scale);
	assert(machine->pixel_on);
	al_set_target_bitmap(machine->pixel_on);
	al_draw_filled_rectangle(0, 0, display_options.scale, display_options.scale, display_options.color_on);
	machine->pixel_off = al_create_bitmap(display_options.scale, display_options.scale);
	assert(machine->pixel_off);
	al_set_target_bitmap(machine->pixel_off);
	al_draw_filled_rectangle(0, 0, display_options.scale, display_options.scale, display_options.color_off);
	al_set_target_backbuffer(machine->display);
}

static void prepare_timers(MACHINE* machine)
{
	machine->counter_timer = al_create_timer(DEFAULT_COUNTER_TIMER_PERIOD);
	assert(machine->counter_timer);
	al_start_timer(machine->counter_timer);
	machine->opcode_timer = al_create_timer(DEFAULT_OPCODE_TIMER_PERIOD);
	assert(machine->opcode_timer);
	al_start_timer(machine->opcode_timer);
}

static void prepare_audio(MACHINE* machine)
{
	machine->beep = al_load_sample("resources/sound.wav");
	assert(machine->beep);
	al_reserve_samples(1);
	machine->beep_playing = false;
}

static void prepare_event_queue(MACHINE* machine)
{
	machine->event_queue = al_create_event_queue();
	assert(machine->event_queue);
	al_register_event_source(machine->event_queue, al_get_keyboard_event_source());
	al_register_event_source(machine->event_queue, al_get_display_event_source(machine->display));
	al_register_event_source(machine->event_queue, al_get_timer_event_source(machine->counter_timer));
	al_register_event_source(machine->event_queue, al_get_timer_event_source(machine->opcode_timer));
}

MACHINE* create_machine(DISPLAY_OPTIONS display_options)
{
	MACHINE* machine = calloc(sizeof(MACHINE), 1);
	assert(machine);
	machine->on = true;
	machine->pc_reg = PROGRAM_BASE_ADDRESS;
	machine->s_reg = STACK_BASE_ADDRESS;

	uint8_t font[FONT_MEMORY_SIZE] = DEFAULT_FONT_MEMORY_CONTENT;
	set_font(machine, font);

	INPUT_KEY** keypad = create_default_keypad();
	set_keypad(machine, keypad);

	prepare_display(machine, display_options);
	prepare_bitmaps(machine, display_options);
	prepare_timers(machine);
	prepare_audio(machine);
	prepare_event_queue(machine);
	machine->debug = create_debug(machine);
	srand(time(NULL));
	return machine;
}

void delete_machine(MACHINE* machine)
{
	al_destroy_event_queue(machine->event_queue);
	al_destroy_bitmap(machine->pixel_on);
	al_destroy_bitmap(machine->pixel_off);
	al_destroy_timer(machine->counter_timer);
	al_destroy_timer(machine->opcode_timer);
	al_destroy_sample(machine->beep);
	al_destroy_display(machine->display);
	for (uint8_t i = 0; i < KEYPAD_HEIGHT; i++)
	{
		free(machine->keypad[i]);
	}
	free(machine->keypad);
	delete_debug(machine->debug);
	free(machine);
}

void set_font(MACHINE* machine, int8_t* font)
{
	memcpy(machine->RAM + FONT_MEMORY_BASE_ADDRESS, font, FONT_MEMORY_SIZE);
}

INPUT_KEY** create_default_keypad()
{
	INPUT_KEY** keypad = malloc(sizeof(INPUT_KEY*) * KEYPAD_HEIGHT);
	if (!keypad) return NULL;
	for (uint8_t i = 0; i < KEYPAD_HEIGHT; i++)
	{
		keypad[i] = malloc(sizeof(INPUT_KEY) * KEYPAD_WIDTH);
		if (!keypad[i]) return NULL;
	}
	keypad[0][0] = (INPUT_KEY){ 1, ALLEGRO_KEY_1 };
	keypad[0][1] = (INPUT_KEY){ 2, ALLEGRO_KEY_2 };
	keypad[0][2] = (INPUT_KEY){ 3, ALLEGRO_KEY_3 };
	keypad[0][3] = (INPUT_KEY){ 12, ALLEGRO_KEY_4 };
	keypad[1][0] = (INPUT_KEY){ 4, ALLEGRO_KEY_Q };
	keypad[1][1] = (INPUT_KEY){ 5, ALLEGRO_KEY_W };
	keypad[1][2] = (INPUT_KEY){ 6, ALLEGRO_KEY_E };
	keypad[1][3] = (INPUT_KEY){ 13, ALLEGRO_KEY_R };
	keypad[2][0] = (INPUT_KEY){ 7, ALLEGRO_KEY_A };
	keypad[2][1] = (INPUT_KEY){ 8, ALLEGRO_KEY_S };
	keypad[2][2] = (INPUT_KEY){ 9, ALLEGRO_KEY_D };
	keypad[2][3] = (INPUT_KEY){ 14, ALLEGRO_KEY_F };
	keypad[3][0] = (INPUT_KEY){ 10, ALLEGRO_KEY_Z };
	keypad[3][1] = (INPUT_KEY){ 0, ALLEGRO_KEY_X };
	keypad[3][2] = (INPUT_KEY){ 11, ALLEGRO_KEY_C };
	keypad[3][3] = (INPUT_KEY){ 15, ALLEGRO_KEY_V };
	return keypad;
}

void set_keypad(MACHINE* machine, INPUT_KEY** keypad)
{
	machine->keypad = keypad;
}

void load_program(MACHINE* machine, const char* file_name)
{
	FILE* file = fopen(file_name, "rb");
	fseek(file, 0, SEEK_END);
	uint16_t file_size = ftell(file);
	fseek(file, 0, SEEK_SET);
	fread(machine->RAM + PROGRAM_BASE_ADDRESS, BYTE_SIZE, file_size, file);
	fclose(file);
	uint16_t opcode = *(uint16_t*)(machine->RAM + machine->pc_reg);
	opcode = ((opcode >> 8) & 0x00FF) | (opcode << 8);
	machine->current_opcode = opcode;
}

static void update_display(MACHINE* machine)
{
	al_set_target_backbuffer(machine->display);
	uint16_t x = 0;
	for (uint64_t i = (uint64_t)1 << 63; i > 0; i >>= 1)
	{
		uint16_t y = 0;
		for (uint8_t j = 0; j < NUM_PIXEL_ROWS; j++)
		{
			if ((machine->pixel_row[j] & i) != 0)
			{
				al_draw_bitmap(machine->pixel_on, x, y, 0);
			}
			else
			{
				al_draw_bitmap(machine->pixel_off, x, y, 0);
			}
			y += machine->display_options.scale;
		}
		x += machine->display_options.scale;
	}
}

static void update_counters(MACHINE* machine)
{
	if (machine->d_counter > 0)
	{
		machine->d_counter--;
	}
	if (machine->s_counter > 0)
	{
		if (!machine->beep_playing)
		{
			al_play_sample(machine->beep, 1, 0, 1, ALLEGRO_PLAYMODE_LOOP, &machine->beep_id);
			machine->beep_playing = true;
		}
		machine->s_counter--;
	}
	else if (machine->beep_playing)
	{
		machine->beep_playing = false;
		al_stop_sample(&machine->beep_id);
	}
}

static void handle_timer_events(MACHINE* machine, ALLEGRO_EVENT event)
{
	if(event.type != ALLEGRO_EVENT_TIMER)
	{
		return;
	}
	if (event.timer.source == machine->opcode_timer)
	{
		uint16_t opcode = fetch_opcode(machine);
		execute_opcode(machine, opcode);
	}
	else if (event.timer.source == machine->counter_timer)
	{
		update_counters(machine);
		update_display(machine);
		al_set_target_backbuffer(machine->display);
		al_flip_display();
	}
}

static void handle_keypad_events(MACHINE* machine, ALLEGRO_EVENT event)
{
	if (event.type != ALLEGRO_EVENT_KEY_DOWN && event.type != ALLEGRO_EVENT_KEY_UP)
	{
		return;
	}
	for (uint8_t i = 0; i < KEYPAD_HEIGHT; i++)
	{
		for (uint8_t j = 0; j < KEYPAD_WIDTH; j++)
		{
			if (event.keyboard.keycode == machine->keypad[i][j].keycode)
			{
				machine->key_pressed[machine->keypad[i][j].value] = (event.type == ALLEGRO_EVENT_KEY_DOWN);
				return;
			}
		}
	}
}

static void handle_display_events(MACHINE* machine, ALLEGRO_EVENT event)
{
	if (event.display.source != machine->display)
	{
		return;
	}
	switch (event.type)
	{
	case ALLEGRO_EVENT_DISPLAY_CLOSE:
		machine->on = false;
	}
}

void run_program(MACHINE* machine)
{
	start_debug_thread(machine->debug);
	while (machine->on)
	{
		ALLEGRO_EVENT event;
		al_wait_for_event(machine->event_queue, &event);
		al_lock_mutex(machine->debug->event_mutex);
		handle_timer_events(machine, event);
		handle_keypad_events(machine, event);
		handle_display_events(machine, event);
		al_unlock_mutex(machine->debug->event_mutex);
	}
}