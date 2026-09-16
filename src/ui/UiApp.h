#ifndef UI_APP_H
#define UI_APP_H

#include <lvgl.h>
#include "board/CYD_Config.h"

// Screen IDs
enum class UiScreenId {
    PERFORMANCE,
    FX_CHAIN,
    EFFECT_EDIT,
    FOOTSWITCH,
    PRESETS,
    SYSTEM
};

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

// UI initialization
void ui_app_init(void);
void ui_app_run(void);

// Screen navigation
void ui_navigate_to(UiScreenId screen);
UiScreenId ui_get_current_screen(void);

// State updates (called from main loop)
void ui_update_link_state(bool connected);
void ui_update_preset(uint16_t id, const char* name);
void ui_update_effect_state(int effectId, bool enabled);
void ui_update_meters(float inputDb, float outputDb);
void ui_update_pitch(float freqHz, int note, bool voiced);

#endif  // UI_APP_H
