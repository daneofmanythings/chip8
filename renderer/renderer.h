#ifndef RENDERER_RENDERER_H
#define RENDERER_RENDERER_H

#include <SDL2/SDL.h>
#include <stdbool.h>
#include <stddef.h>

#define SCREEN_WIDTH 64  // defined for semantic meaning. will never change.
#define SCREEN_HEIGHT 32 // (also defined in chip8.h)

void temp_run_window(void);

typedef struct renderer {
  SDL_Window* window;
  SDL_Surface* surface;
  SDL_Rect pixel;
  const size_t screen_start_x;
  const size_t screen_start_y;
} renderer_t;

renderer_t* renderer_create(const size_t window_width, const size_t window_height);
void renderer_destroy(renderer_t* renderer);
void renderer_draw_screen(renderer_t* renderer, const bool screen[SCREEN_WIDTH * SCREEN_HEIGHT]);

SDL_Rect _create_pixel_offsets_only(size_t window_width, size_t window_height);

#endif // !DEBUG
