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
    uint16_t presetId;
    char presetName[32];
    
    // Effect states
    bool harmonyEnabled;
    bool reverbEnabled;
    bool limiterEnabled;
    bool delayEnabled;
    
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

#endif  // UI_APP_H
