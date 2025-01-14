#include <SDL3/SDL_pixels.h>
#include <SDL3/SDL_render.h>
#include <pthread.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "renderer.h"

bool render_state_init(render_state_t* rs, const size_t width, const size_t height, const SDL_WindowFlags flags) {
  if (width < SCREEN_WIDTH || height < SCREEN_HEIGHT) {
    SDL_Log("ERROR: window width or height are too small. minimum is 64x32. received: width=%zu, height=%zu\n", width,
            height);
    return false;
  }

  SDL_Init(SDL_INIT_VIDEO);

  bool is_successful = SDL_CreateWindowAndRenderer("chip8 emulator", width, height, flags, &rs->window, &rs->renderer);
  if (is_successful == false) {
    return false;
  }

  rs->pixel = _create_pixel_offsets_only(width, height);

  rs->screen_start_x = (width - rs->pixel.w * SCREEN_WIDTH) / 2;
  rs->screen_start_y = (height - rs->pixel.h * SCREEN_HEIGHT) / 2;

  return true;
}

SDL_FRect _create_pixel_offsets_only(const size_t window_width, const size_t window_height) {
  size_t side_len;
  size_t x_len = window_width / SCREEN_WIDTH;
  size_t y_len = window_height / SCREEN_HEIGHT;
  if (x_len < y_len) {
    side_len = x_len;
  } else {
    side_len = y_len;
  }
  return (SDL_FRect){
      .w = side_len,
      .h = side_len,
      .x = 0,
      .y = 0,
  };
}

void _pixel_update_position(render_state_t* rs, size_t i) {
  rs->pixel.x = (i % 64) * rs->pixel.w + rs->screen_start_x;
  rs->pixel.y = (i / 64) * rs->pixel.h + rs->screen_start_y;
}

void renderer_destroy(render_state_t* rs) {
  SDL_DestroyRenderer(rs->renderer);
  rs->renderer = NULL;
  SDL_DestroyWindow(rs->window);
  rs->window = NULL;

  free(rs);
  rs = NULL;
}

void draw_screen(render_state_t* rs, const bool screen[SCREEN_SIZE]) {
  struct color {
    uint8_t r;
    uint8_t g;
    uint8_t b;
  };
  static const struct color background = {200, 50, 50};
  static const struct color black = {0, 0, 0};
  static const struct color white = {210, 210, 210};
  SDL_SetRenderDrawColor(rs->renderer, background.r, background.g, background.b, SDL_ALPHA_OPAQUE);
  SDL_RenderFillRect(rs->renderer, NULL);
  struct color c = {0};
  for (size_t i = 0; i < SCREEN_SIZE; ++i) {
    _pixel_update_position(rs, i);
    if (screen[i] == 0) {
      c = black;
    } else {
      c = white;
    }
    SDL_SetRenderDrawColor(rs->renderer, c.r, c.g, c.b, SDL_ALPHA_OPAQUE);
    SDL_RenderFillRect(rs->renderer, &rs->pixel);
  }
  SDL_RenderPresent(rs->renderer);
}
