#include <SDL2/SDL.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>

#include "emulator/chip8.h"
#include "renderer/renderer.h"

int main(int argc, char** argv) {
  chip8_t chip8 = {0};
  chip8_initialize(&chip8);

  temp_run_window();

  printf("program complete\n");
  return EXIT_SUCCESS;
}
