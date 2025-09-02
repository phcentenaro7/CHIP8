#include <stdio.h>
#include <allegro5/allegro_native_dialog.h>
#include "struct_machine.h"
#include "struct_debug.h"

static void op_push_to_stack(MACHINE* machine);
static void op_jump(MACHINE* machine, uint16_t opcode);
static void op_do_if_not_equal_to_constant(MACHINE* machine, uint16_t opcode);
static void op_do_if_equal_to_constant(MACHINE* machine, uint16_t opcode);
static void op_do_if_not_equal_to_variable(MACHINE* machine, uint16_t opcode);
static void op_do_if_equal_to_variable(MACHINE* machine, uint16_t opcode);
static void op_assign_constant(MACHINE* machine, uint16_t opcode);
static void op_add_constant(MACHINE* machine, uint16_t opcode);
static void op_assign_to_i(MACHINE* machine, uint16_t opcode);
static void op_assign_to_pc(MACHINE* machine, uint16_t opcode);
static void op_assign_random(MACHINE* machine, uint16_t opcode);
static void op_do_if_key_not_pressed(MACHINE* machine, uint16_t opcode);
static void op_do_if_key_pressed(MACHINE* machine, uint16_t opcode);
static void op_handle_key_presses(MACHINE* machine, uint16_t opcode);
static void op_assign_from_d_counter(MACHINE* machine, uint16_t opcode);
static void op_wait_for_key_press(MACHINE* machine, uint16_t opcode);
static void op_assign_to_d_counter(MACHINE* machine, uint16_t opcode);
static void op_assign_to_s_counter(MACHINE* machine, uint16_t opcode);
static void op_add_to_i(MACHINE* machine, uint16_t opcode);
static void op_assign_char_address_to_i(MACHINE* machine, uint16_t opcode);
static void op_store_bcd(MACHINE* machine, uint16_t opcode);
static void op_store_registers(MACHINE* machine, uint16_t opcode);
static void op_load_registers(MACHINE* machine, uint16_t opcode);
static void op_handle_special_registers(MACHINE* machine, uint16_t opcode);
static void op_assign(MACHINE* machine, uint16_t opcode);
static void op_or(MACHINE* machine, uint16_t opcode);
static void op_and(MACHINE* machine, uint16_t opcode);
static void op_xor(MACHINE* machine, uint16_t opcode);
static void op_add(MACHINE* machine, uint16_t opcode);
static void op_sub(MACHINE* machine, uint16_t opcode);
static void op_shift_right(MACHINE* machine, uint16_t opcode);
static void op_distance(MACHINE* machine, uint16_t opcode);
static void op_shift_left(MACHINE* machine, uint16_t opcode);
static void op_handle_variable_arithmetic(MACHINE* machine, uint16_t opcode);
static void op_return_from_subroutine(MACHINE* machine, uint16_t opcode);
static void op_clear_screen(MACHINE* machine, uint16_t opcode);
static void op_handle_base_instructions(MACHINE* machine, uint16_t opcode);
static void op_draw_sprite(MACHINE* machine, uint16_t opcode);
uint16_t fetch_opcode(MACHINE* machine);
void execute_opcode(MACHINE* machine, uint16_t opcode);
void opcode_to_string(char* buffer, uint16_t opcode);

static void op_push_to_stack(MACHINE* machine)
{
	STEP_BACK(machine->s_reg);
	*(int16_t*)(machine->RAM + machine->s_reg) = machine->pc_reg;
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
	if (!machine->waiting_for_input)
	{
		machine->waiting_for_input = true;
		STEP_BACK(machine->pc_reg);
		return;
	}
	else if(machine->input_received)
	{
		for (uint8_t i = 0; i < KEYPAD_HEIGHT; i++)
		{
			for (uint8_t j = 0; j < KEYPAD_WIDTH; j++)
			{
				if (machine->key_pressed[machine->keypad[i][j].value])
				{
					*vx = machine->keypad[i][j].value;
					machine->input_received = false;
					machine->waiting_for_input = false;
					STEP(machine->pc_reg);
					return;
				}
			}
		}
	}
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
	machine->i_reg += vx;
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

static void op_store_registers(MACHINE* machine, uint16_t opcode)
{
	uint8_t x = GET_X(opcode);
	for (uint8_t i = 0; i <= x; i++)
	{
		machine->RAM[machine->i_reg + i] = machine->v_reg[i];
	}
}

static void op_load_registers(MACHINE* machine, uint16_t opcode)
{
	uint8_t x = GET_X(opcode);
	for (uint8_t i = 0; i <= x; i++)
	{
		machine->v_reg[i] = machine->RAM[machine->i_reg + i];
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
	uint8_t* vf = &(machine->v_reg[0xF]);
	*vx |= *vy;
	//*vf = 0;
}

static void op_and(MACHINE* machine, uint16_t opcode)
{
	uint8_t* vx = &(machine->v_reg[GET_X(opcode)]);
	uint8_t* vy = &(machine->v_reg[GET_Y(opcode)]);
	uint8_t* vf = &(machine->v_reg[0xF]);
	*vx &= *vy;
	//*vf = 0;
}

static void op_xor(MACHINE* machine, uint16_t opcode)
{
	uint8_t* vx = &(machine->v_reg[GET_X(opcode)]);
	uint8_t* vy = &(machine->v_reg[GET_Y(opcode)]);
	uint8_t* vf = &(machine->v_reg[0xF]);
	*vx ^= *vy;
	//*vf = 0;
}

static void op_add(MACHINE* machine, uint16_t opcode)
{
	uint8_t* vx = &(machine->v_reg[GET_X(opcode)]);
	uint8_t* vy = &(machine->v_reg[GET_Y(opcode)]);
	uint8_t* vf = &(machine->v_reg[0xF]);
	bool overflow = 0xFF < *vx + *vy;
	*vx += *vy;
	*vf = overflow;
}

static void op_sub(MACHINE* machine, uint16_t opcode)
{
	uint8_t* vx = &(machine->v_reg[GET_X(opcode)]);
	uint8_t* vy = &(machine->v_reg[GET_Y(opcode)]);
	uint8_t* vf = &(machine->v_reg[0xF]);
	bool underflow = *vy > *vx;
	*vx -= *vy;
	*vf = !underflow;
}

static void op_shift_right(MACHINE* machine, uint16_t opcode)
{
	uint8_t* vx = &(machine->v_reg[GET_X(opcode)]);
	uint8_t* vy = &(machine->v_reg[GET_Y(opcode)]);
	uint8_t* vf = &(machine->v_reg[0xF]);
	uint8_t carry = *vx & 1;
	*vx >>= 1;
	*vf = carry;
}

static void op_distance(MACHINE* machine, uint16_t opcode)
{
	uint8_t* vx = &(machine->v_reg[GET_X(opcode)]);
	uint8_t* vy = &(machine->v_reg[GET_Y(opcode)]);
	uint8_t* vf = &(machine->v_reg[0xF]);
	bool underflow = *vx > *vy;
	*vx = *vy - *vx;
	*vf = !underflow;
}

static void op_shift_left(MACHINE* machine, uint16_t opcode)
{
	uint8_t* vx = &(machine->v_reg[GET_X(opcode)]);
	uint8_t* vy = &(machine->v_reg[GET_Y(opcode)]);
	uint8_t* vf = &(machine->v_reg[0xF]);
	uint8_t carry = (*vx & 0x80) >> 7;
	*vx <<= 1;
	*vf = carry;
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
	machine->pc_reg = *(uint16_t*)(machine->RAM + machine->s_reg);
	STEP(machine->s_reg);
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
	uint8_t* vf = &(machine->v_reg[0xF]);
	uint8_t n = GET_N(opcode);
	uint8_t row_count = 0;
	*vf = 0;
	while (row_count < n)
	{
		uint8_t sprite_row = machine->RAM[machine->i_reg + row_count];
		uint64_t row_data = (uint64_t)sprite_row << 56;
		row_data >>= x;
		if (machine->y_wrap_enabled && x > 56)
		{
			uint8_t x_wrap_sprite = sprite_row << (64 - x);
			row_data |= (uint64_t)x_wrap_sprite << 56;
		}
		if (y >= NUM_PIXEL_ROWS)
		{
			if (machine->y_wrap_enabled)
			{
				y = 0;
			}
			else
			{
				break;
			}
		}
		if (*vf == 0)
		{
			*vf = (machine->pixel_row[y] & row_data) > 0;
		}
		machine->pixel_row[y] ^= row_data;
		y++;
		row_count++;
	}
}

uint16_t fetch_opcode(MACHINE* machine)
{
	uint16_t opcode = *(uint16_t*)(machine->RAM + machine->pc_reg);
	opcode = ((opcode >> 8) & 0x00FF) | (opcode << 8);
	machine->current_opcode = opcode;
	if (machine->debug->on && machine->debug->settings.options[DEBUG_STEP_BY_STEP])
	{
		if (!machine->debug->settings.options[DEBUG_NEXT_STEP])
		{
			return;
		}
		machine->debug->settings.options[DEBUG_NEXT_STEP] = false;
	}
	if (!machine->waiting_for_input)
	{
		STEP(machine->pc_reg);
	}
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

void opcode_to_string(char* buffer, uint16_t opcode)
{
	const uint8_t buffer_length = 32;
	uint8_t type = GET_TYPE(opcode);
	uint8_t x = GET_X(opcode);
	uint8_t y = GET_Y(opcode);
	uint16_t nnn = GET_NNN(opcode);
	uint8_t nn = GET_NN(opcode);
	uint8_t n = GET_N(opcode);
	switch (type)
	{
	case 0x0:
		if (nn == 0xE0)
		{
			snprintf(buffer, buffer_length, "CLR");
			return;
		}
		else if (nn == 0xEE)
		{
			snprintf(buffer, buffer_length, "RET");
			return;
		}
		break;
	case 0x1:
		snprintf(buffer, buffer_length, "JMP %03hX", nnn);
		return;
	case 0x2:
		snprintf(buffer, buffer_length, "CALL %03hX", nnn);
		return;
	case 0x3:
		snprintf(buffer, buffer_length, "NEQ V%hhX, %02hhX", x, nn);
		return;
	case 0x4:
		snprintf(buffer, buffer_length, "EQ V%hhX, %02hhX", x, nn);
		return;
	case 0x5:
		snprintf(buffer, buffer_length, "NEQ V%hhX, V%hhX", x, y);
		return;
	case 0x6:
		snprintf(buffer, buffer_length, "MOV V%hhX, %02hhX", x, nn);
		return;
	case 0x7:
		snprintf(buffer, buffer_length, "ADD V%hhX, %02hhX", x, nn);
		return;
	case 0x8:
		switch (n)
		{
		case 0x0:
			snprintf(buffer, buffer_length, "MOV V%hhX, V%hhX", x, y);
			return;
		case 0x1:
			snprintf(buffer, buffer_length, "OR V%hhX, V%hhX", x, y);
			return;
		case 0x2:
			snprintf(buffer, buffer_length, "AND V%hhX, V%hhX", x, y);
			return;
		case 0x3:
			snprintf(buffer, buffer_length, "XOR V%hhX, V%hhX", x, y);
			return;
		case 0x4:
			snprintf(buffer, buffer_length, "ADD V%hhX, V%hhX", x, y);
			return;
		case 0x5:
			snprintf(buffer, buffer_length, "SUB V%hhX, V%hhX", x, y);
			return;
		case 0x6:
			snprintf(buffer, buffer_length, "SHR V%hhX, V%hhX", x, y);
			return;
		case 0x7:
			snprintf(buffer, buffer_length, "DIFF V%hhX, V%hhX", x, y);
			return;
		case 0xE:
			snprintf(buffer, buffer_length, "SHL V%hhX, V%hhX", x, y);
			return;
		}
	case 0x9:
		snprintf(buffer, buffer_length, "EQ V%hhX, V%hhX", x, y);
		return;
	case 0xA:
		snprintf(buffer, buffer_length, "MOV I, %03hX", nnn);
		return;
	case 0xB:
		snprintf(buffer, buffer_length, "JMP0 %03hX", nnn);
		return;
	case 0xC:
		snprintf(buffer, buffer_length, "RND V%hhX %02hhX", x, nn);
		return;
	case 0xD:
		snprintf(buffer, buffer_length, "DRAW V%hhX, V%hhX, %hhX", x, y, n);
		return;
	case 0xE:
		if (nn == 0x9E)
		{
			snprintf(buffer, buffer_length, "NKEY V%hhX", x);
			return;
		}
		else if (nn == 0xA1)
		{
			snprintf(buffer, buffer_length, "KEY V%hhX", x);
			return;
		}
		break;
	case 0xF:
		switch (nn)
		{
		case 0x07:
			snprintf(buffer, buffer_length, "MOV V%hhX, DT", x);
			return;
		case 0x0A:
			snprintf(buffer, buffer_length, "WAIT V%hhX", x);
			return;
		case 0x15:
			snprintf(buffer, buffer_length, "MOV DT, V%hhX", x);
			return;
		case 0x18:
			snprintf(buffer, buffer_length, "MOV ST, V%hhX", x);
			return;
		case 0x1E:
			snprintf(buffer, buffer_length, "ADD I, V%hhX", x);
			return;
		case 0x29:
			snprintf(buffer, buffer_length, "MOV I, CHAR[V%hhX]", x);
			return;
		case 0x33:
			snprintf(buffer, buffer_length, "BCD V%hhX", x);
			return;
		case 0x55:
			snprintf(buffer, buffer_length, "STO %hhX", x);
			return;
		case 0x65:
			snprintf(buffer, buffer_length, "LD %hhX", x);
			return;
		}
	}
	snprintf(buffer, buffer_length, "??? (%04hX)", opcode);
}