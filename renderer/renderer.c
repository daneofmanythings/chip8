#include <SDL2/SDL.h>
#include <stdbool.h>

#include "renderer.h"

void temp_run_window(void) {
  SDL_Init(SDL_INIT_EVERYTHING);
  SDL_Window* window = SDL_CreateWindow("my window", 100, 100, 640, 400, SDL_WINDOW_SHOWN);
  if (window == NULL) {
    fprintf(stderr, "ERROR: SDL failed to create window: %s\n", SDL_GetError());
    exit(1);
  }

  SDL_Surface* window_surface = SDL_GetWindowSurface(window);
  if (window_surface == NULL) {
    fprintf(stderr, "ERROR: SDL failed to get window surface: %s\n", SDL_GetError());
    exit(1);
  }

  SDL_Surface* loaded_image = SDL_LoadBMP("./assets/640-480-sample.bmp");
  if (loaded_image == NULL) {
    fprintf(stderr, "ERROR: SDL failed to load bmp: %s\n", SDL_GetError());
    exit(1);
  }
  SDL_Surface* image = SDL_ConvertSurface(loaded_image, window_surface->format, 0);
  if (loaded_image == NULL) {
    fprintf(stderr, "ERROR: SDL failed to convert bmp image: %s\n", SDL_GetError());
    exit(1);
  } else {
    SDL_FreeSurface(loaded_image);
    loaded_image = NULL;
  }

  SDL_Rect image_rect = (SDL_Rect){
      .x = 100,
      .y = 50,
      .w = 200,
      .h = 100,
  };

  int result = SDL_BlitScaled(image, NULL, window_surface, &image_rect);
  if (result != 0) {
    // handle error
    exit(1);
  }

  /*SDL_FillRect(window_surface, NULL,*/
  /*             SDL_MapRGB(window_surface->format, 255, 90, 120));*/

  SDL_UpdateWindowSurface(window);

  bool running = true;
  SDL_Event event;

  while (running) {
    while (SDL_PollEvent(&event)) {
      switch (event.type) {
      case SDL_QUIT:
        running = false;
        break;
      case SDL_KEYDOWN:
        running = false;
        break;
      default:
        break;
      }
    }
  }

  SDL_FreeSurface(image);
  image = NULL;
  SDL_DestroyWindow(window);
  window = NULL;
  window_surface = NULL;
  SDL_Quit();
}
