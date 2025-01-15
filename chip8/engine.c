#include <bits/types/siginfo_t.h>
#include <pthread.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include "chip8.h"

bool chip8_init(chip8_t* chip8, uint32_t Hz) {
  if (chip8 == NULL) {
    return false;
  }

  if (pthread_mutex_init(&chip8->screen_mutex, NULL)) {
    perror("chip8 screen pthread_mutex_init");
    free(chip8);
    chip8 = NULL;
    return NULL;
  }
  if (pthread_mutex_init(&chip8->delay_timer.mut, NULL)) {
    perror("chip8 delay_timer pthread_mutex_init");
    free(chip8);
    chip8 = NULL;
    return NULL;
  }
  if (pthread_mutex_init(&chip8->sound_timer.mut, NULL)) {
    perror("chip8 screen_timer pthread_mutex_init");
    free(chip8);
    chip8 = NULL;
    return NULL;
  }
  if (pthread_mutex_init(&chip8->keypress.mut, NULL)) {
    perror("chip8 keypress pthread_mutex_init");
    free(chip8);
    chip8 = NULL;
    return NULL;
  }
  if (pthread_cond_init(&chip8->keypress.cond, NULL)) {
    perror("chip8 keypress pthread_mutex_init");
    free(chip8);
    chip8 = NULL;
    return NULL;
  }

  chip8->Hz = Hz;

  // load font.
  memcpy(chip8->memory, DEFAULT_FONT, 16 * 5);

  // set program counter
  chip8->program_counter = PROGRAM_START;

  return chip8;
}

void chip8_destroy(chip8_t* chip8) {
  if (chip8 == NULL) {
    return;
  }

  pthread_mutex_destroy(&chip8->screen_mutex);

  free(chip8);
  chip8 = NULL;
}

void chip8_load_program(chip8_t* chip8, const char* filepath) {
  FILE* f = fopen(filepath, "r");
  fread(&chip8->memory[PROGRAM_START], MAX_PROGRAM_LEN, 1, f);
}

struct engine_clock_args {
  uint32_t Hz;
  pthread_cond_t* tock;
};
void* engine_clock_thread(void* args);
void* chip8_run_thread(void* args) {
  struct run_thread_args* rta = (struct run_thread_args*)args;
  chip8_t* chip8 = rta->chip8;

  pthread_mutex_t tick;
  pthread_mutex_init(&tick, NULL);
  pthread_cond_t tock;
  pthread_cond_init(&tock, NULL);
  pthread_t engine_clock;
  struct engine_clock_args engine_clock_args = {
      .Hz = chip8->Hz,
      .tock = &tock,
  };
  if (pthread_create(&engine_clock, NULL, engine_clock_thread, &engine_clock_args)) {
    fprintf(stderr, "could not create engine_clock_thread\n");
    exit(1);
  }

  while (true) {
    pthread_mutex_lock(&tick);
    pthread_cond_wait(&tock, &tick);
    chip8_emulate_cycle(chip8);
    pthread_mutex_unlock(&tick);
  }
}
void* engine_clock_thread(void* args) {
  struct engine_clock_args e = *(struct engine_clock_args*)args;
  uint32_t Hz = e.Hz;
  pthread_cond_t* tock = e.tock;
  uint32_t usleep_time = 1000000 / Hz;

  while (true) {
    usleep(usleep_time);
    pthread_cond_broadcast(tock);
  }
}

bool chip8_emulate_cycle(chip8_t* chip8) {
  opcode_t opcode = fetch_opcode(chip8);
  opcode_f opcode_function = decode_opcode(opcode);
  int instructions_to_advance = opcode_function(chip8, opcode);
  chip8->program_counter += instructions_to_advance * sizeof(opcode_t);

  return true;
}

void* decrement_timer_thread(void* args) {
  struct timer_thread_args* targs = (struct timer_thread_args*)args;
  _timer_t* timer = targs->timer;
  uint32_t Hz = targs->Hz;

  uint32_t usleep_time = 1000000 / Hz;

  while (timer->value > 0) {
    pthread_mutex_lock(&timer->mut);
    timer->value -= 1;
    pthread_mutex_unlock(&timer->mut);
    usleep(usleep_time);
  }
  return NULL;
}
