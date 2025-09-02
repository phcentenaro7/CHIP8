#pragma once
#include "debug.h"
#include "opcodes.h"

typedef struct DEBUG
{
	bool on;
	DEBUG_SETTINGS settings;
	ALLEGRO_DISPLAY* display;
	ALLEGRO_TIMER* refresh_timer;
	ALLEGRO_EVENT_QUEUE* event_queue;
	ALLEGRO_THREAD* thread;
	ALLEGRO_MUTEX* event_mutex;
	MACHINE* machine;
}DEBUG;