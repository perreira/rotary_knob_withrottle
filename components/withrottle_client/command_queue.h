#pragma once

#include <algorithm>
#include <vector>

namespace withr {
namespace command_queue {

inline constexpr char kDefaultThrottle = 'T';

struct Command {
    enum Kind {
        Speed,
        Direction,
        EStop,
        Function,
        Acquire,
        Release,
    };

    Kind kind = Speed;
    int  arg  = 0;
    bool on   = false;
    int  address = 0;
    char length  = 'L';
    char throttle = kDefaultThrottle;
};

inline bool is_high_priority(Command::Kind kind)
{
    return kind == Command::EStop || kind == Command::Acquire || kind == Command::Release;
}

inline bool same_function_slot(const Command& a, const Command& b)
{
    return a.kind == Command::Function && b.kind == Command::Function &&
           a.arg == b.arg && a.throttle == b.throttle;
}

inline bool same_slot(const Command& a, const Command& b)
{
    if (a.kind != b.kind) return false;

    switch (a.kind) {
        case Command::Speed:
        case Command::Direction:
            return a.throttle == b.throttle;
        case Command::Function:
            return same_function_slot(a, b) && a.on == b.on;
        case Command::Acquire:
            return a.address == b.address && a.length == b.length;
        case Command::EStop:
        case Command::Release:
            return true;
    }
    return false;
}

inline void coalesce_add(std::vector<Command>& queue, const Command& incoming)
{
    // Keep the newest state for a given command slot; e.g. last speed wins and
    // later function updates replace earlier ones for the same function index.
    auto replace = std::find_if(queue.begin(), queue.end(), [&](const Command& item) {
        return same_slot(item, incoming);
    });
    if (replace != queue.end()) {
        *replace = incoming;
        return;
    }

    if (is_high_priority(incoming.kind)) {
        queue.erase(std::remove_if(queue.begin(), queue.end(), [](const Command& item) {
                        return !is_high_priority(item.kind) &&
                               !(item.kind == Command::Function && !item.on);
                    }),
                    queue.end());
    }

    // Keep the queue bounded. For bursts of speed/direction updates the newest
    // value wins; for full queues, we evict the oldest low-priority command
    // first and preserve the newest high-priority command.
    static constexpr size_t kMaxQueued = 8;
    while (queue.size() >= kMaxQueued) {
        auto evict = std::find_if(queue.begin(), queue.end(), [&](const Command& item) {
            return !(incoming.kind == Command::Function && same_function_slot(item, incoming) &&
                     item.on != incoming.on);
        });
        queue.erase(evict == queue.end() ? queue.begin() : evict);
    }

    queue.push_back(incoming);
}

} // namespace command_queue
} // namespace withr
