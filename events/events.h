#ifndef EVENTS_EVENTS_H
#define EVENTS_EVENTS_H

#include <SDL3/SDL.h>

SDL_AppResult event_handle_keypress(SDL_KeyboardEvent ev, uint8_t* keypress);
uint8_t filter_into(const SDL_Scancode sc, const uint8_t emulated_layout[16]); // TODO:

static const uint8_t DEFAULT_LAYOUT[4 * 4] = {
    0x01, 0x02, 0x03, 0x0C, //
    0x04, 0x05, 0x06, 0x0D, //
    0x07, 0x08, 0x09, 0x0E, //
    0x0A, 0x00, 0x0B, 0x0F, //
};

#endif // !DEBUG
