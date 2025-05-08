// Standard Libraries
#include <stdio.h>
#include <string.h>

// External Libraries
#include "chip8.h"

// TODO: Load a ROM when selected in the File menu

int main(int argc, char **argv) {
  // ROM Loader
  char rom[256] = "../roms/";

  if (argv[1]) {
    strcat(rom, argv[1]);
  } else {
    char temp[256];
    printf("Insert a ROM: ");
    scanf("%s", temp);
    strcat(rom, temp);
  }
  strcat(rom, ".ch8");

  // Chip8
  Chip8 chip8(16, 0);
  if (!chip8.LoadROM(rom))
    return -1;
  chip8.StartMainLoop();

  return 0;
}
