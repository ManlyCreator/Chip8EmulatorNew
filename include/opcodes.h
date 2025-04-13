#ifndef OPCODES_H
#define OPCODES_H

#include "chip8.h"

extern void (*opcodes[16])(Chip8*);

void op0xxx(Chip8 *chip8);
void op1xxx(Chip8 *chip8);
void op2xxx(Chip8 *chip8);
void op3xxx(Chip8 *chip8);
void op4xxx(Chip8 *chip8);
void op5xxx(Chip8 *chip8);
void op6xxx(Chip8 *chip8);
void op7xxx(Chip8 *chip8);
void op8xxx(Chip8 *chip8);
void op9xxx(Chip8 *chip8);
void opAxxx(Chip8 *chip8);
void opBxxx(Chip8 *chip8);
void opCxxx(Chip8 *chip8);
void opDxxx(Chip8 *chip8);
void opExxx(Chip8 *chip8);
void opFxxx(Chip8 *chip8);

#endif
