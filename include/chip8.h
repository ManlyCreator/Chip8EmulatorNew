#ifndef CHIP_8_H
#define CHIP_8_H

#include <iostream>
#include <memory>
#include "screen.h"
#include "buzzer.h"

#define MEMORY 4096
#define DISPLAY_WIDTH 64
#define DISPLAY_HEIGHT 32
#define DISPLAY_FREQUENCY (float)1 / 60

#define Byte unsigned char
#define SignedByte char
#define Word unsigned short

typedef enum { DEBUG_FALSE, DEBUG_TRUE } DebugStates;

class Chip8 {
  private:
    // Memory & Registers
    Byte memory[MEMORY];
    Byte V[16];
    Byte key[16];
    Word I;
    Word opcode;

    // Display
    Byte display[DISPLAY_WIDTH * DISPLAY_HEIGHT];
    std::unique_ptr<Screen> screen;

    // Sound
    std::unique_ptr<Buzzer> buzzer;

    // State
    Word pc;
    Word stack[16];
    Byte sp;
    Byte debugFlag;
    Byte instructionFrequency;
    SignedByte keyPressed;

    // Timers
    Byte delayTimer;
    Byte soundTimer;

    // Functions
    void tick();
    void emulateCycle();
    void processInput();

    // Friends
    friend void op0xxx(Chip8*);
    friend void op1xxx(Chip8*);
    friend void op2xxx(Chip8*);
    friend void op3xxx(Chip8*);
    friend void op4xxx(Chip8*);
    friend void op5xxx(Chip8*);
    friend void op6xxx(Chip8*);
    friend void op7xxx(Chip8*);
    friend void op8xxx(Chip8*);
    friend void op9xxx(Chip8*);
    friend void opAxxx(Chip8*);
    friend void opBxxx(Chip8*);
    friend void opCxxx(Chip8*);
    friend void opDxxx(Chip8*);
    friend void opExxx(Chip8*);
    friend void opFxxx(Chip8*);

  public:
    Chip8(Byte instructionFrequency, Byte debugFlag);
    int loadROM(const char *romPath);
    void startMainLoop();
};

void chipStartMainLoop(Chip8 *chip8, unsigned instructionFrequency);
void chipTick(Chip8 *chip8, int steps);
void chipProcessInput(Chip8 *chip8);

#endif
