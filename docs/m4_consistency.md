# M4 — UI consistency, interaction and hardening

Milestone focused on making the already-mature UI behave coherently. No global
redesign, no new screens. This document records the behaviours that are
currently local-only because the backend (VoxLink) does not exist yet.

## Single-authority fan-out

### Preset
* `ui_update_preset(id, name)` is the one logical preset change. It fans out to:
  * the Performance header,
  * the Presets current block,
  * the Presets list selection (row `id`).
* `ui_select_preset(index)` changes the **selection only**; it never changes the
  current preset.
* `ui_load_selected_preset()` (triggered by `UiActionType::LoadPreset`) turns the
  selected preset into the current one and fans out as above.
* `UiAppState.currentPresetId` and `UiAppState.selectedPresetId` are kept
  distinct; screens keep only view-local highlight state.

### Footswitch
* A physical event enters through `ui_update_footswitch_state(index, pressed)`.
  `main.cpp` never calls a screen directly and does not format labels.
* `ui_update_footswitch_config(index, mode, action)` is fed from
  `FootswitchManager` at init (`footswitch_get_config`) and fans out to:
  * the Footswitch screen (mode + short action label),
  * the Performance footswitch summary.
* `ui_footswitch_short_label(action)` is the single place that maps
  "HARMONY TOGGLE" -> "HARMONY" etc.

### Effect enabled state
* `ui_update_effect_state(id, enabled)` fans out to Performance, FX Chain and
  Effect Edit.

## Local-only behaviours (no backend yet)

### ALL ON (temporary)
`UiActionType::AllEffectsOn` sets the **local optimistic** enabled state of
Harmony, Reverb, Delay and Limiter to true via `ui_update_effect_state`, so all
views stay consistent. This is a local simulation until VoxLink exists.

### GLOBAL BYPASS (temporary, semantically explicit)
`UiActionType::GlobalBypass` toggles a dedicated `UiAppState.globalBypass` flag
and updates the FX Chain BYPASS button visual. It deliberately **does not**
change any effect's `enabled` state. A real global bypass (and its tail/limiter
semantics) must come from the backend; the flag is the API seam for that.

### Preset LOAD (temporary)
`LoadPreset` is a local placeholder: selected becomes current and views update.
There is no persistence.

## Controls disabled for lack of backend

* **SAVE AS** and **DELETE** are disabled via `ui_apply_disabled()`: not
  clickable, no pressed feedback, muted border and labels, dimmed. They no
  longer look falsely functional.
* The Effect Edit parameter rows remain visual placeholders; the data-driven
  editor is a separate milestone.

## Disabled style helper

`ui_apply_disabled(obj)` in `UiTheme` clears `LV_OBJ_FLAG_CLICKABLE`, removes
pressed feedback, neutralises the border, dims the object and mutes child labels.

## Touch and visuals

* Preset rows: 24 px -> 28 px (list scrolls).
* Preset action buttons and FX actions: 36–48 px effective height, with
  `lv_obj_set_ext_click_area` where useful.
* Effect Card accent rail moved fully inside the card (clip corner), matching
  the FX Chain module language.
* Nav: all tabs share `COLOR_NAV`; the active tab is a 2 px orange top rail plus
  primary text; pressed shows a subtle elevated surface.
* VuMeter: the bar stays turquoise at all levels; a small right-side marker
  indicates clip (threshold -1 dB) with an 800 ms hold.
* System screen: values are neutral (`COLOR_TEXT_PRIMARY`) and only turn
  warning/error on real conditions (underruns, UART/CRC errors, reconnects,
  CPU thresholds). With no backend, values show `--`, never fake telemetry.

## Memory / LVGL

* Optional `DEBUG_BUILD` logging prints the heap after each screen is created and
  the initial heap before screens; nothing is printed from the main loop.
* Unused LVGL features disabled: `LV_USE_MENU`, `LV_USE_WIN`, `LV_USE_TILEVIEW`,
  `LV_USE_SPAN`, `LV_USE_LIST`, `LV_USE_BTNMATRIX` (and `LV_USE_MSGBOX`),
  `LV_USE_IMAGE`, `LV_USE_GRADIENT_SIMPLE`, `LV_USE_GRID`. Flash reduced by
  ~17 KB.
* Fonts unchanged (Montserrat 10/12/14/16); no Montserrat 20 added.

## Remaining limitations

* No VoxLink/UART backend: preset load, ALL ON and BYPASS are local simulations.
* SAVE AS / DELETE disabled until persistence exists.
* Effect Edit parameters are not backend-driven.
* Mockups updated only where this milestone changed the visuals.
* Hardware validation (touch calibration, contrast under stage light) pending.
