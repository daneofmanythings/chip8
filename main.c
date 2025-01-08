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

  bool test_screen[SCREEN_WIDTH * SCREEN_HEIGHT] = {0};
  bool white = 0;
  bool black = 1;
  for (size_t i = 0; i < SCREEN_WIDTH * SCREEN_HEIGHT; ++i) {
    if (i % SCREEN_WIDTH == 0) {
      white ^= 1;
      black ^= 1;
    }
    if (i % 2 == 0) {
      test_screen[i] = white;
    } else {
      test_screen[i] = black;
    }
  }

  renderer_draw_screen(r, test_screen);

  renderer_destroy(r);
  printf("program complete\n");
  return EXIT_SUCCESS;
}
