#ifndef PRESETS_SCREEN_H
#define PRESETS_SCREEN_H

#include <lvgl.h>

// Initialize the presets management screen
void presets_screen_init(lv_obj_t* parent);

// Update functions
void presets_update_list(const char** presetNames, int count);
void presets_select_preset(int index);
void presets_update_current(int currentId, const char* currentName);

#endif  // PRESETS_SCREEN_H
