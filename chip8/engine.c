#include <bits/types/siginfo_t.h>
#include <pthread.h>
#include <signal.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include "chip8.h"

#define PROGRAM_START 512
#define MAX_PROGRAM_LEN (4096 - (PROGRAM_START))

chip8_t* chip8_create(uint32_t Hz) {
  chip8_t* chip8 = calloc(sizeof(chip8_t), 1);
  if (chip8 == NULL) {
    perror("chip8 calloc");
    return NULL;
  }

  chip8->Hz = Hz;

  // load font.

  // set program counter
  chip8->program_counter = PROGRAM_START;

  return chip8;
}

void chip8_destroy(chip8_t* chip8) {
  if (chip8 == NULL) {
    return;
  }

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
  pid_t pid = getpid();
  union sigval sv_draw = {
      .sival_int = SIGDRAW,
  };

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
    // fprintf(stdout, "run loop...\n");
    // usleep(1000000 / chip8->Hz); // this will become a clock thread eventually
    pthread_mutex_lock(&tick);
    pthread_cond_wait(&tock, &tick);
    if (chip8_emulate_cycle(chip8)) {
      // printf("sending draw signal\n");
      sigqueue(pid, SIGDRAW, sv_draw);
    }
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
  // fetch opcode
  opcode_t opcode = get_opcode(chip8);

  // decode opcode
  opcode_f opcode_function = decode_opcode(opcode);
  // execute opcode
  int instructions_to_advance = opcode_function(chip8, opcode);
  chip8->program_counter += instructions_to_advance * sizeof(opcode_t);

  // update the timers
  if (chip8->sound_timer > 0) {
    // TODO: make sound
    chip8->sound_timer -= 1;
  }
  if (chip8->delay_timer > 0) {
    chip8->delay_timer -= 1;
  }

  // redraw if screen buffer was written to
  if (chip8->should_redraw == true) {
    chip8->should_redraw = false;
    return true;
  }
  return false;
}
