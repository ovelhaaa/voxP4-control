#ifndef FOOTSWITCH_SCREEN_H
#define FOOTSWITCH_SCREEN_H

#include <lvgl.h>

// Initialize the footswitch configuration screen
void footswitch_screen_init(lv_obj_t* parent);

// Update functions
void footswitch_update_config(int fsIndex, uint8_t mode, const char* actionName);
void footswitch_update_state(int fsIndex, bool pressed);

#endif  // FOOTSWITCH_SCREEN_H
