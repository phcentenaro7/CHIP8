#include "struct_machine.h"

static void op_push_to_stack(MACHINE* machine)
{
	STEP_BACK(machine->s_reg);
	*(int16_t*)(machine->RAM + STACK_BASE_ADDRESS - machine->s_reg) = machine->pc_reg;
}

static void op_jump(MACHINE* machine, uint16_t opcode)
{
	uint16_t nnn = GET_NNN(opcode);
	machine->pc_reg = nnn;
}

static void op_do_if_not_equal_to_constant(MACHINE* machine, uint16_t opcode)
{
	uint8_t vx = machine->v_reg[GET_X(opcode)];
	uint8_t nn = GET_NN(opcode);
	if (vx == nn)
		STEP(machine->pc_reg);
}

static void op_do_if_equal_to_constant(MACHINE* machine, uint16_t opcode)
{
	uint8_t vx = machine->v_reg[GET_X(opcode)];
	uint8_t nn = GET_NN(opcode);
	if (vx != nn)
		STEP(machine->pc_reg);
}

static void op_do_if_not_equal_to_variable(MACHINE* machine, uint16_t opcode)
{
	uint8_t vx = machine->v_reg[GET_X(opcode)];
	uint8_t vy = machine->v_reg[GET_Y(opcode)];
	if (vx == vy)
		STEP(machine->pc_reg);
}

static void op_do_if_equal_to_variable(MACHINE* machine, uint16_t opcode)
{
	uint8_t vx = machine->v_reg[GET_X(opcode)];
	uint8_t vy = machine->v_reg[GET_Y(opcode)];
	if (vx != vy)
		STEP(machine->pc_reg);
}

static void op_assign_constant(MACHINE* machine, uint16_t opcode)
{
	uint8_t* vx = &(machine->v_reg[GET_X(opcode)]);
	uint8_t nn = GET_NN(opcode);
	*vx = nn;
}

static void op_add_constant(MACHINE* machine, uint16_t opcode)
{
	uint8_t* vx = &(machine->v_reg[GET_X(opcode)]);
	uint8_t nn = GET_NN(opcode);
	*vx += nn;
}

static void op_assign_to_i(MACHINE* machine, uint16_t opcode)
{
	uint16_t nnn = GET_NNN(opcode);
	machine->i_reg = nnn;
}

static void op_assign_to_pc(MACHINE* machine, uint16_t opcode)
{
	uint16_t nnn = GET_NNN(opcode);
	machine->pc_reg = machine->v_reg[0] + nnn;
}

static void op_assign_random(MACHINE* machine, uint16_t opcode)
{
	uint8_t* vx = &(machine->v_reg[GET_X(opcode)]);
	uint8_t nn = GET_NN(opcode);
	*vx = rand() % 256 & nn;
}

static void op_do_if_key_not_pressed(MACHINE* machine, uint16_t opcode)
{
	uint8_t vx = machine->v_reg[GET_X(opcode)];
	if (machine->key_pressed[vx])
	{
		STEP(machine->pc_reg);
	}
}

static void op_do_if_key_pressed(MACHINE* machine, uint16_t opcode)
{
	uint8_t vx = machine->v_reg[GET_X(opcode)];
	if (!machine->key_pressed[vx])
	{
		STEP(machine->pc_reg);
	}
}

static void op_handle_key_presses(MACHINE* machine, uint16_t opcode)
{
	uint8_t nn = GET_NN(opcode);
	if (nn == 0x9E)
	{
		op_do_if_key_not_pressed(machine, opcode);
	}
	else if (nn == 0xA1)
	{
		op_do_if_key_pressed(machine, opcode);
	}
}

static void op_assign_from_d_counter(MACHINE* machine, uint16_t opcode)
{
	uint8_t* vx = &(machine->v_reg[GET_X(opcode)]);
	*vx = machine->d_counter;
}

static void op_wait_for_key_press(MACHINE* machine, uint16_t opcode)
{
	uint8_t* vx = &(machine->v_reg[GET_X(opcode)]);
	ALLEGRO_EVENT event;
	ALLEGRO_EVENT_QUEUE* event_queue = al_create_event_queue();
	al_register_event_source(event_queue, al_get_keyboard_event_source());
	bool done = false;
	while (!done)
	{
		al_wait_for_event(event_queue, &event);
		if (event.type != ALLEGRO_EVENT_KEY_DOWN)
		{
			continue;
		}
		for (uint8_t i = 0; i < KEYPAD_HEIGHT; i++)
		{
			for (uint8_t j = 0; j < KEYPAD_WIDTH; j++)
			{
				if (machine->keypad[i][j].keycode == event.keyboard.keycode)
				{
					*vx = event.keyboard.keycode;
					done = true;
				}
			}
		}
	}
	al_unregister_event_source(event_queue, al_get_keyboard_event_source());
	al_destroy_event_queue(event_queue);
}

static void op_assign_to_d_counter(MACHINE* machine, uint16_t opcode)
{
	uint8_t vx = machine->v_reg[GET_X(opcode)];
	machine->d_counter = vx;
}

static void op_assign_to_s_counter(MACHINE* machine, uint16_t opcode)
{
	uint8_t vx = machine->v_reg[GET_X(opcode)];
	machine->s_counter = vx;
}

static void op_add_to_i(MACHINE* machine, uint16_t opcode)
{
	uint8_t vx = machine->v_reg[GET_X(opcode)];
	machine->i_reg = vx;
}

static void op_assign_char_address_to_i(MACHINE* machine, uint16_t opcode)
{
	uint8_t vx = machine->v_reg[GET_X(opcode)];
	machine->i_reg = FONT_MEMORY_BASE_ADDRESS + (vx * BYTE_SIZE * 5);
}

static void op_store_bcd(MACHINE* machine, uint16_t opcode)
{
	uint8_t vx = machine->v_reg[GET_X(opcode)];
	machine->RAM[machine->i_reg] = vx / 100;
	machine->RAM[machine->i_reg + 1] = (vx / 10) % 10;
	machine->RAM[machine->i_reg + 2] = vx % 10;
}

static op_store_registers(MACHINE* machine, uint16_t opcode)
{
	uint8_t x = GET_X(opcode);
	for (uint8_t i = 0; i <= x; i++)
	{
		machine->RAM[machine->i_reg + x] = machine->v_reg[i];
	}
}

static void op_load_registers(MACHINE* machine, uint16_t opcode)
{
	uint8_t x = GET_X(opcode);
	for (uint8_t i = 0; i <= x; i++)
	{
		machine->v_reg[i] = machine->RAM[machine->i_reg + x];
	}
}

static void op_handle_special_registers(MACHINE* machine, uint16_t opcode)
{
	uint8_t nn = GET_NN(opcode);
	switch (nn)
	{
	case 0x7:
		op_assign_from_d_counter(machine, opcode);
		return;
	case 0xA:
		op_wait_for_key_press(machine, opcode);
		return;
	case 0x15:
		op_assign_to_d_counter(machine, opcode);
		return;
	case 0x18:
		op_assign_to_s_counter(machine, opcode);
		return;
	case 0x1E:
		op_add_to_i(machine, opcode);
		return;
	case 0x29:
		op_assign_char_address_to_i(machine, opcode);
		return;
	case 0x33:
		op_store_bcd(machine, opcode);
		return;
	case 0x55:
		op_store_registers(machine, opcode);
		return;
	case 0x65:
		op_load_registers(machine, opcode);
		return;
	}
}

static void op_assign(MACHINE* machine, uint16_t opcode)
{
	uint8_t* vx = &(machine->v_reg[GET_X(opcode)]);
	uint8_t* vy = &(machine->v_reg[GET_Y(opcode)]);
	*vx = *vy;
}

static void op_or(MACHINE* machine, uint16_t opcode)
{
	uint8_t* vx = &(machine->v_reg[GET_X(opcode)]);
	uint8_t* vy = &(machine->v_reg[GET_Y(opcode)]);
	*vx |= *vy;
}

static void op_and(MACHINE* machine, uint16_t opcode)
{
	uint8_t* vx = &(machine->v_reg[GET_X(opcode)]);
	uint8_t* vy = &(machine->v_reg[GET_Y(opcode)]);
	*vx &= *vy;
}

static void op_xor(MACHINE* machine, uint16_t opcode)
{
	uint8_t* vx = &(machine->v_reg[GET_X(opcode)]);
	uint8_t* vy = &(machine->v_reg[GET_Y(opcode)]);
	*vx ^= *vy;
}

static void op_add(MACHINE* machine, uint16_t opcode)
{
	uint8_t* vx = &(machine->v_reg[GET_X(opcode)]);
	uint8_t* vy = &(machine->v_reg[GET_Y(opcode)]);
	uint8_t* vf = &(machine->v_reg[0xF]);
	if (0xFF - *vx < *vy)
		*vf = 1;
	*vx += *vy;
}

static void op_sub(MACHINE* machine, uint16_t opcode)
{
	uint8_t* vx = &(machine->v_reg[GET_X(opcode)]);
	uint8_t* vy = &(machine->v_reg[GET_Y(opcode)]);
	uint8_t* vf = &(machine->v_reg[0xF]);
	if (*vy > *vx)
		*vf = 0;
	*vx -= *vy;
}

static void op_shift_right(MACHINE* machine, uint16_t opcode)
{
	uint8_t* vx = &(machine->v_reg[GET_X(opcode)]);
	uint8_t* vy = &(machine->v_reg[GET_Y(opcode)]);
	uint8_t* vf = &(machine->v_reg[0xF]);
	*vf = *vx & 1;
	*vx >>= *vy;
}

static void op_distance(MACHINE* machine, uint16_t opcode)
{
	uint8_t* vx = &(machine->v_reg[GET_X(opcode)]);
	uint8_t* vy = &(machine->v_reg[GET_Y(opcode)]);
	uint8_t* vf = &(machine->v_reg[0xF]);
	if (*vx > *vy)
		*vf = 0;
	*vx = *vy - *vx;
}

static void op_shift_left(MACHINE* machine, uint16_t opcode)
{
	uint8_t* vx = &(machine->v_reg[GET_X(opcode)]);
	uint8_t* vy = &(machine->v_reg[GET_Y(opcode)]);
	uint8_t* vf = &(machine->v_reg[0xF]);
	*vf = *vx & 0x80;
	*vx <<= *vy;
}

static void op_handle_variable_arithmetic(MACHINE* machine, uint16_t opcode)
{
	uint8_t n = GET_N(opcode);
	switch (n)
	{
	case 0x0:
		op_assign(machine, opcode);
		return;
	case 0x1:
		op_or(machine, opcode);
		return;
	case 0x2:
		op_and(machine, opcode);
		return;
	case 0x3:
		op_xor(machine, opcode);
		return;
	case 0x4:
		op_add(machine, opcode);
		return;
	case 0x5:
		op_sub(machine, opcode);
		return;
	case 0x6:
		op_shift_right(machine, opcode);
		return;
	case 0x7:
		op_distance(machine, opcode);
		return;
	case 0xE:
		op_shift_left(machine, opcode);
		return;
	}
}

static void op_return_from_subroutine(MACHINE* machine, uint16_t opcode)
{
	machine->pc_reg = machine->RAM[machine->s_reg];
	machine->s_reg--;
}

static void op_clear_screen(MACHINE* machine, uint16_t opcode)
{
	for (uint8_t i = 0; i < NUM_PIXEL_ROWS; i++)
	{
		machine->pixel_row[i] = 0;
	}
}

static void op_handle_base_instructions(MACHINE* machine, uint16_t opcode)
{
	uint8_t nn = GET_NN(opcode);
	if (nn == 0xE0)
	{
		op_clear_screen(machine, opcode);
	}
	else if (nn == 0xEE)
	{
		op_return_from_subroutine(machine, opcode);
	}
}

static void op_draw_sprite(MACHINE* machine, uint16_t opcode)
{
	uint8_t x = machine->v_reg[GET_X(opcode)] % NUM_PIXEL_COLS;
	uint8_t y = machine->v_reg[GET_Y(opcode)] % NUM_PIXEL_ROWS;
	uint8_t n = GET_N(opcode);
	uint8_t row_count = 0;
	while (n > 0 && y < NUM_PIXEL_ROWS)
	{
		uint64_t row_data = (uint64_t)0xFF & machine->RAM[machine->i_reg + row_count];
		row_data = (row_data << 56) >> x;
		machine->pixel_row[y] ^= row_data;
		y++;
		n--;
		row_count++;
	}
}

uint16_t fetch_opcode(MACHINE* machine)
{
	uint16_t opcode = *(uint16_t*)(machine->RAM + machine->pc_reg);
	opcode = ((opcode >> 8) & 0x00FF) | (opcode << 8);
	STEP(machine->pc_reg);
	return opcode;
}

void execute_opcode(MACHINE* machine, uint16_t opcode)
{
	int opcode_type = GET_TYPE(opcode);
	switch (opcode_type)
	{
	case 0x0:
		op_handle_base_instructions(machine, opcode);
		return;
	case 0x2:
		op_push_to_stack(machine);
		return;
	case 0x1:
		op_jump(machine, opcode);
		return;
	case 0x3:
		op_do_if_not_equal_to_constant(machine, opcode);
		return;
	case 0x4:
		op_do_if_equal_to_constant(machine, opcode);
		return;
	case 0x5:
		op_do_if_not_equal_to_variable(machine, opcode);
		return;
	case 0x6:
		op_assign_constant(machine, opcode);
		return;
	case 0x7:
		op_add_constant(machine, opcode);
		return;
	case 0x8:
		op_handle_variable_arithmetic(machine, opcode);
		return;
	case 0x9:
		op_do_if_equal_to_variable(machine, opcode);
		return;
	case 0xA:
		op_assign_to_i(machine, opcode);
		return;
	case 0xB:
		op_assign_to_pc(machine, opcode);
		return;
	case 0xC:
		op_assign_random(machine, opcode);
		return;
	case 0xD:
		op_draw_sprite(machine, opcode);
		return;
	case 0xE:
		op_handle_key_presses(machine, opcode);
		return;
	case 0xF:
		op_handle_special_registers(machine, opcode);
		return;
	}
}