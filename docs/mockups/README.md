# VoxP4 CYD Control Surface - Interface Mockups

This directory contains SVG mockups of the user interface screens for the VoxP4 CYD control surface.

## Screens

### 1. Performance Screen (`performance_screen.svg`)

The main screen displayed during live performance. Shows:

- **Header**: Preset name and link status indicator
- **Meters**: Input and output level meters with dB values
- **Pitch Display**: Detected pitch, frequency, and voiced/unvoiced state
- **Effect Buttons**: Quick toggle for Harmony, Reverb, Limiter, Delay
- **Footswitch Labels**: Current assignment for FS1 and FS2
- **Navigation Bar**: Quick access to other screens

**Key Features:**
- Dark theme optimized for stage visibility
- High contrast colors (green = ON/active, gray = OFF)
- Large touch targets (minimum 40×40px)
- Link status clearly visible (green ● = connected, red ○ = disconnected)

---

### 2. FX Chain Screen (`fx_chain_screen.svg`)

Visual representation of the effect processing chain. Shows:

- **Flow Diagram**: INPUT → PITCH → HARMONY → DYNAMICS → REVERB → OUTPUT
- **Effect Status**: Green blocks = enabled, Gray = disabled
- **Parameter Summary**: Key parameter values shown on each block
- **Scroll Indicator**: Arrow shows more effects below

**Interaction:**
- Tap any effect block to open its editor
- Tap ON/OFF area to toggle effect

---

### 3. Effect Edit Screen (`effect_edit_screen.svg`)

Detailed parameter editing for individual effects. Shows:

- **Header**: Back button, effect name, global ON/OFF toggle
- **Sliders**: Horizontal sliders for continuous parameters
- **Value Display**: Large numeric readout
- **Segmented Buttons**: For discrete choices (L/C/R pan)
- **Page Indicator**: Shows current page when multiple pages exist

**Design Principles:**
- Avoid small knobs; prefer sliders
- Show current value prominently
- Use segmented buttons for enums
- Support pagination for complex effects

---

## Color Palette

| Color | Hex | Usage |
|-------|-----|-------|
| Background Dark | `#121212` | Main background |
| Card Background | `#1E1E2E` | Cards, containers |
| Header | `#2A2A3A` | Header bars |
| Text Primary | `#FFFFFF` | Main text |
| Text Secondary | `#B0B0B0` | Labels |
| Text Muted | `#707070` | Inactive states |
| Effect ON | `#4CAF50` | Active effects |
| Effect OFF | `#424242` | Disabled effects |
| Link OK | `#66BB6A` | Connected |
| Link Lost | `#EF5350` | Disconnected |
| Accent | `#64B5F6` | Slider knobs, highlights |
| Warning | `#FFA726` | Footswitch pressed |

---

## Layout Grid

The 320×240 display is organized as:

```
┌─────────────────────────────────┐
│ HEADER (32px)                   │
├─────────────────────────────────┤
│ METERS (60px)                   │
├─────────────────────────────────┤
│ PITCH (32px)                    │
├─────────────────────────────────┤
│ EFFECTS (56px)                  │
├─────────────────────────────────┤
│ FOOTSWITCH (32px)               │
├─────────────────────────────────┤
│ NAVIGATION (28px)               │
└─────────────────────────────────┘
```

Total: 240px vertical

---

## Touch Target Sizes

- **Buttons**: 70×50px minimum
- **Sliders**: 272×12px track, 12×12px knob
- **Toggle switches**: 40×20px
- **Navigation tabs**: ~80px width each

All targets exceed the 40×40px minimum recommended for resistive touchscreens.

---

## Files

- `performance_screen.svg` - Main performance view
- `fx_chain_screen.svg` - Effect chain visualization
- `effect_edit_screen.svg` - Parameter editor
- `footswitch_screen.svg` - Footswitch configuration (FS1/FS2 mode and actions)
- `presets_screen.svg` - Preset browser with LOAD/SAVE operations
- `system_screen.svg` - System diagnostics and telemetry
- `boot_screen.svg` - Boot/loading screen with initialization status

These mockups are reference implementations. The actual LVGL-based UI should match these designs closely.

---

## Implementation Status

### Round 4 Complete ✅

**Core Firmware:**
- `src/main.cpp` - Entry point with setup/loop
- `platformio.ini` - Build configuration for ESP32-CYD
- `.github/workflows/ci.yml` - GitHub Actions CI/CD pipeline

**Hardware Drivers:**
- `board/CYD_Config.h` - Pin definitions and hardware config
- `board/LGFX_CYD.h` - LovyanGFX display/touch driver

**UI Framework:**
- `ui/UiApp.cpp/h` - LVGL initialization, state management
- `ui/UiTheme.cpp/h` - Theme colors and styles
- `ui/screens/PerformanceScreen.cpp/h` - Main performance screen
- `ui/screens/FootswitchScreen.cpp/h` - FS1/FS2 configuration
- `ui/screens/PresetsScreen.cpp/h` - Preset management
- `ui/widgets/VuMeter.cpp/h` - VU meter widget
- `ui/widgets/EffectCard.cpp/h` - Effect card widget

**Control Layer:**
- `control/FootswitchManager.cpp/h` - Footswitch polling, debounce, actions

**Mockups Created:**
- Performance Screen (main view with meters, pitch, effects)
- FX Chain Screen (effect flow diagram)
- Effect Edit Screen (parameter editor)
- Footswitch Screen (FS1/FS2 mode and action config)
- Presets Screen (preset list with LOAD/SAVE)
- System Screen (diagnostics, telemetry, link status)
- Boot Screen (initialization progress)

### Next Round TODO

**Protocol Integration:**
- VoxLink UART communication layer
- Frame parser (SOF, CRC16, sequence numbers)
- State synchronization on boot
- Message handlers (HELLO, CAPS, STATE_SNAPSHOT, etc.)

**Additional Screens:**
- Complete navigation between all screens
- Touch event handlers for interactive elements

**Testing:**
- Hardware bring-up validation
- Unit tests for protocol parser
- Integration tests for state sync
