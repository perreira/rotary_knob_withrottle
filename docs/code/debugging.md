# Debugging and development workflow

This page captures the practical debugging and development workflow for the firmware. It is intended to help a developer move from a user-visible symptom to the correct code area quickly.

## 1. Typical workflow

When a problem appears, use this order:

1. Reproduce it on the actual device or with the relevant native test.
2. Identify which screen or subsystem owns the behavior.
3. Check the relevant screen module and the backing logic component.
4. Verify the state transitions in the screen manager.
5. Run the smallest relevant test or build step.

This keeps the investigation focused and avoids changing unrelated code.

## 2. Where to look by symptom

### Connection / Wi‑Fi issues

Check:
- [components/wifi](../../components/wifi)
- [components/netprov](../../components/netprov)
- [components/jmri_discovery](../../components/jmri_discovery)
- [components/ui_screens/screen_connecting.cpp](../../components/ui_screens/screen_connecting.cpp)

Typical symptoms:
- device never leaves the Connecting screen
- failed association or no JMRI server found
- unexpected reconfiguration loop

### Roster / recent list problems

Check:
- [components/ui_screens/screen_roster.cpp](../../components/ui_screens/screen_roster.cpp)
- [components/ui_screens/screen_recent.cpp](../../components/ui_screens/screen_recent.cpp)
- [components/withrottle_client](../../components/withrottle_client)
- [components/recents](../../components/recents)

Typical symptoms:
- list missing entries
- duplicate entries
- wrong loco selected after a click
- recents not persisting

### Throttle control problems

Check:
- [components/ui_screens/screen_throttle.cpp](../../components/ui_screens/screen_throttle.cpp)
- [components/withrottle_client](../../components/withrottle_client)
- [components/ui_screens/include/screen_mgr.h](../../components/ui_screens/include/screen_mgr.h)

Typical symptoms:
- knob movement does not change speed
- speed arc jumps back unexpectedly
- horn stays stuck on or off
- direction label mismatch

### Function list issues

Check:
- [components/ui_screens/screen_functions.cpp](../../components/ui_screens/screen_functions.cpp)
- [components/ui_screens/train_fn_screen.cpp](../../components/ui_screens/train_fn_screen.cpp)
- [components/withrottle_client](../../components/withrottle_client)

Typical symptoms:
- function rows missing
- wrong function name or wrong toggle state
- list focus jumps unexpectedly

### Settings / configuration issues

Check:
- [components/ui_screens/screen_settings.cpp](../../components/ui_screens/screen_settings.cpp)
- [components/param](../../components/param)
- [components/netprov](../../components/netprov)
- [src/secrets.h.example](../../src/secrets.h.example)

Typical symptoms:
- saved network values not showing
- provisioning not triggered when expected
- stale IP or SSID displayed

## 3. Debugging by architecture layer

### UI layer

Look at:
- [components/ui_screens](../../components/ui_screens)
- [components/ui](../../components/ui)
- [main/main.cpp](../../main/main.cpp)

Focus on:
- screen transitions
- event wiring
- object lifetime and object reuse
- LVGL lock timing

Important caution:
- Some screens intentionally reuse UI objects rather than deleting them on every entry. This avoids event corruption and avoids mid-event object destruction while the LVGL event loop is running.

### Network and protocol layer

Look at:
- [components/withrottle_client](../../components/withrottle_client)
- [components/jmri_discovery](../../components/jmri_discovery)
- [components/wifi](../../components/wifi)

Focus on:
- phase transitions
- server discovery and address selection
- TCP session state
- roster and function updates from JMRI

### Persistence layer

Look at:
- [components/param](../../components/param)
- [components/recents](../../components/recents)
- [src](../../src)

Focus on:
- parameter serialization
- NVS read/write success
- default override precedence

## 4. Useful developer checks

### Screen manager state

The screen manager is the central router. If a screen is acting strangely, verify:

- which screen is current
- whether or not `screen_mgr_show()` is being called unexpectedly
- whether a fallback to `CONNECTING` is happening due to `withr::get_phase()`

This is especially relevant when connection loss causes the app to reset to the connecting flow.

### LVGL timing

The app runs UI work under a timed LVGL lock. If a screen seems to freeze or update inconsistently, check:

- lock acquisition attempts in [main/main.cpp](../../main/main.cpp)
- screen `tick()` functions
- input events from the encoder

### Rebuild vs reuse

A recurring pattern in this app is to build list widgets once and reuse them across entries. If a list is not updating correctly, inspect whether the code is:

- repopulating on state change
- keeping focus in sync
- reusing already-created LVGL children safely

## 5. Native tests and logic validation

The project includes native tests for logic-heavy modules:

- [test/native](../../test/native)
- [components/loco_ref](../../components/loco_ref)
- [components/horn_resolver](../../components/horn_resolver)
- [components/recents](../../components/recents)
- [components/netprov](../../components/netprov)
- [components/jmri_discovery](../../components/jmri_discovery)

Use these for anything that is mostly pure logic, especially:

- roster filtering and parsing
- address conversion
- horn selection
- recent serialization
- validation rules for Wi-Fi and JMRI values

Run the native tests with the project script:

```bash
bash test/native/run.sh
```

or a single suite:

```bash
bash test/native/run.sh loco_ref
```

## 6. Build and verification loop

For firmware changes, use the normal project commands:

```bash
./build.sh build
./build.sh flash monitor
```

Then verify the real behavior on the device:

- connection state
- roster list population
- throttle response
- function state updates
- back/home transitions
- reconnect recovery

This is the minimum end-to-end validation for a UI or protocol change.

## 7. Root-cause debugging checklist

Use this checklist before making a fix:

- Does the issue appear in a native test or only on hardware?
- Is the bug caused by a bad screen transition?
- Is the app routing through the wrong screen state?
- Is the UI list being rebuilt too aggressively?
- Is the protocol state still READY when the screen expects it?
- Is the state being mutated from the wrong thread / event context?
- Is there an object lifetime issue in LVGL?

If the answer is not obvious, narrow the scope to one screen and one state transition first.

## 8. Good debugging habits for this project

- Prefer the smallest screen-level reproduction.
- Check `screen_mgr_show()` calls before changing logic.
- Inspect connection-phase transitions before modifying UI behavior.
- Use native Unity tests for pure logic changes.
- Validate UI updates against the actual sequence of events rather than assumptions about the network state.

## 9. Quick map of relevant files

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

This gives a practical route from a symptom to the most likely code path to inspect.
