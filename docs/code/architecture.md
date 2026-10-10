# Architecture overview

This document describes the important software boundaries in the project and the way they interact at runtime.

## 1. App startup and lifecycle

The application enters through [main/main.cpp](../../main/main.cpp). The main function performs the initial hardware and service bootstrapping before entering the steady-state loop.

Important startup responsibilities:

- Console command initialization
- Wi-Fi initialization
- Battery measurement initialization
- WiThrottle client startup
- Screen and display initialization
- UI refresh loop and encoder input handling

The code intentionally stages the WiThrottle client before the display is fully built, because the available RAM is tight and the protocol client should be established before the UI and its buffers consume the remaining internal memory budget.

## 2. UI and display subsystem

The UI layer is a combination of:

- generated LVGL UI assets from the SquareLine Studio project under [sq_studio_prj](../../sq_studio_prj)
- display initialization code under the `ui` and `ui_screens` component groups
- screen manager logic that swaps between application states

The display subsystem is structured around a periodic LVGL lock. The UI task checks for first-frame and touch activity, updates the backlight policy, and then forwards encoder rotations to the active screen via `screen_mgr_on_rotate()`.

The screen manager is the key coordination point for the active UI state. It decides which screen is visible and handles transitions between:

- Home
- Roster
- Recent
- Throttle
- Functions
- Settings
- Connecting fallback states

## 3. Network and JMRI connectivity

The network layer is split into several pieces:

- [components/wifi](../../components/wifi): Wi-Fi station operation and persistence
- [components/jmri_discovery](../../components/jmri_discovery): service discovery for JMRI servers via mDNS and selection logic
- [components/netprov](../../components/netprov): setup / provisioning portal and validation logic
- [components/withrottle_client](../../components/withrottle_client): actual protocol client that talks to JMRI

This separation keeps logic testable. The discovery and validation code can be reasoned about without needing the full ESP32 runtime, and the actual connection logic remains contained in the client component.

## 4. Parameters, storage, and recovery

Runtime values are stored through the parameter and NVS system. The project uses:

- compile-time defaults in [src/secrets.h.example](../../src/secrets.h.example)
- runtime parameter overrides in the `param` subsystem
- NVS-backed configuration for Wi-Fi/JMRI values and recent locos

This is important because the firmware supports both provisioning-first operation and compile-time defaults. The runtime config overrides the compiled defaults when values are present, and the provisioning portal can write values to NVS so the device can operate without serial access.

## 5. Pure logic modules

Several parts of the codebase are intentionally written as pure logic modules with native unit tests. These modules are easier to validate off-device and make debugging easier.

- [components/loco_ref](../../components/loco_ref): locomotive address parsing and value handling
- [components/horn_resolver](../../components/horn_resolver): horn function selection logic
- [components/recents](../../components/recents): serialization and recents state management
- [components/netprov](../../components/netprov): validation and provisioning rules
- [components/jmri_discovery](../../components/jmri_discovery): selection logic for valid servers

These modules are the best place to add logic changes that are not UI-specific and should be validated through the native test suite.

## 6. Hardware interaction

The project wraps several hardware concerns in board-specific BSP modules:

- battery sensing
- encoder input
- backlight control
- display initialization
- touch handling

The hardware-specific logic is separated from the app state logic, which keeps the screen manager, throttle behavior, and protocol client independent of raw GPIO details.

## 7. OTA and operational resilience

The firmware includes an OTA server component and uses a partition layout designed for in-place updates. The project can be updated over Wi-Fi once the device is running, reducing the need for USB re-flashing in normal operation.

Operational resilience features include:

- automatic fallback to the connecting screen on network loss
- reconnect flow back into the normal app state
- NVS-backed recent-loco memory across power cycles
- configuration recovery from the provisioning portal when the stored state is stale or missing

## 8. Typical data flow

The main runtime data path looks like this:

1. Encoder events are read in the input task.
2. The UI task consumes the rotation and touch activity under the LVGL lock.
3. The screen manager updates the visible screen and dispatches user actions.
4. The screen or controller asks the WiThrottle client to change loco state.
5. The WiThrottle client sends requests to the JMRI server.
6. The returned state is reflected back into the UI and any relevant NVS state.

This loop keeps the UI, device inputs, and protocol operations synchronized while respecting the hardware constraints of the ESP32-S3 knob.

## 9. Where to change behavior

When adding or modifying functionality, the usual place depends on the concern:

- Screen or UI behavior: `ui_screens`, `ui`, and `main.cpp`
- Device/network behavior: `wifi`, `netprov`, `jmri_discovery`, `withrottle_client`
- Persistent configuration: `param`, `src/secrets.h`, NVS-backed storage
- Pure logic: the logic libraries in `components/*`
- Native validation: `test/native` and the corresponding component tests

## 10. Summary

The project is best understood as a small embedded app with a strict separation between hardware, view state, protocol I/O, and persistent settings. The code architecture favors testable logic modules, resilient runtime behavior, and a clear split between UI flow and network operations.
