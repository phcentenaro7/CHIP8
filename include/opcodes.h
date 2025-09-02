#pragma once
#include "struct_machine.h"

uint16_t fetch_opcode(MACHINE* machine);
void execute_opcode(MACHINE* machine, uint16_t opcode);
void opcode_to_string(char* buffer, uint16_t opcode);