# Troubleshooting matrix

This matrix is a fast-reference guide for the most common symptoms seen while working with the WiThrottle dial firmware. It is meant to complement the general debugging guide and the screen-by-screen UI guide.

## Quick lookup

| Symptom | Likely cause | First place to inspect | Typical fix |
|---|---|---|---|
| Device never leaves Connecting | Wi-Fi association failure, no JMRI server, or startup state stuck | [components/ui_screens/screen_connecting.cpp](../../components/ui_screens/screen_connecting.cpp) | Check Wi‑Fi credentials, JMRI discovery, or provisioning fallback |
| Home screen is empty or buttons do not work | UI not wired correctly or screen was not loaded | [components/ui_screens/screen_home.cpp](../../components/ui_screens/screen_home.cpp) | Rebind event callbacks and verify `screen_mgr_show()` path |
| Roster list is empty | JMRI roster not loaded yet or state transition issue | [components/ui_screens/screen_roster.cpp](../../components/ui_screens/screen_roster.cpp) | Check roster loading in the WiThrottle client |
| Recent list is missing entries | NVS or recents serialization issue | [components/recents](../../components/recents) | Check recents load/push logic and value persistence |
| Knob rotation does not change speed | Input event not routed or speed arc values not mapped | [components/ui_screens/screen_throttle.cpp](../../components/ui_screens/screen_throttle.cpp) | Inspect encoder task and `on_rotate()` logic |
| Speed arc jumps backward or resets | stale value from JMRI readout while user is actively driving | [components/ui_screens/screen_throttle.cpp](../../components/ui_screens/screen_throttle.cpp) | Respect the recent-input time window before overwriting the arc |
| Horn button does not work | event wiring or wrong command state | [components/ui_screens/screen_throttle.cpp](../../components/ui_screens/screen_throttle.cpp) | Verify `LV_EVENT_PRESSED` / `RELEASED` behavior |
| Function screen shows wrong names or wrong states | function list not refreshed or stale labels | [components/ui_screens/screen_functions.cpp](../../components/ui_screens/screen_functions.cpp) | Check refresh timing and `TrainFnScreen::load_for_loco()` |
| Settings screen shows stale Wi‑Fi or IP text | update code not running or state not refreshed | [components/ui_screens/screen_settings.cpp](../../components/ui_screens/screen_settings.cpp) | Verify `tick()` update path and state values |
| Device boots into setup portal unexpectedly | missing credentials or repeated Wi‑Fi failure | [components/ui_screens/screen_connecting.cpp](../../components/ui_screens/screen_connecting.cpp) and [components/netprov](../../components/netprov) | Check Wi‑Fi config and recovery path |
| Device cannot reconnect after network loss | phase fallback or watchdog logic not restoring session | [components/withrottle_client](../../components/withrottle_client) | Verify reconnect flow and READY-phase restoration |
| App freezes or LVGL stalls | UI lock deadlock or object lifetime issue | [main/main.cpp](../../main/main.cpp) and [components/ui_screens](../../components/ui_screens) | Check LVGL lock acquisition and avoid deleting objects mid-event |

## 1. Connection and session problems

### Device does not connect to Wi‑Fi

Check these in order:

1. [components/wifi](../../components/wifi)
2. [components/netprov](../../components/netprov)
3. [src/secrets.h.example](../../src/secrets.h.example)
4. [components/ui_screens/screen_connecting.cpp](../../components/ui_screens/screen_connecting.cpp)

Common issues:
- no SSID or password saved
- stale bad credentials in NVS
- the app is waiting for the network but the access point is unavailable

### Device connects to Wi‑Fi but not to JMRI

Check:

- [components/jmri_discovery](../../components/jmri_discovery)
- [components/withrottle_client](../../components/withrottle_client)
- [components/ui_screens/screen_connecting.cpp](../../components/ui_screens/screen_connecting.cpp)

Likely causes:
- wrong JMRI host or port
- mDNS server discovery failure
- server selection logic returning the wrong candidate

## 2. UI and event problems

### Buttons do nothing

Check:
- the event callback binding in the relevant screen file
- whether the screen event is reattached after entry
- whether the generated button callback was removed and replaced

Most UI input paths rely on rebinding in the screen `enter()` handler. This is deliberate in this app.

### Screen is visually correct but input is wrong

Focus on:
- `screen_mgr_show()` routing
- encoder rotation handlers
- the active `ScreenModule` for the current screen

The screen manager decides which object owns the current interaction. If the wrong screen is active, a button or knob may appear to “do nothing” even though the logic is present.

## 3. State and persistence problems

### Recents do not survive reboot

Check:
- [components/recents](../../components/recents)
- [components/param](../../components/param)
- any place where NVS values are written or cleared

Likely causes:
- serialization mismatch
- key collision or missing save call
- expired or truncated data

### Settings are stale

Check:
- screen update logic in [components/ui_screens/screen_settings.cpp](../../components/ui_screens/screen_settings.cpp)
- runtime parameter store in [components/param](../../components/param)
- any provisioning save flow in [components/netprov](../../components/netprov)

A screen may show stale values even when the settings are correct in NVS if the UI label update is not refreshed.

## 4. Protocol and data flow problems

### Roster entries or locomotive state seem inconsistent

Check:
- [components/withrottle_client](../../components/withrottle_client)
- [components/ui_screens/screen_roster.cpp](../../components/ui_screens/screen_roster.cpp)
- [components/ui_screens/screen_throttle.cpp](../../components/ui_screens/screen_throttle.cpp)

Look for:
- timing between roster loading and screen entry
- stale loco state after a reconnect
- function state initialization before JMRI sync completes

### Function toggles are delayed or incorrect

Check:
- `TrainFnScreen::load_for_loco()` usage in [components/ui_screens/screen_functions.cpp](../../components/ui_screens/screen_functions.cpp)
- the protocol client’s function update handling in [components/withrottle_client](../../components/withrottle_client)

This is usually a state synchronization issue rather than a UI-only issue.

## 5. LVGL and object lifetime issues

This project reuses LVGL objects intentionally in several places. If you see crashes or “LoadProhibited” style symptoms, suspect one of these patterns:

- deleting list entries during an event callback
- re-creating UI objects while an event is still dispatching
- rebuilding large widget trees during screen entry rather than in the UI tick loop

The app explicitly avoids some of these patterns in comment blocks across the screen code. If a crash is new, compare the affected screen to the others that avoid mid-event mutation.

## 6. Practical debugging order

When a symptom appears, use this order:

1. Confirm the active screen in the screen manager
2. Check whether the user input is reaching the expected handler
3. Check whether the associated protocol state is READY
4. Inspect the backing component that owns the data that should update
5. Run the smallest native test or build/flash cycle to validate the fix

This order tends to isolate the root cause quickly and keeps changes minimal.

## 7. Common “gotcha” patterns in this project

- The app often updates list widgets only when their content changes, not every frame.
- Several screens rebuild their UI once and then reuse it rather than deleting it repeatedly.
- The throttle screen intentionally avoids overwriting arc values immediately after the user changes the speed.
- A loss of Wi‑Fi or JMRI session triggers a fallback to `CONNECTING` rather than a full reboot.

## 8. File map for troubleshooting

- [main/main.cpp](../../main/main.cpp)
- [components/ui_screens/include/screen_mgr.h](../../components/ui_screens/include/screen_mgr.h)
- [components/ui_screens/screen_connecting.cpp](../../components/ui_screens/screen_connecting.cpp)
- [components/ui_screens/screen_home.cpp](../../components/ui_screens/screen_home.cpp)
- [components/ui_screens/screen_roster.cpp](../../components/ui_screens/screen_roster.cpp)
- [components/ui_screens/screen_recent.cpp](../../components/ui_screens/screen_recent.cpp)
- [components/ui_screens/screen_throttle.cpp](../../components/ui_screens/screen_throttle.cpp)
- [components/ui_screens/screen_functions.cpp](../../components/ui_screens/screen_functions.cpp)
- [components/ui_screens/screen_settings.cpp](../../components/ui_screens/screen_settings.cpp)
- [components/withrottle_client](../../components/withrottle_client)
- [components/wifi](../../components/wifi)
- [components/netprov](../../components/netprov)
- [components/recents](../../components/recents)
- [components/jmri_discovery](../../components/jmri_discovery)

This is the fastest route from a runtime symptom to the code that owns that behavior.
