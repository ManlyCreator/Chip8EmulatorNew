#include "chip8.h"
#include <algorithm>
#include <fstream>
#include <iostream>
#include <memory>
#include <time.h>

int virtualKeys[] = { 
  GLFW_KEY_1, // 0
  GLFW_KEY_2, // 1
  GLFW_KEY_3, // 2
  GLFW_KEY_4, // 3
  GLFW_KEY_Q, // 4
  GLFW_KEY_W, // 5
  GLFW_KEY_E, // 6
  GLFW_KEY_R, // 7
  GLFW_KEY_A, // 8
  GLFW_KEY_S, // 9
  GLFW_KEY_D, // A
  GLFW_KEY_F, // B
  GLFW_KEY_Z, // C
  GLFW_KEY_X, // D
  GLFW_KEY_C, // E
  GLFW_KEY_V, // F
};

Byte fontset[80] = { 
  0xF0, 0x90, 0x90, 0x90, 0xF0, // 0
  0x20, 0x60, 0x20, 0x20, 0x70, // 1
  0xF0, 0x10, 0xF0, 0x80, 0xF0, // 2
  0xF0, 0x10, 0xF0, 0x10, 0xF0, // 3
  0x90, 0x90, 0xF0, 0x10, 0x10, // 4
  0xF0, 0x80, 0xF0, 0x10, 0xF0, // 5
  0xF0, 0x80, 0xF0, 0x90, 0xF0, // 6
  0xF0, 0x10, 0x20, 0x40, 0x40, // 7
  0xF0, 0x90, 0xF0, 0x90, 0xF0, // 8
  0xF0, 0x90, 0xF0, 0x10, 0xF0, // 9
  0xF0, 0x90, 0xF0, 0x90, 0x90, // A
  0xE0, 0x90, 0xE0, 0x90, 0xE0, // B
  0xF0, 0x80, 0x80, 0x80, 0xF0, // C
  0xE0, 0x90, 0x90, 0x90, 0xE0, // D
  0xF0, 0x80, 0xF0, 0x80, 0xF0, // E
  0xF0, 0x80, 0xF0, 0x80, 0x80  // F
};

Chip8::Chip8(Byte instructionFrequency, Byte debugFlag) {
  this->instructionFrequency = instructionFrequency;
  this->debugFlag = debugFlag;
  I  = 0;
  pc = 0x200;
  sp = 0;
  delayTimer = 0;
  soundTimer = 0;
  lastTime = 0;
  currentTime = 0;
  elapsedTime = 0;
  deltaTime = 0;
  opcode = 0;
  opcodeTable = {
    &Chip8::op0xxx,
    &Chip8::op1xxx,
    &Chip8::op2xxx,
    &Chip8::op3xxx,
    &Chip8::op4xxx,
    &Chip8::op5xxx,
    &Chip8::op6xxx,
    &Chip8::op7xxx,
    &Chip8::op8xxx,
    &Chip8::op9xxx,
    &Chip8::opAxxx,
    &Chip8::opBxxx,
    &Chip8::opCxxx,
    &Chip8::opDxxx,
    &Chip8::opExxx,
    &Chip8::opFxxx,
  };

  std::fill(memory, memory + MEMORY, 0);
  for (int i = 0; i < 80; i++) {
    memory[i] = fontset[i];
  }
  std::fill(display, display + (DISPLAY_WIDTH * DISPLAY_HEIGHT), 0);
  screen = std::make_unique<Screen>("../vertexShader.glsl", "../fragmentShader.glsl", DISPLAY_WIDTH, DISPLAY_HEIGHT, display);
  buzzer = std::make_unique<Buzzer>();
}

int Chip8::loadROM(const char *romPath) {
  Byte *buffer;
  std::size_t bufferSize;
  std::ifstream rom(romPath, std::ios::binary | std::ios::ate);

  if (!rom.is_open())
    return 0;

  bufferSize = rom.tellg();
  buffer = new Byte[bufferSize];

  rom.seekg(0, rom.beg);
  rom.read(reinterpret_cast<char*>(buffer), bufferSize);

  for (int i = 0; i < bufferSize; i++) {
    memory[0x200 + i] = buffer[i];
  }

  rom.close();
  delete[] buffer;

  return 1; 
}

void Chip8::startMainLoop() {
  Byte soundPlaying = 0;
  while (!glfwWindowShouldClose(screen->window)) {
    currentTime = glfwGetTime();
    deltaTime = currentTime - lastTime;
    elapsedTime += deltaTime;
    lastTime = glfwGetTime();

    // Buzzer Control
    if (soundTimer > 0 && !soundPlaying) {
      buzzer->play();
      soundPlaying = 1;
    }
    else {
      buzzer->stop();
      soundPlaying = 0;
    }
    
    // Display Refresh
    if (elapsedTime < DISPLAY_FREQUENCY) continue;
    tick();
    soundTimer = soundTimer > 0 ? soundTimer - 1 : 0;
    delayTimer = delayTimer > 0 ? delayTimer - 1 : 0;
    elapsedTime = 0;

    screen->draw(this);
  }
}

void Chip8::tick() {
  for (int i = 0; i < instructionFrequency; i++) {
    emulateCycle();
  }
}

void Chip8::emulateCycle() {
  opcode = (memory[pc] << 8) | memory[pc + 1];
  if (debugFlag) {
    printf("Reading memory[0x%.3x] = 0x%.2x and memory[0x%.3x] = 0x%.2x\n", pc, memory[pc], pc + 1, memory[pc + 1]);
    printf("Opcode: 0x%.4x\n", opcode);
  }

  // Process input before decoding
  processInput();

  // Decode Instructions
  (this->*opcodeTable[(opcode & 0xF000) >> 12])();
}

void Chip8::processInput() {
  keyPressed = -1;
  for (int i = 0; i < 16; i++) {
    if (glfwGetKey(screen->window, virtualKeys[i]) == GLFW_PRESS) {
      key[i] = 1;
      keyPressed = i;
    } else {
      key[i] = 0;
    }
  }
}

void Chip8::op0xxx() {
  switch (opcode) {
    // 0x00E0 - Clear Screen
    case 0x00E0:
      if (debugFlag) printf("Clearing Display\n");
      std::fill(display, display + (DISPLAY_WIDTH * DISPLAY_HEIGHT), 0);
      pc += 2;
      break;
    // 0x00EE - Return
    case 0x00EE:
      pc = stack[--sp] + 2;
      if (debugFlag) printf("Returning to 0x%.3x\n", pc);
      break;
  }
}

// 0x1nnn - Jump to address nnn
void Chip8::op1xxx() {
  pc = opcode & 0x0FFF;
  if (debugFlag) printf("Setting PC to: %d\n", pc);
}

// 0x2nnn - Call function at nnn
void Chip8::op2xxx() {
  stack[sp++] = pc;
  pc = opcode & 0x0FFF;
  if (debugFlag) printf("Calling function at 0x%.3x\n", pc);
}

// 0x3xbb - Skip next instruction if V[x] == bb
void Chip8::op3xxx() {
  Byte x = (opcode & 0x0F00) >> 8;
  if (V[x] == (opcode & 0x00FF))
    pc += 2;
  pc += 2;
}

// 0x4xbb - Skip next instruction if V[x] != bb
void Chip8::op4xxx() {
  Byte x = (opcode & 0x0F00) >> 8;
  if (V[x] != (opcode & 0x00FF))
    pc += 2;
  pc += 2;
}

// 0x5xy0 - Skip next instruction if V[x] == V[y]
void Chip8::op5xxx() {
  Byte x = (opcode & 0x0F00) >> 8;
  Byte y = (opcode & 0x00F0) >> 4;
  if (V[x] == V[y])
    pc += 2;
  pc += 2;
}

// 0x6xbb - Load bb into V[x]
void Chip8::op6xxx() {
  Byte x = (opcode & 0x0F00) >> 8;
  V[x] = opcode & 0x00FF;
  if (debugFlag) printf("Loaded %d into V[0x%x]\n", V[x], x);
  pc += 2;
}

// 0x7xbb - Increment V[x] by bb
void Chip8::op7xxx() {
  Byte x = (opcode & 0x0F00) >> 8;
  if (debugFlag) printf("Incrementing V[%d] by %d\n", x, opcode & 0x00FF);
  V[x] += opcode & 0x00FF;
  pc += 2;
}

void Chip8::op8xxx() {
  Byte x = (opcode & 0x0F00) >> 8;
  Byte y = (opcode & 0x00F0) >> 4;
  switch (opcode & 0x000F) {
    // 0x8xy0 - Load V[y] into V[x]
    case 0x0000:
      V[x] = V[y];
      pc += 2;
      break;
    // 0x8xy1 - Set V[x] = V[x] OR V[y]
    case 0x0001:
      V[x] |= V[y];
      pc += 2;
      break;
    // 0x8xy2 - Set V[x] = V[x] AND V[y]
    case 0x0002:
      V[x] &= V[y];
      pc += 2;
      break;
    // 0x8xy3 - Set V[x] = V[x] XOR V[y]
    case 0x0003:
      V[x] ^= V[y];
      pc += 2;
      break;
    // 0x8xy4 - Increment V[x] by V[y]
    case 0x0004:
      if (V[x] + V[y] > 0xFF)
        V[0xF] = 1;
      else
        V[0xF] = 0;
      V[x] += V[y] & 0xFF;
      pc += 2;
      break;
    // 0x8xy5 - Decrement V[x] by V[y]
    case 0x0005:
      if (V[x] > V[y]) 
        V[0xF] = 1;
      else
        V[0xF] = 0;
      V[x] -= V[y];
      pc += 2;
      break;
    // 0x8xy6 - Shift right V[x] by 1 bit
    case 0x0006:
      if ((V[x] & 0x01) == 0x01)
        V[0xF] = 1;
      else
        V[0xF] = 0;
      V[x] /= 2;
      pc += 2;
      break;
    // 0x8xy7 - Set V[x] = V[y] - V[x]
    case 0x0007:
      if (V[y] > V[x])
        V[0xF] = 1;
      else
        V[0xF] = 0;
      V[x] = V[y] - V[x];
      pc += 2;
      break;
    // 0x8xyE - Shift left V[x] by 1 bit
    case 0x000E:
      if ((V[x] & 0x80) == 0x80) 
        V[0xF] = 1;
      else
        V[0xF] = 0;
      V[x] *= 2;
      pc += 2;
      break;
  }
}

// 0x9xy0 - Skip next instruction if V[x] != V[y]
void Chip8::op9xxx() {
  Byte x = (opcode & 0x0F00) >> 8;
  Byte y = (opcode & 0x00F0) >> 4;
  if (V[x] != V[y])
    pc += 2;
  pc += 2;
}

// 0xAnnn - Load nnn into I
void Chip8::opAxxx() {
  I = opcode & 0x0FFF;
  if (debugFlag) {
    printf("Loaded %d into I\n", I);
    printf("memory[I] = 0x%.2x\n", memory[I]);
  }
  pc += 2;
}

// 0xBnnn - Jump to address nnn + V[0]
void Chip8::opBxxx() {
  pc = V[0] + opcode & 0x0FFF;
  if (debugFlag) printf("Jumped to address 0x%.3x\n", pc);
}

// 0xCxbb - Set V[x] = rand(0, 255) AND bb
void Chip8::opCxxx() {
  Byte x = (opcode & 0x0F00) >> 8;
  srand(time(NULL));
  V[x] = (rand() % 256) & (opcode & 0x00FF);
  if (debugFlag) printf("Generated random number in V[0x%x]: 0x%.2x\n", x, V[x]);
  pc += 2;
}

// 0xDxyn - Draw a sprite of n-bytes high at (V[x], V[y])
void Chip8::opDxxx() {
  Byte spriteRow;
  Byte x = (opcode & 0x0F00) >> 8;
  Byte y = (opcode & 0x00F0) >> 4;
  Byte height = opcode & 0x000F;
  V[0xF] = 0;
  for (int i = 0; i < height; i++) {
    spriteRow = memory[I + i];
    for (int j = 0; j < 8; j++) {
      unsigned index = V[x] + j + ((V[y] + i) * DISPLAY_WIDTH);
      Byte pixel = (spriteRow & (0x80 >> j)) > 0 ? 255 : 0;
      if (display[index] == 1)
        V[0xF] = 1;
      display[index] ^= pixel;
    }
  }
  pc += 2;
}

void Chip8::opExxx() {
  Byte x = (opcode & 0x0F00) >> 8;
  Byte y = (opcode & 0x00F0) >> 4;
  switch (opcode & 0x00FF) {
    // 0xEx9E - Skip next instruction if the key value of V[x] is pressed
    case 0x009E:
      if (key[V[x]])
        pc += 2;
      pc += 2;
      break;
    // 0xExA1 - Skip next instruction if the key value of V[x] is NOT pressed
    case 0x00A1:
      if (!key[V[x]])
        pc += 2;
      pc += 2;
      break;
  }
}

void Chip8::opFxxx() {
  Byte x = (opcode & 0x0F00) >> 8;
  Byte y = (opcode & 0x00F0) >> 4;
  switch (opcode & 0x00FF) {
    // 0xFx07 - Set V[x] = delayTimer
    case 0x0007:
      V[x] = delayTimer;
      pc += 2;
      break;
    // 0xFx0A - Wait for input and store the key value in V[x]
    case 0x000A:
      if (debugFlag) printf("Waiting for input...\n");
      if (keyPressed < 0) 
        break;
      V[x] = keyPressed;
      if (debugFlag) printf("Key 0x%.1x Pressed\n", V[x]);
      pc += 2;
      break;
    // 0xFx15 - Set delayTimer = V[x]
    case 0x0015:
      delayTimer = V[x];
      pc += 2;
      break;
    // 0xFx18 - Set soundTimer = V[x]
    case 0x0018:
      /*printf("Setting Sound Timer\n");*/
      soundTimer = V[x];
      pc += 2;
      break;
    // 0xFx1E - Set I = I + V[x]
    case 0x001E:
      I += V[x];
      pc += 2;
      break;
    // 0xFx29 - Set I equal to the memory address of the font-sprite for the value in V[x]
    case 0x0029:
      I = V[x] * 5;
      pc += 2;
      break;
    // 0xFx33 - Store BCD representation of V[x] at memory locations I, I + 1, I + 2
    case 0x0033:
      if (debugFlag) printf("V[%.1x] = %d\n", x, V[x]);
      memory[I] = V[x] / 100;
      memory[I + 1] = (V[x] % 100) / 10;
      memory[I + 2] = V[x] % 10;
      if (debugFlag) {
        printf("memory[0x%.3x] = %d\n", I, memory[I]);
        printf("memory[0x%.3x] = %d\n", I + 1, memory[I + 1]);
        printf("memory[0x%.3x] = %d\n", I + 2, memory[I + 2]);
      }
      pc += 2;
      break;
    // 0xFx55 - Store values from registers V[0] to V[x] into memory[I] onwards
    case 0x0055:
      for (int i = 0; i <= x; i++)
        memory[I + i] = V[i]; 
      pc += 2;
      break;
    // 0xFx55 - Store values starting from memory[I] into registers V[0] to V[x]
    case 0x0065:
      for (int i = 0; i <= x; i++)
        V[i] = memory[I + i]; 
      pc += 2;
      break;
  }
}
