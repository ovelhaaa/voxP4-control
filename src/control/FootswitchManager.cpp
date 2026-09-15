#include "FootswitchManager.h"
#include "board/CYD_Config.h"
#include <Arduino.h>

#define NUM_FOOTSWITCHES 2

// GPIO pins for footswitches
static const int fsPins[NUM_FOOTSWITCHES] = {
    VoxCydConfig::Footswitch1Pin,
    VoxCydConfig::Footswitch2Pin
};

// Configurations
static FootswitchConfig fsConfigs[NUM_FOOTSWITCHES];

// States
static FootswitchState fsStates[NUM_FOOTSWITCHES];

// Debounce parameters (from SPECS.md)
#define DEBOUNCE_STABLE_MS 15
#define LONG_PRESS_MS 500
#define DOUBLE_PRESS_WINDOW_MS 300
#define POLL_INTERVAL_MS 2

// Action names
static const char* actionNames[] = {
    "NONE",
    "HARMONY TOGGLE",
    "HARMONY MOMENTARY",
    "REVERB TOGGLE",
    "REVERB FREEZE",
    "DELAY TOGGLE",
    "TAP TEMPO",
    "PRESET NEXT",
    "PRESET PREV",
    "GLOBAL BYPASS"
};

void footswitch_manager_init(void) {
    // Initialize GPIO pins
    for (int i = 0; i < NUM_FOOTSWITCHES; i++) {
        pinMode(fsPins[i], INPUT_PULLUP);
        
        // Default configuration: momentary mode with toggle action
        fsConfigs[i].mode = FS_MODE_MOMENTARY;
        fsConfigs[i].pressAction = (FootswitchAction)(FS_ACTION_HARMONY_TOGGLE + i);
        fsConfigs[i].releaseAction = FS_ACTION_NONE;
        fsConfigs[i].longPressAction = FS_ACTION_NONE;
        fsConfigs[i].doublePressAction = FS_ACTION_NONE;
        
        // Initialize state
        fsStates[i].isPressed = false;
        fsStates[i].wasPressed = false;
        fsStates[i].pressTime = 0;
        fsStates[i].lastReleaseTime = 0;
        fsStates[i].pressCount = 0;
    }
}

void footswitch_manager_poll(void) {
    static uint32_t lastPollTime = 0;
    uint32_t currentTime = millis();
    
    // Poll at specified interval
    if (currentTime - lastPollTime < POLL_INTERVAL_MS) {
        return;
    }
    lastPollTime = currentTime;
    
    for (int i = 0; i < NUM_FOOTSWITCHES; i++) {
        bool rawState = (digitalRead(fsPins[i]) == LOW);  // Active low
        
        // Simple debounce: require stable reading
        static bool lastRawState[NUM_FOOTSWITCHES] = {false, false};
        static uint32_t stateChangeTime[NUM_FOOTSWITCHES] = {0, 0};
        
        if (rawState != lastRawState[i]) {
            stateChangeTime[i] = currentTime;
            lastRawState[i] = rawState;
        }
        
        // Check if state has been stable long enough
        if (currentTime - stateChangeTime[i] >= DEBOUNCE_STABLE_MS) {
            FootswitchState* fs = &fsStates[i];
            
            if (rawState && !fs->isPressed) {
                // Press detected
                fs->isPressed = true;
                fs->pressTime = currentTime;
                
                // Check for double press
                if (currentTime - fs->lastReleaseTime < DOUBLE_PRESS_WINDOW_MS) {
                    fs->pressCount++;
                    if (fs->pressCount >= 2 && fsConfigs[i].doublePressAction != FS_ACTION_NONE) {
                        // Double press action triggered
                        fs->pressCount = 0;
                    }
                } else {
                    fs->pressCount = 1;
                }
                
                // Execute press action based on mode
                if (fsConfigs[i].mode == FS_MODE_LATCHING) {
                    // Toggle on press for latching mode
                    // Actual toggle logic handled by UI/app layer
                }
                
            } else if (!rawState && fs->isPressed) {
                // Release detected
                fs->isPressed = false;
                fs->lastReleaseTime = currentTime;
                uint32_t pressDuration = currentTime - fs->pressTime;
                
                // Check for long press
                if (pressDuration >= LONG_PRESS_MS && fsConfigs[i].longPressAction != FS_ACTION_NONE) {
                    // Long press action triggered
                } else if (fsConfigs[i].mode == FS_MODE_MOMENTARY && 
                           fsConfigs[i].releaseAction != FS_ACTION_NONE) {
                    // Release action for momentary mode
                }
            }
        }
    }
}

FootswitchConfig* footswitch_get_config(int fsIndex) {
    if (fsIndex < 0 || fsIndex >= NUM_FOOTSWITCHES) {
        return nullptr;
    }
    return &fsConfigs[fsIndex];
}

void footswitch_set_config(int fsIndex, FootswitchConfig* config) {
    if (fsIndex < 0 || fsIndex >= NUM_FOOTSWITCHES || !config) {
        return;
    }
    fsConfigs[fsIndex] = *config;
}

bool footswitch_is_pressed(int fsIndex) {
    if (fsIndex < 0 || fsIndex >= NUM_FOOTSWITCHES) {
        return false;
    }
    return fsStates[fsIndex].isPressed;
}

const char* footswitch_action_name(FootswitchAction action) {
    if (action >= sizeof(actionNames) / sizeof(actionNames[0])) {
        return "UNKNOWN";
    }
    return actionNames[action];
}
