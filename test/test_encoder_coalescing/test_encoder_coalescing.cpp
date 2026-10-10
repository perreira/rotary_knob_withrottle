#include <unity.h>

#include "encoder_accumulator.h"

void test_burst_clicks_coalesce_to_latest_direction() {
    int delta = 0;

    delta = encoder_accumulate(delta, true, false);  // CW
    delta = encoder_accumulate(delta, true, false);  // CW again
    delta = encoder_accumulate(delta, false, true);  // CCW, latest wins

    TEST_ASSERT_EQUAL_INT(-50, delta);
}

void test_parallel_burst_with_both_bits_cancels_to_zero() {
    int delta = 0;

    delta = encoder_accumulate(delta, true, false);
    delta = encoder_accumulate(delta, true, true);

    TEST_ASSERT_EQUAL_INT(0, delta);
}

void test_idle_state_keeps_zero() {
    TEST_ASSERT_EQUAL_INT(0, encoder_accumulate(0, false, false));
}

void setUp() {}
void tearDown() {}

int main(int, char**) {
    UNITY_BEGIN();
    RUN_TEST(test_burst_clicks_coalesce_to_latest_direction);
    RUN_TEST(test_parallel_burst_with_both_bits_cancels_to_zero);
    RUN_TEST(test_idle_state_keeps_zero);
    return UNITY_END();
}
