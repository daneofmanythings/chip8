#ifndef RENDERER_RENDERER_H
#define RENDERER_RENDERER_H

#include <SDL3/SDL.h>
#include <stdbool.h>
#include <stddef.h>

#include "../defines.h"

void temp_run_window(void);

typedef struct {
  SDL_Renderer* renderer;
  SDL_Window* window;
  SDL_FRect pixel;
  size_t screen_start_x;
  size_t screen_start_y;
} render_state_t;

bool render_state_init(render_state_t* rs, const size_t window_width, const size_t window_height,
                       SDL_WindowFlags flags);
void render_state_destroy(render_state_t* renderer);
void draw_screen(render_state_t* renderer, const bool screen[SCREEN_SIZE]);

SDL_FRect _create_pixel_offsets_only(const size_t window_width, const size_t window_height);

#endif // !DEBUG
