#include <SDL2/SDL.h>
#include <SDL2/SDL_pixels.h>
#include <SDL2/SDL_rect.h>
#include <SDL2/SDL_surface.h>
#include <SDL2/SDL_video.h>
#include <pthread.h>
#include <stdbool.h>
#include <string.h>

#include "renderer.h"

renderer_t* renderer_create(const size_t window_width, const size_t window_height) {
  if (window_width < SCREEN_WIDTH || window_height < SCREEN_HEIGHT) {
    fprintf(stderr, "ERROR: window width or height are too small. minimum is 64x32. received: width=%zu, height=%zu\n",
            window_width, window_height);
    exit(1);
  }
  renderer_t* renderer = calloc(sizeof(renderer_t), 1);
  if (renderer == NULL) {
    perror("renderer calloc");
    return NULL;
  }

  SDL_Init(SDL_INIT_EVERYTHING);
  SDL_Window* window = SDL_CreateWindow("chip8 emulator", 100, 100, window_width, window_height, SDL_WINDOW_SHOWN);
  if (window == NULL) {
    free(renderer);
    fprintf(stderr, "ERROR: SDL failed to create window: %s\n", SDL_GetError());
    exit(1);
  }

  renderer->window = window;

  SDL_Surface* window_surface = SDL_GetWindowSurface(window);
  if (window_surface == NULL) {
    SDL_DestroyWindow(window);
    free(renderer);
    fprintf(stderr, "ERROR: SDL failed to get window surface: %s\n", SDL_GetError());
    exit(1);
  }

  renderer->surface = window_surface;
  renderer->pixel = _create_pixel_offsets_only(window_width, window_height);

  size_t screen_start_x = (window_width - renderer->pixel.w * SCREEN_WIDTH) / 2;
  memcpy((void*)&renderer->screen_start_x, &screen_start_x, sizeof(typeof(screen_start_x)));
  size_t screen_start_y = (window_height - renderer->pixel.h * SCREEN_HEIGHT) / 2;
  memcpy((void*)&renderer->screen_start_y, &screen_start_y, sizeof(typeof(screen_start_y)));

  return renderer;
}

SDL_Rect _create_pixel_offsets_only(size_t window_width, size_t window_height) {
  size_t side_len;
  size_t x_len = window_width / SCREEN_WIDTH;
  size_t y_len = window_height / SCREEN_HEIGHT;
  if (x_len < y_len) {
    side_len = x_len;
  } else {
    side_len = y_len;
  }
  return (SDL_Rect){
      .w = side_len,
      .h = side_len,
      .x = 0,
      .y = 0,
  };
}

void _pixel_update_position(renderer_t* renderer, size_t i) {
  renderer->pixel.x = (i % 64) * renderer->pixel.w + renderer->screen_start_x;
  renderer->pixel.y = (i / 64) * renderer->pixel.h + renderer->screen_start_y;
}

void renderer_destroy(renderer_t* renderer) {
  SDL_DestroyWindow(renderer->window);
  renderer->window = NULL;
  renderer->surface = NULL;

  free(renderer);
  renderer = NULL;

  SDL_Quit();
}

void renderer_draw_screen(renderer_t* r, const bool screen[SCREEN_SIZE]) {
  struct color {
    uint8_t r;
    uint8_t g;
    uint8_t b;
  };
  static const struct color background = {200, 50, 50};
  static const struct color black = {0, 0, 0};
  static const struct color white = {210, 210, 210};
  SDL_FillRect(r->surface, NULL, SDL_MapRGB(r->surface->format, background.r, background.g, background.b));
  struct color c = {0};
  for (size_t i = 0; i < SCREEN_SIZE; ++i) {
    _pixel_update_position(r, i);
    if (screen[i] == 0) {
      c = black;
    } else {
      c = white;
    }
    SDL_FillRect(r->surface, &r->pixel, SDL_MapRGB(r->surface->format, c.r, c.g, c.b));
  }
  SDL_UpdateWindowSurface(r->window);
}
