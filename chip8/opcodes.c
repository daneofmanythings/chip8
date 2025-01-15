#include "chip8.h"
#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>

#define X ((oc & 0x0F00) >> 8)
#define Y ((oc & 0x00F0) >> 4)
#define Vx (chip8->registers[X])
#define Vy (chip8->registers[Y])
#define VF (chip8->registers[0xF])
#define I (chip8->index_register)
#define NNN (oc & 0x0FFF)
#define NN (oc & 0x00FF)
#define N (oc & 0x000F)

opcode_t fetch_opcode(const chip8_t* chip8) {
  return chip8->memory[chip8->program_counter] << 8 | chip8->memory[chip8->program_counter + 1];
}

opcode_f decode_opcode(opcode oc) {
  switch (oc & 0xF000) {
  case 0x0000:
    switch (oc & 0x0FFF) {
    case 0x00E0:
      return clear_display;
    case 0x00EE:
      return flow_return;
    default:
      return call_machine_code_routine_at_NNN;
    }
  case 0x1000:
    return flow_goto_NNN;
  case 0x2000:
    return flow_call_subroutine_at_NNN;
  case 0x3000:
    return condition_skip_if_VX_equal_NN;
  case 0x4000:
    return condition_skip_if_VX_not_equal_NN;
  case 0x5000:
    return condition_skip_if_VX_equal_VY;
  case 0x6000:
    return constant_set_VX_to_NN;
  case 0x7000:
    return constant_add_NN_to_VX;
  case 0x8000:
    switch (oc & 0x000F) {
    case 0x0000:
      return assign_VX_to_VY;
    case 0x0001:
      return bitop_VX_to_VX_OR_VY;
    case 0x0002:
      return bitop_VX_to_VX_AND_VY;
    case 0x0003:
      return bitop_VX_to_VX_XOR_VY;
    case 0x0004:
      return math_add_VY_to_VX;
    case 0x0005:
      return math_subtract_VY_from_VX;
    case 0x0006:
      return bitop_VX_right_by_one;
    case 0x0007:
      return math_set_VX_to_VY_minus_VX;
    case 0x000E:
      return bitop_VX_left_by_one;
    default:
      fprintf(stderr, "Unknown opcode: 0x%04x\n", oc);
      exit(1);
    }
  case 0x9000:
    return condition_skip_if_VX_not_equal_VY;
  case 0xA000:
    return memory_set_index_register_to_NNN;
  case 0xB000:
    return flow_program_counter_to_VO_plus_NNN;
  case 0xC000:
    return random_VX_equals_rand_AND_NN;
  case 0xD000:
    return display_draw_sprite_at_VX_VY;
  case 0xE000:
    switch (oc & 0x00FF) {
    case 0x009E:
      return keyop_skip_if_key_equals_VX;
    case 0x00A1:
      return keyop_skip_if_key_not_equals_VX;
    }
  case 0xF000:
    switch (oc & 0x00FF) {
    case 0x0007:
      return timer_set_VX_to_delay_timer;
    case 0x000A:
      return keyop_await_key_press_into_VX;
    case 0x0015:
      return timer_set_delay_timer_to_VX;
    case 0x0018:
      return sound_set_sound_timer_to_VX;
    case 0x001E:
      return memory_add_VX_to_index_register;
    case 0x0029:
      return memory_set_index_register_to_location_of_sprite_in_VX;
    case 0x0033:
      return bcd_store_representation_of_VX;
    case 0x0055:
      return memory_dump_V0_to_VX_at_address_index_register;
    case 0x0065:
      return memory_load_V0_to_VX_from_address_index_register;
    }
  default:
    fprintf(stderr, "Unknown opcode: 0x%04x\n", oc);
    exit(1);
  }
}

// OPCODE FUNCTIONS
/*0NNN 	Call 		Calls machine code routine (RCA 1802 for COSMAC VIP) at
 * address NNN. Not necessary for most ROMs.[24]*/
int call_machine_code_routine_at_NNN(chip8_t* chip8, opcode oc) {
  fprintf(stderr, "0NNN not implemented\n");
  exit(1);
}

//*00E0 	Display 	disp_clear() 	Clears the screen.[24]*/
int clear_display(chip8_t* chip8, opcode oc) {
  for (size_t i = 0; i < SCREEN_SIZE; i++) {
    chip8->screen[i] = 0;
  }
  return 1;
}

/*00EE 	Flow 	return{} 	Returns from a subroutine.[24]*/
int flow_return(chip8_t* chip8, opcode oc) {
  chip8->program_counter = chip8_stack_pop(chip8);
  return 1;
}

/*1NNN 	Flow 	goto NNN{} 	Jumps to address NNN.[24]*/
int flow_goto_NNN(chip8_t* chip8, opcode oc) {
  chip8->program_counter = NNN;
  return 0;
}

/*2NNN 	Flow 	*(0xNNN)() 	Calls subroutine at NNN.[24]*/
int flow_call_subroutine_at_NNN(chip8_t* chip8, opcode oc) {
  chip8_stack_push(chip8, chip8->program_counter);
  chip8->program_counter = NNN;
  return 0;
}

/*3XNN 	Cond 	if (Vx == NN) 	Skips the next instruction if VX equals NN
 * (usually the next instruction is a jump to skip a code block).[24]*/
int condition_skip_if_VX_equal_NN(chip8_t* chip8, opcode oc) {
  if (Vx == NN) {
    return 2;
  }
  return 1;
}

/*4XNN 	Cond 	if (Vx != NN) 	Skips the next instruction if VX does not equal
 * NN (usually the next instruction is a jump to skip a code block).[24]*/
int condition_skip_if_VX_not_equal_NN(chip8_t* chip8, opcode oc) {
  if (Vx != NN) {
    return 2;
  }
  return 1;
}

/*5XY0 	Cond 	if (Vx == Vy) 	Skips the next instruction if VX equals VY
 * (usually the next instruction is a jump to skip a code block).[24]*/
int condition_skip_if_VX_equal_VY(chip8_t* chip8, opcode oc) {
  if (Vx == Vy) {
    return 2;
  }
  return 1;
}

/*6XNN 	Const 	Vx = NN 	Sets VX to NN.[24]*/
int constant_set_VX_to_NN(chip8_t* chip8, opcode oc) {
  Vx = NN;
  return 1;
}

/*7XNN 	Const 	Vx += NN 	Adds NN to VX (carry flag is not changed).[24]*/
int constant_add_NN_to_VX(chip8_t* chip8, opcode oc) {
  Vx += NN;
  return 1;
}

/*8XY0 	Assig 	Vx = Vy 	Sets VX to the value of VY.[24]*/
int assign_VX_to_VY(chip8_t* chip8, opcode oc) {
  Vx = Vy;
  return 1;
}

/*8XY1 	BitOp 	Vx |= Vy 	Sets VX to VX or VY. (bitwise OR operation).[24]*/
int bitop_VX_to_VX_OR_VY(chip8_t* chip8, opcode oc) {
  Vx |= Vy;
  return 1;
}

/*8XY2 	BitOp 	Vx &= Vy 	Sets VX to VX and VY. (bitwise AND operation).[24]*/
int bitop_VX_to_VX_AND_VY(chip8_t* chip8, opcode oc) {
  Vx &= Vy;
  return 1;
}

/*8XY3[a] 	BitOp 	Vx ^= Vy 	Sets VX to VX xor VY.[24]*/
int bitop_VX_to_VX_XOR_VY(chip8_t* chip8, opcode oc) {
  Vx ^= Vy;
  return 1;
}

/*8XY4 	Math 	Vx += Vy 	Adds VY to VX. VF is set to 1 when there's an overflow, and to 0 when there is
 * not.[24]*/
int math_add_VY_to_VX(chip8_t* chip8, opcode oc) {
  if (0xFF - Vx < Vy) {
    VF = 1;
  }
  Vx += Vy;
  return 1;
}

/*8XY5 	Math 	Vx -= Vy 	VY is subtracted from VX. VF is set to 0 when
 * there's an underflow, and 1 when there is not. (i.e. VF set to 1 if VX >= VY
 * and 0 if not).[24]*/
int math_subtract_VY_from_VX(chip8_t* chip8, opcode oc) {
  if (Vx >= Vy) {
    VF = 1;
  } else {
    VF = 0;
  }
  Vx -= Vy;
  return 1;
}

/*8XY6[a] 	BitOp 	Vx >>= 1 	Shifts VX to the right by 1, then stores
 * the least significant bit of VX prior to the shift into VF.[b][24]*/
int bitop_VX_right_by_one(chip8_t* chip8, opcode oc) {
  VF = Vx & 1;
  Vx >>= 1;
  return 1;
}

/*8XY7[a] 	Math 	Vx = Vy - Vx 	Sets VX to VY minus VX. VF is set to 0
 * when there's an underflow, and 1 when there is not. (i.e. VF set to 1 if VY
 * >= VX).[24]*/
int math_set_VX_to_VY_minus_VX(chip8_t* chip8, opcode oc) {
  if (Vy >= Vx) {
    VF = 1;
  } else {
    VF = 0;
  }
  Vx = Vy - Vx;
  return 1;
}

/*8XYE[a] 	BitOp 	Vx <<= 1 	Shifts VX to the left by 1, then sets VF
 * to 1 if the most significant bit of VX prior to that shift was set, or to 0
 * if it was unset.[b][24]*/
int bitop_VX_left_by_one(chip8_t* chip8, opcode oc) {
  VF = (Vx >> 7) & 1;
  Vx <<= 1;
  return 1;
}

/*9XY0 	Cond 	if (Vx != Vy) 	Skips the next instruction if VX does not equal
 * VY. (Usually the next instruction is a jump to skip a code block).[24]*/
int condition_skip_if_VX_not_equal_VY(chip8_t* chip8, opcode oc) {
  if (Vx != Vy) {
    return 2;
  }
  return 1;
}

/*ANNN 	MEM 	I = NNN 	Sets I to the address NNN.[24]*/
int memory_set_index_register_to_NNN(chip8_t* chip8, opcode oc) {
  I = NNN;
  return 1;
}

/*BNNN 	Flow 	PC = V0 + NNN 	Jumps to the address NNN plus V0.[24]*/
int flow_program_counter_to_VO_plus_NNN(chip8_t* chip8, opcode oc) {
  chip8->program_counter = chip8->registers[0] + NNN;
  return 0;
}

/*CXNN 	Rand 	Vx = rand() & NN 	Sets VX to the result of a bitwise and
 * operation on a random number (Typically: 0 to 255) and NN.[24]*/
int random_VX_equals_rand_AND_NN(chip8_t* chip8, opcode oc) {
  Vx = rand() & NN;
  return 1;
}

/*DXYN 	Display 	draw(Vx, Vy, N) 	Draws a sprite at coordinate
 * (VX, VY) that has a width of 8 pixels and a height of N pixels. Each row of 8
 * pixels is read as bit-coded starting from memory location I{} I value does
 * not change after the execution of this instruction. As described above, VF is
 * set to 1 if any screen pixels are flipped from set to unset when the sprite
 * is drawn, and to 0 if that does not happen.[24]*/
int display_draw_sprite_at_VX_VY(chip8_t* chip8, opcode oc) {
  uint8_t x = Vx;
  if (x >= SCREEN_WIDTH) { // bringing x back in range
    x %= SCREEN_WIDTH;
  }
  size_t max_cols = 8;
  if (max_cols + x >= SCREEN_WIDTH) { // adjusting for right clipping
    max_cols = SCREEN_WIDTH - x;
  }
  uint8_t y = Vy;
  if (y >= SCREEN_HEIGHT) { // bringing y back into range
    y %= SCREEN_HEIGHT;
  }
  size_t max_rows = N;
  if (max_rows + y >= SCREEN_HEIGHT) { // adjusting for bottom clipping
    max_rows = SCREEN_HEIGHT - y;
  }

  uint8_t row_draw_data;
  uint16_t row_start_loc;
  size_t draw_loc = 0;
  VF = 0;
  pthread_mutex_lock(&chip8->screen_mutex);
  for (size_t row = 0; row < max_rows; row++) { // iterating down through the rows
    row_draw_data = chip8->memory[I + row];
    row_start_loc = x + ((y + row) * SCREEN_WIDTH);
    for (size_t bit_num = 0; bit_num < max_cols; bit_num++) { // iterating through bits of the sprite
      draw_loc = row_start_loc + bit_num;

      if (draw_loc < 0 || draw_loc >= (SCREEN_SIZE)) {
        fprintf(stderr, "opcode 0x%04x tried to draw out of range at %zu\n", oc, draw_loc);
        exit(1);
      }
      bool* screen_bit_p = &chip8->screen[draw_loc];
      bool draw_bit = nth_bit(row_draw_data, bit_num);
      if (*screen_bit_p == 1 && draw_bit == 1) {
        VF = 1; // collision detection
      }
      *screen_bit_p ^= draw_bit;
    }
  }
  pthread_mutex_unlock(&chip8->screen_mutex);
  return 1;
}

/*EX9E 	KeyOp 	if (key() == Vx) 	Skips the next instruction if the key
 * stored in VX(only consider the lowest nibble) is pressed (usually the next
 * instruction is a jump to skip a code block).[24]*/
int keyop_skip_if_key_equals_VX(chip8_t* chip8, opcode oc) {
  pthread_mutex_lock(&chip8->keypress.mut);
  bool is_equal = (Vx & 0x0F) == ((chip8->keypress.value) & 0x0F);
  pthread_mutex_unlock(&chip8->keypress.mut);

  if (is_equal) {
    return 2;
  }
  return 1;
}

/*EXA1 	KeyOp 	if (key() != Vx) 	Skips the next instruction if the key
 * stored in VX(only consider the lowest nibble) is not pressed (usually the
 * next instruction is a jump to skip a code block).[24]*/
int keyop_skip_if_key_not_equals_VX(chip8_t* chip8, opcode oc) {
  pthread_mutex_lock(&chip8->keypress.mut);
  bool is_not_equal = (Vx & 0x0F) != ((chip8->keypress.value) & 0x0F);
  pthread_mutex_unlock(&chip8->keypress.mut);

  if (is_not_equal) {
    return 2;
  }
  return 1;
}

/*FX07 	Timer 	Vx = get_delay() 	Sets VX to the value of the delay
 * timer.[24]*/
int timer_set_VX_to_delay_timer(chip8_t* chip8, opcode oc) {
  Vx = chip8->delay_timer.value;
  return 1;
}

/*FX0A 	KeyOp 	Vx = get_key() 	A key press is awaited, and then stored in VX
 * (blocking operation, all instruction halted until next key event, delay and
 * sound timers should continue processing).[24]*/
int keyop_await_key_press_into_VX(chip8_t* chip8, opcode oc) {
  pthread_mutex_lock(&chip8->keypress.mut);
  pthread_cond_wait(&chip8->keypress.cond, &chip8->keypress.mut);

  Vx = chip8->keypress.value;

  pthread_mutex_unlock(&chip8->keypress.mut);
  return 1;
}

/*FX15 	Timer 	delay_timer(Vx) 	Sets the delay timer to VX.[24]*/
int timer_set_delay_timer_to_VX(chip8_t* chip8, opcode oc) {
  pthread_mutex_lock(&chip8->delay_timer.mut);
  chip8->delay_timer.value = Vx;
  pthread_mutex_unlock(&chip8->delay_timer.mut);

  pthread_t tid;
  struct timer_thread_args args = {
      &chip8->delay_timer,
      chip8->Hz,
  };
  if (!pthread_create(&tid, NULL, decrement_timer_thread, (void*)&args)) {
    //
  }
  return 1;
}

/*FX18 	Sound 	sound_timer(Vx) 	Sets the sound timer to VX.[24]*/
int sound_set_sound_timer_to_VX(chip8_t* chip8, opcode oc) {
  pthread_mutex_lock(&chip8->sound_timer.mut);
  chip8->sound_timer.value = Vx;
  pthread_mutex_unlock(&chip8->sound_timer.mut);

  pthread_t tid;
  struct timer_thread_args args = {
      &chip8->sound_timer,
      chip8->Hz,
  };
  if (!pthread_create(&tid, NULL, decrement_timer_thread, (void*)&args)) {
    //
  }
  return 1;
}

/*FX1E 	MEM 	I += Vx 	Adds VX to I. VF is not affected.[c][24]*/
int memory_add_VX_to_index_register(chip8_t* chip8, opcode oc) {
  I += Vx;
  return 1;
}

/*FX29 	MEM 	I = sprite_addr[Vx] 	Sets I to the location of the sprite for
 * the character in VX(only consider the lowest nibble). Characters 0-F (in
 * hexadecimal) are represented by a 4x5 font.[24]*/
int memory_set_index_register_to_location_of_sprite_in_VX(chip8_t* chip8, opcode oc) {
  uint8_t c = Vx & 0x0F;
  I = chip8->memory[FONT_START + c * 5];
  return 1;
}

/*FX33 	BCD 	*/
/**/
/*set_BCD(Vx)*/
/**(I+0) = BCD(3){}*/
/**(I+1) = BCD(2){}*/
/**(I+2) = BCD(1){}*/
/**/
/*	Stores the binary-coded decimal representation of VX, with the hundreds
 * digit in memory at location in I, the tens digit at location I+1, and the
 * ones digit at location I+2.[24]*/
int bcd_store_representation_of_VX(chip8_t* chip8, opcode oc) {
  chip8->memory[I] = Vx / 100;
  chip8->memory[I + 1] = (Vx % 100) / 10;
  chip8->memory[I + 2] = Vx % 10;
  return 1;
}

/*FX55 	MEM 	reg_dump(Vx, &I) 	Stores from V0 to VX (including VX) in
 * memory, starting at address I. The offset from I is increased by 1 for each
 * value written, but I itself is left unmodified.[d][24]*/
int memory_dump_V0_to_VX_at_address_index_register(chip8_t* chip8, opcode oc) {
  memcpy(&chip8->memory[I], &chip8->registers[0], X + 1);
  return 1;
}

/*FX65 	MEM 	reg_load(Vx, &I) 	Fills from V0 to VX (including VX) with
 * values from memory, starting at address I. The offset from I is increased by
 * 1 for each value read, but I itself is left unmodified.[d][24]*/
int memory_load_V0_to_VX_from_address_index_register(chip8_t* chip8, opcode oc) {
  memcpy(&chip8->registers[0], &chip8->memory[I], X + 1);
  return 1;
}

bool nth_bit(uint8_t byte, uint8_t bit_num) {
  if (bit_num < 0 || bit_num >= 8) {
    fprintf(stderr, "ERROR | tried to check a bit_num out of bounds: %d bit\n", bit_num);
    exit(1);
  }
  return (byte >> (7 - bit_num)) & 1;
}

uint16_t chip8_stack_pop(chip8_t* chip8) {
  if (chip8->stack.pointer == 0) {
    fprintf(stderr,
            "ERROR | chip8_t::stack_pop: attempted to return from non-existent subroutine (pop from an empty stack)\n");
    exit(1);
  }
  chip8->stack.pointer -= 1;
  return chip8->stack.data[chip8->stack.pointer];
}
void chip8_stack_push(chip8_t* chip8, uint16_t value) {
  if (chip8->stack.pointer == STACK_CAPACITY - 1) {
    fprintf(stderr, "ERROR | chip8_t::stack_push: attempted to push to a full stack (too many jump instructions).\n");
    exit(1);
  }
  chip8->stack.data[chip8->stack.pointer] = value;
  chip8->stack.pointer += 1;
}
