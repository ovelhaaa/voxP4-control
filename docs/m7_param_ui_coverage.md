# M7 — Full parameter UI coverage, master/routing and tempo

Milestone goal: make **every canonical VoxP4 parameter** reachable from the
controller UI without turning the Performance screen into a technical editor,
and eliminate the divergence between the UI parameter model and the VoxLink
registry.

## Architecture: one source of truth

```
voxP4 registry ──(sync)──> src/voxlink/generated/VoxP4ParamIds.h
                                      │
                                      ▼
                       src/model/ParameterRegistry.{h,cpp}
                       ID · type · range · default  (THE source of truth)
                                      │
                    ┌─────────────────┴──────────────────┐
                    ▼                                    ▼
        src/ui/params/UiParamState.{h,cpp}     src/ui/params/UiParamModel.{h,cpp}
        canonical values, indexed by wire ID   presentation metadata ONLY:
        (71 params, pending/confirmed/rollback) label · section · widget ·
                                               uiStep · format · enum labels ·
                                               Basic/Advanced/Global · gating
```

* `UiParamModel` no longer stores `min`/`max`/`default`. It reads them from
  `ParameterRegistry` through `ui_param_min/max/default()`.
* `UiAppState` no longer keeps a partial `UiParamId` enumeration. The canonical
  state is a `UiParamState` sized by `VOXP4_PARAM_COUNT` and keyed by wire ID.
* Effect enables are ordinary parameters (`ui_effect_enable_wire()` maps a
  module to its `*.enable` wire ID); the card LED and the editor header are
  views over that state.
* The P4 remains authoritative:
  `UI -> SetParameter wire ID -> P4 -> PARAM_CHANGED -> UiParamState -> UI`.

Rule enforced by tests + `scripts/audit_ui_coverage.py`: every wire parameter is
classified exactly once as **Effect Enable**, **Basic UI**, **Advanced UI** or
**Master/Global UI**. Adding a parameter to the contract without a UI home fails
`test_ui_coverage_all_parameters`.

## Signal chain

`GATE -> COMPRESSOR -> HARMONY -> DRIVE -> MODULATION -> DELAY -> REVERB`
plus a global `MASTER / ROUTING` area (Tempo, Limiter Ceiling, Mute Dry,
Spatial Routing/Source).

The FX rack and Performance indicators scroll horizontally with snap, so the
seven modules keep large touch targets instead of being squeezed. The former
`LIMITER` card is gone: `harmony.limiter.*` is now `HARMONY -> ADVANCED ->
HARM LIMITER`, and the master `LimiterCeiling` lives in `MASTER`.

## Editors

* Generic `EffectEditScreen` with collapsible `BASIC` (open) / `ADVANCED`
  (closed by default) sections. The split is data-driven via
  `UiVisibility`.
* **Delay**: `TEMPO SYNC` selects between `LEFT/RIGHT` ms and
  `LEFT/RIGHT DIV` (13 canonical subdivisions). Both use the global `TempoBpm`.
* **Modulation**: CHORUS/ENSEMBLE/DIMENSION expose rate/depth/base delay/mix/
  width; `TEMPO SYNC` swaps rate for subdivision. MICROSHIFT exposes
  mix/width/left+right detune (−50..+50 c)/window.
* **Drive**: MODE (WARM/OVERDRIVE/MEGAPHONE), DRIVE, TONE, MIX; OUTPUT is
  Advanced.
* **Gate**: THRESHOLD, RANGE; ATTACK/HOLD/RELEASE Advanced.
* **Compressor**: THRESHOLD, RATIO, MAKEUP; ATTACK/RELEASE/KNEE Advanced.
* **Harmony**: `ADVANCED` holds VOICE smoothing, FORMANT, DYNAMICS, DRY ALIGN,
  MIDI RANGE and HARM LIMITER.

## Tempo and Tap Tempo

`TempoBpm` (0x0010, 30..300, default 120) is a global parameter shown compactly
on Performance and editable in MASTER. `TapTempo` (`src/model/TapTempo.*`, pure
C++) rejects invalid intervals, medians the recent taps, clamps to 30..300 and
sends the result as an ordinary `TempoBpm` parameter. No special protocol.

## Footswitches

`TAP TEMPO` is functional. Actions added: `DRIVE TOGGLE`, `GATE TOGGLE`,
`COMPRESSOR TOGGLE`, `HARMONY MOMENTARY`. `REVERB FREEZE` was removed: there is
no freeze parameter in the registry, so the UI no longer offers a fictional
action. `FootswitchScreen` supports local editing of mode / press / release /
long press / double press behind an explicit `EDIT` affordance.

## Tests

* `pio test -e native` — 82 cases, including `test_ui_coverage_all_parameters`,
  registry range/default parity, mode + sync gating, Delay/Chorus sync, state
  authority + rollback, and TapTempo.
* `python scripts/audit_ui_coverage.py --check` — reports the coverage table and
  fails if a canonical parameter has no UI home or is placed twice.
* Firmware: `pio run -e esp32-cyd`.

## Coverage audit

Generated with `python scripts/audit_ui_coverage.py` (71/71 placed, 0 missing):

| wire ID | parameter | UI location | basic/advanced | control type |
|---|---|---|---|---|
| 0x0010 | tempo.bpm | MASTER / TEMPO / TEMPO | global | Slider |
| 0x0100 | harmony.enable | HARMONY header toggle | basic | Toggle |
| 0x0101 | harmony.interval | HARMONY / TARGET / INTERVAL | basic | Slider |
| 0x0102 | harmony.level | HARMONY / VOICE / LEVEL | basic | Slider |
| 0x0103 | harmony.mode | HARMONY / TARGET / MODE | basic | Segmented |
| 0x0104 | harmony.key | HARMONY / TARGET / KEY | basic | Stepper |
| 0x0105 | harmony.scale | HARMONY / TARGET / SCALE | basic | Stepper |
| 0x0107 | harmony.voice1.pan | HARMONY / VOICE / PAN | basic | Slider |
| 0x0108 | harmony.voice1.degree | HARMONY / TARGET / DEGREE | basic | Slider |
| 0x0109 | harmony.voice1.smoothing_ms | HARMONY / VOICE / SMOOTHING | advanced | Slider |
| 0x010A | harmony.formant.enable | HARMONY / FORMANT / FORMANT | advanced | Toggle |
| 0x010B | harmony.formant.amount | HARMONY / FORMANT / AMOUNT | advanced | Slider |
| 0x010C | harmony.attack_ms | HARMONY / DYNAMICS / ATTACK | advanced | Slider |
| 0x010D | harmony.release_ms | HARMONY / DYNAMICS / RELEASE | advanced | Slider |
| 0x010E | harmony.limiter.enable | HARMONY / HARM LIMITER / ENABLE | advanced | Toggle |
| 0x010F | harmony.limiter.threshold_db | HARMONY / HARM LIMITER / THRESHOLD | advanced | Slider |
| 0x0110 | harmony.dry_alignment.enable | HARMONY / DRY ALIGN / ENABLE | advanced | Toggle |
| 0x0111 | harmony.dry_alignment.ms | HARMONY / DRY ALIGN / DELAY | advanced | Slider |
| 0x0112 | harmony.voice1.non_scale_policy | HARMONY / TARGET / NON-SCALE | basic | Segmented |
| 0x0113 | harmony.voice1.voice_leading | HARMONY / TARGET / VOICE LEAD | basic | Toggle |
| 0x0114 | harmony.voice1.min_midi | HARMONY / MIDI RANGE / MIN NOTE | advanced | Slider |
| 0x0115 | harmony.voice1.max_midi | HARMONY / MIDI RANGE / MAX NOTE | advanced | Slider |
| 0x0200 | compressor.enable | COMPRESSOR header toggle | basic | Toggle |
| 0x0201 | compressor.threshold_db | COMPRESSOR / DETECTOR / THRESHOLD | basic | Slider |
| 0x0202 | compressor.ratio | COMPRESSOR / DETECTOR / RATIO | basic | Slider |
| 0x0203 | compressor.attack_ms | COMPRESSOR / TIMING / ATTACK | advanced | Slider |
| 0x0204 | compressor.release_ms | COMPRESSOR / TIMING / RELEASE | advanced | Slider |
| 0x0205 | compressor.makeup_db | COMPRESSOR / OUTPUT / MAKEUP | basic | Slider |
| 0x0206 | compressor.knee_db | COMPRESSOR / DETECTOR / KNEE | advanced | Slider |
| 0x0207 | gate.enable | GATE header toggle | basic | Toggle |
| 0x0208 | gate.threshold_db | GATE / DETECTOR / THRESHOLD | basic | Slider |
| 0x0209 | gate.attack_ms | GATE / TIMING / ATTACK | advanced | Slider |
| 0x020A | gate.hold_ms | GATE / TIMING / HOLD | advanced | Slider |
| 0x020B | gate.release_ms | GATE / TIMING / RELEASE | advanced | Slider |
| 0x020C | gate.range_db | GATE / DETECTOR / RANGE | basic | Slider |
| 0x0300 | delay.enable | DELAY header toggle | basic | Toggle |
| 0x0301 | delay.left_ms | DELAY / TIME / LEFT | basic | Slider |
| 0x0302 | delay.right_ms | DELAY / TIME / RIGHT | basic | Slider |
| 0x0303 | delay.feedback | DELAY / FEEDBACK / FEEDBACK | basic | Slider |
| 0x0304 | delay.wet | DELAY / MIX / WET | basic | Slider |
| 0x0305 | delay.dry | DELAY / MIX / DRY | basic | Slider |
| 0x0306 | delay.feedback_lowpass_hz | DELAY / FEEDBACK / FILTER | advanced | Slider |
| 0x0307 | delay.sync_enable | DELAY / SYNC / TEMPO SYNC | basic | Toggle |
| 0x0308 | delay.left_subdivision | DELAY / TIME / LEFT DIV | basic | Stepper |
| 0x0309 | delay.right_subdivision | DELAY / TIME / RIGHT DIV | basic | Stepper |
| 0x0400 | reverb.enable | REVERB header toggle | basic | Toggle |
| 0x0401 | reverb.wet | REVERB / REVERB / MIX | basic | Slider |
| 0x0402 | reverb.decay_s | REVERB / REVERB / DECAY | basic | Slider |
| 0x0403 | reverb.damping | REVERB / REVERB / DAMPING | basic | Slider |
| 0x0500 | limiter.ceiling | MASTER / OUTPUT / CEILING | global | Slider |
| 0x0501 | output.mute_dry | MASTER / OUTPUT / MUTE DRY | global | Toggle |
| 0x0502 | output.spatial_routing | MASTER / ROUTING / ROUTING | global | Segmented |
| 0x0503 | output.spatial_source | MASTER / ROUTING / SOURCE | global | Segmented |
| 0x0600 | chorus.enable | MODULATION header toggle | basic | Toggle |
| 0x0601 | chorus.mode | MODULATION / MODE / MODE | basic | Segmented |
| 0x0602 | chorus.mix | MODULATION / MODULATION / MIX | basic | Slider |
| 0x0603 | chorus.sync_enable | MODULATION / ADVANCED / TEMPO SYNC | advanced | Toggle |
| 0x0604 | chorus.rate_hz | MODULATION / LFO / RATE | basic | Slider |
| 0x0605 | chorus.subdivision | MODULATION / LFO / DIVISION | basic | Stepper |
| 0x0606 | chorus.depth_ms | MODULATION / LFO / DEPTH | basic | Slider |
| 0x0607 | chorus.base_delay_ms | MODULATION / ADVANCED / BASE DELAY | advanced | Slider |
| 0x0608 | chorus.width | MODULATION / STEREO / WIDTH | basic | Slider |
| 0x0609 | chorus.microshift_left_cents | MODULATION / DETUNE / LEFT DETUNE | basic | Slider |
| 0x060A | chorus.microshift_right_cents | MODULATION / DETUNE / RIGHT DETUNE | basic | Slider |
| 0x060B | chorus.microshift_window_ms | MODULATION / ADVANCED / WINDOW | advanced | Slider |
| 0x0700 | drive.enable | DRIVE header toggle | basic | Toggle |
| 0x0701 | drive.mode | DRIVE / TONE / MODE | basic | Segmented |
| 0x0702 | drive.drive | DRIVE / TONE / DRIVE | basic | Slider |
| 0x0703 | drive.tone | DRIVE / TONE / TONE | basic | Slider |
| 0x0704 | drive.mix | DRIVE / MIX / MIX | basic | Slider |
| 0x0705 | drive.output_level | DRIVE / OUTPUT / OUTPUT | advanced | Slider |

### Parameters not directly exposed

None. All 71 canonical parameters have a UI placement. Effect `*.enable`
parameters are exposed as the effect's header/card toggle rather than as an
editor row; `harmony.limiter.*` (the harmony bus limiter) is an Advanced section
of HARMONY, and the master `LimiterCeiling` only has a ceiling (there is no
master enable parameter, so none is shown).
