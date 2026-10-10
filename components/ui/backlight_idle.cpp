#include "backlight_idle.h"

BacklightIdle::BacklightIdle(uint32_t timeout_ms)
    : timeout_ms_(timeout_ms), last_activity_ms_(0), state_(State::WAITING_FOR_FRAME)
{
}

BacklightAction BacklightIdle::on_first_frame(uint32_t now_ms)
{
    if (state_ != State::WAITING_FOR_FRAME) {
        return BacklightAction::NONE;
    }

    last_activity_ms_ = now_ms;
    state_ = State::AWAKE;
    return BacklightAction::TURN_ON;
}

BacklightAction BacklightIdle::on_activity(uint32_t now_ms)
{
    if (state_ == State::WAITING_FOR_FRAME) {
        return BacklightAction::NONE;
    }

    last_activity_ms_ = now_ms;
    if (state_ == State::ASLEEP) {
        state_ = State::AWAKE;
        return BacklightAction::TURN_ON;
    }

    return BacklightAction::NONE;
}

BacklightAction BacklightIdle::tick(uint32_t now_ms)
{
    // Unsigned subtraction keeps elapsed-time checks correct across tick wrap.
    if (state_ != State::AWAKE || (now_ms - last_activity_ms_) < timeout_ms_) {
        return BacklightAction::NONE;
    }

    state_ = State::ASLEEP;
    return BacklightAction::TURN_OFF;
}

bool BacklightIdle::is_ready() const
{
    return state_ != State::WAITING_FOR_FRAME;
}

bool BacklightIdle::is_asleep() const
{
    return state_ == State::ASLEEP;
}

bool BacklightTouchGate::should_suppress(bool pressed, bool released, bool backlight_asleep)
{
    if (pressed && backlight_asleep) {
        suppress_until_release_ = true;
    }

    const bool suppress = suppress_until_release_;
    if (released) {
        suppress_until_release_ = false;
    }
    return suppress;
}