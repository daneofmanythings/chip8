#include "test.h"
#include <criterion/criterion.h>

Test(sanity, check) {
  cr_assert_eq(true, true);
}

Test(test_true, pass) {
  cr_assert_eq(test_true(), true);
}
