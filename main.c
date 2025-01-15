#include <SDL3/SDL_init.h>
#define SDL_MAIN_USE_CALLBACKS 1
#include <SDL3/SDL.h>
#include <SDL3/SDL_main.h>

#include <pthread.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/time.h>
#include <sys/types.h>
#include <unistd.h>

#include "chip8/chip8.h"
#include "defines.h"
#include "events/events.h"
#include "render/render_state.h"

#define IBM_LOGO_PROGRAM "./roms/ibm_logo.ch8"
#define BC_TEST_PROGRAM "./roms/BC_test.ch8"
#define TEST_OPCODE_PROGRAM "./roms/test_opcode.ch8"
#define PONG "./roms/pong.ch8"

typedef struct {
  chip8_t* chip8;
  render_state_t* render_state;
} appstate_t;

void loading_screen_init(bool screen[SCREEN_SIZE]);
bool* create_test_screen();

SDL_AppResult SDL_AppInit(void** appstate, int argc, char** argv) {
  appstate_t* as = SDL_calloc(1, sizeof(appstate_t));
  if (as == NULL) {
    SDL_Log("could not calloc appstate_t\n");
  }
  *appstate = as;

  uint32_t Hz = 50; // TODO: should get as input
  as->render_state = SDL_calloc(1, sizeof(render_state_t));
  if (as->render_state == NULL) {
    SDL_Log("could not allocate render_state\n");
    return SDL_APP_FAILURE;
  }
  SDL_WindowFlags window_flags = 0;
  if (!render_state_init(as->render_state, WINDOW_WIDTH, WINDOW_HEIGHT, window_flags)) {
    return SDL_APP_FAILURE;
  }

  bool loading_screen[SCREEN_SIZE] = {0};
  loading_screen_init(loading_screen);
  draw_screen(as->render_state, loading_screen);

  as->chip8 = SDL_calloc(1, sizeof(chip8_t));
  if (!chip8_init(as->chip8, Hz)) {
    SDL_Log("could not init chip8");
    return SDL_APP_FAILURE;
  }

  chip8_load_program(as->chip8, BC_TEST_PROGRAM);

  pthread_t engine_thread;
  struct run_thread_args engine_args = {
      as->chip8,
  };
  if (pthread_create(&engine_thread, NULL, chip8_run_thread, &engine_args) == -1) {
    fprintf(stderr, "couldn't create 'chip8_run_thread' thread\n");
    return SDL_APP_FAILURE;
  }

  return SDL_APP_CONTINUE;
}

SDL_AppResult SDL_AppIterate(void* appstate) {
  appstate_t* as = (appstate_t*)appstate;
  chip8_t* ch8 = as->chip8;
  render_state_t* rs = as->render_state;

  // TODO: play sound if sound timer is non-zero

  pthread_mutex_lock(&ch8->screen_mutex);
  draw_screen(rs, ch8->screen);
  pthread_mutex_unlock(&ch8->screen_mutex);
  usleep(50000);

  return SDL_APP_CONTINUE;
}

SDL_AppResult SDL_AppEvent(void* appstate, SDL_Event* event) {
  appstate_t* as = (appstate_t*)appstate;
  keypress_t* keypress = &as->chip8->keypress;
  SDL_AppResult result;
  switch (event->type) {
  case SDL_EVENT_QUIT:
    return SDL_APP_SUCCESS;
  case SDL_EVENT_KEY_DOWN:
    pthread_mutex_lock(&keypress->mut);
    result = event_handle_keypress(event->key, &keypress->value);
    pthread_mutex_unlock(&keypress->mut);
    pthread_cond_broadcast(&keypress->cond);
    break;
  case SDL_EVENT_KEY_UP:
    pthread_mutex_lock(&keypress->mut);
    keypress->value = 0xFF;
    pthread_mutex_unlock(&keypress->mut);
    result = SDL_APP_CONTINUE;
    break;
  default:
    result = SDL_APP_CONTINUE;
  }
  return result;
}
void SDL_AppQuit(void* appstate, SDL_AppResult result) {
  // TODO: there are leaks that need to be cleaned up here.
  appstate_t* as = (appstate_t*)appstate;
  chip8_t* ch8 = as->chip8;
  render_state_t* rs = as->render_state;
  if (ch8 != NULL) {
    chip8_destroy(ch8);
  }
  if (rs != NULL) {
    // render_state_destroy(rs); // WHY IS THIS NOT FOUND
  }
}

bool* create_test_screen() {
  bool* test_screen = SDL_malloc(sizeof(bool) * (SCREEN_WIDTH * SCREEN_HEIGHT));
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

typedef struct {
  char c;
  uint8_t rows[5];
  uint8_t offset;
} character_t;

typedef struct {
  uint16_t start_x;
  uint16_t start_y;
  character_t data[SCREEN_WIDTH];
} word_t;

void loading_screen_init(bool screen[SCREEN_SIZE]) {
  // this function should be broken up, but nobody will see the load screen anyway.
  // clang-format off
  character_t L   = { 'L', {0b10000000, 0b10000000, 0b10000000, 0b10000000, 0b11110000}, 4 };
  character_t O   = { 'O', {0b11110000, 0b10010000, 0b10010000, 0b10010000, 0b11110000}, 4 };
  character_t A   = { 'A', {0b11110000, 0b10010000, 0b11110000, 0b10010000, 0b10010000}, 4 };
  character_t D   = { 'D', {0b11100000, 0b10010000, 0b10010000, 0b10010000, 0b11100000}, 4 };
  character_t I   = { 'I', {0b11100000, 0b01000000, 0b01000000, 0b01000000, 0b11100000}, 3 };
  character_t N   = { 'N', {0b10010000, 0b11010000, 0b10110000, 0b10010000, 0b10010000}, 4 };
  character_t G   = { 'G', {0b11110000, 0b10000000, 0b10110000, 0b10010000, 0b11110000}, 4 };
  character_t Dot = { '.', {0,          0,          0,          0,          0b10000000}, 1 };
  // clang-format on

  word_t word = {
      .start_x = 12,
      .start_y = 13,
      .data = {L, O, A, D, I, N, G, Dot, Dot, Dot},
  };
  size_t draw_start;
  size_t row_start;

  for (size_t letter = 0; letter < 10; ++letter) {
    draw_start = word.start_x + word.start_y * SCREEN_WIDTH;
    for (size_t i = letter; i > 0; --i) {
      draw_start += word.data[i - 1].offset + 1;
    }
    for (size_t row = 0; row < 5; ++row) {
      row_start = draw_start + (row * SCREEN_WIDTH);
      for (size_t bit = 0; bit < 4; ++bit) {
        if (((word.data[letter].rows[row] >> (7 - bit)) & 1) == 1) {
          if (row_start + bit >= SCREEN_SIZE) {
            fprintf(stderr, "math is wrong in loading_sreen_create\n");
            exit(1);
          }
          screen[row_start + bit] = 1;
        }
      }
    }
  }
}
