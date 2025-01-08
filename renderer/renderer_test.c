#include <criterion/criterion.h>
#include <criterion/internal/new_asserts.h>
#include <criterion/new/assert.h>
#include <criterion/parameterized.h>

#include "renderer.h"

struct renderer_create_test_cases {
  size_t input_width;
  size_t input_height;
  size_t expected_pixel_w;
  size_t expected_pixel_h;
};
ParameterizedTestParameters(renderer, create) {
  static struct renderer_create_test_cases tcs[] = {
      {800, 400, 12, 12},
      {400, 400, 6,  6 },
      {800, 200, 6,  6 },
  };
  size_t size = sizeof(tcs) / sizeof(struct renderer_create_test_cases);
  return cr_make_param_array(struct renderer_create_test_cases, tcs, size);
}
ParameterizedTest(struct renderer_create_test_cases* tc, renderer, create) {
  renderer_t* renderer = renderer_create(tc->input_width, tc->input_height);
  cr_assert(eq(sz, tc->expected_pixel_w, renderer->pixel.w));
  cr_assert(eq(sz, tc->expected_pixel_h, renderer->pixel.h));
  renderer_destroy(renderer);
}
