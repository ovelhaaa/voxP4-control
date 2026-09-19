# M5.1 — VoxLink contract cleanup, parameter ABI hardening and CI

Milestone goal: eliminate divergences between the ESP32-P4 VoxLink registry,
the exported artifacts, the documentation and the controller parameter model,
and add CI that makes schema drift a build failure. No UART/VoxLink transport is
implemented here (that is M6).

## Source of truth

```
components/voxlink/src/voxlink_registry.cpp   -- the ONLY authority
        │  tools/export_voxlink_schema.py
        ▼
voxP4/integration/VoxP4ParamIds.h             -- generated
voxP4/integration/voxlink_params.json         -- generated
        │  scripts/sync_voxlink_schema.py (controller)
        ▼
voxP4-control/src/voxlink/generated/*         -- versioned copy
```

Never edit generated files by hand. `voxP4/integration/voxlink_v1_vectors.h`
(golden frames) is hand-maintained in voxP4 and synchronized to the controller.

## Parameter count

Current schema: **49** parameters
(`VOXP4_PARAM_COUNT == 49`). The count is never hardcoded in logic; tests and
CI compare the generated count against `registry_count()`.

```
0x0000–0x00FF system     0x0100–0x01FF harmony (21)
0x0200–0x02FF dynamics   0x0300–0x03FF delay (7)
0x0400–0x04FF reverb (4) 0x0500–0x05FF output (4)
```

## Type fixes in this milestone

* `0x0112 harmony.voice1.non_scale_policy` changed from `PFLOAT` to **ENUM 0..2**
  (0 NearestScale, 1 PreserveChromatic, 2 BypassHarmony). VoxLink now rejects
  fractional/out-of-range values and the wrong wire tag.
* `0x0113 harmony.voice1.voice_leading` is **BOOL** (covered by tests).
* `0x0114 / 0x0115 harmony.voice1.min_midi / max_midi` are present in the
  exported schema. They are NOT exposed in the current Effect Editor:
  `SUPPORTED BY PROTOCOL != EXPOSED BY CURRENT UI`.
* `0x0303 delay.feedback` range decided: **-0.95 .. +0.95** (matches the DSP's
  `StereoDelay::set_feedback` clamp). Negative feedback is musically valid
  (inverted-phase delay). The controller and the registry now agree.

## UiParamId vs VoxLink ParamId

The controller keeps its own logical `UiParamId` (editor ergonomics). Each
descriptor stores the wire `voxlinkId`, now taken from the generated
`VOXP4_PARAM_*` constants instead of magic literals:

```cpp
{UiParamId::HarmonyMode, ..., VOXP4_PARAM_HARMONY_MODE, ...}
```

`ui_param_voxlink_id(UiParamId)` returns the wire ID. `UiParamId` is never used
as a wire ordinal and the enum order is not an ABI.

### Mapping (exposed parameters)

| UiParamId | VoxLink constant | ID |
|---|---|---|
| HarmonyMode | VOXP4_PARAM_HARMONY_MODE | 0x0103 |
| HarmonyInterval | VOXP4_PARAM_HARMONY_INTERVAL | 0x0101 |
| HarmonyDegree | VOXP4_PARAM_HARMONY_VOICE1_DEGREE | 0x0108 |
| HarmonyKey | VOXP4_PARAM_HARMONY_KEY | 0x0104 |
| HarmonyScale | VOXP4_PARAM_HARMONY_SCALE | 0x0105 |
| HarmonyNonScalePolicy | VOXP4_PARAM_HARMONY_VOICE1_NON_SCALE_POLICY | 0x0112 |
| HarmonyVoiceLeading | VOXP4_PARAM_HARMONY_VOICE1_VOICE_LEADING | 0x0113 |
| HarmonyLevel | VOXP4_PARAM_HARMONY_LEVEL | 0x0102 |
| HarmonyPan | VOXP4_PARAM_HARMONY_VOICE1_PAN | 0x0107 |
| HarmonySmoothingMs | VOXP4_PARAM_HARMONY_VOICE1_SMOOTHING_MS | 0x0109 |
| HarmonyAttackMs | VOXP4_PARAM_HARMONY_ATTACK_MS | 0x010C |
| HarmonyReleaseMs | VOXP4_PARAM_HARMONY_RELEASE_MS | 0x010D |
| FormantEnabled | VOXP4_PARAM_HARMONY_FORMANT_ENABLE | 0x010A |
| FormantAmount | VOXP4_PARAM_HARMONY_FORMANT_AMOUNT | 0x010B |
| ReverbWet | VOXP4_PARAM_REVERB_WET | 0x0401 |
| ReverbDecaySeconds | VOXP4_PARAM_REVERB_DECAY_S | 0x0402 |
| ReverbDamping | VOXP4_PARAM_REVERB_DAMPING | 0x0403 |
| DelayLeftMs | VOXP4_PARAM_DELAY_LEFT_MS | 0x0301 |
| DelayRightMs | VOXP4_PARAM_DELAY_RIGHT_MS | 0x0302 |
| DelayFeedback | VOXP4_PARAM_DELAY_FEEDBACK | 0x0303 |
| DelayWet | VOXP4_PARAM_DELAY_WET | 0x0304 |
| DelayDry | VOXP4_PARAM_DELAY_DRY | 0x0305 |
| DelayFeedbackLowpassHz | VOXP4_PARAM_DELAY_FEEDBACK_LOWPASS_HZ | 0x0306 |
| LimiterThresholdDb | VOXP4_PARAM_HARMONY_LIMITER_THRESHOLD_DB | 0x010F |

## Effect enable mapping

`ui_effect_enable_voxlink_id()` is the single place that maps a visual module to
its enable parameter:

```
HARMONY -> VOXP4_PARAM_HARMONY_ENABLE          (0x0100)
REVERB  -> VOXP4_PARAM_REVERB_ENABLE           (0x0400)
DELAY   -> VOXP4_PARAM_DELAY_ENABLE            (0x0300)
LIMITER -> VOXP4_PARAM_HARMONY_LIMITER_ENABLE  (0x010E)
```

LIMITER means the **harmony bus limiter**, never the master `LimiterCeiling`
(0x0500), which has no enable parameter and is not exposed.

## uiStep vs wire step

`UiParamDescriptor::uiStep` is the UI/touch ergonomics resolution; the wire
step lives in `integration/voxlink_params.json` and is authoritative for
validation. They may differ (larger UI step, e.g. `harmony.level` wire 0.001 vs
UI 0.01) as long as **ranges remain within the wire range**. The UI clamps to
its descriptor range, which is a subset/equal of the wire range.

## Local defaults / authority

Until M6, values and enables are a local simulation:

* continuous/discrete defaults come from the descriptor defaults (which mirror
  the registry defaults);
* enable defaults come from `ui_effect_default_enabled()`
  (Harmony OFF, Reverb ON, Delay ON, Limiter ON) — never zero-initialization;
* `UiValueAuthority` marks each value `LocalDefault` until M6 sets
  `Authoritative` on GET_STATE/PARAM_CHANGED.

## Effect Edit behavior fixes

* `effect_edit_notify()` ignores parameters that do not belong to the open
  effect (`ui_effect_of(id) != current`).
* Only `UiParamId::HarmonyMode` triggers a body rebuild. Other Segmented
  controls (e.g. NON-SCALE) update in place and do not reset scroll.
* Toggle and Segmented visuals are centralized (`set_toggle_visual`,
  `set_segmented_visual`) and used by creation, local events and remote updates,
  so an authoritative update looks identical to a local tap.

## CI

### voxP4 (`voxP4-contract` job in `.github/workflows/esp-idf.yml`)

1. `python tools/export_voxlink_schema.py`
2. `git diff --exit-code -- integration/` (fails with an explicit message if
   generated artifacts are stale)
3. build + run the VoxLink host tests (`ctest -R voxlink`)

### voxP4-control (`.github/workflows/ci.yml`)

* `native-tests`: `pio test -e native`
* `firmware`: `pio run -e esp32-cyd-release`, `pio run -e esp32-cyd-debug`,
  uploads the release firmware artifact.
* PlatformIO cache: `~/.platformio/.cache` only (never `.pio/build`).

## Deliberately still absent

No UART/driver, no parser, no HELLO/CAPS/GET_STATE/SET_PARAM/heartbeat. M6 will
mount the transport on top of this ABI and call `ui_update_parameter()` with
authoritative values.
