#include <pthread.h>
#include <signal.h>
#include <stdbool.h>
#include <stdint.h>

#include "../defines.h"
#include "../renderer/renderer.h"

#define STACK_CAPACITY 16

typedef struct chip_stack_t {
  uint16_t pointer;
  uint16_t data[STACK_CAPACITY];
} chip8_stack_t;

typedef void (*render_f)(bool[SCREEN_SIZE]);

typedef struct chip8 {
  uint32_t Hz;
  uint8_t memory[4096];
  pthread_mutex_t screen_mutex;
  bool screen[SCREEN_SIZE];
  uint8_t registers[16];
  chip8_stack_t stack;
  uint16_t index_register;
  uint16_t program_counter;
  uint8_t delay_timer;
  uint8_t sound_timer;
  uint8_t keypad[16];
} chip8_t;

typedef uint16_t opcode_t;
typedef int (*opcode_f)(chip8_t*, opcode_t);

struct run_thread_args {
  chip8_t* chip8;
};
void* chip8_run_thread(void* args);
bool chip8_init(chip8_t* chip8, uint32_t Hz);
void chip8_destroy(chip8_t* chip8);
void chip8_load_program(chip8_t* chip8, const char* filepath);
bool chip8_emulate_cycle(chip8_t* chip8);
opcode_t get_opcode(const chip8_t* chip8);
opcode_f decode_opcode(opcode_t opcode);
// helpers
bool nth_bit(uint8_t byte, uint8_t bit_to_check);
uint16_t chip8_stack_pop(chip8_t* chip8);
void chip8_stack_push(chip8_t* chip8, uint16_t value);

int call_machine_code_routine_at_NNN(chip8_t* chip8, const opcode_t opcode);
int clear_display(chip8_t* chip8, opcode_t opcode);
int flow_return(chip8_t* chip8, opcode_t opcode);
int flow_goto_NNN(chip8_t* chip8, const opcode_t opcode);
int flow_call_subroutine_at_NNN(chip8_t* chip8, const opcode_t opcode);
int condition_skip_if_VX_equal_NN(chip8_t* chip8, const opcode_t opcode);
int condition_skip_if_VX_not_equal_NN(chip8_t* chip8, const opcode_t opcode);
int condition_skip_if_VX_equal_VY(chip8_t* chip8, const opcode_t opcode);
int constant_set_VX_to_NN(chip8_t* chip8, const opcode_t opcode);
int constant_add_NN_to_VX(chip8_t* chip8, const opcode_t opcode);
int assign_VX_to_VY(chip8_t* chip8, const opcode_t opcode);
int bitop_VX_to_VX_OR_VY(chip8_t* chip8, const opcode_t opcode);
int bitop_VX_to_VX_AND_VY(chip8_t* chip8, const opcode_t opcode);
int bitop_VX_to_VX_XOR_VY(chip8_t* chip8, const opcode_t opcode);
int math_add_VY_to_VX(chip8_t* chip8, const opcode_t opcode);
int math_subtract_VY_from_VX(chip8_t* chip8, const opcode_t opcode);
int bitop_VX_right_by_one(chip8_t* chip8, const opcode_t opcode);
int math_set_VX_to_VY_minus_VX(chip8_t* chip8, const opcode_t opcode);
int bitop_VX_left_by_one(chip8_t* chip8, const opcode_t opcode);
int condition_skip_if_VX_not_equal_VY(chip8_t* chip8, const opcode_t opcode);
int memory_set_index_register_to_NNN(chip8_t* chip8, const opcode_t opcode);
int flow_program_counter_to_VO_plus_NNN(chip8_t* chip8, const opcode_t opcode);
int random_VX_equals_rand_AND_NN(chip8_t* chip8, const opcode_t opcode);
int display_draw_sprite_at_VX_VY(chip8_t* chip8, const opcode_t opcode);
int keyop_skip_if_key_equals_VX(chip8_t* chip8, const opcode_t opcode);
int keyop_skip_if_key_not_equals_VX(chip8_t* chip8, const opcode_t opcode);
int timer_set_VX_to_delay_timer(chip8_t* chip8, const opcode_t opcode);
int keyop_await_key_press_into_VX(chip8_t* chip8, const opcode_t opcode);
int timer_set_delay_timer_to_VX(chip8_t* chip8, const opcode_t opcode);
int sound_set_sound_timer_to_VX(chip8_t* chip8, const opcode_t opcode);
int memory_add_VX_to_index_register(chip8_t* chip8, const opcode_t opcode);
int memory_set_index_register_to_location_of_sprite_in_VX(chip8_t* chip8, const opcode_t opcode);
int bcd_store_representation_of_VX(chip8_t* chip8, const opcode_t opcode);
int memory_dump_V0_to_VX_at_address_index_register(chip8_t* chip8, const opcode_t opcode);
int memory_load_V0_to_VX_from_address_index_register(chip8_t* chip8, const opcode_t opcode);

static const uint8_t DEFAULT_FONT[] = {
    0xF0, 0x90, 0x90, 0x90, 0xF0, // 0
    0x20, 0x60, 0x20, 0x20, 0x70, // 1
    0xF0, 0x10, 0xF0, 0x80, 0xF0, // 2
    0xF0, 0x10, 0xF0, 0x10, 0xF0, // 3
    0x90, 0x90, 0xF0, 0x10, 0x10, // 4
    0xF0, 0x80, 0xF0, 0x10, 0xF0, // 5
    0xF0, 0x80, 0xF0, 0x90, 0xF0, // 6
    0xF0, 0x10, 0x20, 0x40, 0x40, // 7
    0xF0, 0x90, 0xF0, 0x90, 0xF0, // 8
    0xF0, 0x90, 0xF0, 0x10, 0xF0, // 9
    0xF0, 0x90, 0xF0, 0x90, 0x90, // A
    0xE0, 0x90, 0xE0, 0x90, 0xE0, // B
    0xF0, 0x80, 0x80, 0x80, 0xF0, // C
    0xE0, 0x90, 0x90, 0x90, 0xE0, // D
    0xF0, 0x80, 0xF0, 0x80, 0xF0, // E
    0xF0, 0x80, 0xF0, 0x80, 0x80  // F
};
