# M5 — Data-driven effect editor and real parameter model

Milestone goal: turn the Effect Edit screen from a static mock into a real,
reusable, data-driven editor for HARMONY, REVERB, DELAY and LIMITER, with a
parameter model ready to be connected to the ESP32-P4 over VoxLink **without
redesigning the UI**. VoxLink/UART is still not implemented here.

## Architecture

```
widget  --UiAction{SetParameter,id,value}-->  ui_emit_action
   |                                              |
   |                                              v
   |                                   ui_update_parameter(id,value)
   |                                              |
   |                            clamp via descriptor (UiParamModel)
   |                                              |
   |                          +-------------------+-------------------+
   |                          v                                       v
   |                 UiApp parameter state               effect_edit_notify
   |                 (indexed by UiParamId)              (update/open row)
   |                          |
   |                          v
   |                 ui_build_effect_summary(effect)
   |                          |
   |             +------------+-------------+
   |             v                          v
   |      Performance card value      FX Chain value + metadata
   v
 (value label updated immediately by the widget callback)
```

No screen is an authority. `UiAppState.parameterValues[]` is the single
authority for values, and `ui_update_parameter()` is the single logical change.

## UiParamId

Logical, controller-local IDs. They are deliberately **not** the backend's
`VocalFxParameter` ordinals. Each descriptor also stores `voxlinkId`, the
intended mapping to the ESP32-P4 VoxLink parameter ID, for the future transport
layer. No voxP4 header is included by the controller.

```text
Harmony:  HarmonyMode, HarmonyInterval, HarmonyDegree, HarmonyKey,
          HarmonyScale, HarmonyNonScalePolicy, HarmonyVoiceLeading,
          HarmonyLevel, HarmonyPan, HarmonySmoothingMs, HarmonyAttackMs,
          HarmonyReleaseMs, FormantEnabled, FormantAmount
Reverb:   ReverbWet, ReverbDecaySeconds, ReverbDamping
Delay:    DelayLeftMs, DelayRightMs, DelayFeedback, DelayWet, DelayDry,
          DelayFeedbackLowpassHz
Limiter:  LimiterThresholdDb
```

Effect enable is still the consolidated M4 state (`UiAppState.*Enabled`), not a
UiParamId, to avoid duplicating enable authority.

## Parameter state

```cpp
float UiAppState::parameterValues[kUiParamCount]; // indexed by UiParamId
bool  UiAppState::parameterValid[kUiParamCount];

float ui_get_parameter(UiParamId);
bool  ui_parameter_is_valid(UiParamId);
void  ui_update_parameter(UiParamId, float);
```

Values are numeric. Display strings are produced by formatters and never stored.

## Descriptors

`static const` tables in `src/ui/params/UiParamModel.{h,cpp}` (no heap, no
`std::vector`/`std::string`, no LVGL):

```cpp
struct UiParamDescriptor {
    UiParamId id;
    const char* label;
    const char* section;       // section header grouping
    UiControlType type;        // Slider | Toggle | Segmented | Stepper
    UiValueFormat format;      // Percent | ... | EnumLabel
    float minValue, maxValue, step, defaultValue;
    const char* const* options; // enum labels
    uint8_t optionCount;
    uint16_t voxlinkId;         // future mapping
    uint8_t harmonyModeMask;    // which harmony modes show this row
    bool wraparound;            // stepper wrap (KEY/SCALE)
};
```

The editor rebuilds only its scrollable body on `effect_edit_load_effect()` or a
MODE change (`lv_obj_clean(params_area)`); header/back/enable persist. Value
changes never rebuild the screen.

## Controls

* **Slider** — labelled row + formatted value + orange slider (int index mapped
  from `(value-min)/step`).
* **Toggle** — ON/OFF button (used for Formant mode, Voice Leading).
* **Segmented** — 2–3 options (MODE, NON-SCALE).
* **Stepper** — `[-] value [+]` for large enums (KEY, SCALE, 12 values, wrap).

## Ranges and defaults (origin)

Ranges/defaults come from the ESP32-P4 VoxLink registry
(`components/voxlink/src/voxlink_registry.cpp`), which mirrors the engine's own
clamps and is the future mapping target. The spec's approximate harmony defaults
(interval +4, gain 0.5, pan -0.25) match an internal `HarmonyEngine` initializer
that production `vocal_fx_init` overrides; the real boot values equal the
registry defaults, which is what the controller uses.

| Parameter | Range | Step | Default |
|---|---|---|---|
| HarmonyMode | 0..2 | 1 | 0 (FIXED) |
| HarmonyInterval | -12..12 st | 1 | 0 |
| HarmonyDegree | -7..7 deg | 1 | 0 |
| HarmonyKey | 0..11 | 1 | 0 (C) |
| HarmonyScale | 0..11 | 1 | 0 (MAJOR) |
| HarmonyNonScalePolicy | 0..2 | 1 | 0 (NEAREST) |
| HarmonyVoiceLeading | 0..1 | 1 | 0 |
| HarmonyLevel | 0..1 | 0.01 | 1.0 |
| HarmonyPan | -1..1 | 0.01 | 0 |
| HarmonySmoothingMs | 1..500 | 1 | 30 |
| FormantEnabled | 0..1 | 1 | 0 |
| FormantAmount | 0..1 | 0.01 | 1.0 |
| HarmonyAttackMs | 0.1..100 | 0.1 | 4 |
| HarmonyReleaseMs | 1..500 | 1 | 20 |
| ReverbWet | 0..1 | 0.01 | 0.18 |
| ReverbDecaySeconds | 0.15..20 | 0.05 | 2.0 |
| ReverbDamping | 0..1 | 0.01 | 0.45 |
| DelayLeftMs | 1..2000 | 1 | 250 |
| DelayRightMs | 1..2000 | 1 | 375 |
| DelayFeedback | -0.95..0.95 | 0.01 | 0.25 |
| DelayFeedbackLowpassHz | 200..20000 | 10 | 6000 |
| DelayWet | 0..1 | 0.01 | 0.20 |
| DelayDry | 0..1 | 0.01 | 1.0 |
| LimiterThresholdDb | -24..0 | 0.5 | -3.0 |

Notes:
* Delay feedback uses the DSP's real signed range (-0.95..0.95). The current
  VoxLink registry clamps to 0..0.95; this discrepancy is documented for the
  future mapping decision.
* Delay time uses 1..2000 (DSP minimum of one sample); the registry uses 0..2000.

## Harmony modes (FIXED / DIATONIC / MIDI)

* **FIXED** — INTERVAL only (plus shared VOICE / FORMANT / DYNAMICS).
* **DIATONIC** — DEGREE, KEY, SCALE, NON-SCALE, VOICE LEAD (plus shared).
* **MIDI** — shared voice/formant/dynamics only; target comes from MIDI/VoxLink
  later. No virtual keyboard.
* Shared voice controls stay in every mode: LEVEL, PAN, SMOOTHING.

DEGREE uses -7..+7, conservative and consistent with the backend's degree-based
transposition (the backend itself does not clamp degree; the UI bounds it).

## Single harmony voice

The backend declares `MAX_HARMONY_VOICES == 1`. The editor has a single `VOICE`
section (no `VOICE 1`), and no second-voice controls. Legacy Voice2 parameters
are ignored.

## LIMITER semantics (explicit)

The `LIMITER` card/edit represents the **Harmony bus limiter**
(`HarmonyLimiterEnabled` / `HarmonyLimiterThresholdDb`), not the master limiter:

* header ON/OFF = harmony limiter enable (via the consolidated effect enable);
* THRESHOLD = harmony limiter threshold (dB);
* card metadata = `HARM BUS`.

There is no `EnableMasterLimiter` in the backend, so the UI never pretends the
master limiter can be disabled. `LimiterCeiling` (master) is intentionally not
exposed in this milestone.

## Deliberately not exposed

| Backend parameter | Why not exposed |
|---|---|
| Formant shift semitones | Enum exists but `apply_parameter()` has no real path and no public setter. |
| Master `LimiterCeiling` | Mixing master ceiling with the harmony limiter would be misleading; reserved for a future Master/Advanced screen. |
| Non-scale min/max MIDI, dry alignment, gate/compressor | Not part of the four editor effects in this milestone. |
| Reverb algorithm/type | The backend has a single FDN reverb; `PLATE`/`HALL`/`ROOM` do not exist. |
| Delay tempo sync | The DSP works in milliseconds; no BPM/division parameter exists (`1/4`, `1/8`, ... removed). |
| `KEY AUTO` | There is no automatic key detection; the key is explicit. |

## Summaries

Both Performance and FX Chain derive from the same `ui_build_effect_summary()`:

```text
HARMONY FIXED     main "+4 st"      meta "FIXED"
HARMONY DIATONIC  main "+2 deg"     meta "D DORIAN"
HARMONY MIDI      main "MIDI"       meta "CHORD"
REVERB            main "18%"        meta "2.0 s"
DELAY             main "250 ms"     meta "R 375"
LIMITER           main "-3.0 dB"    meta "HARM BUS"
```

Changing a value recalculates only the owning effect's summary; other screens
are not rebuilt.

## Local simulation vs P4 authority

Until VoxLink exists, `ui_update_parameter()` applies edits **optimistically and
locally**. When the transport arrives, the flow becomes:

```text
CYD edit -> SetParameter intent -> VoxLink to P4
P4 -> PARAM_CHANGED -> ui_update_parameter(id, value)  // authoritative
```

The editor does not need to change for that integration: the P4 snapshot simply
calls `ui_update_parameter()` for each received value. The UI is not the
permanent authority.

## Tests

`pio test -e native` (no LVGL/Arduino) covers formatters, enum wrap, clamping,
mode-dependent descriptor selection, scale option count, effect lookup, and all
effect summaries (17 tests).
