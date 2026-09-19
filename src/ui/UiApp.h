#ifndef UI_APP_H
#define UI_APP_H

#include <lvgl.h>
#include "board/CYD_Config.h"

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
    AllEffectsOn
};

struct UiAction {
    UiActionType type;
    uint16_t id;
    int32_t value;
};

// UI Intent emitter
void ui_emit_action(const UiAction& action);

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

    // Effect states (index order: 0 HARMONY, 1 REVERB, 2 DELAY, 3 LIMITER)
    bool harmonyEnabled;
    bool reverbEnabled;
    bool delayEnabled;
    bool limiterEnabled;
    
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

// Canonical effect display metadata (single source of truth for all screens).
// Index order is fixed: 0 HARMONY, 1 REVERB, 2 DELAY, 3 LIMITER.
const char* ui_effect_name(int effectId);
const char* ui_effect_main_value(int effectId);
const char* ui_effect_metadata(int effectId);

#endif  // UI_APP_H
