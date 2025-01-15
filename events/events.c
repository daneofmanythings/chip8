#include <stdio.h>

#include "events.h"

SDL_AppResult event_handle_keypress(SDL_KeyboardEvent ev, uint8_t* keypress) {
  switch (ev.scancode) {
  case SDL_SCANCODE_ESCAPE:
    return SDL_APP_SUCCESS;
  default:
    *keypress = filter_into(ev.scancode, DEFAULT_LAYOUT); // layout should be configurable
    // printf("scancode=%d, keypress=%d\n", ev.scancode, *keypress);
    return SDL_APP_CONTINUE;
  }
}

uint8_t filter_into(const SDL_Scancode sc, const uint8_t emulated_layout[16]) {
  switch (sc) {
  case SDL_SCANCODE_1:
    return emulated_layout[0];
  case SDL_SCANCODE_2:
    return emulated_layout[1];
  case SDL_SCANCODE_3:
    return emulated_layout[2];
  case SDL_SCANCODE_4:
    return emulated_layout[3];
  case SDL_SCANCODE_Q:
    return emulated_layout[4];
  case SDL_SCANCODE_W:
    return emulated_layout[5];
  case SDL_SCANCODE_E:
    return emulated_layout[6];
  case SDL_SCANCODE_R:
    return emulated_layout[7];
  case SDL_SCANCODE_A:
    return emulated_layout[8];
  case SDL_SCANCODE_S:
    return emulated_layout[9];
  case SDL_SCANCODE_D:
    return emulated_layout[10];
  case SDL_SCANCODE_F:
    return emulated_layout[11];
  case SDL_SCANCODE_Z:
    return emulated_layout[12];
  case SDL_SCANCODE_X:
    return emulated_layout[13];
  case SDL_SCANCODE_C:
    return emulated_layout[14];
  case SDL_SCANCODE_V:
    return emulated_layout[15];
  default:
    return 0xFF;
  }
}
