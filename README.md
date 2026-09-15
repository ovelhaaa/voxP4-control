# AGENTS.md — VoxP4 Control Surface

## Project purpose

`VoxP4-control` is the firmware for the external control surface of the VoxP4 vocal effects processor.

The control surface runs on an **ESP32 Cheap Yellow Display (CYD), 2-USB variant**, with:

* 320×240 landscape touchscreen UI
* ST7789 display controller
* XPT2046 resistive touchscreen
* two external footswitches
* bidirectional UART communication with the ESP32-P4 running VoxP4

The ESP32-CYD is a **controller and user interface only**.

It must not perform VoxP4 audio DSP.

The ESP32-P4 remains responsible for:

* audio input/output
* pitch detection
* harmonizer
* dry alignment
* dynamics
* limiter
* reverb
* other audio effects
* presets
* parameter authority
* DSP telemetry

The CYD is responsible for:

* graphical user interface
* touchscreen input
* footswitch input
* parameter editing
* preset selection
* effect enable/disable
* telemetry visualization
* VoxP4 communication

---

# 1. Architectural principles

The most important architectural rule is:

> The ESP32-P4 is the authoritative source of VoxP4 state.

The CYD may optimistically update controls during interaction, but it must eventually reconcile its local UI state with the state reported by the P4.

Expected data flow:

```text
User
  ↓
Touch / Footswitch
  ↓
CYD control event
  ↓
VoxLink command
  ↓
ESP32-P4
  ↓
Parameter manager
  ↓
DSP target state
  ↓
State confirmation / telemetry
  ↓
CYD
  ↓
UI
```

Never couple the UI directly to assumptions about DSP implementation details.

---

# 2. Hardware target

Primary target:

```text
ESP32-2432S028R
Cheap Yellow Display
2-USB variant
ST7789
XPT2046
```

The hardware configuration is based on the known-working configuration from:

```text
github.com/ovelhaaa/beatCYDa
```

Do not silently replace the board configuration with pinouts copied from generic one-USB CYD examples.

The common single-USB CYD frequently uses an ILI9341.

This project targets the **two-USB ST7789 variant**.

---

# 3. Display configuration

Display:

```text
Controller: ST7789
Logical resolution: 320 × 240
Physical panel: 240 × 320
Orientation: landscape
```

Pin mapping:

```text
TFT SCLK       GPIO14
TFT MISO       GPIO12
TFT MOSI       GPIO13
TFT CS         GPIO15
TFT DC         GPIO2
TFT RESET      -1
TFT BACKLIGHT  GPIO21
```

Initial SPI configuration:

```text
SPI mode:       0
write clock:    40 MHz
read clock:     16 MHz
DMA:            enabled
```

The existing BeatCYDa configuration should be considered the hardware reference when resolving display issues.

---

# 4. Touchscreen configuration

Touch controller:

```text
XPT2046
resistive
single-touch
```

Pins:

```text
Touch SCLK  GPIO25
Touch MISO  GPIO39
Touch MOSI  GPIO32
Touch CS    GPIO33
Touch IRQ   unused
```

Current calibration reference:

```text
X min = 240
X max = 3800

Y min = 3700
Y max = 200
```

Touch rotation offset:

```text
2
```

The existing implementation uses software SPI for the touch controller to avoid peripheral conflicts.

Do not change this casually.

A future calibration screen may replace fixed calibration constants with values stored in NVS.

---

# 5. Footswitch configuration

The controller has two physical footswitches.

Default GPIO assignment:

```text
Footswitch 1: GPIO22
Footswitch 2: GPIO27
```

Electrical behavior:

```text
INPUT_PULLUP
active LOW
switch closes to GND
```

The physical switches should normally be momentary.

Momentary versus latching behavior is implemented in software.

Expected semantic events:

```text
PRESS
RELEASE
LONG_PRESS
DOUBLE_PRESS
```

Initial timing:

```text
poll interval:        2–5 ms
debounce:             15 ms
long press:           500 ms
double press window:  300 ms
```

Never make footswitch responsiveness depend on the LVGL frame rate.

Footswitch scanning must remain independent of UI rendering.

---

# 6. UART communication with VoxP4

Default VoxLink UART:

```text
TX: GPIO18
RX: GPIO19
```

These pins are reused from the CYD SD-card interface.

Therefore the initial VoxP4 controller design assumes:

```text
SD CARD DISABLED
```

Do not initialize the SD card while GPIO18/GPIO19 are being used for VoxLink.

Default serial configuration:

```text
baud:      921600
fallback:  460800
format:    8N1
logic:     3.3 V TTL
duplex:    full duplex
```

Keep UART0 available for:

* firmware upload
* serial monitor
* debugging

Do not use UART0 as the normal VoxP4 control link unless there is a strong documented reason.

---

# 7. VoxLink protocol

The communication protocol between CYD and VoxP4 is called:

```text
VoxLink
```

Initial protocol version:

```text
VoxLink v1
```

The runtime control protocol should be binary.

Do not use JSON for continuous parameter updates or telemetry.

JSON may be used for:

* development tools
* debugging
* offline files

but not for the primary real-time control transport.

---

# 8. VoxLink frame

Recommended frame layout:

```text
SOF       0xA5 0x5A
VERSION   uint8
TYPE      uint8
FLAGS     uint8
SEQ       uint16 little-endian
LENGTH    uint16 little-endian
PAYLOAD   variable
CRC16     uint16 little-endian
```

Requirements:

* explicit framing
* explicit length
* protocol version
* sequence number
* CRC validation
* bounded payload
* parser recovery after malformed packets
* no dynamic allocation per frame

Recommended initial maximum payload:

```text
512 bytes
```

---

# 9. Minimum message set

Initial message types should include concepts equivalent to:

```text
HELLO
HELLO_ACK

CAPS_REQUEST
CAPS_RESPONSE

GET_STATE
STATE_SNAPSHOT

SET_PARAM
PARAM_CHANGED

ACTION
ACTION_RESULT

PRESET_LIST_REQUEST
PRESET_LIST
PRESET_LOAD
PRESET_SAVE
PRESET_CHANGED

METER_FRAME
PITCH_FRAME
DSP_STATUS

FOOTSWITCH_CONFIG
FOOTSWITCH_STATE

HEARTBEAT
ACK
NACK
ERROR
```

Exact numeric IDs must live in shared protocol definitions.

Do not duplicate magic numbers throughout the codebase.

---

# 10. Parameter IDs

VoxP4 parameters should use stable numeric IDs.

Example categories:

```text
0x01xx Harmony
0x02xx Reverb
0x03xx Dynamics / Limiter
0x04xx Delay
0x05xx Global
```

Example:

```text
0x0100 Harmony Enabled
0x0101 Harmony Interval
0x0102 Harmony Gain
0x0103 Harmony Pan

0x0200 Reverb Enabled
0x0201 Reverb Mix
0x0202 Reverb Decay

0x0300 Limiter Enabled
0x0301 Limiter Threshold
```

Prefer one canonical protocol definition that can be shared or generated for both repositories.

Avoid maintaining independent hand-written parameter maps in the CYD and P4 repositories.

---

# 11. State synchronization

The P4 is authoritative.

Correct behavior:

```text
CYD:
SET_PARAM harmony_gain = -9 dB

P4:
validate
apply target
update authoritative state

P4:
PARAM_CHANGED harmony_gain = -9 dB

CYD:
reconcile UI state
```

The UI may display the proposed value immediately for responsiveness, but it must accept the P4 response as authoritative.

This matters because parameters may also change through:

* MIDI
* presets
* automation
* another controller
* internal P4 logic

---

# 12. Boot handshake

Expected startup sequence:

```text
CYD boot
  ↓
HELLO
  ↓
HELLO_ACK
  ↓
CAPS_REQUEST
  ↓
CAPS_RESPONSE
  ↓
GET_STATE
  ↓
STATE_SNAPSHOT
  ↓
UI READY
```

Until the first valid state snapshot is received:

* do not assume DSP default values
* show a connecting state
* disable controls that require an active P4 link where appropriate

---

# 13. Reconnection behavior

The UI must survive:

* P4 reboot
* CYD reboot
* UART disconnect
* malformed packets
* temporary communication loss

Recommended heartbeat:

```text
500 ms
```

Consider link lost after approximately:

```text
1500–2000 ms
```

On reconnect:

```text
HELLO
CAPS
GET_STATE
STATE_SNAPSHOT
```

Discard stale local assumptions and converge to P4 state.

---

# 14. UI framework

The intended UI framework is:

```text
LVGL
```

Display/touch hardware integration should use the existing proven LovyanGFX configuration where practical.

Preferred stack:

```text
LVGL
  ↓
display flush callback
  ↓
LovyanGFX
  ↓
ST7789
```

Touch:

```text
XPT2046
  ↓
LovyanGFX / touch adapter
  ↓
LVGL input device
```

Do not rewrite low-level display initialization unless required.

---

# 15. LVGL memory strategy

The ESP32-CYD should not use two full-screen RGB565 framebuffers.

A full frame is approximately:

```text
320 × 240 × 2 bytes
= 153600 bytes
```

Prefer partial draw buffers.

Initial target:

```text
320 × 20 RGB565 pixels
```

per buffer.

Approximate memory:

```text
12800 bytes per buffer
25600 bytes for double buffering
```

Use DMA where available.

---

# 16. UI performance targets

Suggested rates:

```text
LVGL handler:      every 5–10 ms
visual target:     up to ~30 FPS
touch:             ~60 Hz
footswitch scan:   200–500 Hz
meters:            20–30 Hz
pitch display:     10–20 Hz
DSP status:        2–5 Hz
heartbeat:         2 Hz
```

Do not redraw the whole screen for meter updates.

Invalidate only changed regions/widgets.

---

# 17. UI design constraints

This is a stage/performance device.

Optimize for:

* visibility
* low interaction depth
* large touch targets
* fast feedback
* reliable operation

Recommended minimum touch target:

```text
40 × 40 px
```

Prefer:

```text
44–48 px
```

Avoid:

* tiny knobs
* dense menus
* gesture-heavy navigation
* multi-touch assumptions
* unnecessary animation
* full-screen transparency
* blur
* large dynamic shadows
* high-rate waveform rendering
* high-rate FFT/spectrogram rendering

The XPT2046 is a resistive single-touch controller.

Design around:

```text
tap
press
hold
horizontal drag
```

---

# 18. Main screens

The initial UI should provide:

```text
PERFORMANCE
FX CHAIN
EFFECT EDIT
FOOTSWITCH
PRESETS
SYSTEM
```

The primary navigation may expose fewer top-level items, for example:

```text
PERF | FX | PRESET | SET
```

Detailed effect editors can be opened by touching an effect card.

---

# 19. Performance screen

The performance screen is the default live screen.

It should show at minimum:

* preset name
* P4 link state
* input level
* output level
* pitch
* voiced/unvoiced state
* main effect states
* footswitch assignments
* footswitch pressed state

The current preset must always be easy to identify.

---

# 20. FX screen

The effect chain screen should present VoxP4 modules as cards or blocks.

Example:

```text
INPUT
  ↓
PITCH
  ↓
HARMONY
  ↓
DYNAMICS
  ↓
REVERB
  ↓
LIMITER
  ↓
OUTPUT
```

The exact chain must reflect the actual VoxP4 architecture.

Do not invent DSP modules in the UI that do not exist in the P4 firmware.

Capabilities reported by the P4 should determine which modules are displayed.

---

# 21. Effect editors

Prefer:

* horizontal sliders
* large numeric values
* large toggles
* segmented enum buttons
* plus/minus buttons where useful

Avoid packing many rotary knobs into 320×240.

Each parameter should have:

* parameter ID
* label
* unit
* minimum
* maximum
* default if applicable
* formatting rule
* control type

When possible, metadata should come from a shared description rather than being duplicated manually.

---

# 22. Footswitch mapping

Each footswitch should support configurable behavior.

Initial action set can include:

```text
Effect Toggle
Effect Momentary
Global Bypass
Harmony Toggle
Harmony Momentary
Reverb Toggle
Reverb Freeze
Delay Toggle
Tap Tempo
Preset Next
Preset Previous
Scene Next
Scene Previous
Mute Harmony
Custom Action
```

The system should be extensible without requiring footswitch code changes for every new effect.

Prefer action IDs and dispatch tables over large nested `if` statements.

---

# 23. Telemetry

Expected P4 → CYD telemetry includes:

### Audio meters

```text
input peak
input RMS
harmony peak
harmony RMS
output peak
output RMS
limiter gain reduction
```

Recommended rate:

```text
20–30 Hz
```

### Pitch

```text
frequency Hz
MIDI note float
confidence
voiced
detected note
```

Recommended rate:

```text
10–20 Hz
```

### DSP status

```text
CPU load
audio buffer load
underruns
overruns
sample rate
block size
free heap
optional temperature
```

Recommended rate:

```text
2–5 Hz
```

Telemetry must never be generated in a way that endangers real-time audio.

---

# 24. FreeRTOS/task architecture

Conceptual CYD layout:

```text
Core 0
├─ VoxLink RX
├─ VoxLink TX
├─ protocol parser
├─ state synchronization
└─ footswitch manager

Core 1
└─ LVGL/UI
```

This division is a starting point, not an immutable rule.

Important constraints:

* only one context should directly manipulate LVGL
* UART receive must not block UI rendering
* UI callbacks must not block on UART
* footswitch handling must not wait for LVGL
* avoid unnecessary cross-core mutex contention

---

# 25. UI thread safety

Do not manipulate LVGL widgets directly from:

* UART RX task
* footswitch task
* timer callback
* ISR

Instead:

```text
worker task
   ↓
event / queue
   ↓
UI task
   ↓
LVGL widget update
```

Use fixed-size event structures where practical.

---

# 26. Transport rules

UI code should never perform blocking writes directly.

Preferred flow:

```text
UI event
  ↓
ControlEvent
  ↓
TX queue
  ↓
VoxLink writer
```

UART receive:

```text
UART
  ↓
RX buffer
  ↓
frame parser
  ↓
validated message
  ↓
state manager/event bus
```

---

# 27. Real-time audio protection

The P4 must continue processing audio if the CYD:

* crashes
* disconnects
* reboots
* sends malformed traffic

The control surface is never part of the real-time audio dependency chain.

On the P4 side, never implement:

```text
UART callback
  ↓
long mutex
  ↓
audio thread
```

or:

```text
UART callback
  ↓
heavy DSP processing
```

Control messages should update targets or enqueue control events.

Audio processing consumes prepared state.

---

# 28. Parameter smoothing

The CYD sends target values.

The P4 is responsible for audio-safe smoothing.

Example:

```cpp
target_mix = new_value;
```

Then in DSP:

```cpp
current_mix += alpha * (target_mix - current_mix);
```

Do not implement audio smoothing in the CYD.

---

# 29. Slider traffic

Touch sliders may generate many events.

Implement coalescing/rate limiting.

Recommended behavior:

* send first value immediately
* send during drag at bounded rate
* always send final value on release

Initial bound:

```text
50–100 parameter updates/s maximum per active control
```

Do not enqueue thousands of stale slider positions.

If the TX queue contains obsolete updates for the same parameter, newer values may replace older pending values where safe.

---

# 30. Local persistence

CYD local NVS may store UI-specific data such as:

```text
display brightness
touch calibration
UI preferences
last page
telemetry rate
```

By default, full VoxP4 presets belong to the P4.

Avoid maintaining two independent preset databases.

---

# 31. Code quality rules

Prefer:

* clear ownership
* small modules
* explicit state
* fixed-size buffers
* enum classes
* strongly typed message structures
* RAII where appropriate
* deterministic memory behavior

Avoid:

* hidden global mutable state
* `delay()` in runtime control paths
* unbounded dynamic allocation
* large String concatenations in loops
* blocking UI callbacks
* giant switch statements where tables are clearer
* duplicated protocol constants
* undocumented GPIO changes

---

# 32. Memory rules

Avoid heap churn in recurring paths.

Especially avoid repeated allocation in:

* LVGL refresh
* UART frame parsing
* telemetry
* footswitch handling
* slider events

Use:

* static buffers
* bounded queues
* preallocated messages
* fixed-capacity strings where appropriate

Dynamic allocation during initialization is acceptable where libraries require it.

---

# 33. Error handling

Never silently ignore protocol errors.

Maintain counters for:

```text
CRC failures
frame errors
oversize packets
unknown message types
invalid parameter IDs
RX overflow
TX queue overflow
reconnect count
```

Expose useful diagnostics on the SYSTEM screen.

Do not flood serial logs for recurring recoverable errors.

---

# 34. Logging

Suggested levels:

```text
ERROR
WARN
INFO
DEBUG
TRACE
```

Production default:

```text
INFO
```

or:

```text
WARN
```

Do not log every meter packet or every touch movement at normal verbosity.

---

# 35. Build environment

Initial project environment should remain close to the proven BeatCYDa environment:

```ini
platform = espressif32@6.5.0
board = esp32dev
framework = arduino
monitor_speed = 115200
upload_speed = 460800

board_build.flash_mode = dio
board_build.f_flash = 40000000L
```

Important libraries:

```text
LovyanGFX
LVGL
```

Pin dependency versions.

Do not casually upgrade:

* ESP32 Arduino core
* PlatformIO platform
* LovyanGFX
* LVGL major version

without testing hardware behavior.

---

# 36. CPU configuration

Run the ESP32 at:

```text
240 MHz
```

Avoid light sleep during active control-surface operation unless thoroughly tested.

Responsiveness is more important than minimizing controller power consumption.

---

# 37. Suggested repository layout

```text
VoxP4-control/
├─ AGENTS.md
├─ README.md
├─ platformio.ini
├─ docs/
│  ├─ architecture.md
│  ├─ hardware.md
│  ├─ protocol.md
│  └─ ui.md
├─ src/
│  ├─ main.cpp
│  ├─ board/
│  │  ├─ CYD_Config.h
│  │  └─ LGFX_CYD.h
│  ├─ ui/
│  │  ├─ LvglDisplay.cpp
│  │  ├─ LvglTouch.cpp
│  │  ├─ UiApp.cpp
│  │  ├─ UiTheme.cpp
│  │  ├─ screens/
│  │  └─ widgets/
│  ├─ protocol/
│  │  ├─ VoxLink.cpp
│  │  ├─ VoxLinkParser.cpp
│  │  ├─ VoxLinkMessages.h
│  │  ├─ VoxLinkCrc.cpp
│  │  └─ VoxP4ParamIds.h
│  ├─ control/
│  │  ├─ ControlManager.cpp
│  │  ├─ FootswitchManager.cpp
│  │  └─ StateStore.cpp
│  ├─ transport/
│  │  ├─ VoxUart.cpp
│  │  └─ TxQueue.cpp
│  └─ storage/
│     └─ LocalSettings.cpp
└─ test/
```

Structure may evolve, but preserve separation between:

```text
board
UI
transport
protocol
state/control
storage
```

---

# 38. Testing expectations

Changes should be tested at the smallest relevant level.

At minimum consider:

### Protocol

Test:

* valid frame
* bad CRC
* invalid length
* truncated frame
* garbage before SOF
* multiple frames
* recovery after corruption
* unknown message
* maximum payload

### Footswitches

Test:

* clean press
* contact bounce
* release
* long press
* double press
* rapid repeated press

### State synchronization

Test:

* initial snapshot
* parameter update from CYD
* parameter update originating from P4
* reconnect
* P4 reboot
* CYD reboot

### UI

Test:

* display startup
* touch corners
* drag
* page navigation
* meter updates during interaction
* link lost state
* reconnect state

---

# 39. Hardware bring-up order

Do not implement the whole application before validating hardware.

Recommended milestones:

### Milestone 1

```text
ST7789 display
```

### Milestone 2

```text
XPT2046 touch
```

### Milestone 3

```text
LVGL basic screen
```

### Milestone 4

```text
FS1 + FS2
```

### Milestone 5

```text
UART loopback
```

### Milestone 6

```text
VoxLink parser
```

### Milestone 7

```text
P4 HELLO + state snapshot
```

### Milestone 8

```text
effect controls
```

### Milestone 9

```text
meters + pitch telemetry
```

### Milestone 10

```text
presets + complete live UI
```

Do not skip bring-up validation when a lower-level subsystem is not known to work.

---

# 40. Initial acceptance criteria

The first complete usable revision should satisfy:

```text
[ ] ST7789 works in 320×240 landscape
[ ] XPT2046 touch is calibrated
[ ] LVGL is stable
[ ] UI feels responsive
[ ] FS1 has reliable debounce
[ ] FS2 has reliable debounce
[ ] footswitch momentary mode works
[ ] footswitch latching mode works
[ ] VoxLink UART is stable
[ ] automatic reconnect works
[ ] state snapshot works
[ ] P4 remains authoritative
[ ] Harmony can be controlled
[ ] Reverb can be controlled
[ ] footswitch mappings can be edited
[ ] input/output meters work
[ ] pitch/voicing display works
[ ] current preset is visible
[ ] P4 audio continues normally without CYD
[ ] UI activity does not cause P4 audio underruns
```

---

# 41. Things explicitly out of scope for v1

Do not add these unless requested:

```text
Wi-Fi
BLE
ESP-NOW
audio processing on CYD
audio streaming
FFT visualization
spectrogram
continuous waveform
SD-based preset database
P4 firmware update through CYD
desktop editor
multi-touch UI
```

Avoid expanding project scope before the basic wired controller is reliable.

---

# 42. Source of hardware truth

When board-specific questions arise, prefer evidence from the working `beatCYDa` implementation over generic CYD tutorials.

Reference repository:

```text
https://github.com/ovelhaaa/beatCYDa
```

Important reference files:

```text
src/CYD_Config.h
src/ui/LGFX_CYD.h
src/main.cpp
platformio.ini
README.md
touch_e_sd/exemplo_config.txt
```

Do not assume all ESP32-2432S028R revisions are electrically identical.

---

# 43. Documentation rules

When changing any of these, update documentation in the same change:

* GPIO assignment
* UART configuration
* protocol frame
* message IDs
* parameter IDs
* footswitch behavior
* state authority rules
* supported hardware revision
* build environment
* dependencies

Protocol changes must document backward compatibility implications.

---

# 44. Agent behavior

When modifying this repository:

1. Inspect existing implementation before proposing structural rewrites.
2. Preserve working board configuration unless there is evidence it is wrong.
3. Prefer incremental changes over large speculative rewrites.
4. Do not invent VoxP4 DSP parameters; inspect the P4 implementation/specification when parameter definitions matter.
5. Avoid touching unrelated working code.
6. Compile after meaningful implementation changes.
7. Add tests when parser/state logic is changed.
8. Treat warnings, queue overflows, CRC errors and UART framing problems as engineering issues, not cosmetic noise.
9. Keep the P4 independent from the CYD.
10. Never trade audio reliability for UI convenience.

---

# 45. Change reporting

For significant changes, report:

```text
What changed
Why it changed
Files changed
Hardware assumptions
Protocol impact
Tests/build performed
Known limitations
Next recommended validation
```

When hardware testing is required but cannot be performed locally, state that explicitly.

Do not claim hardware validation based only on successful compilation.

---

# 46. Core design rule

When choosing between:

```text
more UI sophistication
```

and:

```text
simpler, deterministic and reliable live performance behavior
```

choose reliability.

The control surface should feel like a musical instrument controller, not a general-purpose application.
