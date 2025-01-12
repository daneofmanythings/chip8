#include <criterion/criterion.h>
#include <criterion/parameterized.h>
#include <stdio.h>
#include <string.h>

#include "chip8.h"

struct nth_bit_params {
  uint8_t byte;
  uint8_t n;
  bool expected;
};
ParameterizedTestParameters(nth_bit, tests) {
  static struct nth_bit_params params[] = {
      {.byte = 1,   .n = 7, .expected = 1},
      {.byte = 0,   .n = 7, .expected = 0},
      {.byte = 128, .n = 0, .expected = 1},
      {.byte = 128, .n = 3, .expected = 0},
  };
  size_t size = sizeof(params) / sizeof(struct nth_bit_params);
  return cr_make_param_array(struct nth_bit_params, params, size);
}
ParameterizedTest(struct nth_bit_params* param, nth_bit, tests) {
  bool result = nth_bit(param->byte, param->n);
  cr_assert_eq(result, param->expected);
}

struct get_opcode_test_case {
  uint8_t opcode[2];
  opcode_t expected;
};
ParameterizedTestParameters(opcode, get) {
  static struct get_opcode_test_case tcs[] = {
      {{0xFF, 0x11}, 0xFF11},
      {{0x12, 0x34}, 0x1234},
  };
  size_t size = sizeof(tcs) / sizeof(struct get_opcode_test_case);
  return cr_make_param_array(struct get_opcode_test_case, tcs, size);
}
ParameterizedTest(struct get_opcode_test_case* tc, opcode, get) {
  chip8_t chip8 = {0};
  memcpy(&chip8.memory[chip8.program_counter], tc->opcode, sizeof(tc->opcode));
  opcode_t result = get_opcode(&chip8);
  cr_assert(result == tc->expected, "input=[%d, %d], expected=%d, result=%d", tc->opcode[0], tc->opcode[1],
            tc->expected, result);
}

struct decode_opcode_params {
  opcode_t input;
  opcode_f expected;
};
ParameterizedTestParameters(decode_opcode, tests) {
  static struct decode_opcode_params params[] = {
      {.input = 0x0111, .expected = call_machine_code_routine_at_NNN                     },
      {.input = 0x00E1, .expected = call_machine_code_routine_at_NNN                     },
      {.input = 0x00E0, .expected = clear_display                                        },
      {.input = 0x00EE, .expected = flow_return                                          },
      {.input = 0x1120, .expected = flow_goto_NNN                                        },
      {.input = 0x2111, .expected = flow_call_subroutine_at_NNN                          },
      {.input = 0x3111, .expected = condition_skip_if_VX_equal_NN                        },
      {.input = 0x4164, .expected = condition_skip_if_VX_not_equal_NN                    },
      {.input = 0x5999, .expected = condition_skip_if_VX_equal_VY                        },
      {.input = 0x6666, .expected = constant_set_VX_to_NN                                },
      {.input = 0x7777, .expected = constant_add_NN_to_VX                                },
      {.input = 0x8880, .expected = assign_VX_to_VY                                      },
      {.input = 0x8881, .expected = bitop_VX_to_VX_OR_VY                                 },
      {.input = 0x8882, .expected = bitop_VX_to_VX_AND_VY                                },
      {.input = 0x8883, .expected = bitop_VX_to_VX_XOR_VY                                },
      {.input = 0x8884, .expected = math_add_VY_to_VX                                    },
      {.input = 0x8885, .expected = math_subtract_VY_from_VX                             },
      {.input = 0x8886, .expected = bitop_VX_right_by_one                                },
      {.input = 0x8887, .expected = math_set_VX_to_VY_minus_VX                           },
      {.input = 0x888E, .expected = bitop_VX_left_by_one                                 },
      {.input = 0x9999, .expected = condition_skip_if_VX_not_equal_VY                    },
      {.input = 0xAAAA, .expected = memory_set_index_register_to_NNN                     },
      {.input = 0xBAAA, .expected = flow_program_counter_to_VO_plus_NNN                  },
      {.input = 0xCAAA, .expected = random_VX_equals_rand_AND_NN                         },
      {.input = 0xDAAA, .expected = display_draw_sprite_at_VX_VY                         },
      {.input = 0xE09E, .expected = keyop_skip_if_key_equals_VX                          },
      {.input = 0xE0A1, .expected = keyop_skip_if_key_not_equals_VX                      },
      {.input = 0xF007, .expected = timer_set_VX_to_delay_timer                          },
      {.input = 0xF00A, .expected = keyop_await_key_press_into_VX                        },
      {.input = 0xF015, .expected = timer_set_delay_timer_to_VX                          },
      {.input = 0xF018, .expected = sound_set_sound_timer_to_VX                          },
      {.input = 0xF01E, .expected = memory_add_VX_to_index_register                      },
      {.input = 0xF029, .expected = memory_set_index_register_to_location_of_sprite_in_VX},
      {.input = 0xF033, .expected = bcd_store_representation_of_VX                       },
      {.input = 0xF055, .expected = memory_dump_V0_to_VX_at_address_index_register       },
      {.input = 0xF065, .expected = memory_load_V0_to_VX_from_address_index_register     },
  };
  size_t size = sizeof(params) / sizeof(struct decode_opcode_params);
  return cr_make_param_array(struct decode_opcode_params, params, size);
}
ParameterizedTest(struct decode_opcode_params* param, decode_opcode, tests) {
  opcode_f result = decode_opcode(param->input);
  cr_assert(result == param->expected, "input=0x%04x\n", param->input);
}
