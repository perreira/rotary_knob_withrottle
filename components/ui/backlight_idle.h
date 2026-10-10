#ifndef BACKLIGHT_IDLE_H
#define BACKLIGHT_IDLE_H

#include <cstdint>

enum class BacklightAction {
    NONE,
    TURN_ON,
    TURN_OFF,
};

// Call this state machine from one task; it does not perform hardware access.
class BacklightIdle {
public:
    static constexpr uint32_t DEFAULT_TIMEOUT_MS = 30000;

    explicit BacklightIdle(uint32_t timeout_ms = DEFAULT_TIMEOUT_MS);

    BacklightAction on_first_frame(uint32_t now_ms);
    BacklightAction on_activity(uint32_t now_ms);
    BacklightAction tick(uint32_t now_ms);

    bool is_ready() const;
    bool is_asleep() const;

private:
    enum class State {
        WAITING_FOR_FRAME,
        AWAKE,
        ASLEEP,
    };

    uint32_t timeout_ms_;
    uint32_t last_activity_ms_;
    State state_;
};

class BacklightTouchGate {
public:
    bool should_suppress(bool pressed, bool released, bool backlight_asleep);

private:
    bool suppress_until_release_ = false;
};

#endif