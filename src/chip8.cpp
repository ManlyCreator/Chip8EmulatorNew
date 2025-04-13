#include "chip8.h"
#include "opcodes.h"
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
  opcode = 0;

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
  float lastTime, currentTime, elapsedTime, deltaTime = 0;
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

    screen->draw(display);
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
  opcodes[(opcode & 0xF000) >> 12](this);
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
