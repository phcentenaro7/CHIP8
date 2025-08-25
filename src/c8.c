#include <stdio.h>
#include <allegro5/allegro.h>
#include "machine.h"

int main()
{
	start_allegro();
	DISPLAY_OPTIONS display_options =
	{
		.scale = 8,
		.color_on = al_map_rgb(255, 255, 255),
		.color_off = al_map_rgb(0, 0, 0)
	};
	MACHINE* m = create_machine(display_options);
	load_program(m, "games/Pong (1 player).ch8");
	run_program(m);
	delete_machine(m);
	end_allegro();
	return 0;
}