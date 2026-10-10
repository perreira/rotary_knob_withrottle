# Changelog

## 1.2.1 - 2026-10-10

### Fixed
- Restored the default WiThrottle throttle ID (`T`) for queued speed, direction, stop, and function commands.
- Preserved distinct function press and release events when the bounded command queue is full.
- Kept direction and function button state synchronized with local requests and JMRI updates.
- Added periodic JMRI `qV` speed queries while a locomotive is acquired, so the dial reflects speed changes made outside the dial.
- Hardened Wi-Fi credential copying and empty-credential handling.

### Improved
- Coalesced rapid encoder and command updates while retaining high-priority stop and locomotive-selection commands.
- Cached the sorted roster snapshot and reused LVGL list widgets to reduce UI work and avoid unsafe widget deletion.
- Updated the build wrapper to use the active ESP-IDF installation.
- Expanded developer, screen, and troubleshooting documentation.
