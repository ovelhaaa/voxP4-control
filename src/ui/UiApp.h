#ifndef UI_APP_H
#define UI_APP_H

#include <lvgl.h>
#include <cstddef>
#include "board/CYD_Config.h"
#include "ui/params/UiParamModel.h"

// Screen IDs - main navigation tabs (0-3 for nav bar, 4+ for subscreens)
enum class UiScreenId {
    PERFORMANCE = 0,
    FX_CHAIN = 1,
    PRESETS = 2,
    FOOTSWITCH = 3,
    SYSTEM = 4,
    EFFECT_EDIT = 5, // Subscreen, not in main nav
    SETTINGS = 6,    // Subscreen for SET tab
    MASTER = 7       // Global routing / master controls
};

// Action layer to decouple UI from State Authority
enum class UiActionType {
    ToggleEffect,
    OpenEffect,
    LoadPreset,
    SavePreset,
    SetFootswitchConfig,
    GlobalBypass,
    AllEffectsOn,
    SetParameter,
    NextSubscene,
    PrevSubscene,
    NextScene,
    PrevScene,
    CommitEdits,
    RevertEdits
};

struct UiAction {
    UiActionType type;
    uint16_t id;
    float value;
};

// UI Intent emitter
void ui_emit_action(const UiAction& action);

// Editable footswitch fields.
enum class UiFootswitchField : uint8_t {
    Mode = 0,
    Press = 1,
    Release = 2,
    LongPress = 3,
    DoublePress = 4,
    Count
};

// Application state
struct UiAppState {
    UiScreenId currentScreen;
    bool linkUp;

    // Preset identity. `currentPresetId` is the preset actually loaded; it only
    // changes on an explicit load. `selectedPresetId` is the row highlighted in
    // the Presets list and is NOT the same thing.
    uint16_t currentPresetId;
    uint16_t selectedPresetId;
    char presetName[32];

    // Local-only global bypass intent. This does NOT alter each effect's
    // enabled state; it is a separate flag pending a real backend (VoxLink).
    bool globalBypass;

    // Canonical parameter state (all VoxLink parameters, keyed by wire ID).
    UiParamState params;

    // Meters
    float inputPeakDb;
    float outputPeakDb;

    // Pitch
    float pitchFreqHz;
    int detectedNote;
    bool voiced;
};

class LGFX_CYD;

// UI initialization
void ui_app_init(LGFX_CYD& display);
void ui_app_run(void);

// Screen navigation
void ui_navigate_to(UiScreenId screen);
UiScreenId ui_get_current_screen(void);
void update_nav_bar(UiScreenId active_screen);

// State updates (called from main loop)
void ui_update_link_state(bool connected);
void ui_update_preset(uint16_t id, const char* name);
void ui_update_meters(float inputDb, float outputDb);
void ui_update_pitch(float freqHz, int note, bool voiced);

// Preset selection is separate from the loaded preset.
void ui_select_preset(int index);
void ui_load_selected_preset(void);
bool ui_is_global_bypass(void);
void ui_commit_edits(void);
void ui_revert_edits(void);
void ui_update_performance_header(void);

// Footswitch fan-out.
void ui_update_footswitch_config(int index, uint8_t mode, uint8_t action);
void ui_update_footswitch_state(int index, bool pressed);
// Local editing (mode / press / release / long / double). Persists to the
// FootswitchManager and fans the change out to every representation.
void ui_set_footswitch_field(int index, UiFootswitchField field, uint8_t value);
uint8_t ui_get_footswitch_field(int index, UiFootswitchField field);

// Short display label for a footswitch action ("HARMONY TOGGLE" -> "HARMONY").
const char* ui_footswitch_short_label(uint8_t action);

// Parameter model access, keyed by VoxLink wire ID. Local edits are optimistic
// (LocalPending) and produce a wire intent; authoritative values come only from
// the P4 and win conflicts.
void ui_set_parameter_local(uint16_t wireId, float value);
void ui_apply_parameter_authoritative(uint16_t wireId, float acceptedValue);
void ui_revert_parameter(uint16_t wireId);
float ui_get_parameter(uint16_t wireId);
float ui_get_authoritative_parameter(uint16_t wireId);
bool ui_parameter_is_valid(uint16_t wireId);
UiValueAuthority ui_parameter_authority(uint16_t wireId);

// Read-only view of the canonical parameter state (views only; never an
// authority). Used for conditional (mode/sync) visibility checks.
const UiParamState& ui_get_param_state(void);

// Effect enable is derived from the canonical enable parameter.
bool ui_effect_enabled(UiEffectId effect);
void ui_set_effect_enable_local(UiEffectId effect, bool enabled);

// Global Tempo. Tap Tempo computes BPM on the CYD and sends it as an ordinary
// TempoBpm parameter; the P4 remains the authority.
void ui_tap_tempo(void);
float ui_get_tempo_bpm(void);

// The App registers one callback so the VoxLink glue can translate local intents
// into wire SET_PARAM requests without the UI depending on the client.
typedef void (*UiWireIntentFn)(uint16_t wireId, float value);
void ui_set_wire_intent_callback(UiWireIntentFn fn);

// Capability gating driven by the VoxLink client (CAPS / link state).
void ui_set_link_capabilities(bool presetsAvailable, bool bypassAvailable);
void ui_refresh_capability_gated_controls();

// Human-readable summary derived from the canonical parameter state (no heap).
void ui_format_effect_summary(UiEffectId effect, char* mainValue,
                              size_t mainSize, char* metadata, size_t metaSize);

#endif  // UI_APP_H
