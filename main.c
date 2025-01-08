#include <SDL2/SDL.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>

#include "emulator/chip8.h"
#include "renderer/renderer.h"

#define WINDOW_WIDTH 900
#define WINDOW_HEIGHT 450

int main(int argc, char** argv) {
  chip8_t chip8 = {0};
  chip8_initialize(&chip8);

  renderer_t* r = renderer_create(WINDOW_WIDTH, WINDOW_HEIGHT);

  bool screen[SCREEN_WIDTH * SCREEN_HEIGHT] = {0};
  for (size_t i = 0; i < SCREEN_WIDTH * SCREEN_HEIGHT; ++i) {
    if (i % 2 == 0) {
      screen[i] = 1;
    }
  }
  renderer_draw_screen(r, screen);

  renderer_destroy(r);
  printf("program complete\n");
  return EXIT_SUCCESS;
}
