#include "unity.h"
#include "backlight_idle.h"

void test_timeout_starts_after_first_frame()
{
    BacklightIdle idle(100);

    TEST_ASSERT_FALSE(idle.is_ready());
    TEST_ASSERT_EQUAL(BacklightAction::NONE, idle.tick(1000));
    TEST_ASSERT_EQUAL(BacklightAction::TURN_ON, idle.on_first_frame(1000));
    TEST_ASSERT_EQUAL(BacklightAction::NONE, idle.tick(1099));
    TEST_ASSERT_EQUAL(BacklightAction::TURN_OFF, idle.tick(1100));
    TEST_ASSERT_TRUE(idle.is_asleep());
}

void test_activity_wakes_and_restarts_timeout()
{
    BacklightIdle idle(100);
    idle.on_first_frame(1000);
    idle.tick(1100);

    TEST_ASSERT_EQUAL(BacklightAction::TURN_ON, idle.on_activity(1200));
    TEST_ASSERT_FALSE(idle.is_asleep());
    TEST_ASSERT_EQUAL(BacklightAction::NONE, idle.tick(1299));
    TEST_ASSERT_EQUAL(BacklightAction::TURN_OFF, idle.tick(1300));
}

void test_activity_while_awake_resets_timeout()
{
    BacklightIdle idle(100);
    idle.on_first_frame(1000);

    TEST_ASSERT_EQUAL(BacklightAction::NONE, idle.on_activity(1090));
    TEST_ASSERT_EQUAL(BacklightAction::NONE, idle.tick(1189));
    TEST_ASSERT_EQUAL(BacklightAction::TURN_OFF, idle.tick(1190));
}

void test_elapsed_time_wraps_safely()
{
    BacklightIdle idle(64);
    idle.on_first_frame(UINT32_C(0xFFFFFFF0));

    TEST_ASSERT_EQUAL(BacklightAction::NONE, idle.tick(UINT32_C(0x0000002F)));
    TEST_ASSERT_EQUAL(BacklightAction::TURN_OFF, idle.tick(UINT32_C(0x00000030)));
}

void test_first_frame_is_processed_only_once()
{
    BacklightIdle idle(100);

    TEST_ASSERT_EQUAL(BacklightAction::TURN_ON, idle.on_first_frame(50));
    TEST_ASSERT_EQUAL(BacklightAction::NONE, idle.on_first_frame(500));
    TEST_ASSERT_EQUAL(BacklightAction::NONE, idle.tick(149));
}

void test_touch_wake_is_suppressed_until_release()
{
    BacklightTouchGate gate;

    TEST_ASSERT_TRUE(gate.should_suppress(true, false, true));
    TEST_ASSERT_TRUE(gate.should_suppress(true, false, false));
    TEST_ASSERT_TRUE(gate.should_suppress(false, true, false));
    TEST_ASSERT_FALSE(gate.should_suppress(true, false, false));
}

int main()
{
    UNITY_BEGIN();
    RUN_TEST(test_timeout_starts_after_first_frame);
    RUN_TEST(test_activity_wakes_and_restarts_timeout);
    RUN_TEST(test_activity_while_awake_resets_timeout);
    RUN_TEST(test_elapsed_time_wraps_safely);
    RUN_TEST(test_first_frame_is_processed_only_once);
    RUN_TEST(test_touch_wake_is_suppressed_until_release);
    return UNITY_END();
}