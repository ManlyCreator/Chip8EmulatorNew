#include "opcodes.h"
#include <time.h>

void (*opcodes[16])(Chip8*) = {
  op0xxx,
  op1xxx,
  op2xxx,
  op3xxx,
  op4xxx,
  op5xxx,
  op6xxx,
  op7xxx,
  op8xxx,
  op9xxx,
  opAxxx,
  opBxxx,
  opCxxx,
  opDxxx,
  opExxx,
  opFxxx,
};

void op0xxx(Chip8 *chip8) {
  switch (chip8->opcode) {
    // 0x00E0 - Clear Screen
    case 0x00E0:
      if (chip8->debugFlag) printf("Clearing Display\n");
      std::fill(chip8->display, chip8->display + (DISPLAY_WIDTH * DISPLAY_HEIGHT), 0);
      chip8->pc += 2;
      break;
    // 0x00EE - Return
    case 0x00EE:
      chip8->pc = chip8->stack[--chip8->sp] + 2;
      if (chip8->debugFlag) printf("Returning to 0x%.3x\n", chip8->pc);
      break;
  }
}

// 0x1nnn - Jump to address nnn
void op1xxx(Chip8 *chip8) {
  chip8->pc = chip8->opcode & 0x0FFF;
  if (chip8->debugFlag) printf("Setting PC to: %d\n", chip8->pc);
}

// 0x2nnn - Call function at nnn
void op2xxx(Chip8 *chip8) {
  chip8->stack[chip8->sp++] = chip8->pc;
  chip8->pc = chip8->opcode & 0x0FFF;
  if (chip8->debugFlag) printf("Calling function at 0x%.3x\n", chip8->pc);
}

// 0x3xbb - Skip next instruction if V[x] == bb
void op3xxx(Chip8 *chip8) {
  Byte x = (chip8->opcode & 0x0F00) >> 8;
  if (chip8->V[x] == (chip8->opcode & 0x00FF))
    chip8->pc += 2;
  chip8->pc += 2;
}

// 0x4xbb - Skip next instruction if V[x] != bb
void op4xxx(Chip8 *chip8) {
  Byte x = (chip8->opcode & 0x0F00) >> 8;
  if (chip8->V[x] != (chip8->opcode & 0x00FF))
    chip8->pc += 2;
  chip8->pc += 2;
}

// 0x5xy0 - Skip next instruction if V[x] == V[y]
void op5xxx(Chip8 *chip8) {
  Byte x = (chip8->opcode & 0x0F00) >> 8;
  Byte y = (chip8->opcode & 0x00F0) >> 4;
  if (chip8->V[x] == chip8->V[y])
    chip8->pc += 2;
  chip8->pc += 2;
}

// 0x6xbb - Load bb into V[x]
void op6xxx(Chip8 *chip8) {
  Byte x = (chip8->opcode & 0x0F00) >> 8;
  chip8->V[x] = chip8->opcode & 0x00FF;
  if (chip8->debugFlag) printf("Loaded %d into V[0x%x]\n", chip8->V[x], x);
  chip8->pc += 2;
}

// 0x7xbb - Increment V[x] by bb
void op7xxx(Chip8 *chip8) {
  Byte x = (chip8->opcode & 0x0F00) >> 8;
  if (chip8->debugFlag) printf("Incrementing V[%d] by %d\n", x, chip8->opcode & 0x00FF);
  chip8->V[x] += chip8->opcode & 0x00FF;
  chip8->pc += 2;
}

void op8xxx(Chip8 *chip8) {
  Byte x = (chip8->opcode & 0x0F00) >> 8;
  Byte y = (chip8->opcode & 0x00F0) >> 4;
  switch (chip8->opcode & 0x000F) {
    // 0x8xy0 - Load V[y] into V[x]
    case 0x0000:
      chip8->V[x] = chip8->V[y];
      chip8->pc += 2;
      break;
    // 0x8xy1 - Set V[x] = V[x] OR V[y]
    case 0x0001:
      chip8->V[x] |= chip8->V[y];
      chip8->pc += 2;
      break;
    // 0x8xy2 - Set V[x] = V[x] AND V[y]
    case 0x0002:
      chip8->V[x] &= chip8->V[y];
      chip8->pc += 2;
      break;
    // 0x8xy3 - Set V[x] = V[x] XOR V[y]
    case 0x0003:
      chip8->V[x] ^= chip8->V[y];
      chip8->pc += 2;
      break;
    // 0x8xy4 - Increment V[x] by V[y]
    case 0x0004:
      if (chip8->V[x] + chip8->V[y] > 0xFF)
        chip8->V[0xF] = 1;
      else
        chip8->V[0xF] = 0;
      chip8->V[x] += chip8->V[y] & 0xFF;
      chip8->pc += 2;
      break;
    // 0x8xy5 - Decrement V[x] by V[y]
    case 0x0005:
      if (chip8->V[x] > chip8->V[y]) 
        chip8->V[0xF] = 1;
      else
        chip8->V[0xF] = 0;
      chip8->V[x] -= chip8->V[y];
      chip8->pc += 2;
      break;
    // 0x8xy6 - Shift right V[x] by 1 bit
    case 0x0006:
      if ((chip8->V[x] & 0x01) == 0x01)
        chip8->V[0xF] = 1;
      else
        chip8->V[0xF] = 0;
      chip8->V[x] /= 2;
      chip8->pc += 2;
      break;
    // 0x8xy7 - Set V[x] = V[y] - V[x]
    case 0x0007:
      if (chip8->V[y] > chip8->V[x])
        chip8->V[0xF] = 1;
      else
        chip8->V[0xF] = 0;
      chip8->V[x] = chip8->V[y] - chip8->V[x];
      chip8->pc += 2;
      break;
    // 0x8xyE - Shift left V[x] by 1 bit
    case 0x000E:
      if ((chip8->V[x] & 0x80) == 0x80) 
        chip8->V[0xF] = 1;
      else
        chip8->V[0xF] = 0;
      chip8->V[x] *= 2;
      chip8->pc += 2;
      break;
  }
}

// 0x9xy0 - Skip next instruction if V[x] != V[y]
void op9xxx(Chip8 *chip8) {
  Byte x = (chip8->opcode & 0x0F00) >> 8;
  Byte y = (chip8->opcode & 0x00F0) >> 4;
  if (chip8->V[x] != chip8->V[y])
    chip8->pc += 2;
  chip8->pc += 2;
}

// 0xAnnn - Load nnn into I
void opAxxx(Chip8 *chip8) {
  chip8->I = chip8->opcode & 0x0FFF;
  if (chip8->debugFlag) {
    printf("Loaded %d into I\n", chip8->I);
    printf("memory[I] = 0x%.2x\n", chip8->memory[chip8->I]);
  }
  chip8->pc += 2;
}

// 0xBnnn - Jump to address nnn + V[0]
void opBxxx(Chip8 *chip8) {
  chip8->pc = chip8->V[0] + chip8->opcode & 0x0FFF;
  if (chip8->debugFlag) printf("Jumped to address 0x%.3x\n", chip8->pc);
}

// 0xCxbb - Set V[x] = rand(0, 255) AND bb
void opCxxx(Chip8 *chip8) {
  Byte x = (chip8->opcode & 0x0F00) >> 8;
  srand(time(NULL));
  chip8->V[x] = (rand() % 256) & (chip8->opcode & 0x00FF);
  if (chip8->debugFlag) printf("Generated random number in V[0x%x]: 0x%.2x\n", x, chip8->V[x]);
  chip8->pc += 2;
}

// 0xDxyn - Draw a sprite of n-bytes high at (V[x], V[y])
void opDxxx(Chip8 *chip8) {
  Byte spriteRow;
  Byte x = (chip8->opcode & 0x0F00) >> 8;
  Byte y = (chip8->opcode & 0x00F0) >> 4;
  Byte height = chip8->opcode & 0x000F;
  chip8->V[0xF] = 0;
  for (int i = 0; i < height; i++) {
    spriteRow = chip8->memory[chip8->I + i];
    for (int j = 0; j < 8; j++) {
      unsigned index = chip8->V[x] + j + ((DISPLAY_HEIGHT - 1 - (chip8->V[y] + i)) * DISPLAY_WIDTH);
      Byte pixel = (spriteRow & (0x80 >> j)) > 0 ? 1 : 0;
      if (chip8->display[index] == 1)
        chip8->V[0xF] = 1;
      chip8->display[index] ^= pixel;
    }
  }
  chip8->pc += 2;
}

void opExxx(Chip8 *chip8) {
  Byte x = (chip8->opcode & 0x0F00) >> 8;
  Byte y = (chip8->opcode & 0x00F0) >> 4;
  switch (chip8->opcode & 0x00FF) {
    // 0xEx9E - Skip next instruction if the key value of V[x] is pressed
    case 0x009E:
      if (chip8->key[chip8->V[x]])
        chip8->pc += 2;
      chip8->pc += 2;
      break;
    // 0xExA1 - Skip next instruction if the key value of V[x] is NOT pressed
    case 0x00A1:
      if (!chip8->key[chip8->V[x]])
        chip8->pc += 2;
      chip8->pc += 2;
      break;
  }
}

void opFxxx(Chip8 *chip8) {
  Byte x = (chip8->opcode & 0x0F00) >> 8;
  Byte y = (chip8->opcode & 0x00F0) >> 4;
  switch (chip8->opcode & 0x00FF) {
    // 0xFx07 - Set V[x] = delayTimer
    case 0x0007:
      chip8->V[x] = chip8->delayTimer;
      chip8->pc += 2;
      break;
    // 0xFx0A - Wait for input and store the key value in V[x]
    case 0x000A:
      if (chip8->debugFlag) printf("Waiting for input...\n");
      if (chip8->keyPressed < 0) 
        break;
      chip8->V[x] = chip8->keyPressed;
      if (chip8->debugFlag) printf("Key 0x%.1x Pressed\n", chip8->V[x]);
      chip8->pc += 2;
      break;
    // 0xFx15 - Set delayTimer = V[x]
    case 0x0015:
      chip8->delayTimer = chip8->V[x];
      chip8->pc += 2;
      break;
    // 0xFx18 - Set soundTimer = V[x]
    case 0x0018:
      /*printf("Setting Sound Timer\n");*/
      chip8->soundTimer = chip8->V[x];
      chip8->pc += 2;
      break;
    // 0xFx1E - Set I = I + V[x]
    case 0x001E:
      chip8->I += chip8->V[x];
      chip8->pc += 2;
      break;
    // 0xFx29 - Set I equal to the memory address of the font-sprite for the value in V[x]
    case 0x0029:
      chip8->I = chip8->V[x] * 5;
      chip8->pc += 2;
      break;
    // 0xFx33 - Store BCD representation of V[x] at memory locations I, I + 1, I + 2
    case 0x0033:
      if (chip8->debugFlag) printf("V[%.1x] = %d\n", x, chip8->V[x]);
      chip8->memory[chip8->I] = chip8->V[x] / 100;
      chip8->memory[chip8->I + 1] = (chip8->V[x] % 100) / 10;
      chip8->memory[chip8->I + 2] = chip8->V[x] % 10;
      if (chip8->debugFlag) {
        printf("memory[0x%.3x] = %d\n", chip8->I, chip8->memory[chip8->I]);
        printf("memory[0x%.3x] = %d\n", chip8->I + 1, chip8->memory[chip8->I + 1]);
        printf("memory[0x%.3x] = %d\n", chip8->I + 2, chip8->memory[chip8->I + 2]);
      }
      chip8->pc += 2;
      break;
    // 0xFx55 - Store values from registers V[0] to V[x] into memory[I] onwards
    case 0x0055:
      for (int i = 0; i <= x; i++)
        chip8->memory[chip8->I + i] = chip8->V[i]; 
      chip8->pc += 2;
      break;
    // 0xFx55 - Store values starting from memory[I] into registers V[0] to V[x]
    case 0x0065:
      for (int i = 0; i <= x; i++)
        chip8->V[i] = chip8->memory[chip8->I + i]; 
      chip8->pc += 2;
      break;
  }
}
