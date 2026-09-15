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
- `footswitch_config_screen.svg` - Footswitch configuration (FS1/FS2 mode and actions)
- `preset_manager_screen.svg` - Preset browser with LOAD/SAVE/COPY operations

These mockups are reference implementations. The actual LVGL-based UI should match these designs closely.

---

## Implementation Status

### Round 1 Complete ✅

**Widgets Created:**
- `VuMeter` - Vertical VU meter with peak hold indicator
- `EffectCard` - Compact effect status card for chain/performance views

**Theme System:**
- `UiThemeColor` enum for programmatic color access
- `UiTheme_get_color()` helper function
- Added colors: `COLOR_ACCENT_GREEN`, `COLOR_DISABLED`, `COLOR_BORDER`

**Screens Mocked:**
- Performance Screen (main view)
- FX Chain Screen (effect flow)
- Effect Edit Screen (parameter editor)
- Footswitch Config Screen (NEW - FS1/FS2 setup)
- Preset Manager Screen (NEW - preset browser)

### Next Round TODO

**Screens to Implement:**
1. `FootswitchConfigScreen` - Configure FS1/FS2 mode and actions
2. `PresetManagerScreen` - Browse, load, save presets
3. `SystemScreen` - Diagnostics, link status, telemetry

**Protocol Integration:**
- VoxLink UART communication layer
- Frame parser (SOF, CRC16, sequence numbers)
- State synchronization on boot

**Hardware Drivers:**
- Footswitch GPIO driver with debounce
- UART driver for VoxLink protocol
