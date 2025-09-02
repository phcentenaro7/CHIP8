#pragma once
#include "machine.h"
#include "debug.h"
#include "struct_machine.h"
#include <allegro5/allegro_audio.h>

#define RAM_SIZE 4096 //Available RAM
#define NUM_V_REGS 16 //Number of variable registers
#define NUM_PIXEL_ROWS 32 //The display is made of 32 rows of 64 pixels each
#define NUM_PIXEL_COLS 64
#define PROGRAM_BASE_ADDRESS 0x200 //Start address compatible with older CHIP-8 programs, where the interpreter would be located at the start of RAM
#define STACK_BASE_ADDRESS 0x200 //Base address for the stack, which grows downwards
#define MEM_STEP 2 * BYTE_SIZE //The step used when incrementing registers like the program counter and the stack pointer
#define STEP(reg) reg += MEM_STEP
#define STEP_BACK(reg) reg -= MEM_STEP
#define FONT_MEMORY_SIZE 80
#define FONT_MEMORY_BASE_ADDRESS 0x050
#define DEFAULT_FONT_MEMORY_CONTENT {0xF0, 0x90, 0x90, 0x90, 0xF0,\
									 0x20, 0x60, 0x20, 0x20, 0x70,\
									 0xF0, 0x10, 0xF0, 0x80, 0xF0,\
									 0xF0, 0x10, 0xF0, 0x10, 0xF0,\
									 0x90, 0x90, 0xF0, 0x10, 0x10,\
									 0xF0, 0x80, 0xF0, 0x10, 0xF0,\
									 0xF0, 0x80, 0xF0, 0x90, 0xF0,\
									 0xF0, 0x10, 0x20, 0x40, 0x40,\
									 0xF0, 0x90, 0xF0, 0x90, 0xF0,\
									 0xF0, 0x90, 0xF0, 0x10, 0xF0,\
									 0xF0, 0x90, 0xF0, 0x90, 0x90,\
									 0xE0, 0x90, 0xE0, 0x90, 0xE0,\
									 0xF0, 0x80, 0x80, 0x80, 0xF0,\
									 0xE0, 0x90, 0x90, 0x90, 0xE0,\
									 0xF0, 0x80, 0xF0, 0x80, 0xF0,\
									 0xF0, 0x80, 0xF0, 0x80, 0x80}
#define KEYPAD_WIDTH 4
#define KEYPAD_HEIGHT 4
#define DEFAULT_COUNTER_TIMER_PERIOD 1 / 60.0
#define DEFAULT_OPCODE_TIMER_PERIOD 1 / 700.0
#define DEFAULT_DISPLAY_WIDTH 64
#define DEFAULT_DISPLAY_HEIGHT 32
#define BYTE_SIZE sizeof(int8_t)
#define GET_TYPE(opcode) (opcode & 0xF000) >> 12
#define GET_X(opcode) (opcode & 0x0F00) >> 8
#define GET_Y(opcode) (opcode & 0x00F0) >> 4
#define GET_NNN(opcode) opcode & 0x0FFF
#define GET_NN(opcode) opcode & 0x00FF
#define GET_N(opcode) opcode & 0x000F

typedef struct MACHINE
{
	int8_t RAM[RAM_SIZE];
	uint16_t pc_reg; //program counter
	uint16_t i_reg; //index
	uint16_t s_reg; //stack
	uint8_t d_counter; //delay timer counter
	uint8_t s_counter; //sound timer counter
	uint8_t v_reg[NUM_V_REGS]; //variables
	uint64_t pixel_row[NUM_PIXEL_ROWS]; //screen
	uint16_t current_opcode;
	INPUT_KEY** keypad;
	bool key_pressed[KEYPAD_WIDTH * KEYPAD_HEIGHT];
	ALLEGRO_TIMER* counter_timer;
	ALLEGRO_TIMER* opcode_timer;
	ALLEGRO_DISPLAY* display;
	DISPLAY_OPTIONS display_options;
	ALLEGRO_BITMAP* pixel_on;
	ALLEGRO_BITMAP* pixel_off;
	ALLEGRO_SAMPLE* beep;
	ALLEGRO_SAMPLE_ID beep_id;
	bool beep_playing;
	ALLEGRO_EVENT_QUEUE* event_queue;
	DEBUG* debug;
}MACHINE;