#include <SDL2/SDL.h>
#include <pthread.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#include "emulator/chip8.h"
#include "renderer/renderer.h"

#define WINDOW_WIDTH 900
#define WINDOW_HEIGHT 450

#define IBM_LOGO_PROGRAM "./roms/ibm_logo.ch8"

chip8_t* CHIP8;
renderer_t* RENDERER;

int main(int argc, char** argv) {
  CHIP8 = chip8_create();
  RENDERER = renderer_create(WINDOW_WIDTH, WINDOW_HEIGHT);

  chip8_initialize(CHIP8);
  chip8_load_program(CHIP8, IBM_LOGO_PROGRAM);

  struct run_thread_args args = {
      CHIP8,
      RENDERER,
  };

  renderer_draw_screen(RENDERER, CHIP8->screen);

  pthread_t pid = {0};
  if (pthread_create(&pid, NULL, chip8_run_thread, &args) == -1) {
    fprintf(stderr, "couldn't create 'chip8_run_thread' thread\n");
    goto cleanup;
  }

  bool running = true;
  SDL_Event event;
  while (running) {
    while (SDL_PollEvent(&event)) {
      switch (event.type) {
      case SDL_QUIT:
        running = false;
        break;
        // case SDL_KEYDOWN:
        //   for (size_t i = 0; i < SCREEN_WIDTH * SCREEN_HEIGHT; ++i) {
        //     test_screen[i] ^= 1;
        //   }
        break;
      default:
        break;
      }
    }
  }

cleanup:
  renderer_destroy(RENDERER);

  printf("program complete\n");
  return EXIT_SUCCESS;
}

bool* create_test_screen() {
  bool* test_screen = malloc(sizeof(bool) * (SCREEN_WIDTH * SCREEN_HEIGHT));
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
  return test_screen;
}
