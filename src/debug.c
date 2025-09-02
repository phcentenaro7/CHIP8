#include <allegro5/allegro_font.h>
#include <allegro5/allegro_ttf.h>
#include <stdio.h>
#include <varargs.h>
#include "debug.h"
#include "struct_debug.h"
#include "struct_machine.h"

#define BOOL_STR(cond) cond ? "True" : "False" 

static void* handle_events(ALLEGRO_THREAD* thread, void* debug);
static void handle_timer_events(DEBUG* debug, ALLEGRO_EVENT event);
static void handle_keyboard_events(DEBUG* debug, ALLEGRO_EVENT event);
static void draw_debug_text(DEBUG* debug);


static DEBUG_SETTINGS create_default_debug_settings()
{
	DEBUG_SETTINGS debug_settings;
	debug_settings.font_size = 12;
	debug_settings.text_color = al_map_rgb(255, 255, 255);
	debug_settings.text_font = al_load_ttf_font("resources/UbuntuMono[wght].ttf", 18, 0);
	debug_settings.keys[DEBUG_ENABLED] = ALLEGRO_KEY_J;
	debug_settings.keys[DEBUG_STEP_BY_STEP] = ALLEGRO_KEY_K;
	debug_settings.keys[DEBUG_NEXT_STEP] = ALLEGRO_KEY_L;
	debug_settings.options[DEBUG_STEP_BY_STEP] = true;
	debug_settings.display_width = 400;
	debug_settings.display_height = 400;
	return debug_settings;
}

DEBUG* create_debug(MACHINE* machine)
{
	DEBUG* debug = calloc(1, sizeof(DEBUG));
	assert(debug);
	debug->settings = create_default_debug_settings();
	debug->machine = machine;
	debug->thread = al_create_thread(handle_events, debug);
	debug->display = al_create_display(debug->settings.display_width, debug->settings.display_height);
	assert(debug->display);
	al_set_window_title(debug->display, "C8 DEBUG");
	debug->refresh_timer = al_create_timer(1 / 30.0);
	assert(debug->refresh_timer);
	al_start_timer(debug->refresh_timer);
	debug->event_queue = al_create_event_queue();
	assert(debug->event_queue);
	debug->event_mutex = al_create_mutex();
	al_register_event_source(debug->event_queue, al_get_keyboard_event_source());
	al_register_event_source(debug->event_queue, al_get_timer_event_source(debug->refresh_timer));
	return debug;
}

void delete_debug(DEBUG* debug)
{
	al_destroy_mutex(debug->event_mutex);
	al_destroy_event_queue(debug->event_queue);
	al_destroy_display(debug->display);
}

static void* handle_events(ALLEGRO_THREAD* thread, void* debug)
{
	DEBUG* dbg = debug;
	ALLEGRO_EVENT event;
	while (true)
	{
		al_wait_for_event(dbg->event_queue, &event);
		al_lock_mutex(dbg->event_mutex);
		handle_timer_events(debug, event);
		handle_keyboard_events(debug, event);
		al_unlock_mutex(dbg->event_mutex);
	}
}

static void handle_timer_events(DEBUG* debug, ALLEGRO_EVENT event)
{
	if (event.type != ALLEGRO_EVENT_TIMER)
	{
		return;
	}
	if (event.timer.source == debug->refresh_timer)
	{
		draw_debug_text(debug);
	}
}

static void handle_keyboard_events(DEBUG* debug, ALLEGRO_EVENT event)
{
	if (event.type != ALLEGRO_EVENT_KEY_DOWN)
	{
		return;
	}
	for (uint8_t i = 0; i < NUM_DEBUG_OPTIONS; i++)
	{
		if (debug->settings.keys[i] == event.keyboard.keycode)
		{
			debug->settings.options[i] = !debug->settings.options[i];
		}
	}
}

void start_debug_thread(DEBUG* debug)
{
	al_start_thread(debug->thread);
}

void delete_debug_thread(DEBUG* debug)
{
	al_destroy_thread(debug->thread);
}

static void draw_debug_text(DEBUG* debug)
{
	MACHINE* machine = debug->machine;
	char asm_text[32] = "";
	opcode_to_string(asm_text, machine->current_opcode);
	al_set_target_backbuffer(debug->display);
	al_clear_to_color(al_map_rgb(0, 0, 0));
	al_draw_multiline_textf(debug->settings.text_font,
		debug->settings.text_color,
		0,
		0, 
		al_get_display_width(debug->display),
		al_get_font_line_height(debug->settings.text_font) + 10,
		0,
		"DEBUG WINDOW\n"
		"ON: %s\n"
		"STEP BY STEP: %s\n"
		"OPCODE: %s\n"
		"PC: %04hX I: %04hX S: %04hX\n"
		"DT: %02hhX ST: %02hhX\n"
		"V0: %02hhX V1: %02hhX V2: %02hhX V3: %02hhX\n"
		"V4: %02hhX V5: %02hhX V6: %02hhX V7: %02hhX\n"
		"V8: %02hhX V9: %02hhX VA: %02hhX VB: %02hhX\n"
		"VC: %02hhX VD: %02hhX VE: %02hhX VF: %02hhX\n",
		BOOL_STR(debug->settings.options[DEBUG_ENABLED]),
		BOOL_STR(debug->settings.options[DEBUG_STEP_BY_STEP]),
		asm_text,
		machine->pc_reg, machine->i_reg, machine->s_reg,
		machine->d_counter, machine->s_counter,
		machine->v_reg[0], machine->v_reg[1], machine->v_reg[2], machine->v_reg[3],
		machine->v_reg[4], machine->v_reg[5], machine->v_reg[6], machine->v_reg[7],
		machine->v_reg[8], machine->v_reg[9], machine->v_reg[10], machine->v_reg[11],
		machine->v_reg[12], machine->v_reg[13], machine->v_reg[14], machine->v_reg[15]);
	al_flip_display();
}