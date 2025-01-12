#include <SDL2/SDL.h>
#include <pthread.h>
#include <signal.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/time.h>
#include <sys/types.h>
#include <unistd.h>

#include "chip8/chip8.h"
#include "defines.h"
#include "renderer/renderer.h"

#define IBM_LOGO_PROGRAM "./roms/ibm_logo.ch8"

chip8_t* CHIP8;
renderer_t* RENDERER;

void draw_handler(int signum) {
  // pthread locking is not signal safe.
  renderer_draw_screen(RENDERER, CHIP8->screen); // i dont think this is either though...
}
int main(int argc, char** argv) {
  uint32_t Hz = 5000; // TODO: should get as input
  CHIP8 = chip8_create(Hz);
  if (CHIP8 == NULL) {
    goto chip8_create_failure;
  }
  chip8_load_program(CHIP8, IBM_LOGO_PROGRAM);

  RENDERER = renderer_create(WINDOW_WIDTH, WINDOW_HEIGHT);
  if (RENDERER == NULL) {
    goto renderer_create_failure;
  }

  struct sigaction sa_draw = {
      .sa_handler = draw_handler,
  };

  if (sigaction(SIGDRAW, &sa_draw, NULL) == -1) {
    fprintf(stderr, "couldn't register SIGDRAW with sigaction\n");
  }

  pthread_t engine_thread;
  struct run_thread_args engine_args = {
      CHIP8,
  };
  // renderer_draw_screen(RENDERER, CHIP8->screen);
  if (pthread_create(&engine_thread, NULL, chip8_run_thread, &engine_args) == -1) {
    fprintf(stderr, "couldn't create 'chip8_run_thread' thread\n");
    goto engine_thread_failure;
  }

  bool running = true;
  SDL_Event event;
  while (running) {
    while (SDL_PollEvent(&event)) {
      switch (event.type) {
      // case SDL_QUIT:
      //   running = false;
      //   break;
      case SDL_KEYDOWN:
        running = false;
        break;
      default:
        break;
      }
    }
  }

engine_thread_failure:
  renderer_destroy(RENDERER);
renderer_create_failure:
  chip8_destroy(CHIP8);
chip8_create_failure:

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
