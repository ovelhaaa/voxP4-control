#ifndef FOOTSWITCH_MANAGER_H
#define FOOTSWITCH_MANAGER_H

#include <stdint.h>
#include <stdbool.h>

// Footswitch modes
typedef enum {
    FS_MODE_MOMENTARY = 0,
    FS_MODE_LATCHING = 1
} FootswitchMode;

// Action types
typedef enum {
    FS_ACTION_NONE = 0,
    FS_ACTION_HARMONY_TOGGLE,
    FS_ACTION_HARMONY_MOMENTARY,
    FS_ACTION_REVERB_TOGGLE,
    FS_ACTION_REVERB_FREEZE,
    FS_ACTION_DELAY_TOGGLE,
    FS_ACTION_TAP_TEMPO,
    FS_ACTION_PRESET_NEXT,
    FS_ACTION_PRESET_PREV,
    FS_ACTION_GLOBAL_BYPASS,
    FS_ACTION_MODULATION_TOGGLE,
    FS_ACTION_SUBSCENE_NEXT,
    FS_ACTION_SUBSCENE_PREV,
    FS_ACTION_SCENE_NEXT,
    FS_ACTION_SCENE_PREV
} FootswitchAction;

// Footswitch configuration
typedef struct {
    FootswitchMode mode;
    FootswitchAction pressAction;
    FootswitchAction releaseAction;
    FootswitchAction longPressAction;
    FootswitchAction doublePressAction;
} FootswitchConfig;

// Footswitch events
typedef enum {
    FS_EVENT_PRESS = 0,
    FS_EVENT_RELEASE,
    FS_EVENT_LONG_PRESS,
    FS_EVENT_DOUBLE_PRESS
} FootswitchEventType;

typedef struct {
    uint8_t index;
    FootswitchEventType type;
} FootswitchEvent;

// Footswitch state
typedef struct {
    bool isPressed;
    bool wasPressed;
    uint32_t pressTime;
    uint32_t lastReleaseTime;
    int pressCount;
    bool longPressFired;
} FootswitchState;

// Event callback type
typedef void (*FootswitchEventCallback)(const FootswitchEvent* event);

// Initialize footswitch manager
void footswitch_manager_init(void);

// Poll footswitches (call every 2-5ms)
void footswitch_manager_poll(void);

// Get configuration for a footswitch
FootswitchConfig* footswitch_get_config(int fsIndex);

// Set configuration for a footswitch
void footswitch_set_config(int fsIndex, FootswitchConfig* config);

// Get current state
bool footswitch_is_pressed(int fsIndex);

// Get action name string
const char* footswitch_action_name(FootswitchAction action);

// Set event callback
void footswitch_set_event_callback(FootswitchEventCallback callback);

// Compatibility wrappers for main.cpp
static inline void footswitch_init(void) { footswitch_manager_init(); }
static inline void footswitch_poll(void) { footswitch_manager_poll(); }

#endif  // FOOTSWITCH_MANAGER_H
