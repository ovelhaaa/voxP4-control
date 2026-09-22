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
    SETTINGS = 6     // Subscreen for SET tab
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
    SetParameter
};

struct UiAction {
    UiActionType type;
    uint16_t id;
    float value;
};

// UI Intent emitter
void ui_emit_action(const UiAction& action);

// Where a value came from. LocalDefault at boot, LocalPending after an
// optimistic local edit, Authoritative once the P4 confirms/announces it.
enum class UiValueAuthority : uint8_t {
    LocalDefault = 0,
    LocalPending = 1,
    Authoritative = 2
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

    // Effect states (index order: 0 HARMONY, 1 REVERB, 2 DELAY, 3 LIMITER, 4 MODULATION)
    bool harmonyEnabled;
    bool reverbEnabled;
    bool delayEnabled;
    bool limiterEnabled;
    bool modulationEnabled;
    UiValueAuthority effectAuthority[kUiEffectCount];
    bool effectAuthoritative[kUiEffectCount]; // last P4-confirmed enable (for rollback)
    
    // Parameter model (values indexed by UiParamId). Still a LOCAL simulation
    // until VoxLink provides P4-authoritative snapshots, but it is the single
    // authority the editor and both effect summaries read from.
    float parameterValues[kUiParamCount];
    float parameterAuthoritativeValues[kUiParamCount]; // last P4 value
    bool parameterValid[kUiParamCount];
    UiValueAuthority parameterAuthority[kUiParamCount];

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
void ui_update_effect_state(int effectId, bool enabled);
void ui_update_meters(float inputDb, float outputDb);
void ui_update_pitch(float freqHz, int note, bool voiced);

// Preset selection is separate from the loaded preset. Selecting highlights the
// list row only; loading (see ui_load_selected_preset) changes the current
// preset and fans the change out to every view.
void ui_select_preset(int index);
void ui_load_selected_preset(void);
bool ui_is_global_bypass(void);

// Footswitch fan-out. A physical event must go through these; screens are never
// called directly from main.cpp. `ui_update_footswitch_config` is fed from the
// FootswitchManager during init and drives both the Footswitch screen and the
// Performance summary.
void ui_update_footswitch_config(int index, uint8_t mode, uint8_t action);
void ui_update_footswitch_state(int index, bool pressed);

// Short display label for a footswitch action ("HARMONY TOGGLE" -> "HARMONY").
// Single source of truth for the abbreviated label used by all screens.
const char* ui_footswitch_short_label(uint8_t action);

// Parameter model access. Local edits are optimistic (LocalPending) and produce
// a wire intent; authoritative values come only from the P4 and win conflicts.
void ui_set_parameter_local(UiParamId id, float value);
void ui_apply_parameter_authoritative(UiParamId id, float acceptedValue);
void ui_revert_parameter(UiParamId id);
float ui_get_parameter(UiParamId id);
float ui_get_authoritative_parameter(UiParamId id);
bool ui_parameter_is_valid(UiParamId id);
UiValueAuthority ui_parameter_authority(UiParamId id);

// Effect enable, same authority model.
void ui_set_effect_enable_local(UiEffectId effect, bool enabled);
void ui_apply_effect_enable_authoritative(UiEffectId effect, bool enabled);
void ui_revert_effect_enable(UiEffectId effect);
UiValueAuthority ui_effect_authority(UiEffectId effect);

// The App registers one callback so the VoxLink glue can translate local intents
// into wire SET_PARAM requests without the UI depending on the client.
typedef void (*UiWireIntentFn)(uint16_t wireId, float value);
void ui_set_wire_intent_callback(UiWireIntentFn fn);

// Capability gating driven by the VoxLink client (CAPS / link state).
void ui_set_link_capabilities(bool presetsAvailable, bool bypassAvailable);
// Rebuilds capability-gated controls (currently the Effect Editor body) after a
// CAPS snapshot arrives. No-op when the editor is not open.
void ui_refresh_capability_gated_controls();
// Human-readable summary derived from the parameter state (no heap). Both
// Performance and FX Chain consume this same function.
void ui_format_effect_summary(int effectId, char* mainValue, size_t mainSize,
                              char* metadata, size_t metaSize);

// Canonical effect display name. Index order is fixed:
// 0 HARMONY, 1 REVERB, 2 DELAY, 3 LIMITER.
const char* ui_effect_name(int effectId);

#endif  // UI_APP_H
