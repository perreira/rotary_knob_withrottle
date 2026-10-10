#include <unity.h>

#include <vector>

#include "command_queue.h"

using withr::command_queue::Command;
using withr::command_queue::coalesce_add;

void test_default_command_uses_default_throttle() {
    Command command;

    TEST_ASSERT_EQUAL_INT('T', command.throttle);
}

void test_speed_coalesces_to_latest_value() {
    std::vector<Command> queue;

    coalesce_add(queue, {Command::Speed, 10, false, 0, 'L', '0'});
    coalesce_add(queue, {Command::Speed, 25, false, 0, 'L', '0'});
    coalesce_add(queue, {Command::Speed, 40, false, 0, 'L', '0'});

    TEST_ASSERT_EQUAL_INT(1, (int) queue.size());
    TEST_ASSERT_EQUAL_INT(40, queue[0].arg);
}

void test_function_updates_do_not_merge_different_functions() {
    std::vector<Command> queue;

    coalesce_add(queue, {Command::Function, 0, true, 0, 'L', '0'});
    coalesce_add(queue, {Command::Function, 1, true, 0, 'L', '0'});

    TEST_ASSERT_EQUAL_INT(2, (int) queue.size());
    TEST_ASSERT_EQUAL_INT(0, queue[0].arg);
    TEST_ASSERT_EQUAL_INT(1, queue[1].arg);
}

void test_function_press_and_release_edges_are_preserved() {
    std::vector<Command> queue;

    coalesce_add(queue, {Command::Function, 2, true, 0, 'L', '0'});
    for (int i = 0; i < 7; ++i) {
        coalesce_add(queue, {Command::Speed, i, false, 0, 'L', static_cast<char>('1' + i)});
    }
    coalesce_add(queue, {Command::Function, 2, false, 0, 'L', '0'});

    TEST_ASSERT_EQUAL_INT(8, (int) queue.size());
    int function_commands = 0;
    bool saw_press = false;
    bool saw_release = false;
    for (const Command& command : queue) {
        if (command.kind != Command::Function || command.arg != 2) continue;
        ++function_commands;
        saw_press = saw_press || command.on;
        saw_release = saw_release || !command.on;
    }
    TEST_ASSERT_EQUAL_INT(2, function_commands);
    TEST_ASSERT_TRUE(saw_press);
    TEST_ASSERT_TRUE(saw_release);
}

void test_estop_stays_priority_when_queue_is_full() {
    std::vector<Command> queue;

    for (int i = 0; i < 8; ++i) {
        coalesce_add(queue, {Command::Speed, i, false, 0, 'L', '0'});
    }
    coalesce_add(queue, {Command::EStop, 0, false, 0, 'L', '0'});

    TEST_ASSERT_EQUAL_INT(1, (int) queue.size());
    TEST_ASSERT_EQUAL((int) Command::EStop, (int) queue.back().kind);
}

void setUp() {}
void tearDown() {}

int main(int, char**) {
    UNITY_BEGIN();
    RUN_TEST(test_default_command_uses_default_throttle);
    RUN_TEST(test_speed_coalesces_to_latest_value);
    RUN_TEST(test_function_updates_do_not_merge_different_functions);
    RUN_TEST(test_function_press_and_release_edges_are_preserved);
    RUN_TEST(test_estop_stays_priority_when_queue_is_full);
    return UNITY_END();
}
